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
* @file xsock.c
* @{
*
* This file contains the routines for client side socket creation and
* management. Applicable only for the AIE simulation environment execution
* on linux.
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

/***************************** Include Files *********************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "string.h"

#include "xsock.h"

/************************** Variable Definitions *****************************/
/**< Handle to store the client socket instance pointer */
XSockStr GblSockStr;

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the API to create the client socket connection to the specified host.
*
* @param	SockPtr: Pointer to the client socket data structure.
* @param	Host: Pointer to the host name string.
* @param	PortNum: Port number to which the socket needs to connect to.
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
sint32 XSock_CliCreate(XSockCli *SockPtr, uint8 *Host, uint32 PortNum)
{
	sint32 RetVal;

	GblSockStr.CliSockPtr = SockPtr;

	XAieSim_print("CLIENT: Socket creation, Host:%s, Port:%d\n",
							Host, PortNum);
	memset(SockPtr, sizeof(XSockCli), 0);

	SockPtr->SockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(SockPtr->SockId < 0) {
		XAieSim_print("CLIENT: Socket creation failed\n");
		return XAIESIM_FAILURE;
	}

	SockPtr->Serv = gethostbyname(Host);
	if(SockPtr->Serv == NULL) {
		XAieSim_print("CLIENT: Invalid host\n");
		return XAIESIM_FAILURE;
	}

	SockPtr->ServAddr.sin_family = AF_INET;
	memcpy((uint8 *)SockPtr->Serv->h_addr,
		(uint8 *)&(SockPtr->ServAddr.sin_addr.s_addr),
		SockPtr->Serv->h_length);
	SockPtr->ServAddr.sin_port = htons(PortNum);

	RetVal = connect(SockPtr->SockId,
				(struct sockaddr *)&SockPtr->ServAddr,
				sizeof(SockPtr->ServAddr));
	if(RetVal < 0) {
		XAieSim_print("CLIENT: Socket connection failed\n");
		return XAIESIM_FAILURE;
	}

	return XAIESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API is to write specified number of bytes to the client socket.
*
* @param	SockPtr: Pointer to the client socket data structure.
* @param	Buffer: Pointer to the data buffer.
* @param	Size: Number of bytes to be written.
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
sint32 XSock_CliWrite(XSockCli *SockPtr, uint8 *Buffer, uint32 Size)
{
	sint32 Count;

	Count = write(SockPtr->SockId, Buffer, Size);
	if(Count < 0) {
		XAieSim_print("CLIENT: Socket write message failed\n");
		return XAIESIM_FAILURE;
	}
	return XAIESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API is to read specified number of bytes from the client socket.
*
* @param	SockPtr: Pointer to the client socket data structure.
* @param	Buffer: Pointer to the data buffer.
* @param	Size: Maximum number of bytes to read.
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
sint32 XSock_CliRead(XSockCli *SockPtr, uint8 *Buffer, uint32 Size)
{
	sint32 Count;

	Count = read(SockPtr->SockId, Buffer, Size);
	if(Count < 0) {
		XAieSim_print("CLIENT: Message received from server failed\n");
		return XAIESIM_FAILURE;
	}
	return XAIESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API is to close the client socket connection.
*
* @param	SockPtr: Pointer to the client socket data structure.
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
sint32 XSock_CliClose(XSockCli *SockPtr)
{
	sint32 RetVal;

	RetVal = close(SockPtr->SockId);
	if(RetVal < 0) {
		XAieSim_print("CLIENT: Socket close failed\n");
		return XAIESIM_FAILURE;
	}
	return XAIESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	Addr: Address to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
uint32 XSock_Read32(uint64_t Addr)
{
	uint8 CmdBuf[XSOCK_MEMIO_CMDBUF_LEN32];
	uint8 CmdData[XSOCK_MEMIO_CMDDATA_LEN32];
	uint8 RespBuf[XSOCK_MEMIO_CMDDATA_LEN32];
	sint32 Status;

	strcpy(CmdBuf, "R ");
	strcpy(RespBuf, XSOCK_MEMIO_RESP_DEFAULT);

	/*
	 * Frame the command buffer to be sent out on TCP socket.
	 * Command syntax: R <Addr> i.e., R 0XXXXXXXXX
	 */
	sprintf(CmdData, "0X%016lx ", Addr);
	strcat(CmdBuf, CmdData);

	XAieSim_print("%s\n", CmdBuf);
	/* Send the read command over the TCP socket */
	Status = XSock_CliWrite(GblSockStr.CliSockPtr, CmdBuf,
							strlen(CmdBuf));
	if(Status == XAIESIM_SUCCESS) {
		/* Read from socket to get the data for the earlier read */
		Status = XSock_CliRead(GblSockStr.CliSockPtr,
							RespBuf, 11);
		if(Status != XAIESIM_SUCCESS) {
			XAieSim_print("Read failed: socket read command err\n");
		}
	} else {
		XAieSim_print("Read failed: socket write command error\n");
	}

	return ((uint32)strtol(RespBuf, NULL, 0));
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 128b data from the specified address.
*
* @param	Addr: Address to read from.
* @param	Data: Pointer to the 128-bit buffer to store the read data.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XSock_Read128(uint64_t Addr, uint32 *Data)
{
	/* TODO: AIE sim no support yet */
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XSock_Write32(uint64_t Addr, uint32 Data)
{
	uint8 CmdBuf[XSOCK_MEMIO_CMDBUF_LEN32];
	uint8 CmdData[XSOCK_MEMIO_CMDDATA_LEN32];

        /*
         * WORK_AROUND: AIE simulator PL interface regs WO error.
         * Check if reg state needs to be maintained locally
         */

	strcpy(CmdBuf, "W ");

	/*
	 * Frame the command buffer to be sent out on TCP socket.
	 * Command syntax: W <Addr> <Data> i.e., W 0XXXXXXXXX 0XYYYYYYYY
	 */
	sprintf(CmdData, "0X%016lx ", Addr);
	strcat(CmdBuf, CmdData);

	sprintf(CmdData, "0X%08x", Data);
	strcat(CmdBuf, CmdData);

	XAieSim_print("%s\n", CmdBuf);
	/* Send the write command over the TCP socket */
	(void)XSock_CliWrite(GblSockStr.CliSockPtr, CmdBuf,
							strlen(CmdBuf));
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 128bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: Pointer to the 128-bit data buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XSock_Write128(uint64_t Addr, uint32 *Data)
{
	uint8 CmdBuf[XSOCK_MEMIO_CMDBUF_LEN64];
	uint8 CmdData[XSOCK_MEMIO_CMDDATA_LEN64];
	
	strcpy(CmdBuf, "P ");

	/*
	 * Frame the command buffer to be sent out on TCP socket.
	 * Command syntax: P 0XXXXXXXXX 0XYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
	 */
	sprintf(CmdData, "0X%016lx ", Addr);
	strcat(CmdBuf, CmdData);

	sprintf(CmdData, "0X%08x%08x%08x%08x ", Data[0], Data[1],
						Data[2], Data[3]);
	strcat(CmdBuf, CmdData);

	XAieSim_print("%s\n", CmdBuf);
	/* Send the write command over the TCP socket */
	(void)XSock_CliWrite(GblSockStr.CliSockPtr, CmdBuf,
							strlen(CmdBuf));
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 128bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: Pointer to the 128-bit data buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XSock_WriteCmd(uint8 Command, uint8 ColId, uint8 RowId, uint32 CmdWd0,
						uint32 CmdWd1, uint8 *CmdStr)
{
	uint8 CmdBuf[XSOCK_CMDIO_CMDBUF_LEN];

	switch(Command) {
		case XAIESIM_CMDIO_CMD_SETSTACK:
			sprintf(CmdBuf, "C STACK %d %d 0X%08x 0x%08x",
				ColId, RowId, CmdWd0, CmdWd1);
			break;

		case XAIESIM_CMDIO_CMD_LOADSYM:
			sprintf(CmdBuf, "C LOAD_SYMBOLS %s %d %d",
				CmdStr, ColId, RowId);
			break;

		default:
			XAieSim_print("ERROR: Invalid command\n");
			break;
	}

	XAieSim_print("%s\n", CmdBuf);

	/* Send the command over the TCP socket */
	(void)XSock_CliWrite(GblSockStr.CliSockPtr, CmdBuf,
							strlen(CmdBuf));
}

/** @} */

