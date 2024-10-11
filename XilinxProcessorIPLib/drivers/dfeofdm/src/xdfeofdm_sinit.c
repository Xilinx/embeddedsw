/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm_sinit.c
*
* @cond NOCOMMENTS
*
* The implementation of the XDfeOfdm component's static initialization
* functionality.
*
* @endcond
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     11/21/22 Initial version
*       dc     02/20/23 Update hw version
* 1.1   dc     05/22/23 State and status upgrades
*       cog    07/04/23 Add support for SDT
* 1.2   dc     10/16/23 Doxygen documenatation update
*       dc     03/22/24 Update hw version
* 1.3   dc     09/23/24 Add frequency range MODEL_PARAM
*
* </pre>
* @addtogroup dfeofdm Overview
* @{
*
******************************************************************************/
/*
* @cond nocomments
*/

/***************************** Include Files *********************************/
#include "xdfeofdm.h"
#include <string.h>
#include <stdio.h>

#ifdef __BAREMETAL__
#ifndef SDT
#include "xparameters.h"
#endif
#include <metal/alloc.h>
#else
#include <dirent.h>
#include <arpa/inet.h>
#include <endian.h>
#endif

#include <metal/device.h>

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __BAREMETAL__
/**
* @endcond
*/
#define XDFEOFDM_COMPATIBLE_STRING                                             \
	"xlnx,xdfe-ofdm-2.2" /**< Device name property. */
#define XDFEOFDM_COMPATIBLE_STRING_2_1                                         \
	"xlnx,xdfe-ofdm-2.1" /**< Device name property. */
#define XDFEOFDM_PLATFORM_DEVICE_DIR                                           \
	"/sys/bus/platform/devices/" /**< Device location in a file system. */
#define XDFEOFDM_COMPATIBLE_PROPERTY "compatible" /**< Device tree property */
#define XDFEOFDM_BUS_NAME "platform" /**< System bus name. */
#define XDFEOFDM_BASEADDR_PROPERTY "reg" /**< Base address property. */
#define XDFEOFDM_BASEADDR_SIZE 8U /**< Base address bit-size. */
/*
* @cond nocomments
*/
#define XDFEOFDM_MODE_SIZE 10U
#define XDFEOFDM_WORD_SIZE 4U

#else
#define XDFEOFDM_BUS_NAME "generic"
#define XDFEOFDM_REGION_SIZE 0x4000U
#endif

/************************** Function Prototypes ******************************/
#ifndef __BAREMETAL__
extern int metal_linux_get_device_property(struct metal_device *device,
					   const char *property_name,
					   void *output, int len);
#endif

/************************** Variable Definitions *****************************/
XDfeOfdm XDfeOfdm_Ofdm[XDFEOFDM_MAX_NUM_INSTANCES];

#ifndef __BAREMETAL__
/*****************************************************************************/
/**
*
* Compares two strings in the reversed order. This function compares only
* the last "Count" number of characters of Str1Ptr and Str2Ptr.
*
* @param    Str1Ptr Base address of first string.
* @param    Str2Ptr Base address of second string.
* @param    Count Number of last characters to be compared between
*           Str1Ptr and Str2Ptr.
*
* @return
*           0 if last "Count" number of bytes matches between Str1Ptr and
*           Str2Ptr, else difference in unmatched character.
*
******************************************************************************/
static s32 XDfeOfdm_Strrncmp(const char *Str1Ptr, const char *Str2Ptr,
			     size_t Count)
{
	u16 Len1 = strlen(Str1Ptr);
	u16 Len2 = strlen(Str2Ptr);
	u8 Diff;

	for (; Len1 && Len2; Len1--, Len2--) {
		if ((Diff = Str1Ptr[Len1 - 1U] - Str2Ptr[Len2 - 1U]) != 0) {
			return Diff;
		}
		if (--Count == 0U) {
			return 0;
		}
	}

	return (Len1 - Len2);
}

/*****************************************************************************/
/**
*
* Traverses the "/sys/bus/platform/device" directory (in Linux) to find a registered
* device with the name DeviceNodeName.
* If the match is found, it checks if the device is compatible with the driver.
*
* @param    DeviceNamePtr Base address of char array where device name
*           will be stored
* @param    DeviceNodeName Device node name
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device id.
*
******************************************************************************/
static s32 XDfeOfdm_IsDeviceCompatible(char *DeviceNamePtr,
				       const char *DeviceNodeName)
{
	char CompatibleString[256];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	int NumFiles;
	u32 Status = XST_FAILURE;
	int i = 0;

	/* Make a list of files in directory in alphabetical order */
	NumFiles = scandir(XDFEOFDM_PLATFORM_DEVICE_DIR, &DirentPtr, NULL,
			   alphasort);
	if (NumFiles < 0) {
		metal_log(METAL_LOG_ERROR,
			  "\n scandir failed to open directory");
		return XST_FAILURE;
	}

	/* Loop through the each device file in directory */
	for (i = 0; i < NumFiles; i++) {
		/* Check the string size */
		if (strlen(DirentPtr[i]->d_name) != strlen(DeviceNodeName)) {
			continue;
		}

		/* Check the device signature */
		if (0 != XDfeOfdm_Strrncmp(DirentPtr[i]->d_name, DeviceNodeName,
					   strlen(DeviceNodeName))) {
			continue;
		}

		/* Open a libmetal device platform */
		if (metal_device_open("platform", DirentPtr[i]->d_name,
				      &DevicePtr)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Failed to open device %s",
				  DirentPtr[i]->d_name);
			continue;
		}

		/* Get a "compatible" device property */
		if (0 > metal_linux_get_device_property(
				DevicePtr, XDFEOFDM_COMPATIBLE_PROPERTY,
				CompatibleString,
				sizeof(CompatibleString) - 1)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Failed to read device tree property");
			metal_device_close(DevicePtr);
			continue;
		}

		/* Check does "compatible" device property has name of this
		   driver instance */
		if ((NULL ==
		     strstr(CompatibleString, XDFEOFDM_COMPATIBLE_STRING)) &&
		    (NULL == strstr(CompatibleString,
				    XDFEOFDM_COMPATIBLE_STRING_2_1))) {
			metal_log(METAL_LOG_ERROR,
				  "No compatible property match. "
				  "(Driver:\"%s\" or \"%s\", Device:\"%s\")\n",
				  XDFEOFDM_COMPATIBLE_STRING,
				  XDFEOFDM_COMPATIBLE_STRING_2_1,
				  CompatibleString);
			metal_device_close(DevicePtr);
			continue;
		}

		/* This is a requested device, save the name */
		strcpy(DeviceNamePtr, DirentPtr[i]->d_name);

		metal_device_close(DevicePtr);
		Status = XST_SUCCESS;
		break;
	}

	while (NumFiles--) {
		free(DirentPtr[NumFiles]);
	}
	free(DirentPtr);
	return Status;
}

