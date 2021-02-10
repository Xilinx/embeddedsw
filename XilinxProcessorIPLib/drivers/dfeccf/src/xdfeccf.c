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
* </pre>
*
******************************************************************************/

#include "xdfeccf.h"
#include "xdfeccf_hw.h"
#include <math.h>
#include <metal/io.h>
#include <metal/device.h>

#ifdef __BAREMETAL__
#include "sleep.h"
#else
#include <unistd.h>
#endif

/**************************** Macros Definitions ****************************/
#define XDFECCF_SEQUENCE_ENTRY_NULL 8U /**< Null sequence entry flag */
#define XDFECCF_NO_EMPTY_CCID_FLAG 0xFFFFU /**< Not Empty CCID flag */
#define XDFECCF_COEFF_LOAD_TIMEOUT 100 /**< Units of 10us */
#define XDFECCF_ACTIVE_SET_NUM 8U /**< Maximum number of active sets */
#define XDFECCF_U32_NUM_BITS 32U

#define XDFECCF_DRIVER_VERSION_MINOR 0U
#define XDFECCF_DRIVER_VERSION_MAJOR 1U

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFECCF_MAX_NUM_INSTANCES];
#endif
static struct metal_device *DevicePtrStorage[XDFECCF_MAX_NUM_INSTANCES];
extern XDfeCcf XDfeCcf_ChFilter[XDFECCF_MAX_NUM_INSTANCES];
static u32 XDfeCcf_DriverHasBeenRegisteredOnce = 0U;

/************************** Function Definitions ****************************/
extern u32 XDfeCcf_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr,
				 const char *DeviceNodeName);
