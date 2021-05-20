/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf.c
* @addtogroup dfeccf_v1_0
* @{
*
* Contains the APIs for DFE Channel Filter component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/29/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/08/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     03/16/21 update activate & deactivate api
*       dc     03/25/21 Device tree item name change
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/08/21 Set sequence length only once
*       dc     04/18/21 Update trigger and event handlers
*       dc     04/20/21 Doxygen documentation update
*       dc     04/22/21 Add write MappedId field
*       dc     05/08/21 Update to common trigger
*       dc     05/18/21 Handling CCUpdate trigger
*
* </pre>
*
******************************************************************************/

#include "xdfeccf.h"
#include "xdfeccf_hw.h"
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
#define XDFECCF_SEQUENCE_ENTRY_DEFAULT 0U /**< Default sequence entry flag */
#define XDFECCF_SEQUENCE_ENTRY_NULL (-1) /**< Null sequence entry flag */
#define XDFECCF_NO_EMPTY_CCID_FLAG 0xFFFFU /**< Not Empty CCID flag */
#define XDFECCF_COEFF_LOAD_TIMEOUT 100U /**< Units of 10us */
#define XDFECCF_ACTIVE_SET_NUM 8U /**< Maximum number of active sets */
#define XDFECCF_U32_NUM_BITS 32U

#define XDFECCF_DRIVER_VERSION_MINOR 0U
#define XDFECCF_DRIVER_VERSION_MAJOR 1U

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFECCF_MAX_NUM_INSTANCES];
extern metal_phys_addr_t metal_phys[XDFECCF_MAX_NUM_INSTANCES];
#endif
extern XDfeCcf XDfeCcf_ChFilter[XDFECCF_MAX_NUM_INSTANCES];
static u32 XDfeCcf_DriverHasBeenRegisteredOnce = 0U;

/************************** Function Definitions ****************************/
extern s32 XDfeCcf_RegisterMetal(XDfeCcf *InstancePtr,
				 struct metal_device **DevicePtr,
				 const char *DeviceNodeName);
extern s32 XDfeCcf_LookupConfig(XDfeCcf *InstancePtr);
extern void XDfeCcf_CfgInitialize(XDfeCcf *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Writes value to register in a Ccf instance.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    AddrOffset is address offset relative to instance base address.
* @param    Data is value to be written.
*
*


****************************************************************************/
void XDfeCcf_WriteReg(const XDfeCcf *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, (unsigned long)AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Reads a value from register using a Ccf instance.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    AddrOffset is address offset relative to instance base address.
*
* @return   Register value.
*
*
****************************************************************************/
u32 XDfeCcf_ReadReg(const XDfeCcf *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, (unsigned long)AddrOffset);
}

/****************************************************************************/
/**
*
* Writes a bit field value to register.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
* @param    FieldData is a bit field data.
*
*
****************************************************************************/
void XDfeCcf_WrRegBitField(const XDfeCcf *InstancePtr, u32 Offset,
			   u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((FieldOffset + FieldWidth) <= XDFECCF_U32_NUM_BITS);

	Data = XDfeCcf_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfeCcf_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads a bit field value from register.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
*
* @return   Bit field data.
*
*
****************************************************************************/
u32 XDfeCcf_RdRegBitField(const XDfeCcf *InstancePtr, u32 Offset,
			  u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFECCF_U32_NUM_BITS);

	Data = XDfeCcf_ReadReg(InstancePtr, Offset);
	return ((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U));
}

/****************************************************************************/
/**
*
* Reads a bit field value from u32 variable.
*
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset in bits number.
* @param    Data is a u32 data. Data is a bit field that the function reads.
*
* @return   Bit field value.
*
*
****************************************************************************/
u32 XDfeCcf_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFECCF_U32_NUM_BITS);
	return (((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U)));
}
/****************************************************************************/
/**
*
* Writes a bit field value to u32 variable.
*
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset in bits number.
* @param    Data is a u32 data. Data is a bit field that the function reads.
* @param    Val is a u32 value to be written in the bit field.
*
* @return   Data with a written bit field.
*
*
****************************************************************************/
u32 XDfeCcf_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
{
	u32 BitFieldSet;
	u32 BitFieldClear;
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFECCF_U32_NUM_BITS);

	BitFieldSet = (Val & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	BitFieldClear =
		Data & (~((((u32)1U << FieldWidth) - 1U) << FieldOffset));
	return (BitFieldSet | BitFieldClear);
}

/************************ DFE Common functions ******************************/

/****************************************************************************/
/**
*
* Finds not used CCID.
*
* @param    Sequence is a CC sequence array.
*
* @return   Unused CCID.
*
*
****************************************************************************/
static s32 XDfeCcf_GetNotUsedCCID(XDfeCcf_CCSequence *Sequence)
{
	u32 Index;
	s32 NotUsedCCID;

	Xil_AssertNonvoid(Sequence != NULL);

	/* Not used Sequence.CCID[] has value -1, but the values in the range
	   [0,15] can be written in the registers, only. Now, we have to detect
	   not used CCID, and save it for the later usage. */
	for (NotUsedCCID = 0U; NotUsedCCID < XDFECCF_CC_NUM; NotUsedCCID++) {
		for (Index = 0U; Index < XDFECCF_CC_NUM; Index++) {
			if (Sequence->CCID[Index] == NotUsedCCID) {
				break;
			}
		}
		if (Index == XDFECCF_CC_NUM) {
			break;
		}
	}
	return (NotUsedCCID);
}

