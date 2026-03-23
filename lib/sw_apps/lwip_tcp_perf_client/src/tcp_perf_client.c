/*
 * Copyright (C) 2018 - 2022 Xilinx, Inc.
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

/* Connection handle for a TCP Client session */

#include "tcp_perf_client.h"
#include <string.h>

static struct tcp_pcb *c_pcb;
static char send_buf[TCP_SEND_BUFSIZE];
static struct perf_stats client;
static iperf_client_test_hdr client_hdr;
static u8_t client_hdr_acked = 0;

/* Forward declarations */

/*****************************************************************************/
/**
 * This function generates and prints a performance report for the TCP
 * client session, showing the transfer interval, bytes transferred,
 * and bandwidth achieved.
 *
 * @param    diff is the time difference in milliseconds for the report
 *           interval.
 * @param    report_type indicates whether this is an interim report or
 *           final report.
 *
 * @return   None.
 *
 *****************************************************************************/
static void tcp_conn_report(u64_t diff, enum report_type report_type);
static void tcp_client_close(struct tcp_pcb *pcb);
void print_app_header(void);

/*****************************************************************************/
/**
 * This function sends the iperf2 client header to the remote server over
 * the established TCP connection. The header contains test parameters
 * such as thread count, port, buffer length, and test duration.
 *
 * @param    pcb is a pointer to the TCP protocol control block for the
 *           active connection.
 *
 * @return   Returns ERR_OK if the header was sent successfully, otherwise
 *           returns an error code if tcp_write or tcp_output failed.
 *
 *****************************************************************************/
static err_t send_iperf_client_header(struct tcp_pcb *pcb)
{
	err_t err;

	client_hdr.flags = 0;
	client_hdr.num_threads = PP_HTONL(1);
	client_hdr.remote_port = PP_HTONL(TCP_CONN_PORT);
	client_hdr.buffer_len = PP_HTONL(TCP_SEND_BUFSIZE);
	client_hdr.win_band = 0;
	/* Negative value = time-based test (unit is 10ms)
	 * Positive value = byte-based test (number of bytes to transfer) */
	client_hdr.amount = PP_HTONL((u32_t)(-(s32_t)(TCP_TIME_INTERVAL * 100)));

	/* Send the header */
	err = tcp_write(pcb, &client_hdr, sizeof(client_hdr), TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		xil_printf("TCP client: Error sending iperf header: %d\r\n", err);
		return err;
	}

	err = tcp_output(pcb);
	if (err != ERR_OK) {
		xil_printf("TCP client: Error on tcp_output for header: %d\r\n", err);
		return err;
	}

	return ERR_OK;
}

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
#if LWIP_IPV6==1
	xil_printf("TCP client connecting to %s on port %d\r\n",
			TCP_SERVER_IPV6_ADDRESS, TCP_CONN_PORT);
	xil_printf("On Host: Run $iperf -V -s -i %d -w 2M\r\n",
			INTERIM_REPORT_INTERVAL);
#else
	xil_printf("TCP client connecting to %s on port %d\r\n",
			TCP_SERVER_IP_ADDRESS, TCP_CONN_PORT);
	xil_printf("On Host: Run $iperf -s -i %d -w 2M\r\n",
			INTERIM_REPORT_INTERVAL);
#endif /* LWIP_IPV6 */
}

/*****************************************************************************/
/**
 * This function prints the TCP connection statistics including the local
 * and remote IP addresses and ports.
 *
 * @return   None.
 *
 *****************************************************************************/
static void print_tcp_conn_stats(void)
{
#if LWIP_IPv6==1
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet6_ntoa(c_pcb->local_ip),
			c_pcb->local_port);
	xil_printf("%s port %d\r\n",inet6_ntoa(c_pcb->remote_ip),
			c_pcb->remote_port);
#else
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet_ntoa(c_pcb->local_ip),
			c_pcb->local_port);
	xil_printf("%s port %d\r\n",inet_ntoa(c_pcb->remote_ip),
			c_pcb->remote_port);
