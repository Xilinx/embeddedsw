/******************************************************************************
*
* Copyright (C) 2016 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xusbpsu.c
* @addtogroup usbpsu_v1_6
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
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbpsu.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* Waits until a bit in a register is cleared or timeout occurs
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
* @param	Offset is register offset.
* @param	BitMask is bit mask of required bit to be checked.
* @param	Timeout is the time to wait specified in micro seconds.
*
* @return
*			- XST_SUCCESS when bit is cleared.
*			- XST_FAILURE when timed out.
*
******************************************************************************/
s32 XUsbPsu_Wait_Clear_Timeout(struct XUsbPsu *InstancePtr, u32 Offset,
						u32 BitMask, u32 Timeout)
{
	u32 RegVal;
	u32 LocalTimeout = Timeout;

	while (LocalTimeout > 0U) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, Offset);
		if ((RegVal & BitMask) == 0U) {
			break;
		}

		LocalTimeout = LocalTimeout - 1U;

		XUsbSleep(1U);
	}

	if (LocalTimeout == 0U) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Waits until a bit in a register is set or timeout occurs
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
* @param	Offset is register offset.
* @param	BitMask is bit mask of required bit to be checked.
* @param	Timeout is the time to wait specified in micro seconds.
*
* @return
*			- XST_SUCCESS when bit is set.
*			- XST_FAILURE when timed out.
*
******************************************************************************/
s32 XUsbPsu_Wait_Set_Timeout(struct XUsbPsu *InstancePtr, u32 Offset,
						u32 BitMask, u32 Timeout)
{
	u32 RegVal;
	u32 LocalTimeout = Timeout;

	while (LocalTimeout > 0U) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, Offset);
		if ((RegVal & BitMask) != 0U) {
			break;
		}

		LocalTimeout = LocalTimeout - 1U;

		XUsbSleep(1U);
	}

	if (LocalTimeout == 0U) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}

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
******************************************************************************/
void XUsbPsu_SetMode(struct XUsbPsu *InstancePtr, u32 Mode)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Mode <= XUSBPSU_GCTL_PRTCAP_OTG) &&
					(Mode >= XUSBPSU_GCTL_PRTCAP_HOST));

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GCTL);
	RegVal &= ~(XUSBPSU_GCTL_PRTCAPDIR(XUSBPSU_GCTL_PRTCAP_OTG));
	RegVal |= XUSBPSU_GCTL_PRTCAPDIR(Mode);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GCTL, RegVal);
}

