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
/* Connection handle for a TCP Client session */

#include "freertos_tcp_perf_client.h"

static char send_buf[TCP_SEND_BUFSIZE];
static struct perf_stats client;

/* Report interval time in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)
/* End time in ms */
#define END_TIME (TCP_TIME_INTERVAL * 1000)

void print_app_header()
{
	xil_printf("TCP client connecting to %s on port %d\r\n",
			TCP_SERVER_IP_ADDRESS, TCP_CONN_PORT);
	xil_printf("On Host: Run $iperf -s -i %d -w 2M\r\n",
			INTERIM_REPORT_INTERVAL);
}

void print_tcp_txperf_header(int sock)
{
	int size;
	struct sockaddr_in local, remote;
	size = sizeof(local);
	getsockname(sock, (struct sockaddr *) &local, (socklen_t *)&size);
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet_ntoa(local.sin_addr),
			ntohs(local.sin_port));

	getpeername(sock, (struct sockaddr *) &remote, (socklen_t *) &size);
	xil_printf("%s port %d\r\n", inet_ntoa(remote.sin_addr),
			ntohs(local.sin_port));

	xil_printf("[ ID] Interval    Transfer     Bandwidth\n\r");

}

static void stats_buffer(char* outString, double data, enum measure_t type)
{
	int conv = KCONV_UNIT;
	const char *format;
	double unit = 1024.0;

	if (type == SPEED)
		unit = 1000.0;

	while (data >= unit && conv <= KCONV_GIGA) {
		data /= unit;
		conv++;
	}

	/* Fit data in 4 places */
	if (data < 9.995) { /* 9.995 rounded to 10.0 */
		format = "%4.2f %c"; /* #.## */
	} else if (data < 99.95) { /* 99.95 rounded to 100 */
		format = "%4.1f %c"; /* ##.# */
	} else {
		format = "%4.0f %c"; /* #### */
	}
	sprintf(outString, format, data, kLabel[conv]);
}

/** The report function of a TCP client session */
static void tcp_conn_report(u64_t diff, enum report_type report_type)
{
	u64_t total_len;
	double duration, bandwidth = 0;
	char data[16], perf[16], time[64];

	if (report_type == INTER_REPORT) {
		total_len = client.i_report.total_bytes;
	} else {
		client.i_report.last_report_time = 0;
		total_len = client.total_bytes;
	}

	/* Converting duration from milliseconds to secs,
	 * and bandwidth to bits/sec .
	 */
	duration = diff / 1000.0; /* secs */
	if (duration)
		bandwidth = (total_len / duration) * 8.0;

	stats_buffer(data, total_len, BYTES);
	stats_buffer(perf, bandwidth, SPEED);

	/* On 32-bit platforms, xil_printf is not able to print
	 * u64_t values, so converting these values in strings and
	 * displaying results
	 */
	sprintf(time, "%4.1f-%4.1f sec",
			(double)client.i_report.last_report_time,
			(double)(client.i_report.last_report_time + duration));
	xil_printf("[%3d] %s  %sBytes  %sbits/sec\n\r", client.client_id,
			time, data, perf);

	if (report_type == INTER_REPORT)
		client.i_report.last_report_time += duration;
}

int tcp_send_perf_traffic(int sock)
{
	int bytes_send;
	u8_t apiflags = MSG_MORE;

	client.start_time = sys_now() * portTICK_RATE_MS;
	client.client_id++;
	client.i_report.last_report_time = 0;
	client.i_report.start_time = 0;
	client.i_report.total_bytes = 0;
	client.total_bytes = 0;

	print_tcp_txperf_header(sock);

#if (defined (__aarch64__) && defined (XLWIP_CONFIG_INCLUDE_GEM))
	 /* For ZynqMP A53 GEM, TCP traffic stopped after few seconds with
	 * MSG_MORE flag. To avoid this reset the flag. */
	apiflags = 0;
#endif

	while (1) {
		if ((bytes_send = lwip_send(sock, send_buf, sizeof(send_buf),
						apiflags)) < 0) {
			xil_printf("TCP Client: Either connection aborted"
					" from remote or Error on tcp_write\r\n");
			u64_t now = sys_now() * portTICK_RATE_MS;
			u64_t diff_ms = now - client.start_time;
			tcp_conn_report(diff_ms, TCP_ABORTED_REMOTE);
			break;
		}

		client.total_bytes += bytes_send;

		if (END_TIME || REPORT_INTERVAL_TIME) {
			u64_t now = sys_now() * portTICK_RATE_MS;
			if (REPORT_INTERVAL_TIME) {
				client.i_report.total_bytes += bytes_send;
				if (client.i_report.start_time) {
					u64_t diff_ms = now -
						client.i_report.start_time;
					if (diff_ms >= REPORT_INTERVAL_TIME) {
						tcp_conn_report(diff_ms,
								INTER_REPORT);
						client.i_report.start_time = 0;
						client.i_report.total_bytes = 0;
					}
				} else {
					client.i_report.start_time = now;
				}
			}

			if (END_TIME) {
				/* this session is time-limited */
				u64_t diff_ms = now - client.start_time;
				if (diff_ms >= END_TIME) {
					/* time specified is over */
					/* close the connection */
					tcp_conn_report(diff_ms,
							TCP_DONE_CLIENT);
					xil_printf("TCP test passed Successfully\n\r");
					break;
				}
			}
		}
	}

	close(sock);
	return ERR_OK;
}

void start_application(void)
{
	int sock, i;
	struct sockaddr_in address;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xil_printf("TCP Client: Error in creating Socket\r\n");
		return;
	}

	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(TCP_CONN_PORT);
	address.sin_addr.s_addr = inet_addr(TCP_SERVER_IP_ADDRESS);

	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
		xil_printf("TCP Client: Error on tcp_connect\r\n");
		close(sock);
		return;
	}

	/* initialize data buffer being sent */
	for (i = 0; i < TCP_SEND_BUFSIZE; i++)
		send_buf[i] = (i % 10) + '0';

	tcp_send_perf_traffic(sock);
}
