/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfexmix.c
* Contains the APIs for DFE Mixer component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/21/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/15/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     03/18/21 New model parameter list
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/08/21 Set sequence length only once
*       dc     04/14/21 Add FIR_ENABLE/MIXER_ENABLE register support
*       dc     04/18/21 Update trigger and event handlers
*       dc     04/20/21 Doxygen documentation update
*       dc     04/22/21 Add CC_GAIN field
*       dc     04/27/21 Update CARRIER_CONFIGURATION handling
*       dc     05/08/21 Update to common trigger
*       dc     05/18/21 Handling CCUpdate trigger
* 1.1   dc     07/13/21 Update to common latency requirements
*       dc     07/21/21 Add and reorganise examples
*       dc     10/26/21 Make driver R5 compatible
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/19/21 Update doxygen documentation
*       dc     11/26/21 Set NCO configuration in GetCurrentCCCfg
*       dc     11/26/21 Set sequence length in GetEmptyCCCfg
*       dc     11/26/21 Add SetAntennaCfgInCCCfg API
*       dc     11/30/21 Convert AntennaCfg to structure
*       dc     12/02/21 Add UpdateAntennaCfg API
*       dc     12/17/21 Update after documentation review
* 1.3   dc     01/07/22 NCO assignment in arch4 mode
*       dc     01/19/22 Assert CCUpdate trigger
*       dc     02/10/22 Add latency information
*       dc     03/21/22 Add prefix to global variables
*
* </pre>
* @addtogroup Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/
#include "xdfemix.h"
#include "xdfemix_hw.h"
#include <math.h>
#include <metal/io.h>
#include <metal/device.h>
#include <string.h>

#ifdef __BAREMETAL__
#include "sleep.h"
#else
#include <unistd.h>
#endif

/**************************** Macros Definitions ****************************/
#define XDFEMIX_SEQUENCE_ENTRY_DEFAULT (0U) /* Default sequence entry flag */
#define XDFEMIX_SEQUENCE_ENTRY_NULL (-1) /* Null sequence entry flag */
#define XDFEMIX_NO_EMPTY_CCID_FLAG (0xFFFFU) /* Not Empty CCID flag */
#define XDFEMIX_U32_NUM_BITS (32U)
#define XDFEMIXER_CURRENT false
#define XDFEMIXER_NEXT true
#define XDFEMIXER_PHACC_DISABLE false
#define XDFEMIXER_PHACC_ENABLE true
#define XDFEMIX_PHASE_OFFSET_ROUNDING_BITS (14U) /**< Number of rounding bits */
#define XDFEMIX_TAP_MAX (24U) /**< Maximum tap value */
#define XDFEMIX_IS_ARCH4_MODE                                                  \
	((InstancePtr->Config.MaxUseableCcids == 8U) &&                        \
	 (InstancePtr->Config.Lanes > 1U)) /* Arch4 mode logical statement */
#define XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE (4U) /* NCO low sub-block size */
/**
* @endcond
*/
#define XDFEMIX_DRIVER_VERSION_MINOR (3U) /**< Driver's minor version number */
#define XDFEMIX_DRIVER_VERSION_MAJOR (1U) /**< Driver's major version number */

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
/**
* @cond nocomments
*/

#ifdef __BAREMETAL__
extern struct metal_device XDfeMix_CustomDevice[XDFEMIX_MAX_NUM_INSTANCES];
extern metal_phys_addr_t XDfeMix_metal_phys[XDFEMIX_MAX_NUM_INSTANCES];
#endif
extern XDfeMix XDfeMix_Mixer[XDFEMIX_MAX_NUM_INSTANCES];
static u32 XDfeMix_DriverHasBeenRegisteredOnce = 0U;

/************************** Function Definitions ****************************/
extern s32 XDfeMix_RegisterMetal(XDfeMix *InstancePtr,
				 struct metal_device **DevicePtr,
				 const char *DeviceNodeName);
extern s32 XDfeMix_LookupConfig(XDfeMix *InstancePtr);
extern void XDfeMix_CfgInitialize(XDfeMix *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Writes a value to register in a Mixer instance.
*
* @param    InstancePtr Pointer to the Mixer driver instance.
* @param    AddrOffset Address offset relative to instance base address.
* @param    Data Value to be written.
*
****************************************************************************/
void XDfeMix_WriteReg(const XDfeMix *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, (unsigned long)AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Reads a value from the register from a Mixer instance.
*
* @param    InstancePtr Pointer to the Mixer driver instance.
* @param    AddrOffset Address offset relative to instance base address.
*
* @return   Register value.
*
****************************************************************************/
u32 XDfeMix_ReadReg(const XDfeMix *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, (unsigned long)AddrOffset);
}

/****************************************************************************/
/**
*
* Writes a bit field value to register.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
* @param    FieldData Bit field data.
*
****************************************************************************/
void XDfeMix_WrRegBitField(const XDfeMix *InstancePtr, u32 Offset,
			   u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);

	Data = XDfeMix_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfeMix_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads a bit field value from the register.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
*
* @return   Bit field data.
*
****************************************************************************/
u32 XDfeMix_RdRegBitField(const XDfeMix *InstancePtr, u32 Offset,
			  u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);

	Data = XDfeMix_ReadReg(InstancePtr, Offset);
	return ((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U));
}

/****************************************************************************/
/**
*
* Reads a bit field value from u32 variable.
*
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 data which bit field this function reads.
*
* @return   Bit field value.
*
****************************************************************************/
u32 XDfeMix_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);
	return (((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U)));
}
/****************************************************************************/
/**
*
* Writes a bit field value to u32 variable.
*
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 data which bit field this function reads.
* @param    Val U32 value to be written in the bit field.
*
* @return   Data with a bit field written.
*
****************************************************************************/
u32 XDfeMix_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
{
	u32 BitFieldSet;
	u32 BitFieldClear;
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);

	BitFieldSet = (Val & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	BitFieldClear =
		Data & (~((((u32)1U << FieldWidth) - 1U) << FieldOffset));
	return (BitFieldSet | BitFieldClear);
}

/************************ Mixer Common functions ******************************/

/****************************************************************************/
/**
*
* Finds unused CCID.
*
* @param    Sequence CC sequence array.
*
* @return Unused CCID.
*
****************************************************************************/
static s32 XDfeMix_GetNotUsedCCID(XDfeMix_CCSequence *Sequence)
{
	u32 Index;
	s32 NotUsedCCID;

	/* Not used Sequence.CCID[] has value -1, but the values in the range
	   [0,15] can be written in the registers, only. Now, we have to detect
	   not used CCID, and save it for the later usage. */
	for (NotUsedCCID = 0U; NotUsedCCID < XDFEMIX_CC_NUM; NotUsedCCID++) {
		for (Index = 0U; Index < XDFEMIX_CC_NUM; Index++) {
			if (Sequence->CCID[Index] == NotUsedCCID) {
				break;
			}
		}
		if (Index == XDFEMIX_CC_NUM) {
			break;
		}
	}
	return (NotUsedCCID);
}

/****************************************************************************/
/**
*
* Count number of 1 in bitmap.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCSeqBitmap maps the sequence.
*
****************************************************************************/
static u32 XDfeMix_CountOnesInBitmap(const XDfeMix *InstancePtr,
				     u32 CCSeqBitmap)
{
	u32 Mask = 1U;
	u32 Index;
	s32 OnesCounter = 0U;
	for (Index = 0U; Index < InstancePtr->SequenceLength; Index++) {
		if (CCSeqBitmap & Mask) {
			OnesCounter++;
		}
		Mask <<= 1U;
	}
	return OnesCounter;
}

