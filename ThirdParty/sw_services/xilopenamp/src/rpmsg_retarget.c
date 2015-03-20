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
#include "rpmsg_retarget.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "sleep.h"



/*************************************************************************
 *	Description
 *	This files contains rpmsg based redefinitions for C RTL system calls
 *	such as _open, _read, _write, _close.
 *************************************************************************/
static struct _rpc_data* rpc_data;
static unsigned int rpc_data_synclock = 0;
int get_response = 0;

int send_rpc(void *data, int len);
static int rpc_count=0;

void rpc_cb(struct rpmsg_channel *rtl_rp_chnl, void *data, int len, void * priv,
			unsigned long src) {
	memcpy(rpc_data->rpc_response, data, len);
	env_release_sync_lock(rpc_data->sync_lock);
	get_response=1;


	if (rpc_data->rpc_response->id == TERM_SYSCALL_ID) {
		/* Application terminate signal is received from the proxy app,
		 * so let the application know of terminate message.
		 */
		rpc_data->shutdown_cb(rtl_rp_chnl);
	}
}

int send_rpc(void *data, int len) {
	int retval;

	retval = rpmsg_sendto(rpc_data->rpmsg_chnl, data, len, PROXY_ENDPOINT);
	return retval;
}



int rpmsg_retarget_init(struct rpmsg_channel *rp_chnl, rpc_shutdown_cb cb) {
	int status;

	/* Allocate memory for rpc control block */
	rpc_data = (struct _rpc_data*) env_allocate_memory(
				sizeof(struct _rpc_data));

	/* Create a mutex for synchronization */
	status = env_create_mutex(&rpc_data->rpc_lock, 1);

	/* Create a mutex for synchronization */
	status = env_create_sync_lock(&rpc_data->sync_lock, LOCKED);

	/* Create a endpoint to handle rpc response from master */
	rpc_data->rpmsg_chnl = rp_chnl;
	rpc_data->rp_ept = rpmsg_create_ept(rpc_data->rpmsg_chnl, rpc_cb,
						RPMSG_NULL, PROXY_ENDPOINT);
	rpc_data->rpc = env_allocate_memory(RPC_BUFF_SIZE);
	rpc_data->rpc_response = env_allocate_memory(RPC_BUFF_SIZE);
	rpc_data->shutdown_cb = cb;

	return status;
}

int rpmsg_retarget_deinit(struct rpmsg_channel *rp_chnl) {
	env_free_memory(rpc_data->rpc);
	env_free_memory(rpc_data->rpc_response);
	env_delete_mutex(rpc_data->rpc_lock);
	env_delete_sync_lock(rpc_data->sync_lock);
	rpmsg_destroy_ept(rpc_data->rp_ept);
	env_free_memory(rpc_data);

	return 0;
}

/*************************************************************************
 *
 *   FUNCTION
 *
 *       _open
 *
 *   DESCRIPTION
 *
 *       Open a file.  Minimal implementation
 *
 *************************************************************************/
int _open(const char * filename, int flags, int mode) {
	int filename_len = strlen(filename) + 1;
	int payload_size = sizeof(struct _sys_rpc) + filename_len;
	int retval = -1;

	if ((!filename) || (filename_len > FILE_NAME_LEN)) {
		return -1;
	}

	/* Construct rpc payload */
	rpc_data->rpc->id = OPEN_SYSCALL_ID;
	rpc_data->rpc->sys_call_args.int_field1 = flags;
	rpc_data->rpc->sys_call_args.int_field2 = mode;
	rpc_data->rpc->sys_call_args.data_len = filename_len;
	memcpy(&rpc_data->rpc->sys_call_args.data, filename, filename_len);

	/* Transmit rpc request */
	env_lock_mutex(rpc_data->rpc_lock);
	send_rpc((void*) rpc_data->rpc, payload_size);
	env_unlock_mutex(rpc_data->rpc_lock);

	/* Wait for response from proxy on master */
	env_acquire_sync_lock(rpc_data->sync_lock);

	/* Obtain return args and return to caller */
	if (rpc_data->rpc_response->id == OPEN_SYSCALL_ID) {
		retval = rpc_data->rpc_response->sys_call_args.int_field1;
	}

	return retval;
}

