/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_sinit.c
* @addtogroup xdfemix_v1_0
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
#define XDFEMIX_BYPASS_DDC_CFG "xlnx,bypass-ddc"
#define XDFEMIX_BYPASS_MIXER_CFG "xlnx,bypass-mixer"
#define XDFEMIX_DATA_IWIDTH_CFG "xlnx,data-iwidth"
#define XDFEMIX_DATA_OWIDTH_CFG "xlnx,data-owidth"
#define XDFEMIX_ENABLE_MIX_IF_CFG "xlnx,enable-mix-if"
#define XDFEMIX_NUM_ANTENNA_CFG "xlnx,num-antenna"
#define XDFEMIX_NUM_CC_PER_ANTENNA_CFG "xlnx,num-cc-per-antenna"
#define XDFEMIX_NUM_SLOT_CHANNELS_CFG "xlnx,num-slot-channels"
#define XDFEMIX_NUM_SLOTS_CFG "xlnx,num-slots"
#define XDFEMIX_TUSER_WIDTH_CFG "xlnx,tuser-width"
#define XDFEMIX_VERSION_REGISTER_CFG "xlnx,version-register"
#define XDFEMIX_MODE_CFG "xlnx,mode"
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
u32 XDfeMix_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr, const char *DeviceNodeName);
u32 XDfeMix_LookupConfig(u16 DeviceId);
void XDfeMix_CfgInitialize(XDfeMix *InstancePtr);

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFEMIX_MAX_NUM_INSTANCES];
extern XDfeMix_Config XDfeMix_ConfigTable[XPAR_XDFEMIX_NUM_INSTANCES];
#endif
XDfeMix XDfeMix_Mixer[XDFEMIX_MAX_NUM_INSTANCES];

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
* Traverse "/sys/bus/platform/device" directory, to find Mixer device entry,
* corresponding to provided device id. The device id is defined by the address
* of the entry in the order from lowest to highest, eg.:
* Id=0 for the Mixer entry located to the lowest address,
* Id=1 for the Mixer entry located to the second lowest address,
* Id=2 for the Mixer entry located to the third lowest address, and so on.
* If device entry corresponding to said device id is found, store it in output
* buffer DeviceNamePtr.
*
* @param    DeviceNamePtr is base address of char array, where device name
*           will be stored
* @param    DeviceId contains the ID of the device to look up the
*           Mixer device name entry in "/sys/bus/platform/device"
* @param    DeviceNodeName is device node name,
*
* @return
 *           - XST_SUCCESS if successful.
 *           - XST_FAILURE if device entry not found for given device id.
 *
 *@note     None.
