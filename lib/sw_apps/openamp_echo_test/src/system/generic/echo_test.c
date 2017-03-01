/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

#include <stdio.h>
#include "openamp/open_amp.h"
#include "rsc_table.h"
#include "platform_info.h"

#define SHUTDOWN_MSG	0xEF56A55A

//#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

#define REMOTE_ACTIVE 0
#define REMOTE_RESETTING 1
#define REMOTE_RESETTED 2

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

/* Local variables */
static struct rpmsg_endpoint *rp_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
static int remote_proc_state;
static int evt_chnl_deleted = 0;

static void virtio_rst_cb(struct hil_proc *hproc, int id)
{
	/* hil_proc only supports single virtio device */
	(void)id;

	if (!proc || proc->proc != hproc || !proc->rdev)
		return;
	LPRINTF("Resetting RPMsg\n");
	atomic_thread_fence(memory_order_seq_cst);
	remote_proc_state = REMOTE_RESETTING;

	rpmsg_deinit(proc->rdev);
	proc->rdev = NULL;

	atomic_thread_fence(memory_order_seq_cst);
	remote_proc_state = REMOTE_RESETTED;
	LPRINTF("RPMsg resetted\n");
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)priv;
	(void)src;

	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		evt_chnl_deleted = 1;
		return;
	}

	/* Send data back to master */
	rpmsg_send(rp_chnl, data, len);
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
	rp_ept = rpmsg_create_ept(rp_chnl, rpmsg_read_cb, RPMSG_NULL,
				  RPMSG_ADDR_ANY);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	(void)rp_chnl;

	rpmsg_destroy_ept(rp_ept);
	rp_ept = NULL;
	evt_chnl_deleted = 1;
}

/* Application entry point */
int app(struct hil_proc *hproc)
{
	int status = 0;

	/* Initialize RPMSG framework */
	LPRINTF("Try to init remoteproc resource\n");
	status =
	    remoteproc_resource_init(&rsc_info, hproc,
				     rpmsg_channel_created,
				     rpmsg_channel_deleted, rpmsg_read_cb,
				     &proc, 0);
	LPRINTF("init remoteproc resource done\n");

	if (RPROC_SUCCESS != status) {
		LPERROR("Failed  to initialize remoteproc resource.\n");
		return -1;
	}
	LPRINTF("init remoteproc resource succeeded\n");

	hil_set_vdev_rst_cb(hproc, 0, virtio_rst_cb);

	LPRINTF("Waiting for events...\n");
	do {
		do {
			hil_poll(proc->proc, 0);
		} while (!evt_chnl_deleted);

		while (remote_proc_state == REMOTE_RESETTING);

		if (remote_proc_state == REMOTE_RESETTED) {
			LPRINTF("Reinitializing RPMsg\n");
			status = rpmsg_init(hproc, &proc->rdev,
					rpmsg_channel_created,
					rpmsg_channel_deleted, rpmsg_read_cb,
					1);
			if (status != RPROC_SUCCESS) {
				LPERROR("Reinit RPMsg failed\n");
				goto out;
			} else {
				LPRINTF("Reinit RPMsg succeeded\n");
				evt_chnl_deleted=0;
			}
		} else {
			break;
		}
	} while(1);

out:
	/* disable interrupts and free resources */
	LPRINTF("De-initializating remoteproc resource\n");
	remoteproc_resource_deinit(proc);

	return 0;
}

int main(int argc, char *argv[])
{
	unsigned long proc_id = 0;
	unsigned long rsc_id = 0;
	struct hil_proc *hproc;
	int status = -1;

	LPRINTF("Starting application...\n");

	/* Initialize HW system components */
	init_system();

	if (argc >= 2) {
		proc_id = strtoul(argv[1], NULL, 0);
	}

	if (argc >= 3) {
		rsc_id = strtoul(argv[2], NULL, 0);
	}

	hproc = platform_create_proc(proc_id);
	if (!hproc) {
		LPERROR("Failed to create proc platform data.\n");
	} else {
		rsc_info.rsc_tab = get_resource_table(
			(int)rsc_id, &rsc_info.size);
		if (!rsc_info.rsc_tab) {
			LPERROR("Failed to get resource table data.\n");
		} else {
			status = app(hproc);
		}
	}

	cleanup_system();

	return status;
}