/****************************************************************************/
/**
*
* Basic calculation for NCO sub-block in ARCH4 mode.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    NCOIdx New NCO Id.
* @param    NCOLowSubBlockUsage.
* @param    NCOHighSubBlockUsage.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeMix_NCOArch4Mode(const XDfeMix *InstancePtr,
				XDfeMix_CCCfg *CCCfg, u32 NCOIdx,
				u32 *NCOLowSubBlockUsage,
				u32 *NCOHighSubBlockUsage)
{
	u32 Index;
	u32 CCSeqBitmapTmp;
	XDfeMix_CarrierCfg CarrierCfgTmp;
	XDfeMix_NCO NCOIdxTmp;

	for (Index = 0U; Index < XDFEMIX_CC_NUM; Index++) {
		/* Check is this CCID disabled */
		if (CCCfg->DUCDDCCfg[Index].Rate == 0) {
			continue;
		}

		/* Check is NCOIdx already used */
		if (NCOIdx == CCCfg->DUCDDCCfg[Index].NCOIdx) {
			metal_log(METAL_LOG_ERROR, "NCOIdx is already used%s\n",
				  __func__);
			return XST_FAILURE;
		}

		/* get CCSeqBitmapTmp for CCID=Index */
		XDfeMix_GetCarrierCfgAndNCO(InstancePtr, CCCfg, Index,
					    &CCSeqBitmapTmp, &CarrierCfgTmp,
					    &NCOIdxTmp);

		/* Is NCOIdx for Low (0-3) or High (4-7) sub-block */
		if (CCCfg->DUCDDCCfg[Index].NCOIdx <
		    XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE) {
			*NCOLowSubBlockUsage += XDfeMix_CountOnesInBitmap(
				InstancePtr, CCSeqBitmapTmp);
		} else {
			*NCOHighSubBlockUsage += XDfeMix_CountOnesInBitmap(
				InstancePtr, CCSeqBitmapTmp);
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Check is NCO usage over 50% of maximum for NCO sub_block
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID.
* @param    CCSeqBitmap CC slot position container.
* @param    NCOIdx New NCO Id to be used from now on.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeMix_NCOArch4ModeInAddCC(const XDfeMix *InstancePtr,
				       XDfeMix_CCCfg *CCCfg, s32 CCID,
				       u32 CCSeqBitmap, u32 NCOIdx)
{
	u32 NCOLowSubBlockUsage = 0;
	u32 NCOHighSubBlockUsage = 0;
	u32 NCOSubBlockUsage = 0;

	if (XST_FAILURE == XDfeMix_NCOArch4Mode(InstancePtr, CCCfg, NCOIdx,
						&NCOLowSubBlockUsage,
						&NCOHighSubBlockUsage)) {
		return XST_FAILURE;
	}

	/* Add new CCID usage to NCO sub-block usage */
	if (NCOIdx < XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE) {
		NCOSubBlockUsage =
			NCOLowSubBlockUsage +
			XDfeMix_CountOnesInBitmap(InstancePtr, CCSeqBitmap);
	} else {
		NCOSubBlockUsage =
			NCOHighSubBlockUsage +
			XDfeMix_CountOnesInBitmap(InstancePtr, CCSeqBitmap);
	}

	/* Check is usage over 50% for this NCO sub-block */
	if (NCOSubBlockUsage * 2U > InstancePtr->SequenceLength) {
		metal_log(METAL_LOG_ERROR,
			  "NCO usage overflow 50%% of NCO sub-block for "
			  "CCID=%d, NCOIdx=%d in %s\n",
			  CCID, NCOIdx, __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Check is NCO usage over 50% of maximum for NCO sub-block.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID.
* @param    NCOIdx New NCO Id to be used from now on.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeMix_NCOArch4ModeInMoveOrUpdateCC(const XDfeMix *InstancePtr,
						XDfeMix_CCCfg *CCCfg, s32 CCID,
						u32 NCOIdx)
{
	u32 NCOLowSubBlockUsage = 0;
	u32 NCOHighSubBlockUsage = 0;
	u32 NCOSubBlockUsage = 0;
	u32 CCSeqBitmapTmp;
	XDfeMix_CarrierCfg CarrierCfgTmp;
	XDfeMix_NCO NCOTmp;

	if (XST_FAILURE == XDfeMix_NCOArch4Mode(InstancePtr, CCCfg, NCOIdx,
						&NCOLowSubBlockUsage,
						&NCOHighSubBlockUsage)) {
		return XST_FAILURE;
	}

	/* get CCSeqBitmapTmp for CCID */
	XDfeMix_GetCarrierCfgAndNCO(InstancePtr, CCCfg, CCID, &CCSeqBitmapTmp,
				    &CarrierCfgTmp, &NCOTmp);

	/* Add new CCID usage to NCO sub-block usage */
	if ((NCOIdx < XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE) &&
	    (CCCfg->DUCDDCCfg[CCID].NCOIdx >= XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE)) {
		NCOSubBlockUsage =
			NCOLowSubBlockUsage +
			XDfeMix_CountOnesInBitmap(InstancePtr, CCSeqBitmapTmp);
	} else if ((NCOIdx >= XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE) &&
		   (CCCfg->DUCDDCCfg[CCID].NCOIdx <
		    XDFEMIX_NCO_LOW_SUB_BLOCK_SIZE)) {
		{
			NCOSubBlockUsage = NCOHighSubBlockUsage +
					   XDfeMix_CountOnesInBitmap(
						   InstancePtr, CCSeqBitmapTmp);
		}
	} else {
		return XST_SUCCESS;
	}

	/* Check is usage over 50% for this NCO sub-block */
	if (NCOSubBlockUsage * 2U > InstancePtr->SequenceLength) {
		metal_log(METAL_LOG_ERROR,
			  "NCO usage overflow 50%% of NCO sub-block for "
			  "CCID=%d, NCOIdx=%d in %s\n",
			  CCID, NCOIdx, __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Adds the specified CCID, to the CC sequence. The sequence is defined with
* CCSeqBitmap where bit0 coresponds to CC[0], bit1 to CC[1], and so on.
*
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID CC ID.
* @param    CCSeqBitmap maps the sequence.
* @param    CCIDSequence CC sequence array.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
****************************************************************************/
static u32 XDfeMix_AddCCIDAndTranslateSeq(XDfeMix *InstancePtr, s32 CCID,
					  u32 CCSeqBitmap,
					  XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 Mask;
	s32 OnesCounter = 0U;

	/* Check does sequence fit in the defined length */
	Mask = (1U << CCIDSequence->Length) - 1U;
	if (0U != (CCSeqBitmap & (~Mask))) {
		metal_log(METAL_LOG_ERROR, "Sequence map overflow\n");
		return XST_FAILURE;
	}

	/* Count ones in bitmap */
	OnesCounter = XDfeMix_CountOnesInBitmap(InstancePtr, CCSeqBitmap);

	/* Validate is number of ones a power of 2 */
	if ((OnesCounter != 0) && (OnesCounter != 1) && (OnesCounter != 2) &&
	    (OnesCounter != 4) && (OnesCounter != 8) && (OnesCounter != 16)) {
		metal_log(METAL_LOG_ERROR,
			  "Number of 1 in CCSeqBitmap is not power of 2: %d\n",
			  OnesCounter);
		return XST_FAILURE;
	}

	/* Check are bits set in CCSeqBitmap to 1 avaliable (-1)*/
	Mask = 1U;
	for (Index = 0U; Index < CCIDSequence->Length; Index++) {
		if (0U != (CCSeqBitmap & Mask)) {
			if (CCIDSequence->CCID[Index] !=
			    XDFEMIX_SEQUENCE_ENTRY_NULL) {
				metal_log(METAL_LOG_ERROR,
					  "Sequence does not fit\n");
				return XST_FAILURE;
			}
		}
		Mask <<= 1U;
	}

	/* Now, write the sequence */
	Mask = 1U;
	for (Index = 0U; Index < CCIDSequence->Length; Index++) {
		if (0U != (CCSeqBitmap & Mask)) {
			CCIDSequence->CCID[Index] = CCID;
		}
		Mask <<= 1U;
	}

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfeMix_GetNotUsedCCID(CCIDSequence);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes the specified CCID from the CC sequence and replaces the CCID
* entries with null (8).
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID CC ID.
* @param    CCIDSequence CC sequence array.
*
****************************************************************************/
static void XDfeMix_RemoveCCID(XDfeMix *InstancePtr, s32 CCID,
			       XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] = XDFEMIX_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfeMix_GetNotUsedCCID(CCIDSequence);
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Detect Rate.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCSeqBitmap Sequence map.
* @param    Rate Rate returned value
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
****************************************************************************/
static u32 XDfeMix_FindRate(const XDfeMix *InstancePtr, u32 CCSeqBitmap,
			    u32 *Rate)
{
	u32 Index;
	u32 Mask = 1U;
	u32 OnesCounter = 0U;
	u32 ConversionRatio;

	/* Detecting rate value */
	/* Validate is CCSeqBitmap inside SequnceLength */
	if ((CCSeqBitmap & ((1 << InstancePtr->SequenceLength) - 1)) !=
	    CCSeqBitmap) {
		metal_log(METAL_LOG_ERROR, "Sequence bitmap is overflowing\n");
		return XST_FAILURE;
	}

	/* Count ones in bitmap */
	for (Index = 0U; Index < InstancePtr->SequenceLength; Index++) {
		if (CCSeqBitmap & Mask) {
			OnesCounter++;
		}
		Mask <<= 1;
	}

	/* Validate is number of ones a power of 2 */
	if ((OnesCounter != 0) && (OnesCounter != 1) && (OnesCounter != 2) &&
	    (OnesCounter != 4) && (OnesCounter != 8) && (OnesCounter != 16)) {
		metal_log(METAL_LOG_ERROR,
			  "Number of ones in CCSeqBitmap is not power of 2\n");
		return XST_FAILURE;
	}

	/* Detect Rate */
	if (OnesCounter == 0) {
		*Rate = 0;
	} else {
		/* Calculate conversion ratio */
		ConversionRatio = InstancePtr->Config.AntennaInterleave *
				  (InstancePtr->SequenceLength / OnesCounter);
		/* Select Rate from Interpolation/decimation rate */
		switch (ConversionRatio) {
		case 1: /* 1: 1x interpolation/decimation */
			*Rate = XDFEMIX_CC_CONFIG_RATE_1X;
			break;
		case 2: /* 2: 2x interpolation/decimation */
			*Rate = XDFEMIX_CC_CONFIG_RATE_2X;
			break;
		case 4: /* 3: 4x interpolation/decimation */
			*Rate = XDFEMIX_CC_CONFIG_RATE_4X;
			break;
		case 8: /* 4: 8x interpolation/decimation */
			*Rate = XDFEMIX_CC_CONFIG_RATE_8X;
			break;
		case 16: /* 5: 16x interpolation/decimation */
			*Rate = XDFEMIX_CC_CONFIG_RATE_16X;
			break;
		default:
			metal_log(METAL_LOG_ERROR,
				  "Wrong conversion ratio %d\n",
				  ConversionRatio);
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Sets Rate and NCO in DUC-DDC configuration for CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Configuration data container.
* @param    CCID Channel ID.
* @param    CCSeqBitmap Sequence map.
* @param    DUCDDCCfg DUC/DDC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
****************************************************************************/
static u32 XDfeMix_SetCCDDC(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			    s32 CCID, u32 CCSeqBitmap,
			    const XDfeMix_DUCDDCCfg *DUCDDCCfg)
{
	u32 Rate = 0U;

	/* Check is NCOIdx below expected value */
	if (DUCDDCCfg->NCOIdx >= InstancePtr->Config.MaxUseableCcids) {
		metal_log(METAL_LOG_ERROR, "NCOIdx %d is greater than %d\n",
			  DUCDDCCfg->NCOIdx,
			  InstancePtr->Config.MaxUseableCcids);
		return XST_FAILURE;
	}

	/* Detecting rate value */
	if (XST_FAILURE == XDfeMix_FindRate(InstancePtr, CCSeqBitmap, &Rate)) {
		metal_log(METAL_LOG_ERROR, "Rate cannot be detected\n");
		return XST_FAILURE;
	}

	CCCfg->DUCDDCCfg[CCID].NCOIdx = DUCDDCCfg->NCOIdx;
	CCCfg->DUCDDCCfg[CCID].Rate = Rate;
	CCCfg->DUCDDCCfg[CCID].CCGain = DUCDDCCfg->CCGain;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Update NCO in DUC-DDC configuration for CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Configuration data container.
* @param    CCID Channel ID.
* @param    DUCDDCCfg DUC/DDC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
****************************************************************************/
static u32 XDfeMix_UpdateCCDDC(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			       s32 CCID, const XDfeMix_DUCDDCCfg *DUCDDCCfg)
{
	/* Check is NCOIdx below expected value */
	if (DUCDDCCfg->NCOIdx >= InstancePtr->Config.MaxUseableCcids) {
		metal_log(METAL_LOG_ERROR, "NCOIdx %d is greater than %d\n",
			  DUCDDCCfg->NCOIdx,
			  InstancePtr->Config.MaxUseableCcids);
		return XST_FAILURE;
	}
	CCCfg->DUCDDCCfg[CCID].NCOIdx = DUCDDCCfg->NCOIdx;
	CCCfg->DUCDDCCfg[CCID].CCGain = DUCDDCCfg->CCGain;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Writes NEXT CC configuration.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Configuration data container.
*
****************************************************************************/
static void XDfeMix_SetNextCCCfg(const XDfeMix *InstancePtr,
				 const XDfeMix_CCCfg *NextCCCfg)
{
	u32 AntennaCfg = 0U;
	u32 DucDdcConfig;
	u32 Index;
	u32 SeqLength;
	s32 NextCCID[XDFEMIX_SEQ_LENGTH_MAX];

	/* Prepare NextCCID[] to be written to registers */
	for (Index = 0U; Index < XDFEMIX_CC_NUM; Index++) {
		if ((NextCCCfg->Sequence.CCID[Index] ==
		     XDFEMIX_SEQUENCE_ENTRY_NULL) ||
		    (Index >= InstancePtr->SequenceLength)) {
			NextCCID[Index] = InstancePtr->NotUsedCCID;
		} else {
			NextCCID[Index] = NextCCCfg->Sequence.CCID[Index];
		}
	}

	/* Sequence Length should remain the same, so copy the sequence length
	   from CURRENT to NEXT, does not take from NextCCCfg. The reason
	   is that NextCCCfg->Sequence.SeqLength can be 0 or 1 for the value 0
	   in the CURRENT seqLength register */
	SeqLength =
		XDfeMix_ReadReg(InstancePtr, XDFEMIX_SEQUENCE_LENGTH_CURRENT);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_SEQUENCE_LENGTH_NEXT, SeqLength);

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEMIX_CC_NUM; Index++) {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_SEQUENCE_NEXT + (sizeof(u32) * Index),
				 NextCCID[Index]);

		DucDdcConfig = XDfeMix_ReadReg(InstancePtr,
					       XDFEMIX_CC_CONFIG_NEXT +
						       ((Index * sizeof(u32))));
		DucDdcConfig =
			XDfeMix_WrBitField(XDFEMIX_CC_CONFIG_NCO_WIDTH,
					   XDFEMIX_CC_CONFIG_NCO_OFFSET,
					   DucDdcConfig,
					   NextCCCfg->DUCDDCCfg[Index].NCOIdx);
		DucDdcConfig =
			XDfeMix_WrBitField(XDFEMIX_CC_CONFIG_RATE_WIDTH,
					   XDFEMIX_CC_CONFIG_RATE_OFFSET,
					   DucDdcConfig,
					   NextCCCfg->DUCDDCCfg[Index].Rate);
		DucDdcConfig =
			XDfeMix_WrBitField(XDFEMIX_CC_CONFIG_CC_GAIN_WIDTH,
					   XDFEMIX_CC_CONFIG_CC_GAIN_OFFSET,
					   DucDdcConfig,
					   NextCCCfg->DUCDDCCfg[Index].CCGain);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_CC_CONFIG_NEXT +
					 ((Index * sizeof(u32))),
				 DucDdcConfig);
	}

	/* Write Antenna configuration */
	for (Index = 0; Index < XDFEMIX_ANT_NUM_MAX; Index++) {
		AntennaCfg += (NextCCCfg->AntennaCfg.Gain[Index] << Index);
	}
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_ANTENNA_GAIN_NEXT, AntennaCfg);
}

/****************************************************************************/
/**
*
* Gets PHACC index from the DUC/DDC Mapping NCO.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
*
* @return   Index
*
****************************************************************************/
static u32 XDfeMix_GetPhaccIndex(const XDfeMix *InstancePtr, bool Next,
				 s32 CCID)
{
	u32 Offset;
	u32 Nco;

	if (Next == XDFEMIXER_NEXT) {
		Offset = XDFEMIX_CC_CONFIG_NEXT;
	} else {
		Offset = XDFEMIX_CC_CONFIG_CURRENT;
	}
	Offset += CCID * sizeof(u32);
	Nco = XDfeMix_RdRegBitField(InstancePtr, Offset,
				    XDFEMIX_CC_CONFIG_NCO_WIDTH,
				    XDFEMIX_CC_CONFIG_NCO_OFFSET);
	return (Nco * XDFEMIX_PHAC_CCID_ADDR_STEP);
}

/****************************************************************************/
/**
*
* Writes the frequency settings for a given CC's phase accumulator.
* The frequency settings for a given CC are shared across all antennas.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
* @param    Freq Frequency setting for CC.
*
****************************************************************************/
static void XDfeMix_SetCCFrequency(const XDfeMix *InstancePtr, bool Next,
				   s32 CCID, const XDfeMix_Frequency *Freq)
{
	u32 Index;

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_CONTROL_WORD + Index,
			 Freq->FrequencyControlWord);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_SINGLE_MOD_COUNT + Index,
			 Freq->SingleModCount);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_DUAL_MOD_COUNT + Index,
			 Freq->DualModCount);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_PHASE_OFFSET + Index,
			 Freq->PhaseOffset.PhaseOffset);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_UPDATE + Index, 1U);
}

/****************************************************************************/
/**
*
* Reads back frequency for particular CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
* @param    Freq Frequency setting for CC.
*
****************************************************************************/
static void XDfeMix_GetCCFrequency(const XDfeMix *InstancePtr, bool Next,
				   s32 CCID, XDfeMix_Frequency *Freq)
{
	u32 Index;

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	Freq->FrequencyControlWord =
		XDfeMix_ReadReg(InstancePtr, XDFEMIX_FREQ_CONTROL_WORD + Index);
	Freq->SingleModCount = XDfeMix_ReadReg(
		InstancePtr, XDFEMIX_FREQ_SINGLE_MOD_COUNT + Index);
	Freq->DualModCount = XDfeMix_ReadReg(
		InstancePtr, XDFEMIX_FREQ_DUAL_MOD_COUNT + Index);
	Freq->PhaseOffset.PhaseOffset =
		XDfeMix_ReadReg(InstancePtr, XDFEMIX_FREQ_PHASE_OFFSET + Index);
}

/****************************************************************************/
/**
*
* Writes the phase settings for a given CC's phase accumulator. The frequency
* settings for a given CC are shared across all antennas.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
* @param    Phase Phase setting for CC.
*
****************************************************************************/
static void XDfeMix_SetCCPhase(const XDfeMix *InstancePtr, bool Next, s32 CCID,
			       const XDfeMix_Phase *Phase)
{
	u32 Index;

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PHASE_UPDATE_ACC + Index,
			 Phase->PhaseAcc);
	XDfeMix_WriteReg(InstancePtr,
			 XDFEMIX_PHASE_UPDATE_DUAL_MOD_COUNT + Index,
			 Phase->DualModCount);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PHASE_UPDATE_DUAL_MOD_SEL + Index,
			 Phase->DualModSel);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PHASE_UPDATE + Index, 1U);
}