/****************************************************************************/
/**
*
* Adds the specified CCID, to the CC sequence. The sequence is defined with
* BitSequence where bit0 coresponds to CC[0], bit1 to CC[1], bit2 to CC[2], and
* so on.
*
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
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
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    CCID is a CC ID.
* @param    BitSequence maps the sequence.
* @param    CCIDSequence is a CC sequence array.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
*
****************************************************************************/
static u32 XDfeCcf_AddCCID(XDfeCcf *InstancePtr, s32 CCID, u32 BitSequence,
			   XDfeCcf_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 Mask;

	Xil_AssertNonvoid(CCIDSequence != NULL);
	Xil_AssertNonvoid(CCIDSequence->Length != 0);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);

	/* Check does sequence fit in the defined length */
	Mask = (1U << CCIDSequence->Length) - 1U;
	if (0U != (BitSequence & (~Mask))) {
		metal_log(METAL_LOG_ERROR, "Sequence map does not fit in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Check are bits set in BitSequence to 1 avaliable (-1)*/
	Mask = 1U;
	for (Index = 0U; Index < CCIDSequence->Length; Index++) {
		if (0U != (BitSequence & Mask)) {
			if (CCIDSequence->CCID[Index] !=
			    XDFECCF_SEQUENCE_ENTRY_NULL) {
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
		if (0U != (BitSequence & Mask)) {
			CCIDSequence->CCID[Index] = CCID;
		}
		Mask <<= 1U;
	}

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfeCcf_GetNotUsedCCID(CCIDSequence);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes the specified CCID from the CC sequence and replaces the CCID
* entries with null (-1).
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    CCID is a CC ID.
* @param    CCIDSequence is a CC sequence array.
*
*
****************************************************************************/
static void XDfeCcf_RemoveCCID(XDfeCcf *InstancePtr, s32 CCID,
			       XDfeCcf_CCSequence *CCIDSequence)
{
	u32 Index;
	Xil_AssertVoid(CCIDSequence != NULL);
	Xil_AssertVoid(CCIDSequence->Length <= XDFECCF_SEQ_LENGTH_MAX);

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] = XDFECCF_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfeCcf_GetNotUsedCCID(CCIDSequence);
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Returns the current CC configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CurrCCCfg is CC configuration container.
*
* @note     For a sequence conversion see XDfeCcf_AddCCID() comment.
*
****************************************************************************/
static void XDfeCcf_GetCurrentCCCfg(const XDfeCcf *InstancePtr,
				    XDfeCcf_CCCfg *CurrCCCfg)
{
	u32 SeqLen;
	u32 AntennaCfg = 0U;
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CurrCCCfg != NULL);

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		CurrCCCfg->Sequence.CCID[Index] = XDfeCcf_ReadReg(
			InstancePtr,
			XDFECCF_SEQUENCE_CURRENT + (sizeof(u32) * Index));
		XDfeCcf_GetCC(InstancePtr, Index,
			      &CurrCCCfg->CarrierCfg[Index]);
	}

	/* Read sequence length */
	SeqLen = XDfeCcf_ReadReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_CURRENT);
	if (SeqLen == 0U) {
		CurrCCCfg->Sequence.Length = InstancePtr->SequenceLength;
	} else {
		CurrCCCfg->Sequence.Length = SeqLen + 1U;
	}

	/* Convert not used CC to -1 */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		if ((CurrCCCfg->Sequence.CCID[Index] ==
		     InstancePtr->NotUsedCCID) ||
		    (Index >= InstancePtr->SequenceLength)) {
			CurrCCCfg->Sequence.CCID[Index] =
				XDFECCF_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Read Antenna configuration */
	AntennaCfg = XDfeCcf_ReadReg(InstancePtr,
				     XDFECCF_ANTENNA_CONFIGURATION_CURRENT);
	for (Index = 0; Index < XDFECCF_ANT_NUM_MAX; Index++) {
		CurrCCCfg->AntennaCfg[Index] = (AntennaCfg >> Index) & 0x01U;
	}
}

/****************************************************************************/
/**
*
* Sets the next CC configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    NextCCCfg is Next CC configuration container.
*
*
****************************************************************************/
static void XDfeCcf_SetNextCCCfg(const XDfeCcf *InstancePtr,
				 const XDfeCcf_CCCfg *NextCCCfg)
{
	u32 AntennaCfg = 0U;
	u32 CarrierCfg;
	u32 Index;
	u32 SeqLength;
	s32 NextCCID[XDFECCF_SEQ_LENGTH_MAX];
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(NextCCCfg != NULL);

	/* Prepare NextCCID[] to be written to registers */
	for (Index = 0U; Index < XDFECCF_CC_NUM; Index++) {
		if ((NextCCCfg->Sequence.CCID[Index] ==
		     XDFECCF_SEQUENCE_ENTRY_NULL) ||
		    (Index >= InstancePtr->SequenceLength)) {
			NextCCID[Index] = InstancePtr->NotUsedCCID;
		} else {
			NextCCID[Index] = NextCCCfg->Sequence.CCID[Index];
		}
	}

	/* Sequence Length should remain the same, so copy the sequence length
	   from CURRENT to NEXT */
	SeqLength =
		XDfeCcf_ReadReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_CURRENT);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_NEXT, SeqLength);

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		XDfeCcf_WriteReg(InstancePtr,
				 XDFECCF_SEQUENCE_NEXT + (sizeof(u32) * Index),
				 NextCCID[Index]);

		CarrierCfg =
			XDfeCcf_WrBitField(XDFECCF_ENABLE_WIDTH,
					   XDFECCF_ENABLE_OFFSET, 0U,
					   NextCCCfg->CarrierCfg[Index].Enable);
		CarrierCfg =
			XDfeCcf_WrBitField(XDFECCF_FLUSH_WIDTH,
					   XDFECCF_FLUSH_OFFSET, CarrierCfg,
					   NextCCCfg->CarrierCfg[Index].Flush);
		CarrierCfg = XDfeCcf_WrBitField(
			XDFECCF_MAPPED_ID_WIDTH, XDFECCF_MAPPED_ID_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].MappedId);
		CarrierCfg =
			XDfeCcf_WrBitField(XDFECCF_GAIN_WIDTH,
					   XDFECCF_GAIN_OFFSET, CarrierCfg,
					   NextCCCfg->CarrierCfg[Index].Gain);
		CarrierCfg = XDfeCcf_WrBitField(
			XDFECCF_IM_COEFF_SET_WIDTH, XDFECCF_IM_COEFF_SET_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].ImagCoeffSet);
		CarrierCfg = XDfeCcf_WrBitField(
			XDFECCF_RE_COEFF_SET_WIDTH, XDFECCF_RE_COEFF_SET_OFFSET,
			CarrierCfg, NextCCCfg->CarrierCfg[Index].RealCoeffSet);
		XDfeCcf_WriteReg(InstancePtr,
				 XDFECCF_CARRIER_CONFIGURATION_NEXT +
					 (sizeof(u32) * Index),
				 CarrierCfg);
	}

	/* Write Antenna configuration */
	for (Index = 0; Index < XDFECCF_ANT_NUM_MAX; Index++) {
		AntennaCfg += ((NextCCCfg->AntennaCfg[Index] & 0x01U) << Index);
	}
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_ANTENNA_CONFIGURATION_NEXT,
			 AntennaCfg);
}