/*****************************************************************************/
/**
* This function puts the controller into idle state by stopping the transfers
* for all endpoints, stopping the usb core and clearing the event buffers.
* buffers.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
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
		u32 RegVal, ResIdx, Cmd;
		u32 EpNums, CurEpNum, InEpNums, OutEpNums, PhyEpNum;
		struct XUsbPsu_EpParams	*Params;
		struct XUsbPsu_Ep	*Ept;

		/* Read HwParams 3 for fetching the max number of eps */
		RegVal = XUsbPsu_ReadHwParams(InstancePtr, 3U);

		EpNums = XUSBPSU_NUM_EPS(RegVal);
		InEpNums = XUSBPSU_NUM_IN_EPS(RegVal);
		OutEpNums = EpNums - InEpNums;

		/* Stop transfers for Out Endpoints */
		for (CurEpNum = 0U; CurEpNum < OutEpNums; CurEpNum++) {

			PhyEpNum = XUSBPSU_PhysicalEp(CurEpNum,
					XUSBPSU_EP_DIR_OUT);

			ResIdx = XUsbPsu_EpGetTransferIndex(InstancePtr,
					(u8)CurEpNum, (u8)XUSBPSU_EP_DIR_OUT);

			Params = XUsbPsu_GetEpParams(InstancePtr);
			Xil_AssertVoid(Params != NULL);

			/* Issue EndTransfer WITH CMDIOC bit set */
			Cmd = XUSBPSU_DEPCMD_ENDTRANSFER;
			Cmd |= XUSBPSU_DEPCMD_HIPRI_FORCERM;
			Cmd |= XUSBPSU_DEPCMD_CMDIOC;
			Cmd |= XUSBPSU_DEPCMD_PARAM(ResIdx);
			(void)XUsbPsu_SendEpCmd(InstancePtr, (u8)CurEpNum,
					(u8)XUSBPSU_EP_DIR_OUT, Cmd, Params);

			Ept = &InstancePtr->eps[PhyEpNum];
			if (Ept != NULL) {
				Ept->ResourceIndex = 0U;
				Ept->EpStatus &= ~XUSBPSU_EP_BUSY;
			}

			/* Wait until CMD ACT bit is cleared */
			if (XUsbPsu_Wait_Clear_Timeout(InstancePtr,
						XUSBPSU_DEPCMD(PhyEpNum),
						XUSBPSU_DEPCMD_CMDACT,
						500U)	== XST_FAILURE) {
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

			ResIdx = XUsbPsu_EpGetTransferIndex(InstancePtr,
					(u8)CurEpNum, (u8)XUSBPSU_EP_DIR_IN);

			Params = XUsbPsu_GetEpParams(InstancePtr);
			Xil_AssertVoid(Params != NULL);

			/* Issue EndTransfer WITH CMDIOC bit set */
			Cmd = XUSBPSU_DEPCMD_ENDTRANSFER;
			Cmd |= XUSBPSU_DEPCMD_HIPRI_FORCERM;
			Cmd |= XUSBPSU_DEPCMD_CMDIOC;
			Cmd |= XUSBPSU_DEPCMD_PARAM(ResIdx);
			(void)XUsbPsu_SendEpCmd(InstancePtr, (u8)CurEpNum,
					(u8)XUSBPSU_EP_DIR_IN, Cmd, Params);

			Ept = &InstancePtr->eps[PhyEpNum];
			if (Ept != NULL) {
				Ept->ResourceIndex = 0U;
				Ept->EpStatus &= ~XUSBPSU_EP_BUSY;
			}

			/* Wait until CMD ACT bit is cleared */
			if (XUsbPsu_Wait_Clear_Timeout(InstancePtr,
						XUSBPSU_DEPCMD(PhyEpNum),
						XUSBPSU_DEPCMD_CMDACT,
						500U)	== XST_FAILURE) {
#ifdef XUSBPSU_DEBUG
				xil_printf(
				"End Transfer on Endpoint %dIN failed\n\r",
								CurEpNum);
#endif
			}
		}

		/* Stop transfers for Out Endpoints */
		for (CurEpNum = 0U; CurEpNum < OutEpNums; CurEpNum++) {

			PhyEpNum = XUSBPSU_PhysicalEp(CurEpNum,
							XUSBPSU_EP_DIR_OUT);

			RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DALEPENA);
			RegVal &= ~XUSBPSU_DALEPENA_EP(PhyEpNum);
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DALEPENA, RegVal);

			Ept = &InstancePtr->eps[PhyEpNum];

			if (Ept != NULL) {
				Ept->Type = 0U;
				Ept->EpStatus = 0U;
				Ept->MaxSize = 0U;
				Ept->TrbEnqueue	= 0U;
				Ept->TrbDequeue	= 0U;
			}

		}

		/* Stop transfers for In Endpoints */
		for (CurEpNum = 0U; CurEpNum < InEpNums; CurEpNum++) {

			PhyEpNum = XUSBPSU_PhysicalEp(CurEpNum,
							XUSBPSU_EP_DIR_IN);

			RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DALEPENA);
			RegVal &= ~XUSBPSU_DALEPENA_EP(PhyEpNum);
			XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DALEPENA, RegVal);

			Ept = &InstancePtr->eps[PhyEpNum];

			if (Ept != NULL) {
				Ept->Type = 0U;
				Ept->EpStatus = 0U;
				Ept->MaxSize = 0U;
				Ept->TrbEnqueue	= 0U;
				Ept->TrbDequeue	= 0U;
			}

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
* Issues core PHY reset.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XUsbPsu_PhyReset(struct XUsbPsu *InstancePtr)
{
	u32		RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

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

	XUsbSleep(XUSBPSU_PHY_TIMEOUT);

	/* Clear USB3 PHY reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U));
	RegVal &= ~XUSBPSU_GUSB3PIPECTL_PHYSOFTRST;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U), RegVal);

	/* Clear USB2 PHY reset */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U));
	RegVal &= ~XUSBPSU_GUSB2PHYCFG_PHYSOFTRST;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U), RegVal);

	XUsbSleep(XUSBPSU_PHY_TIMEOUT);

	/* Take Core out of reset state after PHYS are stable*/
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GCTL);
	RegVal &= ~XUSBPSU_GCTL_CORESOFTRESET;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GCTL, RegVal);
}

