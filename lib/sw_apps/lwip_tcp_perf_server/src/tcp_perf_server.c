/*
 * Copyright (C) 2018 - 2019 Xilinx, Inc.
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

/** Connection handle for a TCP Server session */

#include "tcp_perf_server.h"

extern struct netif server_netif;
static struct tcp_pcb *c_pcb;
static struct perf_stats server;

void print_app_header(void)
{
	xil_printf("TCP server listening on port %d\r\n",
			TCP_CONN_PORT);
#if LWIP_IPV6==1
	xil_printf("On Host: Run $iperf -V -c %s%%<interface> -i %d -t 300 -w 2M\r\n",
			inet6_ntoa(server_netif.ip6_addr[0]),
			INTERIM_REPORT_INTERVAL);
#else
	xil_printf("On Host: Run $iperf -c %s -i %d -t 300 -w 2M\r\n",
			inet_ntoa(server_netif.ip_addr),
			INTERIM_REPORT_INTERVAL);
#endif /* LWIP_IPV6 */
}

static void print_tcp_conn_stats(void)
{
#if LWIP_IPV6==1
	xil_printf("[%3d] local %s port %d connected with ",
			server.client_id, inet6_ntoa(c_pcb->local_ip),
			c_pcb->local_port);
	xil_printf("%s port %d\r\n",inet6_ntoa(c_pcb->remote_ip),
			c_pcb->remote_port);
#else
	xil_printf("[%3d] local %s port %d connected with ",
			server.client_id, inet_ntoa(c_pcb->local_ip),
			c_pcb->local_port);
	xil_printf("%s port %d\r\n",inet_ntoa(c_pcb->remote_ip),
			c_pcb->remote_port);
#endif /* LWIP_IPV6 */

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


/** The report function of a TCP server session */
static void tcp_conn_report(u64_t diff,
		enum report_type report_type)
{
	u64_t total_len;
	double duration, bandwidth = 0;
	char data[16], perf[16], time[64];

	if (report_type == INTER_REPORT) {
		total_len = server.i_report.total_bytes;
	} else {
		server.i_report.last_report_time = 0;
		total_len = server.total_bytes;
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
			(double)server.i_report.last_report_time,
			(double)(server.i_report.last_report_time + duration));
	xil_printf("[%3d] %s  %sBytes  %sbits/sec\n\r", server.client_id,
			time, data, perf);

	if (report_type == INTER_REPORT)
		server.i_report.last_report_time += duration;
}

/** Close a tcp session */
static void tcp_server_close(struct tcp_pcb *pcb)
{
	err_t err;

	if (pcb != NULL) {
		tcp_recv(pcb, NULL);
		tcp_err(pcb, NULL);
		err = tcp_close(pcb);
		if (err != ERR_OK) {
			/* Free memory with abort */
			tcp_abort(pcb);
		}
	}
}

/** Error callback, tcp session aborted */
static void tcp_server_err(void *arg, err_t err)
{
	LWIP_UNUSED_ARG(err);
	u64_t now = get_time_ms();
	u64_t diff_ms = now - server.start_time;
	tcp_server_close(c_pcb);
	c_pcb = NULL;
	tcp_conn_report(diff_ms, TCP_ABORTED_REMOTE);
	xil_printf("TCP connection aborted\n\r");
}


/** Receive data on a tcp session */
static err_t tcp_recv_perf_traffic(void *arg, struct tcp_pcb *tpcb,
		struct pbuf *p, err_t err)
{
	if (p == NULL) {
		u64_t now = get_time_ms();
		u64_t diff_ms = now - server.start_time;
		tcp_server_close(tpcb);
		tcp_conn_report(diff_ms, TCP_DONE_SERVER);
		xil_printf("TCP test passed Successfully\n\r");
		return ERR_OK;
	}

	/* Record total bytes for final report */
	server.total_bytes += p->tot_len;

	if (server.i_report.report_interval_time) {
		u64_t now = get_time_ms();
		/* Record total bytes for interim report */
		server.i_report.total_bytes += p->tot_len;
		if (server.i_report.start_time) {
			u64_t diff_ms = now - server.i_report.start_time;

			if (diff_ms >= server.i_report.report_interval_time) {
				tcp_conn_report(diff_ms, INTER_REPORT);
				/* Reset Interim report counters */
				server.i_report.start_time = 0;
				server.i_report.total_bytes = 0;
			}
		} else {
			/* Save start time for interim report */
			server.i_report.start_time = now;
		}
	}

	tcp_recved(tpcb, p->tot_len);

	pbuf_free(p);
	return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	if ((err != ERR_OK) || (newpcb == NULL)) {
		return ERR_VAL;
	}
	/* Save connected client PCB */
	c_pcb = newpcb;

	/* Save start time for final report */
	server.start_time = get_time_ms();
	server.end_time = 0; /* ms */
	/* Update connected client ID */
	server.client_id++;
	server.total_bytes = 0;

	/* Initialize Interim report parameters */
	server.i_report.report_interval_time =
		INTERIM_REPORT_INTERVAL * 1000; /* ms */
	server.i_report.last_report_time = 0;
	server.i_report.start_time = 0;
	server.i_report.total_bytes = 0;

	print_tcp_conn_stats();

	/* setup callbacks for tcp rx connection */
	tcp_arg(c_pcb, NULL);
	tcp_recv(c_pcb, tcp_recv_perf_traffic);
	tcp_err(c_pcb, tcp_server_err);

	return ERR_OK;
}

void start_application(void)
{
	err_t err;
	struct tcp_pcb *pcb, *lpcb;

	/* Create Server PCB */
	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		xil_printf("TCP server: Error creating PCB. Out of Memory\r\n");
		return;
	}

	err = tcp_bind(pcb, IP_ADDR_ANY, TCP_CONN_PORT);
	if (err != ERR_OK) {
		xil_printf("TCP server: Unable to bind to port %d: "
				"err = %d\r\n" , TCP_CONN_PORT, err);
		tcp_close(pcb);
		return;
	}

	/* Set connection queue limit to 1 to serve
	 * one client at a time
	 */
	lpcb = tcp_listen_with_backlog(pcb, 1);
	if (!lpcb) {
		xil_printf("TCP server: Out of memory while tcp_listen\r\n");
		tcp_close(pcb);
		return;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(lpcb, NULL);

	/* specify callback to use for incoming connections */
	tcp_accept(lpcb, tcp_server_accept);

	return;
}