/****************************************************************************/
/**
*
* Returns the next available mapped ID value for the specified CC
* configuration.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
* @param    CCCfg is a CC configuration container.
* @param    ID is mapped index.
*
* @return   Mapped ID.
*
****************************************************************************/
static u32 XDfeCcf_NextMappedId(const XDfeCcf *InstancePtr,
				const XDfeCcf_CCCfg *CCCfg, u32 *ID)
{
	u32 Used[XDFECCF_CC_NUM];
	u32 Val;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);

	/* For the specified CC configuration returns the next available
	   "mapped" ID. This is the ID value used by the hard block. Provides
	   the facility to use any of the CCID values defined at the system
	   level and map to them to the available hardware IDs. The available
	   hardware IDs are contrained based IP parameters and are required to
	   be enumerated from 0. */
	for (Index = 0U; Index < InstancePtr->Config.NumCCPerAntenna; Index++) {
		Used[Index] = 0U;
	}
	for (Index = 0U; Index < CCCfg->Sequence.Length; Index++) {
		if (CCCfg->Sequence.CCID[Index] ==
		    XDFECCF_SEQUENCE_ENTRY_NULL) {
			continue;
		}
		Val = CCCfg->Sequence.CCID[Index];
		if (0U != CCCfg->CarrierCfg[Val].Enable) {
			if (CCCfg->CarrierCfg[Val].MappedId <
			    InstancePtr->Config.NumCCPerAntenna) {
				Used[CCCfg->CarrierCfg[Val].MappedId] = 1U;
			} else {
				metal_log(METAL_LOG_ERROR,
					  "Failure on MappedId in %s\n",
					  __func__);
				/* exit with error, this shouldn't occur if
				   the API has been used */
				return XST_FAILURE;
			}
		}
	}
	/* Return first available ID */
	for (Index = 0U; Index < InstancePtr->Config.NumCCPerAntenna; Index++) {
		if (0U == Used[Index]) {
			*ID = Index;
			return XST_SUCCESS;
		}
	}
	metal_log(METAL_LOG_ERROR, "HW is over allocated %s\n", __func__);
	/* If all in use exit with error. Hardware is over allocated. IP
	   parameterization is such that it can't support the system request */
	return XST_FAILURE;
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of update trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeCcf_EnableCCUpdateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	/* Exit with error if CC_UPDATE status is high */
	if (XDFECCF_CC_UPDATE_TRIGGERED_HIGH == XDfeCcf_RdRegBitField(
			InstancePtr, XDFECCF_ISR,
			XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
			XDFECCF_CC_UPDATE_TRIGGERED_OFFSET)) {
		metal_log(METAL_LOG_ERROR, "CCUpdate status high in %s\n", __func__);
		return XST_FAILURE;
	}

	/* Enable CCUpdate trigger */
	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET, Data);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of LowPower trigger.
