/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
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
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/15/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     03/18/21 New model parameter list
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
#define XDFEMIX_PHASE_OFFSET_ROUNDING_BITS 14U
#define XDFEMIX_U32_NUM_BITS 32U

#define XDFEMIXER_CURRENT false
#define XDFEMIXER_NEXT true

#define XDFEMIXER_PHACC_DISABLE false
#define XDFEMIXER_PHACC_ENABLE true

#define XDFEMIX_DRIVER_VERSION_MINOR 0U
#define XDFEMIX_DRIVER_VERSION_MAJOR 1U

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFEMIX_MAX_NUM_INSTANCES];
#endif
static struct metal_device *DevicePtrStorage[XDFEMIX_MAX_NUM_INSTANCES];
extern XDfeMix XDfeMix_Mixer[XDFEMIX_MAX_NUM_INSTANCES];
static u32 XDfeMix_DriverHasBeenRegisteredOnce = 0U;

/************************** Function Definitions ****************************/
extern s32 XDfeMix_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr,
				 const char *DeviceNodeName);
extern s32 XDfeMix_LookupConfig(u16 DeviceId);
extern void XDfeMix_CfgInitialize(XDfeMix *InstancePtr);

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
	metal_io_write32(InstancePtr->Io, (unsigned long)AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Read a value from register from a Mixer instance.
*
* @param    InstancePtr is a pointer to the DFE driver instance.
* @param    AddrOffset is address offset relative to instance base address.
*
* @return   Register value.
*
* @note     None
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
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);

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
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);
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
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEMIX_U32_NUM_BITS);

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
static u32 XDfeMix_AddCCID(u32 CCID, u32 Rate, XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 SeqLen;
	u32 SeqMult;
	u32 CCIDCount;
	u32 EmptyCCID = XDFEMIX_NO_EMPTY_CCID_FLAG;
	Xil_AssertNonvoid((Rate == 1U) || (Rate == 2U) || (Rate == 4U) ||
			  (Rate == 8U) || (Rate == 16U));
	Xil_AssertNonvoid(CCIDSequence != NULL);

	/* Assumptions:
	   Rate can be 1, 2, 4 or 8. (include 16 (question))
	   If the function exits with an error the CCIDSequence argument should
	   remain unchanged. Implements a "greedy" allocation of sequence
	   locations */
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
	SeqMult = Rate / SeqLen;
	/* Note when SeqMult > 1 the rate constraints mean it should also be
	   an integer value. */
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
			if (0U == CCIDCount) {
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
			if (0U == CCIDCount) {
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
			      XDFEMIX_ONE_ANTENNA_GAIN_WIDTH, AntennaId,
			      AntennaGain);
}

/****************************************************************************/
/**
*
* Set Rate and NCO.in DUC-DDC configuration for CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCCfg is a configuration data container.
* @param    CCID is a Channel ID.
* @param    DUCDDCCfg is DUC/DDC configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetCCDDC(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			     u32 CCID, const XDfeMix_DUCDDCCfg *DUCDDCCfg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID < XDFEMIX_CC_NUM);
	Xil_AssertVoid(DUCDDCCfg != NULL);
	Xil_AssertVoid(DUCDDCCfg->Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertVoid(DUCDDCCfg->NCO <= XDFEMIX_NCO_MAX);

	CCCfg->DUCDDCCfg[CCID].NCO = DUCDDCCfg->NCO;
	CCCfg->DUCDDCCfg[CCID].Rate = DUCDDCCfg->Rate;
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
	u32 SeqLen;
	u32 AntennaCfg = 0U;
	u32 Data;
	u32 Offset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	if (Next == XDFEMIXER_NEXT) {
		/* Read sequence length */
		SeqLen = XDfeMix_ReadReg(InstancePtr,
					 XDFEMIX_SEQUENCE_LENGTH_NEXT);
		CCCfg->Sequence.Length = SeqLen + 1U;

		/* Read CCID sequence and carrier configurations */
		for (Index = 0; Index < XDFEMIX_SEQUENCE_SIZE; Index++) {
			CCCfg->Sequence.CCID[Index] = XDfeMix_ReadReg(
				InstancePtr,
				XDFEMIX_SEQUENCE_NEXT + (Index * sizeof(u32)));
		}

		for (Index = 0; Index < XDFEMIX_CC_CONFIG_SIZE; Index++) {
			Offset = XDFEMIX_CC_CONFIG_NEXT +
				 ((Index * sizeof(u32)));
			Data = XDfeMix_ReadReg(InstancePtr, Offset);
			CCCfg->DUCDDCCfg[Index].NCO = XDfeMix_RdBitField(
				XDFEMIX_CC_CONFIG_NCO_WIDTH,
				XDFEMIX_CC_CONFIG_NCO_OFFSET, Data);
			CCCfg->DUCDDCCfg[Index].Rate = XDfeMix_RdBitField(
				XDFEMIX_CC_CONFIG_RATE_WIDTH,
				XDFEMIX_CC_CONFIG_RATE_OFFSET, Data);
		}

		/* Read Antenna configuration */
		AntennaCfg =
			XDfeMix_ReadReg(InstancePtr, XDFEMIX_ANTENNA_GAIN_NEXT);
		for (Index = 0; Index < XDFEMIX_ANT_NUM_MAX; Index++) {
			CCCfg->AntennaCfg[Index] =
				(AntennaCfg >> Index) & 0x01U;
		}
	} else {
		/* Read sequence length */
		SeqLen = XDfeMix_ReadReg(InstancePtr,
					 XDFEMIX_SEQUENCE_LENGTH_CURRENT);
		CCCfg->Sequence.Length = SeqLen + 1U;

		for (Index = 0; Index < XDFEMIX_SEQUENCE_SIZE; Index++) {
			CCCfg->Sequence.CCID[Index] = XDfeMix_ReadReg(
				InstancePtr, XDFEMIX_SEQUENCE_CURRENT +
						     (Index * sizeof(u32)));
		}

		/* Read CCID sequence and carrier configurations */
		for (Index = 0; Index < XDFEMIX_CC_CONFIG_SIZE; Index++) {
			Offset = XDFEMIX_CC_CONFIG_CURRENT +
				 (Index * sizeof(u32));
			Data = XDfeMix_ReadReg(InstancePtr, Offset);
			CCCfg->DUCDDCCfg[Index].NCO = XDfeMix_RdBitField(
				XDFEMIX_CC_CONFIG_NCO_WIDTH,
				XDFEMIX_CC_CONFIG_NCO_OFFSET, Data);
			CCCfg->DUCDDCCfg[Index].Rate = XDfeMix_RdBitField(
				XDFEMIX_CC_CONFIG_RATE_WIDTH,
				XDFEMIX_CC_CONFIG_RATE_OFFSET, Data);
		}

		/* Read Antenna configuration */
		AntennaCfg = XDfeMix_ReadReg(InstancePtr,
					     XDFEMIX_ANTENNA_GAIN_CURRENT);
		for (Index = 0; Index < XDFEMIX_ANT_NUM_MAX; Index++) {
			CCCfg->AntennaCfg[Index] =
				(AntennaCfg >> Index) & 0x01U;
		}
	}
}

