/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xv_sdirx.c
*
* This is the main file for Xilinx SDI RX core. Please see xv_sdirx.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	   Date		Changes
* ----- ------ -------- --------------------------------------------------
* 1.0	jsr    07/17/17 Initial release.
#       vve    10/03/18 Add support for ST352 in C-Stream
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sdirx.h"
#include "sleep.h"
#include <string.h>

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSDIRX_LINE_RATE_3G	0
#define XSDIRX_LINE_RATE_6G	1
#define XSDIRX_LINE_RATE_12G8DS	2


/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

static void StubCallback(void *CallbackRef);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/****************************************************************************/
/**
*
* This function is used to enable the global interrupts. This is
* used after setting the interrupts mask before enabling the core.
*
* @param	InstancePtr is a pointer to the SDI RX Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XV_SdiRx_SetGlobalInterrupt(XV_SdiRx *InstancePtr)
{
	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress,
				XV_SDIRX_GIER_OFFSET,
				XV_SDIRX_GIER_GIE_MASK);
}

/****************************************************************************/
/**
*
* This function is used to disable the global interrupts. This is
* done after disabling the core.
*
* @param	InstancePtr is a pointer to the SDI Rx Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XV_SdiRx_ResetGlobalInterrupt(XV_SdiRx *InstancePtr)
{
	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress,
				XV_SDIRX_GIER_OFFSET,
				~XV_SDIRX_GIER_GIE_MASK);
}


/*****************************************************************************/
/**
*
* This function initializes the SDI RX core. This function must be called
* prior to using the SDI RX core. Initialization of the SDI RX includes
* setting up the instance data, and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	CfgPtr points to the configuration structure associated with
*		the SDI RX core.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS if XV_SdiRx_CfgInitialize was successful.
*		- XST_FAILURE if XV_SdiRx_CfgInitialize was unsuccessful.
*
* @note		None.
*
******************************************************************************/
int XV_SdiRx_CfgInitialize(XV_SdiRx *InstancePtr, XV_SdiRx_Config *CfgPtr,
				UINTPTR EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XV_SdiRx));
	(void)memcpy((void *)&(InstancePtr->Config),
	(const void *)CfgPtr,
	sizeof(XV_SdiRx_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Callbacks
	 * These are placeholders pointing to the StubCallback. The actual
	 * callback pointers will be assigned by the SetCallback function.
	 */
	InstancePtr->StreamDownCallback = (XV_SdiRx_Callback)((void *)StubCallback);

	InstancePtr->StreamUpCallback = (XV_SdiRx_Callback)((void *)StubCallback);

	InstancePtr->OverFlowCallback = (XV_SdiRx_Callback)((void *)StubCallback);

	InstancePtr->UnderFlowCallback = (XV_SdiRx_Callback)((void *)StubCallback);

	/* Clear SDI variables */
	XV_SdiRx_ResetStream(InstancePtr);

	/* Stop SDI RX Core */
	XV_SdiRx_Stop(InstancePtr);

	/* Configure SDI RX Core */
	XV_SdiRx_FramerEnable(InstancePtr);
	XV_SdiRx_EnableMode(InstancePtr, XV_SDIRX_SUPPORT_ALL);

	/* Enables FF EDH and AP EDH errors counter */
	XV_SdiRx_SetEdhErrCntTrigger(InstancePtr, 0x420);

	/* Initializes the video lock window */
	XV_SdiRx_SetVidLckWindow(InstancePtr, XV_SDIRX_VID_LCK_WINDOW);

	/* Reset all peripherals */
	XV_SdiRx_IntrDisable(InstancePtr, XV_SDIRX_IER_ALLINTR_MASK);

	/* Start SDI core */
	XV_SdiRx_Start(InstancePtr, XV_SDIRX_MULTISEARCHMODE);

	/* Set global interrupt enable bit */
	XV_SdiRx_SetGlobalInterrupt(InstancePtr);

	/* Clear the interrupt status */
	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_ISR_OFFSET),
				(XV_SDIRX_ISR_VIDEO_LOCK_MASK
				| XV_SDIRX_ISR_VIDEO_UNLOCK_MASK
				| XV_SDIRX_ISR_OVERFLOW_MASK
				| XV_SDIRX_ISR_UNDERFLOW_MASK));

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function clears the SDI RX stream variables and sets them to the defaults.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		This is required after a reset or init.
*
******************************************************************************/
void XV_SdiRx_ResetStream(XV_SdiRx *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	for (int i = 0; i < XV_SDIRX_MAX_DATASTREAM; i++) {
		InstancePtr->Stream[i].Video.VmId = (XSDIVID_VM_720_HD);
		InstancePtr->Stream[i].Video.ColorFormatId = (XVIDC_CSF_YCBCR_422);
		InstancePtr->Stream[i].Video.ColorDepth = (XVIDC_BPC_10);
		InstancePtr->Stream[i].Video.FrameRate = (XVIDC_FR_UNKNOWN);
		InstancePtr->Stream[i].Video.IsInterlaced = (XVIDC_VF_PROGRESSIVE);
		InstancePtr->Stream[i].Video.AspectRatio = (XVIDC_AR_16_9);
		InstancePtr->Stream[i].CAssignment = (XSDIVID_CA_CH1);
	}
	InstancePtr->Transport.IsFractional = 0;
	InstancePtr->Transport.IsLevelB3G = 0;
	InstancePtr->VideoStreamNum = 0;
}