* If Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_EnableLowPowerTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set enable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_EnableActivateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  XDFECCF_TRIGGERS_STATE_OUTPUT_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET, Data);
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
static void XDfeCcf_EnableDeactivateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  XDFECCF_TRIGGERS_STATE_OUTPUT_DISABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers and resets enable a bit of LowPower trigger.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_DisableLowPowerTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API initialises one instance of a channel filter driver.
* Traverses "/sys/bus/platform/device" directory (in Linux), to find registered
* CCF device with the name DeviceNodeName. The first available slot in
* the instances array XDfeCcf_ChFilter[] will be taken as a DeviceNodeName
* object.
*
* @param    DeviceNodeName is the device node name.
*
* @return
*           - Pointer to the instance if successful.
*           - NULL on error.
*
******************************************************************************/
XDfeCcf *XDfeCcf_InstanceInit(const char *DeviceNodeName)
{
	u32 Index;
	XDfeCcf *InstancePtr;
#ifdef __BAREMETAL__
	char Str[XDFECCF_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;
#endif

	Xil_AssertNonvoid(DeviceNodeName != NULL);
	Xil_AssertNonvoid(strlen(DeviceNodeName) <
			  XDFECCF_NODE_NAME_MAX_LENGTH);

	/* Is this first CCF initialisation ever? */
	if (0U == XDfeCcf_DriverHasBeenRegisteredOnce) {
		/* Set up environment to non-initialized */
		for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
			XDfeCcf_ChFilter[Index].StateId =
				XDFECCF_STATE_NOT_READY;
			XDfeCcf_ChFilter[Index].NodeName[0] = '\0';
		}
		XDfeCcf_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check has DeviceNodeName been already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
		if (0U == strncmp(XDfeCcf_ChFilter[Index].NodeName,
				  DeviceNodeName, strlen(DeviceNodeName))) {
			return &XDfeCcf_ChFilter[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
		if (XDfeCcf_ChFilter[Index].NodeName[0] == '\0') {
			strncpy(XDfeCcf_ChFilter[Index].NodeName,
				DeviceNodeName, strlen(DeviceNodeName));
			InstancePtr = &XDfeCcf_ChFilter[Index];
			goto register_metal;
		}
	}

	/* Failing as there is no available slot. */
	return NULL;

register_metal:
#ifdef __BAREMETAL__
	memcpy(Str, InstancePtr->NodeName, XDFECCF_NODE_NAME_MAX_LENGTH);
	AddrStr = strtok(Str, ".");
	Addr = strtol(AddrStr, NULL, 16);
	for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
		if (Addr == metal_phys[Index]) {
			InstancePtr->Device = &CustomDevice[Index];
			goto bm_register_metal;
		}
	}
	return NULL;
bm_register_metal:
#endif

	/* Register libmetal for this OS process */
	if (XST_SUCCESS != XDfeCcf_RegisterMetal(InstancePtr,
						 &InstancePtr->Device,
						 DeviceNodeName)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeCcf_LookupConfig(InstancePtr)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to configure device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Configure HW and the driver instance */
	XDfeCcf_CfgInitialize(InstancePtr);

	InstancePtr->StateId = XDFECCF_STATE_READY;

	return InstancePtr;

return_error:
	InstancePtr->StateId = XDFECCF_STATE_NOT_READY;
	InstancePtr->NodeName[0] = '\0';
	return NULL;
}

/*****************************************************************************/
/**
*
* API closes the instances of a Ccf driver.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
*
*
******************************************************************************/
void XDfeCcf_InstanceClose(XDfeCcf *InstancePtr)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
		/* Find the instance in XDfeCcf_ChFilter array */
		if (&XDfeCcf_ChFilter[Index] == InstancePtr) {
			/* Release libmetal */
			metal_device_close(InstancePtr->Device);
			InstancePtr->StateId = XDFECCF_STATE_NOT_READY;
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
* This function puts block into a reset state.
*
* @param    InstancePtr is a pointer to the Ccf instance.
*
*
****************************************************************************/
void XDfeCcf_Reset(XDfeCcf *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFECCF_STATE_NOT_READY);

	/* Put Ccf in reset */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_RESET_OFFSET, XDFECCF_RESET_ON);
	InstancePtr->StateId = XDFECCF_STATE_RESET;
}

/****************************************************************************/
/**
*
* Reads configuration from device tree/xparameters.h and IP registers.
* S/W reset removed.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Cfg is a configuration data container.
*
*
****************************************************************************/
void XDfeCcf_Configure(XDfeCcf *InstancePtr, XDfeCcf_Cfg *Cfg)
{
	u32 Version;
	u32 ModelParam;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_RESET);
	Xil_AssertVoid(Cfg != NULL);

	/* Read vearsion */
	Version = XDfeCcf_ReadReg(InstancePtr, XDFECCF_VERSION_OFFSET);
	Cfg->Version.Patch =
		XDfeCcf_RdBitField(XDFECCF_VERSION_PATCH_WIDTH,
				   XDFECCF_VERSION_PATCH_OFFSET, Version);
	Cfg->Version.Revision =
		XDfeCcf_RdBitField(XDFECCF_VERSION_REVISION_WIDTH,
				   XDFECCF_VERSION_REVISION_OFFSET, Version);
	Cfg->Version.Minor =
		XDfeCcf_RdBitField(XDFECCF_VERSION_MINOR_WIDTH,
				   XDFECCF_VERSION_MINOR_OFFSET, Version);
	Cfg->Version.Major =
		XDfeCcf_RdBitField(XDFECCF_VERSION_MAJOR_WIDTH,
				   XDFECCF_VERSION_MAJOR_OFFSET, Version);

	ModelParam = XDfeCcf_ReadReg(InstancePtr, XDFECCF_MODEL_PARAM_OFFSET);
	InstancePtr->Config.NumAntenna =
		XDfeCcf_RdBitField(XDFECCF_MODEL_PARAM_NUM_ANTENNA_WIDTH,
				   XDFECCF_MODEL_PARAM_NUM_ANTENNA_OFFSET,
				   ModelParam);
	InstancePtr->Config.NumCCPerAntenna = XDfeCcf_RdBitField(
		XDFECCF_MODEL_PARAM_NUM_CC_PER_ANTENNA_WIDTH,
		XDFECCF_MODEL_PARAM_NUM_CC_PER_ANTENNA_OFFSET, ModelParam);
	InstancePtr->Config.AntenaInterleave = XDfeCcf_RdBitField(
		XDFECCF_MODEL_PARAM_ANTENNA_INTERLEAVE_WIDTH,
		XDFECCF_MODEL_PARAM_ANTENNA_INTERLEAVE_OFFSET, ModelParam);

	/* Copy configs model parameters from InstancePtr */
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.NumCCPerAntenna = InstancePtr->Config.NumCCPerAntenna;
	Cfg->ModelParams.AntenaInterleave =
		InstancePtr->Config.AntenaInterleave;

	/* Release RESET */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_RESET_OFFSET, XDFECCF_RESET_OFF);
	InstancePtr->StateId = XDFECCF_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE Ccf driver one time initialisation.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Init is an initialisation data container.
