/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfexmix.c
* @addtogroup xdfemix_v1_0
* @{
*
* Contains the APIs for DFE Mixer component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/21/20 Initial version
* </pre>
*
******************************************************************************/

#include "xdfemix.h"
#include "xdfemix_hw.h"
#include <math.h>
#include <metal/io.h>
#include <metal/device.h>

#ifdef __BAREMETAL__
#include "sleep.h"
#else
#include <unistd.h>
#endif

/**************************** Macros Definitions ****************************/
#define XDFEMIX_SEQUENCE_ENTRY_NULL 8U /**< Null sequence entry flag */
#define XDFEMIX_CEILING(x, y) (((x) + (y)-1U) / (y)) /**< U32 ceiling */
#define XDFEMIX_NO_EMPTY_CCID_FLAG 0xFFFFU /**< Not Empty CCID flag */
#define XDFEMIX_PHASE_OFFSET_ROUNDING_BITS 14

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFEMIX_MAX_NUM_INSTANCES];
#endif
static struct metal_device *DevicePtrStorage[XDFEMIX_MAX_NUM_INSTANCES];
extern XDfeMix XDfeMix_Mixer[XDFEMIX_MAX_NUM_INSTANCES];

/************************** Function Definitions ****************************/
extern u32 XDfeMix_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr);
extern u32 XDfeMix_LookupConfig(u16 DeviceId);
extern u32 XDfeMix_CfgInitialize(XDfeMix *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Write value to register in a Mixer instance.
*
* @param    InstancePtr is a pointer to the DFE driver instance.
* @param    AddrOffset is address offset relativ to instance base address.
* @param    Data is value to be written.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_WriteReg(const XDfeMix *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Read a value from register from a Mixer instance.
*
* @param    InstancePtr is a pointer to the DFE driver instance.
* @param    AddrOffset is address offset relativ to instance base address.
*
* @return   Register value.
*
* @note     None
*
****************************************************************************/
u32 XDfeMix_ReadReg(const XDfeMix *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, AddrOffset);
}

/****************************************************************************/
/**
*
* Write a bit field value to register.
*
* @param    InstancePtr is a pointer to the XDfeMix instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
* @param    FieldData is a bit field data.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_WrRegBitField(const XDfeMix *InstancePtr, u32 Offset,
			   u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfeMix_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Read a bit field value from register.
*
* @param    InstancePtr is a pointer to the XDfeMix instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
*
* @return   Bit field data.
*
* @note     None
*
****************************************************************************/
u32 XDfeMix_RdRegBitField(const XDfeMix *InstancePtr, u32 Offset,
			  u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, Offset);
	return ((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U));
}

/****************************************************************************/
/**
*
* Reads a bit field value from u32 variable
*
* @param    FieldWidth is a bit field width
* @param    FieldOffset is a bit field offset in bits number
* @param    Data is a u32 data which bit field this function reads
*
* @return   Bit field value
*
* @note     None
*
****************************************************************************/
u32 XDfeMix_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	return (((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U)));
}
/****************************************************************************/
/**
*
* Writes a bit field value to u32 variable
*
* @param    FieldWidth is a bit field width
* @param    FieldOffset is a bit field offset in bits number
* @param    Data is a u32 data which bit field this function reads
* @param    Val is a u32 value to be written in the bit field
*
* @return   Data with a bitfield written
*
* @note     None
*
****************************************************************************/
u32 XDfeMix_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
{
	u32 BitFieldSet;
	u32 BitFieldClear;
	BitFieldSet = (Val & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	BitFieldClear =
		Data & (~((((u32)1U << FieldWidth) - 1U) << FieldOffset));
	return (BitFieldSet | BitFieldClear);
}

/************************ DFE Common functions ******************************/

/****************************************************************************/
/**
*
* Generates a "null"(8) CCID sequence of specified length.
*
* @param    SeqLen is a CC ID
* @param    CCIDSequence is a CC sequence array
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_CreateCCSequence(u32 SeqLen,
				     XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;
	Xil_AssertVoid(SeqLen < XDFEMIX_SEQ_LENGTH_MAX);
	Xil_AssertVoid(CCIDSequence != NULL);

	/* Set sequence length and mark all sequence entries as null (8) */
	CCIDSequence->Length = SeqLen;
	for (Index = 0; Index < XDFEMIX_SEQ_LENGTH_MAX; Index++) {
		CCIDSequence->CCID[Index] = XDFEMIX_SEQUENCE_ENTRY_NULL;
	}
}

