/******************************************************************************
*
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xusbpsu_hibernation.c
*
* This patch adds hibernation support to usbpsu driver when dwc3 is operating
* as a gadget
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver    Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   Mayank 12/01/18 First release
* 1.5   VAK    14/03/19 Enable hibernation related functions only when
*                       XUSBPSU_HIBERNATION_ENABLE is defined
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbpsu.h"
#include "xusbpsu_hw.h"
#include "xusbpsu_endpoint.h"

#ifdef XUSBPSU_HIBERNATION_ENABLE

/************************** Constant Definitions *****************************/

#define NUM_OF_NONSTICKY_REGS    		27U

#define XUSBPSU_HIBER_SCRATCHBUF_SIZE           4096U

#define XUSBPSU_NON_STICKY_SAVE_RETRIES		500U
#define XUSBPSU_PWR_STATE_RETRIES		1500U
#define XUSBPSU_CTRL_RDY_RETRIES		5000U
#define XUSBPSU_TIMEOUT				1000U

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

static u8 ScratchBuf[XUSBPSU_HIBER_SCRATCHBUF_SIZE];

/* Registers saved during hibernation and restored at wakeup */
static u32 save_reg_addr[] = {
        XUSBPSU_DCTL,
        XUSBPSU_DCFG,
        XUSBPSU_DEVTEN,
        XUSBPSU_GSBUSCFG0,
        XUSBPSU_GSBUSCFG1,
        XUSBPSU_GCTL,
        XUSBPSU_GTXTHRCFG,
        XUSBPSU_GRXTHRCFG,
        XUSBPSU_GTXFIFOSIZ(0U),
        XUSBPSU_GTXFIFOSIZ(1U),
        XUSBPSU_GTXFIFOSIZ(2U),
        XUSBPSU_GTXFIFOSIZ(3U),
        XUSBPSU_GTXFIFOSIZ(4U),
        XUSBPSU_GTXFIFOSIZ(5U),
        XUSBPSU_GTXFIFOSIZ(6U),
        XUSBPSU_GTXFIFOSIZ(7U),
        XUSBPSU_GTXFIFOSIZ(8U),
        XUSBPSU_GTXFIFOSIZ(9U),
        XUSBPSU_GTXFIFOSIZ(10U),
        XUSBPSU_GTXFIFOSIZ(11U),
        XUSBPSU_GTXFIFOSIZ(12U),
        XUSBPSU_GTXFIFOSIZ(13U),
        XUSBPSU_GTXFIFOSIZ(14U),
        XUSBPSU_GTXFIFOSIZ(15U),
        XUSBPSU_GRXFIFOSIZ(0U),
        XUSBPSU_GUSB3PIPECTL(0U),
        XUSBPSU_GUSB2PHYCFG(0U),
};
static u32 saved_regs[NUM_OF_NONSTICKY_REGS];