*
*
****************************************************************************/
void XDfeCcf_Initialize(XDfeCcf *InstancePtr, XDfeCcf_Init *Init)
{
	XDfeCcf_Trigger CCUpdate;
	u32 Data;
	u32 Index;
	u32 Offset;
	u32 SequenceLength;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_CONFIGURED);
	Xil_AssertVoid(Init != NULL);

	/* Write "one-time" configuration */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_GAIN_STG_EN_OFFSET,
			 Init->GainStage);

	/* Write "one-time" Sequence length */
	InstancePtr->NotUsedCCID = 0;
	InstancePtr->SequenceLength = Init->Sequence.Length;
	if (Init->Sequence.Length == 0) {
		SequenceLength = 0U;
	} else {
		SequenceLength = Init->Sequence.Length - 1U;
	}
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_NEXT,
			 SequenceLength);

	/* Set default sequence and ensure all CCs are disabled. Not all
	   registers will be cleared by reset as they are implemented using
	   DRAM. This step sets all CC_CONFIGURATION.CARRIER_CONFIGURATION.
	   CURRENT[*]. ENABLE to 0 ensuring the Hardblock will remain disabled
	   following the first call to XDFECCFilterActivate. */
	for (Index = 0; Index < XDFECCF_SEQ_LENGTH_MAX; Index++) {
		Offset = XDFECCF_SEQUENCE_NEXT + (sizeof(u32) * Index);
		XDfeCcf_WriteReg(InstancePtr, Offset,
				 XDFECCF_SEQUENCE_ENTRY_DEFAULT);
		Init->Sequence.CCID[Index] = XDFECCF_SEQUENCE_ENTRY_NULL;
	}
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		Offset = XDFECCF_CARRIER_CONFIGURATION_NEXT +
			 (sizeof(u32) * Index);
		XDfeCcf_WrRegBitField(InstancePtr, Offset, XDFECCF_ENABLE_WIDTH,
				      XDFECCF_ENABLE_OFFSET,
				      XDFECCF_ENABLE_DISABLED);
	}

	/* Trigger CC_UPDATE immediately using Register source to update
	   CURRENT from NEXT */
	CCUpdate.TriggerEnable = XDFECCF_TRIGGERS_TRIGGER_ENABLE_ENABLED;
	CCUpdate.Mode = XDFECCF_TRIGGERS_MODE_IMMEDIATE;
	CCUpdate.StateOutput = XDFECCF_TRIGGERS_STATE_OUTPUT_ENABLED;
	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  CCUpdate.StateOutput);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  CCUpdate.TriggerEnable);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_MODE_WIDTH,
				  XDFECCF_TRIGGERS_MODE_OFFSET, Data,
				  CCUpdate.Mode);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET, Data);

	InstancePtr->StateId = XDFECCF_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activates channel filter.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    EnableLowPower is an flag indicating low power.
*
******************************************************************************/
void XDfeCcf_Activate(XDfeCcf *InstancePtr, bool EnableLowPower)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFECCF_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL));

	/* Do nothing if the block already operational */
	IsOperational = XDfeCcf_RdRegBitField(
		InstancePtr, XDFECCF_STATE_OPERATIONAL_OFFSET,
		XDFECCF_STATE_OPERATIONAL_BITFIELD_WIDTH,
		XDFECCF_STATE_OPERATIONAL_BITFIELD_OFFSET);
	if (IsOperational == XDFECCF_STATE_OPERATIONAL_YES) {
		return;
	}

	/* Enable the Activate trigger and set to one-shot */
	XDfeCcf_EnableActivateTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfeCcf_EnableLowPowerTrigger(InstancePtr);
	}

	/* Channel filter is operational now, change a state */
	InstancePtr->StateId = XDFECCF_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* Deactivates channel filter.