/****************************************************************************/
/**
*
* Write NEXT CC configuration.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCCfg is a configuration data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetNextCCCfg(const XDfeMix *InstancePtr,
				 const XDfeMix_CCCfg *CCCfg)
{
	u32 SeqLen = 0U;
	u32 AntennaCfg = 0U;
	u32 DucDdcConfig;
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	/* Write sequence length */
	if (0U < CCCfg->Sequence.Length) {
		SeqLen = CCCfg->Sequence.Length - 1U;
	}
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_SEQUENCE_LENGTH_NEXT, SeqLen);

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEMIX_SEQUENCE_SIZE; Index++) {
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_SEQUENCE_NEXT + (sizeof(u32) * Index),
				 CCCfg->Sequence.CCID[Index]);
	}

	for (Index = 0; Index < XDFEMIX_CC_CONFIG_SIZE; Index++) {
		DucDdcConfig = XDfeMix_ReadReg(InstancePtr,
					       XDFEMIX_CC_CONFIG_NEXT +
						       ((Index * sizeof(u32))));
		DucDdcConfig =
			XDfeMix_WrBitField(XDFEMIX_CC_CONFIG_NCO_WIDTH,
					   XDFEMIX_CC_CONFIG_NCO_OFFSET,
					   DucDdcConfig,
					   CCCfg->DUCDDCCfg[Index].NCO);
		DucDdcConfig =
			XDfeMix_WrBitField(XDFEMIX_CC_CONFIG_RATE_WIDTH,
					   XDFEMIX_CC_CONFIG_RATE_OFFSET,
					   DucDdcConfig,
					   CCCfg->DUCDDCCfg[Index].Rate);
		XDfeMix_WriteReg(InstancePtr,
				 XDFEMIX_CC_CONFIG_NEXT +
					 ((Index * sizeof(u32))),
				 DucDdcConfig);
	}

	/* Write Antenna configuration */
	for (Index = 0; Index < XDFEMIX_ANT_NUM_MAX; Index++) {
		AntennaCfg += ((CCCfg->AntennaCfg[Index] & 0x01U) << Index);
	}
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_ANTENNA_GAIN_NEXT, AntennaCfg);
}