/******************************************************************************/
/**
*
* This function prints debug information of Stream 0 on STDIO/Uart console.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	SelId specifies which debug information to be printed out
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_DebugInfo(XV_SdiRx *InstancePtr, XV_SdiRx_DebugSelId SelId)
{
	u32 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	switch (SelId) {
	case 0:
		/* Print stream information */
		XVidC_ReportStreamInfo(&InstancePtr->Stream[0].Video);
		break;
	case 1:
		/* Print timing information */
		XVidC_ReportTiming(&InstancePtr->Stream[0].Video.Timing,
		InstancePtr->Stream[0].Video.IsInterlaced);
		break;
	case 2:
		/* Print SDI specific information */
		xil_printf("\tSDI Mode:         ");

		switch (InstancePtr->Transport.TMode) {
		case 0:
			xil_printf("HD");
			break;

		case 1:
			xil_printf("SD");
			break;

		case 2:
			xil_printf("3GA");
			break;

		case 3:
			xil_printf("3GB");
			break;

		case 4:
			xil_printf("6G");
			break;

		case 5:
			xil_printf("12G");
			break;

		default:
			xil_printf("INVALID");
			break;
		}

		xil_printf("\n\r");

		xil_printf("\tBit Rate:         %s\n\r",
		InstancePtr->Transport.IsFractional ? "Fractional" : "Integer");
		xil_printf("\tST352 Payload:    0x%X\n\r",
		InstancePtr->Stream[0].PayloadId);
		break;
	case 3:
		Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_TS_DET_STS_OFFSET);
		xil_printf("  Transport Locked: %d\r\n",
				(Data & XV_SDIRX_TS_DET_STS_T_LOCKED_MASK));
		xil_printf("  Transport Scan: %d\r\n",
				(Data & XV_SDIRX_TS_DET_STS_T_SCAN_MASK)
				>> XV_SDIRX_TS_DET_STS_T_SCAN_SHIFT);
		xil_printf("  Transport Family: %d\r\n",
				(Data & XV_SDIRX_TS_DET_STS_T_FAMILY_MASK)
				>> XV_SDIRX_TS_DET_STS_T_FAMILY_SHIFT);
		xil_printf("  Transport Rate: %d\r\n",
				(Data & XV_SDIRX_TS_DET_STS_T_RATE_MASK)
				>> XV_SDIRX_TS_DET_STS_T_RATE_SHIFT);

		Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_STS_SB_RX_TDATA_OFFSET);
		xil_printf("  Fractional Bit Rate: %d\r\n", (Data
				& XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_BIT_RATE_MASK)
				>> XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_BIT_RATE_SHIFT);

		Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_RX_ERR_OFFSET);
		xil_printf("  CRC Error Counts: %d%\r\n",
				Data & XV_SDIRX_RX_ERR_MASK);

		xil_printf("\r\n");
		xil_printf("RX Video Bridge:\r\n");
		Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_MODE_DET_STS_OFFSET);
		xil_printf("  Bridge Select: ");
		if (((Data & XV_SDIRX_MODE_DET_STS_MODE_MASK) == 5)
			|| (Data & XV_SDIRX_MODE_DET_STS_MODE_MASK) == 6) {
			xil_printf("12G SDI Bridge");
		} else if ((Data & XV_SDIRX_MODE_DET_STS_MODE_MASK) == 2) {
			xil_printf("3G SDI Bridge");
		} else {
			xil_printf("SDI Bridge is not 3G/12G");
		}

		xil_printf("\r\n");

		xil_printf("  Mode Locked: %d\r\n",
				(Data & XV_SDIRX_BRIDGE_STS_MODE_LOCKED_MASK)
				>> XV_SDIRX_BRIDGE_STS_MODE_LOCKED_SHIFT);
		xil_printf("  SDI Mode: ");
		switch ((Data & XV_SDIRX_BRIDGE_STS_MODE_MASK)
			>> XV_SDIRX_BRIDGE_STS_MODE_SHIFT) {
		case 0:
			xil_printf("HD");
			break;

		case 1:
			xil_printf("SD");
			break;

		case 2:
			xil_printf("3G");
			break;

		case 4:
			xil_printf("6G");
			break;

		case 5:
			xil_printf("12G Integer");
			break;

		case 6:
			xil_printf("12G Fractional");
			break;

		default:
			xil_printf("INVALID");
			break;
		}

		xil_printf("\r\n\r\n");

		xil_printf("RX AXIS Bridge:\r\n");
		Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDIRX_ISR_OFFSET);
		xil_printf("  Overflow: %d\r\n", (Data & XV_SDIRX_ISR_OVERFLOW_MASK)
						>> XV_SDIRX_ISR_OVERFLOW_SHIFT);
		xil_printf("  Underflow: %d\r\n", (Data & XV_SDIRX_ISR_OVERFLOW_MASK)
						>> XV_SDIRX_ISR_UNDERFLOW_SHIFT);
		break;
	case 4:
		for (int i = 0; i <= XV_SDIRX_REGISTER_SIZE; i++) {
			Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress, (i*4));
			xil_printf("Address: 0x%X Data: 0x%X\r\n",
				(InstancePtr->Config.BaseAddress + (i*4)), Data);
		}
		break;
	default:
		break;
	}
}