/*****************************************************************************/
/**
* Save non sticky registers
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void save_regs(struct XUsbPsu *InstancePtr)
{
	u32 i;

	for (i = 0U; i < NUM_OF_NONSTICKY_REGS; i++) {
		saved_regs[i] = XUsbPsu_ReadReg(InstancePtr, save_reg_addr[i]);
	}
}

/*****************************************************************************/
/**
* Restore non sticky registers
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void restore_regs(struct XUsbPsu *InstancePtr)
{
	u32 i;

	for (i = 0U; i < NUM_OF_NONSTICKY_REGS; i++) {
		XUsbPsu_WriteReg(InstancePtr, save_reg_addr[i], saved_regs[i]);
	}
}

/*****************************************************************************/
/**
* Send generic command for gadget
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
* @param	cmd is command to be sent
* @param	param is parameter for the command, to be written in DGCMDPAR
* 		register
*
* @return
*		- XST_SUCCESS on success
*		- XST_FAILURE on timeout
*		- XST_REGISTER_ERROR on status error
*
* @note		None.
*
******************************************************************************/
s32 XUsbPsu_SendGadgetGenericCmd(struct XUsbPsu *InstancePtr, u32 cmd,
			u32 param)
{
	u32		RegVal, retry = 500U;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DGCMDPAR, param);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DGCMD, cmd | XUSBPSU_DGCMD_CMDACT);

	while (retry > 0U) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DGCMD);
		if ((RegVal & XUSBPSU_DGCMD_CMDACT) == (u32)0U) {
			if (XUSBPSU_DGCMD_STATUS(RegVal) != (u32)0U) {
				return (s32)XST_REGISTER_ERROR;
			}
			return (s32)XST_SUCCESS;
		}

		retry = retry - 1U;
	}

	return (s32)XST_FAILURE;
}

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
s32 XUsbPsu_SetupScratchpad(struct XUsbPsu *InstancePtr)
{
	s32 Ret;
	Ret = XUsbPsu_SendGadgetGenericCmd(InstancePtr,
		XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_LO, (UINTPTR)ScratchBuf & 0xFFFFFFFFU);
	if (Ret == XST_FAILURE) {
		xil_printf("Failed to set scratchpad low addr: %d\n", Ret);
		return Ret;
	}

	Ret = XUsbPsu_SendGadgetGenericCmd(InstancePtr,
		XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_HI, ((UINTPTR)ScratchBuf >> 16U) >> 16U);
	if (Ret == XST_FAILURE) {
		xil_printf("Failed to set scratchpad high addr: %d\n", Ret);
		return Ret;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Initialize to handle hibernation event when it comes
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	none
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_InitHibernation(struct XUsbPsu *InstancePtr)
{
	u32		RegVal;

	InstancePtr->IsHibernated = 0U;

	memset(ScratchBuf, 0U, sizeof(ScratchBuf));
	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)ScratchBuf, XUSBPSU_HIBER_SCRATCHBUF_SIZE);
	}

	XUsbPsu_SetupScratchpad(InstancePtr);

	/* enable PHY suspend */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U));
	RegVal |= XUSBPSU_GUSB2PHYCFG_SUSPHY;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB2PHYCFG(0U), RegVal);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U));
	RegVal |= XUSBPSU_GUSB3PIPECTL_SUSPHY;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GUSB3PIPECTL(0U), RegVal);
}

