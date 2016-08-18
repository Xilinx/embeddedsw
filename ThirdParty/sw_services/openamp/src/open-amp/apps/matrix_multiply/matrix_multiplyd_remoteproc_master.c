/* This is a sample demonstration application that showcases usage of remoteproc
and rpmsg APIs. This application is meant to run on the master CPU running baremetal
and showcases booting of linux remote firmware using remoteproc and
IPC with remote firmware using rpmsg; Baremetal on master core acts as a remoteproc master
but as an rpmsg remote;It brings up a remote Linux based
firmware which acts as an rpmsg master and offloads matrix multiplication to the baremetal context.
Linux app generates two random matrices and transmits them to baremetal env which computes
the product and transmits results back to Linux. Once Linux application is complete, it
requests a shutdown from baremetal env. Baremetal env acknowledges with a shutdown message which results
in Linux starting a system halt. Baremetal env shutsdown the remot core after a reasonable delay which allows
Linux to gracefully shutdown. */

/* Including required headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openamp/open_amp.h"

#define MAX_SIZE        6
#define NUM_MATRIX      2

#define SHUTDOWN_MSG	0xEF56A55A

typedef struct _matrix {
	unsigned long size;
	unsigned long elements[MAX_SIZE][MAX_SIZE];
} matrix;

static matrix matrix_array[NUM_MATRIX];

static matrix matrix_result;

/* Prototypes */
void sleep();

/* Application provided callbacks */
void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
		   void *pric, unsigned long src);

/* Globals */
struct rpmsg_endpoint *rp_ept;
struct rpmsg_channel *app_rp_chnl;
char fw_name[] = "firmware1";

int int_flag;

static int shutdown_called = 0;

/* External functions */
extern void init_system();
extern void cleanup_system();

/* External variables */
extern struct hil_proc proc_table[];

static void Matrix_Multiply(const matrix * m, const matrix * n, matrix * r)
{
	int i, j, k;

	r->size = m->size;

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < n->size; ++j) {
			r->elements[i][j] = 0;
		}
	}

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < n->size; ++j) {
			for (k = 0; k < r->size; ++k) {
				r->elements[i][j] +=
				    m->elements[i][k] * n->elements[k][j];
			}
		}
	}
}

/* Application entry point */
int main()
{

	int status;
	struct remote_proc *proc;
	int i;
	int shutdown_msg = SHUTDOWN_MSG;

	/* Initialize HW system components */
	init_system();

	status =
	    remoteproc_init((void *)fw_name, &proc_table[0], rpmsg_channel_created,
			    rpmsg_channel_deleted, rpmsg_read_cb, &proc);

	if (!status) {
		status = remoteproc_boot(proc);
	}

	if (status) {
		return -1;
	}
	while (1) {

		if (int_flag) {

			if (shutdown_called == 1) {
				break;
			}

			/* Process received data and multiple matrices. */
			Matrix_Multiply(&matrix_array[0], &matrix_array[1],
					&matrix_result);

			/* Send the result of matrix multiplication back to master. */
			rpmsg_send(app_rp_chnl, &matrix_result, sizeof(matrix));
			int_flag = 0;
		}

		hil_poll(proc->proc, 0);
	}

	/* Send shutdown message to remote */
	rpmsg_send(app_rp_chnl, &shutdown_msg, sizeof(int));

	for (i = 0; i < 100000; i++) {
		sleep();
	}

	remoteproc_shutdown(proc);

	remoteproc_deinit(proc);

	cleanup_system();
	return 0;
}

/* This callback gets invoked when the remote chanl is created */
void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{

	app_rp_chnl = rp_chnl;
	rp_ept =
	    rpmsg_create_ept(rp_chnl, rpmsg_read_cb, RPMSG_NULL,
			     RPMSG_ADDR_ANY);

}

/* This callback gets invoked when the remote channel is deleted */
void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	rpmsg_destroy_ept(rp_ept);
}

/* This is the read callback, note we are in a task context when this callback
is invoked, so kernel primitives can be used freely */
void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
		   void *priv, unsigned long src)
{

	if ((*(int *)data) == SHUTDOWN_MSG) {
		shutdown_called = 1;
	} else {
		memcpy(matrix_array, data, len);
	}

	int_flag = 1;
}

void sleep()
{
	volatile int i;
	for (i = 0; i < 10000000; i++) ;
}