/****************************************************************************/
/**
*
* Add the specified CCID, with the given rate, to the CC sequence. If there
* is insufficient capacity for the new CCID return an error. Implements
* a "greedy" allocation of sequence locations.
*
* @param    CCID is a CC ID
* @param    Rate is a Rate for a given CC ID (can be 1,2,4,8,16)
* @param    CCIDSequence is a CC sequence array
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     None
*
****************************************************************************/
static u32 XDfeMix_Add_CCID(u32 CCID, u32 Rate,
			    XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 SeqLen;
	u32 SeqMult = 1;
	u32 CCIDCount;
	u32 EmptyCCID = XDFEMIX_NO_EMPTY_CCID_FLAG;
	Xil_AssertNonvoid(CCIDSequence != NULL);

	if (0 == CCIDSequence->Length) {
		XDfeMix_CreateCCSequence(Rate, CCIDSequence);
	}
	/* Determine if there is space in the sequence for the new CCID,
	   test for an empty slot. */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		/* search for the first "null" */
		if (CCIDSequence->CCID[Index] == XDFEMIX_SEQUENCE_ENTRY_NULL) {
			EmptyCCID = Index;
			break;
		}
	}

	if (EmptyCCID == XDFEMIX_NO_EMPTY_CCID_FLAG) {
		metal_log(METAL_LOG_ERROR, "No space for new sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Determine if we need to extend, and repeat, the existing sequence to
	   represent the new CCID rate. */
	SeqLen = CCIDSequence->Length;
	/* Calculate Rate/SeqLen */
	if (Rate > SeqLen) {
		SeqMult = Rate / SeqLen;
		/* Fail if (Rate/SeqLen) is not an integer */
		if (Rate != SeqMult * SeqLen) {
			metal_log(METAL_LOG_ERROR,
				  "Rate/SeqLen not integer %s\n", __func__);
			return XST_FAILURE;
		}
	}
	if (SeqMult > 1U) {
		/* Rate is greater than the current sequence so extend and
		   repeat the existing sequence */
		if ((SeqMult * SeqLen) > XDFEMIX_SEQ_LENGTH_MAX) {
			metal_log(METAL_LOG_ERROR,
				  "Not enough space for new sequence in %s\n",
				  __func__);
			return XST_FAILURE;
		}
		CCIDSequence->Length = SeqMult * SeqLen;
		for (Index = SeqLen; Index < CCIDSequence->Length; Index++) {
			CCIDSequence->CCID[Index] =
				CCIDSequence->CCID[Index % SeqLen];
		}
	}

	/* Determine if we find a suitable space in the sequence to add this
	   CCID. Starting at the first empty location can we place the
	   appropriate number of CCID entries. */
	SeqLen = CCIDSequence->Length;
	CCIDCount = SeqLen / Rate; /* Number of entries required in this
				      sequence for the CCID */
	for (Index = EmptyCCID; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == XDFEMIX_SEQUENCE_ENTRY_NULL) {
			CCIDCount--;
			if (0 == CCIDCount) {
				break;
			}
		}
	}
	if (CCIDCount != 0U) {
		/* Not placed all required CCID entries */
		metal_log(METAL_LOG_ERROR,
			  "Not placed all required CCID entries in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Everything is OK. Loop again to mark CCID */
	CCIDCount = SeqLen / Rate;
	for (Index = EmptyCCID; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == XDFEMIX_SEQUENCE_ENTRY_NULL) {
			/* Mark location for this CCID */
			CCIDSequence->CCID[Index] = CCID;
			CCIDCount--;
			if (0 == CCIDCount) {
				break;
			}
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Remove the specified CCID from the CC sequence. Will replace the CCID
* entries with null (8).
*
* @param    CCID is a CC ID
* @param    CCIDSequence is a CC sequence array
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_RemoveCCID(u32 CCID, XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;
	Xil_AssertVoid(CCIDSequence != NULL);
	Xil_AssertVoid(CCIDSequence->Length <= XDFEMIX_SEQ_LENGTH_MAX);

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] = XDFEMIX_SEQUENCE_ENTRY_NULL;
		}
	}
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Set antenna gain.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    AntennaId is an antenna ID.
* @param    AntennaGain is an antenna gain.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetAntennaGainL(const XDfeMix *InstancePtr, u32 AntennaId,
				    u32 AntennaGain)
{
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AntennaGain <= 1U);
	Xil_AssertVoid(AntennaId <= XDFEMIX_ANT_NUM_MAX);

	Offset = XDFEMIX_ANTENNA_GAIN_NEXT;
	XDfeMix_WrRegBitField(InstancePtr, Offset,
			      XDFEMIX_ONE_ANTENNA_GAIN_ZERODB, AntennaId,
			      AntennaGain);
}

/****************************************************************************/
/**
*
* Set CCCfg.DUCDDCCfg[CCID] with Rate and NCO.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCCfg is a configuration data container.
* @param    CCID is a Channel ID.
* @param    Rate is a NCO rate value.
* @param    NCO is a Channel ID.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetCCDDC(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			     u32 CCID, u32 Rate, u32 NCO)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID < XDFEMIX_CC_NUM);
	Xil_AssertVoid(Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertVoid(NCO <= XDFEMIX_NCO_MAX);

	CCCfg->DUCDDCCfg[CCID].NCO = NCO;
	CCCfg->DUCDDCCfg[CCID].Rate = Rate;
}

/****************************************************************************/
/**
*
* Write NEXT CC configuration from CCCfg.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCCfg is a configuration data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetCCCfg(const XDfeMix *InstancePtr,
			     const XDfeMix_CCCfg *CCCfg)
{
	u32 Data, Offset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	Offset = XDFEMIX_SEQUENCE_LENGTH_NEXT;
	XDfeMix_WrRegBitField(InstancePtr, Offset,
			      XDFEMIX_SEQUENCE_LENGTH_WIDTH,
			      XDFEMIX_SEQUENCE_LENGTH_OFFSET,
			      CCCfg->Sequence.Length);

	for (Index = 0; Index < XDFEMIX_CCID_SEQUENCE_SIZE; Index++) {
		Offset = XDFEMIX_CCID_SEQUENCE_NEXT + ((Index * sizeof(u32)));
		XDfeMix_WrRegBitField(InstancePtr, Offset,
				      XDFEMIX_CCID_SEQUENCE_WIDTH,
				      XDFEMIX_CCID_SEQUENCE_OFFSET,
				      CCCfg->Sequence.CCID[Index]);
	}

	for (Index = 0; Index < XDFEMIX_DUC_DDC_MAPPING_SIZE; Index++) {
		Offset = XDFEMIX_DUC_DDC_MAPPING_NEXT + ((Index * sizeof(u32)));
		Data = XDfeMix_ReadReg(InstancePtr, Offset);
		Data = XDfeMix_WrBitField(XDFEMIX_DUC_DDC_MAPPING_NCO_WIDTH,
					  XDFEMIX_DUC_DDC_MAPPING_NCO_OFFSET,
					  Data, CCCfg->DUCDDCCfg[Index].NCO);
		Data = XDfeMix_WrBitField(XDFEMIX_DUC_DDC_MAPPING_RATE_WIDTH,
					  XDFEMIX_DUC_DDC_MAPPING_RATE_OFFSET,
					  Data, CCCfg->DUCDDCCfg[Index].Rate);
		XDfeMix_WriteReg(InstancePtr, Offset, Data);
	}

	Offset = XDFEMIX_ANTENNA_GAIN_NEXT;
	Data = XDfeMix_ReadReg(InstancePtr, Offset);
	for (Index = 0; Index < InstancePtr->Config.NumAntenna; Index++) {
		Data = XDfeMix_WrBitField(XDFEMIX_ONE_ANTENNA_GAIN_WIDTH, Index,
					  Data, CCCfg->AntennaCfg[Index]);
	}
	XDfeMix_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Populate CCCfg by reading either NEXT or CURRENT CC configuration registers.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCCfg is a configuration data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_GetCCCfg(const XDfeMix *InstancePtr, bool Next,
			     XDfeMix_CCCfg *CCCfg)
{
	u32 Data;
	u32 Offset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	if (Next == true) {
		Offset = XDFEMIX_SEQUENCE_LENGTH_NEXT;
		Data = XDfeMix_ReadReg(InstancePtr, Offset);
		CCCfg->Sequence.Length =
			XDfeMix_RdBitField(XDFEMIX_SEQUENCE_LENGTH_WIDTH,
					   XDFEMIX_SEQUENCE_LENGTH_OFFSET,
					   Data);
		for (Index = 0; Index < XDFEMIX_CCID_SEQUENCE_SIZE; Index++) {
			Offset = XDFEMIX_CCID_SEQUENCE_NEXT +
				 ((Index * sizeof(u32)));
			Data = XDfeMix_ReadReg(InstancePtr, Offset);
			CCCfg->Sequence.CCID[Index] =
				XDfeMix_RdBitField(XDFEMIX_CCID_SEQUENCE_WIDTH,
						   XDFEMIX_CCID_SEQUENCE_OFFSET,
						   Data);
		}

		for (Index = 0; Index < XDFEMIX_DUC_DDC_MAPPING_SIZE; Index++) {
			Offset = XDFEMIX_DUC_DDC_MAPPING_NEXT +
				 ((Index * sizeof(u32)));
			Data = XDfeMix_ReadReg(InstancePtr, Offset);
			CCCfg->DUCDDCCfg[Index].NCO = XDfeMix_RdBitField(
				XDFEMIX_DUC_DDC_MAPPING_NCO_WIDTH,
				XDFEMIX_DUC_DDC_MAPPING_NCO_OFFSET, Data);
			CCCfg->DUCDDCCfg[Index].Rate = XDfeMix_RdBitField(
				XDFEMIX_DUC_DDC_MAPPING_RATE_WIDTH,
				XDFEMIX_DUC_DDC_MAPPING_RATE_OFFSET, Data);
		}

		Offset = XDFEMIX_ANTENNA_GAIN_NEXT;
		Data = XDfeMix_ReadReg(InstancePtr, Offset);
		for (Index = 0; Index < InstancePtr->Config.NumAntenna;
		     Index++) {
			CCCfg->AntennaCfg[Index] = XDfeMix_RdBitField(
				XDFEMIX_ONE_ANTENNA_GAIN_WIDTH, Index, Data);
		}
	} else {
		Offset = XDFEMIX_SEQUENCE_LENGTH_CURRENT;
		Data = XDfeMix_ReadReg(InstancePtr, Offset);
		CCCfg->Sequence.Length =
			XDfeMix_RdBitField(XDFEMIX_SEQUENCE_LENGTH_WIDTH,
					   XDFEMIX_SEQUENCE_LENGTH_OFFSET,
					   Data);
		for (Index = 0; Index < XDFEMIX_CCID_SEQUENCE_SIZE; Index++) {
			Offset = XDFEMIX_CCID_SEQUENCE_CURRENT +
				 ((Index * sizeof(u32)));
			Data = XDfeMix_ReadReg(InstancePtr, Offset);
			CCCfg->Sequence.CCID[Index] =
				XDfeMix_RdBitField(XDFEMIX_CCID_SEQUENCE_WIDTH,
						   XDFEMIX_CCID_SEQUENCE_OFFSET,
						   Data);
		}

		for (Index = 0; Index < XDFEMIX_DUC_DDC_MAPPING_SIZE; Index++) {
			Offset = XDFEMIX_DUC_DDC_MAPPING_CURRENT +
				 (Index * sizeof(u32));
			Data = XDfeMix_ReadReg(InstancePtr, Offset);
			CCCfg->DUCDDCCfg[Index].NCO = XDfeMix_RdBitField(
				XDFEMIX_DUC_DDC_MAPPING_NCO_WIDTH,
				XDFEMIX_DUC_DDC_MAPPING_NCO_OFFSET, Data);
			CCCfg->DUCDDCCfg[Index].Rate = XDfeMix_RdBitField(
				XDFEMIX_DUC_DDC_MAPPING_RATE_WIDTH,
				XDFEMIX_DUC_DDC_MAPPING_RATE_OFFSET, Data);
		}

		Offset = XDFEMIX_ANTENNA_GAIN_CURRENT;
		Data = XDfeMix_ReadReg(InstancePtr, Offset);
		for (Index = 0; Index < InstancePtr->Config.NumAntenna;
		     Index++) {
			CCCfg->AntennaCfg[Index] = XDfeMix_RdBitField(
				XDFEMIX_ONE_ANTENNA_GAIN_WIDTH, Index, Data);
		}
	}
}

/****************************************************************************/
/**
*
* Write the frequency settings for a given CC's phase accumulator.
* The frequency settings for a given CC are shared across all antennas.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    Freq is a frequencu setting for CC.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetMIXrequency(const XDfeMix *InstancePtr, bool Next,
				   u32 CCID, const XDfeMix_Frequency *Freq)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Freq != NULL);

	if (Next == true) {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_CONTROL_WORD_NEXT + Index,
				 Freq->FrequencyControlWord);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_SINGLE_MOD_COUNT_NEXT + Index,
				 Freq->SingleModCount);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_DUAL_MOD_COUNT_NEXT + Index,
				 Freq->DualModCount);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_PHASE_OFFSET_NEXT + Index,
				 Freq->PhaseOffset.PhaseOffset);

		XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_UPDATE_NEXT + Index,
				 1U);
	} else {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_CONTROL_WORD_CURRENT + Index,
				 Freq->FrequencyControlWord);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_SINGLE_MOD_COUNT_CURRENT + Index,
				 Freq->SingleModCount);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_DUAL_MOD_COUNT_CURRENT + Index,
				 Freq->DualModCount);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_PHASE_OFFSET_CURRENT + Index,
				 Freq->PhaseOffset.PhaseOffset);

		/* A write of any value will cause an update of the associated
		   phase accumulator from the FREQUENCY registers. */
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_FREQ_UPDATE_CURRENT + Index, 1);
	}
}

