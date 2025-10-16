/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf.c
* Contains the APIs for DFE Channel Filter component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/29/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/08/21 align driver to current specification
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
* 1.1   dc     07/13/21 Update to common latency requirements
*       dc     07/21/21 Add and reorganise examples
*       dc     10/26/21 Make driver R5 compatible
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/19/21 Update doxygen documentation
*       dc     11/26/21 Model parameter NumCCPerAntenna workaround
*       dc     11/26/21 Set sequence length in GetEmptyCCCfg
*       dc     11/26/21 Add SetAntennaCfgInCCCfg API
*       dc     11/30/21 Convert AntennaCfg to structure
*       dc     12/02/21 Add UpdateAntennaCfg API
*       dc     12/17/21 Update after documentation review
* 1.3   dc     01/07/22 Zero-padding coefficients
*       dc     01/19/22 Assert CCUpdate trigger
*       dc     01/21/22 Symmetric filter Zero-padding
*       dc     01/27/22 Get calculated TDataDelay
*       dc     01/31/22 CCF IP MODEL_PARAM register change
*       dc     03/21/22 Add prefix to global variables
* 1.4   dc     04/08/22 Update documentation
* 1.5   dc     10/28/22 Switching Uplink/Downlink support
*       dc     11/11/22 Align AddCC to switchable UL/DL algorithm
*       dc     11/25/22 Update macro of SW version Minor number
* 1.6   dc     06/15/23 Function comment update
*       dc     06/20/23 Deprecate obsolete APIs
*       cog    07/04/23 Add support for SDT
*       dc     08/29/23 Remove immediate trigger
* 1.7   cog    02/02/24 Yocto SDT support
*       dc     03/01/24 Update version number in makefiles
* 1.9   dc     10/16/25 Fix zero padding and GetActivSets
* </pre>
* @addtogroup dfeccf Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/
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
#define XDFECCF_SEQUENCE_ENTRY_DEFAULT (0U) /**< Default sequence entry flag */
#define XDFECCF_SEQUENCE_ENTRY_NULL (-1) /**< Null sequence entry flag */
#define XDFECCF_NO_EMPTY_CCID_FLAG (0xFFFFU) /**< Not Empty CCID flag */
#define XDFECCF_COEFF_LOAD_TIMEOUT (100U) /**< Units of 10us */
#define XDFECCF_COEFF_UNIT_SIZE (4U) /**< Coefficient unit size */
/**
* @endcond
*/
#define XDFECCF_ACTIVE_SET_NUM (8U) /**< Maximum number of active sets */
#define XDFECCF_U32_NUM_BITS (32U) /**< Number of bits in register */
#define XDFECCF_TAP_NUMBER_MAX (256U) /**< Maximum tap number */
#define XDFECCF_DRIVER_VERSION_MINOR (9U) /**< Driver's minor version number */
#define XDFECCF_DRIVER_VERSION_MAJOR (1U) /**< Driver's major version number */

/************************** Function Prototypes *****************************/
static void XDfeCcf_GetCurrentCCCfgLocal(const XDfeCcf *InstancePtr,
					 XDfeCcf_CCCfg *CurrCCCfg);