/*****************************************************************************/
/**
*
* This function stops SDI RX core's modes detection
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return
*		- XST_SUCCESS if SDI Rx stop is successful.
*		- XST_FAILURE if SDI Rx stop is unsuccessful
* @note		None.
*
******************************************************************************/
int XV_SdiRx_Stop(XV_SdiRx *InstancePtr)
{
	u32 Data;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET));
	Data &= ~XV_SDIRX_RST_CTRL_SDIRX_SS_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET), (Data));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables the framer function. When enabled, the framer
* automatically readjusts the output word alignment to match the alignment of
* each timing reference signal (TRS).
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_FramerEnable(XV_SdiRx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_MDL_CTRL_OFFSET));
	Data |= XV_SDIRX_MDL_CTRL_FRM_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_MDL_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function disables the framer function.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_FramerDisable(XV_SdiRx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_MDL_CTRL_OFFSET));
	Data &= ~XV_SDIRX_MDL_CTRL_FRM_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_MDL_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function enables SDI RX core's modes detection function.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	Mode specifies the mode of SDI modes searching operation.
*		- 0 = XV_SDIRX_SINGLESEARCHMODE_HD
*		- 1 = XV_SDIRX_SINGLESEARCHMODE_SD
*		- 2 = XV_SDIRX_SINGLESEARCHMODE_3G
*		- 4 = XV_SDIRX_SINGLESEARCHMODE_6G
*		- 5 = XV_SDIRX_SINGLESEARCHMODE_12GI
*		- 6 = XV_SDIRX_SINGLESEARCHMODE_12GF
*		- 10 = XV_SDIRX_MULTISEARCHMODE where the supported modes will be
*				enabled by XV_SdiRx_EnableMode function
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_Start(XV_SdiRx *InstancePtr, XV_SdiRx_SearchMode Mode)
{
	u32 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_MDL_CTRL_OFFSET));

	if (Mode == XV_SDIRX_MULTISEARCHMODE) {
		/* Operating in auto RX modes detection */
		Data &= ~XV_SDIRX_MDL_CTRL_MODE_EN_MASK;
		Data |= (InstancePtr->SupportedModes << XV_SDIRX_MDL_CTRL_MODE_EN_SHIFT)
				| XV_SDIRX_MDL_CTRL_MODE_DET_EN_MASK;
	} else {
		/* Operating in single RX mode detection */
		Data &= ~(XV_SDIRX_MDL_CTRL_FORCED_MODE_MASK
				| XV_SDIRX_MDL_CTRL_MODE_DET_EN_MASK);
		Data |= (Mode << XV_SDIRX_MDL_CTRL_FORCED_MODE_SHIFT);
	}

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_MDL_CTRL_OFFSET), (Data));

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET));

	Data |= XV_SDIRX_RST_CTRL_SDIRX_SS_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function reports detected CRC or EDH errors count.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	CRC/EDH Error count
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiRx_ReportDetectedError(XV_SdiRx *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	RegValue = XV_SdiRx_GetSdiMode(InstancePtr);

	if (RegValue == XSDIVID_MODE_SD) {
		RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
						(XV_SDIRX_RX_EDH_ERRCNT_OFFSET));
		RegValue &= XV_SDIRX_RX_EDH_ERRCNT_EDH_ERRCNT_MASK;
		xil_printf("\tEDH: %d\r\n", RegValue);
	} else {
		RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
						(XV_SDIRX_RX_ERR_OFFSET));
		RegValue &= XV_SDIRX_RX_ERR_MASK;
		xil_printf("\tCRC: %d\r\n", RegValue);
	}

	return RegValue;
}
/*****************************************************************************/
/**
*
* This function enables 10bit YUV444 and RGB color format support for SDI Rx
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
******************************************************************************/