/****************************************************************************/
/**
*
* Read back frequency for particular CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    Freq is a frequencu setting for CC.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_GetMIXrequency(const XDfeMix *InstancePtr, bool Next,
				   u32 CCID, XDfeMix_Frequency *Freq)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Freq != NULL);

	if (Next == true) {
		Freq->FrequencyControlWord = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_FREQ_CONTROL_WORD_NEXT + Index);
		Freq->SingleModCount = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_FREQ_SINGLE_MOD_COUNT_NEXT + Index);
		Freq->DualModCount = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_FREQ_DUAL_MOD_COUNT_NEXT + Index);
		Freq->PhaseOffset.PhaseOffset = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_FREQ_PHASE_OFFSET_NEXT + Index);
	} else {
		Freq->FrequencyControlWord = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_FREQ_CONTROL_WORD_CURRENT + Index);
		Freq->SingleModCount = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_FREQ_SINGLE_MOD_COUNT_CURRENT + Index);
		Freq->DualModCount = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_FREQ_DUAL_MOD_COUNT_CURRENT + Index);
		Freq->PhaseOffset.PhaseOffset = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_FREQ_PHASE_OFFSET_CURRENT + Index);
	}
}

/****************************************************************************/
/**
*
* Write the phase settings for a given CC's phase accumulator. The frequency
* settings for a given CC are shared across all antennas.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    Phase is a phase setting for CC.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetCCPhase(const XDfeMix *InstancePtr, bool Next, u32 CCID,
			       const XDfeMix_Phase *Phase)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Phase != NULL);

	if (Next == true) {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_ACC_NEXT + Index,
				 Phase->PhaseAcc);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_DUAL_MOD_COUNT_NEXT +
					 Index,
				 Phase->DualModCount);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_DUAL_MOD_SEL_NEXT + Index,
				 Phase->DualModSel);
		XDfeMix_WriteReg(InstancePtr, XDFEMIX_PHASE_UPDATE_NEXT + Index,
				 1);
	} else {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_ACC_CURRENT + Index,
				 Phase->PhaseAcc);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_DUAL_MOD_COUNT_CURRENT +
					 Index,
				 Phase->DualModCount);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_DUAL_MOD_SEL_CURRENT +
					 Index,
				 Phase->DualModSel);
		/* A write of any value will cause an update of the associated
		   phase accumulator from the PHASE registers. */
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_UPDATE_CURRENT + Index, 1);
	}
}