/************************** Variable Definitions ****************************/
/**
* @cond nocomments
*/
#ifdef __BAREMETAL__
extern struct metal_device XDfeCcf_CustomDevice[XDFECCF_MAX_NUM_INSTANCES];
extern metal_phys_addr_t XDfeCcf_metal_phys[XDFECCF_MAX_NUM_INSTANCES];
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
* @param    InstancePtr Pointer to the Ccf instance.
* @param    AddrOffset Address offset relative to instance base address.
* @param    Data Value to be written.
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
* @param    InstancePtr Pointer to the XDfeCcf instance.
* @param    AddrOffset Address offset relative to instance base address.
*
* @return   Register value.
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
* @param    InstancePtr Pointer to the XDfeCcf instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
* @param    FieldData Bit field data.
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
* @param    InstancePtr Pointer to the XDfeCcf instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
*
* @return   Bit field data.
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
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 value of bit field that the function reads.
*
* @return   Bit field value.
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
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 value of bit field that the function reads.
* @param    Val U32 value to be written in the bit field.
*
* @return   Data with a written bit field.
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
* @param    Sequence CC sequence array.
*
* @return   Unused CCID.
*
****************************************************************************/
static s32 XDfeCcf_GetNotUsedCCID(XDfeCcf_CCSequence *Sequence)
{
	u32 Index;
	s32 NotUsedCCID;

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
* Count number of 1 in bitmap.
*
* @param    InstancePtr Pointer to the XDfeCcf instance.
* @param    CCSeqBitmap maps the sequence.
*
****************************************************************************/
static u32 XDfeCcf_CountOnesInBitmap(const XDfeCcf *InstancePtr,
				     u32 CCSeqBitmap)
{
	u32 Mask = 1U;
	u32 Index;
	u32 OnesCounter = 0;
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
* Adds the specified CCID, to the CC sequence. The sequence is defined with
* CCSeqBitmap where bit0 corresponds to CC[0], bit1 to CC[1], and so on. Also
* it saves the smallest not used CC Id.
*
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the XDfeCcf instance.
* @param    CCID CC ID.
* @param    CCSeqBitmap Maps the sequence.
* @param    CCIDSequence CC sequence array.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeCcf_AddCCIDAndTranslateSeq(XDfeCcf *InstancePtr, s32 CCID,
					  u32 CCSeqBitmap,
					  XDfeCcf_CCSequence *CCIDSequence)
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
	OnesCounter = XDfeCcf_CountOnesInBitmap(InstancePtr, CCSeqBitmap);

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
		if (0U != (CCSeqBitmap & Mask)) {
			CCIDSequence->CCID[Index] = CCID;
		}
		Mask <<= 1U;
	}

	/* Set not used CCID */
	CCIDSequence->NotUsedCCID = XDfeCcf_GetNotUsedCCID(CCIDSequence);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes the specified CCID from the CC sequence, replaces the CCID entries
* with null (-1) and saves the smallest not used CC Id.
*
* @param    CCID CC ID.
* @param    CCIDSequence CC sequence array.
*
****************************************************************************/
static void XDfeCcf_RemoveCCID(s32 CCID, XDfeCcf_CCSequence *CCIDSequence)
{
	u32 Index;

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] = XDFECCF_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Set not used CCID */
	CCIDSequence->NotUsedCCID = XDfeCcf_GetNotUsedCCID(CCIDSequence);
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Gets specified CCID carrier configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCID CC ID.
* @param    CarrierCfg Trigger configuration container.
*
*
****************************************************************************/
static void
XDfeCcf_GetInternalCarrierCfg(const XDfeCcf *InstancePtr, s32 CCID,
			      XDfeCcf_InternalCarrierCfg *CarrierCfg)
{
	u32 Val;

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
* Sets the next CC configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    NextCCCfg Next CC configuration container.
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

	/* Prepare NextCCID[] to be written to registers */
	for (Index = 0U; Index < XDFECCF_CC_NUM; Index++) {
		if ((NextCCCfg->Sequence.CCID[Index] ==
		     XDFECCF_SEQUENCE_ENTRY_NULL) ||
		    (Index >= InstancePtr->SequenceLength)) {
			NextCCID[Index] = NextCCCfg->Sequence.NotUsedCCID;
		} else {
			NextCCID[Index] = NextCCCfg->Sequence.CCID[Index];
		}
	}

	/* Sequence Length should remain the same, so copy the sequence length
	   from CURRENT to NEXT, does not take from NextCCCfg. The reason
	   is that NextCCCfg->Sequence.SeqLength can be 0 or 1 for the value 0
	   in the CURRENT seqLength register */
	if (InstancePtr->SequenceLength == 0) {
		SeqLength = 0U;
	} else {
		SeqLength = InstancePtr->SequenceLength - 1U;
	}
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
		AntennaCfg += (NextCCCfg->AntennaCfg.Enable[Index] << Index);
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
* @param    InstancePtr Pointer to the Channel Filter instance.
* @param    CCCfg CC configuration container.
* @param    ID Mapped index.
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
* @param    InstancePtr Pointer to the Channel Filter instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfeCcf_EnableCCUpdateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	/* Exit with error if CC_UPDATE status is high */
	if (XDFECCF_CC_UPDATE_TRIGGERED_HIGH ==
	    XDfeCcf_RdRegBitField(InstancePtr, XDFECCF_ISR,
				  XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFECCF_CC_UPDATE_TRIGGERED_OFFSET)) {
		metal_log(METAL_LOG_ERROR, "CCUpdate status high in %s\n",
			  __func__);
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
* @param    InstancePtr Pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_EnableLowPowerTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

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
* @param    InstancePtr Pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_EnableActivateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

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
* @param    InstancePtr Pointer to the Ch Filter instance.
*
****************************************************************************/
static void XDfeCcf_EnableDeactivateTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

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
* @param    InstancePtr Pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_DisableLowPowerTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Enables switch triggers.
*
* @param    InstancePtr Pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_EnableSwitchTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_SWITCH_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_SWITCH_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Disables switch triggers.
*
* @param    InstancePtr Pointer to the Channel Filter instance.
*
****************************************************************************/
static void XDfeCcf_DisableSwitchTrigger(const XDfeCcf *InstancePtr)
{
	u32 Data;

	Data = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_LOW_POWER_OFFSET, Data);
}

/**
* @endcond
*/

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* Initialises one instance of a channel filter driver.
* Traverses "/sys/bus/platform/device" directory (in Linux), to find registered
* CCF device with the name DeviceNodeName. The first available slot in
* the instances array XDfeCcf_ChFilter[] will be taken as a DeviceNodeName
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
		for (Index = 0; XDFECCF_INSTANCE_EXISTS(Index); Index++) {
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
	for (Index = 0; XDFECCF_INSTANCE_EXISTS(Index); Index++) {
		if (0U == strncmp(XDfeCcf_ChFilter[Index].NodeName,
				  DeviceNodeName, strlen(DeviceNodeName))) {
			XDfeCcf_ChFilter[Index].StateId = XDFECCF_STATE_READY;
			return &XDfeCcf_ChFilter[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; XDFECCF_INSTANCE_EXISTS(Index); Index++) {
		if (XDfeCcf_ChFilter[Index].NodeName[0] == '\0') {
			strncpy(XDfeCcf_ChFilter[Index].NodeName,
				DeviceNodeName, XDFECCF_NODE_NAME_MAX_LENGTH);
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
	Addr = strtoul(AddrStr, NULL, 16);
	for (Index = 0; XDFECCF_INSTANCE_EXISTS(Index); Index++) {
		if (Addr == XDfeCcf_metal_phys[Index]) {
			InstancePtr->Device = &XDfeCcf_CustomDevice[Index];
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
* Closes the instances of a channel filter driver and moves the state
* machine to a Not Ready state.
*
* @param    InstancePtr Pointer to the XDfeCcf instance.
*
******************************************************************************/
void XDfeCcf_InstanceClose(XDfeCcf *InstancePtr)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; XDFECCF_INSTANCE_EXISTS(Index); Index++) {
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
* Resets channel filter and puts block into a reset state.
*
* @param    InstancePtr Pointer to the Ccf instance.
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
* Removes S/W reset and moves the state machine to a Configured state.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Cfg Configuration data container.
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
	InstancePtr->Config.AntennaInterleave = XDfeCcf_RdBitField(
		XDFECCF_MODEL_PARAM_ANTENNA_INTERLEAVE_WIDTH,
		XDFECCF_MODEL_PARAM_ANTENNA_INTERLEAVE_OFFSET, ModelParam);
	InstancePtr->Config.Switchable =
		XDfeCcf_RdBitField(XDFECCF_MODEL_PARAM_SWITCHABLE_WIDTH,
				   XDFECCF_MODEL_PARAM_SWITCHABLE_OFFSET,
				   ModelParam);

	/* Copy configs model parameters from InstancePtr */
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.NumCCPerAntenna = InstancePtr->Config.NumCCPerAntenna;
	Cfg->ModelParams.AntennaInterleave =
		InstancePtr->Config.AntennaInterleave;
	Cfg->ModelParams.Switchable = InstancePtr->Config.Switchable;

	/* Release RESET */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_RESET_OFFSET, XDFECCF_RESET_OFF);
	InstancePtr->StateId = XDFECCF_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE Ccf driver one time initialisation, also moves the state machine to
* an Initialised state.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Init Initialisation data container.
*
****************************************************************************/
void XDfeCcf_Initialize(XDfeCcf *InstancePtr, XDfeCcf_Init *Init)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_CONFIGURED);
	Xil_AssertVoid(Init != NULL);
	Xil_AssertVoid(Init->TuserSelect <= XDFECCF_TUSER_SEL_UPLINK);

	/* Write "one-time" configuration */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_GAIN_STG_EN_OFFSET,
			 Init->GainStage);

	if (InstancePtr->Config.Switchable == XDFECCF_SWITCHABLE_YES) {
		/* Write "one-time" tuser select. If the core is configured for
		   non-switchable mode override tuser select so that the default tuser
		   channel is used */
		if (InstancePtr->Config.Switchable == XDFECCF_SWITCHABLE_NO) {
			Init->TuserSelect = 0U;
		}
		XDfeCcf_WriteReg(InstancePtr, XDFECCF_TUSER_SEL_OFFSET,
				 Init->TuserSelect);
	}

	/* Not used CC index for DL (NotUsedCCID) and UL (NotUsedCCID_UL) in
	   switchable mode otherwise just NotUsedCCID will be relevant */
	InstancePtr->NotUsedCCID = 0;
	InstancePtr->NotUsedCCID_UL = 0;
	/* Write "one-time" Sequence length. InstancePtr->SequenceLength holds
	   the exact sequence length value as register sequence length value 0
	   can be understod as length 0 or 1 */
	InstancePtr->SequenceLength = Init->Sequence.Length;
	InstancePtr->StateId = XDFECCF_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activates channel filter and moves the state machine to an Activated state.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    EnableLowPower Flag indicating low power.
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

	if (InstancePtr->Config.Switchable == XDFECCF_SWITCHABLE_YES) {
		XDfeCcf_EnableSwitchTrigger(InstancePtr);
	}

	/* Channel filter is operational now, change a state */
	InstancePtr->StateId = XDFECCF_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* Deactivates channel filter and moves the state machine to Initialised state.
*
* @param    InstancePtr Pointer to the Ccf instance.
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

	/* Disable Switch trigger (may not be enabled) */
	if (InstancePtr->Config.Switchable == XDFECCF_SWITCHABLE_YES) {
		XDfeCcf_DisableSwitchTrigger(InstancePtr);
	}

	InstancePtr->StateId = XDFECCF_STATE_INITIALISED;
}

/****************************************************************************/
/**
*
* Gets a state machine state id.
*
* @param    InstancePtr Pointer to the Ccf instance.
*
* @return   State machine StateID
*
****************************************************************************/
XDfeCcf_StateId XDfeCcf_GetStateID(XDfeCcf *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return InstancePtr->StateId;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Returns the current CC configuration in non-switchable mode. Not used slot
* ID in a sequence (Sequence.CCID[Index]) are represented as (-1), not
* the value in registers.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CurrCCCfg CC configuration container.
*
* @note     For a sequence conversion see XDfeCcf_AddCCtoCCCfg() comment.
*
****************************************************************************/
void XDfeCcf_GetCurrentCCCfg(const XDfeCcf *InstancePtr,
			     XDfeCcf_CCCfg *CurrCCCfg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CurrCCCfg != NULL);

	CurrCCCfg->Sequence.NotUsedCCID = InstancePtr->NotUsedCCID;
	XDfeCcf_GetCurrentCCCfgLocal(InstancePtr, CurrCCCfg);
}

/**
* @cond nocomments
*/
/****************************************************************************/
/**
*
* Returns the current CC configuration. Not used slot ID in a sequence
* (Sequence.CCID[Index]) are represented as (-1), not the value in registers.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CurrCCCfg CC configuration container.
*
* @note     For a sequence conversion see XDfeCcf_AddCCtoCCCfg() comment.
*
****************************************************************************/
static void XDfeCcf_GetCurrentCCCfgLocal(const XDfeCcf *InstancePtr,
					 XDfeCcf_CCCfg *CurrCCCfg)
{
	u32 AntennaCfg = 0U;
	u32 Index;

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFECCF_CC_NUM; Index++) {
		CurrCCCfg->Sequence.CCID[Index] = XDfeCcf_ReadReg(
			InstancePtr,
			XDFECCF_SEQUENCE_CURRENT + (sizeof(u32) * Index));
		XDfeCcf_GetInternalCarrierCfg(InstancePtr, Index,
					      &CurrCCCfg->CarrierCfg[Index]);
	}

	/* Read sequence length */
	CurrCCCfg->Sequence.Length = InstancePtr->SequenceLength;

	/* Convert not used CC to -1 */
	for (Index = 0U; Index < XDFECCF_CC_NUM; Index++) {
		if ((CurrCCCfg->Sequence.CCID[Index] ==
		     CurrCCCfg->Sequence.NotUsedCCID) ||
		    (Index >= InstancePtr->SequenceLength)) {
			CurrCCCfg->Sequence.CCID[Index] =
				XDFECCF_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Read Antenna configuration */
	AntennaCfg = XDfeCcf_ReadReg(InstancePtr,
				     XDFECCF_ANTENNA_CONFIGURATION_CURRENT);
	for (Index = 0; Index < XDFECCF_ANT_NUM_MAX; Index++) {
		CurrCCCfg->AntennaCfg.Enable[Index] =
			(AntennaCfg >> Index) & XDFECCF_ANTENNA_ENABLE;
	}

	/* Clear Flush on all CCs */
	for (Index = 0U; Index < XDFECCF_CC_NUM; Index++) {
		CurrCCCfg->CarrierCfg[Index].Flush = 0U;
	}
}
/**
* @endcond
*/

/****************************************************************************/
/**
*
* Returns the current CC configuration for Downlink and Uplink in switchable
* mode. Not used slot ID in a sequence (Sequence.CCID[Index]) are
* represented as (-1), not the value in registers.
*
* @param    InstancePtr Pointer to the Mixer instance.
* @param    CCCfgDownlink Downlink CC configuration container.
* @param    CCCfgUplink Uplink CC configuration container.
*
****************************************************************************/
void XDfeCcf_GetCurrentCCCfgSwitchable(const XDfeCcf *InstancePtr,
				       XDfeCcf_CCCfg *CCCfgDownlink,
				       XDfeCcf_CCCfg *CCCfgUplink)
{
	u32 RegBank;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfgDownlink != NULL);
	Xil_AssertVoid(CCCfgUplink != NULL);
	Xil_AssertVoid(InstancePtr->Config.Switchable ==
		       XDFECCF_SWITCHABLE_YES);

	RegBank = XDfeCcf_RdRegBitField(InstancePtr, XDFECCF_REG_BANK_OFFSET,
					XDFECCF_REG_BANK_DL_UL_WIDTH,
					XDFECCF_REG_BANK_DL_UL_OFFSET);

	/* Set Downlink register bank */
	XDfeCcf_SetRegBank(InstancePtr, XDFECCF_SWITCH_DOWNLINK);
	CCCfgDownlink->Sequence.NotUsedCCID = InstancePtr->NotUsedCCID;
	XDfeCcf_GetCurrentCCCfgLocal(InstancePtr, CCCfgDownlink);

	/* Set Uplink register bank */
	XDfeCcf_SetRegBank(InstancePtr, XDFECCF_SWITCH_UPLINK);
	CCCfgUplink->Sequence.NotUsedCCID = InstancePtr->NotUsedCCID_UL;
	XDfeCcf_GetCurrentCCCfgLocal(InstancePtr, CCCfgUplink);

	/* Set to the current register bank */
	XDfeCcf_SetRegBank(InstancePtr, RegBank);
}

/****************************************************************************/
/**
*
* Returns configuration structure CCCfg with CCCfg->Sequence.Length value set
* in XDfeCcf_Configure(), array CCCfg->Sequence.CCID[] members are set to not
* used value (-1) and the other CCCfg members are set to 0.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg CC configuration container.
*
****************************************************************************/
void XDfeCcf_GetEmptyCCCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	memset(CCCfg, 0, sizeof(XDfeCcf_CCCfg));

	/* Convert CC to -1 meaning not used */
	for (Index = 0U; Index < XDFECCF_CC_NUM; Index++) {
		CCCfg->Sequence.CCID[Index] = XDFECCF_SEQUENCE_ENTRY_NULL;
	}
	/* Read sequence length */
	CCCfg->Sequence.Length = InstancePtr->SequenceLength;
}

/****************************************************************************/
/**
*
* Returns the current CCID carrier configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID for which configuration parameters are returned,
*           range [0-15].
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
*
****************************************************************************/
void XDfeCcf_GetCarrierCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg,
			   s32 CCID, u32 *CCSeqBitmap,
			   XDfeCcf_CarrierCfg *CarrierCfg)
{
	u32 Index;
	u32 Mask = 0x1;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertVoid(CCSeqBitmap != NULL);
	Xil_AssertVoid(CarrierCfg != NULL);

	CarrierCfg->Gain = CCCfg->CarrierCfg[CCID].Gain;
	CarrierCfg->ImagCoeffSet = CCCfg->CarrierCfg[CCID].ImagCoeffSet;
	CarrierCfg->RealCoeffSet = CCCfg->CarrierCfg[CCID].RealCoeffSet;

	*CCSeqBitmap = 0;
	for (Index = 0; Index < CCCfg->Sequence.Length; Index++) {
		if (CCCfg->Sequence.CCID[Index] == CCID) {
			*CCSeqBitmap |= Mask;
		}
		Mask <<= 1;
	}
}

/****************************************************************************/
/**
*
* Set antenna configuration in CC configuration container.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg CC configuration container.
* @param    AntennaCfg Array of all antenna configurations.
*
****************************************************************************/
void XDfeCcf_SetAntennaCfgInCCCfg(const XDfeCcf *InstancePtr,
				  XDfeCcf_CCCfg *CCCfg,
				  XDfeCcf_AntennaCfg *AntennaCfg)
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
* The returned CCCfg.Sequence is translated as there is no explicit indication that
* SEQUENCE[i] is not used - 0 can define the slot as either used or not used.
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID.
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_AddCCtoCCCfg(XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg, s32 CCID,
			 u32 CCSeqBitmap, const XDfeCcf_CarrierCfg *CarrierCfg)
{
	u32 AddSuccess;
	u32 IDAvailable;
	u32 NextMappedID;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Get next mapped ID. This should be done before trying to add a new
	   CCID */
	IDAvailable = XDfeCcf_NextMappedId(InstancePtr, CCCfg, &NextMappedID);
	if (IDAvailable == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "ID is not available in %s\n",
			  __func__);
		return XST_FAILURE;
	}
	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfeCcf_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg->Sequence);
	if (AddSuccess == (u32)XST_SUCCESS) {
		/* Update carrier configuration, mark flush as we need to clear
		   data registers */
		CCCfg->CarrierCfg[CCID].Gain = CarrierCfg->Gain;
		CCCfg->CarrierCfg[CCID].ImagCoeffSet = CarrierCfg->ImagCoeffSet;
		CCCfg->CarrierCfg[CCID].RealCoeffSet = CarrierCfg->RealCoeffSet;
		CCCfg->CarrierCfg[CCID].Enable = 1U;
		CCCfg->CarrierCfg[CCID].Flush = 1U;
		CCCfg->CarrierCfg[CCID].MappedId = NextMappedID;
	} else {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes specified CCID from a local CC configuration structure.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID to be removed, range [0-15].
*
* @note     For a sequence conversion see XDfeCcf_AddCCtoCCCfg comment.
*
****************************************************************************/
void XDfeCcf_RemoveCCfromCCCfg(XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg,
			       s32 CCID)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertVoid(CCCfg != NULL);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeCcf_RemoveCCID(CCID, &CCCfg->Sequence);
	CCCfg->CarrierCfg[CCID].Enable = 0U;
	CCCfg->CarrierCfg[CCID].MappedId = 0U;
}

/****************************************************************************/
/**
*
* Updates specified CCID, with specified configuration to a local CC
* configuration structure.
* If there is insufficient capacity for the new CC the function will return
* an error.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID Channel ID to be updated, range [0-15].
* @param    CarrierCfg CC configuration container.
*
****************************************************************************/
void XDfeCcf_UpdateCCinCCCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg,
			     s32 CCID, const XDfeCcf_CarrierCfg *CarrierCfg)
{
	u32 Flush;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertVoid(CarrierCfg != NULL);

	/* Update carrier configuration. Set flush if coefficient set has
	   changed */
	if ((CarrierCfg->RealCoeffSet !=
	     CCCfg->CarrierCfg[CCID].RealCoeffSet) ||
	    (CarrierCfg->ImagCoeffSet !=
	     CCCfg->CarrierCfg[CCID].ImagCoeffSet)) {
		Flush = 1U;
	} else {
		Flush = 0U;
	}

	CCCfg->CarrierCfg[CCID].Flush = Flush;
	CCCfg->CarrierCfg[CCID].Gain = CarrierCfg->Gain;
	CCCfg->CarrierCfg[CCID].ImagCoeffSet = CarrierCfg->ImagCoeffSet;
	CCCfg->CarrierCfg[CCID].RealCoeffSet = CarrierCfg->RealCoeffSet;
}

/****************************************************************************/
/**
*
* Writes local CC configuration to the shadow (NEXT) registers and triggers
* copying from shadow to operational (CURRENT) registers.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_SetNextCCCfgAndTrigger(XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfg != NULL);

	/* If add is successful update next configuration and trigger update */
	XDfeCcf_SetNextCCCfg(InstancePtr, CCCfg);

	/* Trigger the update */
	if (XST_SUCCESS == XDfeCcf_EnableCCUpdateTrigger(InstancePtr)) {
		InstancePtr->NotUsedCCID = CCCfg->Sequence.NotUsedCCID;
		return XST_SUCCESS;
	}
	metal_log(METAL_LOG_ERROR,
		  "CC Update Trigger failed in %s. Restart the system\n",
		  __func__);
	return XST_FAILURE;
}