/*****************************************************************************/
/**
* Sets up Event buffers so that events are written by Core.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XUsbPsu_EventBuffersSetup(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_EvtBuffer *Evt;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EventBuffer != NULL);

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
* Resets Event buffer Registers to zero so that events are not written by Core.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XUsbPsu_EventBuffersReset(struct XUsbPsu *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

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
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked on.
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

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(RegIndex <= (u8)XUSBPSU_GHWPARAMS7);

	RegVal = XUsbPsu_ReadReg(InstancePtr, ((u32)XUSBPSU_GHWPARAMS0_OFFSET +
						((u32)RegIndex * (u32)4U)));
	return RegVal;
}

/*****************************************************************************/
/**
* Initializes Core.
*
* @param  InstancePtr is a pointer to the XUsbPsu instance to be worked on.
*
* @return
*		-XST_SUCCESS if initialization was successful
*		-XST_FAILURE if initialization was not successful
*
******************************************************************************/
s32 XUsbPsu_CoreInit(struct XUsbPsu *InstancePtr)
{
	u32		RegVal;
	u32		Hwparams1;

	/* issue device SoftReset too */
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, XUSBPSU_DCTL_CSFTRST);

	if (XUsbPsu_Wait_Clear_Timeout(InstancePtr, XUSBPSU_DCTL,
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
	if (InstancePtr->HasHibernation == TRUE) {
		XUsbPsu_InitHibernation(InstancePtr);
	}
#endif
	/* Set AXI-cache bits when CCI is Enable */
	if (InstancePtr->ConfigPtr->IsCacheCoherent == 1U) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GSBUSCFG0);
		RegVal |= XUSBPSU_GSBUSCFG0_BITMASK;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GSBUSCFG0, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Enables an interrupt in Event Enable RegValister.
*
* @param  InstancePtr is a pointer to the XUsbPsu instance to be worked on
* @param  Mask is the OR of any Interrupt Enable Masks:
*		- XUSBPSU_DEVTEN_VNDRDEVTSTRCVEDEN
*		- XUSBPSU_DEVTEN_EVNTOVERFLOWEN
*		- XUSBPSU_DEVTEN_CMDCMPLTEN
*		- XUSBPSU_DEVTEN_ERRTICERREN
*		- XUSBPSU_DEVTEN_SOFEN
*		- XUSBPSU_DEVTEN_EOPFEN
*		- XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN
*		- XUSBPSU_DEVTEN_WKUPEVTEN
*		- XUSBPSU_DEVTEN_ULSTCNGEN
*		- XUSBPSU_DEVTEN_CONNECTDONEEN
*		- XUSBPSU_DEVTEN_USBRSTEN
*		- XUSBPSU_DEVTEN_DISCONNEVTEN
*
* @return  None
*
******************************************************************************/
void XUsbPsu_EnableIntr(struct XUsbPsu *InstancePtr, u32 Mask)
{
	u32	RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DEVTEN);
	RegVal |= Mask;

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEVTEN, RegVal);
}

