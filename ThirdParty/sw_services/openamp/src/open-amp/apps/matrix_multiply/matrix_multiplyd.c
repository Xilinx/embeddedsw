/* This is a sample demonstration application that showcases usage of remoteproc
and rpmsg APIs on the remote core. This application is meant to run on the remote CPU
running baremetal code. This applicationr receives two matrices from the master,
multiplies them and returns the result to the master core. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openamp/open_amp.h"
#include "rsc_table.h"
#include "platform_info.h"

#define	MAX_SIZE                6
#define NUM_MATRIX              2
#define SHUTDOWN_MSG            0xEF56A55A

typedef struct _matrix {
	unsigned int size;
	unsigned int elements[MAX_SIZE][MAX_SIZE];
} matrix;

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *,
			  unsigned long);
static void Matrix_Multiply(const matrix * m, const matrix * n, matrix * r);

/* Globals */
static struct rpmsg_channel *app_rp_chnl;
static struct rpmsg_endpoint *rp_ept;
static matrix matrix_array[NUM_MATRIX];
static matrix matrix_result;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
static int evt_chnl_deleted = 0;

extern const struct remote_resource_table resources;
extern struct rproc_info_plat_local proc_table;

/* External functions */
extern void init_system();
extern void cleanup_system();

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

	remoteproc_resource_deinit(proc);
	cleanup_system();

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
	rp_ept = NULL;
	app_rp_chnl = NULL;
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)rp_chnl;
	(void)priv;
	(void)src;

	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		evt_chnl_deleted = 1;
	} else {
		memcpy(matrix_array, data, len);
		/* Process received data and multiple matrices. */
		Matrix_Multiply(&matrix_array[0], &matrix_array[1],
				&matrix_result);

		/* Send the result of matrix multiplication back to master. */
		rpmsg_send(app_rp_chnl, &matrix_result, sizeof(matrix));
	}
}

static void Matrix_Multiply(const matrix * m, const matrix * n, matrix * r)
{
	unsigned int i, j, k;

	memset(r, 0x0, sizeof(matrix));
	r->size = m->size;

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < n->size; ++j) {
			for (k = 0; k < r->size; ++k) {
				r->elements[i][j] +=
				    m->elements[i][k] * n->elements[k][j];
			}
		}
	}
}
