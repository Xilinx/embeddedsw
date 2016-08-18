/*
 * temp.c
 *
 *  Created on: Oct 4, 2014
 *      Author: etsam
 */

/*
 * Test application that data integraty of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

/* Shutdown message ID */
#define SHUTDOWN_MSG	0xEF56A55A

struct _payload {
	unsigned long num;
	unsigned long size;
	char data[];
};

static int fd, err_cnt;

struct _payload *i_payload;
struct _payload *r_payload;

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (512 - RPMSG_HEADER_LEN)
#define PAYLOAD_MIN_SIZE	1
#define PAYLOAD_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 24)
#define NUM_PAYLOADS		(PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

int main(int argc, char *argv[])
{
	int flag = 1;
	int shutdown_msg = SHUTDOWN_MSG;
	int cmd, ret, i, j, expect_rnum, rnum;
	int size, bytes_rcvd, bytes_sent;
	err_cnt = 0;

	printf("\r\n Echo test start \r\n");

	printf("\r\n Open rpmsg dev! \r\n");

	fd = open("/dev/rpmsg0", O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		perror("Failed to open rpmsg file /dev/rpmsg0.");
		return -1;
	}

	printf("\r\n Query internal info .. \r\n");

	ioctl(fd, RPMSG_GET_KFIFO_SIZE, &size);

	printf(" rpmsg kernel fifo size = %u \r\n", size);

	ioctl(fd, RPMSG_GET_FREE_SPACE, &size);

	printf(" rpmsg kernel fifo free space = %u \r\n", size);

	i_payload =
	    (struct _payload *)malloc(2 * sizeof(unsigned long) +
				      PAYLOAD_MAX_SIZE);
	r_payload =
	    (struct _payload *)malloc(2 * sizeof(unsigned long) +
				      PAYLOAD_MAX_SIZE);

	if (i_payload == 0 || r_payload == 0) {
		printf("ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}

	while (flag == 1) {
		printf("\r\n **************************************** \r\n");
		printf(" Please enter command and press enter key\r\n");
		printf(" **************************************** \r\n");
		printf(" 1 - Send data to remote core, retrieve the echo");
		printf(" and validate its integrity .. \r\n");
		printf(" 2 - Quit this application .. \r\n");
		printf(" CMD>");
		ret = scanf("%d", &cmd);
		if (!ret) {
			while (1) {
				if (getchar() == '\n')
					break;
			}

			printf("\r\n invalid command\r\n");
			continue;
		}

		if (cmd == 1) {
			expect_rnum = 0;
			rnum = 0;
			for (i = 0, size = PAYLOAD_MIN_SIZE; i < NUM_PAYLOADS;
			     i++, size++) {
				i_payload->num = i;
				i_payload->size = size;

				/* Mark the data buffer. */
				memset(&(i_payload->data[0]), 0xA5, size);

				printf("\r\n sending payload number");
				printf(" %d of size %d \r\n", i_payload->num,
				       (2 * sizeof(unsigned long)) + size);

				bytes_sent = write(fd, i_payload,
						   (2 * sizeof(unsigned long)) +
						   size);

				if (bytes_sent <= 0) {
					printf("\r\n Error sending data");
					printf(" .. \r\n");
					break;
				}
				expect_rnum++;
				printf("echo test: sent : %d\n", bytes_sent);

				r_payload->num = 0;
				bytes_rcvd = read(fd, r_payload,
						  (2 * sizeof(unsigned long)) +
						  PAYLOAD_MAX_SIZE);
				while (bytes_rcvd <= 0) {
					usleep(10000);
					bytes_rcvd = read(fd, r_payload,
							  (2 *
							   sizeof(unsigned
								  long)) +
							  PAYLOAD_MAX_SIZE);
				}
				printf(" received payload number ");
				printf("%d of size %d \r\n", r_payload->num,
				       bytes_rcvd);
				rnum = r_payload->num + 1;

				/* Validate data buffer integrity. */
				for (i = 0; i < r_payload->size; i++) {

					if (r_payload->data[i] != 0xA5) {
						printf(" \r\n Data corruption");
						printf(" at index %d \r\n", i);
						err_cnt++;
						break;
					}
				}
				bytes_rcvd = read(fd, r_payload,
						  (2 * sizeof(unsigned long)) +
						  PAYLOAD_MAX_SIZE);

			}

			printf("\r\n **********************************");
			printf("****\r\n");
			printf("\r\n Test Results: Error count = %d\r\n",
			       err_cnt);
			printf("\r\n **********************************");
			printf("****\r\n");
		} else if (cmd == 2) {
			flag = 0;
			/* Send shutdown message to remote */
			write(fd, &shutdown_msg, sizeof(int));
			sleep(1);
			printf("\r\n Quitting application .. \r\n");
			printf(" Echo test end \r\n");
		} else {
			printf("\r\n invalid command! \r\n");
		}
	}

	free(i_payload);
	free(r_payload);

	close(fd);

	return 0;
}
