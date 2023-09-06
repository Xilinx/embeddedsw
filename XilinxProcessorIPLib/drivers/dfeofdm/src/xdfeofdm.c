/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm.c
* Contains the APIs for DFE Orthogonal Frequency Division Multiplexing
* component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     11/21/22 Initial version
* 1.1   dc     05/22/23 State and status upgrades
*       dc     06/20/23 Deprecate obsolete APIs
*       dc     06/28/23 Add phase compensation calculation
*       cog    07/04/23 Add support for SDT
*       dc     07/27/23 Output delay in ccid slots
*       dc     07/31/23 Antenna interleave delay reorder
*       dc     08/28/23 Remove immediate trigger
* </pre>
* @addtogroup dfeofdm Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/
#include "xdfeofdm.h"
#include "xdfeofdm_hw.h"
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
#define XDFEOFDM_SEQUENCE_ENTRY_DEFAULT (0U) /**< Default sequence entry flag */
#define XDFEOFDM_SEQUENCE_ENTRY_NULL (-1) /**< Null sequence entry flag */
#define XDFEOFDM_NO_EMPTY_CCID_FLAG (0xFFFFU) /**< Not Empty CCID flag */
#define XDFEOFDM_COEFF_LOAD_TIMEOUT (100U) /**< Units of 10us */
#define XDFEOFDM_COEFF_UNIT_SIZE (4U) /**< Coefficient unit size */
#define XDFEOFDM_U32_NUM_BITS (32U) /**< Number of bits in register */
/**
* @endcond
*/
#define XDFEOFDM_DRIVER_VERSION_MINOR (1U) /**< Driver's minor version number */
#define XDFEOFDM_DRIVER_VERSION_MAJOR (1U) /**< Driver's major version number */

#define XDFEOFDM_PHASE_COMPENSATION_REG_STEP                                   \
	8U /** Address space step between
	space for phase compensation on one carrier */

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
/**
* @cond nocomments
*/
#ifdef __BAREMETAL__
extern struct metal_device XDfeOfdm_CustomDevice[XDFEOFDM_MAX_NUM_INSTANCES];
extern metal_phys_addr_t XDfeOfdm_metal_phys[XDFEOFDM_MAX_NUM_INSTANCES];
#endif
extern XDfeOfdm XDfeOfdm_Ofdm[XDFEOFDM_MAX_NUM_INSTANCES];
static u32 XDfeOfdm_DriverHasBeenRegisteredOnce = 0U;
static const u32 XDfeOfdm_SymbolNumberRanges[4] = { 14, 28, 56, 112 };

/************************** Function Definitions ****************************/
extern s32 XDfeOfdm_RegisterMetal(XDfeOfdm *InstancePtr,
				  struct metal_device **DevicePtr,
				  const char *DeviceNodeName);
extern s32 XDfeOfdm_LookupConfig(XDfeOfdm *InstancePtr);
extern void XDfeOfdm_CfgInitialize(XDfeOfdm *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Writes a value to register in a Ofdm instance.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    AddrOffset Address offset relative to instance base address.
* @param    Data Value to be written.
*
****************************************************************************/
void XDfeOfdm_WriteReg(const XDfeOfdm *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, (unsigned long)AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Reads a value from register using a Ofdm instance.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    AddrOffset Address offset relative to instance base address.
*
* @return   Register value.
*
****************************************************************************/
u32 XDfeOfdm_ReadReg(const XDfeOfdm *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, (unsigned long)AddrOffset);
}

/****************************************************************************/
/**
*
* Writes a bit field value to register.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
* @param    FieldData Bit field data.
*
****************************************************************************/
void XDfeOfdm_WrRegBitField(const XDfeOfdm *InstancePtr, u32 Offset,
			    u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((FieldOffset + FieldWidth) <= XDFEOFDM_U32_NUM_BITS);

	Data = XDfeOfdm_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfeOfdm_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads a bit field value from register.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
*
* @return   Bit field data.
*
****************************************************************************/
u32 XDfeOfdm_RdRegBitField(const XDfeOfdm *InstancePtr, u32 Offset,
			   u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEOFDM_U32_NUM_BITS);

	Data = XDfeOfdm_ReadReg(InstancePtr, Offset);
	return ((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U));
}

/****************************************************************************/
/**
*
* Reads a bit field value from u32 variable.
*
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 value of bit field that the function reads.
*
* @return   Bit field value.
*
****************************************************************************/
u32 XDfeOfdm_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEOFDM_U32_NUM_BITS);
	return (((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U)));
}
/****************************************************************************/
/**
*
* Writes a bit field value to u32 variable.
*
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 value of bit field that the function reads.
* @param    Val U32 value to be written in the bit field.
*
* @return   Data with a written bit field.
*
****************************************************************************/
u32 XDfeOfdm_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
{
	u32 BitFieldSet;
	u32 BitFieldClear;
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEOFDM_U32_NUM_BITS);

	BitFieldSet = (Val & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	BitFieldClear =
		Data & (~((((u32)1U << FieldWidth) - 1U) << FieldOffset));
	return (BitFieldSet | BitFieldClear);
}

/************************ OFDM Common functions ******************************/