/****************************************************************************/
/**
*
* Read back phase from AXI-lite registers for particular CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    Phase is a phase setting for CC.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_GetCCPhase(const XDfeMix *InstancePtr, bool Next, u32 CCID,
			       XDfeMix_Phase *Phase)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Phase != NULL);

	if (Next == true) {
		Phase->PhaseAcc = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_PHASE_CAPTURE_ACC_NEXT + Index);
		Phase->DualModCount = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_PHASE_CAPTURE_DUAL_MOD_COUNT_NEXT + Index);
		Phase->DualModSel = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_PHASE_CAPTURE_DUAL_MOD_SEL_NEXT + Index);
	} else {
		Phase->PhaseAcc = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_PHASE_CAPTURE_ACC_CURRENT + Index);
		Phase->DualModCount = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_PHASE_CAPTURE_DUAL_MOD_COUNT_CURRENT + Index);
		Phase->DualModSel = XDfeMix_ReadReg(
			InstancePtr,
			XDFEMIX_PHASE_CAPTURE_DUAL_MOD_SEL_CURRENT + Index);
	}
}

/****************************************************************************/
/**
*
* Enables the phase accumulator for a particular CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    Enable is a flag which enables a phase accumulator.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetCCPhaseAccumEnable(const XDfeMix *InstancePtr, bool Next,
					  u32 CCID, bool Enable)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;
	u32 d = 0U;

	Xil_AssertVoid(InstancePtr != NULL);

	if (Enable == true) {
		d = 1U;
	}
	if (Next == true) {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_ACC_ENABLE_NEXT + Index, d);
	} else {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_PHASE_ACC_ENABLE_CURRENT + Index, d);
	}
}

/****************************************************************************/
/**
*
* Determines if phase accumulator is enabled for particular CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    Enable is a flag which enables a phase accumulator.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_GetCCPhaseAccumEnable(const XDfeMix *InstancePtr, bool Next,
					  u32 CCID, bool *Enable)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	if (Next == true) {
		Data = XDfeMix_ReadReg(InstancePtr,
				       XDFEMIX_PHASE_ACC_ENABLE_NEXT + Index);
	} else {
		Data = XDfeMix_ReadReg(
			InstancePtr, XDFEMIX_PHASE_ACC_ENABLE_CURRENT + Index);
	}
	if (Data == 1U) {
		*Enable = true;
	} else {
		*Enable = false;
	}
}

/****************************************************************************/
/**
*
* Captures phase for all phase accumulators in associated AXI-lite registers.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_CapturePhase(const XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDfeMix_WriteReg(InstancePtr, XDFEMIX_MIXER_PHASE_CAPTURE,
			 XDFEMIX_MIXER_PHASE_CAPTURE_ON);
}

/****************************************************************************/
/**
*
* Subtracts phase B from phase A to give phase D. These consider full phase
* configurations, so difference will be as accurate as possible.
*
* The delta in phase accumulator is given by:
* PhaseAccDiff = PhaseB.PhaseAcc - PhaseA.PhaseAcc
* Phase offset is only a 18 bit quantity, so is obtained from PhaseAcc by
* dividing by 2^14. Rounding before converting back to an unsigned integer
* will provide better accuracy.
* Note that PhaseAcc can also be interpreted as an unsigned quantity with
* the difference causing a wrap-around a full cycle to give a positive phase
* when otherwise a negative number would be generated.

* @param    InstancePtr is a pointer to the Mixer instance.
* @param    PhaseA is a phase A descriptor.
* @param    PhaseB is a phase B descriptor.
* @param    PhaseOffset is a phase offset descriptor.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_DerivePhaseOffset(const XDfeMix *InstancePtr,
				      const XDfeMix_Phase *PhaseA,
				      const XDfeMix_Phase *PhaseB,
				      XDfeMix_PhaseOffset *PhaseOffset)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PhaseA != NULL);
	Xil_AssertVoid(PhaseB != NULL);
	Xil_AssertVoid(PhaseOffset != NULL);
	u32 PhaseAccDiff;

	PhaseAccDiff = PhaseB->PhaseAcc - PhaseA->PhaseAcc;
	PhaseOffset->PhaseOffset =
		PhaseAccDiff >> XDFEMIX_PHASE_OFFSET_ROUNDING_BITS;
	/* Add 1 if bit 13 = 1 which means that rounding is greater or equal
	   than (2^14)/2 */
	if (PhaseAccDiff & (1 << (XDFEMIX_PHASE_OFFSET_ROUNDING_BITS - 1))) {
		PhaseOffset->PhaseOffset += 1;
	}
}