/*****************************************************************************/
/**
* Disables an interrupt in Event Enable RegValister.
*
* @param  InstancePtr is a pointer to the XUsbPsu instance to be worked on.
* @param  Mask is the OR of Interrupt Enable Masks
*		- XUSBPSU_DEVTEN_VNDRDEVTSTRCVEDEN
*		- XUSBPSU_DEVTEN_EVNTOVERFLOWEN
*		- XUSBPSU_DEVTEN_CMDCMPLTEN
*		- XUSBPSU_DEVTEN_ERRTICERREN
*		- XUSBPSU_DEVTEN_SOFEN
*		- XUSBPSU_DEVTEN_EOPFEN
*		- XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN
*		- XUSBPSU_DEVTEN_WKUPEVTEN
*		- XUSBPSU_DEVTEN_ULSTCNGEN
*		- XUSBPSU_DEVTEN_CONNECTDONEEN
*		- XUSBPSU_DEVTEN_USBRSTEN
*		- XUSBPSU_DEVTEN_DISCONNEVTEN
*
* @return  None
*
******************************************************************************/
void XUsbPsu_DisableIntr(struct XUsbPsu *InstancePtr, u32 Mask)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DEVTEN);
	RegVal &= ~Mask;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEVTEN, RegVal);
}

/****************************************************************************/
/**
*
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
	Xil_AssertNonvoid(BaseAddress != 0U)

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
	Speed = (ConfigPtr->EnableSuperSpeed == TRUE) ?
			XUSBPSU_DCFG_SUPERSPEED : XUSBPSU_DCFG_HIGHSPEED;

	/*
	 * Setting to max speed to support SS and HS
	 */
	XUsbPsu_SetSpeed(InstancePtr, Speed);

	(void)XUsbPsu_SetDeviceAddress(InstancePtr, 0U);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
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

	if (XUsbPsu_Wait_Clear_Timeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_DEVCTRLHLT, 500U) == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
*
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

	if (XUsbPsu_Wait_Set_Timeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_DEVCTRLHLT, 500U) == XST_FAILURE) {
		return (s32)XST_FAILURE;
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
	s32 Status = XST_SUCCESS;

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
		Status = XST_SUCCESS;
	}

	return Status;
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

	return XUSBPSU_DSTS_USBLNKST(RegVal);
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

	Xil_AssertNonvoid(InstancePtr != NULL);

	 /* Wait until device controller is ready. */
	if (XUsbPsu_Wait_Clear_Timeout(InstancePtr, XUSBPSU_DSTS,
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
* Set U1 sleep timeout
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Sleep is time in microseconds
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_SetU1SleepTimeout(struct XUsbPsu *InstancePtr, u8 Sleep)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_PORTMSC_30);
	RegVal &= ~XUSBPSU_PORTMSC_30_U1_TIMEOUT_MASK;
	RegVal |= (u32)(Sleep << XUSBPSU_PORTMSC_30_U1_TIMEOUT_SHIFT);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_PORTMSC_30, RegVal);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* Set U2 sleep timeout
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Sleep is time in microseconds
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note	None.
*
*****************************************************************************/
s32 XUsbPsu_SetU2SleepTimeout(struct XUsbPsu *InstancePtr, u8 Sleep)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_PORTMSC_30);
	RegVal &= ~XUSBPSU_PORTMSC_30_U2_TIMEOUT_MASK;
	RegVal |= (u32)(Sleep << XUSBPSU_PORTMSC_30_U2_TIMEOUT_SHIFT);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_PORTMSC_30, RegVal);

	return (s32)XST_SUCCESS;
}
/****************************************************************************/
/**
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
	if (InstancePtr->AppData->Speed != XUSBPSU_SPEED_SUPER) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}
/** @} */
