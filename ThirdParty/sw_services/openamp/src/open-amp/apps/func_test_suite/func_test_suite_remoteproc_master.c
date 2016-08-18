/* This is a test demonstration application that tests usage of remoteproc
and rpmsg APIs. This application is meant to run on the master CPU running baremetal env
and showcases booting of two sub-sequent remote firmware cycles using remoteproc and
IPC with remote firmware using rpmsg. It brings up a remote Linux based remote
firmware which can respond to test calls. Master app executes tests to validate
the rpmsg APIs and shutsdown the core once the test has been completed.*/

/* Including required headers */
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "openamp/open_amp.h"
#include "test_suite.h"
#include "platform_info.h"


/* Application provided callbacks */
void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
void rpmsg_read_default_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			   void *pric, unsigned long src);
void rpmsg_read_ept1_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			void *pric, unsigned long src);
void rpmsg_read_ept2_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			void *pric, unsigned long src);

int test_rpmsg_send(struct rpmsg_channel *rpmsg_chnl);
int test_rpmsg_send_offchannel(struct rpmsg_channel *rpmsg_chnl,
			       unsigned long src, unsigned long dst);
int test_rpmsg_create_ept(struct rpmsg_channel *rpmsg_chnl);
int test_remoteproc_multiple_lifecycles(char *firmware_name);
int test_rpmsg_send_offchannel_impl(struct rpmsg_channel *rpmsg_chnl,
				    unsigned long src, unsigned long dst);
int test_rpmsg_send_impl(struct rpmsg_channel *rpmsg_chnl);
int test_rpmsg_remote_channel_deletion(struct rpmsg_channel *rpmsg_chnl,
				       char *channel_name);
int test_execute_suite(char *firmware_name);
static void sleep();

int int_flag;

struct rpmsg_endpoint *rp_ept1, *rp_ept2;
struct rpmsg_channel *app_rp_chnl;
char fw_name1[] = "firmware1";

struct _payload *p_payload = NULL;
struct _payload *r_payload = NULL;

/* External functions */
extern void init_system();
extern void cleanup_system();

/* External variables */
extern struct hil_proc proc_table[];

void sleep()
{
	int i;
	for (i = 0; i < 10000; i++) ;
}

int main()
{

	/* Initialize HW system components */
	init_system();

	test_execute_suite(fw_name1);

	cleanup_system();
	return 0;
}

/* This callback gets invoked when the remote chanl is created */
void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
	app_rp_chnl = rp_chnl;

	rp_ept1 =
	    rpmsg_create_ept(rp_chnl, rpmsg_read_ept1_cb, RPMSG_NULL,
			     RPMSG_ADDR_ANY);
}

/* This callback gets invoked when the remote channel is deleted */
void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	rpmsg_destroy_ept(rp_ept1);

	int_flag = 1;

}

/* This is the read callback, note we are in a task context when this callback
 is invoked, so kernel primitives can be used freely */
void rpmsg_read_default_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			   void *priv, unsigned long src)
{
	memcpy(r_payload, data, len);
	int_flag = 1;
}

void rpmsg_read_ept1_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			void *priv, unsigned long src)
{
	memcpy(r_payload, data, len);
	int_flag = 1;
}

void rpmsg_read_ept2_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
			void *priv, unsigned long src)
{
	memcpy(r_payload, data, len);
	int_flag = 1;
}

void wait_for_event(void)
{
	while (1) {
		if (int_flag) {

			int_flag = 0;

			break;
		}

		sleep();
	}
}

void send_test_case_report(char *result_string)
{
	struct command *cmd;

	cmd = malloc(sizeof(struct command) + strlen(result_string) + 1);

	cmd->comm_start = CMD_START;
	cmd->comm_code = PRINT_RESULT;

	strcpy(cmd->data, result_string);

	(void)rpmsg_send(app_rp_chnl, cmd,
			 sizeof(struct command) + strlen(result_string) + 1);

	free(cmd);

	wait_for_event();
}