/****************************************************************************/
/**
*
* Finds unused CCID.
*
* @param   Sequence CC sequence array.
*
* @return  Unused CCID
*
****************************************************************************/
static s32 XDfeOfdm_GetNotUsedCCID(XDfeOfdm_CCSequence *Sequence)
{
	u32 Index;
	s32 NotUsedCCID;

	/* Not used Sequence.CCID[] has value -1, but the values in the range
	   [0,15] can be written in the registers, only. Now, we have to detect
	   not used CCID, and save it for the later usage. */
	for (NotUsedCCID = 0U; NotUsedCCID < XDFEOFDM_CC_NUM; NotUsedCCID++) {
		for (Index = 0U; Index < XDFEOFDM_CC_NUM; Index++) {
			if (Sequence->CCID[Index] == NotUsedCCID) {
				break;
			}
		}
		if (Index == XDFEOFDM_CC_NUM) {
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
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCSeqBitmap maps the sequence.
*
* @return   Number of bits in CCSeqBitmap set to 1.
*
****************************************************************************/
static u32 XDfeOfdm_CountOnesInBitmap(const XDfeOfdm *InstancePtr,
				      u32 CCSeqBitmap)
{
	u32 Mask = 1U;
	u32 Index;
	u32 OnesCounter = 0;
	for (Index = 0U; Index < InstancePtr->CCSequenceLength; Index++) {
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
* Adds the specified CCID, to the CC sequence. The sequence is defined with
* CCSeqBitmap where bit0 corresponds to CC[0], bit1 to CC[1], bit2 to CC[2],
* and so on.
*
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCID CC ID.
* @param    CCSeqBitmap Maps the sequence.
* @param    CCIDSequence CC sequence array.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeOfdm_AddCCIDAndTranslateSeq(XDfeOfdm *InstancePtr, s32 CCID,
					   u32 CCSeqBitmap,
					   XDfeOfdm_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 Mask;
	u32 OnesCounter = 0;

	/* Check does sequence fit in the defined length */
	Mask = (1U << CCIDSequence->Length) - 1U;
	if (0U != (CCSeqBitmap & (~Mask))) {
		metal_log(METAL_LOG_ERROR, "Sequence map overflow\n");
		return XST_FAILURE;
	}

	/* Count ones in bitmap */
	OnesCounter = XDfeOfdm_CountOnesInBitmap(InstancePtr, CCSeqBitmap);

	/* Validate is number of ones a power of 2 */
	if ((OnesCounter != 0) && (OnesCounter != 1) && (OnesCounter != 2) &&
	    (OnesCounter != 4) && (OnesCounter != 8) && (OnesCounter != 16)) {
		metal_log(METAL_LOG_ERROR,
			  "Number of 1 in CCSeqBitmap is not power of 2: %d\n",
			  OnesCounter);
		return XST_FAILURE;
	}

	/* Check are bits set in CCSeqBitmap to 1 available (-1) */
	Mask = 1U;
	for (Index = 0U; Index < CCIDSequence->Length; Index++) {
		if (0U != (CCSeqBitmap & Mask)) {
			if (CCIDSequence->CCID[Index] !=
			    XDFEOFDM_SEQUENCE_ENTRY_NULL) {
				metal_log(METAL_LOG_ERROR,
					  "Sequence does not fit %s\n",
					  __func__);
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
	InstancePtr->NotUsedCCID = XDfeOfdm_GetNotUsedCCID(CCIDSequence);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Translate the sequence back to SEQUENCE register format.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCIDSequence CC sequence array in CCCfg.
* @param    NextCCID Returned CC sequence array.
*
****************************************************************************/
static void XDfeOfdm_TranslateSeq(const XDfeOfdm *InstancePtr,
				  const s32 *CCIDSequence, s32 *NextCCID)
{
	u32 Index;

	/* Prepare NextCCID[] to be written to registers */
	for (Index = 0U; Index < XDFEOFDM_CC_NUM; Index++) {
		if ((CCIDSequence[Index] == XDFEOFDM_SEQUENCE_ENTRY_NULL) ||
		    (Index >= InstancePtr->CCSequenceLength)) {
			NextCCID[Index] = InstancePtr->NotUsedCCID;
		} else {
			NextCCID[Index] = CCIDSequence[Index];
		}
	}
}

/****************************************************************************/
/**
*
* Removes the specified CCID from the CC sequence and replaces the CCID
* entries with null (-1).
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCID CC ID.
* @param    CCIDSequence CC sequence array.
*
****************************************************************************/
static void XDfeOfdm_RemoveCCID(XDfeOfdm *InstancePtr, s32 CCID,
				XDfeOfdm_CCSequence *CCIDSequence)
{
	u32 Index;

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] =
				XDFEOFDM_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfeOfdm_GetNotUsedCCID(CCIDSequence);
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Gets specified CCID carrier configuration.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCID CC ID.
* @param    CarrierCfg Trigger configuration container.
*
*
****************************************************************************/
static void
XDfeOfdm_GetInternalCarrierCfg(const XDfeOfdm *InstancePtr, s32 CCID,
			       XDfeOfdm_InternalCarrierCfg *CarrierCfg)
{
	u32 Val;

	/* Read specified CCID carrier configuration */
	Val = XDfeOfdm_ReadReg(
		InstancePtr,
		XDFEOFDM_CARRIER_CONFIGURATION1_CURRENT_OFFSET(CCID));
	CarrierCfg->Enable = XDfeOfdm_RdBitField(
		XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_WIDTH,
		XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_OFFSET, Val);
	CarrierCfg->Numerology = XDfeOfdm_RdBitField(
		XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_WIDTH,
		XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_OFFSET, Val);
	CarrierCfg->FftSize = XDfeOfdm_RdBitField(
		XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_WIDTH,
		XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_OFFSET, Val);
	CarrierCfg->NumSubcarriers = XDfeOfdm_RdBitField(
		XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH,
		XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_OFFSET, Val);
	CarrierCfg->ScaleFactor = XDfeOfdm_RdBitField(
		XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH,
		XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_OFFSET, Val);
	CarrierCfg->CommsStandard = XDfeOfdm_RdBitField(
		XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_WIDTH,
		XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_OFFSET, Val);
	CarrierCfg->OutputDelay = XDfeOfdm_ReadReg(
		InstancePtr,
		XDFEOFDM_CARRIER_CONFIGURATION2_CURRENT_OFFSET(CCID));
}

/****************************************************************************/
/**
*
* Sets the next FT sequence configuration.
*
* @param    CCCfg CC configuration container.
* @param    NextFtSeq Next FT sequence configuration container.
*
* @warning  FT sequence length does not support size 16.
*
****************************************************************************/
static void XDfeOfdm_SetNextFTSequence(XDfeOfdm_CCCfg *CCCfg,
				       const XDfeOfdm_FTSequence *NextFtSeq)
{
	u32 Index;

	if (NextFtSeq->Length == 0) {
		metal_log(METAL_LOG_ERROR, "FT sequence length is 0 in %s\n",
			  __func__);
	}
	if (NextFtSeq->Length == XDFEOFDM_FT_SEQ_LENGTH_MAX) {
		metal_log(METAL_LOG_ERROR, "FT sequence length is 16 in %s\n",
			  __func__);
	}

	/* Sequence Length */
	CCCfg->FTSequence.Length = NextFtSeq->Length;

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEOFDM_FT_NUM; Index++) {
		CCCfg->FTSequence.CCID[Index] = NextFtSeq->CCID[Index];
	}
}

/****************************************************************************/
/**
*
* Checks is FT Sequence length as expected.
*
* @param    NextCCCfg Next CC configuration container.
* @param    NewFTLength New, expected FT sequence length.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeOfdm_CheckFTSequenceLength(XDfeOfdm_CCCfg *NextCCCfg,
					  u32 NewFTLength)
{
	u32 Index;
	u32 CurrentFTLength = 0;

	/* Calculate expected FT sequence length */
	for (Index = 0; Index < XDFEOFDM_CC_SEQ_LENGTH_MAX; Index++) {
		if (NextCCCfg->CarrierCfg[Index].Enable ==
		    XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_DISABLED) {
			continue;
		}

		/* Is numerology value 15 or 30kHz */
		switch (NextCCCfg->CarrierCfg[Index].Numerology) {
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_15kHz:
			CurrentFTLength += 1U;
			break;
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_30kHz:
			CurrentFTLength += 2U;
			break;
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_60kHz:
			CurrentFTLength += 4U;
			break;
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_120kHz:
			CurrentFTLength += 8U;
			break;
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_240kHz:
			CurrentFTLength += 16U;
			break;
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_480kHz:
			CurrentFTLength += 32U;
			break;
		case XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_960kHz:
			CurrentFTLength += 64U;
			break;
		default:
			metal_log(METAL_LOG_ERROR,
				  "Wrong Numerology on CCID %d\n", Index);
		}
	}

	/* Check is the length as expected */
	if (CurrentFTLength != NewFTLength) {
		metal_log(METAL_LOG_ERROR,
			  "Not expected FT sequence length in %s"
			  "CurrentFTLength=%d, NewFTLength=%d\n",
			  __func__, CurrentFTLength, NewFTLength);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of LowPower trigger.
* If Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
****************************************************************************/
static void XDfeOfdm_EnableLowPowerTrigger(const XDfeOfdm *InstancePtr)
{
	u32 Data;

	Data = XDfeOfdm_ReadReg(InstancePtr,
				XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Data,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET,
			  Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set enable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
****************************************************************************/
static void XDfeOfdm_EnableActivateTrigger(const XDfeOfdm *InstancePtr)
{
	u32 Data;

	Data = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Data,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				   XDFEOFDM_TRIGGERS_STATE_OUTPUT_ENABLED);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set disable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
****************************************************************************/
static void XDfeOfdm_EnableDeactivateTrigger(const XDfeOfdm *InstancePtr)
{
	u32 Data;

	Data = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Data,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				   XDFEOFDM_TRIGGERS_STATE_OUTPUT_DISABLED);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers and resets enable a bit of LowPower trigger.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
****************************************************************************/
static void XDfeOfdm_DisableLowPowerTrigger(const XDfeOfdm *InstancePtr)
{
	u32 Data;

	Data = XDfeOfdm_ReadReg(InstancePtr,
				XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Data,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET,
			  Data);
}

/****************************************************************************/
/**
*
* Gets PhaseCompensationRates and allocate it in corresponding CCID position
* in XDfeOfdm_InternalCarrierCfg.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg CC configuration container.
*
****************************************************************************/
static void XDfeOfdm_GetPhaseCompensation(const XDfeOfdm *InstancePtr,
					  XDfeOfdm_CCCfg *CCCfg)
{
	u32 SeqIndex;
	u32 Index;
	u32 SeqLen;
	u32 UniqueId[XDFEOFDM_FT_SEQ_LENGTH_MAX];
	u32 UniqueIdIndex = 0;
	u32 CCID;
	u32 Offset;
	u32 Range;
	u32 Numerology;
	u32 ContinueFlag;

	if (InstancePtr->Config.PhaseCompensation ==
	    XDFEOFDM_MODEL_PARAM_PHASE_COMPENSATION_DISABLED) {
		return;
	}

	SeqLen = CCCfg->FTSequence.Length;
	for (SeqIndex = 0; SeqIndex < SeqLen; SeqIndex++) {
		/* Get next ccid from FT sequence */
		CCID = CCCfg->FTSequence.CCID[SeqIndex];
		/* Find is it already used */
		ContinueFlag = 0;
		for (Index = 0; Index < UniqueIdIndex; Index++) {
			if (UniqueId[Index] == CCID) {
				/* Already used ccid */
				ContinueFlag = 1;
				break;
			}
		}
		if (ContinueFlag == 1) {
			continue;
		}
		/* This CCID is used first time now, put it in UniqueId array
		   for the future searches */
		UniqueId[UniqueIdIndex] = CCID;

		/* Read phase compensations from registers. Number is defined
		   by Numerology of this CCID */
		Numerology = XDfeOfdm_RdRegBitField(
			InstancePtr,
			XDFEOFDM_CARRIER_CONFIGURATION1_CURRENT_OFFSET(CCID),
			XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_OFFSET);

		/* Finally read compensation data */
		Range = XDfeOfdm_SymbolNumberRanges[Numerology];
		for (Index = 0; Index < Range; Index++) {
			Offset = UniqueIdIndex +
				 (Index * XDFEOFDM_PHASE_COMPENSATION_REG_STEP);
			CCCfg->CarrierCfg[CCID]
				.PhaseCompensation[Index] = XDfeOfdm_ReadReg(
				InstancePtr,
				XDFEOFDM_PHASE_COMPENSATION_CURRENT_OFFSET(
					Offset));
		}
		UniqueIdIndex++;
	}
}

/****************************************************************************/
/**
*
* Sets PhaseCompensationRates.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg CC configuration container.
*
****************************************************************************/
static void XDfeOfdm_SetPhaseCompensation(const XDfeOfdm *InstancePtr,
					  const XDfeOfdm_CCCfg *CCCfg)
{
	u32 SeqIndex;
	u32 Index;
	u32 SeqLen;
	u32 UniqueId[XDFEOFDM_FT_SEQ_LENGTH_MAX];
	u32 UniqueIdIndex = 0;
	u32 CCID;
	u32 Offset;
	u32 Range;
	u32 Numerology;
	u32 ContinueFlag;

	if (InstancePtr->Config.PhaseCompensation ==
	    XDFEOFDM_MODEL_PARAM_PHASE_COMPENSATION_DISABLED) {
		return;
	}

	SeqLen = CCCfg->FTSequence.Length;
	for (SeqIndex = 0; SeqIndex < SeqLen; SeqIndex++) {
		/* Get next ccid from FT sequence */
		CCID = CCCfg->FTSequence.CCID[SeqIndex];
		/* Find is it already used */
		ContinueFlag = 0;
		for (Index = 0; Index < UniqueIdIndex; Index++) {
			if (UniqueId[Index] == CCID) {
				/* Already used ccid */
				ContinueFlag = 1;
				break;
			}
		}
		if (ContinueFlag == 1) {
			continue;
		}

		/* This CCID is used first time now, put it in UniqueId array
		   for the future searches */
		UniqueId[UniqueIdIndex] = CCID;

		/* Get phase compensations from registers. Number is defined
		   by Numerology of this CCID */
		Numerology = CCCfg->CarrierCfg[CCID].Numerology;

		/* Finally write compensation data */
		Range = XDfeOfdm_SymbolNumberRanges[Numerology];
		for (Index = 0; Index < Range; Index++) {
			Offset = UniqueIdIndex +
				 (Index * XDFEOFDM_PHASE_COMPENSATION_REG_STEP);
			XDfeOfdm_WriteReg(
				InstancePtr,
				XDFEOFDM_PHASE_COMPENSATION_NEXT_OFFSET(Offset),
				CCCfg->CarrierCfg[CCID]
					.PhaseCompensation[Index]);
		}
		UniqueIdIndex++;
	}
}

/****************************************************************************/
/**
*
* Sets output delay for CCID.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg CC configuration container.
* @param    CCID CC ID.
* @param    OutputDelay Delay which should be written for this CCID
*
****************************************************************************/
static void XDfeOfdm_SetOutputDelay(const XDfeOfdm *InstancePtr,
				    XDfeOfdm_CCCfg *CCCfg, const s32 CCID,
				    const u32 OutputDelay)
{
	Xil_AssertVoid((InstancePtr->Config.AntennaInterleave == 1U) ||
		       (InstancePtr->Config.AntennaInterleave == 2U) ||
		       (InstancePtr->Config.AntennaInterleave == 4U));
	u32 Ail = InstancePtr->Config.AntennaInterleave;
	u32 Index;
	for (Index = 0; Index < XDFEOFDM_CC_SEQ_LENGTH_MAX / Ail; Index++) {
		if (CCCfg->CCSequence.CCID[Index] == CCID) {
			if (Ail == 1U) {
				CCCfg->CarrierCfg[Index].OutputDelay =
					OutputDelay;
			} else if (Ail == 2U) {
				CCCfg->CarrierCfg[Index * 2].OutputDelay =
					OutputDelay;
				CCCfg->CarrierCfg[Index * 2 + 1].OutputDelay =
					OutputDelay;
			} else if (Ail == 4U) {
				CCCfg->CarrierCfg[Index * 4].OutputDelay =
					OutputDelay;
				CCCfg->CarrierCfg[Index * 4 + 1].OutputDelay =
					OutputDelay;
				CCCfg->CarrierCfg[Index * 4 + 2].OutputDelay =
					OutputDelay;
				CCCfg->CarrierCfg[Index * 4 + 3].OutputDelay =
					OutputDelay;
			}
		}
	}
}

/****************************************************************************/
/**
*
* Gets output delay allocated for CCID.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg CC configuration container.
* @param    CCID CC ID.

* @return   Output delay allocated for this CCID, 0 if not allocated.
*
****************************************************************************/
static u32 XDfeOfdm_GetOutputDelay(const XDfeOfdm *InstancePtr,
				   const XDfeOfdm_CCCfg *CCCfg, s32 CCID)
{
	Xil_AssertVoid((InstancePtr->Config.AntennaInterleave == 1U) ||
		       (InstancePtr->Config.AntennaInterleave == 2U) ||
		       (InstancePtr->Config.AntennaInterleave == 4U));
	u32 Ail = InstancePtr->Config.AntennaInterleave;
	u32 Index;
	for (Index = 0; Index < XDFEOFDM_CC_SEQ_LENGTH_MAX; Index++) {
		if (CCCfg->CCSequence.CCID[Index] == CCID) {
			if (Ail == 1U) {
				return CCCfg->CarrierCfg[Index].OutputDelay;
			} else if (Ail == 2U) {
				return CCCfg->CarrierCfg[Index * 2].OutputDelay;
			} else if (Ail == 4U) {
				return CCCfg->CarrierCfg[Index * 4].OutputDelay;
			}
		}
	}
	metal_log(METAL_LOG_ERROR, "Error, CCID=%d is in sequence: %s\n", CCID,
		  __func__);
	return 0;
}

/**
* @endcond
*/

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* Initialises one instance of a OFDM driver.
* Traverses "/sys/bus/platform/device" directory (in Linux), to find registered
* OFDM device with the name DeviceNodeName. The first available slot in
* the instances array XDfeOfdm_Ofdm[] will be taken as a DeviceNodeName
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
XDfeOfdm *XDfeOfdm_InstanceInit(const char *DeviceNodeName)
{
	u32 Index;
	XDfeOfdm *InstancePtr;
#ifdef __BAREMETAL__
	char Str[XDFEOFDM_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;
#endif

	Xil_AssertNonvoid(DeviceNodeName != NULL);
	Xil_AssertNonvoid(strlen(DeviceNodeName) <
			  XDFEOFDM_NODE_NAME_MAX_LENGTH);

	/* Is this first OFDM initialisation ever? */
	if (0U == XDfeOfdm_DriverHasBeenRegisteredOnce) {
		/* Set up environment to non-initialized */
		for (Index = 0; XDFEOFDM_INSTANCE_EXISTS(Index); Index++) {
			XDfeOfdm_Ofdm[Index].StateId = XDFEOFDM_STATE_NOT_READY;
			XDfeOfdm_Ofdm[Index].NodeName[0] = '\0';
		}
		XDfeOfdm_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check has DeviceNodeName been already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	for (Index = 0; XDFEOFDM_INSTANCE_EXISTS(Index); Index++) {
		if (0U == strncmp(XDfeOfdm_Ofdm[Index].NodeName, DeviceNodeName,
				  strlen(DeviceNodeName))) {
			XDfeOfdm_Ofdm[Index].StateId = XDFEOFDM_STATE_READY;
			return &XDfeOfdm_Ofdm[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; XDFEOFDM_INSTANCE_EXISTS(Index); Index++) {
		if (XDfeOfdm_Ofdm[Index].NodeName[0] == '\0') {
			strncpy(XDfeOfdm_Ofdm[Index].NodeName, DeviceNodeName,
				XDFEOFDM_NODE_NAME_MAX_LENGTH);
			InstancePtr = &XDfeOfdm_Ofdm[Index];
			goto register_metal;
		}
	}

	/* Failing as there is no available slot. */
	return NULL;

register_metal:
#ifdef __BAREMETAL__
	memcpy(Str, InstancePtr->NodeName, XDFEOFDM_NODE_NAME_MAX_LENGTH);
	AddrStr = strtok(Str, ".");
	Addr = strtoul(AddrStr, NULL, 16);
	for (Index = 0; XDFEOFDM_INSTANCE_EXISTS(Index); Index++) {
		if (Addr == XDfeOfdm_metal_phys[Index]) {
			InstancePtr->Device = &XDfeOfdm_CustomDevice[Index];
			goto bm_register_metal;
		}
	}
	return NULL;
bm_register_metal:
#endif

	/* Register libmetal for this OS process */
	if (XST_SUCCESS != XDfeOfdm_RegisterMetal(InstancePtr,
						  &InstancePtr->Device,
						  DeviceNodeName)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeOfdm_LookupConfig(InstancePtr)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to configure device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Configure HW and the driver instance */
	XDfeOfdm_CfgInitialize(InstancePtr);

	InstancePtr->StateId = XDFEOFDM_STATE_READY;

	return InstancePtr;

return_error:
	InstancePtr->StateId = XDFEOFDM_STATE_NOT_READY;
	InstancePtr->NodeName[0] = '\0';
	return NULL;
}

/*****************************************************************************/
/**
*
* Closes the instances of a OFDM driver and moves the state
* machine to a Not Ready state.
*
* @param    InstancePtr Pointer to the Ofdm instance.
*
******************************************************************************/
void XDfeOfdm_InstanceClose(XDfeOfdm *InstancePtr)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; XDFEOFDM_INSTANCE_EXISTS(Index); Index++) {
		/* Find the instance in XDfeOfdm_Ofdm array */
		if (&XDfeOfdm_Ofdm[Index] == InstancePtr) {
			/* Release libmetal */
			metal_device_close(InstancePtr->Device);
			InstancePtr->StateId = XDFEOFDM_STATE_NOT_READY;
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
* Resets OFDM and puts block into a reset state.
*
* @param    InstancePtr Pointer to the Ofdm instance.
*
****************************************************************************/
void XDfeOfdm_Reset(XDfeOfdm *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEOFDM_STATE_NOT_READY);

	/* Put Ofdm in reset */
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_RESET_OFFSET,
			  XDFEOFDM_RESET_ON);
	InstancePtr->StateId = XDFEOFDM_STATE_RESET;
}

/****************************************************************************/
/**
*
* Reads configuration from device tree/xparameters.h and IP registers.
* Removes S/W reset and moves the state machine to a Configured state.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    Cfg Configuration data container.
*
****************************************************************************/
void XDfeOfdm_Configure(XDfeOfdm *InstancePtr, XDfeOfdm_Cfg *Cfg)
{
	u32 Version;
	u32 ModelParam;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEOFDM_STATE_RESET);
	Xil_AssertVoid(Cfg != NULL);

	/* Read vearsion */
	Version = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_VERSION_OFFSET);
	Cfg->Version.Patch =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_PATCH_WIDTH,
				    XDFEOFDM_VERSION_PATCH_OFFSET, Version);
	Cfg->Version.Revision =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_REVISION_WIDTH,
				    XDFEOFDM_VERSION_REVISION_OFFSET, Version);
	Cfg->Version.Minor =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_MINOR_WIDTH,
				    XDFEOFDM_VERSION_MINOR_OFFSET, Version);
	Cfg->Version.Major =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_MAJOR_WIDTH,
				    XDFEOFDM_VERSION_MAJOR_OFFSET, Version);

	ModelParam = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_MODEL_PARAM_OFFSET);
	InstancePtr->Config.NumAntenna =
		XDfeOfdm_RdBitField(XDFEOFDM_MODEL_PARAM_NUM_ANTENNA_WIDTH,
				    XDFEOFDM_MODEL_PARAM_NUM_ANTENNA_OFFSET,
				    ModelParam);
	InstancePtr->Config.AntennaInterleave = XDfeOfdm_RdBitField(
		XDFEOFDM_MODEL_PARAM_ANTENNA_INTERLEAVE_WIDTH,
		XDFEOFDM_MODEL_PARAM_ANTENNA_INTERLEAVE_OFFSET, ModelParam);
	InstancePtr->Config.PhaseCompensation = XDfeOfdm_RdBitField(
		XDFEOFDM_MODEL_PARAM_PHASE_COMPENSATION_WIDTH,
		XDFEOFDM_MODEL_PARAM_PHASE_COMPENSATION_OFFSET, ModelParam);

	/* Copy configs model parameters from InstancePtr */
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.AntennaInterleave =
		InstancePtr->Config.AntennaInterleave;
	Cfg->ModelParams.PhaseCompensation =
		InstancePtr->Config.PhaseCompensation;

	/* Release RESET */
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_RESET_OFFSET,
			  XDFEOFDM_RESET_OFF);
	InstancePtr->StateId = XDFEOFDM_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE Ofdm driver one time initialisation, also moves the state machine to
* an Initialised state.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    Init Initialisation data container.
*
****************************************************************************/
void XDfeOfdm_Initialize(XDfeOfdm *InstancePtr, XDfeOfdm_Init *Init)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEOFDM_STATE_CONFIGURED);
	Xil_AssertVoid(Init != NULL);

	/* Enable OFDM */
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_STATE_OFDM_ENABLE_OFFSET,
			  XDFEOFDM_STATE_OFDM_ENABLE_BF_ENABLED);

	/* Write "one-time" CC Sequence length. InstancePtr->CCSequenceLength holds
	   the exact sequence length value as register sequence length value 0
	   can be understood as length 0 or 1 */
	InstancePtr->NotUsedCCID = 0;
	InstancePtr->CCSequenceLength = Init->CCSequenceLength;

	/* Write 0 to FT Sequence length */
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_FT_SEQUENCE_LENGTH_NEXT_OFFSET,
			  0);

	InstancePtr->StateId = XDFEOFDM_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activates OFDM and moves the state machine to an Activated state.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    EnableLowPower Flag indicating low power.
*
******************************************************************************/
void XDfeOfdm_Activate(XDfeOfdm *InstancePtr, bool EnableLowPower)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEOFDM_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEOFDM_STATE_OPERATIONAL));

	/* Do nothing if the block already operational */
	IsOperational =
		XDfeOfdm_RdRegBitField(InstancePtr,
				       XDFEOFDM_STATE_OPERATIONAL_OFFSET,
				       XDFEOFDM_STATE_OPERATIONAL_BF_WIDTH,
				       XDFEOFDM_STATE_OPERATIONAL_BF_OFFSET);
	if (IsOperational == XDFEOFDM_STATE_OPERATIONAL_YES) {
		return;
	}

	/* Enable the Activate trigger and set to one-shot */
	XDfeOfdm_EnableActivateTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfeOfdm_EnableLowPowerTrigger(InstancePtr);
	}

	/* Channel filter is operational now, change a state */
	InstancePtr->StateId = XDFEOFDM_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* Deactivates OFDM and moves the state machine to Initialised state.
*
* @param    InstancePtr Pointer to the Ofdm instance.
*
******************************************************************************/
void XDfeOfdm_Deactivate(XDfeOfdm *InstancePtr)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEOFDM_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEOFDM_STATE_OPERATIONAL));

	/* Do nothing if the block already deactivated */
	IsOperational =
		XDfeOfdm_RdRegBitField(InstancePtr,
				       XDFEOFDM_STATE_OPERATIONAL_OFFSET,
				       XDFEOFDM_STATE_OPERATIONAL_BF_WIDTH,
				       XDFEOFDM_STATE_OPERATIONAL_BF_OFFSET);
	if (IsOperational == XDFEOFDM_STATE_OPERATIONAL_NO) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfeOfdm_DisableLowPowerTrigger(InstancePtr);

	/* Enable Deactivate trigger */
	XDfeOfdm_EnableDeactivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFEOFDM_STATE_INITIALISED;
}

