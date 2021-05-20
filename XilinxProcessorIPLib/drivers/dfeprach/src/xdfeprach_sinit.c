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
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/21/21 Update due to restructured registers
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
#define XDFEPRACH_NUM_RACH_CHANNELS_CFG "xlnx,num-rach-channels"
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

#ifdef __BAREMETAL__
/*****************************************************************************/
/**
*
* Search for the address match between address in ConfigTable and the address
* extracted from the NodeName. Return pointer to the ConfigTable with a matched
* base address.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    ConfigTable is a configuration table container.
*
* @return
 *           - XST_SUCCESS if successful.
 *           - XST_FAILURE if device entry not found for given device id.
*
*@note     None.
*
******************************************************************************/
u32 XDfePrach_GetConfigTable(XDfePrach *InstancePtr,
			     XDfePrach_Config **ConfigTable)
{
	u32 Index;
	char Str[XDFEPRACH_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	strncpy(Str, InstancePtr->NodeName, sizeof(Str));
	AddrStr = strtok(Str, ".");
	Addr = strtol(AddrStr, NULL, 16);

	for (Index = 0; Index < XDFEPRACH_MAX_NUM_INSTANCES; Index++) {
		if (XDfePrach_ConfigTable[Index].BaseAddr == Addr) {
			*ConfigTable = &XDfePrach_ConfigTable[Index];
			return XST_SUCCESS;
		}
	}
	return XST_FAILURE;
}
#else
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
* Traverse "/sys/bus/platform/device" directory (in Linux), to find registered
* device with the name DeviceNodeName.
* If the match is found than check is the device compatible with the driver.
*
* @param    DeviceNamePtr is base address of char array, where device name
*           will be stored
* @param    DeviceNodeName is device node name,
*
* @return
 *           - XST_SUCCESS if successful.
 *           - XST_FAILURE if device entry not found for given device id.
 *
 *@note     None.
*
******************************************************************************/
static s32 XDfePrach_IsDeviceCompatible(char *DeviceNamePtr,
					const char *DeviceNodeName)
{
	char CompatibleString[100];
	struct metal_device *DevicePtr;
	struct dirent **DirentPtr;
	char Len = strlen(XDFEPRACH_COMPATIBLE_STRING);
	int NumFiles;
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
		/* Check the string size */
		if (strlen(DirentPtr[i]->d_name) != strlen(DeviceNodeName)) {
			continue;
		}

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
* @param    InstancePtr is a pointer to the prach instance.
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
s32 XDfePrach_LookupConfig(XDfePrach *InstancePtr)
{
	struct metal_device *Dev = InstancePtr->Device;
#ifndef __BAREMETAL__
	u64 BaseAddr;
	char *Name;
	u32 d;

	/* Get BaseAddr from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_BASEADDR_PROPERTY,
				   &BaseAddr, XDFEPRACH_BASEADDR_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.BaseAddr = ntohl(BaseAddr);

	/* Get a config data from devicetree */
	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_ANTENNA_CFG, &d,
				   XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_CC_PER_ANTENNA_CFG,
				   &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumCCPerAntenna = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_SLOT_CHANNELS_CFG,
				   &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumAntennaChannels = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEPRACH_NUM_SLOTS_CFG,
					    &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumAntennaSlot = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_RACH_LINES_CFG, &d,
				   XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumRachLanes = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_NUM_RACH_CHANNELS_CFG,
				   &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.NumRachChannels = ntohl(d);

	if (XST_SUCCESS != metal_linux_get_device_property(
				   Dev, Name = XDFEPRACH_HAS_AXIS_CTRL_CFG, &d,
				   XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.HasAxisCtrl = ntohl(d);

	if (XST_SUCCESS !=
	    metal_linux_get_device_property(Dev, Name = XDFEPRACH_HAS_IRQ_CFG,
					    &d, XDFEPRACH_WORD_SIZE)) {
		goto end_failure;
	}
	InstancePtr->Config.HasIrq = ntohl(d);

	return XST_SUCCESS;

end_failure:
	metal_log(METAL_LOG_ERROR, "\n Failed to read device tree property %s",
		  Name);
	metal_device_close(Dev);
	return XST_FAILURE;
#else
	XDfePrach_Config *ConfigTable = NULL;

	/* Find the Config table which base address is a match */
	if (XST_FAILURE ==
	    XDfePrach_GetConfigTable(InstancePtr, &ConfigTable)) {
		metal_log(METAL_LOG_ERROR, "\nFailed to read device tree");
		metal_device_close(Dev);
		return XST_FAILURE;
	}

	InstancePtr->Config.NumAntenna = ConfigTable->NumAntenna;
	InstancePtr->Config.NumCCPerAntenna = ConfigTable->NumCCPerAntenna;
	InstancePtr->Config.NumAntennaChannels =
		ConfigTable->NumAntennaChannels;
	InstancePtr->Config.NumAntennaSlot = ConfigTable->NumAntennaSlot;
	InstancePtr->Config.NumRachLanes = ConfigTable->NumRachLanes;
	InstancePtr->Config.NumRachChannels = ConfigTable->NumRachChannels;
	InstancePtr->Config.HasAxisCtrl = ConfigTable->HasAxisCtrl;
	InstancePtr->Config.HasIrq = ConfigTable->HasIrq;

	InstancePtr->Config.BaseAddr = ConfigTable->BaseAddr;

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
s32 XDfePrach_RegisterMetal(XDfePrach *InstancePtr,
			    struct metal_device **DevicePtr,
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
	Status = XDfePrach_IsDeviceCompatible(DeviceName, DeviceNodeName);
	if (Status != XST_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Failed to find Prach device %s",
			  DeviceNodeName);
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
	InstancePtr->Io = metal_device_io_region(*DevicePtr, 0U);
	if (InstancePtr->Io == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "\n Failed to map Prach region for %s.\n",
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
