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
* @file xsock.h
* @{
*
* This file contains the prototypes of the variables and functions for the
* client socket creation and management. Applicable only for the AIE simulation
* environment execution on linux.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/27/2018  Initial creation
* 1.1  Naresh  06/13/2018  Fixed CR#1003905
* 1.2  Naresh  07/11/2018  Updated copyright info
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
#ifndef XSOCK_H
#define XSOCK_H

/***************************** Include Files *********************************/
#include <sys/socket.h>
#include <netinet/in.h>

#include "xaiesim.h"

/***************************** Macro Definitions *****************************/
#define XSOCK_MEMIO_CMDBUF_LEN32		32U
#define XSOCK_MEMIO_CMDBUF_LEN64		64U

#define XSOCK_MEMIO_CMDDATA_LEN32		32U
#define XSOCK_MEMIO_CMDDATA_LEN64		64U

#define XSOCK_MEMIO_RESP_DEFAULT		"0xDEADDEAD"

#define XSOCK_CMDIO_CMDBUF_LEN			100U

#define XSOCK_CMDIO_CMD_SETSTACK		0U
#define XSOCK_CMDIO_CMD_LOADSYM			1U

/************************** Variable Definitions *****************************/
/**
 * This typedef contains the socket attributes which make the Client socket instance.
 * User need to allocate memory for this instance and the pointer of the same is passed
 * to the other client socket APIs.
 */
typedef struct {
	sint32 SockId;			/**< Socket ID */
	struct sockaddr_in ServAddr;	/**< Server socket address structure */
	struct hostent *Serv;		/**< Server attributes structure */
} XSockCli;

/**
 * This typedef contains pointer to the client socket instance for the same to be used
 * in the memory IO functions.
 */
typedef struct {
	XSockCli *CliSockPtr;	/**< Client socket instance pointer */
} XSockStr;

/************************** Function Prototypes  *****************************/
sint32 XSock_CliCreate(XSockCli *SockPtr, uint8 *Host, uint32 Portnum);
sint32 XSock_CliWrite(XSockCli *SockPtr, uint8 *Buffer, uint32 Size);
sint32 XSock_CliRead(XSockCli *SockPtr, uint8 *Buffer, uint32 Size);
sint32 XSock_CliClose(XSockCli *SockPtr);
uint32 XSock_Read32(uint64_t Addr);
void XSock_Read128(uint64_t Addr, uint32 *Data);
void XSock_Write32(uint64_t Addr, uint32 Data);
void XSock_Write128(uint64_t Addr, uint32 *Data);
void XSock_WriteCmd(uint8 Command, uint8 ColId, uint8 RowId, uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr);

#endif		/* end of protection macro */
/** @} */

