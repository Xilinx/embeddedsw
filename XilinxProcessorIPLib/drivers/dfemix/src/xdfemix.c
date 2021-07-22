/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfexmix.c
* @addtogroup xdfemix_v1_1
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
*
* </pre>
*
******************************************************************************/

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
#define XDFEMIX_SEQUENCE_ENTRY_DEFAULT 0U /* Default sequence entry flag */
#define XDFEMIX_SEQUENCE_ENTRY_NULL (-1) /* Null sequence entry flag */
#define XDFEMIX_NO_EMPTY_CCID_FLAG 0xFFFFU /* Not Empty CCID flag */
#define XDFEMIX_PHASE_OFFSET_ROUNDING_BITS 14U
#define XDFEMIX_U32_NUM_BITS 32U
#define XDFEMIX_TAP_MAX 24U /* Maximum tap value */

#define XDFEMIXER_CURRENT false
#define XDFEMIXER_NEXT true

#define XDFEMIXER_PHACC_DISABLE false
#define XDFEMIXER_PHACC_ENABLE true

#define XDFEMIX_DRIVER_VERSION_MINOR 1U
#define XDFEMIX_DRIVER_VERSION_MAJOR 1U

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFEMIX_MAX_NUM_INSTANCES];
extern metal_phys_addr_t metal_phys[XDFEMIX_MAX_NUM_INSTANCES];
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
* @param    InstancePtr is a pointer to the DFE driver instance.
* @param    AddrOffset is address offset relative to instance base address.
* @param    Data is value to be written.
*
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
* @param    InstancePtr is a pointer to the DFE driver instance.
* @param    AddrOffset is address offset relative to instance base address.
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
* @param    InstancePtr is a pointer to the XDfeMix instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
* @param    FieldData is a bit field data.
*
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
* @param    InstancePtr is a pointer to the XDfeMix instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
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
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset in bits number.
* @param    Data is a u32 data which bit field this function reads.
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
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset in bits number.
* @param    Data is a u32 data which bit field this function reads.
* @param    Val is a u32 value to be written in the bit field.
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

/************************ DFE Common functions ******************************/