/****************************************************************************/
/**
*
* Get PHACC index from the DUC/DDC Mapping NCO.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next TRUE read next config, FALSE read current config.
* @param    CCID is a Channel ID.
*
* @return   Index
*
* @note     None
*
****************************************************************************/
static u32 XDfeMix_GetPhaccIndex(const XDfeMix *InstancePtr, bool Next,
				 u32 CCID)
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
static void XDfeMix_SetCCFrequency(const XDfeMix *InstancePtr, bool Next,
				   u32 CCID, const XDfeMix_Frequency *Freq)
{
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Freq != NULL);

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
static void XDfeMix_GetCCFrequency(const XDfeMix *InstancePtr, bool Next,
				   u32 CCID, XDfeMix_Frequency *Freq)
{
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Freq != NULL);

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
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Phase != NULL);

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
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Phase != NULL);

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
	u32 Index;
	u32 Data = 0U;

	Xil_AssertVoid(InstancePtr != NULL);

	if (Enable == XDFEMIXER_PHACC_ENABLE) {
		Data = 1U;
	}
	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PHASE_ACC_ENABLE + Index, Data);
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
	u32 Index;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_PHASE_ACC_ENABLE + Index);
	if (Data == 1U) {
		*Enable = XDFEMIXER_PHACC_ENABLE;
	} else {
		*Enable = XDFEMIXER_PHACC_DISABLE;
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
	if (0U != (PhaseAccDiff &
		   ((u32)1 << (XDFEMIX_PHASE_OFFSET_ROUNDING_BITS - 1U)))) {
		PhaseOffset->PhaseOffset += 1U;
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
static void XDfeMix_SetCCNCOGain(const XDfeMix *InstancePtr, bool Next,
				 u32 CCID, u32 NCOGain)
{
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_NCO_GAIN + Index, NCOGain);
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
static u32 XDfeMix_GetCCNCOGain(const XDfeMix *InstancePtr, bool Next, u32 CCID)
{
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	return XDfeMix_ReadReg(InstancePtr, XDFEMIX_NCO_GAIN + Index);
}

/****************************************************************************/
/**
*
* Write register CORE.PL_MIXER_DELAY with value 2.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_SetPLMixerDelay(const XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDfeMix_WriteReg(InstancePtr, PL_MIXER_DELAY, PL_MIXER_DELAY_VALUE);
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
static void XDfeMix_EnableCCUpdateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_ENABLED);
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
static void XDfeMix_EnableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_ENABLED);
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
static void XDfeMix_EnableActivateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_ENABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Read Triggers, reset enable bit of LowPower trigger.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeMix_DisableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_ENABLE_DISABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API Initialise one instancies of a Mixer driver.
* Traverse "/sys/bus/platform/device" directory, to find Mixer device id,
* corresponding to provided DeviceNodeName. The device id is defined by
* the address of the entry in the order from lowest to highest, eg.:
* Id=0 for the Equalizer entry located to the lowest address,
* Id=1 for the Equalizer entry located to the second lowest address,
* Id=2 for the Equalizer entry located to the third lowest address, and so on.
*
* @param    DeviceId contains the index number of the device instance,
* @param    DeviceNodeName is device node name,
*
* @return
*           - pointer to instance if successful.
*           - NULL on error.
*
* @note     None.
*
******************************************************************************/
XDfeMix *XDfeMix_InstanceInit(u16 DeviceId, const char *DeviceNodeName)
{
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
	if (XST_SUCCESS != XDfeMix_RegisterMetal(DeviceId,
						 &DevicePtrStorage[DeviceId],
						 DeviceNodeName)) {
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
				   XDFEMIX_MODEL_PARAM_1_MODE_OFFSET, ModelParam);
	InstancePtr->Config.NumAntenna =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_NUM_ANTENNA_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_NUM_ANTENNA_OFFSET,
				   ModelParam);
	InstancePtr->Config.MaxUseableCcids =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_MAX_USEABLE_CCIDS_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_MAX_USEABLE_CCIDS_OFFSET,
				   ModelParam);
	InstancePtr->Config.Lanes =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_LANES_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_LANES_OFFSET,
				   ModelParam);
	InstancePtr->Config.AntennaInterleave =
		XDfeMix_RdBitField(XDFEMIX_MODEL_PARAM_1_ANTENNA_INTERLEAVE_WIDTH,
				   XDFEMIX_MODEL_PARAM_1_ANTENNA_INTERLEAVE_OFFSET,
				   ModelParam);
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

	Cfg->ModelParams.Mode = InstancePtr->Config.Mode;
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.MaxUseableCcids = InstancePtr->Config.MaxUseableCcids;
	Cfg->ModelParams.Lanes = InstancePtr->Config.Lanes;
	Cfg->ModelParams.AntennaInterleave = InstancePtr->Config.AntennaInterleave;
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
* DFE Mixer driver one time initialisation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_Initialize(XDfeMix *InstancePtr)
{
	XDfeMix_Trigger CCUpdate;
	u32 Data;
	u32 Index;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_CONFIGURED);

	XDfeMix_SetPLMixerDelay(InstancePtr);

	/* Set NULL sequence and ensure all CCs are disabled. Not all registers
	   will be cleared by reset as they are implemented using DRAM. This
	   step sets all CC_CONFIGURATION.CARRIER_CONFIGURATION.CURRENT[*].
	   DUC_DDC to 0 ensuring the Hardblock will remain disabled following
	   the first call to XDFEMIXilterActivate. */
	for (Index = 0; Index < XDFEMIX_SEQ_LENGTH_MAX; Index++) {
		Offset = XDFEMIX_SEQUENCE_NEXT + (sizeof(u32) * Index);
		XDfeMix_WriteReg(InstancePtr, Offset,
				 XDFEMIX_SEQUENCE_ENTRY_NULL);
	}
	for (Index = 0; Index < XDFEMIX_CC_NUM; Index++) {
		Offset = XDFEMIX_CC_CONFIG_NEXT + (sizeof(u32) * Index);
		XDfeMix_WriteReg(InstancePtr, Offset, 0U);
	}

	/* Trigger CC_UPDATE immediately using Register source to update
	   CURRENT from NEXT */
	CCUpdate.Enable = 1U;
	CCUpdate.OneShot = 1U;
	CCUpdate.Source = 0U;
	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  CCUpdate.OneShot);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_ENABLE_OFFSET, Data,
				  CCUpdate.Enable);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_SOURCE_WIDTH,
				  XDFEMIX_TRIGGERS_SOURCE_OFFSET, Data,
				  CCUpdate.Source);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Data);

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
* @param    EnableLowPower is an falg indicating low power.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeMix_Activate(XDfeMix *InstancePtr, bool EnableLowPower)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_STATE_OPERATIONAL_OFFSET);
	if (1U == Data) {
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
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_STATE_OPERATIONAL_OFFSET);
	if (0U == Data) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfeMix_DisableLowPowerTrigger(InstancePtr);

	/* Enable Activate trigger (toggles state between operational
	   and intialized) */
	XDfeMix_EnableActivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFEMIX_STATE_INITIALISED;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Add specified CCID, with specified configuration.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    CarrierCfg is a CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     None