/****************************************************************************/
/**
*
* Writes local CC configuration to the shadow (NEXT) registers and triggers
* copying from shadow to operational (CURRENT) registers for both Downlink
* and Upling in switchable mode.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCCfgDownlink Downlink CC configuration container.
* @param    CCCfgUplink Uplink CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_SetNextCCCfgAndTriggerSwitchable(XDfeCcf *InstancePtr,
					     XDfeCcf_CCCfg *CCCfgDownlink,
					     XDfeCcf_CCCfg *CCCfgUplink)
{
	u32 RegBank;
	u32 Return;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCCfgDownlink != NULL);
	Xil_AssertNonvoid(CCCfgUplink != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.Switchable ==
			  XDFECCF_SWITCHABLE_YES);

	RegBank = XDfeCcf_RdRegBitField(InstancePtr, XDFECCF_REG_BANK_OFFSET,
					XDFECCF_REG_BANK_DL_UL_WIDTH,
					XDFECCF_REG_BANK_DL_UL_OFFSET);

	/* If adding is successful, set next configuration and trigger update */
	/* Set the downlink CCCfg */
	XDfeCcf_SetRegBank(InstancePtr, XDFECCF_REG_BANK_DOWNLINK);
	XDfeCcf_SetNextCCCfg(InstancePtr, CCCfgDownlink);

	/* Set the uplink CCCfg */
	XDfeCcf_SetRegBank(InstancePtr, XDFECCF_REG_BANK_UPLINK);
	XDfeCcf_SetNextCCCfg(InstancePtr, CCCfgUplink);

	/* Trigger update */
	if (XST_SUCCESS == XDfeCcf_EnableCCUpdateTrigger(InstancePtr)) {
		InstancePtr->NotUsedCCID = CCCfgDownlink->Sequence.NotUsedCCID;
		InstancePtr->NotUsedCCID_UL = CCCfgUplink->Sequence.NotUsedCCID;
		Return = XST_SUCCESS;
	} else {
		metal_log(
			METAL_LOG_ERROR,
			"CC Update Trigger failed in %s. Restart the system\n",
			__func__);
		Return = XST_FAILURE;
	}

	/* Set to the current register bank */
	XDfeCcf_SetRegBank(InstancePtr, RegBank);

	return Return;
}