extern u32 XDfeCcf_LookupConfig(u16 DeviceId);
extern u32 XDfeCcf_CfgInitialize(XDfeCcf *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Write value to register in a Ccf instance.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    AddrOffset is address offset relative to instance base address.
* @param    Data is value to be written.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_WriteReg(const XDfeCcf *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Read a value from register from a Ccf instance.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    AddrOffset is address offset relative to instance base address.
*
* @return   Register value.
*
* @note     None
*
****************************************************************************/
u32 XDfeCcf_ReadReg(const XDfeCcf *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, AddrOffset);
}

/****************************************************************************/
/**
*
* Write a bit field value to register.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
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
* Read a bit field value from register.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
*
* @return   Bit field data.
*
* @note     None
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
u32 XDfeCcf_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFECCF_U32_NUM_BITS);
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
static void XDfeCcf_CreateCCSequence(u32 SeqLen,
				     XDfeCcf_CCSequence *CCIDSequence)
{
	u32 Index;
	Xil_AssertVoid(SeqLen < XDFECCF_SEQ_LENGTH_MAX);
	Xil_AssertVoid(CCIDSequence != NULL);

	/* Set sequence length and mark all sequence entries as null (8) */
	CCIDSequence->Length = SeqLen;
	for (Index = 0; Index < XDFECCF_SEQ_LENGTH_MAX; Index++) {
		CCIDSequence->CCID[Index] = XDFECCF_SEQUENCE_ENTRY_NULL;
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
static u32 XDfeCcf_AddCCID(u32 CCID, u32 Rate, XDfeCcf_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 SeqLen;
	u32 SeqMult = 1;
	u32 CCIDCount;
	u32 EmptyCCID = XDFECCF_NO_EMPTY_CCID_FLAG;
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
		if (CCIDSequence->CCID[Index] == XDFECCF_SEQUENCE_ENTRY_NULL) {
			EmptyCCID = Index;
			break;
		}
	}

	if (EmptyCCID == XDFECCF_NO_EMPTY_CCID_FLAG) {
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
		if ((SeqMult * SeqLen) > XDFECCF_SEQ_LENGTH_MAX) {
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
		if (CCIDSequence->CCID[Index] == XDFECCF_SEQUENCE_ENTRY_NULL) {
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
		if (CCIDSequence->CCID[Index] == XDFECCF_SEQUENCE_ENTRY_NULL) {
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
static void XDfeCcf_RemoveCCID(u32 CCID, XDfeCcf_CCSequence *CCIDSequence)
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
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Create a "null" CC configuration with the specified sequence length.
*
* @param    SequenceLength is a sequence length.
* @param    CCCfg is CC configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeCcf_CreateCCCfg(u32 SequenceLength, XDfeCcf_CCCfg *CCCfg)
{
	u32 Index;
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(SequenceLength <= XDFECCF_SEQ_LENGTH_MAX);

	/* Create null sequence of SequenceLength and populate CCfg */
	XDfeCcf_CreateCCSequence(SequenceLength, &CCCfg->Sequence);

	/* Set all carrier configurations to null (disabled) */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		CCCfg->CarrierCfg[Index].Enable = 0U;
	}
}

/****************************************************************************/
/**
*
* Returns the current CC configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CurrCCCfg is CC configuration container.
*
* @return   None
*
* @note     None
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

	/* Read sequence length */
	SeqLen = XDfeCcf_ReadReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_CURRENT);
	CurrCCCfg->Sequence.Length = SeqLen + 1;

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		CurrCCCfg->Sequence.CCID[Index] = XDfeCcf_ReadReg(
			InstancePtr,
			XDFECCF_SEQUENCE_CURRENT + (sizeof(u32) * Index));
		XDfeCcf_GetCC(InstancePtr, Index,
			      &CurrCCCfg->CarrierCfg[Index]);
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
* Sets the next CC configuration
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    NextCCCfg is Next CC configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeCcf_SetNextCCCfg(const XDfeCcf *InstancePtr,
				 const XDfeCcf_CCCfg *NextCCCfg)
{
	u32 SeqLen = 0;
	u32 AntennaCfg = 0U;
	u32 CarrierCfg;
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(NextCCCfg != NULL);

	/* Write sequence length */
	if (0 < NextCCCfg->Sequence.Length) {
		SeqLen = NextCCCfg->Sequence.Length - 1;
	}
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_SEQUENCE_LENGTH_NEXT, SeqLen);

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		XDfeCcf_WriteReg(InstancePtr,
				 XDFECCF_SEQUENCE_NEXT + (sizeof(u32) * Index),
				 NextCCCfg->Sequence.CCID[Index]);

		CarrierCfg =
			XDfeCcf_WrBitField(XDFECCF_ENABLE_WIDTH,
					   XDFECCF_ENABLE_OFFSET, 0U,
					   NextCCCfg->CarrierCfg[Index].Enable);
		CarrierCfg =
			XDfeCcf_WrBitField(XDFECCF_FLUSH_WIDTH,
					   XDFECCF_FLUSH_OFFSET, CarrierCfg,
					   NextCCCfg->CarrierCfg[Index].Flush);
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
* @param    CCCfg is CC configuration container.
* @param    ID is mapped index.
*
* @return   Mapped ID
*
* @note     None
*
****************************************************************************/
static u32 XDfeCcf_NextMappedId(const XDfeCcf *InstancePtr,
				XDfeCcf_CCCfg *CCCfg, u32 *ID)
{
	XDfeCcf_ModelParameters ModelParams;
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
	/* Read IP parameters to determine the maximum number of actual IDs
	   that can be supported by the hardware. */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_MODEL_PARAM_OFFSET);
	ModelParams.NumAntenna =
		XDfeCcf_RdBitField(XDFECCF_MODEL_PARAM_NUM_ANTENNA_WIDTH,
				   XDFECCF_MODEL_PARAM_NUM_ANTENNA_OFFSET, Val);
	ModelParams.NumCCPerAntenna = XDfeCcf_RdBitField(
		XDFECCF_MODEL_PARAM_NUM_CC_PER_ANTENNA_WIDTH,
		XDFECCF_MODEL_PARAM_NUM_CC_PER_ANTENNA_OFFSET, Val);
	ModelParams.NumAntSlot =
		XDfeCcf_RdBitField(XDFECCF_MODEL_PARAM_NUM_SLOT_CHANNELS_WIDTH,
				   XDFECCF_MODEL_PARAM_NUM_SLOT_CHANNELS_OFFSET,
				   Val);

	for (Index = 0U; Index < ModelParams.NumCCPerAntenna; Index++) {
		Used[Index] = 0U;
	}
	for (Index = 0U; Index < CCCfg->Sequence.Length; Index++) {
		Val = CCCfg->Sequence.CCID[Index];
		if (0U != CCCfg->CarrierCfg[Val].Enable) {
			if (CCCfg->CarrierCfg[Val].MappedId <
			    ModelParams.NumCCPerAntenna) {
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
	for (Index = 0U; Index < ModelParams.NumCCPerAntenna; Index++) {
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
* Read Triggers, set enable bit of update trigger. If register source, then
* trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeCcf_EnableCCUpdateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_ENABLE_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Read Triggers, set enable bit of LowPower trigger. If register source, then
* trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeCcf_EnableLowPowerTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_ENABLE_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Data);
}
/****************************************************************************/
/**
*
* Read Triggers, set enable bit of Activate trigger. If register source, then
* trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeCcf_EnableActivateTrigger(XDfeCcf *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_ENABLE_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Read Triggers, reset enable bit of LowPower trigger.
*
* @param    InstancePtr is a pointer to the Channel Filter instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
static void XDfeCcf_DisableLowPowerTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_ENABLE_DISABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API Initialise one instancies of a Ccf driver.
* Traverse "/sys/bus/platform/device" directory, to find CCF device id,
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
XDfeCcf *XDfeCcf_InstanceInit(u16 DeviceId, const char *DeviceNodeName)
{
	u32 Index;

	Xil_AssertNonvoid(DeviceId < XDFECCF_MAX_NUM_INSTANCES);

	/* Is for the First initialisation caled ever */
	if (0U == XDfeCcf_DriverHasBeenRegisteredOnce) {
		/* Set up environment environment */
		for (Index = 0; Index < XDFECCF_MAX_NUM_INSTANCES; Index++) {
			XDfeCcf_ChFilter[Index].StateId =
				XDFECCF_STATE_NOT_READY;
#ifdef __BAREMETAL__
			DevicePtrStorage[Index] = &CustomDevice[Index];
#endif
		}
		XDfeCcf_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check is the instance DeviceID already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	if (XDfeCcf_ChFilter[DeviceId].StateId != XDFECCF_STATE_NOT_READY) {
		return &XDfeCcf_ChFilter[DeviceId];
	}

	/* Register libmetal for this OS process */
	if (XST_SUCCESS != XDfeCcf_RegisterMetal(DeviceId,
						 &DevicePtrStorage[DeviceId],
						 DeviceNodeName)) {
		XDfeCcf_ChFilter[DeviceId].StateId = XDFECCF_STATE_NOT_READY;
		return NULL;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeCcf_LookupConfig(DeviceId)) {
		XDfeCcf_ChFilter[DeviceId].StateId = XDFECCF_STATE_NOT_READY;
		return NULL;
	}

	/* Configure HW and the driver instance */
	XDfeCcf_CfgInitialize(&XDfeCcf_ChFilter[DeviceId]);

	XDfeCcf_ChFilter[DeviceId].StateId = XDFECCF_STATE_READY;

	return &XDfeCcf_ChFilter[DeviceId];
}

/*****************************************************************************/
/**
*
* API Close the instancies of a Ccf driver.
*
* @param    InstancePtr is a pointer to the XDfeCcf instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XDfeCcf_InstanceClose(XDfeCcf *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Close the instance */
	InstancePtr->StateId = XDFECCF_STATE_NOT_READY;

	/* Release libmetal */
	metal_device_close(InstancePtr->Device);

	return;
}

/****************************************************************************/
/**
*
* This function puts block into a reset state.
*
* @param    InstancePtr is a pointer to the Ccf instance.
*
* @return   None
*
* @note     None
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
* Read configuration from device tree/xparameters.h and IP registers.
* S/W reset removed.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Cfg is a configuration data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_Configure(XDfeCcf *InstancePtr, XDfeCcf_Cfg *Cfg)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_RESET);

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

	/* Copy configs model parameters from InstancePtr */
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.NumCCPerAntenna = InstancePtr->Config.NumCCPerAntenna;
	Cfg->ModelParams.NumAntSlot = InstancePtr->Config.NumAntSlot;

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
* @param    Init is a initialisation data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_Initialize(XDfeCcf *InstancePtr, const XDfeCcf_Init *Init)
{
	XDfeCcf_Trigger CCUpdate;
	u32 Data;
	u32 Index;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_CONFIGURED);
	Xil_AssertVoid(Init != NULL);

	/* Write "one-time" configuration */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_GAIN_STG_EN_OFFSET,
			 Init->GainStage);
	/* Set NULL sequence and ensure all CCs are disabled. Not all registers
	   will be cleared by reset as they are implemented using DRAM. This
	   step sets all CC_CONFIGURATION.CARRIER_CONFIGURATION.CURRENT[*].
	   ENABLE to 0 ensuring the Hardblock will remain disabled following
	   the first call to XDFECCFilterActivate. */
	for (Index = 0; Index < XDFECCF_SEQ_LENGTH_MAX; Index++) {
		Offset = XDFECCF_SEQUENCE_NEXT + (sizeof(u32) * Index);
		XDfeCcf_WriteReg(InstancePtr, Offset,
				 XDFECCF_SEQUENCE_ENTRY_NULL);
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
	CCUpdate.Enable = 1U;
	CCUpdate.OneShot = 1U;
	CCUpdate.Source = 0U;
	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				  XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Data,
				  CCUpdate.OneShot);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_ENABLE_OFFSET, Data,
				  CCUpdate.Enable);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_SOURCE_WIDTH,
				  XDFECCF_TRIGGERS_SOURCE_OFFSET, Data,
				  CCUpdate.Source);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET, Data);

	InstancePtr->StateId = XDFECCF_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activate channel filter.
* Note: Writting to ACTIVATE reg.toggles between "initialized" and
*       "operational".
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    EnableLowPower is an falg indicating low power.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeCcf_Activate(XDfeCcf *InstancePtr, bool EnableLowPower)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_INITIALISED);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_STATE_OPERATIONAL_OFFSET);
	if (1U == Data) {
		return;
	}

	/* Enable the Activate trigger and set to one-shot */
	XDfeCcf_EnableActivateTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfeCcf_EnableLowPowerTrigger(InstancePtr);
	}

	/* CCF is operational now, change a state */
	InstancePtr->StateId = XDFECCF_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* DeActivate channel filter.