/****************************************************************************/
/**
*
* Reads back phase from AXI-lite registers for particular CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
* @param    Phase Phase setting for CC.
*
****************************************************************************/
static void XDfeMix_GetCCPhase(const XDfeMix *InstancePtr, bool Next, s32 CCID,
			       XDfeMix_Phase *Phase)
{
	u32 Index;

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	Phase->PhaseAcc =
		XDfeMix_ReadReg(InstancePtr, XDFEMIX_PHASE_CAPTURE_ACC + Index);
	Phase->DualModCount = XDfeMix_ReadReg(
		InstancePtr, XDFEMIX_PHASE_CAPTURE_DUAL_MOD_COUNT + Index);
	Phase->DualModSel = XDfeMix_ReadReg(
		InstancePtr, XDFEMIX_PHASE_CAPTURE_DUAL_MOD_SEL + Index);
}

/****************************************************************************/
/**
*
* Enables the phase accumulator for a particular CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
* @param    Enable Flag that enables a phase accumulator.
*
****************************************************************************/
static void XDfeMix_SetCCPhaseAccumEnable(const XDfeMix *InstancePtr, bool Next,
					  s32 CCID, bool Enable)
{
	u32 Index;
	u32 Data = 0U;

	if (Enable == XDFEMIXER_PHACC_ENABLE) {
		Data = 1U;
	}
	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PHASE_ACC_ENABLE + Index, Data);
}

