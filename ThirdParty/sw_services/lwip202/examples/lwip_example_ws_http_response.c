/******************************************************************************
*
* Copyright (C) 2009 - 2017 Xilinx, Inc.  All rights reserved.
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

#include <string.h>

#include "lwip_example_webserver.h"
#include "xil_printf.h"

char *notfound_header =
	"<html> \
	<head> \
		<title>404</title> \
		<style type=\"text/css\"> \
		div#request {background: #eeeeee} \
		</style> \
	</head> \
	<body> \
	<h1>404 Page Not Found</h1> \
	<div id=\"request\">";

char *notfound_footer =
	"</div> \
	</body> \
	</html>";

/* dynamically generate 404 response:
 * this inserts the original request string in betwween the notfound_header &
 * footer strings
 */
int do_404(struct tcp_pcb *pcb, char *req, int rlen)
{
	int len, hlen;
	int BUFSIZE = 1024;
	char buf[BUFSIZE];
	err_t err;

	len = strlen(notfound_header) + strlen(notfound_footer) + rlen;

	hlen = generate_http_header(buf, "html", len);

	if (tcp_sndbuf(pcb) < hlen) {
		xil_printf("cannot send 404 message, tcp_sndbuf = %d bytes, message length = %d bytes\r\n",
				tcp_sndbuf(pcb), hlen);
		http_close(pcb);
		return -1;
	}

	err = tcp_write(pcb, buf, hlen, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		xil_printf("error (%d) writing 404 http header\r\n", err);
		http_close(pcb);
		return -1;
	}

	err = tcp_write(pcb, notfound_header, strlen(notfound_header),
			TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		xil_printf("error (%d) writing not found header\r\n", err);
		http_close(pcb);
		return -1;
	}

	err = tcp_write(pcb, req, rlen, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		xil_printf("error (%d) writing org req\r\n", err);
		http_close(pcb);
		return -1;
	}

	err = tcp_write(pcb, notfound_footer, strlen(notfound_footer),
			TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		xil_printf("error (%d) writing not found footer\r\n", err);
		http_close(pcb);
		return -1;
	}

	return 0;
}

int do_http_post(struct tcp_pcb *pcb, char *req, int rlen)
{
	int BUFSIZE = 1024;
	int len, n, err;
	char buf[BUFSIZE];

	if (is_cmd_print(req)) {
		/* HTTP data starts after "\r\n\r\n" sequence */
		char *data = strstr(req, "\r\n\r\n") + 4;

		/* calculate number of bytes to be printed */
		len = rlen - (data - req);

		xil_printf("http POST: print\r\n");
		xil_printf("-------------------------------------\r\n");
		/* as buffer isn't null terminated, printf %s won't work */
		for (n = 0; n < len; n++)
			xil_printf("%c", data[n]);
		xil_printf("\r\n-------------------------------------\r\n\r\n");

		len = generate_http_header(buf, "js", 1);
		buf[len++] = '0'; /* single byte payload - '0' - to ack */
		buf[len++] = 0;
	} else {
		xil_printf("http POST: unsupported command\r\n");
		return -1;
	}

	err = tcp_write(pcb, buf, len, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		xil_printf("error (%d) writing http POST response\r\n", err);
		xil_printf("http header = %s\r\n", buf);
		http_close(pcb);
		return -1;
	}

	return 0;
}

/* respond for a file GET request */
int do_http_get(struct tcp_pcb *pcb, char *req, int rlen)
{
	int BUFSIZE = 1400;
	char filename[MAX_FILENAME];
	char buf[BUFSIZE];
	int fsize, hlen, n;
	char *fext;
	err_t err;
	FIL fil;
	FRESULT Res;

	/* determine file name */
	extract_file_name(filename, req, rlen, MAX_FILENAME);

	/* respond with 404 if not present */
	Res = f_open(&fil, filename, FA_READ);
	if (Res) {
		xil_printf("requested file %s not found, returning 404\r\n",
			   filename);
		do_404(pcb, req, rlen);
		return -1;
	}

	/* respond with correct file */

	/* debug statement on UART */
	xil_printf("http GET: %s\r\n", filename);

	/* get a pointer to file extension */
	fext = get_file_extension(filename);

	/* obtain file size */
	fsize = fil.fsize;

	/* write the http headers */
	hlen = generate_http_header(buf, fext, fsize);
	err = tcp_write(pcb, buf, hlen, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
	if (err != ERR_OK) {
		xil_printf("error (%d) writing http header to socket\r\n", err);
		xil_printf("attempted to write #bytes = %d, tcp_sndbuf = %d\r\n", hlen, tcp_sndbuf(pcb));
		xil_printf("http header = %s\r\n", buf);
		f_close(&fil);
		http_close(pcb);
		return -1;
	}

	/* now write the file */
	while (fsize > 0) {
		int sndbuf = tcp_sndbuf(pcb);
		if (sndbuf < BUFSIZE) {
			/* not enough space in sndbuf, so send remaining bytes when there is space
			 * this is done by storing the fd in as part of the tcp_arg, so that the sent
			 * callback handler knows to send data */
			http_arg *a = (http_arg *)pcb->callback_arg;
			memcpy(&a->fil, &fil, sizeof(fil));
			a->fsize = fsize;
			return -1;
		}

		f_read(&fil, (void *)buf, BUFSIZE, (unsigned int *)&n);

		err = tcp_write(pcb, buf, n, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
		if (err != ERR_OK) {
			xil_printf("error writing file (%s) to socket, remaining unwritten bytes = %d\r\n",
					filename, fsize - n);
			xil_printf("attempted to lwip_write %d bytes, tcp write error = %d\r\n", n, err);
			http_close(pcb);
			break;
		}

		fsize -= n;
	}

	f_close(&fil);
	return 0;
}

enum http_req_type { HTTP_GET, HTTP_POST, HTTP_UNKNOWN };
enum http_req_type decode_http_request(char *req, int l)
{
	char *get_str = "GET";
	char *post_str = "POST";

	if (!strncmp(req, get_str, strlen(get_str)))
		return HTTP_GET;

	if (!strncmp(req, post_str, strlen(post_str)))
		return HTTP_POST;

	return HTTP_UNKNOWN;
}

void dump_payload(char *p, int len)
{
	int i, j;

	for (i = 0; i < len; i += 16) {
		for (j = 0; j < 16; j++)
			xil_printf("%c ", p[i+j]);
		xil_printf("\r\n");
	}
	xil_printf("total len = %d\r\n", len);
}

/* generate and write out an appropriate response for the http request */
/* this assumes that tcp_sndbuf is high enough to send atleast 1 packet */
int generate_response(struct tcp_pcb *pcb, char *http_req, int http_req_len)
{
	enum http_req_type request_type = decode_http_request(http_req, http_req_len);

	switch (request_type) {
	case HTTP_GET:
		return do_http_get(pcb, http_req, http_req_len);
	case HTTP_POST:
		return do_http_post(pcb, http_req, http_req_len);
	default:
		xil_printf("request_type != GET|POST\r\n");
		dump_payload(http_req, http_req_len);
		return do_404(pcb, http_req, http_req_len);
	}
}