* Note: Writting to ACTIVATE reg.toggles between "initialized" and
*       "operational".
*
* @param    InstancePtr is a pointer to the Ccf instance.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeCcf_Deactivate(XDfeCcf *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_STATE_OPERATIONAL_OFFSET);
	if (0U == Data) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfeCcf_DisableLowPowerTrigger(InstancePtr);

	/* Enable Activate trigger (toggles state between operational
	   and intialized) */
	XDfeCcf_EnableActivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFECCF_STATE_INITIALISED;
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
* @param    InstancePtr is a pointer to the Ccf instance.
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
u32 XDfeCcf_AddCC(XDfeCcf *InstancePtr, u32 CCID,
		  const XDfeCcf_CarrierCfg *CarrierCfg)
{
	XDfeCcf_CCCfg CCCfg;
	u32 AddSuccess;
	u32 IDAvailable;
	u32 NextMappedID;
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
	if (IDAvailable == XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "ID is not available in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeCcf_AddCCID(CCID, CarrierCfg->Rate, &CCCfg.Sequence);
	if (AddSuccess == XST_SUCCESS) {
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
		if (CCID != Index) {
			CCCfg.CarrierCfg[CCID].Flush = 0;
		}
	}

	/* If add is successful update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Remove specified CCID.
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_RemoveCC(XDfeCcf *InstancePtr, u32 CCID)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(CCID <= XDFECCF_CC_NUM);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeCcf_RemoveCCID(CCID, &CCCfg.Sequence);
	CCCfg.CarrierCfg[CCID].Enable = 0U;

	/* Update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Update specified CCID carrier configuration; change gain or filter
* coefficients set.
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
* @param    CarrierCfg is a CC configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_UpdateCC(XDfeCcf *InstancePtr, u32 CCID,
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
	XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Update specified antenna TDM slot enablement
*
* Initiate CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Ant is antenna ID.
* @param    Enabled is a flag indicating enable status of antenna.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_UpdateAntenna(XDfeCcf *InstancePtr, u32 Ant, bool Enabled)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(Ant <= XDFECCF_ANT_NUM_MAX);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Update antenna enablement */
	CCCfg.AntennaCfg[Ant] = (u32)Enabled;

	/* Update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, &CCCfg);
	XDfeCcf_EnableCCUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Return current trigger configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    TriggerCfg is a triger configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_GetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read ACTIVATE triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	TriggerCfg->Activate.Enable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_ENABLE_OFFSET, Val);
	TriggerCfg->Activate.Source =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_SOURCE_WIDTH,
				   XDFECCF_TRIGGERS_SOURCE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.Edge =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_SIGNAL_EDGE_WIDTH,
				   XDFECCF_TRIGGERS_SIGNAL_EDGE_OFFSET, Val);
	TriggerCfg->Activate.OneShot =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				   XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Val);

	/* Read LOW_POWER triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	TriggerCfg->LowPower.Enable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_ENABLE_OFFSET, Val);
	TriggerCfg->LowPower.Source =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_SOURCE_WIDTH,
				   XDFECCF_TRIGGERS_SOURCE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.Edge =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_SIGNAL_EDGE_WIDTH,
				   XDFECCF_TRIGGERS_SIGNAL_EDGE_OFFSET, Val);
	TriggerCfg->LowPower.OneShot =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				   XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Val);

	/* Read CC_UPDATE triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	TriggerCfg->CCUpdate.Enable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_ENABLE_OFFSET, Val);
	TriggerCfg->CCUpdate.Source =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_SOURCE_WIDTH,
				   XDFECCF_TRIGGERS_SOURCE_OFFSET, Val);
	TriggerCfg->CCUpdate.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->CCUpdate.Edge =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_SIGNAL_EDGE_WIDTH,
				   XDFECCF_TRIGGERS_SIGNAL_EDGE_OFFSET, Val);
	TriggerCfg->CCUpdate.OneShot =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				   XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Set trigger configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    TriggerCfg is a triger configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_SetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Write public trigger configuration members and ensure private
	   members (_Enable & _OneShot) are set appropriately */

	/* Activate defined as OneShot (as per the programming model) */
	TriggerCfg->Activate.Enable = 0U;
	TriggerCfg->Activate.OneShot = 1U;
	/* Read/set/write ACTIVATE triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_ENABLE_OFFSET, Val,
				 TriggerCfg->Activate.Enable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				 XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Val,
				 TriggerCfg->Activate.OneShot);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_ACTIVATE_OFFSET, Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.Enable = 0U;
	TriggerCfg->LowPower.OneShot = 0U;
	/* Read LOW_POWER triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_ENABLE_OFFSET, Val,
				 TriggerCfg->LowPower.Enable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				 XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Val,
				 TriggerCfg->LowPower.OneShot);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Val);

	/* CCUpdate defined as OneShot */
	TriggerCfg->CCUpdate.Enable = 0U;
	TriggerCfg->CCUpdate.OneShot = 1U;
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_ENABLE_OFFSET, Val,
				 TriggerCfg->CCUpdate.Enable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_ONE_SHOT_WIDTH,
				 XDFECCF_TRIGGERS_ONE_SHOT_OFFSET, Val,
				 TriggerCfg->CCUpdate.OneShot);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_CC_UPDATE_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Get specified CCID carrier configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    CCID is a Channel ID.
* @param    CarrierCfg is a triger configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_GetCC(const XDfeCcf *InstancePtr, u32 CCID,
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
* Return a list indicating which coefficient sets are currently in use.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    IsActive is a pointer indicating an activation status.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_GetActiveSets(const XDfeCcf *InstancePtr, u32 *IsActive)
{
	u32 Index;
	u32 SeqLen;

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
		u32 CCID = XDfeCcf_ReadReg(InstancePtr,
					   XDFECCF_SEQUENCE_CURRENT +
						   (sizeof(u32) * Index));
		u32 Val = XDfeCcf_ReadReg(
			InstancePtr, XDFECCF_CARRIER_CONFIGURATION_CURRENT +
					     (sizeof(u32) * CCID));
		u32 ReCoefSet =
			XDfeCcf_RdBitField(XDFECCF_RE_COEFF_SET_WIDTH,
					   XDFECCF_RE_COEFF_SET_OFFSET, Val);
		IsActive[ReCoefSet] = 1U;
		u32 ImCoefSet =
			XDfeCcf_RdBitField(XDFECCF_IM_COEFF_SET_WIDTH,
					   XDFECCF_IM_COEFF_SET_OFFSET, Val);
		IsActive[ImCoefSet] = 1U;
	}
}

/****************************************************************************/
/**
*
* Write the coefficient set defined into the register map and commit them to
* the hard block's internal coefficient memory for the specified Set.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    Set is a coefficient set Id.
* @param    Coeffs is an array of filter coefficients.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_LoadCoefficients(const XDfeCcf *InstancePtr, u32 Set,
			      const XDfeCcf_Coefficients *Coeffs)
{
	u32 NumValues;
	u32 IsOdd;
	double CoeffSum = 0L;
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
	IsOdd = Coeffs->Num % 2;
	if (0U != Coeffs->Symmetric) {
		NumValues = (Coeffs->Num + 1U) / 2U;
	} else {
		NumValues = Coeffs->Num;
	}

	for (Index = 0; Index < NumValues; Index++) {
		CoeffSum += Coeffs->Value[Index];
	}

	if (0U != Coeffs->Symmetric) {
		CoeffSum = 2U * CoeffSum;
		if (0U != IsOdd) {
			CoeffSum = CoeffSum - Coeffs->Value[NumValues - 1U];
		}
	}

	ScaleFactor = CoeffSum / (256 * ((u32)1 << 15));
	Shift = (u32)floor(fabs(log2(ScaleFactor)));

	/* Check is load in progress */
	for (Index = 0; Index < XDFECCF_COEFF_LOAD_TIMEOUT; Index++) {
		LoadActive = XDFECCF_STATUS_WIDTH &
			     XDfeCcf_ReadReg(InstancePtr, XDFECCF_COEFF_LOAD);
		if (XDFECCF_STATUS_LOADING != LoadActive) {
			break;
		}
		usleep(10);
		if (Index == (XDFECCF_COEFF_LOAD_TIMEOUT - 1)) {
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
* @param    Version is driver version numbers.
*
* @return   None
*
* @note     None.
*
******************************************************************************/
void XDfeCcf_GetVersions(XDfeCcf_Version *Version)
{
	Version->Major = XDFECCF_DRIVER_VERSION_MAJOR;
	Version->Minor = XDFECCF_DRIVER_VERSION_MINOR;
}
/** @} */