/****************************************************************************/
/**
*
* Gets a state machine state id.
*
* @param    InstancePtr Pointer to the Ofdm instance.
*
* @return   State machine StateID
*
****************************************************************************/
XDfeOfdm_StateId XDfeOfdm_GetStateID(XDfeOfdm *InstancePtr)
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
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CurrCCCfg CC configuration container.
*
* @note     For a sequence conversion see XDfeOfdm_AddCCtoCCCfg() comment.
*
****************************************************************************/
void XDfeOfdm_GetCurrentCCCfg(const XDfeOfdm *InstancePtr,
			      XDfeOfdm_CCCfg *CurrCCCfg)
{
	u32 SeqLen;
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CurrCCCfg != NULL);

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEOFDM_CC_NUM; Index++) {
		CurrCCCfg->CCSequence.CCID[Index] = XDfeOfdm_ReadReg(
			InstancePtr,
			XDFEOFDM_CC_SEQUENCE_CURRENT_OFFSET(Index));
		CurrCCCfg->FTSequence.CCID[Index] = XDfeOfdm_ReadReg(
			InstancePtr,
			XDFEOFDM_FT_SEQUENCE_CURRENT_OFFSET(Index));
		XDfeOfdm_GetInternalCarrierCfg(InstancePtr, Index,
					       &CurrCCCfg->CarrierCfg[Index]);
	}

	/* Read sequence length */
	CurrCCCfg->CCSequence.Length = InstancePtr->CCSequenceLength;
	SeqLen = XDfeOfdm_ReadReg(InstancePtr,
				  XDFEOFDM_FT_SEQUENCE_LENGTH_CURRENT_OFFSET);
	CurrCCCfg->FTSequence.Length = SeqLen + 1U;

	/* Convert not used CC to -1 */
	for (Index = 0U; Index < XDFEOFDM_CC_NUM; Index++) {
		if ((CurrCCCfg->CCSequence.CCID[Index] ==
		     InstancePtr->NotUsedCCID) ||
		    (Index >= InstancePtr->CCSequenceLength)) {
			CurrCCCfg->CCSequence.CCID[Index] =
				XDFEOFDM_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Read PhaseCompensationRates and allocate it in corresponding CCID
	   position in XDfeOfdm_InternalCarrierCfg */
	XDfeOfdm_GetPhaseCompensation(InstancePtr, CurrCCCfg);
}

/****************************************************************************/
/**
*
* Returns configuration structure CCCfg with CCCfg->CCSequence.Length value set
* in XDfeOfdm_Configure(), array CCCfg->CCSequence.CCID[] members are set to not
* used value (-1) and the other CCCfg members are set to 0.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg CC configuration container.
*
****************************************************************************/
void XDfeOfdm_GetEmptyCCCfg(const XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	memset(CCCfg, 0, sizeof(XDfeOfdm_CCCfg));

	/* Convert CC to -1 meaning not used */
	for (Index = 0U; Index < XDFEOFDM_CC_NUM; Index++) {
		CCCfg->CCSequence.CCID[Index] = XDFEOFDM_SEQUENCE_ENTRY_NULL;
		CCCfg->FTSequence.CCID[Index] = 0;
	}
	/* Read sequence length */
	CCCfg->CCSequence.Length = InstancePtr->CCSequenceLength;
	CCCfg->FTSequence.Length = 0;
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
* The returned CCCfg.CCSequence is transleted as there is no explicit indication that
* SEQUENCE[i] is not used - 0 can define the slot as either used or not used.
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID.
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
* @param    FTSeq
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeOfdm_AddCCtoCCCfg(XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			  s32 CCID, u32 CCSeqBitmap,
			  const XDfeOfdm_CarrierCfg *CarrierCfg,
			  XDfeOfdm_FTSequence *FTSeq)
{
	u32 AddSuccess;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(CCID <= XDFEOFDM_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(FTSeq != NULL);
	Xil_AssertNonvoid(CarrierCfg->Numerology <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_960kHz);
	Xil_AssertNonvoid((CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_1024) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_2048) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_4096));
	Xil_AssertNonvoid(
		CarrierCfg->NumSubcarriers <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH));
	Xil_AssertNonvoid(
		CarrierCfg->ScaleFactor <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH));
	Xil_AssertNonvoid(CarrierCfg->CommsStandard <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE);
	Xil_AssertNonvoid(
		CarrierCfg->OutputDelay <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION2_OUTPUT_DELAY_WIDTH));

	/* Numerologu should be set to 15kHz only for LTE standard */
	if ((CarrierCfg->CommsStandard ==
	     XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE) &&
	    (CarrierCfg->Numerology !=
	     XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_15kHz)) {
		metal_log(METAL_LOG_ERROR,
			  "Error, Numerology = %d in LTE mode in %s\n",
			  CarrierCfg->Numerology, __func__);
		return XST_FAILURE;
	}

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeOfdm_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg->CCSequence);
	if (AddSuccess == (u32)XST_SUCCESS) {
		/* Update carrier configuration, mark flush as we need to clear
		   data registers */
		CCCfg->CarrierCfg[CCID].Enable =
			XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_ENABLED;
		CCCfg->CarrierCfg[CCID].Numerology = CarrierCfg->Numerology;
		CCCfg->CarrierCfg[CCID].FftSize = log2(CarrierCfg->FftSize);
		CCCfg->CarrierCfg[CCID].NumSubcarriers =
			CarrierCfg->NumSubcarriers;
		CCCfg->CarrierCfg[CCID].ScaleFactor = CarrierCfg->ScaleFactor;
		CCCfg->CarrierCfg[CCID].CommsStandard =
			CarrierCfg->CommsStandard;
		XDfeOfdm_SetOutputDelay(InstancePtr, CCCfg, CCID,
					CarrierCfg->OutputDelay);
		memcpy(CCCfg->CarrierCfg[CCID].PhaseCompensation,
		       CarrierCfg->PhaseCompensation,
		       sizeof(u32) * XDFEOFDM_PHASE_COMPENSATION_MAX);
	} else {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Check FT sequence length */
	if (XST_FAILURE ==
	    XDfeOfdm_CheckFTSequenceLength(CCCfg, FTSeq->Length)) {
		metal_log(METAL_LOG_ERROR, "Wrong FT sequence length in %s\n",
			  __func__);
		XDfeOfdm_RemoveCCID(InstancePtr, CCID, &CCCfg->CCSequence);
		CCCfg->CarrierCfg[CCID].Enable =
			XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_DISABLED;

		return XST_FAILURE;
	}

	/* Set FT sequence */
	XDfeOfdm_SetNextFTSequence(CCCfg, FTSeq);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Returns the current CCID carrier configuration.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID.
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
*
****************************************************************************/
void XDfeOfdm_GetCarrierCfg(const XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			    s32 CCID, u32 *CCSeqBitmap,
			    XDfeOfdm_CarrierCfg *CarrierCfg)
{
	u32 Index;
	u32 Mask = 0x1;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID <= XDFEOFDM_CC_NUM);
	Xil_AssertVoid(CCSeqBitmap != NULL);
	Xil_AssertVoid(CarrierCfg != NULL);
	Xil_AssertVoid(CCCfg->CarrierCfg[CCID].Numerology <=
		       XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_960kHz);
	Xil_AssertVoid(
		CCCfg->CarrierCfg[CCID].NumSubcarriers <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH));
	Xil_AssertVoid(
		CCCfg->CarrierCfg[CCID].ScaleFactor <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH));
	Xil_AssertVoid(CCCfg->CarrierCfg[CCID].CommsStandard <=
		       XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE);
	Xil_AssertVoid(
		CCCfg->CarrierCfg[CCID].OutputDelay <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION2_OUTPUT_DELAY_WIDTH));

	/* Numerologu should be set to 15kHz only for LTE standard */
	if ((CarrierCfg->CommsStandard ==
	     XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE) &&
	    (CarrierCfg->Numerology !=
	     XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_15kHz)) {
		metal_log(METAL_LOG_ERROR,
			  "Error, Numerology = %d in LTE mode in %s\n",
			  CarrierCfg->Numerology, __func__);
	}

	CarrierCfg->Numerology = CCCfg->CarrierCfg[CCID].Numerology;
	CarrierCfg->FftSize = pow(CCCfg->CarrierCfg[CCID].FftSize, 2);
	CarrierCfg->NumSubcarriers = CCCfg->CarrierCfg[CCID].NumSubcarriers;
	CarrierCfg->ScaleFactor = CCCfg->CarrierCfg[CCID].ScaleFactor;
	CarrierCfg->CommsStandard = CCCfg->CarrierCfg[CCID].CommsStandard;
	CarrierCfg->OutputDelay =
		XDfeOfdm_GetOutputDelay(InstancePtr, CCCfg, CCID);

	*CCSeqBitmap = 0;
	for (Index = 0; Index < CCCfg->CCSequence.Length; Index++) {
		if (CCCfg->CCSequence.CCID[Index] == CCID) {
			*CCSeqBitmap |= Mask;
		}
		Mask <<= 1;
	}
}

