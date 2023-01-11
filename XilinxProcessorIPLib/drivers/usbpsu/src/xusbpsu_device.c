/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_device.c
* @addtogroup usbpsu USBPSU v1.12
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   pm    03/03/20 First release
* 1.7 	pm    25/03/20 Add clocking support
* 1.8	pm    22/08/20 Configure register bits when CCI is enable
*	pm    24/08/20 Fixed MISRA-C and Coverity warnings
* 1.12	pm    10/08/22 Update doxygen tag and addtogroup version
* 1.13	pm    04/01/23 Use Xil_WaitForEvent() API for register bit polling
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbpsu_local.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
static INLINE void XUsbPsu_EventBuffersReset(struct XUsbPsu *InstancePtr);
void XUsbPsu_PhyReset(struct XUsbPsu *InstancePtr);

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* Waits until a bit in a register is cleared or timeout occurs
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on
* @param	Offset is register offset.
* @param	BitMask is bit mask of required bit to be checked.
* @param	Timeout is the time to wait specified in micro seconds.
*
* @return
*			- XST_SUCCESS when bit is cleared.
*			- XST_FAILURE when timed out.
*
******************************************************************************/
s32 XUsbPsu_WaitClearTimeout(struct XUsbPsu *InstancePtr, u32 Offset,
		u32 BitMask, u32 Timeout)
{
	if (Xil_WaitForEvent(InstancePtr->ConfigPtr->BaseAddress + Offset,
			     BitMask, 0x00U, Timeout) != (u32)XST_SUCCESS)
		return (s32)XST_FAILURE;

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Waits until a bit in a register is set or timeout occurs
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on
* @param	Offset is register offset.
* @param	BitMask is bit mask of required bit to be checked.
* @param	Timeout is the time to wait specified in micro seconds.
*
* @return
*			- XST_SUCCESS when bit is set.
*			- XST_FAILURE when timed out.
*
******************************************************************************/
s32 XUsbPsu_WaitSetTimeout(struct XUsbPsu *InstancePtr, u32 Offset,
		u32 BitMask, u32 Timeout)
{
	if (Xil_WaitForEvent(InstancePtr->ConfigPtr->BaseAddress + Offset,
			     BitMask, BitMask, Timeout) != (u32)XST_SUCCESS)
		return (s32)XST_FAILURE;

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Resets Event buffer Registers to zero so that events are not written by Core.
*
* @param    InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return   None
*
****************************************************************************/
static INLINE void XUsbPsu_EventBuffersReset(struct XUsbPsu *InstancePtr)
{
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTADRLO(0U), 0U);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTADRHI(0U), 0U);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTSIZ(0U),
			(u32)XUSBPSU_GEVNTSIZ_INTMASK |
						XUSBPSU_GEVNTSIZ_SIZE(0U));
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U), 0U);
}

/*****************************************************************************/
/**
* Reads data from Hardware Params Registers of Core.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on
* @param	RegIndex is Register number to read
*			- XUSBPSU_GHWPARAMS0
*			- XUSBPSU_GHWPARAMS1
*			- XUSBPSU_GHWPARAMS2
*			- XUSBPSU_GHWPARAMS3
*			- XUSBPSU_GHWPARAMS4
*			- XUSBPSU_GHWPARAMS5
*			- XUSBPSU_GHWPARAMS6
*			- XUSBPSU_GHWPARAMS7
*
* @return	One of the GHWPARAMS RegValister contents.
*
******************************************************************************/
u32 XUsbPsu_ReadHwParams(struct XUsbPsu *InstancePtr, u8 RegIndex)
{
	u32 RegVal;

	Xil_AssertNonvoid(RegIndex <= (u8)XUSBPSU_GHWPARAMS7);

	RegVal = XUsbPsu_ReadReg(InstancePtr, ((u32)XUSBPSU_GHWPARAMS0_OFFSET +
				((u32)RegIndex * (u32)4U)));
	return RegVal;
}

