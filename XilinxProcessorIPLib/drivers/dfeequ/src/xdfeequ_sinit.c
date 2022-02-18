/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_sinit.c
* @addtogroup Overview
* @{
* @cond nocomments
* The implementation of the Equalizer component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial release
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/22/21 align driver to current specification
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/20/21 Doxygen documentation update
* 1.1   dc     10/26/21 Make driver R5 compatible
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/19/21 Update doxygen documentation
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xdfeequ.h"
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
#define XDFEEQU_COMPATIBLE_STRING                                              \
	"xlnx,xdfe-equalizer-1.0" /**< Device name property. */
#define XDFEEQU_PLATFORM_DEVICE_DIR                                            \
	"/sys/bus/platform/devices/" /**< Device location in a file system. */
#define XDFEEQU_COMPATIBLE_PROPERTY "compatible" /**< Device tree property */
#define XDFEEQU_BUS_NAME "platform" /**< System bus name. */
#define XDFEEQU_BASEADDR_PROPERTY "reg" /**< Base address property. */
#define XDFEEQU_BASEADDR_SIZE 8U /**< Base address bit-size */
#define XDFEEQU_COMPLEX_MODE_CFG                                               \
	"xlnx,complex-mode" /**< Complex mode enabled property. */
#define XDFEEQU_COMPONENT_NAME_CFG                                             \
	"xlnx,component-name" /**< Component name property. */
#define XDFEEQU_DATA_IWIDTH_CFG "xlnx,data-iwidth" /**< Data IWIDTH property. */
#define XDFEEQU_DATA_OWIDTH_CFG "xlnx,data-owidth" /**< Data OWIDTH property. */
#define XDFEEQU_MAX_SAMPLE_RATE_CFG                                            \
	"xlnx,max-sample-rate" /**< Sample rate property. */
#define XDFEEQU_NUM_CHANNELS_CFG                                               \
	"xlnx,num-channels" /**< Number of channels property. */
#define XDFEEQU_TUSER_WIDTH_CFG                                                \
	"xlnx,tuser-width" /**< Width of the TUSER bus. */
#define XDFEEQU_USE_EMULATION_CFG                                              \
	"xlnx,use-emulation" /**< Use emulation property. */
/**
* @cond nocomments
*/
#define XDFEEQU_WORD_SIZE 4U

#else
#define XDFEEQU_MAX_NUM_INSTANCES XPAR_XDFEEQU_NUM_INSTANCES
#define XDFEEQU_BUS_NAME "generic"
#define XDFEEQU_REGION_SIZE 0x4000U
#endif

/************************** Function Prototypes ******************************/
#ifndef __BAREMETAL__
extern int metal_linux_get_device_property(struct metal_device *device,
					   const char *property_name,
					   void *output, int len);
#endif

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfeEqu_Config XDfeEqu_ConfigTable[XPAR_XDFEEQU_NUM_INSTANCES];
#endif
XDfeEqu XDfeEqu_Equalizer[XDFEEQU_MAX_NUM_INSTANCES];

#ifdef __BAREMETAL__
/*****************************************************************************/
/**
*
* Search for the address match between address in ConfigTable and the address
* extracted from the NodeName. Return pointer to the ConfigTable with a matched
* base address.
*
* @param    InstancePtr Pointer to the Equalizer instance.
* @param    ConfigTable Configuration table container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if device entry not found for given device id.
*
******************************************************************************/
u32 XDfeEqu_GetConfigTable(XDfeEqu *InstancePtr, XDfeEqu_Config **ConfigTable)
{
	u32 Index;
	char Str[XDFEEQU_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	strncpy(Str, InstancePtr->NodeName, sizeof(Str));
	AddrStr = strtok(Str, ".");
	Addr = strtoul(AddrStr, NULL, 16);

	for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
		if (XDfeEqu_ConfigTable[Index].BaseAddr == Addr) {
			*ConfigTable = &XDfeEqu_ConfigTable[Index];
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
*           0 if the last "Count" number of bytes matches between Str1Ptr
*           and Str2Ptr, else difference in unmatched character.
*
******************************************************************************/
static s32 XDfeEqu_Strrncmp(const char *Str1Ptr, const char *Str2Ptr,
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
static s32 XDfeEqu_IsDeviceCompatible(char *DeviceNamePtr,
				      const char *DeviceNodeName)
{
	char CompatibleString[100];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	char Len = strlen(XDFEEQU_COMPATIBLE_STRING);
	int NumFiles;
	u32 Status = XST_FAILURE;
	int i = 0;

	/* Make a list of files in directory in alphabetical order */
	NumFiles = scandir(XDFEEQU_PLATFORM_DEVICE_DIR, &DirentPtr, NULL,
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
		if (0 != XDfeEqu_Strrncmp(DirentPtr[i]->d_name, DeviceNodeName,
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
				DevicePtr, XDFEEQU_COMPATIBLE_PROPERTY,
				CompatibleString, Len)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Failed to read device tree property");
			metal_device_close(DevicePtr);
			continue;
		}

		/* Check a "compatible" device property */
		if (strncmp(CompatibleString, XDFEEQU_COMPATIBLE_STRING, Len) !=
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
* @param    InstancePtr Pointer to the Equalizer instance.
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
s32 XDfeEqu_LookupConfig(XDfeEqu *InstancePtr)
{
	struct metal_device *Dev = InstancePtr->Device;
#ifndef __BAREMETAL__
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEEQU_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEEQU_BASEADDR_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEEQU_DATA_IWIDTH_CFG,
					    &d, XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.SampleWidth = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEEQU_DATA_OWIDTH_CFG,
					    &d, XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.ComplexModel = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEEQU_NUM_CHANNELS_CFG, &d,
				   XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumChannels = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEEQU_TUSER_WIDTH_CFG,
					    &d, XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.TuserWidth = ntohl(d);

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	XDfeEqu_Config *ConfigTable = NULL;

	/* Find the Config table which base address is a match */
	if (XST_FAILURE == XDfeEqu_GetConfigTable(InstancePtr, &ConfigTable)) {
		metal_log(METAL_LOG_ERROR, "\nFailed to read device tree");
		metal_device_close(Dev);
		return XST_FAILURE;
	}

	InstancePtr->Config.NumChannels = ConfigTable->NumChannels;
	InstancePtr->Config.SampleWidth = ConfigTable->SampleWidth;
	InstancePtr->Config.ComplexModel = ConfigTable->ComplexModel;
	InstancePtr->Config.TuserWidth = ConfigTable->TuserWidth;
	InstancePtr->Config.BaseAddr = ConfigTable->BaseAddr;

	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
*
* Register/open the deviceand map Equalizer to the IO region.
*
* @param    InstancePtr Pointer to the Equalizer instance.
* @param    DevicePtr Pointer to the metal device.
* @param    DeviceNodeName Device node name,
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
******************************************************************************/
s32 XDfeEqu_RegisterMetal(XDfeEqu *InstancePtr, struct metal_device **DevicePtr,
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
	Status = metal_device_open(XDFEEQU_BUS_NAME, DeviceNodeName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device EQU");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfeEqu_IsDeviceCompatible(DeviceName, DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to find EQU device %s",
			  DeviceNodeName);
		return Status;
	}

	/* open the device metal instance */
	Status = metal_device_open(XDFEEQU_BUS_NAME, DeviceName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n",
			  DeviceName);
		return Status;
	}
#endif

	/* Map Equalizer device IO region */
	InstancePtr->Io = metal_device_io_region(*DevicePtr, 0U);
	if (InstancePtr->Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map Equalizer region for %s.\n",
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
* Initializes a specific Equalizer instance such that the driver is ready to
* use.
*
*
* @param    InstancePtr Pointer to the Equalizer instance.
*
*
* @note     The user needs to first call the XDfeEqu_LookupConfig() API,
*           which returns the Configuration structure pointer passed as
*           a parameter to the XDfeEqu_CfgInitialize() API.
*
******************************************************************************/
void XDfeEqu_CfgInitialize(XDfeEqu *InstancePtr)
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
			&InstancePtr->Config.BaseAddr, XDFEEQU_REGION_SIZE,
			(unsigned)(-1), 0U, NULL);
	}
#endif
}

/**
* @endcond
*/
/** @} */
