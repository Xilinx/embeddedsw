/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_socket.c
* @{
*
* This file contains the low level layer IO interface for socket io backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus    08/12/2021  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __AIESOCKET__

#define  _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif /* __AIESOCKET__ */

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"

/***************************** Macro Definitions *****************************/
#define XAIE_IO_SOCKET_CMDBUFSIZE	48U
#define XAIE_IO_SOCKET_RDBUFSIZE	11U /* "0xDEADBEEF\n" */

/****************************** Type Definitions *****************************/
#ifdef __AIESOCKET__

typedef struct XAie_SocketIO {
	u64 BaseAddr;
	u64 NpiBaseAddr;
	int SocketFd;
} XAie_SocketIO;

#endif /* __AIESOCKET__ */
/************************** Function Definitions *****************************/
#ifdef __AIESOCKET__

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_Finish(void *IOInst)
{
	XAie_SocketIO *SocketIOInst = (XAie_SocketIO *)IOInst;

	close(SocketIOInst->SocketFd);
	free(IOInst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	None.
*
* @return	None.
*
* @note		The global IO instance is a singleton and any further attempt
* to initialize just increments the reference count. Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_Init(XAie_DevInst *DevInst)
{
	XAie_SocketIO *IOInst;
	struct addrinfo hints, *slist, *p;
	u32 FileSize;
	char *PortNum;
	int ret;
	int SocketFd;
	FILE *Fd;

	IOInst = (XAie_SocketIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Socket backend init failed. failed to allocated "
				"memory\n");
		return XAIE_ERR;
	}

	/* Get port number from mesim_output/mesimulator_aximm_port */
	Fd = fopen("./mesim_output/mesimulator_aximm_port", "r");
	if(Fd == NULL){
		XAIE_ERROR("Unable to open file to read port number of "
				"simulator\n");
		return XAIE_ERR;
	}

	ret = fseek(Fd, 0L, SEEK_END);
	if(ret != 0U) {
		fclose(Fd);
		XAIE_ERROR("Failed to get end of file\n");
		return XAIE_ERR;
	}

	FileSize = ftell(Fd);
	rewind(Fd);

	PortNum = (char *)malloc(FileSize + 1U);
	if(PortNum == NULL) {
		fclose(Fd);
		XAIE_ERROR("Memory allocation failedi. Unable to read port"
				" number\n");
		return XAIE_ERR;
	}

	ret = fread((void *)PortNum, FileSize, 1U, Fd);
	if(ret == 0U) {
		fclose(Fd);
		free(PortNum);
		XAIE_ERROR("Failed to read port number from file\n");
		return XAIE_ERR;
	}

	fclose(Fd);
	PortNum[FileSize] = '\0';
	printf("[AIE INFO]: Connecting to simulator - localhost:%s\n", PortNum);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	/*
	 * TODO: get address from env variable instead of hardcoding it to
	 * localhost.
	 */
	ret = getaddrinfo("localhost", PortNum, &hints, &slist);
	if(ret != 0) {
		XAIE_ERROR("get addr info failed. ec %s\n", gai_strerror(ret));
		return XAIE_ERR;
	}

	for(p = slist; p != NULL; p = p->ai_next) {

		SocketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(SocketFd < 0)
			continue;

		ret = connect(SocketFd, p->ai_addr, p->ai_addrlen);
		if(ret != -1)
			break;

		close(SocketFd);
	}

	if(p == NULL) {
		XAIE_ERROR("failed to connect to sim\n");
		return XAIE_ERR;
	}

	IOInst->SocketFd = SocketFd;
	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;
	DevInst->IOInst = IOInst;

	freeaddrinfo(slist);
	free(PortNum);
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_SocketIO *SocketIOInst = (XAie_SocketIO *)IOInst;
	char CmdBuf[XAIE_IO_SOCKET_CMDBUFSIZE];
	size_t Len;

	sprintf(CmdBuf, "W 0X%016lX 0X%08X\n", SocketIOInst->BaseAddr + RegOff,
			Value);
	Len = write(SocketIOInst->SocketFd, CmdBuf, strlen(CmdBuf));
	if(Len != strlen(CmdBuf)) {
		XAIE_ERROR("Failed to submit socket command: %s\n", CmdBuf);
		return XAIE_ERR;
	}

	XAIE_DBG("SEND: %s", CmdBuf);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_SocketIO *SocketIOInst = (XAie_SocketIO *)IOInst;
	char CmdBuf[XAIE_IO_SOCKET_CMDBUFSIZE];
	char RdBuf[XAIE_IO_SOCKET_RDBUFSIZE];
	size_t Len;
	int Ret;

	sprintf(CmdBuf, "R 0X%016lX\n", SocketIOInst->BaseAddr + RegOff);
	Len = write(SocketIOInst->SocketFd, CmdBuf, strlen(CmdBuf));
	if(Len != strlen(CmdBuf)) {
		XAIE_ERROR("Failed to submit socket command: %s\n", CmdBuf);
		return XAIE_ERR;
	}

	XAIE_DBG("SEND: %s", CmdBuf);

	Ret = read(SocketIOInst->SocketFd, RdBuf, XAIE_IO_SOCKET_RDBUFSIZE);
	if(Ret == -1) {
		XAIE_ERROR("Failed to read from socket\n");
		return XAIE_ERR;
	}

	XAIE_DBG("RCVD: %s", RdBuf);
	*Data = (u32)strtol(RdBuf, NULL, 0);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write masked 32bit data to the specified
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	AieRC RC;
	u32 RegVal;

	RC = XAie_SocketIO_Read32(IOInst, RegOff, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	RegVal &= ~Mask;
	RegVal |= Value;

	return XAie_SocketIO_Write32(IOInst, RegOff, RegVal);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll an address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK or XAIE_ERR.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	AieRC Ret = XAIE_ERR;
	u32 Count, MinTimeOutUs, RegVal;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = ((u64)TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		XAie_SocketIO_Read32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			return XAIE_OK;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	XAie_SocketIO_Read32(IOInst, RegOff, &RegVal);
	if((RegVal & Mask) == Value) {
		Ret = XAIE_OK;
	}

	return Ret;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a block of data to aie.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to the data buffer.
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_SocketIO_Write32(IOInst, RegOff + i * 4U, *Data);
		Data++;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize a chunk of aie address space with
* a specified value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Data to initialize a chunk of aie address space..
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_SocketIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_SocketIO_Write32(IOInst, RegOff+ i * 4U, Data);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to write 32 bit value to NPI register address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: NPI register offset
* @param	RegVal: Value to write to register
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_SocketIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	XAie_SocketIO *SocketIOInst = (XAie_SocketIO *)IOInst;
	char CmdBuf[XAIE_IO_SOCKET_CMDBUFSIZE];
	size_t Len;

	sprintf(CmdBuf, "W 0X%016lX 0X%08X\n",
			SocketIOInst->NpiBaseAddr + RegOff, RegVal);
	Len = write(SocketIOInst->SocketFd, CmdBuf, strlen(CmdBuf));
	if(Len != strlen(CmdBuf)) {
		XAIE_ERROR("Failed to submit socket command: %s\n", CmdBuf);
		return;
	}

	XAIE_DBG("SEND NPI: %s", CmdBuf);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified NPI
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_SocketIO_NpiRead32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_SocketIO *SocketIOInst = (XAie_SocketIO *)IOInst;
	char CmdBuf[XAIE_IO_SOCKET_CMDBUFSIZE];
	char RdBuf[XAIE_IO_SOCKET_RDBUFSIZE];
	size_t Len;
	int Ret;

	sprintf(CmdBuf, "R 0X%016lX\n", SocketIOInst->NpiBaseAddr + RegOff);
	Len = write(SocketIOInst->SocketFd, CmdBuf, strlen(CmdBuf));
	if(Len != strlen(CmdBuf)) {
		XAIE_ERROR("Failed to submit socket command: %s\n", CmdBuf);
		return XAIE_ERR;
	}

	XAIE_DBG("SEND NPI: %s", CmdBuf);

	Ret = read(SocketIOInst->SocketFd, RdBuf, XAIE_IO_SOCKET_RDBUFSIZE);
	if(Ret == -1) {
		XAIE_ERROR("Failed to read from socket\n");
		return XAIE_ERR;
	}

	XAIE_DBG("RCVD NPI: %s", RdBuf);
	*Data = (u32)strtol(RdBuf, NULL, 0);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll a NPI address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK or XAIE_ERR.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_SocketIO_NpiMaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	AieRC Ret = XAIE_ERR;
	u32 Count, MinTimeOutUs, RegVal;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = ((u64)TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		_XAie_SocketIO_NpiRead32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			return XAIE_OK;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	_XAie_SocketIO_NpiRead32(IOInst, RegOff, &RegVal);
	if((RegVal & Mask) == Value) {
		Ret = XAIE_OK;
	}

	return Ret;
}

static AieRC XAie_SocketIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)DevInst;
	switch(Op) {
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			XAie_ShimDmaBdArgs *BdArgs =
				(XAie_ShimDmaBdArgs *)Arg;
			for(u8 i = 0; i < BdArgs->NumBdWords; i++) {
				XAie_SocketIO_Write32(IOInst,
						BdArgs->Addr + i * 4,
						BdArgs->BdWords[i]);
			}
			break;
		}
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_SocketIO_NpiWrite32(IOInst, Req->NpiRegOff,
					Req->Val);
			break;
		}
		case XAIE_BACKEND_OP_NPIMASKPOLL32:
		{
			XAie_BackendNpiMaskPollReq *Req = Arg;

			return _XAie_SocketIO_NpiMaskPoll(IOInst,
					Req->NpiRegOff, Req->Mask, Req->Val,
					Req->TimeOutUs);
		}
		case XAIE_BACKEND_OP_REQUEST_RESOURCE:
			return _XAie_RequestRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_RELEASE_RESOURCE:
			return _XAie_ReleaseRscCommon(Arg);
		case XAIE_BACKEND_OP_FREE_RESOURCE:
			return _XAie_FreeRscCommon(Arg);
		case XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE:
			return _XAie_RequestAllocatedRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_GET_RSC_STAT:
			return _XAie_GetRscStatCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_REQUEST_TILES:
			return _XAie_PrivilegeRequestTiles(DevInst,
					(XAie_BackendTilesArray *)Arg);
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			return _XAie_PrivilegeTeardownPart(DevInst);
		default:
			XAIE_ERROR("Socket backend does not support operation "
					"%d\n", Op);
			return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return XAIE_OK;
}

#else

static AieRC XAie_SocketIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_SocketIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with socket "
			"backend (__AIESOCKET__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_SocketIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_SocketIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return XAIE_ERR;
}

static AieRC XAie_SocketIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_SocketIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;

	return XAIE_ERR;
}

