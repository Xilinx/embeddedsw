/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "open_amp.h"
/* RPC response buffer size */
#define RPC_BUFF_SIZE 512

/* System call definitions */
#define OPEN_SYSCALL_ID     1
#define CLOSE_SYSCALL_ID    2
#define WRITE_SYSCALL_ID    3
#define READ_SYSCALL_ID     4
#define ACK_STATUS_ID       5
#define TERM_SYSCALL_ID     6


#define FILE_NAME_LEN       50

/* Proxy device endpoint ID */
#define PROXY_ENDPOINT      127

typedef void (*rpc_shutdown_cb)(struct rpmsg_channel *);

struct _rpc_data
{
	struct rpmsg_channel* rpmsg_chnl;
	struct rpmsg_endpoint* rp_ept;
	void* rpc_lock;
	void* sync_lock;
	struct _sys_rpc* rpc;
	struct _sys_rpc* rpc_response;
	rpc_shutdown_cb shutdown_cb;
};

struct _sys_call_args
{
	int int_field1;
	int int_field2;
	unsigned int   data_len;
	char data[0];
};

/* System call rpc data structure */
struct _sys_rpc
{
	unsigned int   id;
	struct _sys_call_args   sys_call_args;
};


void debug_print(char* str, int len);

/* API prototypes */
int rpmsg_retarget_init(struct rpmsg_channel *rp_chnl, rpc_shutdown_cb cb);
int rpmsg_retarget_deinit(struct rpmsg_channel *rp_chnl);
