/******************************************************************************
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmipi_rx_phy_mmcm.c
* @addtogroup mipi_rx_phy Overview
* @{
*
* This file implements the MMCM configuration for dynamic line rate support
* in the MIPI RX PHY driver. It computes the optimal MMCM parameters
* (multiplier M, output divider O, input divider D) for a given line rate
* and programs the corresponding PHY registers. The PHY disable/enable is
* handled by the caller (XMipi_Rx_Phy_DynamicLineRateConfig in
* xmipi_rx_phy.c) so that both PLL and MMCM can be configured in the same
* disable window.
*
* <pre>
* MODIFICATION HISTORY:
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 ai 10/04/26 Initial release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdebug.h"
#include "xstatus.h"
#include "xmipi_rx_phy.h"

/************************** Constant Definitions *****************************/

/** @name MMCM Fixed Parameters
 * @{
 */
#define XMIPI_RX_PHY_MMCM_FIN_MHZ		200U	/**< Input clock frequency in MHz */
#define XMIPI_RX_PHY_MMCM_D_FIXED		5U	/**< Fixed input divider */
#define XMIPI_RX_PHY_MMCM_VCO_MIN_MHZ		2160U	/**< Minimum VCO frequency in MHz */
#define XMIPI_RX_PHY_MMCM_VCO_MAX_MHZ		4320U	/**< Maximum VCO frequency in MHz */
#define XMIPI_RX_PHY_MMCM_O_MIN		2U	/**< Minimum output divider */
#define XMIPI_RX_PHY_MMCM_O_MAX		511U	/**< Maximum output divider */
#define XMIPI_RX_PHY_MMCM_M_MIN		4U	/**< Minimum multiplier */
#define XMIPI_RX_PHY_MMCM_M_MAX		432U	/**< Maximum multiplier */
#define XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION	64U	/**< Fractional M resolution
							  *  (1/64 steps) */
#define XMIPI_RX_PHY_MMCM_VCO_MID_MHZ		3240U	/**< VCO mid-band target
							  *  for tie-break */
/*@}*/

/** @name Line Rate Range Boundaries (Mbps)
 * @{
 */
#define XMIPI_RX_PHY_LINERATE_RANGE1_MIN	400U	/**< Range 1 minimum (K=4) */
#define XMIPI_RX_PHY_LINERATE_RANGE1_MAX	1080U	/**< Range 1 maximum (K=4) */
#define XMIPI_RX_PHY_LINERATE_RANGE2_MIN	1081U	/**< Range 2 minimum (K=8) */
#define XMIPI_RX_PHY_LINERATE_RANGE2_MAX	2160U	/**< Range 2 maximum (K=8) */
#define XMIPI_RX_PHY_LINERATE_RANGE3_MIN	2161U	/**< Range 3 minimum (K=11) */
#define XMIPI_RX_PHY_LINERATE_RANGE3_MAX	2250U	/**< Range 3 maximum (K=11) */
#define XMIPI_RX_PHY_LINERATE_RANGE4_MIN	2251U	/**< Range 4 minimum (K=16) */
#define XMIPI_RX_PHY_LINERATE_RANGE4_MAX	4320U	/**< Range 4 maximum (K=16) */
#define XMIPI_RX_PHY_LINERATE_RANGE5_MIN	4321U	/**< Range 5 minimum (K=22) */
#define XMIPI_RX_PHY_LINERATE_RANGE5_MAX	4500U	/**< Range 5 maximum (K=22) */
/*@}*/

/** @name K-Factor Values for Line Rate Ranges
 * @{
 */
#define XMIPI_RX_PHY_K_FACTOR_RANGE1		4U	/**< K factor for 400-1080 Mbps */
#define XMIPI_RX_PHY_K_FACTOR_RANGE2		8U	/**< K factor for 1081-2160 Mbps */
#define XMIPI_RX_PHY_K_FACTOR_RANGE3		11U	/**< K factor for 2161-2250 Mbps */
#define XMIPI_RX_PHY_K_FACTOR_RANGE4		16U	/**< K factor for 2251-4320 Mbps */
#define XMIPI_RX_PHY_K_FACTOR_RANGE5		22U	/**< K factor for 4321-4500 Mbps */
/*@}*/