static AieRC XAie_SocketIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_SocketIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_SocketIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIESOCKET__ */

static XAie_MemInst* XAie_SocketMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_SocketMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SocketMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SocketMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SocketMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_SocketMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_SocketIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;

	return XAIE_ERR;
}

const XAie_Backend SocketBackend =
{
	.Type = XAIE_IO_BACKEND_SOCKET,
	.Ops.Init = XAie_SocketIO_Init,
	.Ops.Finish = XAie_SocketIO_Finish,
	.Ops.Write32 = XAie_SocketIO_Write32,
	.Ops.Read32 = XAie_SocketIO_Read32,
	.Ops.MaskWrite32 = XAie_SocketIO_MaskWrite32,
	.Ops.MaskPoll = XAie_SocketIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_SocketIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_SocketIO_BlockSet32,
	.Ops.CmdWrite = XAie_SocketIO_CmdWrite,
	.Ops.RunOp = XAie_SocketIO_RunOp,
	.Ops.MemAllocate = XAie_SocketMemAllocate,
	.Ops.MemFree = XAie_SocketMemFree,
	.Ops.MemSyncForCPU = XAie_SocketMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_SocketMemSyncForDev,
	.Ops.MemAttach = XAie_SocketMemAttach,
	.Ops.MemDetach = XAie_SocketMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.SubmitTxn = NULL,
};

/** @} */