*
****************************************************************************/
u32 XDfeMix_AddCC(const XDfeMix *InstancePtr, u32 CCID,
		  const XDfeMix_CarrierCfg *CarrierCfg)
{
	XDfeMix_CCCfg CCCfg;
	u32 AddSuccess;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(CarrierCfg->DUCDDCCfg.Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertNonvoid(CarrierCfg->DUCDDCCfg.NCO <= XDFEMIX_NCO_MAX);

	/* Read current CC configuration. Note that XDfeMix_Initialise writes
	   a NULL CC sequence to H/W */
	XDfeMix_GetCCCfg(InstancePtr, XDFEMIXER_CURRENT, &CCCfg);

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeMix_AddCCID(CCID, CarrierCfg->DUCDDCCfg.Rate,
				     &CCCfg.Sequence);
	if (AddSuccess == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* If add is successful update next configuration and trigger update */
	XDfeMix_SetCCDDC(InstancePtr, &CCCfg, CCID, &CarrierCfg->DUCDDCCfg);
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	XDfeMix_SetCCFrequency(InstancePtr, XDFEMIXER_NEXT, CCID,
			       &CarrierCfg->NCO.FrequencyCfg);
	XDfeMix_SetCCPhase(InstancePtr, XDFEMIXER_NEXT, CCID,
			   &CarrierCfg->NCO.PhaseCfg);
	XDfeMix_SetCCNCOGain(InstancePtr, XDFEMIXER_NEXT, CCID,
			     CarrierCfg->NCO.NCOGain);
	/*
	 *  PHACCs configured, but not running.
	 *  NCOs not running.
	 *  Antenna contribution disabled.
	 */
	XDfeMix_EnableCCUpdateTrigger(InstancePtr);

	return XST_SUCCESS;
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
void XDfeMix_RemoveCC(const XDfeMix *InstancePtr, u32 CCID)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFEMIX_CC_NUM);

	/* Read current CC configuration */
	XDfeMix_GetCCCfg(InstancePtr, XDFEMIXER_CURRENT, &CCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeMix_RemoveCCID(CCID, &CCCfg.Sequence);

	CCCfg.DUCDDCCfg[CCID].Rate = 0U;

	/* Update next configuration and trigger update */
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
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
void XDfeMix_MoveCC(const XDfeMix *InstancePtr, u32 CCID, u32 Rate, u32 FromNCO,
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

	XDfeMix_GetCCCfg(InstancePtr, XDFEMIXER_CURRENT, &NextCCCfg);
	XDfeMix_SetNextCCCfg(InstancePtr, &NextCCCfg);
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
	XDfeMix_DerivePhaseOffset(InstancePtr, &PhaseCurrent, &PhaseNext,
				  &PhaseOffset);
	XDfeMix_SetPhaseOffset(InstancePtr, &Freq, &PhaseDiff);
	XDfeMix_SetCCFrequency(InstancePtr, XDFEMIXER_NEXT, CCID, &Freq);

	XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Placeholder. This would be used to update frequency etc without updating
* sequence.
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
void XDfeMix_SetAntennaGain(const XDfeMix *InstancePtr, u32 AntennaId,
			    u32 AntennaGain)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AntennaGain <= 1U);
	Xil_AssertVoid(AntennaId <= XDFEMIX_ANT_NUM_MAX);

	XDfeMix_GetCCCfg(InstancePtr, XDFEMIXER_CURRENT, &CCCfg);
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
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
void XDfeMix_GetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEMIX_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read ACTIVATE triggers */
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

	/* Read LOW_POWER triggers */
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

	/* Read CC_UPDATE triggers */
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
void XDfeMix_SetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Write public trigger configuration members and ensure private
	   members (_Enable & _OneShot) are set appropriately */

	/* Activate defined as OneShot (as per the programming model) */
	TriggerCfg->Activate.Enable = 0U;
	TriggerCfg->Activate.OneShot = 1U;
	/* Read/set/write ACTIVATE triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				 XDFEMIX_TRIGGERS_ENABLE_OFFSET, Val,
				 TriggerCfg->Activate.Enable);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				 XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Val,
				 TriggerCfg->Activate.OneShot);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_ACTIVATE_OFFSET, Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.Enable = 0U;
	TriggerCfg->LowPower.OneShot = 0U;
	/* Read LOW_POWER triggers */
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				 XDFEMIX_TRIGGERS_ENABLE_OFFSET, Val,
				 TriggerCfg->LowPower.Enable);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				 XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Val,
				 TriggerCfg->LowPower.OneShot);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Val);

	/* CCUpdate defined as OneShot */
	TriggerCfg->CCUpdate.Enable = 0U;
	TriggerCfg->CCUpdate.OneShot = 1U;
	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ENABLE_WIDTH,
				 XDFEMIX_TRIGGERS_ENABLE_OFFSET, Val,
				 TriggerCfg->CCUpdate.Enable);
	Val = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_ONE_SHOT_WIDTH,
				 XDFEMIX_TRIGGERS_ONE_SHOT_OFFSET, Val,
				 TriggerCfg->CCUpdate.OneShot);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_CC_UPDATE_OFFSET, Val);
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

/*****************************************************************************/
/**
*
* This API is used to get the driver version.
*
* @param    SwVersion is driver version numbers.
* @param    HwVersion is HW version numbers.
*
* @return   None
*
* @note     None.
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
