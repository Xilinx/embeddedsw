/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef __FREERTOS_TCP_PERF_CLIENT_H_
#define __FREERTOS_TCP_PERF_CLIENT_H_

#include "lwipopts.h"
#include "xlwipconfig.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "xil_printf.h"
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
	TCP_DONE_CLIENT,
	/* Remote side aborted the test */
	TCP_ABORTED_REMOTE
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
	struct interim_report i_report;
};

/* seconds between periodic bandwidth reports */
#define INTERIM_REPORT_INTERVAL 5

#define TCP_CLIENT_THREAD_STACKSIZE 2048

/* Client port to connect */
#define TCP_CONN_PORT 5001

/* time in seconds to transmit packets */
#define TCP_TIME_INTERVAL 300

#if LWIP_IPV6==1
/* Server to connect with */
#define TCP_SERVER_IPV6_ADDRESS "fe80::6600:6aff:fe71:fde6"
#else
/* Server to connect with */
#define TCP_SERVER_IP_ADDRESS "192.168.1.100"
#endif

/* For ZynqMP GEM, TCP traffic stopped after few seconds.
 * To avoid this, tuned send buffer size.
 */
#if defined (__aarch64__) && defined (XLWIP_CONFIG_INCLUDE_GEM)
#define TCP_SEND_BUFSIZE (6000)
#else
#define TCP_SEND_BUFSIZE (5*TCP_MSS)
#endif /* __aarch64__ */

#endif /* __FREERTOS_TCP_PERF_CLIENT_H_ */
