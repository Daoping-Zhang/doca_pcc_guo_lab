import pyverbs as pv
import pyverbs.qp as qp
import pyverbs.pd as pd
import pyverbs.mr as mr
import pyverbs.cq as cq
import time
import numpy as np

# Parameters
WQE_SIZE = 12.5 * 1024 * 1024  # 12.5 MB
NUM_WQEs = 5
WAIT_TIME_MS = 1

# Setup RDMA resources
ctx = pv.context.Context(name='mlx5_0')  # Replace with your device name
protection_domain = pd.ProtectionDomain(ctx)
completion_queue = cq.CompletionQueue(ctx, 16)

# Create Queue Pair (QP)
qp_init_attr = qp.QPInitAttr(qp_type=qp.IBV_QPT_RC,
                             send_cq=completion_queue,
                             recv_cq=completion_queue,
                             cap=qp.QPCap(max_send_wr=10, max_recv_wr=10, max_send_sge=1, max_recv_sge=1))
queue_pair = qp.QueuePair(protection_domain, qp_init_attr)

# Modify QP to INIT, RTR, and then RTS states (not shown here for brevity)
# This requires exchanging QP info with the peer node

# Allocate memory for the message
buffer = np.zeros(int(WQE_SIZE), dtype=np.uint8)
mr_obj = mr.MemoryRegion(protection_domain, buffer, pv.enums.IBV_ACCESS_LOCAL_WRITE | pv.enums.IBV_ACCESS_REMOTE_WRITE)

# Send loop
for i in range(NUM_WQEs):
    # Create send work request
    send_wr = qp.SendWR(wr_id=i, sg_list=[qp.SGE(addr=mr_obj.buf, length=mr_obj.length, lkey=mr_obj.lkey)],
                        opcode=qp.IBV_WR_SEND)
    
    # Post send request to QP
    queue_pair.post_send(send_wr)
    print(f"WQE {i+1} of {NUM_WQEs} sent.")
    
    # Wait for a completion
    comp = completion_queue.poll(timeout=1000)  # Timeout in ms; adjust as necessary
    if not comp:
        print("Timeout waiting for completion.")
    else:
        print("Completion received for WQE.")

    # Wait before next WQE
    time.sleep(WAIT_TIME_MS / 1000.0)  # Convert ms to seconds
