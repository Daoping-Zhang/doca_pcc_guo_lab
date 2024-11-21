import pyverbs as pv
import pyverbs.qp as qp
import pyverbs.pd as pd
import pyverbs.mr as mr
import pyverbs.cq as cq
import numpy as np

# Parameters
WQE_SIZE = 12.5 * 1024 * 1024  # 12.5 MB
NUM_WQEs = 5

# Setup RDMA resources
ctx = pv.context.Context(name='mlx5_1')  # Replace with your device name
protection_domain = pd.ProtectionDomain(ctx)
completion_queue = cq.CompletionQueue(ctx, 16)

# Create Queue Pair (QP)
qp_init_attr = qp.QPInitAttr(qp_type=qp.IBV_QPT_RC,
                             send_cq=completion_queue,
                             recv_cq=completion_queue,
                             cap=qp.QPCap(max_send_wr=10, max_recv_wr=10, max_send_sge=1, max_recv_sge=1))
queue_pair = qp.QueuePair(protection_domain, qp_init_attr)

# Modify QP to INIT, RTR, and RTS states (not shown here for brevity)
# This requires exchanging QP info with the peer node

# Allocate memory for the receiving buffer
recv_buffer = np.zeros(int(WQE_SIZE), dtype=np.uint8)
mr_obj = mr.MemoryRegion(protection_domain, recv_buffer, pv.enums.IBV_ACCESS_LOCAL_WRITE)

# Post receive requests
recv_wrs = []
for i in range(NUM_WQEs):
    recv_wr = qp.RecvWR(wr_id=i, sg_list=[qp.SGE(addr=mr_obj.buf, length=mr_obj.length, lkey=mr_obj.lkey)])
    recv_wrs.append(recv_wr)

for recv_wr in recv_wrs:
    queue_pair.post_recv(recv_wr)

# Poll for completions
for i in range(NUM_WQEs):
    comp = completion_queue.poll(timeout=1000)  # Timeout in ms; adjust as necessary
    if not comp:
        print("Timeout waiting for completion.")
    else:
        print(f"Received WQE {i+1} of {NUM_WQEs}. Data received.")

# Optional: Access data from recv_buffer if needed