*
******************************************************************************/
static s32 XDfeMix_GetDeviceNameByDeviceId(char *DeviceNamePtr, u16 DeviceId, const char *DeviceNodeName)
{
	char CompatibleString[100];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	char Len = strlen(XDFEMIX_COMPATIBLE_STRING);
	int NumFiles;
	u16 DeviceIdCounter = 0;
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
		/* Check the device signature */
		if (0 != XDfeMix_Strrncmp(DirentPtr[i]->d_name,
					  DeviceNodeName,
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
u32 XDfeMix_LookupConfig(u16 DeviceId)
{
#ifndef __BAREMETAL__
	struct metal_device *Dev = XDfeMix_Mixer[DeviceId].Device;
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEMIX_BASEADDR_SIZE)) {
		goto end_failure;
	}
	XDfeMix_Mixer[DeviceId].Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_BYPASS_DDC_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeMix_Mixer[DeviceId].Config.BypassDDC = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_BYPASS_MIXER_CFG, &d,
				   XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeMix_Mixer[DeviceId].Config.BypassMixer = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_ENABLE_MIX_IF_CFG, &d,
				   XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeMix_Mixer[DeviceId].Config.EnableMixIf = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_NUM_ANTENNA_CFG,
					    &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeMix_Mixer[DeviceId].Config.NumAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEMIX_NUM_SLOT_CHANNELS_CFG,
				   &d, XDFEMIX_WORD_SIZE)) {
		goto end_failure;
	}
	XDfeMix_Mixer[DeviceId].Config.NumSlotChannels = ntohl(d);

	char str[20];
	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEMIX_MODE_CFG, str,
					    XDFEMIX_MODE_SIZE)) {
		goto end_failure;
	}

	if (0 == strncmp(str, "downlink", 8)) {
		XDfeMix_Mixer[DeviceId].Config.Mode = 0;
	} else if (0 == strncmp(str, "uplink", 6)) {
		XDfeMix_Mixer[DeviceId].Config.Mode = 1;
	} else {
		goto end_failure;
	}

	XDfeMix_Mixer[DeviceId].Config.DeviceId = DeviceId;

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	XDfeMix_Mixer[DeviceId].Config.BypassDDC =
		XDfeMix_ConfigTable[DeviceId].BypassDDC;
	XDfeMix_Mixer[DeviceId].Config.BypassMixer =
		XDfeMix_ConfigTable[DeviceId].BypassMixer;
	XDfeMix_Mixer[DeviceId].Config.EnableMixIf =
		XDfeMix_ConfigTable[DeviceId].EnableMixIf;
	XDfeMix_Mixer[DeviceId].Config.Mode =
		XDfeMix_ConfigTable[DeviceId].Mode;
	XDfeMix_Mixer[DeviceId].Config.NumAntenna =
		XDfeMix_ConfigTable[DeviceId].NumAntenna;
	XDfeMix_Mixer[DeviceId].Config.NumCCPerAntenna =
		XDfeMix_ConfigTable[DeviceId].NumCCPerAntenna;
	XDfeMix_Mixer[DeviceId].Config.NumSlotChannels =
		XDfeMix_ConfigTable[DeviceId].NumSlotChannels;

	XDfeMix_Mixer[DeviceId].Config.BaseAddr =
		XDfeMix_ConfigTable[DeviceId].BaseAddr;

	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
*
* Register/open the deviceand map Mixer to the IO region.
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
u32 XDfeMix_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr, const char *DeviceNodeName)
{
	u32 Status;
#ifndef __BAREMETAL__
	char DeviceName[100];
#endif

	Xil_AssertNonvoid(DevicePtr != NULL);

#ifdef __BAREMETAL__
	Status = metal_register_generic_device(*DevicePtr);
	if ((signed)Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device");
		return Status;
	}
	Status = metal_device_open(XDFEMIX_BUS_NAME, DeviceNodeName,
				   DevicePtr);
	if ((signed)Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device Mixer");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfeMix_GetDeviceNameByDeviceId(DeviceName, DeviceId, DeviceNodeName);
	if ((signed)Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to find mixer device with device id %d",
			  DeviceId);
		return Status;
	}

	/* open the device metal instance */
	Status = metal_device_open(XDFEMIX_BUS_NAME, DeviceName, DevicePtr);
	if ((signed)Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n",
			  DeviceName);
		return Status;
	}
#endif

	/* Map Mixer device IO region */
	XDfeMix_Mixer[DeviceId].Io = metal_device_io_region(*DevicePtr, 0U);
	if (XDfeMix_Mixer[DeviceId].Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map Mixer region for %s.\n",
			  (*DevicePtr)->name);
		metal_device_close(*DevicePtr);
		return XST_FAILURE;
	}
	XDfeMix_Mixer[DeviceId].Device = *DevicePtr;

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
* @return   None
*
* @note     The user needs to first call the XDfeMix_LookupConfig() API
*           which returns the Configuration structure pointer which is
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
				sizeof(struct metal_io_region));
		metal_io_init(
			InstancePtr->Io,
			(void *)(metal_phys_addr_t)InstancePtr->Config.BaseAddr,
			&InstancePtr->Config.BaseAddr, XDFEMIX_REGION_SIZE,
			(unsigned)(-1), 0U, NULL);
	}
#endif
}

/** @} */