#endif

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device id.
*
* @note
*         - For Bare-metal, a table contains the configuration information
*           for each device in the system.
*         - For Linux, there is a single config allocated and a pointer
*           pointing to the config returned.
*
******************************************************************************/
s32 XDfeOfdm_LookupConfig(XDfeOfdm *InstancePtr)
{
#ifdef __BAREMETAL__
	char Str[XDFEOFDM_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
#else
	struct metal_device *Dev;
	u64 BaseAddr;
	char *Name;
#endif

#ifndef __BAREMETAL__
	Dev = InstancePtr->Device;
	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEOFDM_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEOFDM_BASEADDR_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.BaseAddr = htobe64(BaseAddr);

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	strncpy(Str, InstancePtr->NodeName, sizeof(Str));
	AddrStr = strtok(Str, ".");
	InstancePtr->Config.BaseAddr = strtoul(AddrStr, NULL, 16);
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Registers/opens the device and maps OFDM to the I/O region.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    DevicePtr Pointer to the metal device.
* @param    DeviceNodeName Device node name,
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
******************************************************************************/
s32 XDfeOfdm_RegisterMetal(XDfeOfdm *InstancePtr,
			   struct metal_device **DevicePtr,
			   const char *DeviceNodeName)
{
	s32 Status;
#ifndef __BAREMETAL__
	char DeviceName[100] = "a";
#endif

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DevicePtr != NULL);

#ifdef __BAREMETAL__
	Status = metal_register_generic_device(*DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device");
		return Status;
	}
	Status =
		metal_device_open(XDFEOFDM_BUS_NAME, DeviceNodeName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device OFDM");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfeOfdm_IsDeviceCompatible(DeviceName, DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to find ofdm device %s",
			  DeviceNodeName);
		return Status;
	}

	/* open the device metal instance */
	Status = metal_device_open(XDFEOFDM_BUS_NAME, DeviceName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n",
			  DeviceName);
		return Status;
	}
#endif

	/* Map OFDM device IO region */
	InstancePtr->Io = metal_device_io_region(*DevicePtr, 0U);
	if (InstancePtr->Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map OFDM region for %s.\n",
			  (*DevicePtr)->name);
		metal_device_close(*DevicePtr);
		return XST_FAILURE;
	}
	InstancePtr->Device = *DevicePtr;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Initializes a specific XDfeOfdm instance such that the driver is ready to use.
*
*
* @param    InstancePtr Pointer to the XDfeOfdm instance.
*
*
* @note     The user must first call the XDfeOfdm_LookupConfig() API
*           which returns the Configuration structure pointer
*           passed as a parameter to the XDfeOfdm_CfgInitialize() API.
*
******************************************************************************/
void XDfeOfdm_CfgInitialize(XDfeOfdm *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

#ifdef __BAREMETAL__
	/*for cases where we haven't registered a custom device*/
	if (InstancePtr->Io == NULL) {
		InstancePtr->Io =
			(struct metal_io_region *)metal_allocate_memory(
				(unsigned)sizeof(struct metal_io_region));
		metal_io_init(
			InstancePtr->Io,
			(void *)(metal_phys_addr_t)InstancePtr->Config.BaseAddr,
			&InstancePtr->Config.BaseAddr, XDFEOFDM_REGION_SIZE,
			(unsigned)(-1), 0U, NULL);
	}
#endif
}

/**
* @endcond
*/
/** @} */
