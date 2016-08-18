/* This is a sample demonstration application that showcases usage of proxy from the remote core.
 This application is meant to run on the remote CPU running baremetal.
 This applicationr can print to to master console and perform file I/O using proxy mechanism. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include"openamp/open_amp.h"
#include "openamp/rpmsg_retarget.h"
#include "metal/alloc.h"
#include "rsc_table.h"
#include "platform_info.h"

#define PROXY_ENDPOINT			127

/* System call definitions */
#define OPEN_SYSCALL_ID		1
#define CLOSE_SYSCALL_ID	2
#define WRITE_SYSCALL_ID	3
#define READ_SYSCALL_ID		4
#define ACK_STATUS_ID		5
#define TERM_SYSCALL_ID		6

#define RPC_BUFF_SIZE 512
#define RPC_CHANNEL_READY_TO_CLOSE "rpc_channel_ready_to_close"

#define raw_printf(format, ...) printf(format, ##__VA_ARGS__)
#define LPRINTF(format, ...) raw_printf("Master> " format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

struct _proxy_data {
	int active;
	int rpmsg_proxy_fd;
	struct _sys_rpc *rpc;
	struct _sys_rpc *rpc_response;
	char *firmware_path;
};

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *,
			  unsigned long);
static void rpmsg_proxy_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src);

/* Globals */
static struct rpmsg_channel *app_rp_chnl;
static struct rpmsg_endpoint *proxy_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
static struct _proxy_data *proxy;
static int err_cnt = 0;

extern const struct remote_resource_table resources;
extern struct rproc_info_plat_local proc_table;

/* External functions */
extern void init_system();
extern void cleanup_system();

#define REDEF_O_CREAT 100
#define REDEF_O_EXCL 200
#define REDEF_O_RDONLY 0
#define REDEF_O_WRONLY 1
#define REDEF_O_RDWR 2
#define REDEF_O_APPEND 2000
#define REDEF_O_ACCMODE 3

#define RPC_CHANNEL_READY_TO_CLOSE "rpc_channel_ready_to_close"

int handle_open(struct _sys_rpc *rpc)
{
	int fd, ret;

	/* Open remote fd */
	fd = open(rpc->sys_call_args.data, rpc->sys_call_args.int_field1,
		  rpc->sys_call_args.int_field2);

	/* Construct rpc response */
	proxy->rpc_response->id = OPEN_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = fd;
	proxy->rpc_response->sys_call_args.int_field2 = 0;	/*not used */
	proxy->rpc_response->sys_call_args.data_len = 0;	/*not used */

	/* Transmit rpc response */
	ret = rpmsg_sendto(app_rp_chnl, proxy->rpc_response,
			sizeof(struct _sys_rpc), PROXY_ENDPOINT);

	return ret;
}

int handle_close(struct _sys_rpc *rpc)
{
	int ret;

	/* Close remote fd */
	ret = close(rpc->sys_call_args.int_field1);

	/* Construct rpc response */
	proxy->rpc_response->id = CLOSE_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = ret;
	proxy->rpc_response->sys_call_args.int_field2 = 0;	/*not used */
	proxy->rpc_response->sys_call_args.data_len = 0;	/*not used */

	/* Transmit rpc response */
	ret = rpmsg_sendto(app_rp_chnl, proxy->rpc_response,
			sizeof(struct _sys_rpc), PROXY_ENDPOINT);

	return ret;
}

int handle_read(struct _sys_rpc *rpc)
{
	int bytes_read, payload_size;
	char *buff;
	int ret;

	/* Allocate buffer for requested data size */
	buff = malloc(rpc->sys_call_args.int_field2);

	if (rpc->sys_call_args.int_field1 == 0)
		/* Perform read from fd for large size since this is a
		   STD/I request */
		bytes_read = read(rpc->sys_call_args.int_field1, buff, 512);
	else
		/* Perform read from fd */
		bytes_read = read(rpc->sys_call_args.int_field1, buff,
				  rpc->sys_call_args.int_field2);

	/* Construct rpc response */
	proxy->rpc_response->id = READ_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = bytes_read;
	proxy->rpc_response->sys_call_args.int_field2 = 0;	/* not used */
	proxy->rpc_response->sys_call_args.data_len = bytes_read;
	if (bytes_read > 0)
		memcpy(proxy->rpc_response->sys_call_args.data, buff,
		       bytes_read);

	payload_size = sizeof(struct _sys_rpc) +
	    ((bytes_read > 0) ? bytes_read : 0);

	/* Transmit rpc response */
	ret = rpmsg_sendto(app_rp_chnl, proxy->rpc_response,
			payload_size, PROXY_ENDPOINT);

	return ret;
}

int handle_write(struct _sys_rpc *rpc)
{
	int bytes_written;
	int ret;

	/* Write to remote fd */
	bytes_written = write(rpc->sys_call_args.int_field1,
			      rpc->sys_call_args.data,
			      rpc->sys_call_args.int_field2);

	/* Construct rpc response */
	proxy->rpc_response->id = WRITE_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = bytes_written;
	proxy->rpc_response->sys_call_args.int_field2 = 0;	/*not used */
	proxy->rpc_response->sys_call_args.data_len = 0;	/*not used */

	/* Transmit rpc response */
	ret = rpmsg_sendto(app_rp_chnl, proxy->rpc_response,
			sizeof(struct _sys_rpc), PROXY_ENDPOINT);

	return ret;
}