/*****************************************************************************/
/**
* Handle hibernation event
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	none
*
* @note		None.
*
******************************************************************************/
void Xusbpsu_HibernationIntr(struct XUsbPsu *InstancePtr)
{
	u8 EpNum;
	u32 RegVal;
	u32 retries;
	XusbPsuLinkState LinkState;

	/* sanity check */
	switch(XUsbPsu_GetLinkState(InstancePtr)) {
	case XUSBPSU_LINK_STATE_SS_DIS:
	case XUSBPSU_LINK_STATE_U3:
		break;
	default:
		/* fake hiber interrupt */
		xil_printf("got fake interrupt\r\n");
		return;
	};

	if (InstancePtr->Ep0State == XUSBPSU_EP0_SETUP_PHASE) {
		XUsbPsu_StopTransfer(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT, TRUE);
		XUsbPsu_RecvSetup(InstancePtr);
	}

	/* stop active transfers for all endpoints including control
	 * endpoints force rm bit should be 0 when we do this */
	for (EpNum = 0U; EpNum < XUSBPSU_ENDPOINTS_NUM; EpNum++) {
		struct XUsbPsu_Ep *Ept;

		Ept = &InstancePtr->eps[EpNum];
		if (Ept == NULL) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		/* save srsource index for later use */
		XUsbPsu_StopTransfer(InstancePtr, Ept->UsbEpNum,
				Ept->Direction, FALSE);

		XUsbPsu_SaveEndpointState(InstancePtr, Ept);
	}

	/*
	 * ack events, don't process them; h/w decrements the count by the value
	 * written
	 */
	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U));
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_GEVNTCOUNT(0U), RegVal);
	InstancePtr->Evt.Count = 0U;
	InstancePtr->Evt.Flags &= ~XUSBPSU_EVENT_PENDING;

	if (XUsbPsu_Stop(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to stop USB core\r\n");
		return;
	}

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);

	/* Check the link state and if it is disconnected, set
	 * KEEP_CONNECT to 0
	 */
	LinkState = XUsbPsu_GetLinkState(InstancePtr);
	if (LinkState == XUSBPSU_LINK_STATE_SS_DIS) {
		RegVal &= ~XUSBPSU_DCTL_KEEP_CONNECT;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

		/* update LinkState to be used while wakeup */
		InstancePtr->LinkState = XUSBPSU_LINK_STATE_SS_DIS;
	}

	save_regs(InstancePtr);

	/* ask core to save state */
	RegVal |= XUSBPSU_DCTL_CSS;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	/* wait till core saves */
	if (XUsbPsu_Wait_Clear_Timeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_SSS, XUSBPSU_NON_STICKY_SAVE_RETRIES) == XST_FAILURE) {
		xil_printf("Failed to save core state\r\n");
		return;
	}

	/* Enable PME to wakeup from hibernation */
	XUsbPsu_WriteVendorReg(XIL_PME_ENABLE, XIL_PME_ENABLE_SIG_GEN);

	/* change power state to D3 */
	XUsbPsu_WriteVendorReg(XIL_REQ_PWR_STATE, XIL_REQ_PWR_STATE_D3);

	/* wait till current state is changed to D3 */
        retries = (u32)XUSBPSU_PWR_STATE_RETRIES;

        while (retries > 0U) {
		RegVal = XUsbPsu_ReadVendorReg(XIL_CUR_PWR_STATE);
                if ((RegVal & XIL_CUR_PWR_STATE_BITMASK) == XIL_CUR_PWR_STATE_D3) {
                        break;
                }

                XUsbSleep(XUSBPSU_TIMEOUT);
                retries = retries - 1U;
        }

	if (retries == 0U) {
		xil_printf("Failed to change power state to D3\r\n");
		return;
	}
	XUsbSleep(XUSBPSU_TIMEOUT);

	RegVal = XUsbPsu_ReadLpdReg(RST_LPD_TOP);
	if (InstancePtr->ConfigPtr->DeviceId == (u16)XPAR_XUSBPSU_0_DEVICE_ID) {
		XUsbPsu_WriteLpdReg(RST_LPD_TOP, RegVal | (u32)USB0_CORE_RST);
	}

	InstancePtr->IsHibernated = 1U;
	xil_printf("Hibernated!\r\n");
}