/****************************************************************************/
/**
*
* Adds specified CCID, with specified configuration.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCID CC ID.
* @param    CCSeqBitmap - up to 16 defined slots into which a CC can be
*           allocated. The number of slots can be from 1 to 16 depending on
*           system initialization. The number of slots is defined by the
*           "sequence length" parameter which is provided during initialization.
*           The Bit offset within the CCSeqBitmap indicates the equivalent
*           Slot number to allocate. e.g. 0x0003  means the caller wants the
*           passed component carrier (CC) to be allocated to slots 0 and 1.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeCcf_ClearEventStatus() before
*           running this API.
*
* @attention:  This API is deprecated in the release 2023.2. Source code will
*              be removed from in the release 2024.1 release. The functionality
*              of this API can be reproduced with the following API sequence:
*                  XDfeCcf_GetCurrentCCCfg(InstancePtr, CCCfg);
*                  XDfeCcf_AddCCtoCCCfg(InstancePtr, CCCfg, CCID, CCSeqBitmap,
*                      CarrierCfg);
*                  XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, CCCfg);
*
****************************************************************************/
u32 XDfeCcf_AddCC(XDfeCcf *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		  const XDfeCcf_CarrierCfg *CarrierCfg)
{
	XDfeCcf_CCCfg CCCfg;
	u32 AddSuccess;
	u32 IDAvailable;
	u32 NextMappedID = 0;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);
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
	AddSuccess = XDfeCcf_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg.Sequence);
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
	return XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, &CCCfg);
}