/****************************************************************************/
/**
*
* Removes specified CCID from a local CC configuration structure.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     For a sequence conversion see XDfeOfdm_AddCCtoCCCfg comment.
*
****************************************************************************/
u32 XDfeOfdm_RemoveCCfromCCCfg(XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			       s32 CCID, XDfeOfdm_FTSequence *FTSeq)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCID <= XDFEOFDM_CC_NUM);
	Xil_AssertNonvoid(CCCfg != NULL);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeOfdm_RemoveCCID(InstancePtr, CCID, &CCCfg->CCSequence);
	CCCfg->CarrierCfg[CCID].Enable =
		XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_DISABLED;

	/* Check FT sequence length */
	if (XST_FAILURE ==
	    XDfeOfdm_CheckFTSequenceLength(CCCfg, FTSeq->Length)) {
		metal_log(METAL_LOG_ERROR, "Wrong FT sequence length in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Set FT sequence */
	XDfeOfdm_SetNextFTSequence(CCCfg, FTSeq);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Updates specified CCID, with specified configuration to a local CC
* configuration structure.
* If there is insufficient capacity for the new CC the function will return
* an error.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeOfdm_UpdateCCinCCCfg(XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			     s32 CCID, const XDfeOfdm_CarrierCfg *CarrierCfg,
			     XDfeOfdm_FTSequence *FTSeq)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(CCID <= XDFEOFDM_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(CarrierCfg->Numerology <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_960kHz);
	Xil_AssertNonvoid((CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_1024) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_2048) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_4096));
	Xil_AssertNonvoid(
		CarrierCfg->NumSubcarriers <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH));
	Xil_AssertNonvoid(
		CarrierCfg->ScaleFactor <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH));
	Xil_AssertNonvoid(CarrierCfg->CommsStandard <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE);
	Xil_AssertNonvoid(
		CarrierCfg->OutputDelay <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION2_OUTPUT_DELAY_WIDTH));

	/* Numerologu should be set to 15kHz only for LTE standard */
	if ((CarrierCfg->CommsStandard ==
	     XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE) &&
	    (CarrierCfg->Numerology !=
	     XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_15kHz)) {
		metal_log(METAL_LOG_ERROR,
			  "Error, Numerology = %d in LTE mode in %s\n",
			  CarrierCfg->Numerology, __func__);
		return XST_FAILURE;
	}

	CCCfg->CarrierCfg[CCID].Numerology = CarrierCfg->Numerology;
	CCCfg->CarrierCfg[CCID].FftSize = log2(CarrierCfg->FftSize);
	CCCfg->CarrierCfg[CCID].NumSubcarriers = CarrierCfg->NumSubcarriers;
	CCCfg->CarrierCfg[CCID].ScaleFactor = CarrierCfg->ScaleFactor;
	CCCfg->CarrierCfg[CCID].CommsStandard = CarrierCfg->CommsStandard;
	XDfeOfdm_SetOutputDelay(InstancePtr, CCCfg, CCID,
				CarrierCfg->OutputDelay);

	memcpy(CCCfg->CarrierCfg[CCID].PhaseCompensation,
	       CarrierCfg->PhaseCompensation,
	       sizeof(u32) * XDFEOFDM_PHASE_COMPENSATION_MAX);

	/* Check FT sequence length */
	if (XST_FAILURE ==
	    XDfeOfdm_CheckFTSequenceLength(CCCfg, FTSeq->Length)) {
		metal_log(METAL_LOG_ERROR, "Wrong FT sequence length in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Set FT sequence */
	XDfeOfdm_SetNextFTSequence(CCCfg, FTSeq);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Sets the next CC configuration.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    NextCCCfg Next CC configuration container.
*
****************************************************************************/
void XDfeOfdm_SetNextCCCfg(const XDfeOfdm *InstancePtr,
			   const XDfeOfdm_CCCfg *NextCCCfg)
{
	u32 CarrierCfg;
	u32 Index;
	u32 SeqLength;
	s32 NextCCID[XDFEOFDM_CC_SEQ_LENGTH_MAX];

	/* Prepare NextCCID[] to be written to registers */
	XDfeOfdm_TranslateSeq(InstancePtr, NextCCCfg->CCSequence.CCID,
			      NextCCID);

	/* Sequence Length should remain the same, so take the sequence length
	   from InstancePtr->SequenceLength and decrement for 1. The following
	   if statement is to distinguish how to calculate length in case
	   InstancePtr->SequenceLength = 0 or 1 whih both will put 0 in the
	   CURRENT seqLength register */
	if (InstancePtr->CCSequenceLength == 0) {
		SeqLength = 0U;
	} else {
		SeqLength = InstancePtr->CCSequenceLength - 1U;
	}
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_CC_SEQUENCE_LENGTH_NEXT_OFFSET,
			  SeqLength);

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEOFDM_CC_NUM; Index++) {
		XDfeOfdm_WriteReg(InstancePtr,
				  XDFEOFDM_CC_SEQUENCE_NEXT_OFFSET(Index),
				  NextCCID[Index]);

		CarrierCfg = XDfeOfdm_WrBitField(
			XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_OFFSET, 0U,
			NextCCCfg->CarrierCfg[Index].Enable);
		CarrierCfg = XDfeOfdm_WrBitField(
			XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].Numerology);
		if ((NextCCCfg->CarrierCfg[Index].Enable ==
		     XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_ENABLED) &&
		    ((NextCCCfg->CarrierCfg[Index].FftSize !=
		      XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_1024) &&
		     (NextCCCfg->CarrierCfg[Index].FftSize !=
		      XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_2048) &&
		     (NextCCCfg->CarrierCfg[Index].FftSize !=
		      XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_4096))) {
			metal_log(METAL_LOG_ERROR,
				  "Unexpected FftSize %d for CCID %d in %s\n",
				  NextCCCfg->CarrierCfg[Index].FftSize, Index,
				  __func__);
		}
		CarrierCfg = XDfeOfdm_WrBitField(
			XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_FFT_SIZE_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].FftSize);
		CarrierCfg = XDfeOfdm_WrBitField(
			XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_OFFSET,
			CarrierCfg,
			NextCCCfg->CarrierCfg[Index].NumSubcarriers);
		CarrierCfg = XDfeOfdm_WrBitField(
			XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].ScaleFactor);
		CarrierCfg = XDfeOfdm_WrBitField(
			XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_WIDTH,
			XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].CommsStandard);
		XDfeOfdm_WriteReg(
			InstancePtr,
			XDFEOFDM_CARRIER_CONFIGURATION1_NEXT_OFFSET(Index),
			CarrierCfg);
		XDfeOfdm_WriteReg(
			InstancePtr,
			XDFEOFDM_CARRIER_CONFIGURATION2_NEXT_OFFSET(Index),
			NextCCCfg->CarrierCfg[Index].OutputDelay);
	}

	/* Set PhaseCompensationRates and allocate it in corresponding CCID
	   position in PHASE_COMPENSATION_WEIGHTS */
	XDfeOfdm_SetPhaseCompensation(InstancePtr, NextCCCfg);

	/* Set FT sequence data */
	/* Sequence Length */
	if (NextCCCfg->FTSequence.Length == 0) {
		XDfeOfdm_WriteReg(InstancePtr,
				  XDFEOFDM_FT_SEQUENCE_LENGTH_NEXT_OFFSET, 0);
	} else {
		XDfeOfdm_WriteReg(InstancePtr,
				  XDFEOFDM_FT_SEQUENCE_LENGTH_NEXT_OFFSET,
				  NextCCCfg->FTSequence.Length - 1U);
	}

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEOFDM_FT_NUM; Index++) {
		XDfeOfdm_WriteReg(InstancePtr,
				  XDFEOFDM_FT_SEQUENCE_NEXT_OFFSET(Index),
				  NextCCCfg->FTSequence.CCID[Index]);
	}
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of update trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeOfdm_EnableCCUpdateTrigger(const XDfeOfdm *InstancePtr)
{
	u32 Data;

	/* Exit with error if CC_UPDATE status is high */
	if (XDFEOFDM_CC_UPDATE_TRIGGERED_HIGH ==
	    XDfeOfdm_RdRegBitField(InstancePtr, XDFEOFDM_STATUS_ISR_OFFSET,
				   XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_OFFSET)) {
		metal_log(METAL_LOG_ERROR, "CCUpdate status high in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Enable CCUpdate trigger */
	Data = XDfeOfdm_ReadReg(InstancePtr,
				XDFEOFDM_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Data,
				   XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_CC_UPDATE_OFFSET,
			  Data);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Writes local CC configuration to the shadow (NEXT) registers and triggers
* copying from shadow to operational registers.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    CCCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeOfdm_SetNextCCCfgAndTrigger(const XDfeOfdm *InstancePtr,
				    XDfeOfdm_CCCfg *CCCfg)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);

	/* If add is successful update next configuration and trigger update */
	XDfeOfdm_SetNextCCCfg(InstancePtr, CCCfg);
	return XDfeOfdm_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Adds specified CCID, with specified configuration.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    CCID CC ID.
* @param    CCSeqBitmap - up to 16 defined slots into which a CC can be
*           allocated. The number of slots can be from 1 to 16 depending on
*           system initialization. The number of slots is defined by the
*           "sequence length" parameter which is provided during initialization.
*           The Bit offset within the CCSeqBitmap indicates the equivalent
*           Slot number to allocate. e.g. 0x0003  means the caller wants the
*           passed component carrier (CC) to be allocated to slots 0 and 1.
* @param    CarrierCfg CC configuration container.
* @param    FTSeq Fourier transform sequence container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeOfdm_ClearEventStatus() before
*           running this API.
*
* @attention:  This API is deprecated in the release 2023.2. Source code will
*              be removed from in the release 2024.1 release. The functionality
*              of this API can be reproduced with the following API sequence:
*                  XDfeOfdm_GetCurrentCCCfg(InstancePtr, CCCfg);
*                  XDfeOfdm_AddCCtoCCCfg(InstancePtr, CCCfg, CCID, CCSeqBitmap,
*                      CarrierCfg, FTseq);
*                  XDfeOfdm_SetNextCCCfgAndTrigger(InstancePtr, CCCfg);
*
****************************************************************************/
u32 XDfeOfdm_AddCC(XDfeOfdm *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		   const XDfeOfdm_CarrierCfg *CarrierCfg,
		   XDfeOfdm_FTSequence *FTSeq)
{
	XDfeOfdm_CCCfg CCCfg;
	u32 AddSuccess;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEOFDM_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEOFDM_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(CarrierCfg->Numerology <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_960kHz);
	Xil_AssertNonvoid((CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_1024) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_2048) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_4096));
	Xil_AssertNonvoid(
		CarrierCfg->NumSubcarriers <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH));
	Xil_AssertNonvoid(
		CarrierCfg->ScaleFactor <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH));
	Xil_AssertNonvoid(CarrierCfg->CommsStandard <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE);
	Xil_AssertNonvoid(
		CarrierCfg->OutputDelay <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION2_OUTPUT_DELAY_WIDTH));

	/* Numerologu should be set to 15kHz only for LTE standard */
	if ((CarrierCfg->CommsStandard ==
	     XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE) &&
	    (CarrierCfg->Numerology !=
	     XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_15kHz)) {
		metal_log(METAL_LOG_ERROR,
			  "Error, Numerology = %d in LTE mode in %s\n",
			  CarrierCfg->Numerology, __func__);
		return XST_FAILURE;
	}

	/* Read current CC configuration. Note that XDfeOfdm_Initialise writes
	   a NULL CC sequence to H/W */
	XDfeOfdm_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeOfdm_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg.CCSequence);
	if (AddSuccess == (u32)XST_SUCCESS) {
		/* Update carrier configuration, mark flush as we need to clear
		   data registers */
		CCCfg.CarrierCfg[CCID].Enable =
			XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_ENABLED;
		CCCfg.CarrierCfg[CCID].Numerology = CarrierCfg->Numerology;
		CCCfg.CarrierCfg[CCID].FftSize = log2(CarrierCfg->FftSize);
		CCCfg.CarrierCfg[CCID].NumSubcarriers =
			CarrierCfg->NumSubcarriers;
		CCCfg.CarrierCfg[CCID].ScaleFactor = CarrierCfg->ScaleFactor;
		CCCfg.CarrierCfg[CCID].CommsStandard =
			CarrierCfg->CommsStandard;
		CCCfg.CarrierCfg[CCID].OutputDelay = CarrierCfg->OutputDelay;
	} else {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Check FT sequence length */
	if (XST_FAILURE ==
	    XDfeOfdm_CheckFTSequenceLength(&CCCfg, FTSeq->Length)) {
		metal_log(METAL_LOG_ERROR, "Wrong FT sequence length in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Set FT sequence */
	XDfeOfdm_SetNextFTSequence(&CCCfg, FTSeq);

	/* If add is successful update next configuration and trigger update */
	XDfeOfdm_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeOfdm_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Removes specified CCID.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    CCID CC ID.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeOfdm_ClearEventStatus() before
*           running this API.
*
* @attention:  This API is deprecated in the release 2023.2. Source code will
*              be removed from in the release 2024.1 release. The functionality
*              of this API can be reproduced with the following API sequence:
*                  XDfeOfdm_GetCurrentCCCfg(InstancePtr, CCCfg);
*                  XDfeOfdm_RemoveCCfromCCCfg(InstancePtr, CCCfg, CCID, FTseq);
*                  XDfeOfdm_SetNextCCCfgAndTrigger(InstancePtr, CCCfg);
*
****************************************************************************/
u32 XDfeOfdm_RemoveCC(XDfeOfdm *InstancePtr, s32 CCID,
		      XDfeOfdm_FTSequence *FTSeq)
{
	XDfeOfdm_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEOFDM_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEOFDM_CC_NUM);

	/* Read current CC configuration */
	XDfeOfdm_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeOfdm_RemoveCCID(InstancePtr, CCID, &CCCfg.CCSequence);
	CCCfg.CarrierCfg[CCID].Enable =
		XDFEOFDM_CARRIER_CONFIGURATION1_ENABLE_DISABLED;

	/* Check FT sequence length */
	if (XST_FAILURE ==
	    XDfeOfdm_CheckFTSequenceLength(&CCCfg, FTSeq->Length)) {
		metal_log(METAL_LOG_ERROR, "Wrong FT sequence length in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Set FT sequence */
	XDfeOfdm_SetNextFTSequence(&CCCfg, FTSeq);

	/* Update next configuration and trigger update */
	XDfeOfdm_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeOfdm_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Updates specified CCID carrier configuration; change gain or filter
* coefficients set.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    CCID CC ID.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeOfdm_ClearEventStatus() before
*           running this API.
*
* @attention:  This API is deprecated in the release 2023.2. Source code will
*              be removed from in the release 2024.1 release. The functionality
*              of this API can be reproduced with the following API sequence:
*                  XDfeOfdm_GetCurrentCCCfg(InstancePtr, CCCfg);
*                  XDfeOfdm_UpdateCCinCCCfg(InstancePtr, CCCfg, CCID,
*                      CarrierCfg, FTseq);
*                  XDfeOfdm_SetNextCCCfgAndTrigger(InstancePtr, CCCfg);
*
****************************************************************************/
u32 XDfeOfdm_UpdateCC(XDfeOfdm *InstancePtr, s32 CCID,
		      const XDfeOfdm_CarrierCfg *CarrierCfg,
		      XDfeOfdm_FTSequence *FTSeq)
{
	XDfeOfdm_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEOFDM_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEOFDM_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(CarrierCfg->Numerology <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_960kHz);
	Xil_AssertNonvoid((CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_1024) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_2048) ||
			  (CarrierCfg->FftSize == XDFEOFDM_FFT_SIZE_4096));
	Xil_AssertNonvoid(
		CarrierCfg->NumSubcarriers <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_NUM_SUBCARRIERS_WIDTH));
	Xil_AssertNonvoid(
		CarrierCfg->ScaleFactor <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION1_SCALE_FACTOR_WIDTH));
	Xil_AssertNonvoid(CarrierCfg->CommsStandard <=
			  XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE);
	Xil_AssertNonvoid(
		CarrierCfg->OutputDelay <
		(1 << XDFEOFDM_CARRIER_CONFIGURATION2_OUTPUT_DELAY_WIDTH));

	/* Numerologu should be set to 15kHz only for LTE standard */
	if ((CarrierCfg->CommsStandard ==
	     XDFEOFDM_CARRIER_CONFIGURATION1_COMMS_STANDARD_LTE) &&
	    (CarrierCfg->Numerology !=
	     XDFEOFDM_CARRIER_CONFIGURATION1_NUMEROLOGY_15kHz)) {
		metal_log(METAL_LOG_ERROR,
			  "Error, Numerology = %d in LTE mode in %s\n",
			  CarrierCfg->Numerology, __func__);
		return XST_FAILURE;
	}

	/* Read current CC configuration */
	XDfeOfdm_GetCurrentCCCfg(InstancePtr, &CCCfg);

	CCCfg.CarrierCfg[CCID].Numerology = CarrierCfg->Numerology;
	CCCfg.CarrierCfg[CCID].FftSize = log2(CarrierCfg->FftSize);
	CCCfg.CarrierCfg[CCID].NumSubcarriers = CarrierCfg->NumSubcarriers;
	CCCfg.CarrierCfg[CCID].ScaleFactor = CarrierCfg->ScaleFactor;
	CCCfg.CarrierCfg[CCID].CommsStandard = CarrierCfg->CommsStandard;
	CCCfg.CarrierCfg[CCID].OutputDelay = CarrierCfg->OutputDelay;

	/* Check FT sequence length */
	if (XST_FAILURE ==
	    XDfeOfdm_CheckFTSequenceLength(&CCCfg, FTSeq->Length)) {
		metal_log(METAL_LOG_ERROR, "Wrong FT sequence length in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Set FT sequence */
	XDfeOfdm_SetNextFTSequence(&CCCfg, FTSeq);

	/* Update next configuration and trigger update */
	XDfeOfdm_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeOfdm_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Returns current trigger configuration.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfeOfdm_GetTriggersCfg(const XDfeOfdm *InstancePtr,
			     XDfeOfdm_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEOFDM_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read ACTIVATE triggers */
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET);
	TriggerCfg->Activate.TriggerEnable =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Val);
	TriggerCfg->Activate.Mode =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_MODE_WIDTH,
				    XDFEOFDM_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TUSER_BIT_WIDTH,
				    XDFEOFDM_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.TuserEdgeLevel =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				    XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				    Val);
	TriggerCfg->Activate.StateOutput =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				    XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read LOW_POWER triggers */
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET);
	TriggerCfg->LowPower.TriggerEnable =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Val);
	TriggerCfg->LowPower.Mode =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_MODE_WIDTH,
				    XDFEOFDM_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TUSER_BIT_WIDTH,
				    XDFEOFDM_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.TuserEdgeLevel =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				    XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				    Val);
	TriggerCfg->LowPower.StateOutput =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				    XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read CC_UPDATE triggers */
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_CC_UPDATE_OFFSET);
	TriggerCfg->CCUpdate.TriggerEnable =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Val);
	TriggerCfg->CCUpdate.Mode =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_MODE_WIDTH,
				    XDFEOFDM_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->CCUpdate.TUSERBit =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TUSER_BIT_WIDTH,
				    XDFEOFDM_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->CCUpdate.TuserEdgeLevel =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				    XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				    Val);
	TriggerCfg->CCUpdate.StateOutput =
		XDfeOfdm_RdBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				    XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets trigger configuration.