/*****************************************************************************/
/**
* Restarts transfer for active endpoint
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
* @param	EpNum is an endpoint number.
*
* @return	XST_SUCCESS on success or else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static s32 XUsbPsu_RestartEp(struct XUsbPsu *InstancePtr, u8 EpNum)
{
	struct XUsbPsu_EpParams *Params;
	struct XUsbPsu_Trb	*TrbPtr;
	struct XUsbPsu_Ep	*Ept;
	u32	Cmd;
	s32	Ret;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	Ept = &InstancePtr->eps[EpNum];

	/* check if we need to restart transfer */
	if ((Ept->ResourceIndex == (u32)0U) && (Ept->PhyEpNum != (u32)0U)) {
		return XST_SUCCESS;
	}

	if (Ept->UsbEpNum != (u32)0U) {
		TrbPtr = &Ept->EpTrb[Ept->TrbDequeue];
	} else {
		TrbPtr = &InstancePtr->Ep0_Trb;
	}

	Xil_AssertNonvoid(TrbPtr != NULL);

	TrbPtr->Ctrl |= XUSBPSU_TRB_CTRL_HWO;

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr, sizeof(struct XUsbPsu_Trb));
		Xil_DCacheInvalidateRange((INTPTR)Ept->BufferPtr, Ept->RequestedBytes);
	}

	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	Cmd = XUSBPSU_DEPCMD_STARTTRANSFER;

	Ret = XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
			Cmd, Params);
	if (Ret == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	Ept->EpStatus |= XUSBPSU_EP_BUSY;
	Ept->ResourceIndex = (u8)XUsbPsu_EpGetTransferIndex(InstancePtr,
			Ept->UsbEpNum, Ept->Direction);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Restarts EP0 endpoint
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	XST_SUCCESS on success or else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static s32 XUsbPsu_RestoreEp0(struct XUsbPsu *InstancePtr)
{
        struct XUsbPsu_Ep *Ept;
        s32 Ret;
        u8 EpNum;

        for (EpNum = 0U; EpNum < 2U; EpNum++) {
		Ept = &InstancePtr->eps[EpNum];

		if (Ept == NULL) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		Ret = XUsbPsu_EpEnable(InstancePtr, Ept->UsbEpNum,
				Ept->Direction, Ept->MaxSize, Ept->Type, TRUE);
		if (Ret == XST_FAILURE) {
			xil_printf("Failed to enable EP %d on wakeup: %d\r\n",
					EpNum, Ret);
			return (s32)XST_FAILURE;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_STALL) != (u32)0U) {
			XUsbPsu_Ep0StallRestart(InstancePtr);
		} else {
			Ret = XUsbPsu_RestartEp(InstancePtr, Ept->PhyEpNum);
			if (Ret == XST_FAILURE) {
				xil_printf("Failed to restart EP %d on wakeup: %d\r\n",
						EpNum, Ret);
				return (s32)XST_FAILURE;
			}
		}
        }

        return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Restarts non EP0 endpoints
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	XST_SUCCESS on success or else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static s32 XUsbPsu_RestoreEps(struct XUsbPsu *InstancePtr)
{
        struct XUsbPsu_Ep *Ept;
	s32 Ret;
	u8 EpNum;

	for (EpNum = 2U; EpNum < XUSBPSU_ENDPOINTS_NUM; EpNum++) {
		Ept = &InstancePtr->eps[EpNum];

		if (Ept == NULL) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		Ret = XUsbPsu_EpEnable(InstancePtr, Ept->UsbEpNum,
				Ept->Direction, Ept->MaxSize, Ept->Type, TRUE);
		if (Ret == XST_FAILURE) {
			xil_printf("Failed to enable EP %d on wakeup: %d\r\n",
					EpNum, Ret);
			return (s32)XST_FAILURE;
		}
	}

	for (EpNum = 2U; EpNum < XUSBPSU_ENDPOINTS_NUM; EpNum++) {
		Ept = &InstancePtr->eps[EpNum];

		if (Ept == NULL) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_STALL) != (u32)0U) {
			XUsbPsu_EpSetStall(InstancePtr, Ept->UsbEpNum, Ept->Direction);
		} else {
			Ret = XUsbPsu_RestartEp(InstancePtr, Ept->PhyEpNum);
			if (Ret == XST_FAILURE) {
				xil_printf("Failed to restart EP %d on wakeup: %d\r\n",
						EpNum, Ret);
				return (s32)XST_FAILURE;
			}
		}
        }

        return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* Handle wakeup event
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
*
* @return	none
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_WakeupIntr(struct XUsbPsu *InstancePtr)
{
	u32 RegVal, link_state;
	u32 retries;
	u8 enter_hiber = (u8)0U;

	RegVal = XUsbPsu_ReadLpdReg(RST_LPD_TOP);
	if (InstancePtr->ConfigPtr->DeviceId == (u16)XPAR_XUSBPSU_0_DEVICE_ID) {
		XUsbPsu_WriteLpdReg(RST_LPD_TOP, (u32)(RegVal & ~(u32)USB0_CORE_RST));
	}

	/* change power state to D0 */
	XUsbPsu_WriteVendorReg(XIL_REQ_PWR_STATE, XIL_REQ_PWR_STATE_D0);

	/* wait till current state is changed to D0 */
        retries = (u32)XUSBPSU_PWR_STATE_RETRIES;

	while (retries > 0U) {
		RegVal = XUsbPsu_ReadVendorReg(XIL_CUR_PWR_STATE);
                if ((RegVal & XIL_CUR_PWR_STATE_BITMASK) == XIL_CUR_PWR_STATE_D0) {
                        break;
                }

                XUsbSleep(XUSBPSU_TIMEOUT);
                retries = retries - 1U;
        }

	if (retries == 0U) {
		xil_printf("Failed to change power state to D0\r\n");
		return;
	}

	/* ask core to restore non-sticky registers */
	XUsbPsu_SetupScratchpad(InstancePtr);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
	RegVal |= XUSBPSU_DCTL_CRS;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);

	/* wait till non-sticky registers are restored */
	if (XUsbPsu_Wait_Clear_Timeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_RSS, XUSBPSU_NON_STICKY_SAVE_RETRIES) == XST_FAILURE) {
		xil_printf("Failed to restore USB core\r\n");
		return;
	}

	restore_regs(InstancePtr);

	/* setup event buffers */
	XUsbPsu_EventBuffersSetup(InstancePtr);

	/* nothing to do when in OTG host mode */
	if ((XUsbPsu_ReadReg(InstancePtr, XUSBPSU_GSTS) & XUSBPSU_GSTS_CUR_MODE) != (u32)0U) {
		return;
	}

	if (XUsbPsu_RestoreEp0(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to restore EP0\r\n");
		return;
	}

	/* start controller */
	if (XUsbPsu_Start(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to start core on wakeup\r\n");
		return;
	}

	/* Wait until device controller is ready */
	if (XUsbPsu_Wait_Clear_Timeout(InstancePtr, XUSBPSU_DSTS,
			XUSBPSU_DSTS_DCNRD, XUSBPSU_CTRL_RDY_RETRIES) == XST_FAILURE) {
		xil_printf("Failed to ready device controller\r\n");
		return;
	}

	/*
	 * there can be spurious wakeup events , so wait for some time and check
	 * the link state
	 */
	XUsbSleep(XUSBPSU_TIMEOUT * 10U);

	link_state = XUsbPsu_GetLinkState(InstancePtr);

	switch(link_state) {
	case XUSBPSU_LINK_STATE_RESET:
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DSTS);
		RegVal &= ~XUSBPSU_DCFG_DEVADDR_MASK;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DSTS, RegVal);

		if (XUsbPsu_SetLinkState(InstancePtr, XUSBPSU_LINK_STATE_RECOV) == XST_FAILURE) {
			xil_printf("Failed to put link in Recovery\r\n");
			return;
		}
		break;
	case XUSBPSU_LINK_STATE_SS_DIS:
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DCTL);
		RegVal &= ~XUSBPSU_DCTL_KEEP_CONNECT;
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DCTL, RegVal);
		enter_hiber = (u8)1U;
		break;
	case XUSBPSU_LINK_STATE_U3:
		/* enter hibernation again */
		enter_hiber = (u8)1U;
		break;
	default:
		if (XUsbPsu_SetLinkState(InstancePtr, XUSBPSU_LINK_STATE_RECOV) == XST_FAILURE) {
			xil_printf("Failed to put link in Recovery\r\n");
			return;
		}
		break;
	};

	if (XUsbPsu_RestoreEps(InstancePtr) == XST_FAILURE) {
		xil_printf("Failed to restore EPs\r\n");
		return;
	}

	InstancePtr->IsHibernated = 0U;

	if (enter_hiber == (u8)1U)  {
		Xusbpsu_HibernationIntr(InstancePtr);
		return;
	}

	xil_printf("We are back from hibernation!\r\n");
}

#endif /* XUSBPSU_HIBERNATION_ENABLE */
/** @} */