/****************************************************************************/
/**
*
* Captures phase for all phase accumulators in associated AXI-lite registers.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_CapturePhase(const XDfeMix *InstancePtr)
{
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_MIXER_PHASE_CAPTURE,
			 XDFEMIX_MIXER_PHASE_CAPTURE_ON);
}

/****************************************************************************/
/**
*
* Subtracts phase B from phase A to give phase D. These consider full
* phase configurations so that difference will be as accurate as possible.
*
* The delta in phase accumulator is given by:
* PhaseAccDiff = PhaseB.PhaseAcc - PhaseA.PhaseAcc
* Phase offset is only an 18-bit quantity, so it is obtained from PhaseAcc by
* PhaseAcc by dividing by 2^14. Rounding before converting back to an
* unsigned integer will provide better accuracy.
*
* @param    PhaseA Phase A descriptor.
* @param    PhaseB Phase B descriptor.
* @param    PhaseOffset Phase offset descriptor.
*
* @note     PhaseAcc can also be interpreted as an unsigned quantity with
*           the difference causing a wrap-around of a full cycle to give
*           a positive phase when otherwise a negative number would be
*           generated.
*
****************************************************************************/
static void XDfeMix_DerivePhaseOffset(const XDfeMix_Phase *PhaseA,
				      const XDfeMix_Phase *PhaseB,
				      XDfeMix_PhaseOffset *PhaseOffset)
{
	u32 PhaseAccDiff;

	PhaseAccDiff = PhaseB->PhaseAcc - PhaseA->PhaseAcc;
	PhaseOffset->PhaseOffset =
		PhaseAccDiff >> XDFEMIX_PHASE_OFFSET_ROUNDING_BITS;
	/* Add 1 if bit 13 = 1 which means that rounding is greater or equal
	   than (2^14)/2 */
	if (0U != (PhaseAccDiff &
		   ((u32)1 << (XDFEMIX_PHASE_OFFSET_ROUNDING_BITS - 1U)))) {
		PhaseOffset->PhaseOffset += 1U;
	}
}

/****************************************************************************/
/**
*
* Sets phase offset component of frequency.
*
* @param    Frequency Frequency container.
* @param    PhaseOffset Phase offset container.
*
****************************************************************************/
static void XDfeMix_SetPhaseOffset(XDfeMix_Frequency *Frequency,
				   const XDfeMix_PhaseOffset *PhaseOffset)
{
	Frequency->PhaseOffset = *PhaseOffset;
}

/****************************************************************************/
/**
*
* Sets NCO output attenuation.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
* @param    NCOGain NCO attenuation.
*
****************************************************************************/
static void XDfeMix_SetCCNCOGain(const XDfeMix *InstancePtr, bool Next,
				 s32 CCID, u32 NCOGain)
{
	u32 Index;

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_NCO_GAIN + Index, NCOGain);
}

/****************************************************************************/
/**
*
* Gets NCO output attenuation.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Next TRUE - read next config, FALSE - read current config.
* @param    CCID Channel ID.
*
* @return   NCO attenuation.
*
****************************************************************************/
static u32 XDfeMix_GetCCNCOGain(const XDfeMix *InstancePtr, bool Next, s32 CCID)
{
	u32 Index;

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	return XDfeMix_ReadReg(InstancePtr, XDFEMIX_NCO_GAIN + Index);
}

/****************************************************************************/
/**
*
* Writes register CORE.PL_MIXER_DELAY with value 2.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_SetPLMixerDelay(const XDfeMix *InstancePtr)
{
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PL_MIXER_DELAY,
			 XDFEMIX_PL_MIXER_DELAY_VALUE);
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of update trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeMix_EnableCCUpdateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	/* Exit with error if CC_UPDATE status is high */
	if (XDFEMIX_CC_UPDATE_TRIGGERED_HIGH ==
	    XDfeMix_RdRegBitField(InstancePtr, XDFEMIX_ISR,
				  XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET)) {
		metal_log(METAL_LOG_ERROR, "CCUpdate status high in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Enable CCUpdate trigger */
	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Data);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of LowPower trigger.
* If Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_EnableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set enable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_EnableActivateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  XDFEMIX_TRIGGERS_STATE_OUTPUT_ENABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set disable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_EnableDeactivateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  XDFEMIX_TRIGGERS_STATE_OUTPUT_DISABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers and resets enable a bit of LowPower trigger.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_DisableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/**
* @endcond
*/

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API initialises one instance of a Mixer driver.
* Traverses "/sys/bus/platform/device" directory (in Linux), to find registered
* XDfeMix device with the name DeviceNodeName. The first available slot in
* the instances array XDfeMix_Mixer[] will be taken as a DeviceNodeName
* object. On success it moves the state machine to a Ready state, while on
* failure stays in a Not Ready state.
*
* @param    DeviceNodeName Device node name.
*
* @return
*           - Pointer to the instance if successful.
*           - NULL on error.
*
******************************************************************************/
XDfeMix *XDfeMix_InstanceInit(const char *DeviceNodeName)
{
	u32 Index;
	XDfeMix *InstancePtr;
#ifdef __BAREMETAL__
	char Str[XDFEMIX_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;
#endif

	Xil_AssertNonvoid(DeviceNodeName != NULL);
	Xil_AssertNonvoid(strlen(DeviceNodeName) <
			  XDFEMIX_NODE_NAME_MAX_LENGTH);

	/* Is this first mixer initialisation ever? */
	if (0U == XDfeMix_DriverHasBeenRegisteredOnce) {
		/* Set up environment to non-initialized */
		for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
			XDfeMix_Mixer[Index].StateId = XDFEMIX_STATE_NOT_READY;
			XDfeMix_Mixer[Index].NodeName[0] = '\0';
		}
		XDfeMix_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check has DeviceNodeName been already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		if (0U == strncmp(XDfeMix_Mixer[Index].NodeName, DeviceNodeName,
				  strlen(DeviceNodeName))) {
			XDfeMix_Mixer[Index].StateId = XDFEMIX_STATE_READY;
			return &XDfeMix_Mixer[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		if (XDfeMix_Mixer[Index].NodeName[0] == '\0') {
			strncpy(XDfeMix_Mixer[Index].NodeName, DeviceNodeName,
				XDFEMIX_NODE_NAME_MAX_LENGTH);
			InstancePtr = &XDfeMix_Mixer[Index];
			goto register_metal;
		}
	}

	/* Failing as there is no available slot. */
	return NULL;

register_metal:
#ifdef __BAREMETAL__
	memcpy(Str, InstancePtr->NodeName, XDFEMIX_NODE_NAME_MAX_LENGTH);
	AddrStr = strtok(Str, ".");
	Addr = strtoul(AddrStr, NULL, 16);
	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		if (Addr == XDfeMix_metal_phys[Index]) {
			InstancePtr->Device = &XDfeMix_CustomDevice[Index];
			goto bm_register_metal;
		}
	}
	return NULL;
bm_register_metal:
#endif

	/* Register libmetal for this OS process */
	if (XST_SUCCESS != XDfeMix_RegisterMetal(InstancePtr,
						 &InstancePtr->Device,
						 DeviceNodeName)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeMix_LookupConfig(InstancePtr)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to configure device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Configure HW and the driver instance */
	XDfeMix_CfgInitialize(InstancePtr);

	InstancePtr->StateId = XDFEMIX_STATE_READY;

	return InstancePtr;

return_error:
	InstancePtr->StateId = XDFEMIX_STATE_NOT_READY;
	InstancePtr->NodeName[0] = '\0';
	return NULL;
}

/*****************************************************************************/
/**
*
* API closes the instance of a Mixer driver and moves the state machine to
* a Not Ready state.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
******************************************************************************/
void XDfeMix_InstanceClose(XDfeMix *InstancePtr)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		/* Find the instance in XDfeMix_Mixer array */
		if (&XDfeMix_Mixer[Index] == InstancePtr) {
			/* Release libmetal */
			metal_device_close(InstancePtr->Device);
			InstancePtr->StateId = XDFEMIX_STATE_NOT_READY;
			InstancePtr->NodeName[0] = '\0';
			return;
		}
	}

	/* Assert as you should never get to this point. */
	Xil_AssertVoidAlways();
	return;
}

/****************************************************************************/
/**
*
* Resets Mixer and puts block into a reset state.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
****************************************************************************/
void XDfeMix_Reset(XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEMIX_STATE_NOT_READY);

	/* Put Mixer in reset */
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_RESET_OFFSET, XDFEMIX_RESET_ON);
	InstancePtr->StateId = XDFEMIX_STATE_RESET;
}