/****************************************************************************/
/**
*
* Removes specified CCID.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCID CC ID.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeCcf_ClearEventStatus() before
*           running this API.
*
* @attention:  This API is deprecated in the release 2023.2. Source code will
*              be removed from in the release 2024.1 release. The functionality
*              of this API can be reproduced with the following API sequence:
*                  XDfeCcf_GetCurrentCCCfg(InstancePtr, CCCfg);
*                  XDfeCcf_RemoveCCfromCCCfg(InstancePtr, CCCfg, CCID);
*                  XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, CCCfg);
*
****************************************************************************/
u32 XDfeCcf_RemoveCC(XDfeCcf *InstancePtr, s32 CCID)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfeCcf_RemoveCCID(CCID, &CCCfg.Sequence);
	CCCfg.CarrierCfg[CCID].Enable = 0U;
	CCCfg.CarrierCfg[CCID].MappedId = 0U;

	/* Update next configuration and trigger update */
	return XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, &CCCfg);
}

/****************************************************************************/
/**
*
* Updates specified CCID carrier configuration; change gain or filter
* coefficients set.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCID CC ID.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeCcf_ClearEventStatus() before
*           running this API.
*
* @attention:  This API is deprecated in the release 2023.2. Source code will
*              be removed from in the release 2024.1 release. The functionality
*              of this API can be reproduced with the following API sequence:
*                  XDfeCcf_GetCurrentCCCfg(InstancePtr, CCCfg);
*                  XDfeCcf_UpdateCCinCCCfg(InstancePtr, CCCfg, CCID,
*                      CarrierCfg);
*                  XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, CCCfg);
*
****************************************************************************/
u32 XDfeCcf_UpdateCC(XDfeCcf *InstancePtr, s32 CCID,
		     const XDfeCcf_CarrierCfg *CarrierCfg)
{
	XDfeCcf_CCCfg CCCfg;
	u32 Flush = 0U;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Update carrier configuration. Set flush if coefficient set has
	   changed */
	if ((CarrierCfg->RealCoeffSet != CCCfg.CarrierCfg[CCID].RealCoeffSet) ||
	    (CarrierCfg->ImagCoeffSet != CCCfg.CarrierCfg[CCID].ImagCoeffSet)) {
		Flush = 1U;
	} else {
		Flush = 0U;
	}

	CCCfg.CarrierCfg[CCID].Flush = Flush;
	CCCfg.CarrierCfg[CCID].Gain = CarrierCfg->Gain;
	CCCfg.CarrierCfg[CCID].ImagCoeffSet = CarrierCfg->ImagCoeffSet;
	CCCfg.CarrierCfg[CCID].RealCoeffSet = CarrierCfg->RealCoeffSet;

	/* Update next configuration and trigger update */
	return XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, &CCCfg);
}

