/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_selftest.c
 *
 * This file demonstrates the example to do selftest on the device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/

#include <metal/device.h>

#include <dirent.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <metal/sys.h>
#include <metal/irq.h>
#include <metal/sleep.h>
#include "metal/alloc.h"
#include <metal/device.h>
#include <metal/io.h>

#include "xaxidma.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

//int AxiDMASelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;

s32 XDMA_GetDeviceNameByDeviceId(char *DevNamePtr, u16 DevId);
XAxiDma_Config *XDMA_LookupConfig(struct metal_device **Deviceptr, u16 DeviceId);
u32 XAxiDma_RegisterMetal(XAxiDma *InstancePtr, u16 DeviceId, struct metal_device **DevicePtr);


/*****************************************************************************/
/**
*
* Compare two strings in the reversed order.This function compares only
* the last "Count" number of characters of Str1Ptr and Str2Ptr.
*
* @param    Str1Ptr is base address of first string
* @param    Str2Ptr is base address of second string
* @param    Count is number of last characters  to be compared between
*           Str1Ptr and Str2Ptr
*
* @return
*           0 if last "Count" number of bytes matches between Str1Ptr and
*           Str2Ptr, else difference in unmatched character.
*
*@note     None.
*
******************************************************************************/
static s32 XRFdc_Strrncmp(const char *Str1Ptr, const char *Str2Ptr, size_t Count)
{
	u16 Len1 = strlen(Str1Ptr);
	u16 Len2 = strlen(Str2Ptr);
	u8 Diff;

	for (; Len1 && Len2; Len1--, Len2--) {
		if ((Diff = Str1Ptr[Len1 - 1] - Str2Ptr[Len2 - 1]) != 0) {
			return Diff;
		}
		if (--Count == 0) {
			return 0;
		}
	}

	return (Len1 - Len2);
}

/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
int main(int argc, char **argv)
{
	int Status;
	u16 deviceId = 0;
	char deviceName[NAME_MAX];
	struct metal_device *DevicePtr;

	// if (argc == 2) {
	// 	deviceId = ((char *)argv[1])[0] - '0';
	// } else {
	// 	return -1;
	// }

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;

	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	XAxiDma_Config *CfgPtr = XDMA_LookupConfig(&DevicePtr, deviceId);

	if (NULL == CfgPtr) {
		printf("ERROR: Failed to run XDMA_LookupConfig\n");
		return XST_FAILURE;
	}

	Status = XAxiDma_RegisterMetal(&AxiDma, deviceId, &DevicePtr);
	if (Status != XST_SUCCESS) {
		printf("Register metal failed %d\r\n", Status);
		metal_device_close(DevicePtr);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		printf("Initialization failed %d\r\n", Status);
		metal_device_close(DevicePtr);
		return XST_FAILURE;
	}

	//Actually does nothing, just sets the registers
	Status = XAxiDma_SimpleTransfer(&AxiDma,(UINTPTR) 0x0,
					0x10000, XAXIDMA_DEVICE_TO_DMA);

	if (Status != XST_SUCCESS) {
		printf("XAxiDma_SimpleTransfer failed %d\r\n", Status);
		metal_device_close(DevicePtr);
		return XST_FAILURE;
	}

	printf("Test Passed \r\n");
	metal_device_close(DevicePtr);
	return XST_SUCCESS;
}

#define XDMA_BUS_NAME "platform"

XAxiDma_Config *XDMA_LookupConfig(struct metal_device **Deviceptr, u16 DeviceId)
{
	XAxiDma_Config *CfgPtr = NULL;
	s32 Status = 0;
	u32 NumInstances = 1;
	u32 AddrWidth = 0;
	char DeviceName[NAME_MAX];

	Status = XDMA_GetDeviceNameByDeviceId(DeviceName, DeviceId);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to find axidma device with device id %d", DeviceId);
		return NULL;
	}

	Status = metal_device_open("platform", DeviceName, Deviceptr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n", DeviceName);
		return NULL;
	}

	CfgPtr = (XAxiDma_Config *)malloc(sizeof(XAxiDma_Config));
	if (CfgPtr == NULL) {
		metal_log(METAL_LOG_ERROR, "\n Failed to allocate memory for XAxiDma_Config");
		metal_device_close(*Deviceptr);
		return NULL;
	}

	Status = metal_linux_get_device_property(*Deviceptr, "xlnx,addrwidth", &AddrWidth, sizeof(AddrWidth));

	AddrWidth = ntohl(AddrWidth);

	if (Status == XST_SUCCESS) {
		//TODO:
		CfgPtr->AddrWidth = AddrWidth;
		CfgPtr->BaseAddr = 0;
		CfgPtr->DeviceId = DeviceId;
		CfgPtr->HasMm2S = 0;
		CfgPtr->HasMm2SDRE = 0;
		CfgPtr->HasS2Mm = 1;
		CfgPtr->HasS2MmDRE = 0;
		CfgPtr->HasSg = 0;
		CfgPtr->HasStsCntrlStrm = 0;
		CfgPtr->MicroDmaMode = 0;
		CfgPtr->Mm2SBurstSize = 0;
		CfgPtr->Mm2SDataWidth = 0;
		CfgPtr->Mm2sNumChannels = 0;
		CfgPtr->S2MmBurstSize = 0x100;
		CfgPtr->S2MmDataWidth = 0x100;
		CfgPtr->S2MmNumChannels = 1;
		CfgPtr->SgLengthWidth = 20;
	} else {
		metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property \"reg\"");
		metal_device_close(*Deviceptr);
		return NULL;
	}

	return CfgPtr;
}

