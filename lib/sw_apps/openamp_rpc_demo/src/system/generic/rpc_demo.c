/* This is a sample demonstration application that showcases usage of proxy from the remote core.
 This application is meant to run on the remote CPU running baremetal.
 This applicationr can print to to master console and perform file I/O using proxy mechanism. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "openamp/open_amp.h"
#include "openamp/rpmsg_retarget.h"
#include "rsc_table.h"
#include "platform_info.h"

#define REDEF_O_CREAT 100
#define REDEF_O_EXCL 200
#define REDEF_O_RDONLY 0
#define REDEF_O_WRONLY 1
#define REDEF_O_RDWR 2
#define REDEF_O_APPEND 2000
#define REDEF_O_ACCMODE 3

#define RPC_CHANNEL_READY_TO_CLOSE "rpc_channel_ready_to_close"

#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

/* Global functions and variables */
extern int init_system(void);
extern void cleanup_system(void);

/* Local variables */
static struct rpmsg_channel *app_rp_chnl;
static volatile int chnl_is_alive = 0;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;

/*-----------------------------------------------------------------------------*
 *  RPMSG callbacks setup by remoteproc_resource_init()
 *-----------------------------------------------------------------------------*/
static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			  void *priv, unsigned long src)
{
	(void)rp_chnl;
	(void)data;
	(void)len;
	(void)priv;
	(void)src;
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
	app_rp_chnl = rp_chnl;
	chnl_is_alive = 1;
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	(void)rp_chnl;
	app_rp_chnl = NULL;
}

static void shutdown_cb(struct rpmsg_channel *rp_chnl)
{
	(void)rp_chnl;
	chnl_is_alive = 0;
}

/*-----------------------------------------------------------------------------*
 *  Application specific
 *-----------------------------------------------------------------------------*/
int app (struct hil_proc *hproc)
{
	int fd, bytes_written, bytes_read;
	char fname[] = "remote.file";
	char wbuff[50];
	char rbuff[1024];
	char ubuff[50];
	float fdata;
	int idata;
	int ret;
	int status;

	/* Initialize framework */
	LPRINTF("Try to init remoteproc resource\n");
	status = remoteproc_resource_init(&rsc_info, hproc,
					  rpmsg_channel_created,
					  rpmsg_channel_deleted, rpmsg_read_cb,
					  &proc, 0);
	if (RPROC_SUCCESS != status) {
		LPERROR("Failed  to initialize remoteproc resource.\n");
		return -1;
	}

	LPRINTF("Init remoteproc resource done\n");

	LPRINTF("Waiting for channel creation...\n");
	while (!chnl_is_alive) {
		hil_poll(proc->proc, 0);
	}

	/* redirect I/Os */
	LPRINTF("Initializating I/Os redirection...\n");
	rpmsg_retarget_init(app_rp_chnl, shutdown_cb);

	printf("\r\nRemote>Baremetal Remote Procedure Call (RPC) Demonstration\r\n");
	printf("\r\nRemote>***************************************************\r\n");

	printf("\r\nRemote>Rpmsg based retargetting to proxy initialized..\r\n");

	/* Remote performing file IO on Master */
	printf("\r\nRemote>FileIO demo ..\r\n");

	printf("\r\nRemote>Creating a file on master and writing to it..\r\n");
	fd = open(fname, REDEF_O_CREAT | REDEF_O_WRONLY | REDEF_O_APPEND,
		  S_IRUSR | S_IWUSR);
	printf("\r\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);

	sprintf(wbuff, "This is a test string being written to file..");
	bytes_written = write(fd, wbuff, strlen(wbuff));
	printf("\r\nRemote>Wrote to fd = %d, size = %d, content = %s\r\n", fd,
	       bytes_written, wbuff);
	close(fd);
	printf("\r\nRemote>Closed fd = %d\r\n", fd);

	/* Remote performing file IO on Master */
	printf("\r\nRemote>Reading a file on master and displaying its contents..\r\n");
	fd = open(fname, REDEF_O_RDONLY, S_IRUSR | S_IWUSR);
	printf("\r\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);
	bytes_read = read(fd, rbuff, 1024);
	*(char *)(&rbuff[0] + bytes_read + 1) = 0;
	printf("\r\nRemote>Read from fd = %d, size = %d, printing contents below .. %s\r\n",
		fd, bytes_read, rbuff);
	close(fd);
	printf("\r\nRemote>Closed fd = %d\r\n", fd);

	while (1) {
		/* Remote performing STDIO on Master */
		printf("\r\nRemote>Remote firmware using scanf and printf ..\r\n");
		printf("\r\nRemote>Scanning user input from master..\r\n");
		printf("\r\nRemote>Enter name\r\n");
		ret = scanf("%s", ubuff);
		if (ret) {
			printf("\r\nRemote>Enter age\r\n");
			ret = scanf("%d", &idata);
			if (ret) {
				printf("\r\nRemote>Enter value for pi\r\n");
				ret = scanf("%f", &fdata);
				if (ret) {
					printf("\r\nRemote>User name = '%s'\r\n", ubuff);
					printf("\r\nRemote>User age = '%d'\r\n", idata);
					printf("\r\nRemote>User entered value of pi = '%f'\r\n", fdata);
				}
			}
		}
		if (!ret) {
			scanf("%s", ubuff);
			printf("Remote> Invalid value. Starting again....");
		} else {
			printf("\r\nRemote>Repeat demo ? (enter yes or no) \r\n");
			scanf("%s", ubuff);
			if ((strcmp(ubuff, "no")) && (strcmp(ubuff, "yes"))) {
				printf("\r\nRemote>Invalid option. Starting again....\r\n");
			} else if ((!strcmp(ubuff, "no"))) {
				printf("\r\nRemote>RPC retargetting quitting ...\r\n");
				break;
			}
		}
	}

	printf("\r\nRemote> Firmware's rpmsg-openamp-demo-channel going down! \r\n");

	sprintf(wbuff, RPC_CHANNEL_READY_TO_CLOSE);
	rpmsg_retarget_send(wbuff, sizeof (RPC_CHANNEL_READY_TO_CLOSE) + 1);

	LPRINTF("Waiting for channel deletion...\n");
	while (chnl_is_alive) {
		hil_poll(proc->proc, 0);
	}

	LPRINTF("De-initializating rpmsg_retarget\n");
	rpmsg_retarget_deinit(app_rp_chnl);
	/* disable interrupts and free resources */
	LPRINTF("De-initializating remoteproc resource\n");
	remoteproc_resource_deinit(proc);

	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
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

	/* Create HIL proc */
	hproc = platform_create_proc(proc_id);
	if (!hproc) {
		LPERROR("Failed to create hil proc.\n");
	} else {
		rsc_info.rsc_tab =
			get_resource_table((int)rsc_id, &rsc_info.size);
		if (!rsc_info.rsc_tab) {
			LPERROR("Failed to get resource table data.\n");
		} else {
			status = app(hproc);
		}
	}

	LPRINTF("Stopping application...\n");

	cleanup_system();
	return status;
}