/*****************************************************************************/
/**
* Issues core PHY reset.
*
* @param   InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return  None
*
******************************************************************************/
void XUsbPsu_PhyReset(struct XUsbPsu *InstancePtr)
{
	u32             RegVal;

	/* Before Resetting PHY, put Core in Reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GCTL);
	RegVal |= XUSBPSU_GCTL_CORESOFTRESET;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GCTL, RegVal);

	/* Assert USB3 PHY reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U));
	RegVal |= XUSBPSU_GUSB3PIPECTL_PHYSOFTRST;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U), RegVal);

	/* Assert USB2 PHY reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U));
	RegVal |= XUSBPSU_GUSB2PHYCFG_PHYSOFTRST;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U), RegVal);

	XUsbPsu_Sleep(XUSBPSU_PHY_TIMEOUT);

	/* Clear USB3 PHY reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U));
	RegVal &= ~XUSBPSU_GUSB3PIPECTL_PHYSOFTRST;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U), RegVal);

	/* Clear USB2 PHY reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U));
	RegVal &= ~XUSBPSU_GUSB2PHYCFG_PHYSOFTRST;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U), RegVal);

	XUsbPsu_Sleep(XUSBPSU_PHY_TIMEOUT);

	/* Take Core out of reset state after PHYS are stable*/
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GCTL);
	RegVal &= ~XUSBPSU_GCTL_CORESOFTRESET;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GCTL, RegVal);
}

/****************************************************************************/
/**
* @brief
* Sets speed of the Core for connecting to Host
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Speed is required speed
*				- XUSBPSU_DCFG_HIGHSPEED
*				- XUSBPSU_DCFG_FULLSPEED2
*				- XUSBPSU_DCFG_LOWSPEED
*				- XUSBPSU_DCFG_FULLSPEED1
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_SetSpeed(struct XUsbPsu *InstancePtr, u32 Speed)
{
	u32	RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Speed <= (u32)XUSBPSU_DCFG_SUPERSPEED);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCFG);
	RegVal &= ~(XUSBPSU_DCFG_SPEED_MASK);
	RegVal |= Speed;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCFG, RegVal);
}

/****************************************************************************/
/**
* @brief
* Sets Device Address of the Core
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Addr is address to set.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_SetDeviceAddress(struct XUsbPsu *InstancePtr, u16 Addr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Addr <= 127U);

	if (InstancePtr->AppData->State == XUSBPSU_STATE_CONFIGURED) {
		return (s32)XST_FAILURE;
	}

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCFG);
	RegVal &= ~(XUSBPSU_DCFG_DEVADDR_MASK);
	RegVal |= XUSBPSU_DCFG_DEVADDR(Addr);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCFG, RegVal);

	if (Addr > 0U) {
		InstancePtr->AppData->State = XUSBPSU_STATE_ADDRESS;
	} else {
		InstancePtr->AppData->State = XUSBPSU_STATE_DEFAULT;
	}

	return (s32)XST_SUCCESS;
}


/****************************************************************************/
/**
 * Enables USB2 Test Modes
 *
 * @param	InstancePtr is a pointer to the XUsbPsu instance.
 * @param	Mode is Test mode to set.
 *
 * @return	XST_SUCCESS else XST_FAILURE
 *
 * @note	None.
 *
 ****************************************************************************/