*
* @param    InstancePtr Pointer to the Ofdm instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfeOfdm_SetTriggersCfg(const XDfeOfdm *InstancePtr,
			     XDfeOfdm_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEOFDM_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);
	Xil_AssertVoid(TriggerCfg->CCUpdate.Mode !=
		       XDFEOFDM_TRIGGERS_MODE_TUSER_CONTINUOUS);

	/* Write public trigger configuration members and ensure private members
	  (TriggerEnable & Immediate) are set appropriately */

	/* Activate defined as Single Shot/Immediate (as per the programming model) */
	TriggerCfg->Activate.TriggerEnable =
		XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->Activate.StateOutput =
		XDFEOFDM_TRIGGERS_STATE_OUTPUT_ENABLED;
	/* Read/set/write ACTIVATE triggers */
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				  TriggerCfg->Activate.TriggerEnable);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_MODE_WIDTH,
				  XDFEOFDM_TRIGGERS_MODE_OFFSET, Val,
				  TriggerCfg->Activate.Mode);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				  XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				  Val, TriggerCfg->Activate.TuserEdgeLevel);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TUSER_BIT_WIDTH,
				  XDFEOFDM_TRIGGERS_TUSER_BIT_OFFSET, Val,
				  TriggerCfg->Activate.TUSERBit);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				  TriggerCfg->Activate.StateOutput);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_ACTIVATE_OFFSET, Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.TriggerEnable =
		XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->LowPower.Mode = XDFEOFDM_TRIGGERS_MODE_TUSER_CONTINUOUS;
	/* Read LOW_POWER triggers */
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				  TriggerCfg->LowPower.TriggerEnable);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_MODE_WIDTH,
				  XDFEOFDM_TRIGGERS_MODE_OFFSET, Val,
				  TriggerCfg->LowPower.Mode);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				  XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				  Val, TriggerCfg->LowPower.TuserEdgeLevel);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TUSER_BIT_WIDTH,
				  XDFEOFDM_TRIGGERS_TUSER_BIT_OFFSET, Val,
				  TriggerCfg->LowPower.TUSERBit);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				  TriggerCfg->LowPower.StateOutput);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_LOW_POWER_OFFSET, Val);

	/* CCUpdate defined as Single Shot/Immediate */
	TriggerCfg->CCUpdate.TriggerEnable =
		XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->CCUpdate.StateOutput =
		XDFEOFDM_TRIGGERS_STATE_OUTPUT_ENABLED;
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_TRIGGERS_CC_UPDATE_OFFSET);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEOFDM_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				  TriggerCfg->CCUpdate.TriggerEnable);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_MODE_WIDTH,
				  XDFEOFDM_TRIGGERS_MODE_OFFSET, Val,
				  TriggerCfg->CCUpdate.Mode);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				  XDFEOFDM_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				  Val, TriggerCfg->CCUpdate.TuserEdgeLevel);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_TUSER_BIT_WIDTH,
				  XDFEOFDM_TRIGGERS_TUSER_BIT_OFFSET, Val,
				  TriggerCfg->CCUpdate.TUSERBit);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEOFDM_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				  TriggerCfg->CCUpdate.StateOutput);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TRIGGERS_CC_UPDATE_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets TUSER Framing bit Location register where bit location indicates which