int test_execute_suite(char *firmware_name)
{
	struct remote_proc *proc;
	int status;
	char default_channel[] = "rpmsg-openamp-demo-channel";
	struct command *cmd;
	int i;

	status =
	    remoteproc_init((void *)firmware_name, &proc_table[0], rpmsg_channel_created,
			    rpmsg_channel_deleted, rpmsg_read_default_cb,
			    &proc);

	if (status) {
		printf
		    ("\r\n CRITICAL ERROR: remoteproc_init call for remote context %s failed \r\n",
		     firmware_name);

		return -1;
	}

	status = remoteproc_boot(proc);

	if (status) {
		printf
		    ("\r\n CRITICAL ERROR: remoteproc_boot call for remote context %s failed \r\n",
		     firmware_name);

		return -1;
	}

	/* Wait for channel creation event */
	wait_for_event();

	/* Obtain remote firmware name */

	cmd = malloc(sizeof(struct command));

	cmd->comm_start = CMD_START;
	cmd->comm_code = QUERY_FW_NAME;
	status = rpmsg_send(app_rp_chnl, cmd, sizeof(struct command));

	free(cmd);

	/* Wait to receive firmware name */
	wait_for_event();

	/* Test rpmsg_send API */
	status = test_rpmsg_send(app_rp_chnl);

	if (!status) {
		send_test_case_report("\r\nRPMSG Send Test: Passed\r\n");
	} else {
		send_test_case_report("\r\nRPMSG Send Test: Failed\r\n");
	}

	/* Test rpmsg_send_offchannel API. */
	status =
	    test_rpmsg_send_offchannel(app_rp_chnl, rp_ept1->addr,
				       app_rp_chnl->dst);

	if (!status) {
		send_test_case_report
		    ("\r\nRPMSG Send Offchannel Test: Passed\r\n");
	} else {
		send_test_case_report("\r\nRPMSG Send Offchannel: Failed\r\n");
	}

	status = test_rpmsg_create_ept(app_rp_chnl);

	if (!status) {
		send_test_case_report("\r\nRPMSG Create EPT Test: Passed\r\n");
	} else {
		send_test_case_report("\r\nRPMSG Create EPT Test: Failed\r\n");
	}

	send_test_case_report
	    ("\r\nChannel Deletion. Shutdown would be next\r\n");

	status =
	    test_rpmsg_remote_channel_deletion(app_rp_chnl, default_channel);

	for (i = 0; i < 200000; i++) {
		sleep();
	}

	status = remoteproc_shutdown(proc);
	if (!status) {
		status = remoteproc_deinit(proc);
	}

	/* The multiple life-cycles test has been disabled for remote Linux configuration
	   as it would require manual user input at linux console to complete
	   the rpmsg connection and would be cumbersome for the user. The multiple
	   lifecycles have been tested seperately. */

	/*if(!status)
	   {
	   status = test_remoteproc_multiple_lifecycles(firmware_name);
	   } */

	return status;
}

int test_remoteproc_multiple_lifecycles(char *firmware_name)
{
	int i, j, status;
	struct remote_proc *proc;

	for (i = 0; i < 2; i++) {
		status =
		    remoteproc_init((void *)firmware_name,
				    rpmsg_channel_created,
				    rpmsg_channel_deleted,
				    rpmsg_read_default_cb, &proc);

		if (status) {
			break;
		}

		status = remoteproc_boot(proc);

		if (status) {
			break;
		}

		/* Wait for channel creation event */
		wait_for_event();

		if (!status) {
			status = test_rpmsg_send_impl(app_rp_chnl);
		}

		if (!status) {
			test_rpmsg_remote_channel_deletion(app_rp_chnl,
							   app_rp_chnl->name);
		}

		if (!status) {
			for (j = 0; j < 200000; j++) {
				sleep();
			}

			status = remoteproc_shutdown(proc);
		}
		if (status) {
			break;
		}
		status = remoteproc_deinit(proc);

		if (status) {
			break;
		}
	}

	return status;
}

int test_rpmsg_remote_channel_deletion(struct rpmsg_channel *rpmsg_chnl,
				       char *channel_name)
{
	struct command *cmd;
	int status;
	struct chnl_cmd_data *chnl_data;

	cmd = malloc(sizeof(struct command) + sizeof(struct chnl_cmd_data));

	cmd->comm_code = DELETE_CHNL;
	cmd->comm_start = CMD_START;

	chnl_data = (struct chnl_cmd_data *)cmd->data;

	strncpy(chnl_data->name, channel_name, sizeof(struct chnl_cmd_data));

	/* Let the other side that uninit its resources */
	status =
	    rpmsg_send(rpmsg_chnl, cmd,
		       sizeof(struct command) + sizeof(struct chnl_cmd_data));
	if (status) {
		return status;
	}

	/* Wait for echo back */
	wait_for_event();

	free(cmd);

	return status;
}

int test_rpmsg_create_ept(struct rpmsg_channel *rpmsg_chnl)
{
	struct command *cmd;
	int status = -1, i;
	struct ept_cmd_data *ept_data;
	struct rpmsg_endpoint *test_ept[NUM_TEST_EPS];

	cmd = malloc(sizeof(struct command) + sizeof(struct ept_cmd_data));

	if (!cmd) {
		return status;
	}

	for (i = 0; i < NUM_TEST_EPS; i++) {
		/* Tell the remote to create a new endpoint. */
		cmd->comm_code = CREATE_EPT;
		cmd->comm_start = CMD_START;

		/* Send create endpoint command to remote */
		ept_data = (struct ept_cmd_data *)cmd->data;
		ept_data->dst = EPT_TEST_ADDR + i;
		ept_data->src = EPT_TEST_ADDR + i;

		/* Let the other side know that it needs to create endpoint with the given address */
		status =
		    rpmsg_send(rpmsg_chnl, cmd,
			       sizeof(struct command) +
			       sizeof(struct ept_cmd_data));

		if (!status) {
			/* Wait for ack */
			wait_for_event();
		}

		if (!status) {
			test_ept[i] =
			    rpmsg_create_ept(rpmsg_chnl, rpmsg_read_ept2_cb,
					     RPMSG_NULL, EPT_TEST_ADDR + i);

			if (!test_ept[i]) {
				status = -1;
			}

		}
		if (!status) {
			status =
			    test_rpmsg_send_offchannel_impl(rpmsg_chnl,
							    test_ept[i]->addr,
							    test_ept[i]->addr);
		}

		if (!status) {
			/* Tell the remote to delete the endpoint. */
			cmd->comm_code = DELETE_EPT;
			cmd->comm_start = CMD_START;
			/* Send delete endpoint command to remote */
			ept_data = (struct ept_cmd_data *)cmd->data;
			ept_data->dst = EPT_TEST_ADDR + i;
			ept_data->src = EPT_TEST_ADDR + i;

			/* Let the other side know that it needs to delete endpoint with the given address */
			status =
			    rpmsg_send(rpmsg_chnl, cmd,
				       sizeof(struct command) +
				       sizeof(struct ept_cmd_data));
		}

		if (!status) {
			/* Wait for ack */
			wait_for_event();
		}

		if (!status) {
			rpmsg_destroy_ept(test_ept[i]);
		}
	}

	free(cmd);

	if (status) {
		return -1;
	}

	return status;
}