u32 XAxiDma_RegisterMetal(XAxiDma *InstancePtr, u16 DeviceId, struct metal_device **DevicePtr)
{
	s32 Status;

	/* Map RFDC device IO region */
	InstancePtr->io = metal_device_io_region(*DevicePtr, 0);
	if (InstancePtr->io == NULL) {
		metal_log(METAL_LOG_ERROR, "\n Failed to map AXIDMA region for %s.\n", (*DevicePtr)->name);
		return XST_DMA_ERROR;

	}
	InstancePtr->device = *DevicePtr;

	return XST_SUCCESS;
}


#define XAXIDMA_SIGNATURE "dma"
#define XAXIDMA_COMPATIBLE_STRING "xlnx,axi-dma"
#define XRFDC_PLATFORM_DEVICE_DIR "/sys/bus/platform/devices/"
#define XRFDC_COMPATIBLE_PROPERTY "compatible" /* device tree property */
#define XRFDC_CONFIG_DATA_PROPERTY "name" /* device tree property */
#define XRFDC_DEVICE_ID_SIZE 4U
#define XRFDC_NUM_INST_SIZE 4U

s32 XDMA_GetDeviceNameByDeviceId(char *DevNamePtr, u16 DevId)
{
	s32 Status = -1;
	u32 Data = 0;
	char CompatibleString[NAME_MAX];
	char DeviceName[NAME_MAX];
	struct metal_device *DevicePtr;
	DIR *DirPtr;
	struct dirent *DirentPtr;
	char Len = strlen(XAXIDMA_COMPATIBLE_STRING);
	char SignLen = strlen(XAXIDMA_SIGNATURE);

	DirPtr = opendir(XRFDC_PLATFORM_DEVICE_DIR);
	if (DirPtr) {
		while ((DirentPtr = readdir(DirPtr)) != NULL) {
			if (XRFdc_Strrncmp(DirentPtr->d_name, XAXIDMA_SIGNATURE, SignLen) == 0) {
				Status = metal_device_open("platform", DirentPtr->d_name, &DevicePtr);
				if (Status) {
					metal_log(METAL_LOG_ERROR, "\n Failed to open device %s", DirentPtr->d_name);
					continue;
				}
				Status = metal_linux_get_device_property(DevicePtr, XRFDC_COMPATIBLE_PROPERTY,
									 CompatibleString, Len);
				if (Status < 0) {
					metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property");
				} else if (strncmp(CompatibleString, XAXIDMA_COMPATIBLE_STRING, Len) == 0) {
					//FIXME:
					{
						strcpy(DevNamePtr, DirentPtr->d_name);
						Status = 0;
						metal_device_close(DevicePtr);
						break;
					}
					Status = metal_linux_get_device_property(DevicePtr, XRFDC_CONFIG_DATA_PROPERTY,
										 DeviceName, XRFDC_DEVICE_ID_SIZE);
					printf("Data=%s, Status=%d\r\n", DeviceName, Status);
					if (Status < 0) {
						metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property");
					} else if (Data == DevId) {
						strcpy(DevNamePtr, DirentPtr->d_name);
						Status = 0;
						metal_device_close(DevicePtr);
						break;
					}
				}
				metal_device_close(DevicePtr);
			}
		}
	}

   Status = (s32)closedir(DirPtr);
   if (Status < 0) {
      metal_log(METAL_LOG_ERROR, "\n Failed to close directory");
   }

	return Status;
}

void Xil_AssertNonvoid(int Expression)
{
	if (!Expression) {
		abort();
	}
}