* bit to be used for sending framing information on DL_DOUT IF and
* M_AXIS_TBASE IF.
* TUSER bit width is fixed to its default value of 8. Therefore, legal values
* of FRAME_BIT are 0 to 7.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    TuserOutFrameLocation Requested TUSER OutFrame Location.
*
****************************************************************************/
void XDfeOfdm_SetTuserOutFrameLocation(const XDfeOfdm *InstancePtr,
				       u32 TuserOutFrameLocation)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TuserOutFrameLocation <
		       (1 << XDFEOFDM_TUSER_OUTFRAME_LOCATION_BF_WIDTH));
	Xil_AssertVoid(InstancePtr->StateId == XDFEOFDM_STATE_OPERATIONAL);

	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_TUSER_OUTFRAME_LOCATION_OFFSET,
			  TuserOutFrameLocation);
}

/****************************************************************************/
/**
*
* Gets TUSER Framing bit Location register where bit location indicates which
* bit to be used for sending framing information on DL_DOUT IF and
* M_AXIS_TBASE IF.
* TUSER bit width is fixed to its default value of 8. Therefore, legal values
* of FRAME_BIT are 0 to 7.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
* @return   TUSER OutFrame Location
*
****************************************************************************/
u32 XDfeOfdm_GetTuserOutFrameLocation(const XDfeOfdm *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfeOfdm_RdRegBitField(
		InstancePtr, XDFEOFDM_TUSER_OUTFRAME_LOCATION_OFFSET,
		XDFEOFDM_TUSER_OUTFRAME_LOCATION_BF_WIDTH,
		XDFEOFDM_TUSER_OUTFRAME_LOCATION_BF_OFFSET);
}

