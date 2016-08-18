/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "openamp/open_amp.h"
#include "metal/alloc.h"
#include "rsc_table.h"
#include "platform_info.h"

#define SHUTDOWN_MSG	0xEF56A55A
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

struct _payload {
	unsigned long num;
	unsigned long size;
	char data[];
};

static int err_cnt;

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

#define RPMSG_HEADER_LEN sizeof(struct rpmsg_hdr)
#define MAX_RPMSG_BUFF_SIZE (RPMSG_BUFFER_SIZE - RPMSG_HEADER_LEN)
#define PAYLOAD_MIN_SIZE	1
#define PAYLOAD_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 24)
#define NUM_PAYLOADS		(PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *,
			  unsigned long);
/* Globals */
static struct rpmsg_channel *app_rp_chnl;
static struct rpmsg_endpoint *rp_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
static struct _payload *i_payload;
static int rnum = 0;
static int err_cnt = 0;
extern const struct remote_resource_table resources;
extern struct rproc_info_plat_local proc_table;

/* External functions */
extern void init_system();
extern void cleanup_system();

/* Application entry point */
int main()
{
	int status = 0;
	int shutdown_msg = SHUTDOWN_MSG;
	int i;
	int size;
	int expect_rnum = 0;

	LPRINTF(" 1 - Send data to remote core, retrieve the echo");
	LPRINTF(" and validate its integrity ..\n");
	/* Initialize HW system components */
	init_system();

	rsc_info.rsc_tab = (struct resource_table *)&resources;
	rsc_info.size = sizeof(resources);

	i_payload =
	    (struct _payload *)metal_allocate_memory(2 * sizeof(unsigned long) +
				      PAYLOAD_MAX_SIZE);

	if (!i_payload) {
		LPERROR("memory allocation failed.\n");
		return -1;
	}

	/* Initialize RPMSG framework */
	status =
	    remoteproc_resource_init(&rsc_info, &proc_table,
				     rpmsg_channel_created,
				     rpmsg_channel_deleted, rpmsg_read_cb,
				     &proc, 1);

	if (status) {
		LPERROR("Failed  to initialize remoteproc resource.\n");
		return -1;
	}

	LPRINTF("Remote proc resource initialized.\n");
	while (!app_rp_chnl) {
		hil_poll(proc->proc, 0);
	}

	LPRINTF("RPMSG channel has created.\n");
	for (i = 0, size = PAYLOAD_MIN_SIZE; i < (int)NUM_PAYLOADS; i++, size++) {
		i_payload->num = i;
		i_payload->size = size;

		/* Mark the data buffer. */
		memset(&(i_payload->data[0]), 0xA5, size);

		LPRINTF("sending payload number %lu of size %d\n",
			i_payload->num, (2 * sizeof(unsigned long)) + size);

		status = rpmsg_send(app_rp_chnl, i_payload,
			(2 * sizeof(unsigned long)) + size);

		if (status) {
			LPRINTF("Error sending data...\n");
			break;
		}
		LPRINTF("echo test: sent : %d\n",
		(2 * sizeof(unsigned long)) + size);

		expect_rnum++;
		do {
			hil_poll(proc->proc, 0);
		} while ((rnum < expect_rnum) && !err_cnt && app_rp_chnl);

	}

	LPRINTF("**********************************\n");
	LPRINTF(" Test Results: Error count = %d \n", err_cnt);
	LPRINTF("**********************************\n");
	/* Send shutdown message to remote */
	rpmsg_send(app_rp_chnl, &shutdown_msg, sizeof(int));
	sleep(1);
	LPRINTF("Quitting application .. Echo test end\n");

	remoteproc_resource_deinit(proc);
	cleanup_system();
	metal_free_memory(i_payload);
	return 0;
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
	app_rp_chnl = rp_chnl;
	rp_ept = rpmsg_create_ept(rp_chnl, rpmsg_read_cb, RPMSG_NULL,
				  RPMSG_ADDR_ANY);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	(void)rp_chnl;
	rpmsg_destroy_ept(rp_ept);
	LPRINTF("%s\n", __func__);
	app_rp_chnl = NULL;
	rp_ept = NULL;
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)rp_chnl;
	(void)src;
	(void)priv;
	int i;
	struct _payload *r_payload = (struct _payload *)data;

	LPRINTF(" received payload number %lu of size %d \r\n",
		r_payload->num, len);

	if (r_payload->size == 0) {
		LPERROR(" Invalid size of package is received.\n");
		err_cnt++;
		return;
	}
	/* Validate data buffer integrity. */
	for (i = 0; i < (int)r_payload->size; i++) {
		if (r_payload->data[i] != 0xA5) {
			LPRINTF("Data corruption at index %d \n", i);
			err_cnt++;
			break;
		}
	}
	rnum = r_payload->num + 1;
}
