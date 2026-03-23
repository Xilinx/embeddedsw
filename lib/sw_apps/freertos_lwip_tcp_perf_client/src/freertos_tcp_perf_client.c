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

#include "freertos_tcp_perf_client.h"

static char send_buf[TCP_SEND_BUFSIZE];
static struct perf_stats client;
static iperf_client_test_hdr client_hdr;

/* Report interval time in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)
/* End time in ms */
#define END_TIME (TCP_TIME_INTERVAL * 1000)

/*****************************************************************************/
/**
 * This function sends the iperf2 client header to the remote server over
 * the established TCP connection. The header contains test parameters
 * such as thread count, port, buffer length, and test duration.
 *
 * @param    sock is the socket descriptor for the active connection.
 *
 * @return   Returns 0 if the header was sent successfully, otherwise
 *           returns -1 if lwip_send failed.
 *
 *****************************************************************************/
static int send_client_header(int sock)
{
	int bytes_sent;

	client_hdr.flags = 0;
	client_hdr.num_threads = PP_HTONL(1);
	client_hdr.remote_port = PP_HTONL(TCP_CONN_PORT);
	client_hdr.buffer_len = PP_HTONL(TCP_SEND_BUFSIZE);
	client_hdr.win_band = 0;
	/* Negative value = time-based test (unit is 10ms)
	 * Positive value = byte-based test (number of bytes to transfer) */
	client_hdr.amount = PP_HTONL((u32_t)(-(s32_t)(TCP_TIME_INTERVAL * 100)));

	bytes_sent = lwip_send(sock, &client_hdr, sizeof(client_hdr), 0);
	if (bytes_sent < 0) {
		xil_printf("TCP Client: Error sending client header\r\n");
		return -1;
	}
	return 0;
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
void print_app_header()
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
 * and remote IP addresses and ports for the performance test.
 *
 * @param    sock is the socket descriptor for the active connection.
 *
 * @return   None.
 *
 *****************************************************************************/
void print_tcp_txperf_header(int sock)
{
	int size;
#if LWIP_IPV6==1
	struct sockaddr_in6 local, remote;
#else
	struct sockaddr_in local, remote;
#endif /* LWIP_IPV6 */
	size = sizeof(local);
	getsockname(sock, (struct sockaddr *) &local, (socklen_t *)&size);
	getpeername(sock, (struct sockaddr *) &remote, (socklen_t *) &size);
#if LWIP_IPV6==1
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet6_ntoa(local.sin6_addr),
			ntohs(local.sin6_port));
	xil_printf("%s port %d\r\n", inet6_ntoa(remote.sin6_addr),
			ntohs(local.sin6_port));
#else
	xil_printf("[%3d] local %s port %d connected with ",
			client.client_id, inet_ntoa(local.sin_addr),
			ntohs(local.sin_port));
	xil_printf("%s port %d\r\n", inet_ntoa(remote.sin_addr),
			ntohs(local.sin_port));
#endif /* LWIP_IPV6 */
	xil_printf("[ ID] Interval    Transfer     Bandwidth\n\r");

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

/*****************************************************************************/
/**
 * This function sends TCP performance test traffic over the established
 * connection. It continuously sends data and handles interim and final
 * reporting based on configured time intervals.
 *
 * @param    sock is the socket descriptor for the active connection.
 *
 * @return   Returns ERR_OK when the test completes.
 *
 *****************************************************************************/
int tcp_send_perf_traffic(int sock)
{
	int bytes_send;
	u8_t apiflags = MSG_MORE;

	client.start_time = sys_now();
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
			u64_t now = sys_now();
			u64_t diff_ms = now - client.start_time;
			tcp_conn_report(diff_ms, TCP_ABORTED_REMOTE);
			break;
		}

		client.total_bytes += bytes_send;

		if (END_TIME || REPORT_INTERVAL_TIME) {
			u64_t now = sys_now();
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

/*****************************************************************************/
/**
 * This function initializes and starts the TCP performance client
 * application. It creates a socket, connects to the remote iperf server,
 * sends the client header, and starts the performance test.
 *
 * @return   None.
 *
 *****************************************************************************/
void start_application(void)
{
	int sock, i;
#if LWIP_IPV6==1
	struct sockaddr_in6 address;
#else
	struct sockaddr_in address;
#endif /* LWIP_IPV6 */

	/* set up address to connect to */
        memset(&address, 0, sizeof(address));

#if LWIP_IPV6==1
	if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
	        xil_printf("TCP Client: Error in creating Socket\r\n");
	        return;
	}
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(TCP_CONN_PORT);
	address.sin6_len = sizeof(address);
	inet6_aton(TCP_SERVER_IPV6_ADDRESS, &address.sin6_addr.s6_addr);
#else
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xil_printf("TCP Client: Error in creating Socket\r\n");
		return;
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(TCP_CONN_PORT);
	address.sin_len = sizeof(address);
	if (inet_aton(TCP_SERVER_IP_ADDRESS, &address.sin_addr) == 0) {
		xil_printf("TCP Client: Invalid server IP address\r\n");
		close(sock);
		return;
	}
#endif /* LWIP_IPV6 */
	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
		xil_printf("TCP Client: Error on tcp_connect\r\n");
		close(sock);
		return;
	}

	/* Send client header to server*/
	if (send_client_header(sock) < 0) {
		xil_printf("ERROR: send_client_header failed, closing socket %d\r\n", sock);
		close(sock);
		return;
	}

	/* initialize data buffer being sent */
	for (i = 0; i < TCP_SEND_BUFSIZE; i++)
		send_buf[i] = (i % 10) + '0';

	tcp_send_perf_traffic(sock);
}
