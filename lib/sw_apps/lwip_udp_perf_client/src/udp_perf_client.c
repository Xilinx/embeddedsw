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

/* Connection handle for a UDP Client session */

#include "udp_perf_client.h"
#include <string.h>

extern struct netif server_netif;
static struct udp_pcb *pcb[NUM_OF_PARALLEL_CLIENTS];
static struct perf_stats client;
static char send_buf[UDP_SEND_BUFSIZE];
static int udp_packet_id[NUM_OF_PARALLEL_CLIENTS];
static u8_t udp_client_hdr_sent[NUM_OF_PARALLEL_CLIENTS];

#define FINISH	1
/* Report interval time in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 20)
/* End time in ms */
#define END_TIME (UDP_TIME_INTERVAL * 20)

/*****************************************************************************/
/**
 * This function prints the application header showing the server IP address
 * and port the client will connect to, along with the iperf command to run
 * on the host.
 *
 * @return   None.
 *
 *****************************************************************************/
void print_app_header(void)
{
	xil_printf("UDP client connecting to %s on port %d\r\n",
			UDP_SERVER_IP_ADDRESS, UDP_CONN_PORT);
	xil_printf("On Host: Run $iperf -s -i %d -u\r\n\r\n",
			INTERIM_REPORT_INTERVAL);
}

/*****************************************************************************/
/**
 * This function prints the UDP connection statistics including the local
 * and remote IP addresses and ports.
 *
 * @return   None.
 *
 *****************************************************************************/
static void print_udp_conn_stats(void)
{
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet_ntoa(server_netif.ip_addr),
			pcb[0]->local_port);
	xil_printf("%s port %d\r\n",inet_ntoa(pcb[0]->remote_ip),
			pcb[0]->remote_port);
	xil_printf("[ ID] Interval\t\tTransfer   Bandwidth\n\r");
}

/*****************************************************************************/
/**
 * This function formats a data value into a human-readable string with
 * appropriate unit prefix (K, M, G) for display in performance reports.
 *
 * @param    outString is a pointer to the output buffer for the formatted
 *           string.
 * @param    data is the value to format.
 * @param    type indicates whether the value represents BYTES or SPEED.
 *
 * @return   None.
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * This function generates and prints a performance report for the UDP
 * client session, showing the transfer interval, bytes transferred,
 * bandwidth achieved, and datagram count.
 *
 * @param    diff is the time difference in milliseconds for the report
 *           interval.
 * @param    report_type indicates whether this is an interim report or
 *           final report.
 *
 * @return   None.
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * This function resets the client statistics and initializes the state
 * for a new UDP performance test session. It sets up packet IDs and
 * header sent flags for all parallel client streams.
 *
 * @return   None.
 *
 *****************************************************************************/
static void reset_stats(void)
{
	u16_t i;
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
	/* Initialize packet IDs to 2 (packet 1 is iperf header)
	 * and reset header sent flags */
	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {
		udp_packet_id[i] = 2;
		udp_client_hdr_sent[i] = 0;
	}
}

/*****************************************************************************/
/**
 * This function sends the iperf2 client header to the remote server over
 * the established UDP connection. The header contains test parameters
 * such as thread count, port, buffer length, and test duration.
 *
 * @param    stream_idx is the index of the parallel client stream.
 *
 * @return   Returns ERR_OK if the header was sent successfully, otherwise
 *           returns an error code if pbuf allocation or udp_send failed.
 *
 *****************************************************************************/
