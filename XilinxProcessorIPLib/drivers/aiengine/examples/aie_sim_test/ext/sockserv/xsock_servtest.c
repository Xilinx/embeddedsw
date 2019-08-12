/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xsock_servtest.c
* @{
*
* This file contains the entry point for the server socket connection test.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  02/19/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/************************** Constant Definitions *****************************/
#define XAIE_BUFLEN 50	/**< Buffer length for socket communication */

/************************** Variable Definitions *****************************/
/**< Server socket receive data success message */
char rcvmsgpass[] = "SERVER: Receive message successful";
/**< Server socket receive data fail message */
char rcvmsgfail[] = "SERVER: Receive message failed";

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the main entry point for the server socket test.
*
* @param	argc : Number of command line arguments.
* @param	argv[0] : File name.
* @param	argv[1] : Port number for the socket connection.
*
* @return	0 on success, else 1.
*
* @note		None.
*
*******************************************************************************/
int main(int argc, char *argv[])
{
	int sockid, newsockid, clilen, retval, count;
	unsigned int data = 0x5AFE0000;
	char buffer[XAIE_BUFLEN] = "";
	struct sockaddr_in servaddr, cliaddr;

	sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockid < 0) {
		printf("SERVER: Socket creation failed\n");
		return 1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(atoi(argv[1]));

	retval = bind(sockid, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if(retval < 0) {
		printf("SERVER: Socket binding failed\n");
		return 1;
	}

	retval = listen(sockid, 10);
	if(retval < 0) {
		printf("SERVER: Socket listening failed\n");
		return 1;
	}

	clilen = sizeof(cliaddr);
	newsockid = accept(sockid, (struct sockaddr *)&cliaddr, &clilen);
	if(newsockid < 0) {
		printf("SERVER: Socket acceptance failed\n");
		return 1;
	}

	while(1) {
		count = read(newsockid, buffer, XAIE_BUFLEN);
		if(count < 1) {
			printf("SERVER: Socket read message failed\n");
		} else {
			printf("SERVER: Msg from client: %s\n", buffer);
			if(strstr(buffer, "R ") != NULL) {
				/* Read data command received from client */
				data += 4;
				sprintf(buffer, "0x%08x", data);
				(void)write(newsockid, buffer, 11);
			}
			if(strstr(buffer, "END OF COMMS") != NULL) {
				break;
			}
		}
	}

	close(sockid);
	return 0;
}

/** @} */
