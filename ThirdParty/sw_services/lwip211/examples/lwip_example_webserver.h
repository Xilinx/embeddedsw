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

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "lwip/tcp.h"
#include "ff.h"

#define MAX_FILENAME 256
#define HTTP_PORT    80

/* initialize file system layer */
int platform_init_fs();

/* web_utils.c utilities */
void extract_file_name(char *filename, char *req, int rlen, int maxlen);

char *get_file_extension(char *buf);
int is_cmd_print(char *buf);

int generate_response(struct tcp_pcb *pcb, char *http_req, int http_req_len);
int generate_http_header(char *buf, char *fext, int fsize);

typedef struct {
	int count;
	FIL fil;
	int fsize;
} http_arg;

http_arg *palloc_arg();
void pfree_arg(http_arg *);

/* clear callback functions and close connection */
#define http_close(tpcb) { \
	tcp_recv(tpcb, NULL); \
	tcp_sent(tpcb, NULL); \
	tcp_close(tpcb); \
}

#endif