/****************************************************************************/
/**
*
* Updates specified antenna TDM slot enablement.
*
* Initiates CC update (enable CCUpdate trigger one-shot).
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Ant Antenna ID.
* @param    Enabled Flag indicating enable status of the antenna.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeCcf_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeCcf_UpdateAntenna(XDfeCcf *InstancePtr, u32 Ant, bool Enabled)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertNonvoid(Ant <= XDFECCF_ANT_NUM_MAX);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Update antenna enablement */
	if (Enabled == true) {
		CCCfg.AntennaCfg.Enable[Ant] = XDFECCF_ANTENNA_ENABLE;
	} else {
		CCCfg.AntennaCfg.Enable[Ant] = XDFECCF_ANTENNA_DISABLE;
	}

	/* Update next configuration and trigger update */
	return XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, &CCCfg);
}

/****************************************************************************/
/**
*
* Updates antenna configuration to all antennas.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    AntennaCfg Array of all antenna configurations.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeCcf_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeCcf_UpdateAntennaCfg(XDfeCcf *InstancePtr,
			     XDfeCcf_AntennaCfg *AntennaCfg)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AntennaCfg != NULL);

	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);
	CCCfg.AntennaCfg = *AntennaCfg;
	/* Update next configuration and trigger update */
	return XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, &CCCfg);
}

/****************************************************************************/
/**
*
* Updates antenna configuration of all antennas. Applies gain to downlink only
* in switchable mode.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    AntennaCfgDownlink Array of all downlink antenna configurations.
* @param    AntennaCfgUplink Array of all uplink antenna configurations.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfeCcf_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfeCcf_UpdateAntennaCfgSwitchable(XDfeCcf *InstancePtr,
				       XDfeCcf_AntennaCfg *AntennaCfgDownlink,
				       XDfeCcf_AntennaCfg *AntennaCfgUplink)
{
	XDfeCcf_CCCfg CCCfgDownlink;
	XDfeCcf_CCCfg CCCfgUplink;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AntennaCfgDownlink != NULL);
	Xil_AssertNonvoid(AntennaCfgUplink != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.Switchable ==
			  XDFECCF_SWITCHABLE_YES);

	/* Get the Current downlink and uplink CCCfg */
	XDfeCcf_GetCurrentCCCfgSwitchable(InstancePtr, &CCCfgDownlink,
					  &CCCfgUplink);

	/* Update the downlink antenna in CCCfg */
	CCCfgDownlink.AntennaCfg = *AntennaCfgDownlink;

	/* Update the uplink antenna in CCCfg */
	CCCfgUplink.AntennaCfg = *AntennaCfgUplink;

	/* Set CCCfg to Next registers and trigger CC_UPDAT */
	return XDfeCcf_SetNextCCCfgAndTriggerSwitchable(
		InstancePtr, &CCCfgDownlink, &CCCfgUplink);
}