/****************************************************************************/
/**
*
* Reads configuration from device tree/xparameters.h and IP registers.
* Removes S/W reset and moves the state machine to a Configured state.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Cfg Configuration data container.
*
****************************************************************************/
void XDfeMix_Configure(XDfeMix *InstancePtr, XDfeMix_Cfg *Cfg)
{
	u32 Version;
	u32 ModelParam;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_RESET);
	Xil_AssertVoid(Cfg != NULL);

	/* Read vearsion */
	Version = XDfeMix_ReadReg(InstancePtr, XDFEMIX_VERSION_OFFSET);
	Cfg->Version.Patch =
		XDfeMix_RdBitField(XDFEMIX_VERSION_PATCH_WIDTH,
				   XDFEMIX_VERSION_PATCH_OFFSET, Version);
	Cfg->Version.Revision =
		XDfeMix_RdBitField(XDFEMIX_VERSION_REVISION_WIDTH,
				   XDFEMIX_VERSION_REVISION_OFFSET, Version);
	Cfg->Version.Minor =
		XDfeMix_RdBitField(XDFEMIX_VERSION_MINOR_WIDTH,
				   XDFEMIX_VERSION_MINOR_OFFSET, Version);
	Cfg->Version.Major =
		XDfeMix_RdBitField(XDFEMIX_VERSION_MAJOR_WIDTH,
				   XDFEMIX_VERSION_MAJOR_OFFSET, Version);

	/* Read model parameters */
	ModelParam = XDfeMix_ReadReg(InstancePtr, XDFEMIX_MODEL_PARAM_1_OFFSET);
	InstancePtr->Config.Mode =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_MODE_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_MODE_OFFSET,
				   ModelParam);
	InstancePtr->Config.NumAntenna =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_NUM_ANTENNA_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_NUM_ANTENNA_OFFSET,
				   ModelParam);
	InstancePtr->Config.MaxUseableCcids = XDfeMix_RdBitField(
		XDFEMIX_MODEL_PARAM_1_MAX_USEABLE_CCIDS_WIDTH,
		XDFEMIX_MODEL_PARAM_1_MAX_USEABLE_CCIDS_OFFSET, ModelParam);
	InstancePtr->Config.Lanes =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_LANES_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_LANES_OFFSET,
				   ModelParam);
	InstancePtr->Config.AntennaInterleave = XDfeMix_RdBitField(
		XDFEMIX_MODEL_PARAM_1_ANTENNA_INTERLEAVE_WIDTH,
		XDFEMIX_MODEL_PARAM_1_ANTENNA_INTERLEAVE_OFFSET, ModelParam);
	InstancePtr->Config.MixerCps =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_MIXER_CPS_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_MIXER_CPS_OFFSET,
				   ModelParam);

	ModelParam = XDfeMix_ReadReg(InstancePtr, XDFEMIX_MODEL_PARAM_2_OFFSET);
	InstancePtr->Config.DataIWidth =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_2_DATA_IWIDTH_WIDTH,
				   XDFEMIX_MODEL_PARAM_2_DATA_IWIDTH_OFFSET,
				   ModelParam);
	InstancePtr->Config.DataOWidth =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_2_DATA_OWIDTH_WIDTH,
				   XDFEMIX_MODEL_PARAM_2_DATA_OWIDTH_OFFSET,
				   ModelParam);
	InstancePtr->Config.TUserWidth =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_2_TUSER_WIDTH_WIDTH,
				   XDFEMIX_MODEL_PARAM_2_TUSER_WIDTH_OFFSET,
				   ModelParam);

	/* Copy configs model parameters from devicetree config data stored in
	   InstancePtr */
	Cfg->ModelParams.Mode = InstancePtr->Config.Mode;
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.MaxUseableCcids = InstancePtr->Config.MaxUseableCcids;
	Cfg->ModelParams.Lanes = InstancePtr->Config.Lanes;
	Cfg->ModelParams.AntennaInterleave =
		InstancePtr->Config.AntennaInterleave;
	Cfg->ModelParams.MixerCps = InstancePtr->Config.MixerCps;
	Cfg->ModelParams.DataIWidth = InstancePtr->Config.DataIWidth;
	Cfg->ModelParams.DataOWidth = InstancePtr->Config.DataOWidth;
	Cfg->ModelParams.TUserWidth = InstancePtr->Config.TUserWidth;

	/* Release RESET */
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_RESET_OFFSET, XDFEMIX_RESET_OFF);
	InstancePtr->StateId = XDFEMIX_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE Mixer driver one time initialisation and moves the state machine to
* a Initialised state.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Init Initialisation data container.
*
****************************************************************************/
void XDfeMix_Initialize(XDfeMix *InstancePtr, XDfeMix_Init *Init)
{
	XDfeMix_Trigger CCUpdate;
	u32 Data;
	u32 Index;
	u32 Offset;
	u32 SequenceLength;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_CONFIGURED);
	Xil_AssertVoid(Init != NULL);

	/* Enable FIR and MIXER registers */
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_STATE_FIR_ENABLE_OFFSET,
			 XDFEMIX_STATE_FIR_ENABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_STATE_MIXER_ENABLE_OFFSET,
			 XDFEMIX_STATE_MIXER_ENABLED);

	XDfeMix_SetPLMixerDelay(InstancePtr);

	/* Write "one-time" Sequence length */
	InstancePtr->NotUsedCCID = 0;
	InstancePtr->SequenceLength = Init->Sequence.Length;
	if (Init->Sequence.Length == 0) {
		SequenceLength = 0U;
	} else {
		SequenceLength = Init->Sequence.Length - 1U;
	}
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_SEQUENCE_LENGTH_NEXT,
			 SequenceLength);

	/* Set default sequence and ensure all CCs are disabled. Not all
	   registers will be cleared by reset as they are implemented using
	   DRAM. This step sets all CC_CONFIGURATION.CARRIER_CONFIGURATION.
	   CURRENT[*]. ENABLE to 0 ensuring the Hardblock will remain disabled
	   following the first call to XDFEMIXilterActivate. */
	for (Index = 0; Index < XDFEMIX_SEQ_LENGTH_MAX; Index++) {
		Offset = XDFEMIX_SEQUENCE_NEXT + (sizeof(u32) * Index);
		XDfeMix_WriteReg(InstancePtr, Offset,
				 XDFEMIX_SEQUENCE_ENTRY_DEFAULT);
		Init->Sequence.CCID[Index] = XDFEMIX_SEQUENCE_ENTRY_NULL;
	}
	for (Index = 0; Index < XDFEMIX_CC_NUM; Index++) {
		Offset = XDFEMIX_CC_CONFIG_NEXT + (sizeof(u32) * Index);
		XDfeMix_WriteReg(InstancePtr, Offset, 0U);
	}

	/* Trigger CC_UPDATE immediately using Register source to update
	   CURRENT from NEXT */
	CCUpdate.TriggerEnable = XDFEMIX_TRIGGERS_TRIGGER_ENABLE_ENABLED;
	CCUpdate.Mode = XDFEMIX_TRIGGERS_MODE_IMMEDIATE;
	CCUpdate.StateOutput = XDFEMIX_TRIGGERS_STATE_OUTPUT_ENABLED;
	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  CCUpdate.StateOutput);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  CCUpdate.TriggerEnable);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_MODE_WIDTH,
				  XDFEMIX_TRIGGERS_MODE_OFFSET, Data,
				  CCUpdate.Mode);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Data);

	InstancePtr->StateId = XDFEMIX_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activates Mixer and moves the state machine to an Activated state.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    EnableLowPower Flag indicating low power.
*
******************************************************************************/
void XDfeMix_Activate(XDfeMix *InstancePtr, bool EnableLowPower)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEMIX_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL));

	/* Do nothing if the block already operational */
	IsOperational =
		XDfeMix_RdRegBitField(InstancePtr,
				      XDFEMIX_STATE_OPERATIONAL_OFFSET,
				      XDFEMIX_STATE_OPERATIONAL_FIELD_WIDTH,
				      XDFEMIX_STATE_OPERATIONAL_FIELD_OFFSET);
	if (IsOperational == XDFEMIX_STATE_OPERATIONAL_YES) {
		return;
	}

	/* Enable the Activate trigger and set to one-shot */
	XDfeMix_EnableActivateTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfeMix_EnableLowPowerTrigger(InstancePtr);
	}

	/* Mixer is operational now, change a state */
	InstancePtr->StateId = XDFEMIX_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* Deactivates Mixer and moves the state machine to Initialised state.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
******************************************************************************/
void XDfeMix_Deactivate(XDfeMix *InstancePtr)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEMIX_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL));

	/* Do nothing if the block already deactivated */
	IsOperational =
		XDfeMix_RdRegBitField(InstancePtr,
				      XDFEMIX_STATE_OPERATIONAL_OFFSET,
				      XDFEMIX_STATE_OPERATIONAL_FIELD_WIDTH,
				      XDFEMIX_STATE_OPERATIONAL_FIELD_OFFSET);
	if (IsOperational == XDFEMIX_STATE_OPERATIONAL_NO) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfeMix_DisableLowPowerTrigger(InstancePtr);

	/* Enable Deactivate trigger */
	XDfeMix_EnableDeactivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFEMIX_STATE_INITIALISED;
}