/****************************************************************************/
/**
*
* Set phase offset component of frequency.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Frequency is a frequency container.
* @param    PhaseOffset is a phase offset container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetPhaseOffset(const XDfeMix *InstancePtr,
				   XDfeMix_Frequency *Frequency,
				   const XDfeMix_PhaseOffset *PhaseOffset)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Frequency != NULL);
	Xil_AssertVoid(PhaseOffset != NULL);

	Frequency->PhaseOffset = *PhaseOffset;
}

/****************************************************************************/
/**
*
* Set NCO output attenuation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
* @param    NCOGain is a NCO attenuation.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetNCOGain(const XDfeMix *InstancePtr, bool Next, u32 CCID,
			       u32 NCOGain)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;

	Xil_AssertVoid(InstancePtr != NULL);

	if (Next == true) {
		XDfeMix_WriteReg(InstancePtr, XDFEMIX_NCO_GAIN_NEXT + Index,
				 NCOGain);
	} else {
		XDfeMix_WriteReg(InstancePtr, XDFEMIX_NCO_GAIN_CURRENT + Index,
				 NCOGain);
	}
}
/****************************************************************************/
/**
*
* Get NCO output attenuation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
*
* @return   NCO attenuation.
*
* @note     None
*
****************************************************************************/
static u32 XDfeMix_GetNCOGain(const XDfeMix *InstancePtr, bool Next, u32 CCID)
{
	u32 Index = CCID * XDFEMIX_PHAC_CCID_ADDR_STEP;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if (Next == true) {
		return XDfeMix_ReadReg(InstancePtr,
				       XDFEMIX_NCO_GAIN_NEXT + Index);
	}
	return XDfeMix_ReadReg(InstancePtr, XDFEMIX_NCO_GAIN_CURRENT + Index);
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API Initialise one instancies of a Mixer driver.
*
* @param    DeviceId contains the index number of the device instance,
*
* @return
*           - pointer to instance if successful.
*           - NULL if error occurs.
*
* @note     None.
*
******************************************************************************/
XDfeMix *XDfeMix_InstanceInit(u16 DeviceId)
{
	static u32 XDfeMix_DriverHasBeenRegisteredOnce = 0U;
	u32 Index;

	Xil_AssertNonvoid(DeviceId < XDFEMIX_MAX_NUM_INSTANCES);

	/* Is for the First initialisation caled ever */
	if (0U == XDfeMix_DriverHasBeenRegisteredOnce) {
		/* Set up environment environment */
		for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
			XDfeMix_Mixer[Index].StateId = XDFEMIX_STATE_NOT_READY;
#ifdef __BAREMETAL__
			DevicePtrStorage[Index] = &CustomDevice[Index];
#endif
		}
		XDfeMix_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check is the instance DeviceID already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	if (XDfeMix_Mixer[DeviceId].StateId != XDFEMIX_STATE_NOT_READY) {
		return &XDfeMix_Mixer[DeviceId];
	}

	/* Register libmetal for this OS process */
	if (XST_SUCCESS !=
	    XDfeMix_RegisterMetal(DeviceId, &DevicePtrStorage[DeviceId])) {
		XDfeMix_Mixer[DeviceId].StateId = XDFEMIX_STATE_NOT_READY;
		return NULL;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeMix_LookupConfig(DeviceId)) {
		XDfeMix_Mixer[DeviceId].StateId = XDFEMIX_STATE_NOT_READY;
		return NULL;
	}

	/* Configure HW and the driver instance */
	XDfeMix_CfgInitialize(&XDfeMix_Mixer[DeviceId]);

	XDfeMix_Mixer[DeviceId].StateId = XDFEMIX_STATE_READY;

	return &XDfeMix_Mixer[DeviceId];
}

/*****************************************************************************/
/**
*
* API Close the instancies of a Mixer driver.
*
* @param    InstancePtr is a pointer to the XDfeMix instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XDfeMix_InstanceClose(XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Close the instance */
	InstancePtr->StateId = XDFEMIX_STATE_NOT_READY;

	/* Release libmetal */
	metal_device_close(InstancePtr->Device);

	return;
}