*
* @param    InstancePtr is a pointer to the Ccf instance.
*
******************************************************************************/
void XDfeCcf_Deactivate(XDfeCcf *InstancePtr)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFECCF_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL));

	/* Do nothing if the block already deactivated */
	IsOperational = XDfeCcf_RdRegBitField(
		InstancePtr, XDFECCF_STATE_OPERATIONAL_OFFSET,
		XDFECCF_STATE_OPERATIONAL_BITFIELD_WIDTH,
		XDFECCF_STATE_OPERATIONAL_BITFIELD_OFFSET);
	if (IsOperational == XDFECCF_STATE_OPERATIONAL_NO) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfeCcf_DisableLowPowerTrigger(InstancePtr);

	/* Disable Activate trigger */
	XDfeCcf_EnableDeactivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFECCF_STATE_INITIALISED;
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
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
* @param    BitSequence maps the sequence.
* @param    CarrierCfg is a CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_AddCC(XDfeCcf *InstancePtr, s32 CCID, u32 BitSequence,
		  const XDfeCcf_CarrierCfg *CarrierCfg)
{
	XDfeCcf_CCCfg CCCfg;
	u32 AddSuccess;
	u32 IDAvailable;
	u32 NextMappedID = 0;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID <= XDFECCF_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Read current CC configuration. Note that XDfeCcf_Initialise writes
	   a NULL CC sequence to H/W */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Get next mapped ID. This should be done before trying to add a new
	   CCID */
	IDAvailable = XDfeCcf_NextMappedId(InstancePtr, &CCCfg, &NextMappedID);
	if (IDAvailable == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "ID is not available in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeCcf_AddCCID(InstancePtr, CCID, BitSequence,
				     &CCCfg.Sequence);
	if (AddSuccess == (u32)XST_SUCCESS) {
		/* Update carrier configuration, mark flush as we need to clear
		   data registers */
		CCCfg.CarrierCfg[CCID].Gain = CarrierCfg->Gain;
		CCCfg.CarrierCfg[CCID].ImagCoeffSet = CarrierCfg->ImagCoeffSet;
		CCCfg.CarrierCfg[CCID].RealCoeffSet = CarrierCfg->RealCoeffSet;
		CCCfg.CarrierCfg[CCID].Enable = 1U;
		CCCfg.CarrierCfg[CCID].Flush = 1U;
		CCCfg.CarrierCfg[CCID].MappedId = NextMappedID;
	} else {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Clear Flush on other active CCs */
	for (Index = 0U; Index < CCCfg.Sequence.Length; Index++) {
		if ((u32)CCID != Index) {
			CCCfg.CarrierCfg[CCID].Flush = 0;
		}
	}

	/* If add is successful update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Removes specified CCID.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_RemoveCC(XDfeCcf *InstancePtr, s32 CCID)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFECCF_CC_NUM);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeCcf_RemoveCCID(InstancePtr, CCID, &CCCfg.Sequence);
	CCCfg.CarrierCfg[CCID].Enable = 0U;

	/* Update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Updates specified CCID carrier configuration; change gain or filter
* coefficients set.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
* @param    CarrierCfg is a CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_UpdateCC(const XDfeCcf *InstancePtr, s32 CCID,
		      XDfeCcf_CarrierCfg *CarrierCfg)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFECCF_CC_NUM);
	Xil_AssertVoid(CarrierCfg != NULL);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Update carrier configuration. Set flush if coefficient set has
	   changed */
	if ((CarrierCfg->RealCoeffSet != CCCfg.CarrierCfg[CCID].RealCoeffSet) ||
	    (CarrierCfg->ImagCoeffSet != CCCfg.CarrierCfg[CCID].ImagCoeffSet)) {
		CarrierCfg->Flush = 1U;
	} else {
		CarrierCfg->Flush = 0U;
	}

	CCCfg.CarrierCfg[CCID].Enable = CarrierCfg->Enable;
	CCCfg.CarrierCfg[CCID].Flush = CarrierCfg->Flush;
	CCCfg.CarrierCfg[CCID].MappedId = CarrierCfg->MappedId;
	CCCfg.CarrierCfg[CCID].Rate = CarrierCfg->Rate;
	CCCfg.CarrierCfg[CCID].Gain = CarrierCfg->Gain;
	CCCfg.CarrierCfg[CCID].ImagCoeffSet = CarrierCfg->ImagCoeffSet;
	CCCfg.CarrierCfg[CCID].RealCoeffSet = CarrierCfg->RealCoeffSet;

	/* Update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Updates specified antenna TDM slot enablement.
*
* Initiates CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Ant is antenna ID.
* @param    Enabled is a flag indicating enable status of the antenna.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_UpdateAntenna(const XDfeCcf *InstancePtr, u32 Ant, bool Enabled)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(Ant <= XDFECCF_ANT_NUM_MAX);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Update antenna enablement */
	if (Enabled == true) {
		CCCfg.AntennaCfg[Ant] = 1U;
	} else {
		CCCfg.AntennaCfg[Ant] = 0U;
	}

	/* Update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	return XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Returns current trigger configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    TriggerCfg is a trigger configuration container.
*
*
****************************************************************************/
void XDfeCcf_GetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFECCF_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read ACTIVATE triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	TriggerCfg->Activate.TriggerEnable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->Activate.Mode = XDfeCcf_RdBitField(
		XDFECCF_TRIGGERS_MODE_WIDTH, XDFECCF_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.TuserEdgeLevel =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->Activate.StateOutput =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read LOW_POWER triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	TriggerCfg->LowPower.TriggerEnable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->LowPower.Mode = XDfeCcf_RdBitField(
		XDFECCF_TRIGGERS_MODE_WIDTH, XDFECCF_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.TuserEdgeLevel =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->LowPower.StateOutput =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read CC_UPDATE triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	TriggerCfg->CCUpdate.TriggerEnable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->CCUpdate.Mode = XDfeCcf_RdBitField(
		XDFECCF_TRIGGERS_MODE_WIDTH, XDFECCF_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->CCUpdate.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->CCUpdate.TuserEdgeLevel =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->CCUpdate.StateOutput =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets trigger configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    TriggerCfg is a trigger configuration container.
*
*
****************************************************************************/
void XDfeCcf_SetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Write public trigger configuration members and ensure private members
	  (TriggerEnable & Immediate) are set appropriately */

	/* Activate defined as Single Shot/Immediate (as per the programming model) */
	TriggerCfg->Activate.TriggerEnable =
		XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->Activate.StateOutput =
		XDFECCF_TRIGGERS_STATE_OUTPUT_ENABLED;
	/* Read/set/write ACTIVATE triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->Activate.TriggerEnable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_MODE_WIDTH,
				 XDFECCF_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->Activate.Mode);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->Activate.TuserEdgeLevel);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->Activate.TUSERBit);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->Activate.StateOutput);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET, Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.TriggerEnable =
		XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->LowPower.Mode = XDFECCF_TRIGGERS_MODE_TUSER_CONTINUOUS;
	/* Read LOW_POWER triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->LowPower.TriggerEnable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_MODE_WIDTH,
				 XDFECCF_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->LowPower.Mode);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->LowPower.TuserEdgeLevel);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->LowPower.TUSERBit);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->LowPower.StateOutput);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Val);

	/* CCUpdate defined as Single Shot/Immediate */
	TriggerCfg->CCUpdate.TriggerEnable =
		XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->CCUpdate.StateOutput =
		XDFECCF_TRIGGERS_STATE_OUTPUT_ENABLED;
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->CCUpdate.TriggerEnable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_MODE_WIDTH,
				 XDFECCF_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->CCUpdate.Mode);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->CCUpdate.TuserEdgeLevel);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->CCUpdate.TUSERBit);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->CCUpdate.StateOutput);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Gets specified CCID carrier configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
* @param    CarrierCfg is a trigger configuration container.
*
*
****************************************************************************/
void XDfeCcf_GetCC(const XDfeCcf *InstancePtr, s32 CCID,
		   XDfeCcf_CarrierCfg *CarrierCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCID <= XDFECCF_CC_NUM);
	Xil_AssertVoid(CarrierCfg != NULL);

	/* Read specified CCID carrier configuration */
	Val = XDfeCcf_ReadReg(InstancePtr,
			      XDFECCF_CARRIER_CONFIGURATION_CURRENT +
				      (sizeof(u32) * CCID));
	CarrierCfg->Enable = XDfeCcf_RdBitField(XDFECCF_ENABLE_WIDTH,
						XDFECCF_ENABLE_OFFSET, Val);
	CarrierCfg->Flush = XDfeCcf_RdBitField(XDFECCF_FLUSH_WIDTH,
					       XDFECCF_FLUSH_OFFSET, Val);
	CarrierCfg->MappedId = XDfeCcf_RdBitField(
		XDFECCF_MAPPED_ID_WIDTH, XDFECCF_MAPPED_ID_OFFSET, Val);
	CarrierCfg->RealCoeffSet = XDfeCcf_RdBitField(
		XDFECCF_RE_COEFF_SET_WIDTH, XDFECCF_RE_COEFF_SET_OFFSET, Val);
	CarrierCfg->ImagCoeffSet = XDfeCcf_RdBitField(
		XDFECCF_IM_COEFF_SET_WIDTH, XDFECCF_IM_COEFF_SET_OFFSET, Val);
	CarrierCfg->Gain = XDfeCcf_RdBitField(XDFECCF_GAIN_WIDTH,
					      XDFECCF_GAIN_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Returns a list indicating which coefficient sets are currently in use.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    IsActive is a pointer indicating an activation status.
*
****************************************************************************/
void XDfeCcf_GetActiveSets(const XDfeCcf *InstancePtr, u32 *IsActive)
{
	u32 Offset;
	u32 Index;
	u32 SeqLen;
	s32 CCID;
	u32 Val;
	u32 ReCoefSet;
	u32 ImCoefSet;
	u32 Enable;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(IsActive != NULL);

	/* Clear return value */
	for (Index = 0; Index < XDFECCF_ACTIVE_SET_NUM; Index++) {
		IsActive[Index] = 0U;
	}

	/* Read all coefficient sets in use and update IsActive */
	SeqLen = XDfeCcf_ReadReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_CURRENT);
	for (Index = 0; Index < SeqLen; Index++) {
		Offset = XDFECCF_CARRIER_CONFIGURATION_CURRENT +
			 (sizeof(u32) * Index);
		Enable = XDfeCcf_RdRegBitField(InstancePtr, Offset,
					       XDFECCF_ENABLE_OFFSET,
					       XDFECCF_ENABLE_WIDTH);
		if (Enable == XDFECCF_ENABLE_DISABLED) {
			/* Skip this CC, it's not enabled */
			continue;
		}

		/* CC is enabled, than activate coeficients */
		Offset = XDFECCF_SEQUENCE_CURRENT + (sizeof(u32) * Index);
		CCID = XDfeCcf_ReadReg(InstancePtr, Offset);
		Offset = XDFECCF_CARRIER_CONFIGURATION_CURRENT +
			 (sizeof(u32) * CCID);
		Val = XDfeCcf_ReadReg(InstancePtr, Offset);
		ReCoefSet =
			XDfeCcf_RdBitField(XDFECCF_RE_COEFF_SET_WIDTH,
					   XDFECCF_RE_COEFF_SET_OFFSET, Val);
		IsActive[ReCoefSet] = 1U;
		ImCoefSet =
			XDfeCcf_RdBitField(XDFECCF_IM_COEFF_SET_WIDTH,
					   XDFECCF_IM_COEFF_SET_OFFSET, Val);
		IsActive[ImCoefSet] = 1U;
	}
}

/****************************************************************************/
/**
*
* Writes the coefficient set defined into the register map and commit them
* to the hard block's internal coefficient memory for the specified Set.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Set is a coefficient set Id.
* @param    Coeffs is an array of filter coefficients.
*
*
****************************************************************************/
void XDfeCcf_LoadCoefficients(const XDfeCcf *InstancePtr, u32 Set,
			      const XDfeCcf_Coefficients *Coeffs)
{
	u32 NumValues;
	u32 IsOdd;
	s32 CoeffSum = 0;
	double ScaleFactor;
	u32 Shift;
	u32 LoadActive;
	u32 Val;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coeffs != NULL);
	Xil_AssertVoid(Coeffs->Num != 0U); /* Protect from division with 0 */

	/* Determine scale shift value using expression defined in
	   Channel Filter FIR (R16) #DetailedDescription */
	IsOdd = Coeffs->Num % 2U;
	if (0U != Coeffs->Symmetric) {
		NumValues = (Coeffs->Num + 1U) / 2U;
	} else {
		NumValues = Coeffs->Num;
	}

	for (Index = 0; Index < NumValues; Index++) {
		CoeffSum += Coeffs->Value[Index];
	}

	if (0U != Coeffs->Symmetric) {
		CoeffSum = 2 * CoeffSum;
		if (0U != IsOdd) {
			CoeffSum = CoeffSum - Coeffs->Value[NumValues - 1U];
		}
	}

	ScaleFactor = (double)CoeffSum / (256U * ((u32)1 << 15));
	Shift = (u32)floor(fabs(log2(ScaleFactor)));

	/* Check is load in progress */
	for (Index = 0; Index < XDFECCF_COEFF_LOAD_TIMEOUT; Index++) {
		LoadActive = XDFECCF_STATUS_WIDTH &
			     XDfeCcf_ReadReg(InstancePtr, XDFECCF_COEFF_LOAD);
		if (XDFECCF_STATUS_LOADING != LoadActive) {
			break;
		}
		usleep(10);
		if (Index == (XDFECCF_COEFF_LOAD_TIMEOUT - 1U)) {
			Xil_AssertVoidAlways();
		}
	}

	/* When no load is active write filter coefficients and initiate load */
	Val = XDfeCcf_WrBitField(XDFECCF_NUMBER_UNITS_WIDTH,
				 XDFECCF_NUMBER_UNITS_OFFSET, 0U,
				 (NumValues + 3U) / 4U);
	Val = XDfeCcf_WrBitField(XDFECCF_SHIFT_VALUE_WIDTH,
				 XDFECCF_SHIFT_VALUE_OFFSET, Val, Shift);
	Val = XDfeCcf_WrBitField(XDFECCF_IS_SYMMETRIC_WIDTH,
				 XDFECCF_IS_SYMMETRIC_OFFSET, Val,
				 Coeffs->Symmetric);
	Val = XDfeCcf_WrBitField(XDFECCF_USE_ODD_TAPS_WIDTH,
				 XDFECCF_USE_ODD_TAPS_OFFSET, Val, IsOdd);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_COEFF_CFG, Val);

	for (Index = 0; Index < NumValues; Index++) {
		XDfeCcf_WriteReg(InstancePtr,
				 XDFECCF_COEFF_VALUE + (sizeof(u32) * Index),
				 Coeffs->Value[Index]);
	}

	/* Set the coefficient set value */
	XDfeCcf_WrRegBitField(InstancePtr, XDFECCF_COEFF_LOAD,
			      XDFECCF_SET_NUM_WIDTH, XDFECCF_SET_NUM_OFFSET,
			      Set);
	/* Load coefficients */
	XDfeCcf_WrRegBitField(InstancePtr, XDFECCF_COEFF_LOAD,
			      XDFECCF_STATUS_WIDTH, XDFECCF_STATUS_OFFSET, 1U);
}