/****************************************************************************/
/**
*
* Gets a state machine state id.
*
* @param    InstancePtr Pointer to the Mixer instance.
*
* @return   State machine StateID
*
****************************************************************************/
XDfeMix_StateId XDfeMix_GetStateID(XDfeMix *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return InstancePtr->StateId;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Returns the current CC configuration. Not used slot ID in a sequence
* (Sequence.CCID[Index]) are represented as (-1), not the value in registers.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CurrCCCfg CC configuration container.
*
****************************************************************************/
void XDfeMix_GetCurrentCCCfg(const XDfeMix *InstancePtr,
			     XDfeMix_CCCfg *CurrCCCfg)
{
	u32 SeqLen;
	u32 AntennaCfg = 0U;
	u32 Data;
	u32 Offset;
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CurrCCCfg != NULL);

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEMIX_CC_NUM; Index++) {
		CurrCCCfg->Sequence.CCID[Index] = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_SEQUENCE_CURRENT + (sizeof(u32) * Index));
	}

	/* Read sequence length */
	SeqLen = XDfeMix_ReadReg(InstancePtr, XDFEMIX_SEQUENCE_LENGTH_CURRENT);
	if (SeqLen == 0U) {
		CurrCCCfg->Sequence.Length = InstancePtr->SequenceLength;
	} else {
		CurrCCCfg->Sequence.Length = SeqLen + 1U;
	}

	/* Convert not used CC to -1 */
	for (Index = 0; Index < XDFEMIX_CC_NUM; Index++) {
		if ((CurrCCCfg->Sequence.CCID[Index] ==
		     InstancePtr->NotUsedCCID) ||
		    (Index >= InstancePtr->SequenceLength)) {
			CurrCCCfg->Sequence.CCID[Index] =
				XDFEMIX_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEMIX_CC_NUM; Index++) {
		Offset = XDFEMIX_CC_CONFIG_CURRENT + (Index * sizeof(u32));
		Data = XDfeMix_ReadReg(InstancePtr, Offset);
		CurrCCCfg->DUCDDCCfg[Index].NCOIdx =
			XDfeMix_RdBitField(XDFEMIX_CC_CONFIG_NCO_WIDTH,
					   XDFEMIX_CC_CONFIG_NCO_OFFSET, Data);
		CurrCCCfg->DUCDDCCfg[Index].Rate =
			XDfeMix_RdBitField(XDFEMIX_CC_CONFIG_RATE_WIDTH,
					   XDFEMIX_CC_CONFIG_RATE_OFFSET, Data);
		CurrCCCfg->DUCDDCCfg[Index].CCGain =
			XDfeMix_RdBitField(XDFEMIX_CC_CONFIG_CC_GAIN_WIDTH,
					   XDFEMIX_CC_CONFIG_CC_GAIN_OFFSET,
					   Data);
	}

	/* Read NCO configurations */
	for (Index = 0; Index <= XDFEMIX_NCO_MAX; Index++) {
		/* Get frequency configuration */
		Offset = XDFEMIX_FREQ_CONTROL_WORD +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].FrequencyCfg.FrequencyControlWord =
			XDfeMix_ReadReg(InstancePtr, Offset);
		Offset = XDFEMIX_FREQ_SINGLE_MOD_COUNT +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].FrequencyCfg.SingleModCount =
			XDfeMix_ReadReg(InstancePtr, Offset);
		Offset = XDFEMIX_FREQ_DUAL_MOD_COUNT +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].FrequencyCfg.DualModCount =
			XDfeMix_ReadReg(InstancePtr, Offset);
		Offset = XDFEMIX_FREQ_PHASE_OFFSET +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].FrequencyCfg.PhaseOffset.PhaseOffset =
			XDfeMix_ReadReg(InstancePtr, Offset);
		/* Get phase configuration */
		Offset = XDFEMIX_PHASE_UPDATE_ACC +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].PhaseCfg.PhaseAcc =
			XDfeMix_ReadReg(InstancePtr, Offset);
		Offset = XDFEMIX_PHASE_UPDATE_DUAL_MOD_COUNT +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].PhaseCfg.DualModCount =
			XDfeMix_ReadReg(InstancePtr, Offset);
		Offset = XDFEMIX_PHASE_UPDATE_DUAL_MOD_SEL +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].PhaseCfg.DualModSel =
			XDfeMix_ReadReg(InstancePtr, Offset);
		/* Get NCO gain */
		Offset = XDFEMIX_NCO_GAIN +
			 (Index * XDFEMIX_PHAC_CCID_ADDR_STEP);
		CurrCCCfg->NCO[Index].NCOGain =
			XDfeMix_ReadReg(InstancePtr, Offset);
	}

	/* Read Antenna configuration */
	AntennaCfg = XDfeMix_ReadReg(InstancePtr, XDFEMIX_ANTENNA_GAIN_CURRENT);
	for (Index = 0; Index < XDFEMIX_ANT_NUM_MAX; Index++) {
		CurrCCCfg->AntennaCfg.Gain[Index] =
			(AntennaCfg >> Index) & XDFEMIX_ONE_ANTENNA_GAIN_ZERODB;
	}
}

/****************************************************************************/
/**
*
* Returns configuration structure CCCfg with CCCfg->Sequence.Length value set
* in XDfeMix_Configure(), array CCCfg->Sequence.CCID[] members are set to not
* used value (-1) and the other CCCfg members are set to 0.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg CC configuration container.
*
****************************************************************************/
void XDfeMix_GetEmptyCCCfg(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg)
{
	u32 Index;
	u32 SeqLen;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	memset(CCCfg, 0, sizeof(XDfeMix_CCCfg));

	/* Convert CC to -1 meaning not used */
	for (Index = 0U; Index < XDFEMIX_CC_NUM; Index++) {
		CCCfg->Sequence.CCID[Index] = XDFEMIX_SEQUENCE_ENTRY_NULL;
	}
	/* Read sequence length */
	SeqLen = XDfeMix_ReadReg(InstancePtr, XDFEMIX_SEQUENCE_LENGTH_CURRENT);
	if (SeqLen == 0U) {
		CCCfg->Sequence.Length = InstancePtr->SequenceLength;
	} else {
		CCCfg->Sequence.Length = SeqLen + 1U;
	}
}

/****************************************************************************/
/**
*
* Returns the current CC sequence bitmap, CCID carrier configuration and
* NCO configuration.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID.
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
* @param    NCO NCO configuration container.
*
****************************************************************************/
void XDfeMix_GetCarrierCfgAndNCO(const XDfeMix *InstancePtr,
				 XDfeMix_CCCfg *CCCfg, s32 CCID,
				 u32 *CCSeqBitmap,
				 XDfeMix_CarrierCfg *CarrierCfg,
				 XDfeMix_NCO *NCO)
{
	u32 Index;
	u32 Mask = 1U;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertVoid(CCSeqBitmap != NULL);
	Xil_AssertVoid(CarrierCfg != NULL);
	Xil_AssertVoid(NCO != NULL);

	CarrierCfg->DUCDDCCfg.NCOIdx = CCCfg->DUCDDCCfg[CCID].NCOIdx;
	CarrierCfg->DUCDDCCfg.CCGain = CCCfg->DUCDDCCfg[CCID].CCGain;
	*NCO = CCCfg->NCO[CCCfg->DUCDDCCfg[CCID].NCOIdx];

	*CCSeqBitmap = 0U;
	for (Index = 0U; Index < CCCfg->Sequence.Length; Index++) {
		if (CCCfg->Sequence.CCID[Index] == CCID) {
			*CCSeqBitmap |= Mask;
		}
		Mask <<= 1U;
	}
}

/****************************************************************************/
/**
*
* Set antenna configuration in CC configuration container.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg CC configuration container.
* @param    AntennaCfg Array of all antenna configurations.
*
****************************************************************************/
void XDfeMix_SetAntennaCfgInCCCfg(const XDfeMix *InstancePtr,
				  XDfeMix_CCCfg *CCCfg,
				  XDfeMix_AntennaCfg *AntennaCfg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(AntennaCfg != NULL);

	CCCfg->AntennaCfg = *AntennaCfg;
}

/****************************************************************************/
/**
*
* Adds specified CCID, with specified configuration, to a local CC
* configuration structure.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* The returned CCCfg.Sequence is transleted as there is no explicit indication
* that SEQUENCE[i] is not used - 0 can define the slot as either used or
* not used. Sequence data that is returned in the CCIDSequence is not the same
* as what is written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID.
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
* @param    NCO NCO configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_AddCCtoCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg, s32 CCID,
			 u32 CCSeqBitmap, const XDfeMix_CarrierCfg *CarrierCfg,
			 const XDfeMix_NCO *NCO)
{
	u32 AddSuccess;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(NCO != NULL);

	/* Check is Mixer in ARCH4 mode */
	if (XDFEMIX_IS_ARCH4_MODE) {
		if (XST_FAILURE ==
		    XDfeMix_NCOArch4ModeInAddCC(InstancePtr, CCCfg, CCID,
						CCSeqBitmap,
						CarrierCfg->DUCDDCCfg.NCOIdx)) {
			metal_log(METAL_LOG_ERROR,
				  "NCO failure in ARCH4 mode in %s\n",
				  __func__);
			return XST_FAILURE;
		}
	}

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeMix_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg->Sequence);
	if (AddSuccess == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Update carrier configuration NEXT registers */
	if (XST_FAILURE == XDfeMix_SetCCDDC(InstancePtr, CCCfg, CCID,
					    CCSeqBitmap,
					    &CarrierCfg->DUCDDCCfg)) {
		metal_log(METAL_LOG_ERROR, "AddCC failed on SetCCDDC\n");
		return XST_FAILURE;
	}

	/* Update NCO registers for CCID */
	CCCfg->NCO[CCCfg->DUCDDCCfg[CCID].NCOIdx] = *NCO;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes specified CCID from a local CC configuration structure.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID.
*
* @note     For a sequence conversion see XDfeMix_AddCCtoCCCfg() comment.
*
****************************************************************************/
void XDfeMix_RemoveCCfromCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			       s32 CCID)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertVoid(CCCfg != NULL);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeMix_RemoveCCID(InstancePtr, CCID, &CCCfg->Sequence);
	CCCfg->DUCDDCCfg[CCID].Rate = 0U;
}

