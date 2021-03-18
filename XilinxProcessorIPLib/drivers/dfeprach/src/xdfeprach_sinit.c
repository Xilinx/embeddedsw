/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_sinit.c
* @addtogroup xdfeprach_v1_0
* @{
*
* The implementation of the XDfePrach component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     03/08/21 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xdfeprach.h"
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
#define XDFEPRACH_CONFIG_DATA_PROPERTY "param-list" /* device tree property */
#define XDFEPRACH_COMPATIBLE_STRING "xlnx,xdfe-nr-prach-1.0"
#define XDFEPRACH_PLATFORM_DEVICE_DIR "/sys/bus/platform/devices/"
#define XDFEPRACH_COMPATIBLE_PROPERTY "compatible" /* device tree property */
#define XDFEPRACH_BUS_NAME "platform"
#define XDFEPRACH_DEVICE_ID_SIZE 4U
#define XDFEPRACH_CONFIG_DATA_SIZE sizeof(XDfePrach_Config)
#define XDFEPRACH_BASEADDR_PROPERTY "reg" /* device tree property */
#define XDFEPRACH_BASEADDR_SIZE 8U
#define XDFEPRACH_NUM_ANTENNA_CFG "xlnx,num-antenna"
#define XDFEPRACH_NUM_CC_PER_ANTENNA_CFG "xlnx,num-cc-per-antenna"
#define XDFEPRACH_NUM_SLOT_CHANNELS_CFG "xlnx,num-slot-channels"
#define XDFEPRACH_NUM_SLOTS_CFG "xlnx,num-slots"
#define XDFEPRACH_NUM_RACH_LINES_CFG "xlnx,num-rach-lanes"
#define XDFEPRACH_HAS_AXIS_CTRL_CFG "xlnx,has-axis-ctrl"
#define XDFEPRACH_HAS_IRQ_CFG "xlnx,has-irq"
#define XDFEPRACH_WORD_SIZE 4U
#else
#define XDFEPRACH_BUS_NAME "generic"
#define XDFEPRACH_REGION_SIZE 0x4000U
#endif

/************************** Function Prototypes ******************************/
#ifndef __BAREMETAL__
extern int metal_linux_get_device_property(struct metal_device *device,
					   const char *property_name,
					   void *output, int len);
#endif

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfePrach_Config XDfePrach_ConfigTable[XPAR_XDFEPRACH_NUM_INSTANCES];
#endif
XDfePrach XDfePrach_Prach[XDFEPRACH_MAX_NUM_INSTANCES];

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
static s32 XDfePrach_Strrncmp(const char *Str1Ptr, const char *Str2Ptr,
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
* Traverse "/sys/bus/platform/device" directory, to find Prach device entry,
* corresponding to provided device id. The device id is defined by the address
* of the entry in the order from lowest to highest, eg.:
* Id=0 for the Prach entry located to the lowest address,
* Id=1 for the Prach entry located to the second lowest address,
* Id=2 for the Prach entry located to the third lowest address, and so on.
* If device entry corresponding to said device id is found, store it in output
* buffer DeviceNamePtr.
*
* @param    DeviceNamePtr is base address of char array, where device name
*           will be stored
* @param    DeviceId contains the ID of the device to look up the
*           Prach device name entry in "/sys/bus/platform/device"
* @param    DeviceNodeName is device node name,
*
* @return
 *           - XST_SUCCESS if successful.
 *           - XST_FAILURE if device entry not found for given device id.
 *
 *@note     None.
*
******************************************************************************/
static s32 XDfePrach_GetDeviceNameByDeviceId(char *DeviceNamePtr, u16 DeviceId,
					     const char *DeviceNodeName)
{
	char CompatibleString[100];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	char Len = strlen(XDFEPRACH_COMPATIBLE_STRING);
	int NumFiles;
	u16 DeviceIdCounter = 0;
	u32 Status = XST_FAILURE;
	int i = 0;

	/* Make a list of files in directory in alphabetical order */
	NumFiles = scandir(XDFEPRACH_PLATFORM_DEVICE_DIR, &DirentPtr, NULL,
			   alphasort);
	if (NumFiles < 0) {
		metal_log(METAL_LOG_ERROR,
			  "\n scandir failed to open directory");
		return XST_FAILURE;
	}

	/* Loop through the each device file in directory */
	for (i = 0; i < NumFiles; i++) {
		/* Check the device signature */
		if (0 != XDfePrach_Strrncmp(DirentPtr[i]->d_name,
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
				DevicePtr, XDFEPRACH_COMPATIBLE_PROPERTY,
				CompatibleString, Len)) {
			metal_log(METAL_LOG_ERROR,
				  "\n Failed to read device tree property");
			metal_device_close(DevicePtr);
			continue;
		}

		/* Check a "compatible" device property */
		if (strncmp(CompatibleString, XDFEPRACH_COMPATIBLE_STRING,
			    Len) != 0) {
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
s32 XDfePrach_LookupConfig(u16 DeviceId)
{
#ifndef __BAREMETAL__
	struct metal_device *Dev = XDfePrach_Prach[DeviceId].Device;
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEPRACH_BASEADDR_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_ANTENNA_CFG, &d,
				   XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.NumAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_CC_PER_ANTENNA_CFG,
				   &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.NumCCPerAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_SLOT_CHANNELS_CFG,
				   &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.NumAntennaChannels = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEPRACH_NUM_SLOTS_CFG,
					    &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.NumAntennaSlot = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_RACH_LINES_CFG, &d,
				   XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.NumRachLanes = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_HAS_AXIS_CTRL_CFG, &d,
				   XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.HasAxisCtrl = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEPRACH_HAS_IRQ_CFG,
					    &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	XDfePrach_Prach[DeviceId].Config.HasIrq = ntohl(d);

	XDfePrach_Prach[DeviceId].Config.DeviceId = DeviceId;

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	XDfePrach_Prach[DeviceId].Config.NumAntenna =
		XDfePrach_ConfigTable[DeviceId].NumAntenna;
	XDfePrach_Prach[DeviceId].Config.NumCCPerAntenna =
		XDfePrach_ConfigTable[DeviceId].NumCCPerAntenna;
	XDfePrach_Prach[DeviceId].Config.NumAntennaChannels =
		XDfePrach_ConfigTable[DeviceId].NumAntennaChannels;
	XDfePrach_Prach[DeviceId].Config.NumAntennaSlot =
		XDfePrach_ConfigTable[DeviceId].NumAntennaSlot;
	XDfePrach_Prach[DeviceId].Config.NumRachLanes =
		XDfePrach_ConfigTable[DeviceId].NumRachLanes;
	XDfePrach_Prach[DeviceId].Config.HasAxisCtrl =
		XDfePrach_ConfigTable[DeviceId].HasAxisCtrl;
	XDfePrach_Prach[DeviceId].Config.HasIrq =
		XDfePrach_ConfigTable[DeviceId].HasIrq;

	XDfePrach_Prach[DeviceId].Config.BaseAddr =
		XDfePrach_ConfigTable[DeviceId].BaseAddr;

	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
*
* Register/open the deviceand map Prach to the IO region.
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
s32 XDfePrach_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr,
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
	Status = metal_device_open(XDFEPRACH_BUS_NAME, DeviceNodeName,
				   DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device Prach");
		return Status;
	}
#else
	/* Get device name */
	Status = XDfePrach_GetDeviceNameByDeviceId(DeviceName, DeviceId,
						   DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to find prach device with device id %d",
			  DeviceId);
		return Status;
	}

	/* open the device metal instance */
	Status = metal_device_open(XDFEPRACH_BUS_NAME, DeviceName, DevicePtr);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to open device %s.\n",
			  DeviceName);
		return Status;
	}
#endif

	/* Map Prach device IO region */
	XDfePrach_Prach[DeviceId].Io = metal_device_io_region(*DevicePtr, 0U);
	if (XDfePrach_Prach[DeviceId].Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map Prach region for %s.\n",
			  (*DevicePtr)->name);
		metal_device_close(*DevicePtr);
		return XST_FAILURE;
	}
	XDfePrach_Prach[DeviceId].Device = *DevicePtr;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Initializes a specific XDfePrach instance such that the driver is ready to use.
*
*
* @param    InstancePtr is a pointer to the XDfePrach instance.
*
* @return   None
*
* @note     The user needs to first call the XDfePrach_LookupConfig() API
*           which returns the Configuration structure pointer which is
*           passed as a parameter to the XDfePrach_CfgInitialize() API.
*
******************************************************************************/
void XDfePrach_CfgInitialize(XDfePrach *InstancePtr)
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
			&InstancePtr->Config.BaseAddr, XDFEPRACH_REGION_SIZE,
			(unsigned)(-1), 0U, NULL);
	}
#endif
}

/** @} */