void XV_SdiRx_SetYCbCr444_RGB_10bit(XV_SdiRx *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDIRX_RST_CTRL_OFFSET);
	Data |= XV_SDIRX_RST_CTRL_CH_FORMAT_AXI_EN_MASK;

	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress,
			  XV_SDIRX_RST_CTRL_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* This function disables 10bit YUV444 and RGB color format support for SDI Rx
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
******************************************************************************/
void XV_SdiRx_ClearYCbCr444_RGB_10bit(XV_SdiRx *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDIRX_RST_CTRL_OFFSET);
	Data &= ~XV_SDIRX_RST_CTRL_CH_FORMAT_AXI_EN_MASK;

	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress,
			  XV_SDIRX_RST_CTRL_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* This function enables the video bridge
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_VidBridgeEnable(XV_SdiRx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET));
	Data |= XV_SDIRX_RST_CTRL_SDIRX_BRIDGE_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function disables the video bridge
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_VidBridgeDisable(XV_SdiRx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET));
	Data &= ~XV_SDIRX_RST_CTRL_SDIRX_BRIDGE_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function enable the AXI4S Bridge
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_Axi4sBridgeEnable(XV_SdiRx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET));

	Data |= XV_SDIRX_RST_CTRL_VID_IN_AXI4S_MDL_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function disables the AXI4S Bridge
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_Axi4sBridgeDisable(XV_SdiRx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET));
	Data &= ~XV_SDIRX_RST_CTRL_VID_IN_AXI4S_MDL_EN_MASK;

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_RST_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function returns incoming stream's incoming SDI payload ID.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	DataStream specifies which payload ID is to be returned.
*
* @return	SDI payload ID
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiRx_GetPayloadId(XV_SdiRx *InstancePtr, u8 DataStream)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DataStream <= 7);

	RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDIRX_RX_ST352_VLD_OFFSET));
	if (InstancePtr->Transport.TMode == XSDIVID_MODE_6G ||
		InstancePtr->Transport.TMode == XSDIVID_MODE_12G) {
		RegValue = (RegValue >> DataStream) & 0x00000101;
	} else {
		RegValue = (RegValue >> DataStream) & 0x00000001;
	}

	if (RegValue) {
		RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
						(XV_SDIRX_RX_ST352_0_OFFSET
						+ (DataStream * 4)));
	} else {
		/* Invalid st352 data */
		return 0;
	}

	return RegValue;
}