/** @name MMCM DRP Register Encoding Constants
 * @{
 */
#define XMIPI_RX_PHY_CLKOUT_EVEN_NO_EDGE	0x1A00U	/**< CLKOUT0 Reg1: even divide,
							  *  no edge */
#define XMIPI_RX_PHY_CLKOUT_EVEN_EDGE		0x1B00U	/**< CLKOUT0 Reg1: even divide,
							  *  edge bit set */
#define XMIPI_RX_PHY_CLKOUT_ODD_NO_EDGE		0xBA00U	/**< CLKOUT0 Reg1: odd divide,
							  *  no edge */
#define XMIPI_RX_PHY_CLKOUT_ODD_EDGE		0xBB00U	/**< CLKOUT0 Reg1: odd divide,
							  *  edge bit set */
#define XMIPI_RX_PHY_MMCM_MULT_ODD_FLAG		0x1700U	/**< MULT Reg1 for odd M_int */
#define XMIPI_RX_PHY_MMCM_MULT_EVEN_FLAG	0x1600U	/**< MULT Reg1 for even M_int */
#define XMIPI_RX_PHY_MMCM_FRAC_ENABLE		0x0002U	/**< Fractional M enable bit */
#define XMIPI_RX_PHY_MMCM_FRAC_MASK		0x3FU	/**< Fractional M value mask
							  *  (6 bits) */
#define XMIPI_RX_PHY_MMCM_DIV_D5_REG1		0x0400U	/**< DIV Reg1 encoding for D=5 */
#define XMIPI_RX_PHY_MMCM_DIV_D5_HTLT_VAL	2U	/**< DIV Reg2 HT/LT value for
							  *  D=5 */
/*@}*/

/** @name Pack HT/LT Bit Definitions
 * @{
 */
#define XMIPI_RX_PHY_HTLT_HIGH_SHIFT		8U	/**< High-time bit shift */
#define XMIPI_RX_PHY_HTLT_BYTE_MASK		0xFFU	/**< Byte mask for HT/LT fields */
/*@}*/

/** @name MMCM Score Computation Constants
 * @{
 */
#define XMIPI_RX_PHY_SCORE_PPM_WEIGHT		1e3	/**< PPM error weight in score */
#define XMIPI_RX_PHY_SCORE_O_WEIGHT		0.1	/**< Output divider weight in
							  *  score */
#define XMIPI_RX_PHY_SCORE_FRAC_PENALTY		10.0	/**< Penalty for fractional M
							  *  in score */
#define XMIPI_RX_PHY_PPM_SCALE			1e6	/**< PPM scale factor */
#define XMIPI_RX_PHY_SCORE_INIT			1e300	/**< Initial best score
							  *  (very large) */
/*@}*/

/**************************** Type Definitions *******************************/

/**
* MMCM computation result structure.
*
* Holds the computed MMCM parameters and the corresponding DRP register
* writes that must be applied to the RX PHY register space.
*/
typedef struct {
	u32 Valid;			/**< Non-zero if solution found */
	u32 KFactor;			/**< Derived K factor from line rate */
	u32 InputDivider;		/**< Input divider (fixed at 5) */
	u32 OutputDivider;		/**< Output divider */
	u32 MultiplierInt;		/**< M integer part */
	u32 MultiplierFrac;		/**< M fractional part (0..63) */
} XMipi_Rx_Phy_MmcmResult;

/************************** Macros Definitions *****************************/

/** @name Math helper macros to avoid -lm dependency
 * @{
 */
#define XMIPI_RX_PHY_ABS(x)		(((x) >= 0) ? (x) : (-(x)))
					/**< Absolute value of x */
#define XMIPI_RX_PHY_ROUND_S32(x)	((s32)(((x) >= 0) ? ((x) + 0.5) : ((x) - 0.5)))
					/**< Round double to nearest s32 integer */
#define XMIPI_RX_PHY_ROUND_DBL(x)	((double)XMIPI_RX_PHY_ROUND_S32(x))
					/**< Round double to nearest integer returned as double */