/****************************************************************************/
/**
*
* Reset and put block into a reset state.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
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
* Read configuration from device tree/xparameters.h and IP registers.
* S/W reset removed.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Cfg is a configuration data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_Configure(XDfeMix *InstancePtr, XDfeMix_Cfg *Cfg)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_RESET);

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

	/* Copy configs model parameters from InstancePtr */
	Cfg->ModelParams.BypassDDC = InstancePtr->Config.BypassDDC;
	Cfg->ModelParams.BypassMixer = InstancePtr->Config.BypassMixer;
	Cfg->ModelParams.EnableMixIf = InstancePtr->Config.EnableMixIf;
	Cfg->ModelParams.Mode = InstancePtr->Config.Mode;
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.NumCCPerAntenna = InstancePtr->Config.NumCCPerAntenna;
	Cfg->ModelParams.NumSlotChannels = InstancePtr->Config.NumSlotChannels;

	/* Release RESET */
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_RESET_OFFSET, XDFEMIX_RESET_OFF);
	InstancePtr->StateId = XDFEMIX_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE Mixer driver one time initialisation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note		None
*
****************************************************************************/
void XDfeMix_Initialize(XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_CONFIGURED);

	/* TODO */

	InstancePtr->StateId = XDFEMIX_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activate mixer.
* Note: Writting to ACTIVATE reg.toggles between "initialized" and
*       "operational".
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    EnableLowPower is an falg indicateing low power.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeMix_Activate(XDfeMix *InstancePtr, bool EnableLowPower)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);

	/* Enable the Activate trigger and set to one-shot */
	XDfeMix_EnableActivatePowerTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfeMix_EnableLowPowerTrigger(InstancePtr);
	}

	InstancePtr->StateId = XDFEMIX_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* DeActivate mixer.
