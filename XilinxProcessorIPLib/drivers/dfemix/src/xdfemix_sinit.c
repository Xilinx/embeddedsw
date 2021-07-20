/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_sinit.c
* @addtogroup xdfemix_v1_1
* @{
*
* The implementation of the XDfeMix component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     07/22/20 Initial release
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/15/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     03/18/21 New model parameter list
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/20/21 Doxygen documentation update
* 1.1   dc     07/13/21 Update to common latency requirements
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xdfemix.h"
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
#define XDFEMIX_CONFIG_DATA_PROPERTY "param-list" /* device tree property */
#define XDFEMIX_COMPATIBLE_STRING "xlnx,xdfe-cc-mixer-1.0"
#define XDFEMIX_PLATFORM_DEVICE_DIR "/sys/bus/platform/devices/"
#define XDFEMIX_COMPATIBLE_PROPERTY "compatible" /* device tree property */
#define XDFEMIX_BUS_NAME "platform"
#define XDFEMIX_DEVICE_ID_SIZE 4U
#define XDFEMIX_CONFIG_DATA_SIZE sizeof(XDfeMix_Config)
#define XDFEMIX_BASEADDR_PROPERTY "reg" /* device tree property */
#define XDFEMIX_BASEADDR_SIZE 8U

#define XDFEMIX_MODE_CFG "xlnx,mode"
#define XDFEMIX_NUM_ANTENNA_CFG "xlnx,num-antenna"
#define XDFEMIX_MAX_USABLE_CCIDS_CFG "xlnx,max-useable-ccids"
#define XDFEMIX_LANES_CFG "xlnx,lanes"
#define XDFEMIX_ANTENNA_INTERLEAVE_CFG "xlnx,antenna-interleave"
#define XDFEMIX_MIXER_CPS_CFG "xlnx,mixer-cps"
#define XDFEMIX_DATA_IWIDTH_CFG "xlnx,data-iwidth"
#define XDFEMIX_DATA_OWIDTH_CFG "xlnx,data-owidth"
#define XDFEMIX_TUSER_WIDTH_CFG "xlnx,tuser-width"
#define XDFEMIX_MODE_SIZE 10U
#define XDFEMIX_WORD_SIZE 4U

#else
#define XDFEMIX_BUS_NAME "generic"
#define XDFEMIX_REGION_SIZE 0x4000U
#endif

/************************** Function Prototypes ******************************/
#ifndef __BAREMETAL__
extern int metal_linux_get_device_property(struct metal_device *device,
					   const char *property_name,
					   void *output, int len);
#endif

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfeMix_Config XDfeMix_ConfigTable[XPAR_XDFEMIX_NUM_INSTANCES];
#endif
XDfeMix XDfeMix_Mixer[XDFEMIX_MAX_NUM_INSTANCES];

#ifdef __BAREMETAL__
/*****************************************************************************/
/**
*
* Searches for the address match between address in ConfigTable and the address
* extracted from the device node name. Returns pointer to the ConfigTable with
* a matched base address.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    ConfigTable is a configuration table container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device ID.
*
******************************************************************************/
u32 XDfeMix_GetConfigTable(XDfeMix *InstancePtr, XDfeMix_Config **ConfigTable)
{
	u32 Index;
	char Str[XDFEMIX_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	strncpy(Str, InstancePtr->NodeName, sizeof(Str));
	AddrStr = strtok(Str, ".");
	Addr = strtol(AddrStr, NULL, 16);

	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		if (XDfeMix_ConfigTable[Index].BaseAddr == Addr) {
			*ConfigTable = &XDfeMix_ConfigTable[Index];
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
* @param    Str1Ptr is base address of first string.
* @param    Str2Ptr is base address of second string.
* @param    Count is the number of last characters to be compared between
*           Str1Ptr and Str2Ptr.
*
* @return
*           0 if last "Count" number of bytes matches between Str1Ptr and
*           Str2Ptr, else difference in unmatched character.
*
******************************************************************************/
static s32 XDfeMix_Strrncmp(const char *Str1Ptr, const char *Str2Ptr,
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
* Traverses "/sys/bus/platform/device" directory (in Linux), to find registered
* device with the name DeviceNodeName.
* If the match is found then check if the device is compatible with the driver.
*
* @param    DeviceNamePtr is base address of char array, where device name
*           will be stored.
* @param    DeviceNodeName is device node name.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device ID.
*
******************************************************************************/
static s32 XDfeMix_IsDeviceCompatible(char *DeviceNamePtr,
				      const char *DeviceNodeName)
{
	char CompatibleString[100];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	char Len = strlen(XDFEMIX_COMPATIBLE_STRING);
	int NumFiles;
	u32 Status = XST_FAILURE;
	int i = 0;

	/* Make a list of files in directory in alphabetical order */
	NumFiles = scandir(XDFEMIX_PLATFORM_DEVICE_DIR, &DirentPtr, NULL,
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
		if (0 != XDfeMix_Strrncmp(DirentPtr[i]->d_name, DeviceNodeName,
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
				DevicePtr, XDFEMIX_COMPATIBLE_PROPERTY,
				CompatibleString, Len)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Failed to read device tree property");
			metal_device_close(DevicePtr);
			continue;
		}

		/* Check a "compatible" device property */
		if (strncmp(CompatibleString, XDFEMIX_COMPATIBLE_STRING, Len) !=
		    0) {
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
* @param    InstancePtr is a pointer to the mixer instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device ID.
*
* @note
*         - For BM, a table contains the configuration info for each device
*           in the system.
*         - For Linux, there will be just one config allocated and a pointer
*           pointing to the config returned.
*
******************************************************************************/
s32 XDfeMix_LookupConfig(XDfeMix *InstancePtr)
{
	struct metal_device *Dev = InstancePtr->Device;
#ifndef __BAREMETAL__
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEMIX_BASEADDR_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_TUSER_WIDTH_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.TUserWidth = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_DATA_OWIDTH_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.DataOWidth = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_DATA_IWIDTH_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.DataIWidth = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_MIXER_CPS_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.MixerCps = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_ANTENNA_INTERLEAVE_CFG,
				   &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.AntennaInterleave = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_LANES_CFG, &d,
					    XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.Lanes = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_MAX_USABLE_CCIDS_CFG, &d,
				   XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.MaxUseableCcids = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_NUM_ANTENNA_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumAntenna = ntohl(d);

	char str[20];
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_MODE_CFG, str,
					    XDFEMIX_MODE_SIZE)) {
		goto end_failure;
	}

	if (0 == strncmp(str, "downlink", 8)) {
		InstancePtr->Config.Mode = 0;
	} else if (0 == strncmp(str, "uplink", 6)) {
		InstancePtr->Config.Mode = 1;
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
	XDfeMix_Config *ConfigTable = NULL;

	/* Find the Config table which base address is a match */
	if (XST_FAILURE == XDfeMix_GetConfigTable(InstancePtr, &ConfigTable)) {
		metal_log(METAL_LOG_ERROR, "\nFailed to read device tree");
		metal_device_close(Dev);
		return XST_FAILURE;
	}

	InstancePtr->Config.Mode = ConfigTable->Mode;
	InstancePtr->Config.NumAntenna = ConfigTable->NumAntenna;
	InstancePtr->Config.MaxUseableCcids = ConfigTable->MaxUseableCcids;
	InstancePtr->Config.Lanes = ConfigTable->Lanes;
	InstancePtr->Config.AntennaInterleave = ConfigTable->AntennaInterleave;
	InstancePtr->Config.MixerCps = ConfigTable->MixerCps;
	InstancePtr->Config.DataIWidth = ConfigTable->DataIWidth;
	InstancePtr->Config.DataOWidth = ConfigTable->DataOWidth;
	InstancePtr->Config.TUserWidth = ConfigTable->TUserWidth;
	InstancePtr->Config.BaseAddr = ConfigTable->BaseAddr;

	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
*
* Registers/opens the device and maps Mixer to the IO region.
*
* @param    DeviceId contains the ID of the device to register/map.
* @param    DevicePtr is a pointer to the metal device.
* @param    DeviceNodeName is device node name.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
******************************************************************************/
s32 XDfeMix_RegisterMetal(XDfeMix *InstancePtr, struct metal_device **DevicePtr,
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
	Status = metal_device_open(XDFEMIX_BUS_NAME, DeviceNodeName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device Mixer");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfeMix_IsDeviceCompatible(DeviceName, DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to find Mixer device %s",
			  DeviceNodeName);
		return Status;
	}

	/* open the device metal instance */
	Status = metal_device_open(XDFEMIX_BUS_NAME, DeviceName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n",
			  DeviceName);
		return Status;
	}
#endif

	/* Map Mixer device IO region */
	InstancePtr->Io = metal_device_io_region(*DevicePtr, 0U);
	if (InstancePtr->Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map Mixer region for %s.\n",
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
* Initializes a specific XDfeMix instance such that the driver is ready to use.
*
*
* @param    InstancePtr is a pointer to the XDfeMix instance.
*
*
* @note     The user needs to first call the XDfeMix_LookupConfig() API,
*           which returns the Configuration structure pointer, which is
*           passed as a parameter to the XDfeMix_CfgInitialize() API.
*
******************************************************************************/
void XDfeMix_CfgInitialize(XDfeMix *InstancePtr)
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
			&InstancePtr->Config.BaseAddr, XDFEMIX_REGION_SIZE,
			(unsigned)(-1), 0U, NULL);
	}
#endif
}

/** @} */