s32 XUsbPsu_SetTestMode(struct XUsbPsu *InstancePtr, u32 Mode)
{
	u32	RegVal;
	s32 Status = (s32)XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal &= ~XUSBPSU_DCTL_TSTCTRL_MASK;

	switch (Mode) {

	case XUSBPSU_TEST_J:
	case XUSBPSU_TEST_K:
	case XUSBPSU_TEST_SE0_NAK:
	case XUSBPSU_TEST_PACKET:
	case XUSBPSU_TEST_FORCE_ENABLE:
		RegVal |= (u32)Mode << 1;
		break;

	default:
		Status = (s32)XST_FAILURE;
		break;
	}

	if (Status != (s32)XST_FAILURE) {
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
		Status = (s32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function puts the controller into idle state by stopping the transfers
* for all endpoints, stopping the usb core and clearing the event buffers.
* buffers.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on
*
* @return	None
*
******************************************************************************/
void XUsbPsu_Idle(struct XUsbPsu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Stop the transfers when in peripheral mode */
	if ((XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GSTS) &
				XUSBPSU_GSTS_CUR_MODE) == 0U) {
		u32 RegVal;
		u32 EpNums, CurEpNum, InEpNums, OutEpNums, PhyEpNum;

		/* Read HwParams 3 for fetching the max number of eps */
		RegVal = XUsbPsu_ReadHwParams(InstancePtr, 3U);

		EpNums = XUSBPSU_NUM_EPS(RegVal);
		InEpNums = XUSBPSU_NUM_IN_EPS(RegVal);
		OutEpNums = EpNums - InEpNums;

		/* Stop transfers for Out Endpoints */
		for (CurEpNum = 0U; CurEpNum < OutEpNums; CurEpNum++) {

			PhyEpNum = XUSBPSU_PhysicalEp(CurEpNum,
					XUSBPSU_EP_DIR_OUT);

			XUsbPsu_StopTransfer(InstancePtr, (u8)CurEpNum,
					XUSBPSU_EP_DIR_OUT, (u8)FALSE);

			/* Wait until CMD ACT bit is cleared */
			if (XUsbPsu_WaitClearTimeout(InstancePtr,
					XUSBPSU_DEPCMD(PhyEpNum),
					XUSBPSU_DEPCMD_CMDACT, 500U) ==
					XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
				xil_printf(
				"End Transfer on Endpoint %dOUT failed\n\r",
					CurEpNum);
#endif
			}
		}

		/* Stop transfers for In Endpoints */
		for (CurEpNum = 0U; CurEpNum < InEpNums; CurEpNum++) {

			PhyEpNum = XUSBPSU_PhysicalEp(CurEpNum,
					XUSBPSU_EP_DIR_IN);

			XUsbPsu_StopTransfer(InstancePtr, (u8)CurEpNum,
					XUSBPSU_EP_DIR_IN, (u8)FALSE);

			/* Wait until CMD ACT bit is cleared */
			if (XUsbPsu_WaitClearTimeout(InstancePtr,
					XUSBPSU_DEPCMD(PhyEpNum),
					XUSBPSU_DEPCMD_CMDACT, 500U) ==
					XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
				xil_printf(
				"End Transfer on Endpoint %dIN failed\n\r",
					CurEpNum);
#endif
			}
		}

		/* Stop transfers for Out Endpoints */
		for (CurEpNum = 0U; CurEpNum < OutEpNums; CurEpNum++) {

			XUsbPsu_EpTransferDeactive(InstancePtr, (u8)CurEpNum,
					XUSBPSU_EP_DIR_OUT);

		}

		/* Stop transfers for In Endpoints */
		for (CurEpNum = 0U; CurEpNum < InEpNums; CurEpNum++) {

			XUsbPsu_EpTransferDeactive(InstancePtr, (u8)CurEpNum,
					XUSBPSU_EP_DIR_IN);

		}

		/* Stop the USB core */
		if (XUsbPsu_Stop(InstancePtr) == XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
			xil_printf("Failed to stop USB core\r\n");
#endif
		}

		/* Reset the Event buffers to 0 */
		XUsbPsu_EventBuffersReset(InstancePtr);
	}
}

/*****************************************************************************/
/**
* Initializes Core.
*
* @param  InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return
*	 	- XST_SUCCESS if initialization was successful
*		- XST_FAILURE if initialization was not successful
*
*******************************************************************************/
s32 XUsbPsu_CoreInit(struct XUsbPsu *InstancePtr)
{
	u32		RegVal;
	u32		Hwparams1;

	/* issue device SoftReset too */
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, XUSBPSU_DCTL_CSFTRST);

	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DCTL,
				XUSBPSU_DCTL_CSFTRST, 500U) == XST_FAILURE) {
		/* timed out return failure */
		return (s32)XST_FAILURE;
	}

	XUsbPsu_PhyReset(InstancePtr);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GCTL);
	RegVal &= ~XUSBPSU_GCTL_SCALEDOWN_MASK;
	RegVal &= ~XUSBPSU_GCTL_DISSCRAMBLE;
	RegVal |= XUSBPSU_GCTL_U2EXIT_LFPS;

	Hwparams1 = XUsbPsu_ReadHwParams(InstancePtr, 1U);

	switch (XUSBPSU_GHWPARAMS1_EN_PWROPT(Hwparams1)) {
	case XUSBPSU_GHWPARAMS1_EN_PWROPT_CLK:
		RegVal &= ~XUSBPSU_GCTL_DSBLCLKGTNG;
		break;

	case XUSBPSU_GHWPARAMS1_EN_PWROPT_HIB:
		/* enable ref clocks */
#if defined (XCLOCKING)
		Xil_ClockEnable(InstancePtr->ConfigPtr->RefClk);
#endif
		/* enable hibernation here */
#ifdef XUSBPSU_HIBERNATION_ENABLE
		RegVal |= XUSBPSU_GCTL_GBLHIBERNATIONEN;
		InstancePtr->HasHibernation = 1U;
#endif
		break;

	default:
		/* Made for Misra-C Compliance. */
		break;
	}

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GCTL, RegVal);