#define XMIPI_RX_PHY_FLOOR(x)		((double)(((double)(s64)(x) <= (x)) ? \
					 (s64)(x) : ((s64)(x) - 1)))
					/**< Floor of x returned as double */
/*@}*/

/************************** Function Prototypes ******************************/

static u32 XMipi_Rx_Phy_MmcmDeriveK(u32 LineRate);
static u16 XMipi_Rx_Phy_MmcmPackHtLt(u32 HighTime, u32 LowTime);
static void XMipi_Rx_Phy_MmcmClkout0Regs(u32 OutputDivider, u16 *ClkoutReg1,
					 u16 *ClkoutReg2);
static XMipi_Rx_Phy_MmcmResult XMipi_Rx_Phy_MmcmCompute(u32 LineRate);

/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
* Derive the K factor from the line rate.
*
* The K factor maps line rate ranges to a divider used to compute the
* target output frequency: Fout = LineRate / K.
*
* @param	LineRate is the desired line rate in Mbps.
*
* @return	K factor, or 0 if line rate is out of range.
*
* @note		None.
*
*****************************************************************************/
static u32 XMipi_Rx_Phy_MmcmDeriveK(u32 LineRate)
{
	u32 KFactor;

	if ((LineRate >= XMIPI_RX_PHY_LINERATE_RANGE1_MIN) &&
	    (LineRate <= XMIPI_RX_PHY_LINERATE_RANGE1_MAX)) {
		KFactor = XMIPI_RX_PHY_K_FACTOR_RANGE1;
	} else if ((LineRate >= XMIPI_RX_PHY_LINERATE_RANGE2_MIN) &&
		   (LineRate <= XMIPI_RX_PHY_LINERATE_RANGE2_MAX)) {
		KFactor = XMIPI_RX_PHY_K_FACTOR_RANGE2;
	} else if ((LineRate >= XMIPI_RX_PHY_LINERATE_RANGE3_MIN) &&
		   (LineRate <= XMIPI_RX_PHY_LINERATE_RANGE3_MAX)) {
		KFactor = XMIPI_RX_PHY_K_FACTOR_RANGE3;
	} else if ((LineRate >= XMIPI_RX_PHY_LINERATE_RANGE4_MIN) &&
		   (LineRate <= XMIPI_RX_PHY_LINERATE_RANGE4_MAX)) {
		KFactor = XMIPI_RX_PHY_K_FACTOR_RANGE4;
	} else if ((LineRate >= XMIPI_RX_PHY_LINERATE_RANGE5_MIN) &&
		   (LineRate <= XMIPI_RX_PHY_LINERATE_RANGE5_MAX)) {
		KFactor = XMIPI_RX_PHY_K_FACTOR_RANGE5;
	} else {
		KFactor = 0U;
	}

	return KFactor;
}

/****************************************************************************/
/**
* Pack high-time and low-time values into a 16-bit register value.
*
* @param	HighTime is the high-time count (bits [15:8]).
* @param	LowTime is the low-time count (bits [7:0]).
*
* @return	Packed 16-bit value.
*
* @note		None.
*
*****************************************************************************/
static u16 XMipi_Rx_Phy_MmcmPackHtLt(u32 HighTime, u32 LowTime)
{
	return (u16)(((HighTime & XMIPI_RX_PHY_HTLT_BYTE_MASK) <<
		      XMIPI_RX_PHY_HTLT_HIGH_SHIFT) |
		     (LowTime & XMIPI_RX_PHY_HTLT_BYTE_MASK));
}