#endif /* LWIP_IPV6 */

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
 * This function generates and prints a performance report for the TCP
 * client session, showing the transfer interval, bytes transferred, and
 * bandwidth achieved.
 *
 * @param    diff is the time difference in milliseconds for the report
 *           interval.
 * @param    report_type indicates whether this is an interim report or
 *           final report.
 *
 * @return   None.
 *
 *****************************************************************************/
static void tcp_conn_report(u64_t diff,
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
}

/*****************************************************************************/
/**
 * This function closes the TCP client connection by clearing callbacks
 * and releasing the protocol control block resources.
 *
 * @param    pcb is a pointer to the TCP protocol control block to close.
 *
 * @return   None.
 *
 *****************************************************************************/
static void tcp_client_close(struct tcp_pcb *pcb)
{
	err_t err;

	if (pcb != NULL) {
		tcp_sent(pcb, NULL);
		tcp_err(pcb, NULL);
		err = tcp_close(pcb);
		if (err != ERR_OK) {
			/* Free memory with abort */
			tcp_abort(pcb);
		}
	}
}

/*****************************************************************************/
/**
 * This function is the error callback invoked when the TCP connection is
 * aborted by the remote host or encounters an error. It closes the
 * connection and prints a final report.
 *
 * @param    arg is a pointer to user-supplied argument (unused).
 * @param    err is the error code indicating the reason for abort.
 *
 * @return   None.
 *
 *****************************************************************************/
static void tcp_client_err(void *arg, err_t err)
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	u64_t now = get_time_ms();
	u64_t diff_ms = now - client.start_time;

	tcp_client_close(c_pcb);
	c_pcb = NULL;
	tcp_conn_report(diff_ms, TCP_ABORTED_REMOTE);
	xil_printf("TCP connection aborted\n\r");
}

/*****************************************************************************/
/**
 * This function sends performance test traffic over the established TCP
 * connection. It writes data to the TCP send buffer and handles interim
 * and final reporting based on configured time intervals.
 *
 * @return   Returns ERR_OK on success, ERR_CONN if no connection exists,
 *           or other error codes if tcp_write or tcp_output fails.
 *
 *****************************************************************************/
static err_t tcp_send_perf_traffic(void)
{
	err_t err;
	u8_t apiflags = TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE;

	if (c_pcb == NULL) {
		return ERR_CONN;
	}

	if (!client_hdr_acked) {
		return ERR_OK;
	}

#ifdef __MICROBLAZE__
	/* Zero-copy pbufs is used to get maximum performance for Microblaze.
	 * For Zynq A9, ZynqMP A53 and R5 zero-copy pbufs does not give
	 * significant improvement hence not used. */
	apiflags = 0;
#endif

	while (tcp_sndbuf(c_pcb) > TCP_SEND_BUFSIZE) {
		err = tcp_write(c_pcb, send_buf, TCP_SEND_BUFSIZE, apiflags);
		if (err != ERR_OK) {
			xil_printf("TCP client: Error on tcp_write: %d\r\n",
					err);
			return err;
		}

		err = tcp_output(c_pcb);
		if (err != ERR_OK) {
			xil_printf("TCP client: Error on tcp_output: %d\r\n",
					err);
			return err;
		}
		client.total_bytes += TCP_SEND_BUFSIZE;
		client.i_report.total_bytes += TCP_SEND_BUFSIZE;
	}

	if (client.end_time || client.i_report.report_interval_time) {
		u64_t now = get_time_ms();
		if (client.i_report.report_interval_time) {
			if (client.i_report.start_time) {
				u64_t diff_ms = now - client.i_report.start_time;
				u64_t rtime_ms = client.i_report.report_interval_time;
				if (diff_ms >= rtime_ms) {
					tcp_conn_report(diff_ms, INTER_REPORT);
					client.i_report.start_time = 0;
					client.i_report.total_bytes = 0;
				}
			} else {
				client.i_report.start_time = now;
			}
		}

		if (client.end_time) {
			/* this session is time-limited */
			u64_t diff_ms = now - client.start_time;
			if (diff_ms >= client.end_time) {
				/* time specified is over,
				 * close the connection */
				tcp_conn_report(diff_ms, TCP_DONE_CLIENT);
				xil_printf("TCP test passed Successfully\n\r");
				tcp_client_close(c_pcb);
				c_pcb = NULL;
				return ERR_OK;
			}
		}
	}
	return ERR_OK;
}

