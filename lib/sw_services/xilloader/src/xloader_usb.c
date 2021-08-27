/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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
* 1.00   bsv 02/10/2019 First release
*        bsv 04/09/2020 Code clean up
* 1.01   bsv 07/08/2020 Moved Ch9Handler APIs to xloader_dfu_util.c
*        skd 07/14/2020 XLoader_UsbCopy prototype changed
*        td  08/19/2020 Fixed MISRA C violations Rule 10.3
* 1.02   bsv 08/26/2021 Code clean up
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

/************************** Constant Definitions ****************************/
#define XLOADER_USB_DEVICE_ID		(XPAR_XUSBPSU_0_DEVICE_ID)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u8* DfuVirtFlash = (u8*)XLOADER_DDR_TEMP_BUFFER_ADDRESS;
u8 DownloadDone = 0U;

/*****************************************************************************/
/**
 * @brief	This function initializes the USB interface.
 *
 * @param	Device Flags are unused and only passed to maintain
 *		compatibility with init functions of other boot modes
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
*****************************************************************************/
int XLoader_UsbInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	Usb_Config *UsbConfigPtr;
	struct XUsbPsu UsbPrivateData;
	struct Usb_DevData UsbInstance;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;

	(void) DeviceFlags;
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_USB_0,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemSetBytes(&UsbInstance, sizeof(UsbInstance),
				0U, sizeof(UsbInstance));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_USB_INSTANCE);
		goto END;
	}
	Status = XPlmi_MemSetBytes(&UsbPrivateData, sizeof(struct XUsbPsu),
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

	UsbConfigPtr = XUsbPsu_LookupConfig(XLOADER_USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_USB_LOOKUP, Status);
		goto END;
	}

	UsbPrivateData.AppData = &UsbInstance;
	UsbInstance.PrivateData = (void*)&UsbPrivateData;

	Status = (int) XUsbPsu_CfgInitialize(
			(struct XUsbPsu*)UsbInstance.PrivateData,
			UsbConfigPtr, UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_USB_CFG, Status);
		goto END;
	}

	Status = XLoader_DdrInit(XLOADER_PDI_SRC_DDR);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Hook up chapter9 handler */
	XUsbPsu_set_ch9handler((struct XUsbPsu*)UsbInstance.PrivateData,
		XLoader_Ch9Handler);

	/* Set the reset event handler */
	XUsbPsu_set_rsthandler((struct XUsbPsu*)UsbInstance.PrivateData,
		XLoader_DfuReset);

	DfuObj.InstancePtr = &UsbInstance;

	/* Set DFU state to APP_IDLE */
	XLoader_DfuSetState(&UsbInstance, XLOADER_STATE_APP_IDLE);

	/* Assign the data to usb driver */
	XUsbPsu_set_drvdata((struct XUsbPsu*)UsbInstance.PrivateData, &Dfu_data);

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr((struct XUsbPsu*)UsbInstance.PrivateData,
		XUSBPSU_DEVTEN_EVNTOVERFLOWEN | XUSBPSU_DEVTEN_WKUPEVTEN
		| XUSBPSU_DEVTEN_ULSTCNGEN | XUSBPSU_DEVTEN_CONNECTDONEEN
		| XUSBPSU_DEVTEN_USBRSTEN | XUSBPSU_DEVTEN_DISCONNEVTEN);

	/* Start the controller so that Host can see our device */
	Status = XUsbPsu_Start((struct XUsbPsu*)UsbInstance.PrivateData);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_USB_START, Status);
		goto END;
	}

	while ((DownloadDone < XLOADER_DOWNLOAD_COMPLETE) && \
		(DfuObj.CurrStatus != XLOADER_STATE_DFU_ERROR)) {
		XUsbPsu_IntrHandler((struct XUsbPsu*)UsbInstance.PrivateData);
	}
	(void)XUsbPsu_Stop((struct XUsbPsu*)UsbInstance.PrivateData);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function copies from DFU temporary address in DDR to
* destination.
*
* @param	Source Address is the offset of the memory chunk to be copied.
*		This value is added to fixed DDR address to calculate the actual
*		DDR address where the image resides.
* @param	Destination Address is the address to which the memory chunk
		is to be copied.
* @param	Number of Bytes to be copied
* @param	Flags is unused and is only passed to maintain compatibility
*		with copy functions of other boot modes
*
* @return	XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
int XLoader_UsbCopy(u64 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;

	Status = XLoader_DdrCopy((SrcAddress + XLOADER_DDR_TEMP_BUFFER_ADDRESS),
			DestAddress, Length, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function releases control of USB.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_UsbRelease(void)
{
	int Status = XST_FAILURE;

	Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_USB_0,
		XPLMI_CMD_SECURE);

	return Status;
}
#endif