/*****************************************************************************/
/**
* This function is used to wait for the payload valid bit to be set.
* This has to be called from application based on the callback indication of
* the video lock interrupt handler. Without this function being called, it may
* be guaranteed that payload bits are valid after video lock interrupt occured.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return
*		- XST_FAILURE if the payload valid bits are not set.
*		- XST_SUCCESS if ST352 registers are read and loaded to
*		  Rx structures
*
* @note         None.
*
******************************************************************************/
u32 XV_SdiRx_WaitforPayLoad(XV_SdiRx *InstancePtr)
{
	u32 Data0, Data1, Data3;
	int StreamId;
	XSdiVid_TransMode TMode;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Data0 = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_SDIRX_MODE_DET_STS_OFFSET));
	Data1 = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_SDIRX_TS_DET_STS_OFFSET));

	/* Check if mode and transport are locked */
	if (!(((Data0 & XV_SDIRX_MODE_DET_STS_MODE_LOCKED_MASK)
		== XV_SDIRX_MODE_DET_STS_MODE_LOCKED_MASK)
		&& ((Data1 & XV_SDIRX_TS_DET_STS_T_LOCKED_MASK)
		== XV_SDIRX_TS_DET_STS_T_LOCKED_MASK)))
		return XST_FAILURE;

	TMode = Data0 & XV_SDIRX_MODE_DET_STS_MODE_MASK;

	/* Based on TMode, calculate the expected values to be present in
	 * the st352 valid register
	 */
	switch (TMode) {
		case XSDIVID_MODE_HD:
		case XSDIVID_MODE_SD:
			Data3 = 0x1;
			break;
		case XSDIVID_MODE_3GA:
			Data3 = 0x3;
			break;
		case XSDIVID_MODE_6G:
		case XSDIVID_MODE_12G:
		case XSDIVID_MODE_12GF:
			Data3 = 0xF;
			break;
		default:
			return XST_FAILURE;
	}

	/*
	 * Wait for 50 ms, the maximum frame time for any SDI supported
	 * resolution. The highest frametime is 1/23.98 ~ 41 ms. With some
	 * buffer we consider 50 ms wait time affter which we can ensure that
	 * a frame has been passed.
	 */
	usleep(50000);

	/* Payload are expected for 3GA and above modes */
	if ((XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDIRX_RX_ST352_VLD_OFFSET)) != Data3) &&
		(TMode >= XSDIVID_MODE_3GA)) {
		return XST_FAILURE;
	}

	for (StreamId = 0; StreamId < XV_SDIRX_MAX_DATASTREAM; StreamId++) {
		InstancePtr->Stream[StreamId].PayloadId
			= XV_SdiRx_GetPayloadId(InstancePtr, StreamId);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the current SDI transport mode detected.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	SDI transport mode
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiRx_GetSdiMode(XV_SdiRx *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	RegValue = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDIRX_MODE_DET_STS_OFFSET));

	return RegValue & XV_SDIRX_MODE_DET_STS_MODE_MASK;
}