/****************************************************************************/
/**
*
* Returns current trigger configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    TriggerCfg Trigger configuration container.
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

	/* Read SWITCH triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_SWITCH_OFFSET);
	TriggerCfg->Switch.TriggerEnable =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->Switch.Mode = XDfeCcf_RdBitField(
		XDFECCF_TRIGGERS_MODE_WIDTH, XDFECCF_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Switch.TUSERBit =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Switch.TuserEdgeLevel =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->Switch.StateOutput =
		XDfeCcf_RdBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets trigger configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfeCcf_SetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);
	Xil_AssertVoid(TriggerCfg->CCUpdate.Mode !=
		       XDFECCF_TRIGGERS_MODE_TUSER_CONTINUOUS);

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

	/* Switch defined as Continuous */
	TriggerCfg->Switch.TriggerEnable =
		XDFECCF_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->Switch.Mode = XDFECCF_TRIGGERS_MODE_TUSER_CONTINUOUS;
	/* Read SWITCH triggers */
	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_TRIGGERS_SWITCH_OFFSET);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFECCF_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->Switch.TriggerEnable);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_MODE_WIDTH,
				 XDFECCF_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->Switch.Mode);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->Switch.TuserEdgeLevel);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFECCF_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->Switch.TUSERBit);
	Val = XDfeCcf_WrBitField(XDFECCF_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFECCF_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->Switch.StateOutput);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_TRIGGERS_SWITCH_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Gets specified CCID carrier configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    CCID CC ID.
* @param    CarrierCfg Trigger configuration container.
*
****************************************************************************/
void XDfeCcf_GetCC(const XDfeCcf *InstancePtr, s32 CCID,
		   XDfeCcf_CarrierCfg *CarrierCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertVoid(CarrierCfg != NULL);

	/* Read specified CCID carrier configuration */
	Val = XDfeCcf_ReadReg(InstancePtr,
			      XDFECCF_CARRIER_CONFIGURATION_CURRENT +
				      (sizeof(u32) * CCID));
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
* @param    InstancePtr Pointer to the Ccf instance.
* @param    IsActive Pointer indicating an activation status.
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
	SeqLen = InstancePtr->SequenceLength;
	for (Index = 0; Index < SeqLen; Index++) {
		Offset = XDFECCF_CARRIER_CONFIGURATION_CURRENT +
			 (sizeof(u32) * Index);
		Enable = XDfeCcf_RdRegBitField(InstancePtr, Offset,
					       XDFECCF_ENABLE_WIDTH,
					       XDFECCF_ENABLE_OFFSET);
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
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Set Coefficient set Id.
* @param    Shift Coefficient shift value.
* @param    Coeffs Array of filter coefficients.
*
****************************************************************************/
void XDfeCcf_LoadCoefficients(XDfeCcf *InstancePtr, u32 Set, u32 Shift,
			      const XDfeCcf_Coefficients *Coeffs)
{
	u32 NumValues;
	u32 NumUnits;
	u32 IsOdd;
	u32 LoadActive;
	u32 Val;
	u32 Index;
	u32 NumPadding = 0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Coeffs != NULL);
	Xil_AssertVoid(Coeffs->Num != 0U); /* Protect from division with 0 */

	IsOdd = Coeffs->Num % 2U;
	if (0U != Coeffs->Symmetric) {
		NumValues = (Coeffs->Num + 1U) / 2U;
	} else {
		NumValues = Coeffs->Num;
	}
	Xil_AssertVoid(NumValues <= XDFECCF_NUM_COEFF);

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

	/* Number of units */
	NumUnits = (NumValues + (XDFECCF_COEFF_UNIT_SIZE - 1)) /
		   XDFECCF_COEFF_UNIT_SIZE;

	/* When no load is active write filter coefficients and initiate load */
	Val = XDfeCcf_WrBitField(XDFECCF_NUMBER_UNITS_WIDTH,
				 XDFECCF_NUMBER_UNITS_OFFSET, 0U, NumUnits);
	Val = XDfeCcf_WrBitField(XDFECCF_SHIFT_VALUE_WIDTH,
				 XDFECCF_SHIFT_VALUE_OFFSET, Val, Shift);
	Val = XDfeCcf_WrBitField(XDFECCF_IS_SYMMETRIC_WIDTH,
				 XDFECCF_IS_SYMMETRIC_OFFSET, Val,
				 Coeffs->Symmetric);
	Val = XDfeCcf_WrBitField(XDFECCF_USE_ODD_TAPS_WIDTH,
				 XDFECCF_USE_ODD_TAPS_OFFSET, Val, IsOdd);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_COEFF_CFG, Val);

	if ((NumValues % XDFECCF_COEFF_UNIT_SIZE) != 0) {
		NumPadding = XDFECCF_COEFF_UNIT_SIZE -
			     (NumValues % XDFECCF_COEFF_UNIT_SIZE);
	}
	if (0U == Coeffs->Symmetric) {
		for (Index = 0; Index < NumValues; Index++) {
			XDfeCcf_WriteReg(InstancePtr,
					 XDFECCF_COEFF_VALUE +
						 (sizeof(u32) * Index),
					 Coeffs->Value[Index]);
		}
		/* Non-symmetric filter: Zero-padding at the end of array */
		for (Index = NumValues; Index < NumValues + NumPadding;
		     Index++) {
			XDfeCcf_WriteReg(
				InstancePtr,
				XDFECCF_COEFF_VALUE + (sizeof(u32) * Index), 0);
		}
	} else {
		/* Symmetric filter: Zero-padding at the beginning of array */
		for (Index = 0; Index < NumPadding; Index++) {
			XDfeCcf_WriteReg(
				InstancePtr,
				XDFECCF_COEFF_VALUE + (sizeof(u32) * Index), 0);
		}
		for (Index = NumPadding; Index < NumValues + NumPadding; Index++) {
			XDfeCcf_WriteReg(
				InstancePtr,
				XDFECCF_COEFF_VALUE +
					(sizeof(u32) * (Index)),
				Coeffs->Value[Index]);
		}
	}

	/* Set the coefficient set value */
	XDfeCcf_WrRegBitField(InstancePtr, XDFECCF_COEFF_LOAD,
			      XDFECCF_SET_NUM_WIDTH, XDFECCF_SET_NUM_OFFSET,
			      Set);
	/* Load coefficients */
	XDfeCcf_WrRegBitField(InstancePtr, XDFECCF_COEFF_LOAD,
			      XDFECCF_STATUS_WIDTH, XDFECCF_STATUS_OFFSET, 1U);
}

