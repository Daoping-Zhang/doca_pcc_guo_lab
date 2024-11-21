#include <infiniband/verbs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 13107200  // 12.5MB
#define PORT 12345  // TCP端口
#define NUM_WQES 5
#define WAIT_TIME_US 1000  // 1ms in microseconds

struct qp_info {
    uint32_t qp_num;
    uint16_t lid;
    uint8_t gid[16];
};

void die(const char *reason) {
    perror(reason);
    exit(EXIT_FAILURE);
}

int main() {
    // 1. 创建TCP套接字并连接到接收方
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("Socket creation failed");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("192.10.10.101");  // 替换为接收方的IP地址

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        die("TCP connect failed");

    // 2. 获取RDMA设备并创建上下文
    struct ibv_device **dev_list = ibv_get_device_list(NULL);
    struct ibv_context *ctx = ibv_open_device(dev_list[0]);
    struct ibv_pd *pd = ibv_alloc_pd(ctx);
    struct ibv_cq *cq = ibv_create_cq(ctx, 16, NULL, NULL, 0);

    // 3. 创建Queue Pair (QP)
    struct ibv_qp_init_attr qp_init_attr = {
        .send_cq = cq,
        .recv_cq = cq,
        .cap = {
            .max_send_wr = 1,
            .max_recv_wr = 1,
            .max_send_sge = 1,
            .max_recv_sge = 1
        },
        .qp_type = IBV_QPT_RC
    };
    struct ibv_qp *qp = ibv_create_qp(pd, &qp_init_attr);

    // 4. 获取本地QP信息
    struct ibv_port_attr port_attr;
    ibv_query_port(ctx, 1, &port_attr);
    struct qp_info local_qp_info = {
        .qp_num = qp->qp_num,
        .lid = port_attr.lid
    };
    memset(local_qp_info.gid, 0, sizeof(local_qp_info.gid));

    // 5. 通过TCP发送本地QP信息，并接收远端的QP信息
    write(sockfd, &local_qp_info, sizeof(local_qp_info));
    struct qp_info remote_qp_info;
    read(sockfd, &remote_qp_info, sizeof(remote_qp_info));
    close(sockfd);

    // 6. 将QP转换到RTR（Ready to Receive）状态
    struct ibv_qp_attr attr = {
        .qp_state = IBV_QPS_RTR,
        .path_mtu = IBV_MTU_256,
        .dest_qp_num = remote_qp_info.qp_num,
        .rq_psn = 0,
        .max_dest_rd_atomic = 1,
        .min_rnr_timer = 12,
        .ah_attr = {
            .is_global = 0,
            .dlid = remote_qp_info.lid,
            .sl = 0,
            .src_path_bits = 0,
            .port_num = 1
        }
    };
    ibv_modify_qp(qp, &attr, IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
                          IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);

    // 7. 将QP转换到RTS（Ready to Send）状态
    attr.qp_state = IBV_QPS_RTS;
    attr.timeout = 14;
    attr.retry_cnt = 7;
    attr.rnr_retry = 7;
    attr.sq_psn = 0;
    attr.max_rd_atomic = 1;
    ibv_modify_qp(qp, &attr, IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
                          IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);

    // 8. 分配和注册发送缓冲区
    void *buf = malloc(SIZE);
    struct ibv_mr *mr = ibv_reg_mr(pd, buf, SIZE, IBV_ACCESS_LOCAL_WRITE);

    // 9. 循环发送WQE
    for (int i = 0; i < NUM_WQES; i++) {
        struct ibv_sge sge = {
            .addr = (uintptr_t)buf,
            .length = SIZE,
            .lkey = mr->lkey
        };
        struct ibv_send_wr wr = {
            .wr_id = i,
            .sg_list = &sge,
            .num_sge = 1,
            .opcode = IBV_WR_SEND,
            .send_flags = IBV_SEND_SIGNALED
        }, *bad_wr;

        if (ibv_post_send(qp, &wr, &bad_wr)) {
            die("Failed to post send request");
        }

        // 等待完成队列通知
        struct ibv_wc wc;
        while (ibv_poll_cq(cq, 1, &wc) < 1);
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "Send failed with status %d\n", wc.status);
            break;
        }

        printf("WQE %d of %d sent\n", i + 1, NUM_WQES);
        usleep(WAIT_TIME_US);
    }

    // 清理资源
    ibv_dereg_mr(mr);
    ibv_dealloc_pd(pd);
    ibv_close_device(ctx);
    free(buf);
    return 0;
}