* Note: Writting to ACTIVATE reg.toggles between "initialized" and
*       "operational".
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeMix_Deactivate(XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	/* Disable LowPower trigger (may not be enabled) */
	XDfeMix_WrRegBitField(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET,
			      XDFEMIX_TRIGGERS_ENABLE_WIDTH,
			      XDFEMIX_TRIGGERS_ENABLE_OFFSET,
			      XDFEMIX_TRIGGERS_ENABLE_DISABLED);

	/* Enable Deactivate trigger and set to one-shot */
	XDfeMix_EnableActivatePowerTrigger(InstancePtr);

	InstancePtr->StateId = XDFEMIX_STATE_INITIALISED;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Add specified CCID, with specified rate, NCO mapping and associated phase
* accumulator configuration.
* If there is insufficient capacity for the new CC the function will return
* an error (non zero value).
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    Rate is a NCO rate value.
* @param    NCO is a Channel ID.
* @param    Freq is a frequencu setting for CC.
* @param    Phase is a phase setting for CC.
* @param    NCOGain is a NCO gain value.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_AddCC(XDfeMix *InstancePtr, u32 CCID, u32 Rate, u32 NCO,
		   XDfeMix_Frequency *Freq, XDfeMix_Phase *Phase, u32 NCOGain)
{
	XDfeMix_CCCfg NextCCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertVoid(Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertVoid(NCO <= XDFEMIX_NCO_MAX);
	Xil_AssertVoid(Freq != NULL);
	Xil_AssertVoid(Phase != NULL);

	XDfeMix_GetCCCfg(InstancePtr, true, &NextCCCfg);
	(void)XDfeMix_Add_CCID(CCID, Rate, &NextCCCfg.Sequence);
	XDfeMix_SetCCDDC(InstancePtr, &NextCCCfg, CCID, Rate, NCO);
	XDfeMix_SetCCCfg(InstancePtr, &NextCCCfg);
	XDfeMix_SetMIXrequency(InstancePtr, true, CCID, Freq);
	XDfeMix_SetCCPhase(InstancePtr, true, CCID, Phase);
	XDfeMix_SetNCOGain(InstancePtr, true, CCID, NCOGain);
	XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Remove specified CCID.
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_RemoveCC(XDfeMix *InstancePtr, u32 CCID)
{
	XDfeMix_CCCfg NextCCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFEMIX_CC_NUM);

	XDfeMix_GetCCCfg(InstancePtr, false, &NextCCCfg);
	XDfeMix_RemoveCCID(CCID, &NextCCCfg.Sequence);
	XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Move specified CCID from one NCO to another aligning phase to make it
* transparrent.
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    Rate is a NCO rate value.
* @param    FromNCO is a NCO value moving from.
* @param    ToNCO is a NCO value moving to.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_MoveCC(XDfeMix *InstancePtr, u32 CCID, u32 Rate, u32 FromNCO,
		    u32 ToNCO)
{
	XDfeMix_CCCfg NextCCCfg;
	XDfeMix_Frequency Freq;
	XDfeMix_Phase PhaseNext;
	XDfeMix_Phase PhaseCurrent;
	XDfeMix_PhaseOffset PhaseOffset;
	XDfeMix_PhaseOffset PhaseDiff = { 0 };
	u32 NCOGain;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertVoid(Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertVoid(FromNCO <= XDFEMIX_NCO_MAX);
	Xil_AssertVoid(ToNCO <= XDFEMIX_NCO_MAX);

	XDfeMix_GetCCCfg(InstancePtr, true, &NextCCCfg);
	XDfeMix_SetCCCfg(InstancePtr, &NextCCCfg);
	NCOGain = XDfeMix_GetNCOGain(InstancePtr, false, CCID);
	XDfeMix_SetNCOGain(InstancePtr, true, CCID, NCOGain);
	XDfeMix_GetMIXrequency(InstancePtr, false, CCID, &Freq);
	XDfeMix_SetMIXrequency(InstancePtr, true, CCID, &Freq);
	XDfeMix_SetCCPhaseAccumEnable(InstancePtr, true, CCID, true);
	XDfeMix_CapturePhase(InstancePtr);
	XDfeMix_GetCCPhase(InstancePtr, false, CCID, &PhaseCurrent);
	XDfeMix_GetCCPhase(InstancePtr, true, CCID, &PhaseNext);
	XDfeMix_DerivePhaseOffset(InstancePtr, &PhaseCurrent, &PhaseNext,
				  &PhaseOffset);
	XDfeMix_SetPhaseOffset(InstancePtr, &Freq, &PhaseDiff);
	XDfeMix_SetMIXrequency(InstancePtr, true, CCID, &Freq);
	XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Placeholder. This would be used to update frequency etc without updating
* sequence. - TODO
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_UpdateCC(const XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
}

/****************************************************************************/
/**
*
* Set antenna gain.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    AntennaId is an antenna ID.
* @param    AntennaGain is an antenna gain.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_SetAntennaGain(XDfeMix *InstancePtr, u32 AntennaId,
			    u32 AntennaGain)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AntennaGain <= 1U);
	Xil_AssertVoid(AntennaId <= XDFEMIX_ANT_NUM_MAX);

	XDfeMix_GetCCCfg(InstancePtr, false, &CCCfg);
	XDfeMix_SetCCCfg(InstancePtr, &CCCfg);
	XDfeMix_SetAntennaGainL(InstancePtr, AntennaId, AntennaGain);
	XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Return current trigger configuration.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    TriggerCfg is a trigger configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_GetTriggers(const XDfeMix *InstancePtr,
			 XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEMIX_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	TriggerCfg->Activate.Enable =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				   XDFEMIX_TRIGGERS_ENABLE_OFFSET, Val);
	TriggerCfg->Activate.Source =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				   XDFEMIX_TRIGGERS_SOURCE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.Edge =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_SIGNAL_EDGE_WIDTH,
				   XDFEMIX_TRIGGERS_SIGNAL_EDGE_OFFSET, Val);
	TriggerCfg->Activate.OneShot =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				   XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Val);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	TriggerCfg->LowPower.Enable =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				   XDFEMIX_TRIGGERS_ENABLE_OFFSET, Val);
	TriggerCfg->LowPower.Source =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				   XDFEMIX_TRIGGERS_SOURCE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.Edge =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_SIGNAL_EDGE_WIDTH,
				   XDFEMIX_TRIGGERS_SIGNAL_EDGE_OFFSET, Val);
	TriggerCfg->LowPower.OneShot =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				   XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Val);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	TriggerCfg->CCUpdate.Enable =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				   XDFEMIX_TRIGGERS_ENABLE_OFFSET, Val);
	TriggerCfg->CCUpdate.Source =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				   XDFEMIX_TRIGGERS_SOURCE_OFFSET, Val);
	TriggerCfg->CCUpdate.TUSERBit =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->CCUpdate.Edge =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_SIGNAL_EDGE_WIDTH,
				   XDFEMIX_TRIGGERS_SIGNAL_EDGE_OFFSET, Val);
	TriggerCfg->CCUpdate.OneShot =
		XDfeMix_RdBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				   XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Set trigger configuration.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    TriggerCfg is a trigger configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_SetTriggers(const XDfeMix *InstancePtr,
			 const XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  TriggerCfg->Activate.Enable);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				  XDFEMIX_TRIGGERS_SOURCE_OFFSET, Data,
				  TriggerCfg->Activate.Source);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				  XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Data,
				  TriggerCfg->Activate.TUSERBit);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SIGNAL_EDGE_WIDTH,
				  XDFEMIX_TRIGGERS_SIGNAL_EDGE_OFFSET, Data,
				  TriggerCfg->Activate.Edge);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  TriggerCfg->Activate.OneShot);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Data);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  TriggerCfg->LowPower.Enable);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				  XDFEMIX_TRIGGERS_SOURCE_OFFSET, Data,
				  TriggerCfg->LowPower.Source);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				  XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Data,
				  TriggerCfg->LowPower.TUSERBit);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SIGNAL_EDGE_WIDTH,
				  XDFEMIX_TRIGGERS_SIGNAL_EDGE_OFFSET, Data,
				  TriggerCfg->LowPower.Edge);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  TriggerCfg->LowPower.OneShot);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Data);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  TriggerCfg->CCUpdate.Enable);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				  XDFEMIX_TRIGGERS_SOURCE_OFFSET, Data,
				  TriggerCfg->CCUpdate.Source);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TUSER_BIT_WIDTH,
				  XDFEMIX_TRIGGERS_TUSER_BIT_OFFSET, Data,
				  TriggerCfg->CCUpdate.TUSERBit);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SIGNAL_EDGE_WIDTH,
				  XDFEMIX_TRIGGERS_SIGNAL_EDGE_OFFSET, Data,
				  TriggerCfg->CCUpdate.Edge);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  TriggerCfg->CCUpdate.OneShot);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Get DUCDDC status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    DUCDDCStatus is a DUCDDC status container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_GetDUCDDCStatus(const XDfeMix *InstancePtr, u32 CCID,
			     XDfeMix_DUCDDCStatus *DUCDDCStatus)
{
	(void)CCID;
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DUCDDCStatus != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_DUC_DDC_STATUS);
	DUCDDCStatus->RealOverflowStage =
		XDfeMix_RdBitField(XDFEMIX_DUC_DDC_STAGE_I_WIDTH,
				   XDFEMIX_DUC_DDC_STAGE_I_OFFSET, Val);
	DUCDDCStatus->ImagOverflowStage =
		XDfeMix_RdBitField(XDFEMIX_DUC_DDC_STAGE_Q_WIDTH,
				   XDFEMIX_DUC_DDC_STAGE_Q_OFFSET, Val);
	DUCDDCStatus->FirstAntennaOverflowing =
		XDfeMix_RdBitField(XDFEMIX_DUC_DDC_ANTENNA_WIDTH,
				   XDFEMIX_DUC_DDC_ANTENNA_OFFSET, Val);
	DUCDDCStatus->FirstCCIDOverflowing = XDfeMix_RdBitField(
		XDFEMIX_DUC_DDC_CCID_WIDTH, XDFEMIX_DUC_DDC_CCID_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Get Mixer status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    DUCDDCStatus is a DUCDDC status container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_GetMixerStatus(const XDfeMix *InstancePtr, u32 CCID,
			    XDfeMix_MixerStatus *MixerStatus)
{
	(void)CCID;
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MixerStatus != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_OVERFLOW_STATUS);
	MixerStatus->AdderStage =
		XDfeMix_RdBitField(XDFEMIX_OVERFLOW_ADDER_STAGE_WIDTH,
				   XDFEMIX_OVERFLOW_ADDER_STAGE_OFFSET, Val);
	MixerStatus->AdderAntenna =
		XDfeMix_RdBitField(XDFEMIX_OVERFLOW_ADDER_ANTENNA_WIDTH,
				   XDFEMIX_OVERFLOW_ADDER_ANTENNA_OFFSET, Val);
	MixerStatus->MixAntenna =
		XDfeMix_RdBitField(XDFEMIX_OVERFLOW_MIX_ANTENNA_WIDTH,
				   XDFEMIX_OVERFLOW_MIX_ANTENNA_OFFSET, Val);
	MixerStatus->MixCCID =
		XDfeMix_RdBitField(XDFEMIX_OVERFLOW_MIX_CCID_WIDTH,
				   XDFEMIX_OVERFLOW_MIX_CCID_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Read Triggers, set enable bit of update trigger. If register source, then
* trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_EnableCCUpdateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEMIX_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL));

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_ENABLED);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ONE_SHOT_ONESHOT);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Read Triggers, set enable bit of LowPower trigger. If register source, then
* trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_EnableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_ENABLED);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ONE_SHOT_ONESHOT);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Data);
}
/****************************************************************************/
/**
*
* Read Triggers, set enable bit of Activate trigger. If register source, then
* trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_EnableActivatePowerTrigger(XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEMIX_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL));

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_ENABLED);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ONE_SHOT_ONESHOT);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Data);

	InstancePtr->StateId = XDFEMIX_STATE_OPERATIONAL;
}
/** @} */