int handle_rpc(struct _sys_rpc *rpc)
{
	int retval;
	char *data = (char *)rpc;
	if (!strcmp(data, RPC_CHANNEL_READY_TO_CLOSE)) {
		proxy->active = 0;
		return 0;
	}

	/* Handle RPC */
	switch ((int)(rpc->id)) {
	case OPEN_SYSCALL_ID:
		{
			retval = handle_open(rpc);
			break;
		}
	case CLOSE_SYSCALL_ID:
		{
			retval = handle_close(rpc);
			break;
		}
	case READ_SYSCALL_ID:
		{
			retval = handle_read(rpc);
			break;
		}
	case WRITE_SYSCALL_ID:
		{
			retval = handle_write(rpc);
			break;
		}
	default:
		{
			LPERROR
			    ("Invalid RPC sys call ID: %d:%d!\n",
			     rpc->id, WRITE_SYSCALL_ID);
			retval = -1;
			break;
		}
	}

	return retval;
}

void terminate_rpc_app()
{
	int msg = TERM_SYSCALL_ID;
	LPRINTF("sending shutdown signal.\n");
	rpmsg_sendto(app_rp_chnl, &msg, sizeof(msg), PROXY_ENDPOINT);
}

void exit_action_handler(int signum)
{
	(void)signum;
	proxy->active = 0;
}

void kill_action_handler(int signum)
{
	(void)signum;
	LPRINTF("RPC service killed !!\n");

	/* Send shutdown signal to remote application */
	if (app_rp_chnl)
		terminate_rpc_app();

	/* wait for a while to let the remote finish cleanup */
	sleep(1);
	remoteproc_resource_deinit(proc);

	/* Free up resources */
	free(proxy->rpc);
	free(proxy->rpc_response);
	free(proxy);

	cleanup_system();
}

/* Application entry point */
int main()
{
	int status;
	int ret = 0;
	struct sigaction exit_action;
	struct sigaction kill_action;

	/* Initialize HW system components */
	init_system();

	/* Allocate memory for proxy data structure */
	proxy = metal_allocate_memory(sizeof(struct _proxy_data));
	if (!proxy) {
		LPERROR("Failed to allocate memory for proxy\n");
		return -1;
	}
	memset(proxy, 0, sizeof(struct _proxy_data));
	proxy->active = 1;
	/* Initialize signalling infrastructure */
	memset(&exit_action, 0, sizeof(struct sigaction));
	memset(&kill_action, 0, sizeof(struct sigaction));
	exit_action.sa_handler = exit_action_handler;
	kill_action.sa_handler = kill_action_handler;
	sigaction(SIGTERM, &exit_action, NULL);
	sigaction(SIGINT, &exit_action, NULL);
	sigaction(SIGKILL, &kill_action, NULL);
	sigaction(SIGHUP, &kill_action, NULL);

	/* Allocate memory for rpc payloads */
	proxy->rpc = metal_allocate_memory(RPC_BUFF_SIZE);
	proxy->rpc_response = metal_allocate_memory(RPC_BUFF_SIZE);
	if (!proxy->rpc || !proxy->rpc_response) {
		LPERROR("Failed to allocate memory for proxy data\n");
		ret = -1;
		goto error1;
	}

	rsc_info.rsc_tab = (struct resource_table *)&resources;
	rsc_info.size = sizeof(resources);

	/* Initialize RPMSG framework */
	status =
	    remoteproc_resource_init(&rsc_info, &proc_table,
				     rpmsg_channel_created,
				     rpmsg_channel_deleted, rpmsg_read_cb,
				     &proc, 1);

	if (status) {
		LPERROR("Failed  to initialize remoteproc resource.\n");
		ret = -1;
		goto error1;
	}

	LPRINTF("Remote proc resource initialized.\n");
	while (!app_rp_chnl) {
		hil_poll(proc->proc, 0);
	}

	LPRINTF("RPMSG channel has created.\n");
	while(proxy->active && app_rp_chnl && !err_cnt) {
		 hil_poll(proc->proc, 0);
	}

	if (err_cnt) {
		LPERROR("Got error!\n");
		ret = -1;
	}
	LPRINTF("\nRPC service exiting !!\n");

	/* Send shutdown signal to remote application */
	if (app_rp_chnl)
		terminate_rpc_app();

	/* Need to wait here for sometime to allow remote application to
	   complete its unintialization */
	sleep(1);

	remoteproc_resource_deinit(proc);

error1:
	/* Free up resources */
	if (proxy->rpc)
		metal_free_memory(proxy->rpc);
	if (proxy->rpc_response)
		metal_free_memory(proxy->rpc_response);
	if (proxy)
		metal_free_memory(proxy);

	cleanup_system();

	return ret;
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
	proxy_ept = rpmsg_create_ept(rp_chnl, rpmsg_proxy_cb, RPMSG_NULL,
				  PROXY_ENDPOINT);
	app_rp_chnl = rp_chnl;
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	(void)rp_chnl;
	rpmsg_destroy_ept(proxy_ept);
	proxy_ept = NULL;
	app_rp_chnl = NULL;
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)rp_chnl;
	(void)data;
	(void)len;
	(void)priv;
	(void)src;
}

static void rpmsg_proxy_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)rp_chnl;
	(void)priv;
	(void)src;

	if (len < (int)sizeof(struct _sys_rpc)) {
		LPERROR("Received data is less than the rpc structure: %d\n",
			len);
		err_cnt++;
	}

	/* In case the shared memory is device memory
	 * E.g. For now, we only use UIO device memory in Linux.
	 */
	metal_memcpy_io(proxy->rpc, data, len);
	if (handle_rpc(proxy->rpc)) {
		LPRINTF("\nHandling remote procedure call errors:\n");
		raw_printf("rpc id %d\n", proxy->rpc->id);
		raw_printf("rpc int field1 %d\n",
		       proxy->rpc->sys_call_args.int_field1);
		raw_printf("\nrpc int field2 %d\n",
		       proxy->rpc->sys_call_args.int_field2);
		err_cnt++;
	}
}