/****************************************************************************/
/**
*
* Updates specified CCID, with specified configuration to a local CC
* configuration structure.
* If there is insufficient capacity for the new CC the function will return
* an error.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_UpdateCCinCCCfg(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			    s32 CCID, const XDfeMix_CarrierCfg *CarrierCfg)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Check is Mixer in ARCH4 mode */
	if (XDFEMIX_IS_ARCH4_MODE) {
		if (XST_FAILURE == XDfeMix_NCOArch4ModeInMoveOrUpdateCC(
					   InstancePtr, CCCfg, CCID,
					   CarrierCfg->DUCDDCCfg.NCOIdx)) {
			metal_log(METAL_LOG_ERROR,
				  "NCO failure in ARCH4 mode in %s\n",
				  __func__);
			return XST_FAILURE;
		}
	}

	/* Update carrier configuration NEXT registers */
	if (XST_FAILURE == XDfeMix_UpdateCCDDC(InstancePtr, CCCfg, CCID,
					       &CarrierCfg->DUCDDCCfg)) {
		metal_log(METAL_LOG_ERROR, "AddCC failed on SetCCDDC\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Writes local CC configuration to the shadow (NEXT) registers and triggers
* copying from shadow to operational registers.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_SetNextCCCfgAndTrigger(const XDfeMix *InstancePtr,
				   const XDfeMix_CCCfg *CCCfg)
{
	u32 Index;
	u32 NCOIdx;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);

	/* Update carrier configuration NEXT registers */
	XDfeMix_SetNextCCCfg(InstancePtr, CCCfg);

	/* Update all NCO registers */
	for (Index = 0U; Index < XDFEMIX_CC_NUM; Index++) {
		if (CCCfg->DUCDDCCfg[Index].Rate ==
		    XDFEMIX_CC_CONFIG_RATE_DISABLED) {
			continue;
		}
		NCOIdx = CCCfg->DUCDDCCfg[Index].NCOIdx;
		if (NCOIdx >= InstancePtr->Config.MaxUseableCcids) {
			metal_log(METAL_LOG_ERROR,
				  "NCOIdx %d is greater than %d\n", NCOIdx,
				  InstancePtr->Config.MaxUseableCcids);
			continue;
		}
		XDfeMix_SetCCFrequency(InstancePtr, XDFEMIXER_NEXT, Index,
				       &CCCfg->NCO[NCOIdx].FrequencyCfg);
		XDfeMix_SetCCPhase(InstancePtr, XDFEMIXER_NEXT, Index,
				   &CCCfg->NCO[NCOIdx].PhaseCfg);
		XDfeMix_SetCCNCOGain(InstancePtr, XDFEMIXER_NEXT, Index,
				     CCCfg->NCO[NCOIdx].NCOGain);
	}

	/* Trigger the update */
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Adds specified CCID, with specified configuration.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID Channel ID.
* @param    CCSeqBitmap - up to 16 defined slots into which a CC can be
*           allocated. The number of slots can be from 1 to 16 depending on
*           system initialization. The number of slots is defined by the
*           "sequence length" parameter which is provided during initialization.
*           The Bit offset within the CCSeqBitmap indicates the equivalent
*           Slot number to allocate. e.g. 0x0003  means the caller wants the
*           passed component carrier (CC) to be allocated to slots 0 and 1.
* @param    CarrierCfg CC configuration container.
* @param    NCO NCO configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeMix_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeMix_AddCC(XDfeMix *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		  const XDfeMix_CarrierCfg *CarrierCfg, const XDfeMix_NCO *NCO)
{
	XDfeMix_CCCfg CCCfg;
	u32 AddSuccess;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(CarrierCfg->DUCDDCCfg.NCOIdx <= XDFEMIX_NCO_MAX);
	Xil_AssertNonvoid(CarrierCfg->DUCDDCCfg.CCGain <= XDFEMIX_CC_GAIN_MAX);
	Xil_AssertNonvoid(NCO != NULL);

	if (CarrierCfg->DUCDDCCfg.NCOIdx >=
	    InstancePtr->Config.MaxUseableCcids) {
		metal_log(METAL_LOG_ERROR, "NCOIdx %d is greater than %d\n",
			  CarrierCfg->DUCDDCCfg.NCOIdx,
			  InstancePtr->Config.MaxUseableCcids);
		return XST_FAILURE;
	}

	/* Read current CC configuration. Note that XDfeMix_Initialise writes
	   a NULL CC sequence to H/W */
	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Check is Mixer in ARCH4 mode */
	if (XDFEMIX_IS_ARCH4_MODE) {
		if (XST_FAILURE ==
		    XDfeMix_NCOArch4ModeInAddCC(InstancePtr, &CCCfg, CCID,
						CCSeqBitmap,
						CarrierCfg->DUCDDCCfg.NCOIdx)) {
			metal_log(METAL_LOG_ERROR,
				  "NCO failure in ARCH4 mode in %s\n",
				  __func__);
			return XST_FAILURE;
		}
	}

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeMix_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg.Sequence);
	if (AddSuccess == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Set DUCDDCCfg in CCCfg */
	XDfeMix_SetCCDDC(InstancePtr, &CCCfg, CCID, CCSeqBitmap,
			 &CarrierCfg->DUCDDCCfg);

	/* Update registers and trigger update */
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	XDfeMix_SetCCFrequency(InstancePtr, XDFEMIXER_NEXT, CCID,
			       &NCO->FrequencyCfg);
	XDfeMix_SetCCPhase(InstancePtr, XDFEMIXER_NEXT, CCID, &NCO->PhaseCfg);
	XDfeMix_SetCCNCOGain(InstancePtr, XDFEMIXER_NEXT, CCID, NCO->NCOGain);
	/*
	 *  PHACCs configured, but not running.
	 *  NCOs not running.
	 *  Antenna contribution disabled.
	 */
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Removes specified CCID.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID Channel ID.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeMix_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeMix_RemoveCC(XDfeMix *InstancePtr, s32 CCID)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);

	/* Read current CC configuration */
	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeMix_RemoveCCID(InstancePtr, CCID, &CCCfg.Sequence);

	CCCfg.DUCDDCCfg[CCID].Rate = 0U;

	/* Update next configuration and trigger update */
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Moves specified CCID from one NCO to another aligning phase to make it
* transparent.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID Channel ID.
* @param    Rate NCO rate value [1,2,4].
* @param    FromNCO NCO value moving from.
* @param    ToNCO NCO value moving to.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeMix_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeMix_MoveCC(XDfeMix *InstancePtr, s32 CCID, u32 Rate, u32 FromNCO,
		   u32 ToNCO)
{
	XDfeMix_CCCfg CCCfg;
	XDfeMix_Frequency Freq;
	XDfeMix_Phase PhaseNext;
	XDfeMix_Phase PhaseCurrent;
	XDfeMix_PhaseOffset PhaseOffset;
	XDfeMix_PhaseOffset PhaseDiff = { 0 };
	u32 NCOGain;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertNonvoid(FromNCO <= XDFEMIX_NCO_MAX);
	Xil_AssertNonvoid(ToNCO <= XDFEMIX_NCO_MAX);

	if (FromNCO >= InstancePtr->Config.MaxUseableCcids) {
		metal_log(METAL_LOG_ERROR, "FromNCO %d is greater than %d\n",
			  FromNCO, InstancePtr->Config.MaxUseableCcids);
		return XST_FAILURE;
	}
	if (ToNCO >= InstancePtr->Config.MaxUseableCcids) {
		metal_log(METAL_LOG_ERROR, "ToNCO %d is greater than %d\n",
			  ToNCO, InstancePtr->Config.MaxUseableCcids);
		return XST_FAILURE;
	}
	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);
	/* Check is Mixer in ARCH4 mode */
	if (XDFEMIX_IS_ARCH4_MODE) {
		if (XST_FAILURE == XDfeMix_NCOArch4ModeInMoveOrUpdateCC(
					   InstancePtr, &CCCfg, CCID, ToNCO)) {
			metal_log(METAL_LOG_ERROR,
				  "NCO failure in ARCH4 mode in %s\n",
				  __func__);
			return XST_FAILURE;
		}
	}
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	/* Copy NCO */
	NCOGain = XDfeMix_GetCCNCOGain(InstancePtr, XDFEMIXER_CURRENT, CCID);
	XDfeMix_SetCCNCOGain(InstancePtr, XDFEMIXER_NEXT, CCID, NCOGain);
	XDfeMix_GetCCFrequency(InstancePtr, XDFEMIXER_CURRENT, CCID, &Freq);
	XDfeMix_SetCCFrequency(InstancePtr, XDFEMIXER_NEXT, CCID, &Freq);
	XDfeMix_SetCCPhaseAccumEnable(InstancePtr, XDFEMIXER_NEXT, CCID,
				      XDFEMIXER_PHACC_ENABLE);
	/* Align phase */
	XDfeMix_CapturePhase(InstancePtr);
	XDfeMix_GetCCPhase(InstancePtr, XDFEMIXER_CURRENT, CCID, &PhaseCurrent);
	XDfeMix_GetCCPhase(InstancePtr, XDFEMIXER_NEXT, CCID, &PhaseNext);
	XDfeMix_DerivePhaseOffset(&PhaseCurrent, &PhaseNext, &PhaseOffset);
	XDfeMix_SetPhaseOffset(&Freq, &PhaseDiff);
	XDfeMix_SetCCFrequency(InstancePtr, XDFEMIXER_NEXT, CCID, &Freq);

	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Updates specified CCID, with a configuration defined in CarrierCfg
* structure.
* If there is insufficient capacity for the new CC the function will return
* an error.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID Channel ID.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeMix_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeMix_UpdateCC(const XDfeMix *InstancePtr, s32 CCID,
		     const XDfeMix_CarrierCfg *CarrierCfg)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	/* Read current CC configuration. Note that XDfeMix_Initialise writes
	   a NULL CC sequence to H/W */
	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Check is Mixer in ARCH4 mode */
	if (XDFEMIX_IS_ARCH4_MODE) {
		if (XST_FAILURE == XDfeMix_NCOArch4ModeInMoveOrUpdateCC(
					   InstancePtr, &CCCfg, CCID,
					   CarrierCfg->DUCDDCCfg.NCOIdx)) {
			metal_log(METAL_LOG_ERROR,
				  "NCO failure in ARCH4 mode in %s\n",
				  __func__);
			return XST_FAILURE;
		}
	}

	/* Update carrier configuration */
	CCCfg.DUCDDCCfg[CCID].NCOIdx = CarrierCfg->DUCDDCCfg.NCOIdx;
	CCCfg.DUCDDCCfg[CCID].CCGain = CarrierCfg->DUCDDCCfg.CCGain;

	/* Update registers and trigger update */
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	/*
	 *  PHACCs configured, but not running.
	 *  NCOs not running.
	 *  Antenna contribution disabled.
	 */
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Sets antenna gain. Initiate CC update (enable CCUpdate trigger TUSER
* Single Shot).
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    AntennaId Antenna ID.
* @param    AntennaGain Antenna gain, 0 for -6dB and 1 for 0dB.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeMix_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeMix_SetAntennaGain(XDfeMix *InstancePtr, u32 AntennaId, u32 AntennaGain)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AntennaGain <= 1U);
	Xil_AssertNonvoid(AntennaId <= XDFEMIX_ANT_NUM_MAX);

	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);
	CCCfg.AntennaCfg.Gain[AntennaId] = AntennaGain;
	/* Update next configuration and trigger update */
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Updates antenna cofiguration to all antennas.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    AntennaCfg Array of all antenna configurations.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeMix_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeMix_UpdateAntennaCfg(XDfeMix *InstancePtr,
			     XDfeMix_AntennaCfg *AntennaCfg)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AntennaCfg != NULL);

	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);
	CCCfg.AntennaCfg = *AntennaCfg;
	/* Update next configuration and trigger update */
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Returns current trigger configuration.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfeMix_GetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEMIX_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read ACTIVATE triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	TriggerCfg->Activate.TriggerEnable =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->Activate.Mode = XDfeMix_RdBitField(
		XDFEMIX_TRIGGERS_MODE_WIDTH, XDFEMIX_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.TuserEdgeLevel =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->Activate.StateOutput =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read LOW_POWER triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	TriggerCfg->LowPower.TriggerEnable =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->LowPower.Mode = XDfeMix_RdBitField(
		XDFEMIX_TRIGGERS_MODE_WIDTH, XDFEMIX_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.TuserEdgeLevel =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->LowPower.StateOutput =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read CC_UPDATE triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	TriggerCfg->CCUpdate.TriggerEnable =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->CCUpdate.Mode = XDfeMix_RdBitField(
		XDFEMIX_TRIGGERS_MODE_WIDTH, XDFEMIX_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->CCUpdate.TUSERBit =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->CCUpdate.TuserEdgeLevel =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->CCUpdate.StateOutput =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets trigger configuration.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfeMix_SetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);
	Xil_AssertVoid(TriggerCfg->CCUpdate.Mode !=
		       XDFEMIX_TRIGGERS_MODE_TUSER_CONTINUOUS);

	/* Write public trigger configuration members and ensure private members
	  (TriggerEnable & Immediate) are set appropriately */

	/* Activate defined as Single Shot/Immediate (as per the programming model) */
	TriggerCfg->Activate.TriggerEnable =
		XDFEMIX_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->Activate.StateOutput =
		XDFEMIX_TRIGGERS_STATE_OUTPUT_ENABLED;
	/* Read/set/write ACTIVATE triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->Activate.TriggerEnable);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_MODE_WIDTH,
				 XDFEMIX_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->Activate.Mode);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->Activate.TuserEdgeLevel);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->Activate.TUSERBit);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->Activate.StateOutput);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.TriggerEnable =
		XDFEMIX_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->LowPower.Mode = XDFEMIX_TRIGGERS_MODE_TUSER_CONTINUOUS;
	/* Read/set/write LOW_POWER triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->LowPower.TriggerEnable);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_MODE_WIDTH,
				 XDFEMIX_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->LowPower.Mode);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->LowPower.TuserEdgeLevel);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->LowPower.TUSERBit);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->LowPower.StateOutput);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Val);

	/* CCUpdate defined as Single Shot/Immediate */
	TriggerCfg->CCUpdate.TriggerEnable =
		XDFEMIX_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->CCUpdate.StateOutput =
		XDFEMIX_TRIGGERS_STATE_OUTPUT_ENABLED;
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->CCUpdate.TriggerEnable);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_MODE_WIDTH,
				 XDFEMIX_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->CCUpdate.Mode);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFEMIX_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->CCUpdate.TuserEdgeLevel);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->CCUpdate.TUSERBit);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFEMIX_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->CCUpdate.StateOutput);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Gets DUC/DDC status for a specified CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID Channel ID.
