/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

#include "rsc_table.h"
#include "platform_info.h"

#define SHUTDOWN_MSG	0xEF56A55A

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *,
			  unsigned long);

/* Globals */
static struct rpmsg_endpoint *rp_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
static int evt_chnl_deleted = 0;
extern const struct remote_resource_table resources;
extern struct rproc_info_plat_local proc_table;

/* External functions */
extern void init_system(void);
extern void cleanup_system(void);

/* Application entry point */
int main(void)
{
	int status = 0;

	/* Initialize HW system components */
	init_system();

	rsc_info.rsc_tab = (struct resource_table *)&resources;
	rsc_info.size = sizeof(resources);

	/* Initialize RPMSG framework */
	status =
	    remoteproc_resource_init(&rsc_info, &proc_table,
				     rpmsg_channel_created,
				     rpmsg_channel_deleted, rpmsg_read_cb,
				     &proc, 0);

	if (RPROC_SUCCESS != status) {
		return -1;
	}

	do {
		hil_poll(proc->proc, 0);
	} while (!evt_chnl_deleted);

	/* disable interrupts and free resources */
	remoteproc_resource_deinit(proc);

	cleanup_system();

	return 0;
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
