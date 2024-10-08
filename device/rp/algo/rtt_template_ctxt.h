/*
 * Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
 *
 * This software product is a proprietary product of NVIDIA CORPORATION &
 * AFFILIATES (the "Company") and all right, title, and interest in and to the
 * software product, including all associated intellectual property rights, are
 * and shall remain exclusively with the Company.
 *
 * This software product is governed by the End User License Agreement
 * provided with the software product.
 *
 */

#ifndef RTT_TEMPLATE_CTXT_H_
#define RTT_TEMPLATE_CTXT_H_
//#define DOCA_PCC_NP_RX_RATE 1
typedef struct {
	uint8_t was_nack : 1; /* Signal the reception of a NACK */
	uint8_t was_cnp : 1;  /* Signal the reception of a CNP */

	uint8_t up_protect_token;
	uint8_t down_protect_token;
	uint8_t up_congestion_token;
	uint8_t down_congestion_token;

	uint8_t reserved : 2; /* Reserved bits */
} rtt_template_flags_t;

typedef struct {
	uint32_t cur_rate;	     /* Current rate */
	uint32_t start_delay;	     /* The time at which the RTT packet was sent by the NIC's Tx pipe */
	uint32_t rtt;		     /* Value of the last measured round trip time */
	rtt_template_flags_t flags;  /* Flags struct */
	uint8_t abort_cnt;	     /* Counter of abort RTT requests */
	uint8_t rtt_meas_psn;	     /* RTT request sequence number */
	uint8_t rtt_req_to_rtt_sent; /* Set between the algorithm's RTT request until the time at which the RTT packet
					was sent */
#ifdef DOCA_PCC_NP_RX_RATE
	uint32_t last_np_rx_bytes;		/* Last received NP RX bytes */
	uint32_t last_np_rx_bytes_timestamp_us; /* Last received NP RX timestamp (usec) */
	
	uint32_t rx_rate;
	uint32_t pro_rate;
	uint32_t con_rate;
	uint32_t min_rtt;
	uint32_t last_rtt;	
	uint32_t last_rx_time;

	uint32_t reserved[0];/* Reserved bits */
#else
	uint32_t last_rate;
	uint32_t pro_rate;
	uint32_t con_rate;
	uint32_t min_rtt;
	uint32_t last_rtt;
	uint32_t last_rx_time;
	uint32_t rx_rate;





	uint32_t reserved[2]; /* Reserved bits */
#endif
} cc_ctxt_rtt_template_t;

#endif /* RTT_TEMPLATE_CTXT_H_ */
