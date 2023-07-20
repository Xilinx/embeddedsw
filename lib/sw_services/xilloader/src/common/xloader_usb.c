/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
****************************************************************************/

/**
*
* @file xloader_usb.c
*
* This file contains definitions of the generic handler functions to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv  02/10/2019 First release
*       bsv  04/09/2020 Code clean up
* 1.01  bsv  07/08/2020 Moved Ch9Handler APIs to xloader_dfu_util.c
*       skd  07/14/2020 XLoader_UsbCopy prototype changed
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
* 1.02  bsv  08/31/2021 Code clean up
* 1.03  ma   01/17/2022 Enable SLVERR for USB registes
*       bsv  01/21/2022 Reduce stack usage
* 1.04  bm   07/06/2022 Refactor versal and versal_net code
* 1.05  ng   11/11/2022 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       ng   06/26/2023 Added support for system device tree flow
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xplmi_hw.h"
#include "xloader_usb.h"
#ifdef XLOADER_USB
#include "xloader_dfu_util.h"
#include "xplmi_util.h"
#include "sleep.h"
#include "xloader.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xplmi.h"
#include "xloader_ddr.h"
#include "xloader_plat.h"

/************************** Constant Definitions ****************************/
#define XLOADER_USB2_REG_CTRL_OFFSET	(0x60U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u8* DfuVirtFlash = (u8*)XLOADER_DDR_TEMP_BUFFER_ADDRESS;
u8 DownloadDone = 0U;

/*****************************************************************************/
/**
 * @brief	This function initializes the USB interface.
 *
 * @param	Device Flags are unused and only passed to maintain
 *			compatibility with init functions of other boot modes
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_USB_INSTANCE if USB instance creation fails.
 * 			- XLOADER_ERR_MEMSET_USB_PRIVATE_DATA if USB private instance
 * 			creation fails.
 * 			- XLOADER_ERR_MEMSET_DFU_OBJ if DFU object instance creation fails.
 * 			- XLOADER_ERR_USB_LOOKUP if failed to lookup USB config.
 * 			- XLOADER_ERR_USB_CFG if USB fails to configure.
 * 			- XLOADER_ERR_USB_START if USB fails to start.
 *
*****************************************************************************/
int XLoader_UsbInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	Usb_Config *UsbConfigPtr;
	struct XUsbPsu *UsbPrivateDataPtr = (struct XUsbPsu *)
		XPLMI_PMCRAM_CHUNK_MEMORY;
	struct Usb_DevData *UsbInstancePtr = (struct Usb_DevData *)
		XPLMI_PMCRAM_CHUNK_MEMORY_1;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;

	(void) DeviceFlags;
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_USB_0,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemSetBytes((void *)UsbInstancePtr, sizeof(struct Usb_DevData), 0U,
		sizeof(struct Usb_DevData));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_USB_INSTANCE);
		goto END;
	}
	Status = XPlmi_MemSetBytes((void *)UsbPrivateDataPtr, sizeof(struct XUsbPsu),
				0U, sizeof(struct XUsbPsu));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_USB_PRIVATE_DATA);
		goto END;
	}
	Status = XPlmi_MemSetBytes(&DfuObj, sizeof(DfuObj), 0U, sizeof(DfuObj));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_DFU_OBJ);
		goto END;
	}

	UsbConfigPtr = XUsbPsu_LookupConfig(XLOADER_USB_DEVICE);
	if (NULL == UsbConfigPtr) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_USB_LOOKUP, Status);
		goto END;
	}

	UsbPrivateDataPtr->AppData = UsbInstancePtr;
	UsbInstancePtr->PrivateData = (void*)UsbPrivateDataPtr;

	Status = (int) XUsbPsu_CfgInitialize(
			(struct XUsbPsu*)UsbInstancePtr->PrivateData,
			UsbConfigPtr, UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_USB_CFG, Status);
		goto END;
	}

	/**
     * - Enable SLVERR.
    */
	XPlmi_UtilRMW((VENDOR_BASE_ADDRESS + XLOADER_USB2_REG_CTRL_OFFSET),
			XPLMI_SLAVE_ERROR_ENABLE_MASK, XPLMI_SLAVE_ERROR_ENABLE_MASK);

	Status = XLoader_DdrInit(XLOADER_PDI_SRC_DDR);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
     * - Hook up chapter9 handler.
    */
	XUsbPsu_set_ch9handler((struct XUsbPsu*)UsbInstancePtr->PrivateData,
		XLoader_Ch9Handler);

	/**
     * - Set the reset event handler.
    */
	XUsbPsu_set_rsthandler((struct XUsbPsu*)UsbInstancePtr->PrivateData,
		XLoader_DfuReset);

	DfuObj.InstancePtr = UsbInstancePtr;

	/**
     * - Set DFU state to APP_IDLE.
    */
	XLoader_DfuSetState(UsbInstancePtr, XLOADER_STATE_APP_IDLE);

	/**
     * - Assign the data to usb driver.
    */
	XUsbPsu_set_drvdata((struct XUsbPsu*)UsbInstancePtr->PrivateData, &Dfu_data);

	/**
	 * - Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr((struct XUsbPsu*)UsbInstancePtr->PrivateData,
		XUSBPSU_DEVTEN_EVNTOVERFLOWEN | XUSBPSU_DEVTEN_WKUPEVTEN
		| XUSBPSU_DEVTEN_ULSTCNGEN | XUSBPSU_DEVTEN_CONNECTDONEEN
		| XUSBPSU_DEVTEN_USBRSTEN | XUSBPSU_DEVTEN_DISCONNEVTEN);

	/**
     * - Start the controller so that Host can see our device.
    */
	Status = XUsbPsu_Start((struct XUsbPsu*)UsbInstancePtr->PrivateData);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_USB_START, Status);
		goto END;
	}

	while ((DownloadDone < XLOADER_DOWNLOAD_COMPLETE) && \
		(DfuObj.CurrStatus != XLOADER_STATE_DFU_ERROR)) {
		XUsbPsu_IntrHandler((struct XUsbPsu*)UsbInstancePtr->PrivateData);
	}
	(void)XUsbPsu_Stop((struct XUsbPsu*)UsbInstancePtr->PrivateData);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function copies from DFU temporary address in DDR to
* 			destination.
*
* @param	Source Address is the offset of the memory chunk to be copied.
*			This value is added to fixed DDR address to calculate the actual
*			DDR address where the image resides.
* @param	Destination Address is the address to which the memory chunk
* 			is to be copied.
* @param	Number of Bytes to be copied
* @param	Flags is unused and is only passed to maintain compatibility
*			with copy functions of other boot modes
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
int XLoader_UsbCopy(u64 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;

	/**
	 * - Start the transfer using DDR copy.
	*/
	Status = XLoader_DdrCopy((SrcAddress + XLOADER_DDR_TEMP_BUFFER_ADDRESS),
			DestAddress, Length, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function releases control of USB.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_UsbRelease(void)
{
	int Status = XST_FAILURE;

	/**
	 * - Release the USB device.
	*/
	Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_USB_0,
		XPLMI_CMD_SECURE);

	return Status;
}
#endif
