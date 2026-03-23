/*
 * Copyright (C) 2017 - 2022 Xilinx, Inc.
 * Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#ifndef __UDP_PERF_CLIENT_H_
#define __UDP_PERF_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lwipopts.h"
#include "xlwipconfig.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "xil_printf.h"
#include "platform.h"
#include <sleep.h>

/* used as indices into kLabel[] */
enum {
	KCONV_UNIT,
	KCONV_KILO,
	KCONV_MEGA,
	KCONV_GIGA,
};

/* labels for formats [KMG] */
const char kLabel[] =
{
	' ',
	'K',
	'M',
	'G'
};

/* used as type of print */
enum measure_t {
	BYTES,
	SPEED
};

/* Report Type */
enum report_type {
	/* The Intermediate report */
	INTER_REPORT,
	/* The client side test is done */
	UDP_DONE_CLIENT,
	/* Remote side aborted the test */
	UDP_ABORTED_REMOTE
};

struct interim_report {
	u64_t start_time;
	u64_t last_report_time;
	u32_t total_bytes;
};

struct perf_stats {
	u8_t client_id;
	u64_t start_time;
	u64_t total_bytes;
	u64_t cnt_datagrams;
	struct interim_report i_report;
};


/**
 * This structure defines the iperf2 UDP client test header sent to the server
 * at the beginning of a performance test session. It contains the UDP datagram
 * header fields (id, tv_sec, tv_usec, id2) followed by test configuration
 * parameters such as thread count, port, buffer size, and test duration or
 * byte amount.
 */
typedef struct udp_client_test_hdr_v1 {
	/* UDP datagram fields */
	u32_t id;           /**< Unique datagram identifier. */
	u32_t tv_sec;       /**< Timestamp seconds component. */
	u32_t tv_usec;      /**< Timestamp microseconds component. */
	u32_t id2;          /**< Secondary datagram identifier. */
	/* client_hdr fields */
	u32_t flags;        /**< Test configuration flags. */
	u32_t num_threads;  /**< Number of parallel threads for the test. */
	u32_t remote_port;  /**< Remote server port number. */
	u32_t buffer_len;   /**< Length of the data buffer in bytes. */
	u32_t win_band;     /**< Window size or bandwidth limit. */
	u32_t amount;       /**< Test duration (negative) or bytes to transfer. */
} udp_client_test_hdr;

/* seconds between periodic bandwidth reports */
#define INTERIM_REPORT_INTERVAL 5

/* Client port to connect */
#define UDP_CONN_PORT 5001

/* time in seconds to transmit packets */
#define UDP_TIME_INTERVAL 300

/* Server to connect with */
#define UDP_SERVER_IP_ADDRESS "192.168.1.100"

/* UDP buffer length in bytes */
#define UDP_SEND_BUFSIZE 1440

/* MAX UDP send retries */
#define MAX_SEND_RETRY 10

/**
 * @def SEND_RETRY_DELAY_US
 * @brief Retry delay in microseconds between UDP send attempts.
 */
#define SEND_RETRY_DELAY_US 100

/**
 * @def IPERF_HDR_SEND_DELAY_US
 * @brief Delay after sending iperf header in microseconds (100ms).
 */
#define IPERF_HDR_SEND_DELAY_US 100000

/* Number of parallel UDP clients */
#define NUM_OF_PARALLEL_CLIENTS 1

#ifdef __cplusplus
}
#endif

#endif /* __UDP_PERF_CLIENT_H_ */
