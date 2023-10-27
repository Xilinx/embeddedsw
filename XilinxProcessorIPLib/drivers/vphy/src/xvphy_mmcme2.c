/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvphy_mmcme2.c
 *
 * Contains a minimal set of functions for the XVphy driver that allow access
 * to all of the Video PHY core's functionality. See xvphy.h for a detailed
 * description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.7   gm   13/09/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xvphy.h"
#include "xvphy_i.h"
#if (XPAR_VPHY_0_TRANSCEIVER <= XVPHY_GTPE2)

/**************************** Function Prototypes *****************************/
static u32 XVphy_Mmcme2DividerEncoding(XVphy_MmcmDivType DivType, u8 Div);
static u16 XVphy_Mmcme2FilterReg2Encoding(u8 Mult, u8 TxIsPlle2);
static u16 XVphy_Mmcme2LockReg1Encoding(u8 Mult);
static u16 XVphy_Mmcme2LockReg2Encoding(u8 Mult);
static u16 XVphy_Mmcme2LockReg3Encoding(u8 Mult);

/************************** Constant Definitions ******************************/

/*****************************************************************************/
/**
* This function returns the DRP encoding of ClkFbOutMult optimized for:
* Phase = 0; Dutycycle = 0.5; No Fractional division
* The calculations are based on XAPP888
*
* @param	Div is the divider to be encoded
*
* @return
*		- Encoded Value for ClkReg1 [31: 0]
*       - Encoded Value for ClkReg2 [63:32]
*
* @note		None.
*
******************************************************************************/
u32 XVphy_Mmcme2DividerEncoding(XVphy_MmcmDivType DivType, u8 Div)
{
	u32 DrpEnc;
	u32 ClkReg1;
    u32 ClkReg2;
    u8 HiTime, LoTime;

    if (Div == 1) {
        ClkReg1 = 0x00001041;
        ClkReg2 = 0x00C00000;
    }
    else {
        HiTime = Div / 2;
        LoTime = Div - HiTime;

        ClkReg1 = LoTime & 0x3F;
        ClkReg1 |= (HiTime & 0x3F) << 6;
        ClkReg1 |= (DivType == MMCM_DIVCLK_DIVIDE) ? 0x0000 : 0x1000;

        ClkReg2 = (Div % 2) ? 0x00800000 : 0x00000000;
    }

    DrpEnc = ClkReg2 | ClkReg1;

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function returns the DRP encoding of FilterReg1 optimized for:
* Phase = 0; Dutycycle = 0.5; BW = low; No Fractional division
*
* @param	Mult is the divider to be encoded
*
* @return
*		- Encoded Value
*
* @note		None.
*
******************************************************************************/
u16 XVphy_Mmcme2FilterReg2Encoding(u8 Mult, u8 TxIsPlle2)
{
	u16 DrpEnc;

    switch (Mult) {
	case 1: case 2:
		DrpEnc = 0x9900;
		break;
	case 3:
		DrpEnc = TxIsPlle2 ? 0x1900 : 0x9900;
		break;
    case 4:
		DrpEnc = TxIsPlle2 ? 0x9100 : 0x9900;
		break;
	case 5:
		DrpEnc = TxIsPlle2 ? 0x1100 : 0x1900;
		break;
	case 6:
		DrpEnc = TxIsPlle2 ? 0x1100 : 0x8900;
		break;
	case 7:
		DrpEnc = TxIsPlle2 ? 0x8100 : 0x9100;
		break;
	case 8:
		DrpEnc = TxIsPlle2 ? 0x9800 : 0x0900;
		break;
	case 9:
		DrpEnc = TxIsPlle2 ? 0x9800 : 0x1100;
		break;
	case 10:
		DrpEnc = TxIsPlle2 ? 0x0100 : 0x1100;
		break;
	case 11:
		DrpEnc = TxIsPlle2 ? 0x0100 : 0x8100;
		break;
	case 12: case 13: case 14:
	case 15:
		DrpEnc = TxIsPlle2 ? 0x1800 : 0x9800;
		break;
	case 16:
	case 17:
	case 18:
		DrpEnc = TxIsPlle2 ? 0x8800 : 0x0100;
		break;
	case 19:
		DrpEnc = TxIsPlle2 ? 0x8800 : 0x1800;
		break;
    case 20: case 21: case 22:
	case 23: case 24: case 25:
		DrpEnc = TxIsPlle2 ? 0x9000 : 0x1800;
		break;
	case 26: case 27: case 28:
	case 29: case 30:
		DrpEnc = TxIsPlle2 ? 0x9000 : 0x8800;
		break;
	case 31: case 32: case 33:
	case 34: case 35: case 36:
	case 37: case 38: case 39:
	case 40:
		DrpEnc = TxIsPlle2 ? 0x0800 : 0x9000;
		break;
    case 41: case 42:
	case 43: case 44: case 45:
	case 46: case 47:
		DrpEnc = 0x9000;
		break;
	default:
		DrpEnc = TxIsPlle2 ? 0x1000 : 0x0800;
		break;
	}

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function returns the DRP encoding of LockReg1 optimized for:
* Phase = 0; Dutycycle = 0.5; No Fractional division
*
* @param	Mult is the divider to be encoded
*
* @return
*		- Encoded Value
*
* @note		None.
*
******************************************************************************/
u16 XVphy_Mmcme2LockReg1Encoding(u8 Mult)
{
	u16 DrpEnc;

    switch (Mult) {
	case 1: case 2: case 3:
	case 4: case 5: case 6:
	case 7: case 8: case 9:
	case 10:
		DrpEnc = 0x01e8;
		break;
	case 11:
		DrpEnc = 0x0184;
		break;
	case 12:
		DrpEnc = 0x0139;
		break;
	case 13:
		DrpEnc = 0x01ee;
		break;
	case 14:
		DrpEnc = 0x01bc;
		break;
	case 15:
		DrpEnc = 0x018a;
		break;
	case 16:
		DrpEnc = 0x0171;
		break;
	case 17:
		DrpEnc = 0x013f;
		break;
	case 18:
		DrpEnc = 0x0126;
		break;
	case 19:
		DrpEnc = 0x010d;
		break;
	case 20:
		DrpEnc = 0x00f4;
		break;
	case 21:
		DrpEnc = 0x00db;
		break;
	case 22:
		DrpEnc = 0x00c2;
		break;
	case 23:
		DrpEnc = 0x00a9;
		break;
	case 24:
	case 25:
		DrpEnc = 0x0090;
		break;
	case 26:
		DrpEnc = 0x0077;
		break;
	case 27:
	case 28:
		DrpEnc = 0x005e;
		break;
	case 29:
	case 30:
		DrpEnc = 0x0045;
		break;
	case 31:
	case 32:
	case 33:
		DrpEnc = 0x002c;
		break;
	case 34:
	case 35:
	case 36:
		DrpEnc = 0x0013;
		break;
	default:
		DrpEnc = 0x00fa;
		break;
	}

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function returns the DRP encoding of LockReg2 optimized for:
* Phase = 0; Dutycycle = 0.5; No Fractional division
*
* @param	Mult is the divider to be encoded
*
* @return
*		- Encoded Value
*
* @note		None.
*
******************************************************************************/
u16 XVphy_Mmcme2LockReg2Encoding(u8 Mult)
{
	u16 DrpEnc;

    switch (Mult) {
	case 1:
	case 2:
		DrpEnc = 0x1801;
		break;
	case 3:
		DrpEnc = 0x2001;
		break;
	case 4:
		DrpEnc = 0x2c01;
		break;
	case 5:
		DrpEnc = 0x3801;
		break;
	case 6:
		DrpEnc = 0x4401;
		break;
	case 7:
		DrpEnc = 0x4c01;
		break;
	case 8:
		DrpEnc = 0x5801;
		break;
	case 9:
		DrpEnc = 0x6401;
		break;
	case 10:
		DrpEnc = 0x7001;
		break;
	default:
		DrpEnc = 0x7c01;
		break;
	}

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function returns the DRP encoding of LockReg3 optimized for:
* Phase = 0; Dutycycle = 0.5; No Fractional division
*
* @param	Mult is the divider to be encoded
*
* @return
*		- Encoded Value
*
* @note		None.
*
******************************************************************************/
u16 XVphy_Mmcme2LockReg3Encoding(u8 Mult)
{
	u16 DrpEnc;

    switch (Mult) {
	case 1:
	case 2:
		DrpEnc = 0x19e9;
		break;
	case 3:
		DrpEnc = 0x21e9;
		break;
	case 4:
		DrpEnc = 0x2de9;
		break;
	case 5:
		DrpEnc = 0x39e9;
		break;
	case 6:
		DrpEnc = 0x45e9;
		break;
	case 7:
		DrpEnc = 0x4de9;
		break;
	case 8:
		DrpEnc = 0x59e9;
		break;
	case 9:
		DrpEnc = 0x65e9;
		break;
	case 10:
		DrpEnc = 0x71e9;
		break;
	default:
		DrpEnc = 0x7de9;
		break;
	}

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function will write the mixed-mode clock manager (MMCM) values currently
* stored in the driver's instance structure to hardware .
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if the MMCM write was successful.
*		- XST_FAILURE otherwise, if the configuration success bit did
*		  not go low.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_MmcmWriteParameters(XVphy *InstancePtr, u8 QuadId,
							XVphy_DirectionType Dir)
{
	u8 ChId;
	u16 DrpVal;
	u32 DrpVal32;
	XVphy_Mmcm *MmcmParams;
	u8 TxIsPlle2 = FALSE;

	if ((XVphy_IsHDMI(InstancePtr, XVPHY_DIR_TX)) &&
            (Dir == XVPHY_DIR_TX)) {
        TxIsPlle2 = TRUE;
    }

    ChId = (Dir == XVPHY_DIR_TX) ?
						XVPHY_CHANNEL_ID_TXMMCM :
						XVPHY_CHANNEL_ID_RXMMCM;

	MmcmParams = &InstancePtr->Quads[QuadId].Mmcm[Dir];

	/* Check Parameters if has been Initialized */
	if (!MmcmParams->DivClkDivide && !MmcmParams->ClkFbOutMult &&
			!MmcmParams->ClkOut0Div && !MmcmParams->ClkOut1Div &&
			!MmcmParams->ClkOut2Div) {
		return XST_FAILURE;
	}


	/* Write Power Register Value */
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x28, 0xFFFF);

	/* Write CLKFBOUT Reg1 & Reg2 Values */
	DrpVal32 = XVphy_Mmcme2DividerEncoding(MMCM_CLKFBOUT_MULT_F,
						MmcmParams->ClkFbOutMult);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x14,
						(u16)(DrpVal32 & 0xFFFF));
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x15,
						(u16)((DrpVal32 >> 16) & 0xFFFF));

	/* Write DIVCLK_DIVIDE Value */
	DrpVal32 = XVphy_Mmcme2DividerEncoding(MMCM_DIVCLK_DIVIDE,
						MmcmParams->DivClkDivide) ;
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x16,
						(u16)(DrpVal32 & 0xFFFF));

	/* Write CLKOUT0 Reg1 & Reg2 Values */
	DrpVal32 = XVphy_Mmcme2DividerEncoding(MMCM_CLKOUT_DIVIDE,
						MmcmParams->ClkOut0Div);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x08,
						(u16)(DrpVal32 & 0xFFFF));
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x09,
						(u16)((DrpVal32 >> 16) & 0xFFFF));

	/* Write CLKOUT1 Reg1 & Reg2 Values */
	DrpVal32 = XVphy_Mmcme2DividerEncoding(MMCM_CLKOUT_DIVIDE,
						MmcmParams->ClkOut1Div);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x0A,
						(u16)(DrpVal32 & 0xFFFF));
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x0B,
						(u16)((DrpVal32 >> 16) & 0xFFFF));

	/* Write CLKOUT2 Reg1 & Reg2 Values */
	DrpVal32 = XVphy_Mmcme2DividerEncoding(MMCM_CLKOUT_DIVIDE,
						MmcmParams->ClkOut2Div);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x0C,
						(u16)(DrpVal32 & 0xFFFF));
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x0D,
						(u16)((DrpVal32 >> 16) & 0xFFFF));

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTPE2)
	/* Configure CLKOUT3 if TX is HDMI and is GTPE2 */
	if (TxIsPlle2 == TRUE) {
		/* Write CLKOUT3 Reg1 & Reg2 Values */
		DrpVal32 = XVphy_Mmcme2DividerEncoding(MMCM_CLKOUT_DIVIDE,
							MmcmParams->ClkOut0Div / 2);
		XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x0E,
							(u16)(DrpVal32 & 0xFFFF));
		XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x0F,
							(u16)((DrpVal32 >> 16) & 0xFFFF));
	}
#endif

	/* Write Lock Reg1 Value */
	DrpVal = XVphy_Mmcme2LockReg1Encoding(MmcmParams->ClkFbOutMult);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x18, DrpVal);

	/* Write Lock Reg2 Value */
	DrpVal = XVphy_Mmcme2LockReg2Encoding(MmcmParams->ClkFbOutMult);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x19, DrpVal);

	/* Write Lock Reg3 Value */
	DrpVal = XVphy_Mmcme2LockReg3Encoding(MmcmParams->ClkFbOutMult);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x1A, DrpVal);

	/* Write Filter Reg1 Value */
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x4E, 0x0800);

	/* Write Filter Reg2 Value */
	DrpVal = XVphy_Mmcme2FilterReg2Encoding(MmcmParams->ClkFbOutMult,
                    TxIsPlle2);
	XVphy_DrpWr(InstancePtr, QuadId, ChId, 0x4F, DrpVal);

	return XST_SUCCESS;
}

#endif
