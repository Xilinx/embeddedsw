/*
 * Copyright (C) 2017 - 2021 Xilinx, Inc.
 * Copyright (C) 2021 - 2024 Advanced Micro Devices, Inc.
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
static struct udp_pcb *pcb[NUM_OF_PARALLEL_CLIENTS];
static struct perf_stats client;
static char send_buf[UDP_SEND_BUFSIZE];
#define FINISH	1
/* Report interval time in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 20)
/* End time in ms */
#define END_TIME (UDP_TIME_INTERVAL * 20)

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
			pcb[0]->local_port);
	xil_printf("%s port %d\r\n",inet_ntoa(pcb[0]->remote_ip),
			pcb[0]->remote_port);
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
	duration = diff / 20.0; /* secs */
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
	client.start_time = get_time_ms();
	client.total_bytes = 0;
	client.cnt_datagrams = 0;

	/* Initialize Interim report parameters */
	client.i_report.start_time = 0;
	client.i_report.total_bytes = 0;
	client.i_report.last_report_time = 0;
}

static void udp_packet_send(u8_t finished)
{
	int *payload;
	static int packet_id;
	u8_t i;
	u8_t retries = MAX_SEND_RETRY;
	struct pbuf *packet;
	err_t err;

	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {

		packet = pbuf_alloc(PBUF_TRANSPORT, UDP_SEND_BUFSIZE, PBUF_POOL);
		if (!packet) {
			xil_printf("error allocating pbuf to send\r\n");
			return;
		} else {
			pbuf_take(packet, send_buf, UDP_SEND_BUFSIZE);
		}

		/* always increment the id */
		payload = (int*) (packet->payload);
		if (finished == FINISH)
			packet_id = -1;

		payload[0] = htonl(packet_id);

		while (retries) {
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
			err = udp_send_blocking(pcb[i], packet);
#else
			err = udp_send(pcb[i], packet);
#endif
			if (err != ERR_OK) {
				xil_printf("Error on udp_send: %d\r\n", err);
				retries--;
				usleep(100);
			} else {
				client.total_bytes += UDP_SEND_BUFSIZE;
				client.cnt_datagrams++;
				client.i_report.total_bytes += UDP_SEND_BUFSIZE;
				break;
			}
		}
		if (!retries) {
			/* Terminate this app */
			u64_t now = get_time_ms();
			u64_t diff_ms = now - client.start_time;
			xil_printf("Too many udp_send() retries, ");
			xil_printf("Terminating application\n\r");
			udp_conn_report(diff_ms, UDP_DONE_CLIENT);
			xil_printf("UDP test failed\n\r");
			udp_remove(pcb[i]);
			pcb[i] = NULL;
		}
		if (finished == FINISH)
			pcb[i] = NULL;

		pbuf_free(packet);

		/* For ZynqMP SGMII, At high speed,
		 * "pack dropped, no space" issue observed.
		 * To avoid this, added delay of 2us between each
		 * packets.
		 */
#if !LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
#if defined (__aarch64__) && defined (XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_DMA)
		usleep(2);
#endif /* __aarch64__ */
#endif
	}
	packet_id++;
}

/** Transmit data on a udp session */
void transfer_data(void)
{
	int i = 0;
	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {
		if (pcb[i] == NULL)
			return;
	}

	if (END_TIME || REPORT_INTERVAL_TIME) {
		u64_t now = get_time_ms();
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
				return;
			}
		}
	}

	udp_packet_send(!FINISH);
}

void start_application(void)
{
	err_t err;
	ip_addr_t remote_addr;
	u32_t i;

	err = inet_aton(UDP_SERVER_IP_ADDRESS, &remote_addr);
	if (!err) {
		xil_printf("Invalid Server IP address: %d\r\n", err);
		return;
	}

	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {
		/* Create Client PCB */
		pcb[i] = udp_new();
		if (!pcb[i]) {
			xil_printf("Error in PCB creation. out of memory\r\n");
			return;
		}

		err = udp_connect(pcb[i], &remote_addr, UDP_CONN_PORT);
		if (err != ERR_OK) {
			xil_printf("udp_client: Error on udp_connect: %d\r\n", err);
			udp_remove(pcb[i]);
			return;
		}
	}
	/* Wait for successful connection */
	usleep(10);
	reset_stats();

	/* initialize data buffer being sent with same as used in iperf */
	for (i = 0; i < UDP_SEND_BUFSIZE; i++)
		send_buf[i] = (i % 10) + '0';
}