static err_t send_iperf_udp_client_header(u8_t stream_idx)
{
	err_t err;
	struct pbuf *packet;
	udp_client_test_hdr *hdr;
	char tx_buf[UDP_SEND_BUFSIZE];

	memcpy(tx_buf, send_buf, UDP_SEND_BUFSIZE);
	hdr = (udp_client_test_hdr *)tx_buf;

	/* UDP datagram header */
	hdr->id = PP_HTONL(1);
	hdr->tv_sec = 0;
	hdr->tv_usec = 0;
	hdr->id2 = 0;

	/* iperf2 client header */
	hdr->flags = 0;
	hdr->num_threads = PP_HTONL(NUM_OF_PARALLEL_CLIENTS);
	hdr->remote_port = PP_HTONL(UDP_CONN_PORT);
	hdr->buffer_len = PP_HTONL(UDP_SEND_BUFSIZE);
	hdr->win_band = 0;
	/* Negative value = time-based test (unit is 10ms)
	 * Positive value = byte-based test (number of bytes to transfer) */
	hdr->amount = PP_HTONL((u32_t)(-(s32_t)(UDP_TIME_INTERVAL * 100)));

	packet = pbuf_alloc(PBUF_TRANSPORT, UDP_SEND_BUFSIZE, PBUF_POOL);
	if (!packet) {
		xil_printf("UDP client: error allocating pbuf for iperf header\r\n");
		return ERR_MEM;
	}
	pbuf_take(packet, tx_buf, UDP_SEND_BUFSIZE);
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
	err = udp_send_blocking(pcb[stream_idx], packet);
#else
	err = udp_send(pcb[stream_idx], packet);
#endif
	pbuf_free(packet);
	if (err == ERR_OK) {
		client.total_bytes += UDP_SEND_BUFSIZE;
		client.cnt_datagrams++;
		client.i_report.total_bytes += UDP_SEND_BUFSIZE;
	}
	return err;
}

/*****************************************************************************/
/**
 * This function sends UDP packets for the performance test. It handles
 * sending the iperf header on the first packet and then sends data
 * packets with incrementing sequence IDs. It also handles retry logic
 * and error reporting.
 *
 * @param    finished indicates whether this is the final packet (FINISH)
 *           or a regular data packet.
 *
 * @return   None.
 *
 *****************************************************************************/
static void udp_packet_send(u8_t finished)
{
	int *payload;
	u8_t i;
	u8_t retries;
	struct pbuf *packet;
	err_t err;
	for (i = 0; i < NUM_OF_PARALLEL_CLIENTS; i++) {
		retries = MAX_SEND_RETRY;
		if (!finished && !udp_client_hdr_sent[i]) {
			while (retries) {
				err = send_iperf_udp_client_header(i);
				if (err != ERR_OK) {
					xil_printf("UDP client[%d]: Error sending iperf header: %d\r\n", i, err);
					retries--;
					usleep(SEND_RETRY_DELAY_US);
				} else {
					udp_client_hdr_sent[i] = 1;
					usleep(IPERF_HDR_SEND_DELAY_US);
					break;
				}
			}
			if (!retries) {
				u64_t now = get_time_ms();
				u64_t diff_ms = now - client.start_time;
				xil_printf("Too many retries sending udp iperf header, ");
				xil_printf("Terminating application\n\r");
				udp_conn_report(diff_ms, UDP_DONE_CLIENT);
				xil_printf("UDP test failed\n\r");
				udp_remove(pcb[i]);
				pcb[i] = NULL;
			}
			continue;
		}
		packet = pbuf_alloc(PBUF_TRANSPORT, UDP_SEND_BUFSIZE, PBUF_POOL);
		if (!packet) {
			xil_printf("error allocating pbuf to send\r\n");
			return;
		}
		pbuf_take(packet, send_buf, UDP_SEND_BUFSIZE);
		/* always increment the id */
		payload = (int*) (packet->payload);
		if (finished == FINISH)
			udp_packet_id[i] = -1;

		payload[0] = htonl(udp_packet_id[i]);
		while (retries) {
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
			err = udp_send_blocking(pcb[i], packet);
#else
			err = udp_send(pcb[i], packet);
#endif
			if (err != ERR_OK) {
				xil_printf("Error on udp_send: %d\r\n", err);
				retries--;
				usleep(SEND_RETRY_DELAY_US);
			} else {
				client.total_bytes += UDP_SEND_BUFSIZE;
				client.cnt_datagrams++;
				client.i_report.total_bytes += UDP_SEND_BUFSIZE;
				udp_packet_id[i]++;
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
}

/*****************************************************************************/
/**
 * This function is called from the main application loop to continue
 * sending performance test traffic on active UDP connections. It handles
 * interim reporting and test termination based on configured time.
 *
 * @return   None.
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * This function initializes and starts the UDP performance client
 * application. It creates UDP PCBs for parallel client streams, connects
 * to the remote iperf server, and initializes the data buffer.
 *
 * @return   None.
 *
 *****************************************************************************/
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