/*************************************************************************
 *
 *   FUNCTION
 *
 *       _read
 *
 *   DESCRIPTION
 *
 *       Low level function to redirect IO to serial.
 *
 *************************************************************************/
int _read(int fd, char * buffer, int buflen) {
	int payload_size = sizeof(struct _sys_rpc);
	int retval = -1;

	if (!buffer || !buflen)
		return retval;

	/* Construct rpc payload */
	rpc_data->rpc->id = READ_SYSCALL_ID;
	rpc_data->rpc->sys_call_args.int_field1 = fd;
	rpc_data->rpc->sys_call_args.int_field2 = buflen;
	rpc_data->rpc->sys_call_args.data_len = 0; /*not used*/

	/* Transmit rpc request */
	env_lock_mutex(rpc_data->rpc_lock);
	get_response=0;
	send_rpc((void*) rpc_data->rpc, payload_size);
	env_unlock_mutex(rpc_data->rpc_lock);

	/* Wait for response from proxy on master */
	env_acquire_sync_lock(rpc_data->sync_lock);

	/* Obtain return args and return to caller */
	if (rpc_data->rpc_response->id == READ_SYSCALL_ID) {
		if (rpc_data->rpc_response->sys_call_args.int_field1 > 0) {
			memcpy(buffer, rpc_data->rpc_response->sys_call_args.data,
					rpc_data->rpc_response->sys_call_args.data_len);
		}

	    retval = rpc_data->rpc_response->sys_call_args.int_field1;
	}

	return retval;
}

/*************************************************************************
 *
 *   FUNCTION
 *
 *       _write
 *
 *   DESCRIPTION
 *
 *       Low level function to redirect IO to serial.
 *
 *************************************************************************/
int _write(int fd, const char * ptr, int len) {
	int retval = -1;
	int payload_size = sizeof(struct _sys_rpc) + len;
	int null_term = 0;

	if (fd == 1) {
		null_term = 1;
	}

	rpc_data->rpc->id = WRITE_SYSCALL_ID;
	rpc_data->rpc->sys_call_args.int_field1 = fd;
	rpc_data->rpc->sys_call_args.int_field2 = len;
	rpc_data->rpc->sys_call_args.data_len = len + null_term;
	memcpy(rpc_data->rpc->sys_call_args.data, ptr, len);
	if (null_term) {
		*(char*) (rpc_data->rpc->sys_call_args.data + len + null_term) = 0;
	}

	env_lock_mutex(rpc_data->rpc_lock);
	send_rpc((void*) rpc_data->rpc, payload_size);
	env_unlock_mutex(rpc_data->rpc_lock);

	env_acquire_sync_lock(rpc_data->sync_lock);

	if (rpc_data->rpc_response->id == WRITE_SYSCALL_ID) {
		retval = rpc_data->rpc_response->sys_call_args.int_field1;
	}

	return retval;

}

/*************************************************************************
 *
 *   FUNCTION
 *
 *       _close
 *
 *   DESCRIPTION
 *
 *       Close a file.  Minimal implementation
 *
 *************************************************************************/
int _close(int fd) {
	int payload_size = sizeof(struct _sys_rpc);
	int retval = -1;

	rpc_data->rpc->id = CLOSE_SYSCALL_ID;
	rpc_data->rpc->sys_call_args.int_field1 = fd;
	rpc_data->rpc->sys_call_args.int_field2 = 0; /*not used*/
	rpc_data->rpc->sys_call_args.data_len = 0; /*not used*/

	env_lock_mutex(rpc_data->rpc_lock);
	send_rpc((void*) rpc_data->rpc, payload_size);
	env_unlock_mutex(rpc_data->rpc_lock);

	/* Wait for response from proxy on master */
	env_acquire_sync_lock(rpc_data->sync_lock);

	if (rpc_data->rpc_response->id == CLOSE_SYSCALL_ID) {
		retval = rpc_data->rpc_response->sys_call_args.int_field1;
	}

	return retval;
}
