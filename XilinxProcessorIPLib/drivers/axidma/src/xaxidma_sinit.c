/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxidma_sinit.c
* @addtogroup axidma_v9_12
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xaxidma_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/16/10 First release
* 2.00a jz   08/10/10 Second release, added in xaxidma_g.c, xaxidma_sinit.c,
*                     updated tcl file, added xaxidma_porting_guide.h
* 3.00a jz   11/22/10 Support IP core parameters change
* 5.00a srt  08/29/11 Removed a compiler warning
* 9.5   rsp  11/01/17 Add interface to do config lookup based on base address.
* 9.6   rsp  01/11/18 In LookupConfig use UINTPTR for Baseaddr CR#976392
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaxidma.h"

#if !defined(__BAREMETAL__) && defined(__LIBMETAL__)
#include <dirent.h>
#include <arpa/inet.h>
#else
#include "xparameters.h"
#endif

#if defined (__LIBMETAL__)
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/log.h>
#endif

#if defined(__LIBMETAL__)
u32 XAxiDma_RegisterMetal(XAxiDma *InstancePtr, UINTPTR Baseaddr, struct metal_device **DevicePtr)
{
	s32 Status;

#if !defined(__BAREMETAL__) && defined(__LIBMETAL__)
	char DevName[256];

	memset(DevName, 0, 256);

	Status = metal_devname_from_addr(Baseaddr, DevName);
	if (Status < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to find device name by id ret : %d\n", Status);
		goto RETURN_PATH;
	}

	Status = metal_device_open("platform", DevName, DevicePtr);
	if (Status) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s\n", DevName);
		goto RETURN_PATH;
	}

	/* Set DMA as 64 bit addressing capable */
	metal_device_set_dmacap(*DevicePtr, 64);

	/* Map device IO region */
	InstancePtr->io = metal_device_io_region(*DevicePtr, 0);
	if (InstancePtr->io == NULL) {
		metal_log(METAL_LOG_ERROR, "\n Failed to map region for %s.\n", (*DevicePtr)->name);
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->device = *DevicePtr;

	Status = XST_SUCCESS;
#else
	(void)Baseaddr;

	Status = metal_register_generic_device(*DevicePtr);
	if (Status < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to register generic device : %d\n", Status);
		goto RETURN_PATH;
	}

	Status = metal_device_open("generic", "axidma", DevicePtr);
	if (Status) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s\n", "axidma");
		goto RETURN_PATH;
	}

	InstancePtr->io = metal_device_io_region(*DevicePtr, 0);
	if (InstancePtr->io == NULL) {
		metal_log(METAL_LOG_ERROR, "\n Failed to map region for %s.\n", (*DevicePtr)->name);
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->device = *DevicePtr;

	Status = XST_SUCCESS;
#endif

RETURN_PATH:
	return (u32)Status;
}
#endif

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return
 *		The configuration structure for the device. If the device ID is
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 ******************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfig(u32 DeviceId)
{
#if defined(__LIBMETAL__) && !defined( __BAREMETAL__)
	return (XAxiDma_Config *)NULL;
#else
	extern XAxiDma_Config XAxiDma_ConfigTable[XPAR_XAXIDMA_NUM_INSTANCES];
	XAxiDma_Config *CfgPtr = NULL;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XAXIDMA_NUM_INSTANCES; Index++) {
		if (XAxiDma_ConfigTable[Index].DeviceId == DeviceId) {

			CfgPtr = &XAxiDma_ConfigTable[Index];
			break;
		}
	}

	return (XAxiDma_Config *)CfgPtr;
#endif
}

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance based on base address
 *
 * @param	Baseaddr is the base address of the device to lookup for
 *
 * @return
 *		The configuration structure for the device. If the device base
 *		address is not found,a NULL pointer is returned.
 *
 * @note	None
 *
 ******************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfigBaseAddr(UINTPTR Baseaddr)
{
#if defined (__LIBMETAL__) && !defined(__BAREMETAL__)
	s32 Status;
	s32 Ret;
	u32 Value[2];
	char DevName[256];
	struct metal_device *Deviceptr;
	XAxiDma_Config *CfgPtr = NULL;
	extern XAxiDma_Config XAxiDma_ConfigTable;

	memset(DevName, 0, 256);

	Ret = metal_devname_from_addr(Baseaddr, DevName);
	if (Ret < 0) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to find device name by id ret : %d\n", Ret);
	} else {
		metal_log(METAL_LOG_INFO, "Got device name = %s\n", DevName);
	}

	Status = metal_device_open("platform", DevName, &Deviceptr);
	if (Status) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to open device %s\n", DevName);
		return NULL;
	}

	CfgPtr = &XAxiDma_ConfigTable;

	CfgPtr->DeviceId = 0;

	Status = metal_linux_get_device_property(Deviceptr,
						 "xlnx,sg-length-width",
						 Value, 4);
	if (Status < 0) {
		CfgPtr->SgLengthWidth = 0;
	} else {
		CfgPtr->SgLengthWidth = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr,
						 "xlnx,addrwidth",
						 Value, 4);
	if (Status < 0) {
		CfgPtr->AddrWidth = 0;
	} else {
		CfgPtr->AddrWidth = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr,
						 "xlnx,include-sg", Value, 1);
	if (Status < 0) {
		CfgPtr->HasSg = 0;
	} else {
		CfgPtr->HasSg = 1;
	}

	Status = metal_linux_get_device_property(Deviceptr,
						 "xlnx,sg-include-stscntrl-strm",
						 Value, 1);
	if (Status < 0) {
		CfgPtr->HasStsCntrlStrm = 0;
	} else {
		CfgPtr->HasStsCntrlStrm = 1;
	}

	Status = metal_linux_get_device_property(Deviceptr, "reg", Value, 8);
	if (Status < 0) {
		CfgPtr->BaseAddr = 0;
	} else {
		CfgPtr->BaseAddr = (u32)htonl(Value[1]);
	}

	/* FIXME: Read the MM2S parameters from device tree */
	CfgPtr->HasMm2S = 1;
	CfgPtr->HasMm2SDRE = 1;
	CfgPtr->Mm2SDataWidth = 64;
	CfgPtr->Mm2sNumChannels = 1;

	/* FIXME: Read the S2MM parameters from device tree */
	CfgPtr->HasS2Mm = 1;
	CfgPtr->HasS2MmDRE = 1;
	CfgPtr->S2MmDataWidth = 64;
	CfgPtr->S2MmNumChannels = 1;

	metal_device_close(Deviceptr);

	return (XAxiDma_Config *)CfgPtr;
#else
	extern XAxiDma_Config XAxiDma_ConfigTable[XPAR_XAXIDMA_NUM_INSTANCES];
	XAxiDma_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XAXIDMA_NUM_INSTANCES; Index++) {
		if (XAxiDma_ConfigTable[Index].BaseAddr == Baseaddr) {

			CfgPtr = &XAxiDma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
#endif
}

/** @} */
