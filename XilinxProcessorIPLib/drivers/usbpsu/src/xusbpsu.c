/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu.c
* @addtogroup usbpsu_v1_8
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   sg    06/16/16 First release
* 1.1   sg    10/24/16 Added new function XUsbPsu_IsSuperSpeed
* 1.4	bk    12/01/18 Modify USBPSU driver code to fit USB common example code
*		       for all USB IPs.
*	myk   12/01/18 Added hibernation support for device mode
* 1.4	vak   30/05/18 Removed xusb_wrapper files
*	vak   24/09/18 Add support for connecting to host in high-speed
* 1.5	vak   02/06/19 Add API for idling usb controller
* 1.6	pm    22/07/19 Removed coverity warnings
*	pm    08/08/19 Added support to set AXI-Cache bits when CCI is enable
*	pm    28/08/19 Removed 80-character warnings
* 1.7	pm    02/20/20 Add support to set CCI bit in Coherency Mode Register
*			when CCI is enable
* 	pm    03/23/20 Restructured the code for more readability and modularity
* 1.8	pm    24/07/20 Fixed MISRA-C and Coverity warnings
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbpsu_local.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
static INLINE void XUsbPsu_SetMode(struct XUsbPsu *InstancePtr, u32 Mode);
static void XUsbPsu_InitializeEps(struct XUsbPsu *InstancePtr);

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* Sets mode of Core to USB Device/Host/OTG.
*
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
* @param	Mode is mode to set
*			- XUSBPSU_GCTL_PRTCAP_OTG
*			- XUSBPSU_GCTL_PRTCAP_HOST
*			- XUSBPSU_GCTL_PRTCAP_DEVICE
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static INLINE void XUsbPsu_SetMode(struct XUsbPsu *InstancePtr, u32 Mode)
{
	u32 RegVal;

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GCTL);
	RegVal &= ~(XUSBPSU_GCTL_PRTCAPDIR(XUSBPSU_GCTL_PRTCAP_OTG));
	RegVal |= XUSBPSU_GCTL_PRTCAPDIR(Mode);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GCTL, RegVal);
}

/****************************************************************************/
/**
* Initializes Endpoints. All OUT endpoints are even numbered and all IN
* endpoints are odd numbered. EP0 is for Control OUT and EP1 is for
* Control IN.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void XUsbPsu_InitializeEps(struct XUsbPsu *InstancePtr)
{
	u8  i;
	u8 Epnum;

	for (i = 0U; i < InstancePtr->NumOutEps; i++) {
		Epnum = (i << 1U) | XUSBPSU_EP_DIR_OUT;
		InstancePtr->eps[Epnum].PhyEpNum = Epnum;
		InstancePtr->eps[Epnum].Direction = XUSBPSU_EP_DIR_OUT;
		InstancePtr->eps[Epnum].ResourceIndex = 0U;
	}
	for (i = 0U; i < InstancePtr->NumInEps; i++) {
		Epnum = (i << 1U) | XUSBPSU_EP_DIR_IN;
		InstancePtr->eps[Epnum].PhyEpNum = Epnum;
		InstancePtr->eps[Epnum].Direction = XUSBPSU_EP_DIR_IN;
		InstancePtr->eps[Epnum].ResourceIndex = 0U;
	}
}

/****************************************************************************/
/**
* @brief
* This function does the following:
*	- initializes a specific XUsbPsu instance.
*	- sets up Event Buffer for Core to write events.
*	- Core Reset and PHY Reset.
*	- Sets core in Device Mode.
*	- Sets default speed as HIGH_SPEED.
*	- Sets Device Address to 0.
*	- Enables interrupts.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	ConfigPtr points to the XUsbPsu device configuration structure.
* @param	BaseAddress is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_CfgInitialize(struct XUsbPsu *InstancePtr,
				XUsbPsu_Config *ConfigPtr, u32 BaseAddress)
{
	s32 Status;
	u32 RegVal;
	u32 Speed;


	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr   != NULL);
	Xil_AssertNonvoid(BaseAddress != 0U);

	InstancePtr->ConfigPtr = ConfigPtr;

	Status = XUsbPsu_CoreInit(InstancePtr);
	if (Status != XST_SUCCESS) {
#ifdef XUSBPSU_DEBUG
		xil_printf("Core initialization failed\r\n");
#endif
		return (s32)XST_FAILURE;
	}

	RegVal = XUsbPsu_ReadHwParams(InstancePtr, 3U);
	InstancePtr->NumInEps = (u8)XUSBPSU_NUM_IN_EPS(RegVal);
	InstancePtr->NumOutEps = (u8)(XUSBPSU_NUM_EPS(RegVal) -
			InstancePtr->NumInEps);

	/* Map USB and Physical Endpoints */
	XUsbPsu_InitializeEps(InstancePtr);

	XUsbPsu_EventBuffersSetup(InstancePtr);

	XUsbPsu_SetMode(InstancePtr, XUSBPSU_GCTL_PRTCAP_DEVICE);

	/*
	 * Set connection speed based on EnableSuperSpeed parameter
	 */
	Speed = (ConfigPtr->EnableSuperSpeed == (u8)TRUE) ?
			XUSBPSU_DCFG_SUPERSPEED : XUSBPSU_DCFG_HIGHSPEED;

	/*
	 * Setting to max speed to support SS and HS
	 */
	XUsbPsu_SetSpeed(InstancePtr, Speed);

	(void)XUsbPsu_SetDeviceAddress(InstancePtr, 0U);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