/****************************************************************************/
/**
*
* Sets the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    Delay Requested delay variable.
*
****************************************************************************/
void XDfeOfdm_SetTUserDelay(const XDfeOfdm *InstancePtr, u32 Delay)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Delay < (1 << XDFEOFDM_DELAY_BF_WIDTH));
	Xil_AssertVoid(InstancePtr->StateId == XDFEOFDM_STATE_INITIALISED);

	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_DELAY_OFFSET, Delay);
}

/****************************************************************************/
/**
*
* Reads the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the OFDM instance.
*
* @return   Delay value
*
****************************************************************************/
u32 XDfeOfdm_GetTUserDelay(const XDfeOfdm *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfeOfdm_RdRegBitField(InstancePtr, XDFEOFDM_DELAY_OFFSET,
				      XDFEOFDM_DELAY_BF_WIDTH,
				      XDFEOFDM_DELAY_BF_OFFSET);
}

/****************************************************************************/
/**
*
* Returns data latency.
*
* @param    InstancePtr Pointer to the OFDM instance.
*
* @return   Returned Data latency.
*
****************************************************************************/
u32 XDfeOfdm_GetDataLatency(const XDfeOfdm *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfeOfdm_RdRegBitField(InstancePtr, XDFEOFDM_DATA_LATENCY_OFFSET,
				      XDFEOFDM_DATA_LATENCY_BF_WIDTH,
				      XDFEOFDM_DATA_LATENCY_BF_OFFSET);
}

