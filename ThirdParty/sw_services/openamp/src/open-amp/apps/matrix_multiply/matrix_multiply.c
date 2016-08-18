/* This is a sample demonstration application that showcases usage of remoteproc
and rpmsg APIs on the remote core. This application is meant to run on the remote CPU
running baremetal code. This applicationr receives two matrices from the master,
multiplies them and returns the result to the master core. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "openamp/open_amp.h"
#include "rsc_table.h"
#include "platform_info.h"

#define	MAX_SIZE                6
#define NUM_MATRIX              2
#define SHUTDOWN_MSG            0xEF56A55A

#define raw_printf(format, ...) printf(format, ##__VA_ARGS__)
#define LPRINTF(format, ...) raw_printf("CLIENT> " format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

typedef struct _matrix {
	unsigned int size;
	unsigned int elements[MAX_SIZE][MAX_SIZE];
} matrix;

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *,
			  unsigned long);

/* Globals */
static struct rpmsg_channel *app_rp_chnl;
static struct _matrix i_matrix[2];
static struct _matrix e_matrix;
static unsigned int result_returned = 0;
static int err_cnt = 0;;
static struct rpmsg_endpoint *rp_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
extern const struct remote_resource_table resources;
extern struct rproc_info_plat_local proc_table;

/* External functions */
extern void init_system();
extern void cleanup_system();

int __attribute__((weak)) _gettimeofday(struct timeval *tv,
					void *tz)
{
	(void)tv;
	(void)tz;
	return 0;
}

static void matrix_print(struct _matrix *m)
{
	unsigned int i, j;

	/* Generate two random matrices */
	LPRINTF("Printing matrix... \n");

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < m->size; ++j)
			raw_printf(" %u ", m->elements[i][j]);
		raw_printf("\n");
	}
}

static void generate_matrices(int num_matrices,
			      unsigned int matrix_size, void *p_data)
{
	unsigned int i, j, k;
	struct _matrix *p_matrix = p_data;
	unsigned long value;


	for (i = 0; i < (unsigned int)num_matrices; i++) {
		/* Initialize workload */
		p_matrix[i].size = matrix_size;

		//LPRINTF("Input matrix %d \n", i);
		for (j = 0; j < matrix_size; j++) {
			//printf("\n");
			for (k = 0; k < matrix_size; k++) {

				value = (rand() & 0x7F);
				value = value % 10;
				p_matrix[i].elements[j][k] = value;
				//printf(" %u ", p_matrix[i].elements[j][k]);
			}
		}
		//printf("\n");
	}

}

static void matrix_multiply(const matrix * m, const matrix * n, matrix * r)
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

/* Application entry point */
int main()
{
	int shutdown_msg = SHUTDOWN_MSG;
	int c;
	int status = 0;

	/* Initialize HW system components */
	init_system();

	rsc_info.rsc_tab = (struct resource_table *)&resources;
	rsc_info.size = sizeof(resources);

	LPRINTF("Compute thread unblocked ..\n");
	LPRINTF("It will generate two random matrices.\n");
	LPRINTF("Send to the remote and get the computation result back.\n");
	LPRINTF("It will then check if the result is expected.\n");

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
	err_cnt = 0;
	srand(time(NULL));
	for (c = 0; c < 200; c++) {
		generate_matrices(2, MAX_SIZE, i_matrix);
		matrix_multiply(&i_matrix[0], &i_matrix[1],
				&e_matrix);
		result_returned = 0;
		status = rpmsg_send(app_rp_chnl, i_matrix, sizeof(i_matrix));

		if (status) {
			LPRINTF("Error sending data...\n");
			break;
		}
		LPRINTF("Matrix multiply: sent : %u\n", sizeof(i_matrix));
		do {
			hil_poll(proc->proc, 0);
		} while (!result_returned && !err_cnt && app_rp_chnl);

		if (err_cnt && !app_rp_chnl)
			break;
	}

	if (app_rp_chnl) {
		/* Send shutdown message to remote */
		rpmsg_send(app_rp_chnl, &shutdown_msg, sizeof(int));
	}
	sleep(1);
	LPRINTF("Quitting application .. Matrix multiplication end\n");

	LPRINTF("**********************************\n");
	LPRINTF(" Test Results: Error count = %d \n", err_cnt);
	LPRINTF("**********************************\n");

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
	LPRINTF("%s\n", __func__);
	app_rp_chnl = NULL;
	rp_ept = NULL;
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)rp_chnl;
	(void)priv;
	(void)src;
	struct _matrix *r_matrix = (struct _matrix *)data;
	int i, j;
	if (len != sizeof(struct _matrix)) {
		LPERROR("Received matrix is of invalid len: %d:%d\n",
			(int)sizeof(struct _matrix), len);
		err_cnt++;
		return;
	}
	for (i = 0; i < MAX_SIZE; i++) {
		for (j = 0; j < MAX_SIZE; j++) {
			if (r_matrix->elements[i][j] !=
				e_matrix.elements[i][j]) {
				err_cnt++;
				break;
			}
		}
	}
	if (err_cnt) {
		LPERROR("Result mismatched...\n");
		LPERROR("Expected matrix:\n");
		matrix_print(&e_matrix);
		LPERROR("Actual matrix:\n");
		matrix_print(r_matrix);
	} else {
		result_returned = 1;
	}
}