/*****************************************************************************/
/**
*
* This API is used to get the driver version.
*
* @param    SwVersion is driver version numbers.
* @param    HwVersion is HW version numbers.
*
*
******************************************************************************/
void XDfeCcf_GetVersions(const XDfeCcf *InstancePtr, XDfeCcf_Version *SwVersion,
			 XDfeCcf_Version *HwVersion)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr->StateId != XDFECCF_STATE_NOT_READY);

	/* Driver version */
	SwVersion->Major = XDFECCF_DRIVER_VERSION_MAJOR;
	SwVersion->Minor = XDFECCF_DRIVER_VERSION_MINOR;

	/* Component HW version */
	Version = XDfeCcf_ReadReg(InstancePtr, XDFECCF_VERSION_OFFSET);
	HwVersion->Patch =
		XDfeCcf_RdBitField(XDFECCF_VERSION_PATCH_WIDTH,
				   XDFECCF_VERSION_PATCH_OFFSET, Version);
	HwVersion->Revision =
		XDfeCcf_RdBitField(XDFECCF_VERSION_REVISION_WIDTH,
				   XDFECCF_VERSION_REVISION_OFFSET, Version);
	HwVersion->Minor =
		XDfeCcf_RdBitField(XDFECCF_VERSION_MINOR_WIDTH,
				   XDFECCF_VERSION_MINOR_OFFSET, Version);
	HwVersion->Major =
		XDfeCcf_RdBitField(XDFECCF_VERSION_MAJOR_WIDTH,
				   XDFECCF_VERSION_MAJOR_OFFSET, Version);
}
/** @} */
