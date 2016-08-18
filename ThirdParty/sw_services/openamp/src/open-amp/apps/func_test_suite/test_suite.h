/*
 * m_ept_func_test.h
 *
 *  Created on: Mar 14, 2014
 *      Author: etsam
 */

#ifndef M_EPT_FUNC_TEST_H_
#define M_EPT_FUNC_TEST_H_

OPENAMP_PACKED_BEGIN
struct command {
	unsigned int comm_start;
	unsigned int comm_code;
	char data[0];
} OPENAMP_PACKED_END;

struct ept_cmd_data {
	unsigned int src;
	unsigned int dst;
};

struct chnl_cmd_data {
	char name[32];
};

/* Command Codes */

#define CREATE_EPT      0x00000000
#define DELETE_EPT      0x00000001
#define CREATE_CHNL     0x00000002
#define DELETE_CHNL     0x00000003
#define START_ECHO      0x00000004
#define STOP_ECHO       0x00000005
#define QUERY_FW_NAME   0x00000006
#define PRINT_RESULT    0x00000007

#define CMD_START       0xEF56A55A

/* Define the main task's stack size */
#define STACK_SIZE      (NU_MIN_STACK_SIZE * 400)

/* Define the main task's priority */
#define TASK_PRIORITY   26

/* Define the main task's time slice */
#define TASK_SLICE      20

#define EPT_TEST_ADDR      59

#define NUM_TEST_EPS       4

#define MAX_RPMSG_BUFF_SIZE 512
#define PAYLOAD_MIN_SIZE    1
#define PAYLOAD_MAX_SIZE    (MAX_RPMSG_BUFF_SIZE - 24)
#define NUM_PAYLOADS        (PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

OPENAMP_PACKED_BEGIN
struct _payload {
	unsigned long num;
	unsigned long size;
	char data[0];
} OPENAMP_PACKED_END;

#endif				/* M_EPT_FUNC_TEST_H_ */
