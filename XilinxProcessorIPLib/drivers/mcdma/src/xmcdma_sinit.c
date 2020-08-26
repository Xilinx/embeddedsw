/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmcdma_sinit.c
* @addtogroup mcdma_v1_5
* @{
*
* This file contains static initialization methods for Xilinx MCDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* 1.2   mj      05/03/18 Implemented XMcdma_LookupConfigBaseAddr() to lookup
*                        configuration based on base address.
* 1.5   vak     02/08/20 Add libmetal support for mcdma.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xmcdma.h"

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

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

#if defined(__LIBMETAL__)
u32 XMcdma_RegisterMetal(XMcdma *InstancePtr, UINTPTR Baseaddr, struct metal_device **DevicePtr)
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
	(void) Baseaddr;

	Status = metal_register_generic_device(*DevicePtr);
	if (Status < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to register generic device : %d\n", Status);
		goto RETURN_PATH;
	}

	Status = metal_device_open("generic", "mcdma", DevicePtr);
	if (Status) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s\n", "mcdma");
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
*
* XMcdma_LookupConfig returns a reference to an XMcdma_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xmcdma_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XMcdma_Config *XMcdma_LookupConfig(u16 DeviceId)
{
#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
	return (XMcdma_Config *)NULL;
#else
	extern XMcdma_Config XMcdma_ConfigTable[XPAR_XMCDMA_NUM_INSTANCES];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XMCDMA_NUM_INSTANCES);
								Index++) {
		if (XMcdma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}

	return (XMcdma_Config *)CfgPtr;
#endif
}

/*****************************************************************************/
/**
*
* XMcdma_LookupConfigBaseAddr returns a reference to an XMcdma_Config structure
* based on base address. The return value will refer to an entry in the device
* configuration table defined in the xmcdma_g.c file.
*
* @param	Baseaddr is the base address of the device to lookup for
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to Baseaddr, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XMcdma_Config *XMcdma_LookupConfigBaseAddr(UINTPTR Baseaddr)
{
#if defined (__LIBMETAL__) && !defined(__BAREMETAL__)
	extern XMcdma_Config XMcdma_ConfigTable;
	XMcdma_Config *CfgPtr = NULL;
	s32 Status;
	s32 Ret;
	u32 Value[2];
	char DevName[256];
	struct metal_device *Deviceptr;

	memset(DevName, 0, 256);

	Ret = metal_devname_from_addr(Baseaddr, DevName);
	if (Ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to find device name by id ret : %d\n", Ret);
	} else {
		metal_log(METAL_LOG_INFO, "Got device name = %s\n", DevName);
	}

	Status = metal_device_open("platform", DevName, &Deviceptr);
	if (Status) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s\n", DevName);
		return NULL;
	}

	CfgPtr = &XMcdma_ConfigTable;

	CfgPtr->DeviceId = 0;

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,sg-length-width", Value, 4);
	if (Status < 0) {
		CfgPtr->MaxTransferlen = 0;
	} else {
		CfgPtr->MaxTransferlen = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,addrwidth", Value, 4);
	if (Status < 0) {
		CfgPtr->AddrWidth = 0;
	} else {
		CfgPtr->AddrWidth = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,sg-include-stscntrl-strm", Value, 4);
	if (Status < 0) {
		CfgPtr->HasStsCntrlStrm = 0;
	} else {
		CfgPtr->HasStsCntrlStrm = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "reg", Value, 8);
	if (Status < 0) {
		CfgPtr->BaseAddress = 0;
	} else {
		CfgPtr->BaseAddress = (u32)htonl(Value[1]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,enable-single-intr", Value, 4);
	if (Status < 0) {
		CfgPtr->Has_SingleIntr = 0;
	} else {
		CfgPtr->Has_SingleIntr = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,include-mm2s", Value, 4);
	if (Status < 0) {
		CfgPtr->HasMM2S = 0;
	} else {
		CfgPtr->HasMM2S = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,include-mm2s-dre", Value, 4);
	if (Status < 0) {
		CfgPtr->HasS2MMDRE = 0;
	} else {
		CfgPtr->HasS2MMDRE = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,num-mm2s-channels", Value, 4);
	if (Status < 0) {
		CfgPtr->TxNumChannels = 0;
	} else {
		CfgPtr->TxNumChannels = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,include-s2mm", Value, 4);
	if (Status < 0) {
		CfgPtr->HasS2MM = 0;
	} else {
		CfgPtr->HasS2MM = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,include-s2mm-dre", Value, 4);
	if (Status < 0) {
		CfgPtr->HasS2MMDRE = 0;
	} else {
		CfgPtr->HasS2MMDRE = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,num-s2mm-channels", Value, 4);
	if (Status < 0) {
		CfgPtr->RxNumChannels = 0;
	} else {
		CfgPtr->RxNumChannels = (u32)htonl(Value[0]);
	}

	Status = metal_linux_get_device_property(Deviceptr, "xlnx,sg-use-stsapp-length", Value, 4);
	if (Status < 0) {
		CfgPtr->HasRxLength = 0;
	} else {
		CfgPtr->HasRxLength = (u32)htonl(Value[0]);
	}

	/* FIXME: Read DataWidth and CacheCoherent from device tree */
	CfgPtr->MM2SDataWidth = 64;
	CfgPtr->S2MMDataWidth = 64;
	CfgPtr->IsTxCacheCoherent = 1;
	CfgPtr->IsRxCacheCoherent = 1;

	metal_device_close(Deviceptr);

	return (XMcdma_Config *)CfgPtr;
#else
	extern XMcdma_Config XMcdma_ConfigTable[XPAR_XMCDMA_NUM_INSTANCES];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XMCDMA_NUM_INSTANCES);
								Index++) {
		if (XMcdma_ConfigTable[Index].BaseAddress == Baseaddr) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
#endif
}
/** @} */