/****************************************************************************/
/**
*
* Finds unused CCID.
*
* @param    Sequence is a CC sequence array.
*
* @return Unused CCID.
*
*
****************************************************************************/
static s32 XDfeMix_GetNotUsedCCID(XDfeMix_CCSequence *Sequence)
{
	u32 Index;
	s32 NotUsedCCID;

	Xil_AssertNonvoid(Sequence != NULL);

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
* Adds the specified CCID, to the CC sequence. The sequence is defined with
* SlotSeqBitmap where bit0 coresponds to CC[0], bit1 to CC[1], and so on.
*
* Sequence data in returned CCIDSequence is not the same as what is written
* in registers, the translation is:
*              registers       passed back CCIDSequence
* -----------------------------------------------------
* Length:      0               0 - if (SEQUENCE[0] == SEQUENCE[1])
*              0               1 - if (SEQUENCE[0] != SEQUENCE[1])
*              1               2
*              2               3
*              .....
*              15              16
* SEQUENCE[x]: x               -1 - if unused CC
* SEQUENCE[x]: x               x - if used CC
*
* @param    InstancePtr is a pointer to the XDfeMix instance.
* @param    CCID is a CC ID.
* @param    SlotSeqBitmap maps the sequence.
* @param    CCIDSequence is a CC sequence array.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
*
****************************************************************************/
static u32 XDfeMix_AddCCID(XDfeMix *InstancePtr, s32 CCID, u32 SlotSeqBitmap,
			   XDfeMix_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 Mask;

	Xil_AssertNonvoid(CCIDSequence != NULL);
	Xil_AssertNonvoid(CCIDSequence->Length != 0);
	Xil_AssertNonvoid(CCID < XDFEMIX_CC_NUM);

	/* Check does sequence fit in the defined length */
	Mask = (1U << CCIDSequence->Length) - 1U;
	if (0U != (SlotSeqBitmap & (~Mask))) {
		metal_log(METAL_LOG_ERROR, "Sequence map does not fit in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Check are bits set in SlotSeqBitmap to 1 avaliable (-1)*/
	Mask = 1U;
	for (Index = 0U; Index < CCIDSequence->Length; Index++) {
		if (0U != (SlotSeqBitmap & Mask)) {
			if (CCIDSequence->CCID[Index] !=
			    XDFEMIX_SEQUENCE_ENTRY_NULL) {
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
		if (0U != (SlotSeqBitmap & Mask)) {
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
* @param    InstancePtr is a pointer to the XDfeMix instance.
* @param    CCID is a CC ID.
* @param    CCIDSequence is a CC sequence array.
*
*
****************************************************************************/
static void XDfeMix_RemoveCCID(XDfeMix *InstancePtr, s32 CCID,
			       XDfeMix_CCSequence *CCIDSequence)
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

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfeMix_GetNotUsedCCID(CCIDSequence);
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Sets antenna gain.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    AntennaId is an antenna ID.
* @param    AntennaGain is an antenna gain.
*
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
* Sets Rate and NCO in DUC-DDC configuration for CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCCfg is a configuration data container.
* @param    CCID is a Channel ID.
* @param    DUCDDCCfg is DUC/DDC configuration container.
*
*
****************************************************************************/
static void XDfeMix_SetCCDDC(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			     s32 CCID, const XDfeMix_DUCDDCCfg *DUCDDCCfg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID < XDFEMIX_CC_NUM);
	Xil_AssertVoid(DUCDDCCfg != NULL);
	Xil_AssertVoid(DUCDDCCfg->Rate <= XDFEMIX_RATE_MAX);
	Xil_AssertVoid(DUCDDCCfg->NCO <= XDFEMIX_NCO_MAX);
	Xil_AssertVoid(DUCDDCCfg->CCGain <= XDFEMIX_CC_GAIN_MAX);

	CCCfg->DUCDDCCfg[CCID].NCO = DUCDDCCfg->NCO;
	CCCfg->DUCDDCCfg[CCID].Rate = DUCDDCCfg->Rate;
	CCCfg->DUCDDCCfg[CCID].CCGain = DUCDDCCfg->CCGain;
}

/****************************************************************************/
/**
*
* Returns the current CC configuration.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CurrCCCfg is CC configuration container.
*
*
* @note For a sequence conversion see XDfeMix_AddCCID() comment.
*
****************************************************************************/
static void XDfeMix_GetCurrentCCCfg(XDfeMix *InstancePtr,
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
		CurrCCCfg->DUCDDCCfg[Index].NCO =
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

	/* Read Antenna configuration */
	AntennaCfg = XDfeMix_ReadReg(InstancePtr, XDFEMIX_ANTENNA_GAIN_CURRENT);
	for (Index = 0; Index < XDFEMIX_ANT_NUM_MAX; Index++) {
		CurrCCCfg->AntennaCfg[Index] = (AntennaCfg >> Index) & 0x01U;
	}
}

/****************************************************************************/
/**
*
* Writes NEXT CC configuration.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCCfg is a configuration data container.
*
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
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(NextCCCfg != NULL);

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
	   from CURRENT to NEXT */
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
					   NextCCCfg->DUCDDCCfg[Index].NCO);
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
		AntennaCfg += ((NextCCCfg->AntennaCfg[Index] & 0x01U) << Index);
	}
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_ANTENNA_GAIN_NEXT, AntennaCfg);
}

/****************************************************************************/
/**
*
* Gets PHACC index from the DUC/DDC Mapping NCO.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
*
* @return   Index
*
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
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
* @param    Freq is a frequency setting for CC.
*
*
****************************************************************************/
static void XDfeMix_SetCCFrequency(const XDfeMix *InstancePtr, bool Next,
				   s32 CCID, const XDfeMix_Frequency *Freq)
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
* Reads back frequency for particular CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
* @param    Freq is a frequency setting for CC.
*
****************************************************************************/
static void XDfeMix_GetCCFrequency(const XDfeMix *InstancePtr, bool Next,
				   s32 CCID, XDfeMix_Frequency *Freq)
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
* Writes the phase settings for a given CC's phase accumulator. The frequency
* settings for a given CC are shared across all antennas.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
* @param    Phase is a phase setting for CC.
*
*
****************************************************************************/
static void XDfeMix_SetCCPhase(const XDfeMix *InstancePtr, bool Next, s32 CCID,
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
* Reads back phase from AXI-lite registers for particular CCID.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
* @param    Phase is a phase setting for CC.
*
*
****************************************************************************/
static void XDfeMix_GetCCPhase(const XDfeMix *InstancePtr, bool Next, s32 CCID,
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
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
* @param    Enable is a flag that enables a phase accumulator.
*
*
****************************************************************************/
static void XDfeMix_SetCCPhaseAccumEnable(const XDfeMix *InstancePtr, bool Next,
					  s32 CCID, bool Enable)
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
* Captures phase for all phase accumulators in associated AXI-lite registers.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
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
* Subtracts phase B from phase A to give phase D. These consider full
* phase configurations so that difference will be as accurate as possible.
*
* The delta in phase accumulator is given by:
* PhaseAccDiff = PhaseB.PhaseAcc - PhaseA.PhaseAcc
* Phase offset is only an 18-bit quantity, so it is obtained from PhaseAcc by
* PhaseAcc by dividing by 2^14. Rounding before converting back to an
* unsigned integer will provide better accuracy.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    PhaseA is a phase A descriptor.
* @param    PhaseB is a phase B descriptor.
* @param    PhaseOffset is a phase offset descriptor.
*
* @note     PhaseAcc can also be interpreted as an unsigned quantity with
*           the difference causing a wrap-around of a full cycle to give
*           a positive phase when otherwise a negative number would be
*           generated.
*
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
* Sets phase offset component of frequency.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Frequency is a frequency container.
* @param    PhaseOffset is a phase offset container.
*
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
* Sets NCO output attenuation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
* @param    NCOGain is an NCO attenuation.
*
*
****************************************************************************/
static void XDfeMix_SetCCNCOGain(const XDfeMix *InstancePtr, bool Next,
				 s32 CCID, u32 NCOGain)
{
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_NCO_GAIN + Index, NCOGain);
}

/****************************************************************************/
/**
*
* Gets NCO output attenuation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Next is TRUE, read next config. If Next is FALSE, read
*           current config.
* @param    CCID is a Channel ID.
*
* @return   NCO attenuation.
*
*
****************************************************************************/
static u32 XDfeMix_GetCCNCOGain(const XDfeMix *InstancePtr, bool Next, s32 CCID)
{
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Index = XDfeMix_GetPhaccIndex(InstancePtr, Next, CCID);
	return XDfeMix_ReadReg(InstancePtr, XDFEMIX_NCO_GAIN + Index);
}

/****************************************************************************/
/**
*
* Writes register CORE.PL_MIXER_DELAY with value 2.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
*
****************************************************************************/
static void XDfeMix_SetPLMixerDelay(const XDfeMix *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDfeMix_WriteReg(InstancePtr, XDFEMIX_PL_MIXER_DELAY,
			 XDFEMIX_PL_MIXER_DELAY_VALUE);
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of update trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeMix_EnableCCUpdateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);

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
* @param    InstancePtr is a pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_EnableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

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
* @param    InstancePtr is a pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_EnableActivateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

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
* @param    InstancePtr is a pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_EnableDeactivateTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

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
* @param    InstancePtr is a pointer to the Mixer instance.
*
****************************************************************************/
static void XDfeMix_DisableLowPowerTrigger(const XDfeMix *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeMix_ReadReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeMix_WrBitField(XDFEMIX_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEMIX_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API initialises one instance of a channel filter driver.
* Traverses "/sys/bus/platform/device" directory (in Linux), to find registered
* MIX device with the name DeviceNodeName. The first available slot in
* the instances array XDfeMix_Mixer[] will be taken as a DeviceNodeName
* object.
*
* @param    DeviceNodeName is device node name.
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
			return &XDfeMix_Mixer[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		if (XDfeMix_Mixer[Index].NodeName[0] == '\0') {
			strncpy(XDfeMix_Mixer[Index].NodeName, DeviceNodeName,
				strlen(DeviceNodeName));
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
	Addr = strtol(AddrStr, NULL, 16);
	for (Index = 0; Index < XDFEMIX_MAX_NUM_INSTANCES; Index++) {
		if (Addr == metal_phys[Index]) {
			InstancePtr->Device = &CustomDevice[Index];
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
* API closes the instances of a Mixer driver.
*
* @param    InstancePtr is a pointer to the XDfeMix instance.
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
* Resets and put block into a reset state.
*
* @param    InstancePtr is a pointer to the Mixer instance.
*
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
* S/W reset removed.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Cfg is a configuration data container.
*
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
* DFE Mixer driver one time initialisation.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Init is a initialisation data container.
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
* Activates mixer.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    EnableLowPower is a flag indicating low power.
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
* Deactivates mixer.
*
* @param    InstancePtr is a pointer to the Mixer instance.
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

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Adds specified CCID, with specified configuration.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    SlotSeqBitmap - up to 16 defined slots into which a CC can be
*           allocated. The number of slots can be from 1 to 16 depending on
*           system initialization. The number of slots is defined by the
*           "sequence length" parameter which is provided during initialization.
*           The Bit offset within the SlotSeqBitmap indicates the equivalent
*           Slot number to allocate. e.g. 0x0003  means the caller wants the
*           passed component carrier (CC) to be allocated to slots 0 and 1.
* @param    CarrierCfg is a CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_AddCC(XDfeMix *InstancePtr, s32 CCID, u32 SlotSeqBitmap,
		  const XDfeMix_CarrierCfg *CarrierCfg)
{
	XDfeMix_CCCfg CCCfg;
	u32 AddSuccess;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFEMIX_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(CarrierCfg->DUCDDCCfg.NCO <= XDFEMIX_NCO_MAX);

	/* Read current CC configuration. Note that XDfeMix_Initialise writes
	   a NULL CC sequence to H/W */
	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeMix_AddCCID(InstancePtr, CCID, SlotSeqBitmap,
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
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Removes specified CCID.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
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
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    Rate is a NCO rate value.
* @param    FromNCO is a NCO value moving from.
* @param    ToNCO is a NCO value moving to.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_MoveCC(XDfeMix *InstancePtr, s32 CCID, u32 Rate, u32 FromNCO,
		   u32 ToNCO)
{
	XDfeMix_CCCfg NextCCCfg;
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

	XDfeMix_GetCurrentCCCfg(InstancePtr, &NextCCCfg);
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

	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Placeholder. This would be used to update frequency etc without updating
* sequence.
*
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
* Sets antenna gain. Initiate CC update (enable CCUpdate trigger TUSER
* Single Shot).
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    AntennaId is an antenna ID.
* @param    AntennaGain is an antenna gain.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeMix_SetAntennaGain(XDfeMix *InstancePtr, u32 AntennaId, u32 AntennaGain)
{
	XDfeMix_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AntennaGain <= 1U);
	Xil_AssertNonvoid(AntennaId <= XDFEMIX_ANT_NUM_MAX);

	XDfeMix_GetCurrentCCCfg(InstancePtr, &CCCfg);
	XDfeMix_SetNextCCCfg(InstancePtr, &CCCfg);
	XDfeMix_SetAntennaGainL(InstancePtr, AntennaId, AntennaGain);
	return XDfeMix_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Returns current trigger configuration.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    TriggerCfg is a trigger configuration container.
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
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    TriggerCfg is a trigger configuration container.
*
****************************************************************************/
void XDfeMix_SetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);

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
* Gets DUC/DDC status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    DUCDDCStatus is a DUC/DDC status container.
*
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
* Gets Mixer status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    CCID is a Channel ID.
* @param    DUCDDCStatus is a DUC/DDC status container.
*
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
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Delay is a requested delay variable.
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
* @param    InstancePtr is a pointer to the Mixer instance.
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
* Returns CONFIG.DATA_LATENCY.VALUE + tap, where the tap is between 0
* and 23 in real mode and between 0 and 11 in complex/matrix mode.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Tap is a tap variable.
*
* @return   Data latency value.
*
****************************************************************************/
u32 XDfeMix_GetTDataDelay(const XDfeMix *InstancePtr, u32 Tap)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tap < XDFEMIX_TAP_MAX);

	Data = XDfeMix_RdRegBitField(InstancePtr, XDFEMIX_LATENCY_OFFSET,
				     XDFEMIX_LATENCY_VALUE_WIDTH,
				     XDFEMIX_LATENCY_VALUE_OFFSET);
	return (Data + Tap);
}

/*****************************************************************************/
/**
*
* This API gets the driver and HW design version.
*
* @param    SwVersion is driver version number.
* @param    HwVersion is HW version number.
*
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