*
* @brief
* Starts the controller so that Host can detect this device.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_Start(struct XUsbPsu *InstancePtr)
{
	u32	RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);

	RegVal |= XUSBPSU_DCTL_RUN_STOP;

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_DEVCTRLHLT, 500U) == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
*
* @brief
* Stops the controller so that Device disconnects from Host.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_Stop(struct XUsbPsu *InstancePtr)
{
	u32	RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_RUN_STOP;

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	if (XUsbPsu_WaitSetTimeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_DEVCTRLHLT, 500U) == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
 * Gets current State of USB Link
 *
 * @param	InstancePtr is a pointer to the XUsbPsu instance.
 *
 * @return	Link State
 *
 * @note	None.
 *
 ****************************************************************************/
u8 XUsbPsu_GetLinkState(struct XUsbPsu *InstancePtr)
{
	u32		RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DSTS);

	return (u8)XUSBPSU_DSTS_USBLNKST(RegVal);
}

/****************************************************************************/
/**
 * Sets USB Link to a particular State
 *
 * @param	InstancePtr is a pointer to the XUsbPsu instance.
 * @param	State is State of Link to set.
 *
 * @return	XST_SUCCESS else XST_FAILURE
 *
 * @note	None.
 *
 ****************************************************************************/
s32 XUsbPsu_SetLinkState(struct XUsbPsu *InstancePtr,
		XusbPsuLinkStateChange State)
{
	u32		RegVal;

	 /* Wait until device controller is ready. */
	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_DCNRD, 500U) == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_ULSTCHNGREQ_MASK;

	RegVal |= XUSBPSU_DCTL_ULSTCHNGREQ(State);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Set U1 sleep timeout
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Timeout is time in microseconds
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_SetU1SleepTimeout(struct XUsbPsu *InstancePtr, u8 Timeout)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_PORTMSC_30);
	RegVal &= ~XUSBPSU_PORTMSC_30_U1_TIMEOUT_MASK;
	RegVal |= ((u32)Timeout << XUSBPSU_PORTMSC_30_U1_TIMEOUT_SHIFT);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_PORTMSC_30, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Set U2 sleep timeout
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Timeout is time in microseconds
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_SetU2SleepTimeout(struct XUsbPsu *InstancePtr, u8 Timeout)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_PORTMSC_30);
	RegVal &= ~XUSBPSU_PORTMSC_30_U2_TIMEOUT_MASK;
	RegVal |= ((u32)Timeout << XUSBPSU_PORTMSC_30_U2_TIMEOUT_SHIFT);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_PORTMSC_30, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Enable Accept U1 and U2 sleep enable
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_AcceptU1U2Sleep(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal |= XUSBPSU_DCTL_ACCEPTU2ENA | XUSBPSU_DCTL_ACCEPTU1ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Enable U1 enable sleep
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_U1SleepEnable(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal |= XUSBPSU_DCTL_INITU1ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Enable U2 enable sleep
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_U2SleepEnable(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal |= XUSBPSU_DCTL_INITU2ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Enable U1 disable sleep
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_U1SleepDisable(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_INITU1ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Enable U2 disable sleep
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_U2SleepDisable(struct XUsbPsu *InstancePtr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_INITU2ENA;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Checks if the current speed is Super Speed or not
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_IsSuperSpeed(struct XUsbPsu *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->AppData->Speed != XUSBPSU_SPEED_SUPER) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}
/** @} */
