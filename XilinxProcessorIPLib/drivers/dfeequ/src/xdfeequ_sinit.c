/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_sinit.c
* @addtogroup xdfeequ_v1_0
* @{
*
* The implementation of the XDfeEqu component's static initialization
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
#define XDFEEQU_MAX_NUM_INSTANCES 10U
#define XDFEEQU_CONFIG_DATA_PROPERTY "param-list" /* device tree property */
#define XDFEEQU_COMPATIBLE_STRING "xlnx,xdfe-equalizer-1.0"
#define XDFEEQU_PLATFORM_DEVICE_DIR "/sys/bus/platform/devices/"
#define XDFEEQU_COMPATIBLE_PROPERTY "compatible" /* device tree property */
#define XDFEEQU_BUS_NAME "platform"
#define XDFEEQU_DEVICE_ID_SIZE 4U
#define XDFEEQU_CONFIG_DATA_SIZE sizeof(XDfeEqu_EqConfig)
#define XDFEEQU_BASEADDR_PROPERTY "reg" /* device tree property */
#define XDFEEQU_BASEADDR_SIZE 8U
#define XDFEEQU_COMPLEX_MODE_CFG "xlnx,complex-mode"
#define XDFEEQU_COMPONENT_NAME_CFG "xlnx,component-name"
#define XDFEEQU_DATA_IWIDTH_CFG "xlnx,data-iwidth"
#define XDFEEQU_DATA_OWIDTH_CFG "xlnx,data-owidth"
#define XDFEEQU_MAX_SAMPLE_RATE_CFG "xlnx,max-sample-rate"
#define XDFEEQU_NUM_CHANNELS_CFG "xlnx,num-channels"
#define XDFEEQU_TUSER_WIDTH_CFG "xlnx,tuser-width"
#define XDFEEQU_USE_EMULATION_CFG "xlnx,use-emulation"
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

#ifndef __BAREMETAL__
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
* Traverse "/sys/bus/platform/device" directory, to find Equalizer device entry,
* corresponding to provided device id. The device id is defined by the address
* of the entry in the order from lowest to highest, eg.:
* Id=0 for the Equalizer entry located to the lowest address,
* Id=1 for the Equalizer entry located to the second lowest address,
* Id=2 for the Equalizer entry located to the third lowest address, and so on.
* If device entry corresponding to said device id is found, store it in output
* buffer DeviceNamePtr.
*
* @param    DeviceNamePtr is base address of char array, where device name
*           will be stored
* @param    DeviceId contains the ID of the device to look up the
*           Equalizer device name entry in "/sys/bus/platform/device"
* @param    DeviceNodeName is device node name,
*
* @return
 *           - XST_SUCCESS if successful.
 *           - XST_FAILURE if device entry not found for given device id.
 *
 *@note     None.
*
******************************************************************************/
static s32 XDfeEqu_GetDeviceNameByDeviceId(char *DeviceNamePtr, u16 DeviceId,
					   const char *DeviceNodeName)
{
	char CompatibleString[100];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	char Len = strlen(XDFEEQU_COMPATIBLE_STRING);
	int NumFiles;
	u16 DeviceIdCounter = 0;
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

		/* Is a device Id as requested? */
		if (DeviceIdCounter != DeviceId) {
			DeviceIdCounter++;
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
* @param    DeviceId contains the ID of the device to register/map
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
s32 XDfeEqu_LookupConfig(u16 DeviceId)
{
#ifndef __BAREMETAL__
	struct metal_device *Dev = XDfeEqu_Equalizer[DeviceId].Device;
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEEQU_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEEQU_BASEADDR_SIZE)) {
		goto end_failure;
	}
	XDfeEqu_Equalizer[DeviceId].Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEEQU_DATA_IWIDTH_CFG,
					    &d, XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeEqu_Equalizer[DeviceId].Config.SampleWidth = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEEQU_DATA_OWIDTH_CFG,
					    &d, XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeEqu_Equalizer[DeviceId].Config.ComplexModel = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEEQU_NUM_CHANNELS_CFG, &d,
				   XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeEqu_Equalizer[DeviceId].Config.NumChannels = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEEQU_TUSER_WIDTH_CFG,
					    &d, XDFEEQU_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeEqu_Equalizer[DeviceId].Config.TuserWidth = ntohl(d);

	XDfeEqu_Equalizer[DeviceId].Config.DeviceId = DeviceId;

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	XDfeEqu_Equalizer[DeviceId].Config.NumChannels =
		XDfeEqu_ConfigTable[DeviceId].NumChannels;
	XDfeEqu_Equalizer[DeviceId].Config.SampleWidth =
		XDfeEqu_ConfigTable[DeviceId].SampleWidth;
	XDfeEqu_Equalizer[DeviceId].Config.ComplexModel =
		XDfeEqu_ConfigTable[DeviceId].ComplexModel;
	XDfeEqu_Equalizer[DeviceId].Config.TuserWidth =
		XDfeEqu_ConfigTable[DeviceId].TuserWidth;

	XDfeEqu_Equalizer[DeviceId].Config.BaseAddr =
		XDfeEqu_ConfigTable[DeviceId].BaseAddr;
	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
*
* Register/open the deviceand map Equalizer to the IO region.
*
* @param    DeviceId contains the ID of the device to register/map
* @param    DevicePtr is a pointer to the metal device.
* @param    DeviceNodeName is device node name,
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
s32 XDfeEqu_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr,
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
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to open device Equalizer");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfeEqu_GetDeviceNameByDeviceId(DeviceName, DeviceId,
						 DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(
			METAL_LOG_ERROR,
			"\n Failed to find equalizer device with device id %d",
			DeviceId);
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
	/* TODO - this has to be revisited when device IP got delivered! */
	XDfeEqu_Equalizer[DeviceId].Io = metal_device_io_region(*DevicePtr, 0U);
	if (XDfeEqu_Equalizer[DeviceId].Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map Equalizer region for %s.\n",
			  (*DevicePtr)->name);
		metal_device_close(*DevicePtr);
		return XST_FAILURE;
	}
	XDfeEqu_Equalizer[DeviceId].Device = *DevicePtr;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Initializes a specific XDfeEqu instance such that the driver is ready to use.
*
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
*
* @return   None
*
* @note     The user needs to first call the XDfeEqu_LookupConfig() API
*           which returns the Configuration structure pointer which is
*           passed as a parameter to the XDfeEqu_CfgInitialize() API.
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

/** @} */