/****************************************************************************/
/**
*
* Sets the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Delay Requested delay variable.
*
****************************************************************************/
void XDfeCcf_SetTUserDelay(const XDfeCcf *InstancePtr, u32 Delay)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_INITIALISED);
	Xil_AssertVoid(Delay < (1U << XDFECCF_DELAY_VALUE_WIDTH));

	XDfeCcf_WriteReg(InstancePtr, XDFECCF_DELAY_OFFSET, Delay);
}

/****************************************************************************/
/**
*
* Reads the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the Ccf instance.
*
* @return   Delay value
*
****************************************************************************/
u32 XDfeCcf_GetTUserDelay(const XDfeCcf *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfeCcf_RdRegBitField(InstancePtr, XDFECCF_DELAY_OFFSET,
				     XDFECCF_DELAY_VALUE_WIDTH,
				     XDFECCF_DELAY_VALUE_OFFSET);
}

/****************************************************************************/
/**
*
* Gets calculated TDataDelay value for CCID from current CC configuration.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Tap Tap value.
* @param    CCID CC ID.
* @param    Symmetric Select symmetric (1) or non-symmetric (0) filter.
* @param    Num Number of coefficients values.
* @param    TDataDelay Data latency value.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_GetTDataDelay(XDfeCcf *InstancePtr, u32 Tap, s32 CCID,
			  u32 Symmetric, u32 Num, u32 *TDataDelay)
{
	XDfeCcf_CCCfg CCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tap < XDFECCF_TAP_NUMBER_MAX);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertNonvoid(TDataDelay != NULL);

	/* Read current CC configuration */
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	/* Calculate TDataDelay */
	return XDfeCcf_GetTDataDelayFromCCCfg(InstancePtr, Tap, CCID, &CCCfg,
					      Symmetric, Num, TDataDelay);
}

/****************************************************************************/
/**
*
* Gets calculated TDataDelay value for CCID.
*
* @param    InstancePtr Pointer to the Ccf instance.
* @param    Tap Tap value.
* @param    CCID CC ID.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    Symmetric Select symmetric (1) or non-symmetric (0) filter.
* @param    Num Number of coefficients values.
* @param    TDataDelay Data latency value.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfeCcf_GetTDataDelayFromCCCfg(XDfeCcf *InstancePtr, u32 Tap, s32 CCID,
				   XDfeCcf_CCCfg *CCCfg, u32 Symmetric, u32 Num,
				   u32 *TDataDelay)
{
	u32 DataLatency;
	u32 CCSeqBitmap;
	XDfeCcf_CarrierCfg CarrierCfg;
	u32 OnesCounter;
	u32 NumPadding = 0U;
	u32 NumValues;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tap < XDFECCF_TAP_NUMBER_MAX);
	Xil_AssertNonvoid(CCID < XDFECCF_CC_NUM);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(TDataDelay != NULL);

	if (0U != Symmetric) {
		NumValues = (Num + 1U) / 2U;
	} else {
		NumValues = Num;
	}
	Xil_AssertNonvoid(NumValues <= XDFECCF_NUM_COEFF);
	if ((NumValues % XDFECCF_COEFF_UNIT_SIZE) != 0) {
		NumPadding = XDFECCF_COEFF_UNIT_SIZE -
			     (NumValues % XDFECCF_COEFF_UNIT_SIZE);
	}
	DataLatency =
		XDfeCcf_RdRegBitField(InstancePtr, XDFECCF_DATA_LATENCY_OFFSET,
				      XDFECCF_DATA_LATENCY_VALUE_WIDTH,
				      XDFECCF_DATA_LATENCY_VALUE_OFFSET);
	XDfeCcf_GetCarrierCfg(InstancePtr, CCCfg, CCID, &CCSeqBitmap,
			      &CarrierCfg);

	OnesCounter = XDfeCcf_CountOnesInBitmap(InstancePtr, CCSeqBitmap);
	if (OnesCounter == 0) {
		metal_log(METAL_LOG_ERROR, "CCID %d is not allocated %s\n",
			  CCID, __func__);
		return XST_FAILURE;
	}

	/* Calculate TDataDelay */
	*TDataDelay = (NumPadding + Tap) *
			      (InstancePtr->SequenceLength / OnesCounter) +
		      DataLatency;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Sets uplink/downlink register bank.
*
* @param    InstancePtr Pointer to the Channel Filter instance.
* @param    RegBank Register bank value to be set.
*
****************************************************************************/
void XDfeCcf_SetRegBank(const XDfeCcf *InstancePtr, u32 RegBank)
{
	Xil_AssertVoid(InstancePtr != NULL);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_REG_BANK_OFFSET, RegBank);
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