#ifdef XUSBPSU_HIBERNATION_ENABLE
	if (InstancePtr->HasHibernation == (u8)TRUE) {
		if (XUsbPsu_InitHibernation(InstancePtr) == XST_FAILURE) {
			return (s32)XST_FAILURE;
		}
	}
#endif

	/* Set AXI-cache bits when CCI is Enable */
	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)1U) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GSBUSCFG0);
		RegVal |= XUSBPSU_GSBUSCFG0_BITMASK;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GSBUSCFG0, RegVal);

		XUsbPsu_WriteVendorReg(XUSBPSU_COHERENCY,
					XUSBPSU_COHERENCY_MODE_ENABLE);
	}

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Sets up Event buffers so that events are written by Core.
*
* @param    InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return   None
*
****************************************************************************/
void XUsbPsu_EventBuffersSetup(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_EvtBuffer *Evt;

	Evt = &InstancePtr->Evt;
	Evt->BuffAddr = (void *)InstancePtr->EventBuffer;
	Evt->Offset = 0U;

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTADRLO(0U),
			(UINTPTR)InstancePtr->EventBuffer);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTADRHI(0U),
			((UINTPTR)(InstancePtr->EventBuffer) >> 16U) >> 16U);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTSIZ(0U),
			XUSBPSU_GEVNTSIZ_SIZE(
					sizeof(InstancePtr->EventBuffer)));
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U), 0U);
}

/*****************************************************************************/
/**
* @brief
* API for Sleep routine.
*
* @param	USeconds is time in MicroSeconds.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_Sleep(u32 USeconds)
{
	(void)usleep(USeconds);
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/*****************************************************************************/
/**
* Sets scratchpad buffers
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	XST_SUCCESS on success or else error code
*
* @note		None.
*
******************************************************************************/
s32 XUsbPsu_SetupScratchpad(struct XUsbPsu *InstancePtr, u8 *ScratchBuf)
{
	s32 Ret;
	Ret = XUsbPsu_SendGadgetGenericCmd(InstancePtr,
			XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_LO,
				(u32)((UINTPTR)ScratchBuf & 0xFFFFFFFFU));
	if (Ret == XST_FAILURE) {
		xil_printf("Failed to set scratchpad low addr: %d\n", Ret);
		return Ret;
	}

	Ret = XUsbPsu_SendGadgetGenericCmd(InstancePtr,
			XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_HI,
				(u32)(((UINTPTR)ScratchBuf >> 16U) >> 16U));
	if (Ret == XST_FAILURE) {
		xil_printf("Failed to set scratchpad high addr: %d\n", Ret);
		return Ret;
	}

	return (s32)XST_SUCCESS;
}

#endif
/** @} */
