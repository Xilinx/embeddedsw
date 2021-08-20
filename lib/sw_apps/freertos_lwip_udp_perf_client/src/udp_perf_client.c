/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
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

/* Connection handle for a UDP Client session */

#include "udp_perf_client.h"

extern struct netif server_netif;
static struct perf_stats client;
static char send_buf[UDP_SEND_BUFSIZE];
static struct sockaddr_in addr;
static int sock[NUM_OF_PARALLEL_CLIENTS];
#define FINISH	1
/* Report interval time in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)
/* End time in ms */
#define END_TIME (UDP_TIME_INTERVAL * 1000)

void print_app_header(void)
{
	xil_printf("UDP client connecting to %s on port %d\r\n",
			UDP_SERVER_IP_ADDRESS, UDP_CONN_PORT);
	xil_printf("On Host: Run $iperf -s -i %d -u\r\n\r\n",
			INTERIM_REPORT_INTERVAL);
}

static void print_udp_conn_stats(void)
{
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet_ntoa(server_netif.ip_addr),
			UDP_CONN_PORT);
	xil_printf("%s port %d\r\n", UDP_SERVER_IP_ADDRESS,
			UDP_CONN_PORT);
	xil_printf("[ ID] Interval\t\tTransfer   Bandwidth\n\r");
}

static void stats_buffer(char* outString,
		double data, enum measure_t type)
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


/* The report function of a UDP client session */
static void udp_conn_report(u64_t diff,
		enum report_type report_type)
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
	else
		xil_printf("[%3d] sent %llu datagrams\n\r",
				client.client_id, client.cnt_datagrams);
}


static void reset_stats(void)
{
	client.client_id++;

	/* Print connection statistics */
	print_udp_conn_stats();

	/* Save start time for final report */
	client.start_time = sys_now();
	client.total_bytes = 0;
	client.cnt_datagrams = 0;

	/* Initialize Interim report parameters */
	client.i_report.start_time = 0;
	client.i_report.total_bytes = 0;
	client.i_report.last_report_time = 0;
}

static int udp_packet_send(u8_t finished)
{
	static int packet_id;
	int i, count, *payload;
	u8_t retries = MAX_SEND_RETRY;
	socklen_t len = sizeof(addr);

	payload = (int*) (send_buf);
	if (finished == FINISH)
		packet_id = -1;
	*payload = htonl(packet_id);

	/* always increment the id */
	packet_id++;

	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {
		while (retries) {
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
			count = lwip_sendto_blocking(sock[i], send_buf, sizeof(send_buf), 0,
					(struct sockaddr *)&addr, len);
#else
			count = lwip_sendto(sock[i], send_buf, sizeof(send_buf), 0,
								(struct sockaddr *)&addr, len);
#endif
			if (count <= 0) {
				retries--;
				usleep(ERROR_SLEEP);
			} else {
				client.total_bytes += count;
				client.cnt_datagrams++;
				client.i_report.total_bytes += count;
				break;
			}
		}

		if (!retries) {
			/* Terminate this app */
			u64_t now = sys_now();
			u64_t diff_ms = now - client.start_time;
			xil_printf("Too many udp_send() retries, ");
			xil_printf("Terminating application\n\r");
			udp_conn_report(diff_ms, UDP_DONE_CLIENT);
			xil_printf("UDP test failed\n\r");
			return -1;
		}
		retries = MAX_SEND_RETRY;

		/* For ZynqMP GEM, At high speed, packets are being
		 * received as Out of order at Iperf server running
		 * on remote host machine.
		 * To avoid this, added delay of 1us between each
		 * packets
		 */
#if !LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
#if defined (__aarch64__) && defined (XLWIP_CONFIG_INCLUDE_GEM)
		usleep(1);
#endif /* __aarch64__ */
#endif
	}
	return 0;
}

/* Transmit data on a udp session */
int transfer_data(void)
{
	if (END_TIME || REPORT_INTERVAL_TIME) {
		u64_t now = sys_now();
		if (REPORT_INTERVAL_TIME) {
			if (client.i_report.start_time) {
				u64_t diff_ms = now - client.i_report.start_time;
				if (diff_ms >= REPORT_INTERVAL_TIME) {
					udp_conn_report(diff_ms, INTER_REPORT);
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
				/* time specified is over,
				 * close the connection */
				udp_packet_send(FINISH);
				udp_conn_report(diff_ms, UDP_DONE_CLIENT);
				xil_printf("UDP test passed Successfully\n\r");
				return FINISH;
			}
		}
	}

	if (udp_packet_send(!FINISH) < 0)
		return FINISH;

	return 0;
}

void start_application(void)
{
	err_t err;
	u32_t i;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(UDP_CONN_PORT);
	addr.sin_addr.s_addr = inet_addr(UDP_SERVER_IP_ADDRESS);

	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {
		if ((sock[i] = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			xil_printf("UDP client: Error creating Socket\r\n");
			return;
		}

		err = connect(sock[i], (struct sockaddr *)&addr, sizeof(addr));
		if (err != ERR_OK) {
			xil_printf("UDP client: Error on connect: %d\r\n", err);
			close(sock[i]);
			return;
		}
	}

	/* Wait for successful connections */
	usleep(10);

	reset_stats();

	/* initialize data buffer being sent with same as used in iperf */
	for (i = 0; i < UDP_SEND_BUFSIZE; i++)
		send_buf[i] = (i % 10) + '0';

	while (!transfer_data());
}