/****************************************************************************/
/**
* Compute CLKOUT0 register values from the output divider.
*
*
* @param	OutputDivider is the integer output divider.
* @param	ClkoutReg1 is a pointer to store the first register value.
* @param	ClkoutReg2 is a pointer to store the second register value.
*
* @return	None.
*
* @note		OutputDivider is assumed to be integer only.
*
*****************************************************************************/
static void XMipi_Rx_Phy_MmcmClkout0Regs(u32 OutputDivider, u16 *ClkoutReg1,
					  u16 *ClkoutReg2)
{
	u32 HalfDivider;
	u32 HighTime;
	u32 LowTime;
	u32 OddEdgeFlag;

	/* Verify arguments */
	Xil_AssertVoid(ClkoutReg1 != NULL);
	Xil_AssertVoid(ClkoutReg2 != NULL);

	HalfDivider = OutputDivider / 2U;
	HighTime = HalfDivider / 2U;
	LowTime = HalfDivider - HighTime;
	OddEdgeFlag = LowTime - HighTime;

	/* Force 50% duty: LowTime = HighTime */
	LowTime = HighTime;

	if ((OutputDivider % 2U) == 0U) {
		/* Even divide */
		*ClkoutReg1 = (OddEdgeFlag == 0U) ?
			      XMIPI_RX_PHY_CLKOUT_EVEN_NO_EDGE :
			      XMIPI_RX_PHY_CLKOUT_EVEN_EDGE;
	} else {
		/* Odd divide */
		*ClkoutReg1 = (OddEdgeFlag == 0U) ?
			      XMIPI_RX_PHY_CLKOUT_ODD_NO_EDGE :
			      XMIPI_RX_PHY_CLKOUT_ODD_EDGE;
	}

	*ClkoutReg2 = XMipi_Rx_Phy_MmcmPackHtLt(HighTime, LowTime);
}

/****************************************************************************/
/**
* Compute the MMCM parameters and DRP writes for a given line rate.
*
* @param	LineRate is the desired line rate in Mbps.
*
* @return	MmcmResult structure with Valid set if a solution was found.
*
* @note		Uses double-precision floating point arithmetic matching
*		the reference MMCM C-code algorithm.
*
*****************************************************************************/
static XMipi_Rx_Phy_MmcmResult XMipi_Rx_Phy_MmcmCompute(u32 LineRate)
{
	XMipi_Rx_Phy_MmcmResult Result;
	u32 KFactor;
	u32 OutputDiv;
	double FoutTarget;
	double VcoFreq;
	double MultiplierIdeal;
	double MultiplierQuantized;
	double VcoQuantized;
	double FoutActual;
	double ErrorPpm;
	double CandidateScore;
	double BestScore;
	u32 BestOutputDiv;
	double BestMultiplierQuantized;
	u32 FracPart;
	s32 MultiplierScaled;

	/* Initialize result */
	Result.Valid = 0U;
	Result.InputDivider = XMIPI_RX_PHY_MMCM_D_FIXED;

	/* Derive K from line rate */
	KFactor = XMipi_Rx_Phy_MmcmDeriveK(LineRate);
	if (KFactor == 0U) {
		return Result;
	}
	Result.KFactor = KFactor;

	FoutTarget = (double)LineRate / (double)KFactor;

	BestScore = XMIPI_RX_PHY_SCORE_INIT;
	BestOutputDiv = 0U;
	BestMultiplierQuantized = 0.0;

	for (OutputDiv = XMIPI_RX_PHY_MMCM_O_MIN;
	     OutputDiv <= XMIPI_RX_PHY_MMCM_O_MAX; OutputDiv++) {

		VcoFreq = FoutTarget * (double)OutputDiv;
		if ((VcoFreq < (double)XMIPI_RX_PHY_MMCM_VCO_MIN_MHZ) ||
		    (VcoFreq > (double)XMIPI_RX_PHY_MMCM_VCO_MAX_MHZ)) {
			continue;
		}

		MultiplierIdeal = VcoFreq * (double)Result.InputDivider /
				  (double)XMIPI_RX_PHY_MMCM_FIN_MHZ;
		if ((MultiplierIdeal < (double)XMIPI_RX_PHY_MMCM_M_MIN) ||
		    (MultiplierIdeal > (double)XMIPI_RX_PHY_MMCM_M_MAX)) {
			continue;
		}

		/* Quantize M to 1/64 resolution */
		MultiplierQuantized = XMIPI_RX_PHY_ROUND_DBL(MultiplierIdeal *
				(double)XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION) /
				(double)XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION;

		/* Recompute VCO with quantized M */
		VcoQuantized = ((double)XMIPI_RX_PHY_MMCM_FIN_MHZ /
			       (double)Result.InputDivider) *
			       MultiplierQuantized;
		if ((VcoQuantized < (double)XMIPI_RX_PHY_MMCM_VCO_MIN_MHZ) ||
		    (VcoQuantized > (double)XMIPI_RX_PHY_MMCM_VCO_MAX_MHZ)) {
			continue;
		}

		FoutActual = (double)XMIPI_RX_PHY_MMCM_FIN_MHZ *
			     MultiplierQuantized /
			     ((double)Result.InputDivider * (double)OutputDiv);
		ErrorPpm = (FoutActual - FoutTarget) / FoutTarget *
			   XMIPI_RX_PHY_PPM_SCALE;

		/* Compute fractional part for tie-break */
		FracPart = (u32)XMIPI_RX_PHY_ROUND_S32((MultiplierQuantized -
			   XMIPI_RX_PHY_FLOOR(MultiplierQuantized)) *
			   (double)XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION);
		if (FracPart == XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION) {
			FracPart = 0U;
		}

		/* Compute candidate score: prioritize low PPM error,
		 * mid-band VCO, smaller output divider, integer M */
		CandidateScore = XMIPI_RX_PHY_ABS(ErrorPpm) * XMIPI_RX_PHY_SCORE_PPM_WEIGHT +
			XMIPI_RX_PHY_ABS(VcoFreq - (double)XMIPI_RX_PHY_MMCM_VCO_MID_MHZ) +
			XMIPI_RX_PHY_SCORE_O_WEIGHT * (double)OutputDiv +
			(FracPart ? XMIPI_RX_PHY_SCORE_FRAC_PENALTY : 0.0);

		if (CandidateScore < BestScore) {
			BestScore = CandidateScore;
			BestOutputDiv = OutputDiv;
			BestMultiplierQuantized = MultiplierQuantized;
		}
	}

	if (BestOutputDiv == 0U) {
		return Result;
	}

	Result.OutputDivider = BestOutputDiv;

	/* Split quantized M into integer + fractional (frac/64) parts */
	MultiplierScaled = XMIPI_RX_PHY_ROUND_S32(BestMultiplierQuantized *
			   (double)XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION);
	Result.MultiplierInt = (u32)(MultiplierScaled /
			       (s32)XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION);
	Result.MultiplierFrac = (u32)(MultiplierScaled %
				(s32)XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION);
	if ((s32)Result.MultiplierFrac < 0) {
		Result.MultiplierFrac += XMIPI_RX_PHY_MMCM_FRAC_RESOLUTION;
	}

	Result.Valid = 1U;

	return Result;
}