/*****************************************************************************/
/**
*
* This function sets the clock period for video lock signal to be asserted
* before video lock interrupt is triggered
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	Data specifies the clock period.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_SetVidLckWindow(XV_SdiRx *InstancePtr, u32 Data)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				XV_SDIRX_VIDLCK_WINDOW_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* This function sets the type of EDH errors which will trigger the error count.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	Enable specifies the type of EDH errors to be allowed to increment
*			the error counter.
*		- Bit 0 = ANC EDH error
*		- Bit 1 = ANC EDA error
*		- Bit 2 = ANC IDH error
*		- Bit 3 = ANC IDA error
*		- Bit 4 = ANC UES error
*		- Bit 5 = FF EDH error
*		- Bit 6 = FF EDA error
*		- Bit 7 = FF IDH error
*		- Bit 8 = FF IDA error
*		- Bit 9 = FF UES error
*		- Bit 10 = AP EDH error
*		- Bit 11 = AP EDA error
*		- Bit 12 = AP IDH error
*		- Bit 13 = AP IDA error
*		- Bit 14 = AP UES error
*		- Bit 15 = EDH packet checksum-error
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_SetEdhErrCntTrigger(XV_SdiRx *InstancePtr, u32 Enable)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Enable <= XV_SDIRX_EDH_ERRCNT_EN_MASK);

	Data = (Enable & XV_SDIRX_EDH_ERRCNT_EN_MASK);

	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDIRX_EDH_ERRCNT_EN_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function enables the modes which the SDI RX core will try to lock on.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	SupportModes specifies which SDI mode is to be supported from SDI
*			mode detection logic.
*
* @return	None.
*
* @note		XV_SDIRX_MULTISEARCHMODE must be selected in the XV_SdiRx_Start
*			function.
*
******************************************************************************/
void XV_SdiRx_EnableMode(XV_SdiRx *InstancePtr,
				XV_SdiRx_SupportedModes SupportModes)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SupportModes <= XV_SDIRX_SUPPORT_ALL);
	/* Following assertions make sure the IP is configured with in the
	 * subcore GUI parameter limit
	 */
	Xil_AssertVoid(!((InstancePtr->Config.MaxRateSupported == XSDIRX_LINE_RATE_3G) &&
			(SupportModes & (XV_SDIRX_SUPPORT_6G |
			XV_SDIRX_SUPPORT_12GI | XV_SDIRX_SUPPORT_12GF))));
	Xil_AssertVoid(!((InstancePtr->Config.MaxRateSupported == XSDIRX_LINE_RATE_6G) &&
			(SupportModes & (XV_SDIRX_SUPPORT_12GI | XV_SDIRX_SUPPORT_12GF))));

	InstancePtr->SupportedModes |= SupportModes;
}

/*****************************************************************************/
/**
*
* This function disables the modes which the SDI RX core will try to lock on.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	RemoveModes specifies which SDI mode is to be removed from SDI mode
*			detection logic.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiRx_DisableMode(XV_SdiRx *InstancePtr,
				XV_SdiRx_SupportedModes RemoveModes)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RemoveModes <= XV_SDIRX_SUPPORT_ALL);

	InstancePtr->SupportedModes &= ~RemoveModes;
}

/*****************************************************************************/
/**
*
* This function is a stub for the asynchronous callback. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubCallback(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef != NULL);
	Xil_AssertVoidAlways();
}