/*****************************************************************************/
/**
*
* This API is used to get the driver version.
*
* @param    SwVersion Driver version numbers.
* @param    HwVersion HW version numbers.
*
******************************************************************************/
void XDfeOfdm_GetVersions(const XDfeOfdm *InstancePtr,
			  XDfeOfdm_Version *SwVersion,
			  XDfeOfdm_Version *HwVersion)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SwVersion != NULL);
	Xil_AssertVoid(HwVersion != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEOFDM_STATE_NOT_READY);

	/* Driver version */
	SwVersion->Major = XDFEOFDM_DRIVER_VERSION_MAJOR;
	SwVersion->Minor = XDFEOFDM_DRIVER_VERSION_MINOR;

	/* Component HW version */
	Version = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_VERSION_OFFSET);
	HwVersion->Patch =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_PATCH_WIDTH,
				    XDFEOFDM_VERSION_PATCH_OFFSET, Version);
	HwVersion->Revision =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_REVISION_WIDTH,
				    XDFEOFDM_VERSION_REVISION_OFFSET, Version);
	HwVersion->Minor =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_MINOR_WIDTH,
				    XDFEOFDM_VERSION_MINOR_OFFSET, Version);
	HwVersion->Major =
		XDfeOfdm_RdBitField(XDFEOFDM_VERSION_MAJOR_WIDTH,
				    XDFEOFDM_VERSION_MAJOR_OFFSET, Version);
}
/** @} */