* @param    DUCDDCStatus DUC/DDC status container.
*
****************************************************************************/
void XDfeMix_GetDUCDDCStatus(const XDfeMix *InstancePtr, s32 CCID,
			     XDfeMix_DUCDDCStatus *DUCDDCStatus)
{
	(void)CCID;
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DUCDDCStatus != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_MIXER_STATUS_OVERFLOW);
	DUCDDCStatus->RealOverflowStage =
		XDfeMix_RdBitField(XDFEMIX_DUC_DDC_STATUS_OVERFLOW_STAGE_WIDTH,
				   XDFEMIX_DUC_DDC_STATUS_OVERFLOW_STAGE_OFFSET,
				   Val);
	DUCDDCStatus->FirstAntennaOverflowing = XDfeMix_RdBitField(
		XDFEMIX_DUC_DDC_STATUS_OVERFLOW_ANTENNA_WIDTH,
		XDFEMIX_DUC_DDC_STATUS_OVERFLOW_ANTENNA_OFFSET, Val);
	DUCDDCStatus->FirstCCIDOverflowing = XDfeMix_RdBitField(
		XDFEMIX_DUC_DDC_STATUS_OVERFLOW_ASSOCIATED_NCO_WIDTH,
		XDFEMIX_DUC_DDC_STATUS_OVERFLOW_ASSOCIATED_NCO_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Gets Mixer status  for a specified CCID.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCID Channel ID.
* @param    MixerStatus Mixer status container.
*
****************************************************************************/
void XDfeMix_GetMixerStatus(const XDfeMix *InstancePtr, s32 CCID,
			    XDfeMix_MixerStatus *MixerStatus)
{
	(void)CCID;
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MixerStatus != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_MIXER_STATUS_OVERFLOW);
	MixerStatus->AdderStage =
		XDfeMix_RdBitField(XDFEMIX_MIXER_STATUS_OVERFLOW_STAGE_WIDTH,
				   XDFEMIX_MIXER_STATUS_OVERFLOW_STAGE_OFFSET,
				   Val);
	MixerStatus->AdderAntenna =
		XDfeMix_RdBitField(XDFEMIX_MIXER_STATUS_OVERFLOW_ANTENNA_WIDTH,
				   XDFEMIX_MIXER_STATUS_OVERFLOW_ANTENNA_OFFSET,
				   Val);
	MixerStatus->MixAntenna =
		XDfeMix_RdBitField(XDFEMIX_MIXER_STATUS_OVERFLOW_NCO_WIDTH,
				   XDFEMIX_MIXER_STATUS_OVERFLOW_NCO_OFFSET,
				   Val);
}

/****************************************************************************/
/**
*
* Sets the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Delay Requested delay variable.
*
****************************************************************************/
void XDfeMix_SetTUserDelay(const XDfeMix *InstancePtr, u32 Delay)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);
	Xil_AssertVoid(Delay < (1U << XDFEMIX_DELAY_VALUE_WIDTH));

	XDfeMix_WriteReg(InstancePtr, XDFEMIX_DELAY_OFFSET, Delay);
}

/****************************************************************************/
/**
*
* Reads the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the Mixer instance.
*
* @return   Delay value
*
****************************************************************************/
u32 XDfeMix_GetTUserDelay(const XDfeMix *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfeMix_RdRegBitField(InstancePtr, XDFEMIX_DELAY_OFFSET,
				     XDFEMIX_DELAY_VALUE_WIDTH,
				     XDFEMIX_DELAY_VALUE_OFFSET);
}

/****************************************************************************/
/**
*
* Returns data latency + tap.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Tap Tap value.
* @param    TDataDelay Returned Data latency value.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_GetTDataDelay(const XDfeMix *InstancePtr, u32 Tap, u32 *TDataDelay)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tap < XDFEMIX_TAP_MAX);
	Xil_AssertNonvoid(TDataDelay != NULL);

	Data = XDfeMix_RdRegBitField(InstancePtr, XDFEMIX_LATENCY_OFFSET,
				     XDFEMIX_LATENCY_VALUE_WIDTH,
				     XDFEMIX_LATENCY_VALUE_OFFSET);
	*TDataDelay = Data + Tap;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Returns predefined Central Tap value for chosen RATE.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    Rate Interpolation/decimation rate index value [1-5].
* @param    CenterTap Returned Central Tap value.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_GetCenterTap(const XDfeMix *InstancePtr, u32 Rate, u32 *CenterTap)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Rate >= XDFEMIX_CC_CONFIG_RATE_1X);
	Xil_AssertNonvoid(Rate <= XDFEMIX_CC_CONFIG_RATE_16X);
	Xil_AssertNonvoid(CenterTap != NULL);

	/* Predefined Center Tap values */
	const u32 XDfeMix_CentralTap[5] = { 0, 23U, 55U, 115U, 235U };

	*CenterTap = XDfeMix_CentralTap[Rate - 1U];

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API gets the driver and HW design version.
*
* @param    SwVersion Driver version number.
* @param    HwVersion HW version number.
*
******************************************************************************/
void XDfeMix_GetVersions(const XDfeMix *InstancePtr, XDfeMix_Version *SwVersion,
			 XDfeMix_Version *HwVersion)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr->StateId != XDFEMIX_STATE_NOT_READY);

	/* Driver version */
	SwVersion->Major = XDFEMIX_DRIVER_VERSION_MAJOR;
	SwVersion->Minor = XDFEMIX_DRIVER_VERSION_MINOR;

	/* Component HW version */
	Version = XDfeMix_ReadReg(InstancePtr, XDFEMIX_VERSION_OFFSET);
	HwVersion->Patch =
		XDfeMix_RdBitField(XDFEMIX_VERSION_PATCH_WIDTH,
				   XDFEMIX_VERSION_PATCH_OFFSET, Version);
	HwVersion->Revision =
		XDfeMix_RdBitField(XDFEMIX_VERSION_REVISION_WIDTH,
				   XDFEMIX_VERSION_REVISION_OFFSET, Version);
	HwVersion->Minor =
		XDfeMix_RdBitField(XDFEMIX_VERSION_MINOR_WIDTH,
				   XDFEMIX_VERSION_MINOR_OFFSET, Version);
	HwVersion->Major =
		XDfeMix_RdBitField(XDFEMIX_VERSION_MAJOR_WIDTH,
				   XDFEMIX_VERSION_MAJOR_OFFSET, Version);
}
/** @} */