/****************************************************************************/
/**
* This function configures the MMCM registers in the RX PHY for a given
* line rate. It computes the optimal MMCM parameters (M, D, O) and writes
* the corresponding values to the PHY MMCM registers.
*
* @param	InstancePtr is the XMipi_Rx_Phy instance to operate on.
* @param	LineRate is the desired line rate in Mbps (400-4500 Mbps).
*
* @return
*		- XST_SUCCESS if MMCM programming is successful.
*		- XST_FAILURE if no valid MMCM solution is found.
*
* @note		None.
*
*****************************************************************************/
u32 XMipi_Rx_Phy_MmcmConfig(XMipi_Rx_Phy *InstancePtr, u32 LineRate)
{
	XMipi_Rx_Phy_MmcmResult Result;
	UINTPTR BaseAddr;
	u32 MmcmMultRegVal;
	u32 MmcmFractRegVal;
	u32 MmcmOutputDivRegVal;
	u32 MmcmInputDivRegVal;
	u32 LineRateRegVal;
	u32 MultiplierHalfTime;
	u16 ClkoutReg1;
	u16 ClkoutReg2;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	BaseAddr = InstancePtr->Config.BaseAddr;

	/* Compute MMCM parameters for the requested line rate */
	Result = XMipi_Rx_Phy_MmcmCompute(LineRate);
	if (Result.Valid == 0U) {
		xdbg_printf(XDBG_DEBUG_ERROR, "MIPI RX PHY ERR:: No valid"
			 " MMCM solution for line rate %u Mbps\n\r", LineRate);
		return XST_FAILURE;
	}

	/* PHY 0x34: MMCM_MULT - M integer
	 * [15:0]  = M_REG1: odd/even flag encoding
	 * [31:16] = M_REG2: pack_htlt(M_int/2, M_int/2)
	 */
	MultiplierHalfTime = Result.MultiplierInt / 2U;
	MmcmMultRegVal = ((u32)XMipi_Rx_Phy_MmcmPackHtLt(MultiplierHalfTime,
			  MultiplierHalfTime) << 16U) |
			 ((Result.MultiplierInt & 1U) ?
			  XMIPI_RX_PHY_MMCM_MULT_ODD_FLAG :
			  XMIPI_RX_PHY_MMCM_MULT_EVEN_FLAG);

	/* PHY 0x38: MMCM_FRACT_MULT - M fractional
	 * [15:0]  = M_FRAC_EN:  enable bit if fractional, 0 otherwise
	 * [31:16] = M_FRAC_VAL: fractional value if fractional, 0 otherwise
	 */
	if (Result.MultiplierFrac != 0U) {
		MmcmFractRegVal = ((Result.MultiplierFrac &
				   XMIPI_RX_PHY_MMCM_FRAC_MASK) << 16U) |
				  XMIPI_RX_PHY_MMCM_FRAC_ENABLE;
	} else {
		MmcmFractRegVal = 0U;
	}

	/* PHY 0x3C: MMCM_DIV - D integer (fixed D=5)
	 * [15:0]  = D_REG1: encoding for D=5 (odd)
	 * [31:16] = D_REG2: pack_htlt(2, 2)
	 */
	MmcmInputDivRegVal = ((u32)XMipi_Rx_Phy_MmcmPackHtLt(
			      XMIPI_RX_PHY_MMCM_DIV_D5_HTLT_VAL,
			      XMIPI_RX_PHY_MMCM_DIV_D5_HTLT_VAL) << 16U) |
			     XMIPI_RX_PHY_MMCM_DIV_D5_REG1;

	/* PHY 0x40: MMCM_O - O integer (CLKOUT0 encoding) */
	XMipi_Rx_Phy_MmcmClkout0Regs(Result.OutputDivider, &ClkoutReg1,
				      &ClkoutReg2);
	MmcmOutputDivRegVal = ((u32)ClkoutReg2 << 16U) | (u32)ClkoutReg1;

	/* Write the line rate to the DYNAMIC_LINE_RATE register (0x0C) */
	LineRateRegVal = XMipi_Rx_Phy_ReadReg(BaseAddr,
				XMIPI_RX_PHY_DYNAMIC_LINERATE_REG_OFFSET);
	LineRateRegVal = (LineRateRegVal &
			  ~XMIPI_RX_PHY_DYNAMIC_LINERATE_REG_LINERATE_MASK) |
			 (LineRate &
			  XMIPI_RX_PHY_DYNAMIC_LINERATE_REG_LINERATE_MASK);

	/* Write MMCM_MULT register (0x34) - M integer values */
	XMipi_Rx_Phy_WriteReg(BaseAddr,
			XMIPI_RX_PHY_MMCM_MULT_REG_OFFSET, MmcmMultRegVal);

	/* Write MMCM_FRACT_MULT register (0x38) - M fractional values */
	XMipi_Rx_Phy_WriteReg(BaseAddr,
			XMIPI_RX_PHY_MMCM_FRACT_MULT_REG_OFFSET,
			MmcmFractRegVal);

	/* Write MMCM_DIV register (0x3C) - D integer values */
	XMipi_Rx_Phy_WriteReg(BaseAddr,
			XMIPI_RX_PHY_MMCM_DIV_REG_OFFSET, MmcmInputDivRegVal);

	/* Write MMCM_O register (0x40) - O integer values */
	XMipi_Rx_Phy_WriteReg(BaseAddr,
			XMIPI_RX_PHY_MMCM_O_REG_OFFSET, MmcmOutputDivRegVal);

	XMipi_Rx_Phy_WriteReg(BaseAddr,
			XMIPI_RX_PHY_DYNAMIC_LINERATE_REG_OFFSET,
			LineRateRegVal);

	return XST_SUCCESS;
}

/** @} */