int test_rpmsg_send_impl(struct rpmsg_channel *rpmsg_chnl)
{
	struct command cmd;
	int status;
	int i, size, idx;

	/* Tell the remote to be prepared for echo payloads. */
	cmd.comm_start = CMD_START;
	cmd.comm_code = START_ECHO;

	status = rpmsg_send(rpmsg_chnl, &cmd, sizeof(struct command));

	if (!status) {
		/* Wait for cmd ack. */
		wait_for_event();
		if (status) {
			return -1;
		}
		for (i = 0, size = PAYLOAD_MIN_SIZE; i < NUM_PAYLOADS;
		     i++, size++) {
			p_payload = malloc(sizeof(struct _payload) + size);

			p_payload->num = i;
			p_payload->size = size;

			/* Setup the buffer with a pattern */
			memset(p_payload->data, 0xA5, size);

			/* Send data to remote side. */
			status =
			    rpmsg_send(rpmsg_chnl, p_payload,
				       sizeof(struct _payload) + size);

			if (status != 0) {
				break;
			}

			/* Wait for echo. */
			wait_for_event();

			/* Validate the data integrity. */
			for (idx = 0; idx < r_payload->size; idx++) {
				if (p_payload->data[idx] !=
				    r_payload->data[idx]) {
					status = -1;
					break;
				}
			}

			if (status != 0) {
				break;
			}

			free(p_payload);

		}
		if (status) {
			return -1;
		}
		cmd.comm_start = CMD_START;
		cmd.comm_code = STOP_ECHO;

		status = rpmsg_send(rpmsg_chnl, &cmd, sizeof(struct command));
		if (status)
			if (status) {
				return -1;
			}

		/* Wait for echo. */
		wait_for_event();

	}

	return status;
}

int test_rpmsg_send(struct rpmsg_channel *rpmsg_chnl)
{
	return test_rpmsg_send_impl(rpmsg_chnl);
}

int test_rpmsg_send_offchannel_impl(struct rpmsg_channel *rpmsg_chnl,
				    unsigned long src, unsigned long dst)
{
	struct command cmd;
	int status;
	int i, size, idx;

	/* Tell the remote to be prepared for echo payloads. */
	cmd.comm_code = START_ECHO;
	cmd.comm_start = CMD_START;
	status = rpmsg_send(rpmsg_chnl, &cmd, sizeof(struct command));

	if (!status) {
		/* Wait for cmd ack. */
		wait_for_event();

		for (i = 0, size = PAYLOAD_MIN_SIZE; i < NUM_PAYLOADS;
		     i++, size++) {
			p_payload = malloc(sizeof(struct _payload) + size);

			p_payload->num = i;
			p_payload->size = size;

			/* Setup the buffer with a pattern */
			memset(p_payload->data, 0xA5, size);

			/* Send data to remote side. */
			status =
			    rpmsg_send_offchannel(app_rp_chnl, src, dst,
						  p_payload,
						  sizeof(struct _payload) +
						  size);

			if (status) {
				break;
			}

			/* Wait for echo. */
			wait_for_event();

			/* Validate the data integrity. */
			for (idx = 0; idx < r_payload->size; idx++) {
				if (p_payload->data[idx] !=
				    r_payload->data[idx]) {
					status = -1;
					break;
				}
			}

			if (status) {
				break;
			}

			free(p_payload);
		}
		cmd.comm_start = CMD_START;
		cmd.comm_code = STOP_ECHO;

		status = rpmsg_send(rpmsg_chnl, &cmd, sizeof(struct command));

		/* Wait for cmd ack. */
		wait_for_event();
	}

	return status;
}

int test_rpmsg_send_offchannel(struct rpmsg_channel *rpmsg_chnl,
			       unsigned long src, unsigned long dst)
{
	return test_rpmsg_send_offchannel_impl(rpmsg_chnl, src, dst);
}