/*****************************************************************************/
/**
 * This function is the TCP sent callback invoked when data has been
 * acknowledged by the remote host. It checks for the client header ACK
 * and triggers sending more performance traffic.
 *
 * @param    arg is a pointer to user-supplied argument (unused).
 * @param    tpcb is a pointer to the TCP protocol control block.
 * @param    len is the number of bytes acknowledged.
 *
 * @return   Returns ERR_OK on success or error code from tcp_send_perf_traffic.
 *
 *****************************************************************************/
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(tpcb);

	/* Check if this is server ACK for client header */
	if (!client_hdr_acked && len >= sizeof(iperf_client_test_hdr)) {
		client_hdr_acked = 1;
	}

	return tcp_send_perf_traffic();
}

/*****************************************************************************/
/**
 * This function is the TCP connected callback invoked when the active
 * connection to the remote server is established. It initializes the
 * client state and sends the iperf client header to begin the test.
 *
 * @param    arg is a pointer to user-supplied argument (unused).
 * @param    tpcb is a pointer to the TCP protocol control block.
 * @param    err is the connection result status.
 *
 * @return   Returns ERR_OK on success or error code if connection failed.
 *
 *****************************************************************************/
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	LWIP_UNUSED_ARG(arg);

	if (err != ERR_OK) {
		tcp_client_close(tpcb);
		xil_printf("Connection error\n\r");
		return err;
	}
	/* store state */
	c_pcb = tpcb;

	client.start_time = get_time_ms();
	client.end_time = TCP_TIME_INTERVAL * 20; /* ms */
	client.client_id++;
	client.total_bytes = 0;

	/* report interval time in ms */
	memset(&client.i_report, 0, sizeof(client.i_report));
	client.i_report.report_interval_time = INTERIM_REPORT_INTERVAL * 20;

	print_tcp_conn_stats();

	/* Send iperf client header before starting data transfer */
	client_hdr_acked = 0;
	err = send_iperf_client_header(c_pcb);
	if (err != ERR_OK) {
		xil_printf("Failed to send iperf2 client header\r\n");
		tcp_client_close(c_pcb);
		c_pcb = NULL;
		return err;
	}

	/* initiate data transfer */
	return ERR_OK;
}

/*****************************************************************************/
/**
 * This function is called from the main application loop to continue
 * sending performance test traffic on an active TCP connection.
 *
 * @return   None.
 *
 *****************************************************************************/
void transfer_data(void)
{
	if (client.client_id)
		tcp_send_perf_traffic();
}

/*****************************************************************************/
/**
 * This function initializes and starts the TCP performance client
 * application. It creates a TCP PCB, sets up callbacks, and initiates
 * a connection to the remote iperf server.
 *
 * @return   None.
 *
 *****************************************************************************/
void start_application(void)
{
	err_t err;
	struct tcp_pcb* pcb;
	ip_addr_t remote_addr;
	u32_t i;

#if LWIP_IPV6==1
	remote_addr.type = IPADDR_TYPE_V6;
	err = inet6_aton(TCP_SERVER_IPV6_ADDRESS, &remote_addr);
#else
	err = inet_aton(TCP_SERVER_IP_ADDRESS, &remote_addr);
#endif /* LWIP_IPV6 */

	if (!err) {
		xil_printf("Invalid Server IP address: %d\r\n", err);
		return;
	}

	/* Create Client PCB */
	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		xil_printf("Error in PCB creation. out of memory\r\n");
		return;
	}

	/* Initialize callbacks */
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, tcp_client_sent);
	tcp_err(pcb, tcp_client_err);

	err = tcp_connect(pcb, &remote_addr, TCP_CONN_PORT,
			tcp_client_connected);
	if (err) {
		xil_printf("Error on tcp_connect: %d\r\n", err);
		tcp_client_close(pcb);
		return;
	}
	client.client_id = 0;

	/* initialize data buffer being sent with same as used in iperf */
	for (i = 0; i < TCP_SEND_BUFSIZE; i++)
		send_buf[i] = (i % 10) + '0';

	return;
}
