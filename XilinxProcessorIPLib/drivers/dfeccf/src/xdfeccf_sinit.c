/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_sinit.c
* The implementation of the XDfeCcf component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/27/20 Initial release
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/08/21 align driver to current specification
*       dc     02/22/21 include HW in versioning
*       dc     03/25/21 Device tree item name change
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/20/21 Doxygen documentation update
* 1.1   dc     10/26/21 Make driver R5 compatible
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/19/21 Update doxygen documentation
* 1.4   dc     08/18/22 Update IP version number
* 1.5   dc     10/28/22 Switching Uplink/Downlink support
*
* </pre>
* @addtogroup dfeccf Overview
* @{
*
******************************************************************************/
/*
* @cond nocomments
*/

/***************************** Include Files *********************************/
#include "xdfeccf.h"
#include "xdfeccf_hw.h"
#include <string.h>
#include <stdio.h>

#ifdef __BAREMETAL__
#include "xparameters.h"
#include <metal/alloc.h>
#else
#include <dirent.h>
#include <arpa/inet.h>
#endif

#include <metal/device.h>

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __BAREMETAL__
/**
* @endcond
*/
#define XDFECCF_COMPATIBLE_STRING                                              \
	"xlnx,xdfe-cc-filter-1.1" /**< Device name property. */
#define XDFECCF_PLATFORM_DEVICE_DIR                                            \
	"/sys/bus/platform/devices/" /**< Device location in a file system. */
#define XDFECCF_COMPATIBLE_PROPERTY "compatible" /**< Device tree property */
#define XDFECCF_BUS_NAME "platform" /**< System bus name. */
#define XDFECCF_BASEADDR_PROPERTY "reg" /**< Base address property. */
#define XDFECCF_BASEADDR_SIZE 8U /**< Base address bit-size. */
#define XDFECCF_NUM_ANTENNA_CFG                                                \
	"xlnx,num-antenna" /**< Number of antenna property. */
#define XDFECCF_NUM_CC_PER_ANTENNA_CFG                                         \
	"xlnx,num-cc-per-antenna" /**< Maximum number of CC's per antenna. */
#define XDFECCF_ANTENNA_INTERLEAVE_CFG                                         \
	"xlnx,antenna-interleave" /**< Number of Antenna TDM slots, per CC. */
#define XDFECCF_SWITCHABLE_CFG "xlnx,switchable" /**< DL/UL switching support.*/
#define XDFECCF_TUSER_WIDTH_CFG "xlnx,tuser-width" /**< TUSER width property. */
/*
* @cond nocomments
*/
#define XDFECCF_SWITCHABLE_SIZE 15U
#define XDFECCF_WORD_SIZE 4U

#else
#define XDFECCF_BUS_NAME "generic"
#define XDFECCF_REGION_SIZE 0x4000U
#endif

/************************** Function Prototypes ******************************/
#ifndef __BAREMETAL__
extern int metal_linux_get_device_property(struct metal_device *device,
					   const char *property_name,
					   void *output, int len);
#endif

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfeCcf_Config XDfeCcf_ConfigTable[XPAR_XDFECCF_NUM_INSTANCES];
#endif
XDfeCcf XDfeCcf_ChFilter[XDFECCF_MAX_NUM_INSTANCES];

#ifdef __BAREMETAL__
/*****************************************************************************/
/**
*
* Search for the address match between address in ConfigTable and the address
* extracted from the NodeName. Return pointer to the ConfigTable with a matched
* base address.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    ConfigTable Configuration table container.
*
* @return
 *           - XST_SUCCESS if successful.
 *           - XST_FAILURE if device entry not found for given device id.
*
******************************************************************************/
u32 XDfeCcf_GetConfigTable(XDfeCcf *InstancePtr, XDfeCcf_Config **ConfigTable)
{
	u32 Index;
	char Str[XDFECCF_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	strncpy(Str, InstancePtr->NodeName, sizeof(Str));
	AddrStr = strtok(Str, ".");
	Addr = strtoul(AddrStr, NULL, 16);

	for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
		if (XDfeCcf_ConfigTable[Index].BaseAddr == Addr) {
			*ConfigTable = &XDfeCcf_ConfigTable[Index];
			return XST_SUCCESS;
		}
	}
	return XST_FAILURE;
}
#else
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
static s32 XDfeCcf_Strrncmp(const char *Str1Ptr, const char *Str2Ptr,
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
* Traverse "/sys/bus/platform/device" directory (in Linux), to find registered
* device with the name DeviceNodeName.
* If the match is found than check is the device compatible with the driver.
*
* @param    DeviceNamePtr Base address of char array, where device name
*           will be stored
* @param    DeviceNodeName Device node name
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device id.
*
******************************************************************************/
static s32 XDfeCcf_IsDeviceCompatible(char *DeviceNamePtr,
				      const char *DeviceNodeName)
{
	char CompatibleString[256];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	int NumFiles;
	u32 Status = XST_FAILURE;
	int i = 0;

	/* Make a list of files in directory in alphabetical order */
	NumFiles = scandir(XDFECCF_PLATFORM_DEVICE_DIR, &DirentPtr, NULL,
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
		if (0 != XDfeCcf_Strrncmp(DirentPtr[i]->d_name, DeviceNodeName,
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
				DevicePtr, XDFECCF_COMPATIBLE_PROPERTY,
				CompatibleString,
				sizeof(CompatibleString) - 1)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Failed to read device tree property");
			metal_device_close(DevicePtr);
			continue;
		}

		/* Check does "compatible" device property has name of this
		   driver instance */
		if (NULL ==
		    strstr(CompatibleString, XDFECCF_COMPATIBLE_STRING)) {
			metal_log(
				METAL_LOG_ERROR,
				"No compatible property match.(Driver:%s, Device:%s)\n",
				XDFECCF_COMPATIBLE_STRING, CompatibleString);
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
* @param    InstancePtr Pointer to the Channel Filter instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device id.
*
* @note
*         - For BM a table contains the configuration info for each device
*           in the system.
*         - For Linux there will be just one config allocated and pointer to
*           pointing to the config returned.
*
******************************************************************************/
s32 XDfeCcf_LookupConfig(XDfeCcf *InstancePtr)
{
	struct metal_device *Dev = InstancePtr->Device;
#ifndef __BAREMETAL__
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFECCF_BASEADDR_PROPERTY,
				   &BaseAddr, XDFECCF_BASEADDR_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFECCF_NUM_ANTENNA_CFG,
					    &d, XDFECCF_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFECCF_NUM_CC_PER_ANTENNA_CFG,
				   &d, XDFECCF_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumCCPerAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFECCF_ANTENNA_INTERLEAVE_CFG,
				   &d, XDFECCF_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.AntennaInterleave = ntohl(d);

	char str[20];
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFECCF_SWITCHABLE_CFG,
					    str, XDFECCF_SWITCHABLE_SIZE)) {
		goto end_failure;
	}
	if (0 == strncmp(str, "false", 5)) {
		InstancePtr->Config.Switchable = XDFECCF_SWITCHABLE_NO;
	} else if (0 == strncmp(str, "true", 4)) {
		InstancePtr->Config.Switchable = XDFECCF_SWITCHABLE_YES;
	} else {
		goto end_failure;
	}

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	XDfeCcf_Config *ConfigTable = NULL;

	/* Find the Config table which base address is a match */
	if (XST_FAILURE == XDfeCcf_GetConfigTable(InstancePtr, &ConfigTable)) {
		metal_log(METAL_LOG_ERROR, "\nFailed to read device tree");
		metal_device_close(Dev);
		return XST_FAILURE;
	}

	InstancePtr->Config.NumAntenna = ConfigTable->NumAntenna;
	InstancePtr->Config.NumCCPerAntenna = ConfigTable->NumCCPerAntenna;
	InstancePtr->Config.AntennaInterleave = ConfigTable->AntennaInterleave;
	InstancePtr->Config.Switchable = ConfigTable->Switchable;
	InstancePtr->Config.BaseAddr = ConfigTable->BaseAddr;

	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
*
* Register/open the deviceand map CCF to the IO region.
*
* @param    InstancePtr Pointer to the Channel Filter instance.
* @param    DevicePtr Pointer to the metal device.
* @param    DeviceNodeName Device node name,
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
******************************************************************************/
s32 XDfeCcf_RegisterMetal(XDfeCcf *InstancePtr, struct metal_device **DevicePtr,
			  const char *DeviceNodeName)
{
	s32 Status;
#ifndef __BAREMETAL__
	char DeviceName[100];
#endif

	Xil_AssertNonvoid(DevicePtr != NULL);

#ifdef __BAREMETAL__
	Status = metal_register_generic_device(*DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device");
		return Status;
	}
	Status = metal_device_open(XDFECCF_BUS_NAME, DeviceNodeName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device CCF");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfeCcf_IsDeviceCompatible(DeviceName, DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to find ccf device %s",
			  DeviceNodeName);
		return Status;
	}

	/* open the device metal instance */
	Status = metal_device_open(XDFECCF_BUS_NAME, DeviceName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n",
			  DeviceName);
		return Status;
	}
#endif

	/* Map CCF device IO region */
	InstancePtr->Io = metal_device_io_region(*DevicePtr, 0U);
	if (InstancePtr->Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map CCF region for %s.\n",
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
* Initializes a specific XDfeCcf instance such that the driver is ready to use.
*
*
* @param    InstancePtr Pointer to the XDfeCcf instance.
*
*
* @note     The user needs to first call the XDfeCcf_LookupConfig() API
*           which returns the Configuration structure pointer
*           passed as a parameter to the XDfeCcf_CfgInitialize() API.
*
******************************************************************************/
void XDfeCcf_CfgInitialize(XDfeCcf *InstancePtr)
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
			&InstancePtr->Config.BaseAddr, XDFECCF_REGION_SIZE,
			(unsigned)(-1), 0U, NULL);
	}
#endif
}

/**
* @endcond
*/
/** @} */
