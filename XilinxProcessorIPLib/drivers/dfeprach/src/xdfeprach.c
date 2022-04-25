/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach.c
* Contains the APIs for DFE Prach component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     03/08/21 Initial version
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/10/21 Set sequence length only once
*       dc     04/18/21 Update trigger and event handlers
*       dc     04/21/21 Update due to restructured registers
*       dc     05/08/21 Update to common trigger
*       dc     05/18/21 Handling RachUpdate trigger
* 1.1   dc     06/30/21 Doxygen documentation update
*       dc     07/13/21 Update to common latency requirements
*       dc     07/21/21 Add and reorganise examples
*       dc     10/26/21 Make driver R5 compatible
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*       dc     11/26/21 Model parameter NumCCPerAntenna workaround
*       dc     11/26/21 Set sequence length in GetEmptyCCCfg
*       dc     11/26/21 Add SetAntennaCfgInCCCfg API
*       dc     11/26/21 Assert RachChan equal RCId
*       dc     12/17/21 Update after documentation review
* 1.3   dc     01/11/22 Compilation warrning fix
*       dc     01/19/22 Assert RachUpdate trigger
*       dc     01/31/22 Add CORE_SETTINGS register
*       dc     02/17/22 Physical channel index RACH config array
*       dc     03/21/22 Add prefix to global variables
* 1.4   dc     04/04/22 Correct PatternPeriod represantion
*       dc     04/06/22 Update documentation
*
* </pre>
* @addtogroup dfeprach Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/
#include "xdfeprach.h"
#include "xdfeprach_hw.h"
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
#define XDFEPRACH_SEQUENCE_ENTRY_DEFAULT (0U) /* Default sequence entry flag */
#define XDFEPRACH_SEQUENCE_ENTRY_NULL (-1) /* Null sequence entry flag */
#define XDFEPRACH_NO_EMPTY_CCID_FLAG (0xFFFFU) /* Not Empty CCID flag */
#define XDFEPRACH_U32_NUM_BITS (32U) /**< Number of bits in register */
/**
* @endcond
*/

#define XDFEPRACH_DRIVER_VERSION_MINOR                                         \
	(4U) /**< Driver's minor version number */
#define XDFEPRACH_DRIVER_VERSION_MAJOR                                         \
	(1U) /**< Driver's major version number */

/************************** Function Prototypes *****************************/
/**
* @cond nocomments
*/
static void XDfePrach_GetRC(const XDfePrach *InstancePtr, bool Next, u32 RCId,
			    XDfePrach_RCCfg *RCCfg);
static void XDfePrach_AddRC(u32 RCId, u32 RachChan, s32 CCID,
			    XDfePrach_RCCfg *RCCfg, XDfePrach_DDCCfg *DdcCfg,
			    XDfePrach_NCO *NcoCfg,
			    XDfePrach_Schedule *Schedule);
static void XDfePrach_RemoveOneRC(XDfePrach_InternalChannelCfg *InternalRCCfg);
static void XDfePrach_SetRC(const XDfePrach *InstancePtr,
			    XDfePrach_RCCfg *RCCfg, u32 RCId);
static void XDfePrach_GetRCEnable(const XDfePrach *InstancePtr, bool Next,
				  u32 RCId, u32 *Enable);
static void XDfePrach_AddRCEnable(u32 Enable,
				  XDfePrach_InternalChannelCfg *InternalRCCfg);
static void XDfePrach_GetRCRestart(const XDfePrach *InstancePtr, bool Next,
				   u32 RCId, u32 *Restart);
static void XDfePrach_AddRCRestart(u32 Restart, XDfePrach_RCCfg *RCCfg,
				   u32 RCId);
static void XDfePrach_GetRachChannel(const XDfePrach *InstancePtr, bool Next,
				     u32 RCId, u32 *RachChan);
static void
XDfePrach_AddRachChannel(u32 RachChan,
			 XDfePrach_InternalChannelCfg *InternalRCCfg);
static void XDfePrach_GetRC_CCID(const XDfePrach *InstancePtr, bool Next,
				 u32 RCId, s32 *CCID);
static void XDfePrach_AddRC_CCID(s32 CCID, XDfePrach_RCCfg *RCCfg, u32 RCId);
static void XDfePrach_GetNCO(const XDfePrach *InstancePtr, u32 RCId,
			     u32 RachChan, XDfePrach_NCO *NcoCfg);
static void XDfePrach_AddNCO(XDfePrach_RCCfg *RCCfg,
			     const XDfePrach_NCO *NcoCfg, u32 RCId);
static void XDfePrach_SetNCO(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *RCCfg, u32 RCId);
static void XDfePrach_GetDDC(const XDfePrach *InstancePtr, u32 RachChan,
			     XDfePrach_DDCCfg *DdcCfg);
static void XDfePrach_AddDDC(XDfePrach_RCCfg *RCCfg,
			     const XDfePrach_DDCCfg *DdcCfg, u32 RCId);
static void XDfePrach_SetDDC(const XDfePrach *InstancePtr,
			     const XDfePrach_RCCfg *RCCfg, u32 RCId);
static void XDfePrach_GetSchedule(const XDfePrach *InstancePtr, bool Next,
				  u32 RCId, XDfePrach_Schedule *Schedule);
static void XDfePrach_AddSchedule(XDfePrach_RCCfg *RCCfg,
				  const XDfePrach_Schedule *Schedule, u32 RCId);
static void XDfePrach_SetSchedule(const XDfePrach *InstancePtr,
				  const XDfePrach_RCCfg *RCCfg, u32 RCId);
/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device XDfePrach_CustomDevice[XDFEPRACH_MAX_NUM_INSTANCES];
extern metal_phys_addr_t XDfePrach_metal_phys[XDFEPRACH_MAX_NUM_INSTANCES];
#endif
extern XDfePrach XDfePrach_Prach[XDFEPRACH_MAX_NUM_INSTANCES];
static u32 XDfePrach_DriverHasBeenRegisteredOnce = 0U;

/************************** Function Definitions ****************************/
extern s32 XDfePrach_RegisterMetal(XDfePrach *InstancePtr,
				   struct metal_device **DevicePtr,
				   const char *DeviceNodeName);
extern s32 XDfePrach_LookupConfig(XDfePrach *InstancePtr);
extern void XDfePrach_CfgInitialize(XDfePrach *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Writes a value to register in a PRACH instance.
*
* @param    InstancePtr Pointer to the PRACH driver instance.
* @param    AddrOffset Address offset relative to instance base address.
* @param    Data Value to be written.
*
****************************************************************************/
void XDfePrach_WriteReg(const XDfePrach *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, (unsigned long)AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Reads a value the register in a PRACH instance.
*
* @param    InstancePtr Pointer to the PRACH driver instance.
* @param    AddrOffset Address offset relative to instance base address.
*
* @return   Register value.
*
****************************************************************************/
u32 XDfePrach_ReadReg(const XDfePrach *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, (unsigned long)AddrOffset);
}

/****************************************************************************/
/**
*
* Writes a bit field value to register.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
* @param    FieldData Bit field data.
*
****************************************************************************/
void XDfePrach_WrRegBitField(const XDfePrach *InstancePtr, u32 Offset,
			     u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((FieldOffset + FieldWidth) <= XDFEPRACH_U32_NUM_BITS);

	Data = XDfePrach_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfePrach_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads a bit field value from the register.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Offset Address offset relative to instance base address.
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset.
*
* @return   Bit field data.
*
****************************************************************************/
u32 XDfePrach_RdRegBitField(const XDfePrach *InstancePtr, u32 Offset,
			    u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEPRACH_U32_NUM_BITS);

	Data = XDfePrach_ReadReg(InstancePtr, Offset);
	return ((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U));
}

/****************************************************************************/
/**
*
* Reads a bit field value from the u32 variable.
*
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 data which bit field this function reads.
*
* @return   Bit field value.
*
****************************************************************************/
u32 XDfePrach_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEPRACH_U32_NUM_BITS);
	return (((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U)));
}

/****************************************************************************/
/**
*
* Writes a bit field value to the u32 variable.
*
* @param    FieldWidth Bit field width.
* @param    FieldOffset Bit field offset in bits number.
* @param    Data U32 data which bit field this function reads.
* @param    Val U32 value to be written in the bit field.
*
* @return   Data with a bitfield written.
*
****************************************************************************/
u32 XDfePrach_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
{
	u32 BitFieldSet;
	u32 BitFieldClear;
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEPRACH_U32_NUM_BITS);

	BitFieldSet = (Val & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	BitFieldClear =
		Data & (~((((u32)1U << FieldWidth) - 1U) << FieldOffset));
	return (BitFieldSet | BitFieldClear);
}

/************************ PRACH Common functions ******************************/

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
static s32 XDfePrach_GetNotUsedCCID(XDfePrach_CCSequence *Sequence)
{
	u32 Index;
	s32 NotUsedCCID;

	/* Not used Sequence.CCID[] has value -1, but the values in the range
	   [0,15] can be written in the registers, only. Now, we have to detect
	   not used CCID, and save it for the later usage. */
	for (NotUsedCCID = 0U; NotUsedCCID < XDFEPRACH_CC_NUM_MAX;
	     NotUsedCCID++) {
		for (Index = 0U; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
			if (Sequence->CCID[Index] == NotUsedCCID) {
				break;
			}
		}
		if (Index == XDFEPRACH_CC_NUM_MAX) {
			break;
		}
	}
	return (NotUsedCCID);
}

/****************************************************************************/
/**
*
* Adds the specified CCID to the CC sequence. The sequence is defined with
* CCSeqBitmap where bit0 corresponds to CC[0], bit1 to CC[1], and so on.
*
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC ID.
* @param    CCSeqBitmap maps the sequence.
* @param    CCIDSequence CC sequence array.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfePrach_AddCCIDAndTranslateSeq(XDfePrach *InstancePtr, s32 CCID,
					    u32 CCSeqBitmap,
					    XDfePrach_CCSequence *CCIDSequence)
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
	Mask = 1U;
	for (Index = 0U; Index < InstancePtr->SequenceLength; Index++) {
		if (CCSeqBitmap & Mask) {
			OnesCounter++;
		}
		Mask <<= 1U;
	}

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
			    XDFEPRACH_SEQUENCE_ENTRY_NULL) {
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
	InstancePtr->NotUsedCCID = XDfePrach_GetNotUsedCCID(CCIDSequence);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes the specified CCID from the CC sequence and replaces the CCID
* entries with null (8).
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC ID.
* @param    CCIDSequence CC sequence array.
*
****************************************************************************/
static void XDfePrach_RemoveCCID(XDfePrach *InstancePtr, s32 CCID,
				 XDfePrach_CCSequence *CCIDSequence)
{
	u32 Index;

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] =
				XDFEPRACH_SEQUENCE_ENTRY_NULL;
		}
	}

	/* Set not used CCID */
	InstancePtr->NotUsedCCID = XDfePrach_GetNotUsedCCID(CCIDSequence);
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Detect Rate.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCSeqBitmap Sequence map.
* @param    Rate Rate returned value.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if an error occurs.
*
****************************************************************************/
static u32 XDfePrach_FindRate(const XDfePrach *InstancePtr, u32 CCSeqBitmap,
			      u32 *Rate)
{
	u32 Index;
	u32 Mask = 1U;
	u32 OnesCounter = 0U;
	u32 CCRatio;

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
		/* Calculate conversion ratio from CC rates to 30.72MHz or
		   equivalent if not using 491.52MHz */
		CCRatio = InstancePtr->Config.NumAntennaSlot *
			  (InstancePtr->SequenceLength / OnesCounter);
		/* Select Rate from Interpolation/decimation rate */
		switch (CCRatio) {
		case 2: /* 2: 8x interpolation/decimation */
			*Rate = XDFEPRACH_CC_MAPPING_DECIMATION_RATE_8X;
			break;
		case 4: /* 3: 4x interpolation/decimation */
			*Rate = XDFEPRACH_CC_MAPPING_DECIMATION_RATE_4X;
			break;
		case 8: /* 4: 2x interpolation/decimation */
			*Rate = XDFEPRACH_CC_MAPPING_DECIMATION_RATE_2X;
			break;
		case 16: /* 5: 1x interpolation/decimation */
			*Rate = XDFEPRACH_CC_MAPPING_DECIMATION_RATE_1X;
			break;
		default:
			metal_log(METAL_LOG_ERROR,
				  "Wrong conversion ratio %d\n", CCRatio);
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Sets the next CC configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    NextCCCfg Next CC configuration container.
*
****************************************************************************/
static void XDfePrach_SetNextCCCfg(const XDfePrach *InstancePtr,
				   const XDfePrach_CCCfg *NextCCCfg)
{
	u32 Data = 0U;
	u32 Index;
	u32 SeqLength;
	s32 NextCCID[XDFEPRACH_SEQ_LENGTH_MAX];

	/* Prepare NextCCID[] to be written to registers */
	for (Index = 0U; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		if ((NextCCCfg->Sequence.CCID[Index] ==
		     XDFEPRACH_SEQUENCE_ENTRY_NULL) ||
		    (Index >= InstancePtr->SequenceLength)) {
			NextCCID[Index] = InstancePtr->NotUsedCCID;
		} else {
			NextCCID[Index] = NextCCCfg->Sequence.CCID[Index];
		}
	}

	/* Sequence Length should remain the same, so copy the sequence length
	   from CURRENT to NEXT */
	SeqLength = XDfePrach_ReadReg(InstancePtr,
				      XDFEPRACH_CC_SEQUENCE_LENGTH_CURRENT);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_CC_SEQUENCE_LENGTH_NEXT,
			   SeqLength);

	/* Write CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEPRACH_SEQ_LENGTH_MAX; Index++) {
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_CC_SEQUENCE_NEXT +
					   (Index * sizeof(u32)),
				   NextCCID[Index]);

		Data = XDfePrach_WrBitField(
			XDFEPRACH_CC_MAPPING_ENABLE_WIDTH,
			XDFEPRACH_CC_MAPPING_ENABLE_OFFSET, 0U,
			NextCCCfg->CarrierCfg[Index].Enable);
		Data = XDfePrach_WrBitField(XDFEPRACH_CC_MAPPING_SCS_WIDTH,
					    XDFEPRACH_CC_MAPPING_SCS_OFFSET,
					    Data,
					    NextCCCfg->CarrierCfg[Index].SCS);
		Data = XDfePrach_WrBitField(
			XDFEPRACH_CC_MAPPING_DECIMATION_RATE_WIDTH,
			XDFEPRACH_CC_MAPPING_DECIMATION_RATE_OFFSET, Data,
			NextCCCfg->CarrierCfg[Index].CCRate);
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_CC_MAPPING_NEXT +
					   (Index * sizeof(u32)),
				   Data);
	}
}

/****************************************************************************/
/**
*
* Adds a single instance of an RCCfg.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Boolean flag indicating NEXT or CURRENT register.
* @param    RCId RC Id.
* @param    RCCfg RC config container.
*
****************************************************************************/
static void XDfePrach_GetRC(const XDfePrach *InstancePtr, bool Next, u32 RCId,
			    XDfePrach_RCCfg *RCCfg)
{
	RCCfg->InternalRCCfg[RCId].RCId = RCId;

	/* get the RCID's enable status */
	XDfePrach_GetRCEnable(InstancePtr, Next, RCId,
			      &RCCfg->InternalRCCfg[RCId].Enable);
	/* get the restart status */
	XDfePrach_GetRCRestart(InstancePtr, Next, RCId,
			       &RCCfg->InternalRCCfg[RCId].Restart);
	/* get the rach channel number */
	XDfePrach_GetRachChannel(InstancePtr, Next, RCId,
				 &RCCfg->InternalRCCfg[RCId].RachChannel);
	/* get the CCID number */
	XDfePrach_GetRC_CCID(InstancePtr, Next, RCId,
			     &RCCfg->InternalRCCfg[RCId].CCID);
	/* get the NCO configuration - no Next/current available here! */
	XDfePrach_GetNCO(InstancePtr, RCId,
			 RCCfg->InternalRCCfg[RCId].RachChannel,
			 &RCCfg->NcoCfg[RCId]);
	/* get the DDC configuration - no Next/current available here! */
	XDfePrach_GetDDC(InstancePtr, RCId, &RCCfg->DdcCfg[RCId]);
	/* get the DDC configuration */
	XDfePrach_GetSchedule(InstancePtr, Next, RCId,
			      &RCCfg->StaticSchedule[RCId]);
}
/****************************************************************************/
/**
*
* Adds a single instance of an RCCfg.
*
* @param    RCId RC Id.
* @param    RachChan RACH channel Id.
* @param    CCID CC Id.
* @param    RCCfg RC config container.
* @param    DdcCfg DDC data container.
* @param    NcoCfg NCO data container.
* @param    Schedule Schedule data container.
*
****************************************************************************/
static void XDfePrach_AddRC(u32 RCId, u32 RachChan, s32 CCID,
			    XDfePrach_RCCfg *RCCfg, XDfePrach_DDCCfg *DdcCfg,
			    XDfePrach_NCO *NcoCfg, XDfePrach_Schedule *Schedule)
{
	RCCfg->InternalRCCfg[RCId].RCId = RCId;
	/* Set the physical channel first so the physicla channels are loaded
	   correctly */
	XDfePrach_AddRachChannel(RachChan, &RCCfg->InternalRCCfg[RCId]);
	/* Add the DDC cfg */
	XDfePrach_AddDDC(RCCfg, DdcCfg, RCId);
	/* Add the NCO */
	XDfePrach_AddNCO(RCCfg, NcoCfg, RCId);
	/* Add the schedule */
	XDfePrach_AddSchedule(RCCfg, Schedule, RCId);
	/* Add the CCID number */
	XDfePrach_AddRC_CCID(CCID, RCCfg, RCId);
	/* Adding a new channel - always restart it */
	XDfePrach_AddRCRestart(XDFEPRACH_RCID_MAPPING_CHANNEL_RESTART_YES,
			       RCCfg, RCId);
	/* Enable the RC */
	XDfePrach_AddRCEnable(XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLED,
			      &RCCfg->InternalRCCfg[RCId]);
}

/****************************************************************************/
/**
*
* Removes a single instance of an RCCfg.
*
* @param    InternalRCCfg Internal RCCfg configuration container.
*
****************************************************************************/
static void XDfePrach_RemoveOneRC(XDfePrach_InternalChannelCfg *InternalRCCfg)
{
	/* Simplest method is to mark the RCID enable as 0. Disable the RC. */
	XDfePrach_AddRCEnable(XDFEPRACH_RCID_MAPPING_CHANNEL_NOT_ENABLED,
			      InternalRCCfg);
}

/****************************************************************************/
/**
*
* Writes a RC channel to the registers.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC config container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_SetRC(const XDfePrach *InstancePtr,
			    XDfePrach_RCCfg *RCCfg, u32 RCId)
{
	u32 Data = 0U;
	u32 Offset;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RCCfg != NULL);

	XDfePrach_SetSchedule(InstancePtr, RCCfg, RCId);

	/* Only update the DDC and the NCO if the channel is restarting,
	   otherwise they can be left alone. */
	if (RCCfg->InternalRCCfg[RCId].Restart !=
	    XDFEPRACH_RCID_MAPPING_CHANNEL_RESTART_NO) {
		XDfePrach_SetNCO(InstancePtr, RCCfg, RCId);
		XDfePrach_SetDDC(InstancePtr, RCCfg, RCId);
	}

	/* Write the mapping register */
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLE_WIDTH,
		XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLE_OFFSET, 0U,
		RCCfg->InternalRCCfg[RCId].Enable);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_MAPPING_CHANNEL_RACH_CHANNEL_WIDTH,
		XDFEPRACH_RCID_MAPPING_CHANNEL_RACH_CHANNEL_OFFSET, Data,
		RCCfg->InternalRCCfg[RCId].RachChannel);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_MAPPING_CHANNEL_RCID_RESTART_WIDTH,
		XDFEPRACH_RCID_MAPPING_CHANNEL_RCID_RESTART_OFFSET, Data,
		RCCfg->InternalRCCfg[RCId].Restart);
	Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_NEXT + (RCId * sizeof(u32));
	XDfePrach_WriteReg(InstancePtr, Offset, Data);

	XDfePrach_WriteReg(InstancePtr,
			   XDFEPRACH_RCID_MAPPING_SOURCE_NEXT +
				   (RCId * sizeof(u32)),
			   RCCfg->InternalRCCfg[RCId].CCID);
}

/****************************************************************************/
/**
*
* Reads the physical Enable for a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Boolean flag indicating NEXT or CURRENT register.
* @param    RCId RC Id.
* @param    Enable Flag indicating restart.
*
****************************************************************************/
static void XDfePrach_GetRCEnable(const XDfePrach *InstancePtr, bool Next,
				  u32 RCId, u32 *Enable)
{
	u32 Offset;

	if (Next == true) {
		Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_NEXT;
	} else {
		Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_CURRENT;
	}
	*Enable = XDfePrach_RdRegBitField(
		InstancePtr, Offset + (RCId * sizeof(u32)),
		XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLE_WIDTH,
		XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLE_OFFSET);
}

/****************************************************************************/
/**
*
* Adds the Enable to an RCCfg instance.
*
* @param    Enable Flag indicating restart.
* @param    InternalRCCfg Internal RCCfg configuration container.
*
****************************************************************************/
static void XDfePrach_AddRCEnable(u32 Enable,
				  XDfePrach_InternalChannelCfg *InternalRCCfg)
{
	InternalRCCfg->Enable = Enable;
}

/****************************************************************************/
/**
*
* Reads the Restart signal for a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Boolean flag indicating NEXT or CURRENT register.
* @param    RCId RC Id.
* @param    Restart Flag indicating restart:
*                - 0 = no restart,
*                - 1 = restart and flush.
*
****************************************************************************/
static void XDfePrach_GetRCRestart(const XDfePrach *InstancePtr, bool Next,
				   u32 RCId, u32 *Restart)
{
	u32 Offset;

	if (Next == true) {
		Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_NEXT;
	} else {
		Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_CURRENT;
	}
	*Restart = XDfePrach_RdRegBitField(
		InstancePtr, Offset + (RCId * sizeof(u32)),
		XDFEPRACH_RCID_MAPPING_CHANNEL_RCID_RESTART_WIDTH,
		XDFEPRACH_RCID_MAPPING_CHANNEL_RCID_RESTART_OFFSET);
}

/****************************************************************************/
/**
*
* Adds the Restart to an RCCfg instance.
*
* @param    Restart Flag indicating restart.
* @param    RCCfg RC config container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_AddRCRestart(u32 Restart, XDfePrach_RCCfg *RCCfg,
				   u32 RCId)
{
	RCCfg->InternalRCCfg[RCId].Restart = Restart;
}

/****************************************************************************/
/**
*
* Read the physical RACH channel for a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Boolean flag indicating NEXT or CURRENT register.
* @param    RCId RC Id.
* @param    RachChan RACH Channel Id.
*
****************************************************************************/
static void XDfePrach_GetRachChannel(const XDfePrach *InstancePtr, bool Next,
				     u32 RCId, u32 *RachChan)
{
	u32 Offset;

	if (Next == true) {
		Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_NEXT;
	} else {
		Offset = XDFEPRACH_RCID_MAPPING_CHANNEL_CURRENT;
	}
	*RachChan = XDfePrach_RdRegBitField(
		InstancePtr, Offset + (RCId * sizeof(u32)),
		XDFEPRACH_RCID_MAPPING_CHANNEL_RACH_CHANNEL_WIDTH,
		XDFEPRACH_RCID_MAPPING_CHANNEL_RACH_CHANNEL_OFFSET);
}

/****************************************************************************/
/**
*
* Adds the Rach Channel to a RCCfg instance.
*
* @param    RachChan RACH channel Id.
* @param    InternalRCCfg Internal RCCfg configuration container.
*
****************************************************************************/
static void
XDfePrach_AddRachChannel(u32 RachChan,
			 XDfePrach_InternalChannelCfg *InternalRCCfg)
{
	InternalRCCfg->RachChannel = RachChan;
}

/****************************************************************************/
/**
*
* Reads the CCID allocated to a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Boolean flag indicating NEXT or CURRENT register.
* @param    RCId RC Id.
* @param    CCID CC Id.
*
****************************************************************************/
static void XDfePrach_GetRC_CCID(const XDfePrach *InstancePtr, bool Next,
				 u32 RCId, s32 *CCID)
{
	u32 Offset;

	if (Next == true) {
		Offset = XDFEPRACH_RCID_MAPPING_SOURCE_NEXT;
	} else {
		Offset = XDFEPRACH_RCID_MAPPING_SOURCE_CURRENT;
	}
	*CCID = XDfePrach_RdRegBitField(
		InstancePtr, Offset + (RCId * sizeof(u32)),
		XDFEPRACH_RCID_MAPPING_SOURCE_CCID_WIDTH,
		XDFEPRACH_RCID_MAPPING_SOURCE_CCID_OFFSET);
}

/****************************************************************************/
/**
*
* Adds the CCID to an RCCfg instance.
*
* @param    CCID CC Id.
* @param    RCCfg RC config container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_AddRC_CCID(s32 CCID, XDfePrach_RCCfg *RCCfg, u32 RCId)
{
	RCCfg->InternalRCCfg[RCId].CCID = CCID;
}

/****************************************************************************/
/**
*
* Reads the NCO for a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCId RC Id.
* @param    RachChan RACH channel number.
* @param    NcoCfg NCO data container.
*
****************************************************************************/
static void XDfePrach_GetNCO(const XDfePrach *InstancePtr, u32 RCId,
			     u32 RachChan, XDfePrach_NCO *NcoCfg)
{
	u32 Offset;

	Offset = XDFEPRACH_PHASE_PHASE_OFFSET +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	NcoCfg->PhaseOffset = XDfePrach_ReadReg(InstancePtr, Offset);
	Offset = XDFEPRACH_PHASE_PHASE_ACC +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	NcoCfg->PhaseAcc = XDfePrach_ReadReg(InstancePtr, Offset);
	Offset = XDFEPRACH_PHASE_DUAL_MOD_COUNT +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	NcoCfg->DualModCount = XDfePrach_ReadReg(InstancePtr, Offset);
	Offset = XDFEPRACH_PHASE_DUAL_MOD_SEL +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	NcoCfg->DualModSel = XDfePrach_ReadReg(InstancePtr, Offset);
	Offset = XDFEPRACH_NCO_GAIN + (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	NcoCfg->NcoGain = XDfePrach_ReadReg(InstancePtr, Offset);

	Offset = XDFEPRACH_FREQUENCY_CONTROL_WORD +
		 (RCId * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	NcoCfg->Frequency = XDfePrach_ReadReg(InstancePtr, Offset);
}

/****************************************************************************/
/**
*
* Adds and populates new NCO to the RCCfg.
*
* @param    RCCfg RC config container.
* @param    NcoCfg NCO data container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_AddNCO(XDfePrach_RCCfg *RCCfg,
			     const XDfePrach_NCO *NcoCfg, u32 RCId)
{
	RCCfg->NcoCfg[RCId].PhaseOffset = NcoCfg->PhaseOffset;
	RCCfg->NcoCfg[RCId].PhaseAcc = NcoCfg->PhaseAcc;
	RCCfg->NcoCfg[RCId].DualModCount = NcoCfg->DualModCount;
	RCCfg->NcoCfg[RCId].DualModSel = NcoCfg->DualModSel;
	RCCfg->NcoCfg[RCId].Frequency = NcoCfg->Frequency;
	RCCfg->NcoCfg[RCId].NcoGain = NcoCfg->NcoGain;
}

/****************************************************************************/
/**
*
* Loads the NCO registers from the RCCfg.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC configuration container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_SetNCO(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *RCCfg, u32 RCId)
{
	u32 Offset;

	/* Set RACH_MIXER.NCO_CTRL[RCCfg->RachChan].PHASE */
	Offset = XDFEPRACH_PHASE_PHASE_OFFSET +
		 (RCCfg->InternalRCCfg[RCId].RachChannel *
		  XDFEPRACH_NCO_CTRL_ADDR_STEP);
	XDfePrach_WriteReg(InstancePtr, Offset,
			   RCCfg->NcoCfg[RCId].PhaseOffset);
	Offset = XDFEPRACH_PHASE_PHASE_ACC +
		 (RCCfg->InternalRCCfg[RCId].RachChannel *
		  XDFEPRACH_NCO_CTRL_ADDR_STEP);
	XDfePrach_WriteReg(InstancePtr, Offset, RCCfg->NcoCfg[RCId].PhaseAcc);
	Offset = XDFEPRACH_PHASE_DUAL_MOD_COUNT +
		 (RCCfg->InternalRCCfg[RCId].RachChannel *
		  XDFEPRACH_NCO_CTRL_ADDR_STEP);
	XDfePrach_WriteReg(InstancePtr, Offset,
			   RCCfg->NcoCfg[RCId].DualModCount);
	Offset = XDFEPRACH_PHASE_DUAL_MOD_SEL +
		 (RCCfg->InternalRCCfg[RCId].RachChannel *
		  XDFEPRACH_NCO_CTRL_ADDR_STEP);
	XDfePrach_WriteReg(InstancePtr, Offset, RCCfg->NcoCfg[RCId].DualModSel);

	Offset = XDFEPRACH_NCO_GAIN + (RCCfg->InternalRCCfg[RCId].RachChannel *
				       XDFEPRACH_NCO_CTRL_ADDR_STEP);
	XDfePrach_WriteReg(InstancePtr, Offset, RCCfg->NcoCfg[RCId].NcoGain);

	Offset = XDFEPRACH_FREQUENCY_CONTROL_WORD +
		 (RCCfg->InternalRCCfg[RCId].RachChannel * sizeof(u32));
	XDfePrach_WriteReg(InstancePtr, Offset, RCCfg->NcoCfg[RCId].Frequency);
}

/****************************************************************************/
/**
*
* Reads the DDC for a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCId RC Id.
* @param    DdcCfg DDC data container.
*
****************************************************************************/
static void XDfePrach_GetDDC(const XDfePrach *InstancePtr, u32 RCId,
			     XDfePrach_DDCCfg *DdcCfg)
{
	u32 Offset;
	u32 Data;

	Offset = XDFEPRACH_CHANNEL_CONFIG_RATE + (RCId * sizeof(u32));
	Data = XDfePrach_ReadReg(InstancePtr, Offset);

	DdcCfg->DecimationRate = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_RATE_DECIMATION_RATE_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_RATE_DECIMATION_RATE_OFFSET, Data);
	DdcCfg->SCS =
		XDfePrach_RdBitField(XDFEPRACH_CHANNEL_CONFIG_RATE_SCS_WIDTH,
				     XDFEPRACH_CHANNEL_CONFIG_RATE_SCS_OFFSET,
				     Data);
	Offset = XDFEPRACH_CHANNEL_CONFIG_GAIN + (RCId * sizeof(u32));
	Data = XDfePrach_ReadReg(InstancePtr, Offset);
	DdcCfg->RachGain[0] = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN0_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN0_OFFSET, Data);
	DdcCfg->RachGain[1] = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN1_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN1_OFFSET, Data);
	DdcCfg->RachGain[2] = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN2_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN2_OFFSET, Data);
	DdcCfg->RachGain[3] = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN3_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN3_OFFSET, Data);
	DdcCfg->RachGain[4] = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN4_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN4_OFFSET, Data);
	DdcCfg->RachGain[5] = XDfePrach_RdBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN5_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN5_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Adds a new DDC to the RCCfg.
*
* @param    RCCfg RC configuration container.
* @param    DdcCfg DDC data container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_AddDDC(XDfePrach_RCCfg *RCCfg,
			     const XDfePrach_DDCCfg *DdcCfg, u32 RCId)
{
	RCCfg->DdcCfg[RCId].DecimationRate = DdcCfg->DecimationRate;
	RCCfg->DdcCfg[RCId].SCS = DdcCfg->SCS;
	RCCfg->DdcCfg[RCId].RachGain[0] = DdcCfg->RachGain[0];
	RCCfg->DdcCfg[RCId].RachGain[1] = DdcCfg->RachGain[1];
	RCCfg->DdcCfg[RCId].RachGain[2] = DdcCfg->RachGain[2];
	RCCfg->DdcCfg[RCId].RachGain[3] = DdcCfg->RachGain[3];
	RCCfg->DdcCfg[RCId].RachGain[4] = DdcCfg->RachGain[4];
	RCCfg->DdcCfg[RCId].RachGain[5] = DdcCfg->RachGain[5];
}

/****************************************************************************/
/**
*
* Loads the DDC registers from the RCCfg.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC configuration container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_SetDDC(const XDfePrach *InstancePtr,
			     const XDfePrach_RCCfg *RCCfg, u32 RCId)
{
	u32 Offset;
	u32 Data;

	/* Set RACH_MIXER.CHANNEL[RCCfg->RachChan].CONFIG */
	Data = XDfePrach_WrBitField(XDFEPRACH_CHANNEL_CONFIG_RATE_SCS_WIDTH,
				    XDFEPRACH_CHANNEL_CONFIG_RATE_SCS_OFFSET,
				    0U, RCCfg->DdcCfg[RCId].SCS);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_RATE_DECIMATION_RATE_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_RATE_DECIMATION_RATE_OFFSET, Data,
		RCCfg->DdcCfg[RCId].DecimationRate);
	Offset = XDFEPRACH_CHANNEL_CONFIG_RATE + (RCId * sizeof(u32));
	XDfePrach_WriteReg(InstancePtr, Offset, Data);

	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN0_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN0_OFFSET, 0U,
		RCCfg->DdcCfg[RCId].RachGain[0]);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN1_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN1_OFFSET, Data,
		RCCfg->DdcCfg[RCId].RachGain[1]);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN2_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN2_OFFSET, Data,
		RCCfg->DdcCfg[RCId].RachGain[2]);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN3_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN3_OFFSET, Data,
		RCCfg->DdcCfg[RCId].RachGain[3]);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN4_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN4_OFFSET, Data,
		RCCfg->DdcCfg[RCId].RachGain[4]);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN5_WIDTH,
		XDFEPRACH_CHANNEL_CONFIG_GAIN_DECIMATION_GAIN5_OFFSET, Data,
		RCCfg->DdcCfg[RCId].RachGain[5]);
	Offset = XDFEPRACH_CHANNEL_CONFIG_GAIN + (RCId * sizeof(u32));
	XDfePrach_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads the Current or Next Static Schedule for a given RCID.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Flag indicating NEXT or CURRENT register.
* @param    RCId RACH channel number.
* @param    Schedule Schedule data container.
*
****************************************************************************/
static void XDfePrach_GetSchedule(const XDfePrach *InstancePtr, bool Next,
				  u32 RCId, XDfePrach_Schedule *Schedule)
{
	u32 Offset;
	u32 Data2;
	u32 Data3;

	if (Next == true) {
		Offset = XDFEPRACH_RCID_SCHEDULE_LOCATION_NEXT +
			 (RCId * sizeof(u32));
		Data2 = XDfePrach_ReadReg(InstancePtr, Offset);
		Offset = XDFEPRACH_RCID_SCHEDULE_LENGTH_NEXT +
			 (RCId * sizeof(u32));
		Data3 = XDfePrach_ReadReg(InstancePtr, Offset);
	} else {
		Offset = XDFEPRACH_RCID_SCHEDULE_LOCATION_CURRENT +
			 (RCId * sizeof(u32));
		Data2 = XDfePrach_ReadReg(InstancePtr, Offset);
		Offset = XDFEPRACH_RCID_SCHEDULE_LENGTH_CURRENT +
			 (RCId * sizeof(u32));
		Data3 = XDfePrach_ReadReg(InstancePtr, Offset);
	}

	/* Read RCID_SCHEDULE.LOCATION */
	Schedule->PatternPeriod =
		1U +
		XDfePrach_RdBitField(
			XDFEPRACH_RCID_SCHEDULE_LOCATION_PATTERN_PERIOD_WIDTH,
			XDFEPRACH_RCID_SCHEDULE_LOCATION_PATTERN_PERIOD_OFFSET,
			Data2);
	Schedule->FrameID = XDfePrach_RdBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_FRAMEID_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_FRAMEID_OFFSET, Data2);
	Schedule->SubframeID = XDfePrach_RdBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SUBFRAME_ID_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SUBFRAME_ID_OFFSET, Data2);
	Schedule->SlotId = XDfePrach_RdBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SLOT_ID_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SLOT_ID_OFFSET, Data2);

	/* Read RCID_SCHEDULE.LENGTH */
	Schedule->Duration = XDfePrach_RdBitField(
		XDFEPRACH_RCID_SCHEDULE_LENGTH_DURATION_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LENGTH_DURATION_OFFSET, Data3);
	Schedule->Repeats = XDfePrach_RdBitField(
		XDFEPRACH_RCID_SCHEDULE_LENGTH_NUM_REPEATS_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LENGTH_NUM_REPEATS_OFFSET, Data3);
}

/****************************************************************************/
/**
*
* Adds a new Static Schedule to the RCCfg.
*
* @param    RCCfg RC config container.
* @param    Schedule Schedule data container.
* @param    RCId RC Id.
*
****************************************************************************/
static void XDfePrach_AddSchedule(XDfePrach_RCCfg *RCCfg,
				  const XDfePrach_Schedule *Schedule, u32 RCId)
{
	RCCfg->StaticSchedule[RCId].PatternPeriod = Schedule->PatternPeriod;
	RCCfg->StaticSchedule[RCId].FrameID = Schedule->FrameID;
	RCCfg->StaticSchedule[RCId].SubframeID = Schedule->SubframeID;
	RCCfg->StaticSchedule[RCId].SlotId = Schedule->SlotId;
	RCCfg->StaticSchedule[RCId].Duration = Schedule->Duration;
	RCCfg->StaticSchedule[RCId].Repeats = Schedule->Repeats;
}

/****************************************************************************/
/**
*
* Loads the Static Schedule registers from the RCCfg.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC configuration container.
*
****************************************************************************/
static void XDfePrach_SetSchedule(const XDfePrach *InstancePtr,
				  const XDfePrach_RCCfg *RCCfg, u32 RCId)
{
	u32 Offset;
	u32 Data;
	/* Set RCID_SCHEDULE.LOCATION */
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_PATTERN_PERIOD_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_PATTERN_PERIOD_OFFSET, 0U,
		(RCCfg->StaticSchedule[RCId].PatternPeriod - 1U));
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_FRAMEID_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_FRAMEID_OFFSET, Data,
		RCCfg->StaticSchedule[RCId].FrameID);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SUBFRAME_ID_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SUBFRAME_ID_OFFSET, Data,
		RCCfg->StaticSchedule[RCId].SubframeID);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SLOT_ID_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LOCATION_SLOT_ID_OFFSET, Data,
		RCCfg->StaticSchedule[RCId].SlotId);
	Offset = XDFEPRACH_RCID_SCHEDULE_LOCATION_NEXT + (RCId * sizeof(u32));
	XDfePrach_WriteReg(InstancePtr, Offset, Data);

	/* Set RCID_SCHEDULE.LENGTH */
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_SCHEDULE_LENGTH_DURATION_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LENGTH_DURATION_OFFSET, 0U,
		RCCfg->StaticSchedule[RCId].Duration);
	Data = XDfePrach_WrBitField(
		XDFEPRACH_RCID_SCHEDULE_LENGTH_NUM_REPEATS_WIDTH,
		XDFEPRACH_RCID_SCHEDULE_LENGTH_NUM_REPEATS_OFFSET, Data,
		RCCfg->StaticSchedule[RCId].Repeats);
	Offset = XDFEPRACH_RCID_SCHEDULE_LENGTH_NEXT + (RCId * sizeof(u32));
	XDfePrach_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads the trigger and sets enable bit of update trigger. If register
* source, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
static u32 XDfePrach_EnableUpdateTrigger(const XDfePrach *InstancePtr)
{
	u32 Data;

	/* Exit with error if RACH_UPDATE status is high */
	if (XDFEPRACH_RACH_UPDATE_TRIGGERED_HIGH ==
	    XDfePrach_RdRegBitField(InstancePtr, XDFEPRACH_ISR,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET)) {
		metal_log(METAL_LOG_ERROR, "RachUpdate status high in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Enable CCUpdate trigger */
	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET,
			   Data);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Reads the trigger and sets enable bit of LowPower trigger. If register
* source, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
****************************************************************************/
static void XDfePrach_EnableLowPowerTrigger(const XDfePrach *InstancePtr)
{
	u32 Data;

	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET,
			   Data);
}

/****************************************************************************/
/**
*
* Enables the Activate trigger.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
****************************************************************************/
static void XDfePrach_EnableActivateTrigger(const XDfePrach *InstancePtr)
{
	u32 Data;

	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				    XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_STATE_OUTPUT_ENABLED);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET,
			   Data);
}

/****************************************************************************/
/**
*
* Reads the trigger and sets disable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
****************************************************************************/
static void XDfePrach_EnableDeactivateTrigger(const XDfePrach *InstancePtr)
{
	u32 Data;

	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				    XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_STATE_OUTPUT_DISABLED);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET,
			   Data);
}

/****************************************************************************/
/**
*
* Reads the Trigger and resets enable bit of LowPower trigger.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
****************************************************************************/
static void XDfePrach_DisableLowPowerTrigger(const XDfePrach *InstancePtr)
{
	u32 Data;

	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET,
			   Data);
}

/****************************************************************************/
/**
*
* Reads the trigger and sets enable bit of frame marker trigger. If
* register source, then trigger will be applied immediately.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
****************************************************************************/
static void XDfePrach_EnableFrameMarkerTrigger(const XDfePrach *InstancePtr)
{
	u32 Data;

	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_FRAME_INIT_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_FRAME_INIT_OFFSET,
			   Data);
}
/**
* @endcond
*/

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API initialises an instance of the driver.
* Traverse "/sys/bus/platform/device" directory (in Linux), to find registered
* PRACH device with the name DeviceNodeName. The first available slot in
* the instance array XDfePrach_Prach[] will be taken as a DeviceNodeName
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
XDfePrach *XDfePrach_InstanceInit(const char *DeviceNodeName)
{
	u32 Index;
	XDfePrach *InstancePtr;
#ifdef __BAREMETAL__
	char Str[XDFEPRACH_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;
#endif

	Xil_AssertNonvoid(DeviceNodeName != NULL);
	Xil_AssertNonvoid(strlen(DeviceNodeName) <
			  XDFEPRACH_NODE_NAME_MAX_LENGTH);

	/* Is this first PRACH initialisation ever? */
	if (0U == XDfePrach_DriverHasBeenRegisteredOnce) {
		/* Set up environment to non-initialized */
		for (Index = 0; Index < XDFEPRACH_MAX_NUM_INSTANCES; Index++) {
			XDfePrach_Prach[Index].StateId =
				XDFEPRACH_STATE_NOT_READY;
			XDfePrach_Prach[Index].NodeName[0] = '\0';
		}
		XDfePrach_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check has DeviceNodeName been already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	for (Index = 0; Index < XDFEPRACH_MAX_NUM_INSTANCES; Index++) {
		if (0U == strncmp(XDfePrach_Prach[Index].NodeName,
				  DeviceNodeName, strlen(DeviceNodeName))) {
			XDfePrach_Prach[Index].StateId = XDFEPRACH_STATE_READY;
			return &XDfePrach_Prach[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; Index < XDFEPRACH_MAX_NUM_INSTANCES; Index++) {
		if (XDfePrach_Prach[Index].NodeName[0] == '\0') {
			strncpy(XDfePrach_Prach[Index].NodeName, DeviceNodeName,
				XDFEPRACH_NODE_NAME_MAX_LENGTH);
			InstancePtr = &XDfePrach_Prach[Index];
			goto register_metal;
		}
	}

	/* Failing as there is no available slot. */
	return NULL;

register_metal:
#ifdef __BAREMETAL__
	memcpy(Str, InstancePtr->NodeName, XDFEPRACH_NODE_NAME_MAX_LENGTH);
	AddrStr = strtok(Str, ".");
	Addr = strtoul(AddrStr, NULL, 16);
	for (Index = 0; Index < XDFEPRACH_MAX_NUM_INSTANCES; Index++) {
		if (Addr == XDfePrach_metal_phys[Index]) {
			InstancePtr->Device = &XDfePrach_CustomDevice[Index];
			goto bm_register_metal;
		}
	}
	return NULL;
bm_register_metal:
#endif

	/* Register libmetal for this OS process */
	if (XST_SUCCESS != XDfePrach_RegisterMetal(InstancePtr,
						   &InstancePtr->Device,
						   DeviceNodeName)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfePrach_LookupConfig(InstancePtr)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to configure device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Configure HW and the driver instance */
	XDfePrach_CfgInitialize(InstancePtr);

	InstancePtr->StateId = XDFEPRACH_STATE_READY;

	return InstancePtr;

return_error:
	InstancePtr->StateId = XDFEPRACH_STATE_NOT_READY;
	InstancePtr->NodeName[0] = '\0';
	return NULL;
}

/*****************************************************************************/
/**
*
* API closes the instance of a PRACH driver and moves the state machine to
* a Not Ready state.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
******************************************************************************/
void XDfePrach_InstanceClose(XDfePrach *InstancePtr)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; Index < XDFEPRACH_MAX_NUM_INSTANCES; Index++) {
		/* Find the instance in XDfePrach_Prach array */
		if (&XDfePrach_Prach[Index] == InstancePtr) {
			/* Release libmetal */
			metal_device_close(InstancePtr->Device);
			InstancePtr->StateId = XDFEPRACH_STATE_NOT_READY;
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
* Resets PRACH and puts block into a reset state.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
****************************************************************************/
void XDfePrach_Reset(XDfePrach *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEPRACH_STATE_NOT_READY);

	/* Put Prach in reset */
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_RESET_OFFSET,
			   XDFEPRACH_RESET_ON);
	/* Release reset */
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_RESET_OFFSET,
			   XDFEPRACH_RESET_OFF);
	InstancePtr->StateId = XDFEPRACH_STATE_RESET;
}

/****************************************************************************/
/**
*
* Reads configuration from device tree/xparameters.h and IP registers.
* Removes S/W reset and moves the state machine to a Configured state.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Cfg Configuration data container.
*
****************************************************************************/
void XDfePrach_Configure(XDfePrach *InstancePtr, XDfePrach_Cfg *Cfg)
{
	u32 Version;
	u32 ModelParam;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_RESET);
	Xil_AssertVoid(Cfg != NULL);

	/* Read vearsion */
	Version = XDfePrach_ReadReg(InstancePtr, XDFEPRACH_VERSION_OFFSET);
	Cfg->Version.Patch =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_PATCH_WIDTH,
				     XDFEPRACH_VERSION_PATCH_OFFSET, Version);
	Cfg->Version.Revision =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_REVISION_WIDTH,
				     XDFEPRACH_VERSION_REVISION_OFFSET,
				     Version);
	Cfg->Version.Minor =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_MINOR_WIDTH,
				     XDFEPRACH_VERSION_MINOR_OFFSET, Version);
	Cfg->Version.Major =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_MAJOR_WIDTH,
				     XDFEPRACH_VERSION_MAJOR_OFFSET, Version);

	/* Read model parameters */
	ModelParam =
		XDfePrach_ReadReg(InstancePtr, XDFEPRACH_MODEL_PARAM_OFFSET);
	InstancePtr->Config.NumAntenna =
		XDfePrach_RdBitField(XDFEPRACH_MODEL_PARAM_NUM_ANTENNA_WIDTH,
				     XDFEPRACH_MODEL_PARAM_NUM_ANTENNA_OFFSET,
				     ModelParam);
	InstancePtr->Config.NumCCPerAntenna = XDfePrach_RdBitField(
		XDFEPRACH_MODEL_PARAM_NUM_CC_PER_ANTENNA_WIDTH,
		XDFEPRACH_MODEL_PARAM_NUM_CC_PER_ANTENNA_OFFSET, ModelParam);
	/* The workaround for NumCCPerAntenna Model parameter which field is
	   4 bits in size. 0 is converted to 16 */
	if (InstancePtr->Config.NumCCPerAntenna == 0U) {
		InstancePtr->Config.NumCCPerAntenna = XDFEPRACH_CC_NUM_MAX;
	}
	InstancePtr->Config.NumAntennaChannels = XDfePrach_RdBitField(
		XDFEPRACH_MODEL_PARAM_NUM_SLOT_CHANNELS_WIDTH,
		XDFEPRACH_MODEL_PARAM_NUM_SLOT_CHANNELS_OFFSET, ModelParam);
	InstancePtr->Config.NumAntennaSlot =
		XDfePrach_RdBitField(XDFEPRACH_MODEL_PARAM_NUM_SLOTS_WIDTH,
				     XDFEPRACH_MODEL_PARAM_NUM_SLOTS_OFFSET,
				     ModelParam);
	InstancePtr->Config.NumRachLanes = XDfePrach_RdBitField(
		XDFEPRACH_MODEL_PARAM_NUM_RACH_LANES_WIDTH,
		XDFEPRACH_MODEL_PARAM_NUM_RACH_LANES_OFFSET, ModelParam);
	InstancePtr->Config.NumRachChannels = XDfePrach_RdBitField(
		XDFEPRACH_MODEL_PARAM_NUM_RACH_CHANNELS_WIDTH,
		XDFEPRACH_MODEL_PARAM_NUM_RACH_CHANNELS_OFFSET, ModelParam);
	InstancePtr->Config.HasAxisCtrl =
		XDfePrach_RdBitField(XDFEPRACH_MODEL_PARAM_HAS_AXIS_CTRL_WIDTH,
				     XDFEPRACH_MODEL_PARAM_HAS_AXIS_CTRL_OFFSET,
				     ModelParam);
	InstancePtr->Config.HasIrq =
		XDfePrach_RdBitField(XDFEPRACH_MODEL_PARAM_HAS_IRQ_WIDTH,
				     XDFEPRACH_MODEL_PARAM_HAS_IRQ_OFFSET,
				     ModelParam);

	/* Copy configs model parameters from devicetree config data stored in
	   InstancePtr */
	Cfg->ModelParams.NumAntenna = InstancePtr->Config.NumAntenna;
	Cfg->ModelParams.NumCCPerAntenna = InstancePtr->Config.NumCCPerAntenna;
	Cfg->ModelParams.NumAntennaChannels =
		InstancePtr->Config.NumAntennaChannels;
	Cfg->ModelParams.NumAntennaSlot = InstancePtr->Config.NumAntennaSlot;
	Cfg->ModelParams.NumRachLanes = InstancePtr->Config.NumRachLanes;
	Cfg->ModelParams.NumRachChannels = InstancePtr->Config.NumRachChannels;
	Cfg->ModelParams.HasAxisCtrl = InstancePtr->Config.HasAxisCtrl;
	Cfg->ModelParams.HasIrq = InstancePtr->Config.HasIrq;

	InstancePtr->StateId = XDFEPRACH_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE PRACH driver one time initialisation also moves the state machine to
* an Initialised state.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Init Initialisation data container.
*
****************************************************************************/
void XDfePrach_Initialize(XDfePrach *InstancePtr, XDfePrach_Init *Init)
{
	XDfePrach_Trigger RachUpdate;
	u32 Data;
	s32 Index;
	u32 Offset;
	u32 SequenceLength;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_CONFIGURED);
	Xil_AssertVoid(Init != NULL);

	/* Write "one-time" Sequence length */
	InstancePtr->NotUsedCCID = 0;
	InstancePtr->SequenceLength = Init->Sequence.Length;
	if (Init->Sequence.Length == 0U) {
		SequenceLength = 0U;
	} else {
		SequenceLength = Init->Sequence.Length - 1U;
	}
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_CC_SEQUENCE_LENGTH_NEXT,
			   SequenceLength);

	/* Set NULL sequence and ensure all CCs are disabled. Not all registers
	   will be cleared by reset as they are implemented using DRAM. This
	   step sets all RACH_CONFIGURATION.CC_MAPPING.NEXT[*].ENABLE to 0
	   ensuring the Hardblock will remain disabled following the first call
	   to XDfePrach_Activate. */
	for (Index = 0U; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		Offset = XDFEPRACH_CC_SEQUENCE_NEXT + (Index * sizeof(u32));
		XDfePrach_WriteReg(InstancePtr, Offset,
				   XDFEPRACH_SEQUENCE_ENTRY_DEFAULT);
		Init->Sequence.CCID[Index] = XDFEPRACH_SEQUENCE_ENTRY_NULL;
	}
	for (Index = 0U; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		Offset = XDFEPRACH_CC_MAPPING_NEXT + (Index * sizeof(u32));
		XDfePrach_WriteReg(InstancePtr, Offset, 0U);
	}

	/* Write EnableStaticSchedule to RCID_SCHEDULE.STATIC_SCHEDULE */
	if (Init->EnableStaticSchedule == true) {
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_RCID_SCHEDULE_STATIC_SCHEDULE,
				   XDFEPRACH_RCID_SCHEDULE_STATIC_SCHEDULE_ON);
	} else {
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_RCID_SCHEDULE_STATIC_SCHEDULE,
				   XDFEPRACH_RCID_SCHEDULE_STATIC_SCHEDULE_OFF);
	}

	/* Set USE_FREQ_OFFSET */
	if (Init->EnableUseFreqOffset == true) {
		XDfePrach_WriteReg(InstancePtr, XDFEPRACH_CORE_SETTINGS,
				   XDFEPRACH_USE_FREQ_OFFSET_ENABLE);
	} else {
		XDfePrach_WriteReg(InstancePtr, XDFEPRACH_CORE_SETTINGS,
				   XDFEPRACH_USE_FREQ_OFFSET_DISABLE);
	}

	/* Clear RACH data */
	for (Index = 0U; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		Offset = Index * sizeof(u32);
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_RCID_MAPPING_CHANNEL_NEXT + Offset,
				   0U);
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_RCID_MAPPING_SOURCE_NEXT + Offset,
				   0U);
		XDfePrach_WriteReg(
			InstancePtr,
			XDFEPRACH_RCID_SCHEDULE_LOCATION_NEXT + Offset, 0U);
		XDfePrach_WriteReg(InstancePtr,
				   XDFEPRACH_RCID_SCHEDULE_LENGTH_NEXT + Offset,
				   0U);
	}

	/* Trigger RACH_UPDATE immediately using Register source to update
	   CURRENT from NEXT */
	RachUpdate.TriggerEnable = XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_ENABLED;
	RachUpdate.Mode = XDFEPRACH_TRIGGERS_MODE_IMMEDIATE;
	RachUpdate.StateOutput = XDFEPRACH_TRIGGERS_STATE_OUTPUT_ENABLED;
	Data = XDfePrach_ReadReg(InstancePtr,
				 XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				    XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				    Data, RachUpdate.StateOutput);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				    XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				    Data, RachUpdate.TriggerEnable);
	Data = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				    XDFEPRACH_TRIGGERS_MODE_OFFSET, Data,
				    RachUpdate.Mode);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET,
			   Data);

	InstancePtr->StateId = XDFEPRACH_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activates PRACH and moves the state machine to an Activated state.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    EnableLowPower Flag indicating low power.
*
******************************************************************************/
void XDfePrach_Activate(XDfePrach *InstancePtr, bool EnableLowPower)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEPRACH_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL));

	/* Do nothing if the block already operational */
	IsOperational = XDfePrach_ReadReg(InstancePtr,
					  XDFEPRACH_STATE_OPERATIONAL_OFFSET);
	if (IsOperational == XDFEPRACH_STATE_IS_OPERATIONAL) {
		return;
	}

	/* Enable the Activate trigger and set to one-shot */
	XDfePrach_EnableActivateTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfePrach_EnableLowPowerTrigger(InstancePtr);
	}

	/* Prach is operational now, change a state */
	InstancePtr->StateId = XDFEPRACH_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* Deactivates PRACH and moves the state machine to Initialised state.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
******************************************************************************/
void XDfePrach_Deactivate(XDfePrach *InstancePtr)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEPRACH_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL));

	/* Do nothing if the block already deactivated (in Initialized
	   state) */
	IsOperational = XDfePrach_ReadReg(InstancePtr,
					  XDFEPRACH_STATE_OPERATIONAL_OFFSET);
	if (IsOperational == XDFEPRACH_STATE_NOT_OPERATIONAL) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfePrach_DisableLowPowerTrigger(InstancePtr);

	/* Enable Activate trigger (toggles state between operational
	   and intialized) */
	XDfePrach_EnableDeactivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFEPRACH_STATE_INITIALISED;
}

/****************************************************************************/
/**
*
* Gets a state machine state id.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
* @return   State machine StateID
*
****************************************************************************/
XDfePrach_StateId XDfePrach_GetStateID(XDfePrach *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return InstancePtr->StateId;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Returns the current CC configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CurrCCCfg CC configuration container.
*
****************************************************************************/
void XDfePrach_GetCurrentCCCfg(const XDfePrach *InstancePtr,
			       XDfePrach_CCCfg *CurrCCCfg)
{
	u32 SeqLen;
	u32 Index;
	u32 Data;
	u32 Offset;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CurrCCCfg != NULL);

	/* Read CCID sequence and carrier configurations */
	for (Index = 0; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		CurrCCCfg->Sequence.CCID[Index] = XDfePrach_ReadReg(
			InstancePtr,
			XDFEPRACH_CC_SEQUENCE_CURRENT + (sizeof(u32) * Index));
	}

	/* Read sequence length */
	SeqLen = XDfePrach_ReadReg(InstancePtr,
				   XDFEPRACH_CC_SEQUENCE_LENGTH_CURRENT);
	if (SeqLen == 0U) {
		CurrCCCfg->Sequence.Length = InstancePtr->SequenceLength;
	} else {
		CurrCCCfg->Sequence.Length = SeqLen + 1U;
	}

	/* Convert not used CC to -1 */
	for (Index = 0; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		if ((CurrCCCfg->Sequence.CCID[Index] ==
		     InstancePtr->NotUsedCCID) ||
		    (Index >= InstancePtr->SequenceLength)) {
			CurrCCCfg->Sequence.CCID[Index] =
				XDFEPRACH_SEQUENCE_ENTRY_NULL;
		}
	}
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		/* Read current carrier configuration */
		Offset = XDFEPRACH_CC_MAPPING_CURRENT + (Index * sizeof(u32));
		Data = XDfePrach_ReadReg(InstancePtr, Offset);
		CurrCCCfg->CarrierCfg[Index].Enable =
			XDfePrach_RdBitField(XDFEPRACH_CC_MAPPING_ENABLE_WIDTH,
					     XDFEPRACH_CC_MAPPING_ENABLE_OFFSET,
					     Data);
		CurrCCCfg->CarrierCfg[Index].SCS =
			XDfePrach_RdBitField(XDFEPRACH_CC_MAPPING_SCS_WIDTH,
					     XDFEPRACH_CC_MAPPING_SCS_OFFSET,
					     Data);
		CurrCCCfg->CarrierCfg[Index].CCRate = XDfePrach_RdBitField(
			XDFEPRACH_CC_MAPPING_DECIMATION_RATE_WIDTH,
			XDFEPRACH_CC_MAPPING_DECIMATION_RATE_OFFSET, Data);
	}
}

/****************************************************************************/
/**
*
* Returns configuration structure CCCfg with CCCfg->Sequence.Length value set
* in XDfePrach_Configure(), array CCCfg->Sequence.CCID[] members are set to not
* used value (-1) and the other CCCfg members are set to 0.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCCfg CC configuration container.
*
****************************************************************************/
void XDfePrach_GetEmptyCCCfg(const XDfePrach *InstancePtr,
			     XDfePrach_CCCfg *CCCfg)
{
	u32 Index;
	u32 SeqLen;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);

	memset(CCCfg, 0, sizeof(XDfePrach_CCCfg));

	/* Convert CC to -1 meaning not used */
	for (Index = 0U; Index < XDFEPRACH_CC_NUM_MAX; Index++) {
		CCCfg->Sequence.CCID[Index] = XDFEPRACH_SEQUENCE_ENTRY_NULL;
	}
	/* Read sequence length */
	SeqLen = XDfePrach_ReadReg(InstancePtr,
				   XDFEPRACH_CC_SEQUENCE_LENGTH_CURRENT);
	if (SeqLen == 0U) {
		CCCfg->Sequence.Length = InstancePtr->SequenceLength;
	} else {
		CCCfg->Sequence.Length = SeqLen + 1U;
	}
}

/****************************************************************************/
/**
*
* Returns the current CCID carrier configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID [0-15].
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
*
****************************************************************************/
void XDfePrach_GetCarrierCfg(const XDfePrach *InstancePtr,
			     XDfePrach_CCCfg *CCCfg, s32 CCID, u32 *CCSeqBitmap,
			     XDfePrach_CarrierCfg *CarrierCfg)
{
	u32 Index;
	u32 Mask = 1U;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(CCID <= XDFEPRACH_CC_NUM_MAX);
	Xil_AssertVoid(CCSeqBitmap != NULL);
	Xil_AssertVoid(CarrierCfg != NULL);

	CarrierCfg->SCS = CCCfg->CarrierCfg[CCID].SCS;

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
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCCfg CC configuration container.
* @param    AntennaCfg Array of all antenna configurations.
*
****************************************************************************/
void XDfePrach_SetAntennaCfgInCCCfg(const XDfePrach *InstancePtr,
				    XDfePrach_CCCfg *CCCfg, u32 *AntennaCfg)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCCfg != NULL);
	Xil_AssertVoid(AntennaCfg != NULL);

	for (Index = 0; Index < XDFEPRACH_ANT_NUM_MAX; Index++) {
		CCCfg->AntennaCfg[Index] = AntennaCfg[Index];
	}
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
* The returned CCCfg.Sequence is translated as there is no explicit indication
* that SEQUENCE[i] is not used - 0 can define the slot as either used or not
* used.
* Sequence data that is returned in the CCIDSequence is not the same as what is
* written in the registers. The translation is:
* - CCIDSequence.CCID[i] = -1    - if [i] is unused slot
* - CCIDSequence.CCID[i] = CCID  - if [i] is used slot
* - a returned CCIDSequence->Length = length in register + 1
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID [0-15].
* @param    CCSeqBitmap CC slot position container.
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfePrach_AddCCtoCCCfg(XDfePrach *InstancePtr, XDfePrach_CCCfg *CCCfg,
			   s32 CCID, u32 CCSeqBitmap,
			   const XDfePrach_CarrierCfg *CarrierCfg)
{
	u32 AddSuccess;
	u32 Rate;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCCfg != NULL);
	Xil_AssertNonvoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfePrach_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg->Sequence);
	if (AddSuccess == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Update carrier configuration, first detect rate value */
	if (XST_FAILURE ==
	    XDfePrach_FindRate(InstancePtr, CCSeqBitmap, &Rate)) {
		metal_log(METAL_LOG_ERROR, "Rate cannot be detected\n");
		return XST_FAILURE;
	}
	CCCfg->CarrierCfg[CCID].Enable = XDFEPRACH_CC_MAPPING_ENABLED;
	CCCfg->CarrierCfg[CCID].CCRate = Rate;
	CCCfg->CarrierCfg[CCID].SCS = CarrierCfg->SCS;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes specified CCID from a local CC configuration structure.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID [0-15].
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     For a sequence conversion see XDfePrach_AddCCtoCCCfg() comment.
*
****************************************************************************/
u32 XDfePrach_RemoveCCfromCCCfg(XDfePrach *InstancePtr, XDfePrach_CCCfg *CCCfg,
				s32 CCID)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCID <= XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(CCCfg != NULL);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfePrach_RemoveCCID(InstancePtr, CCID, &CCCfg->Sequence);
	CCCfg->CarrierCfg[CCID].Enable = 0U;

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
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCCfg Component carrier (CC) configuration container.
* @param    CCID CC ID [0-15].
* @param    CarrierCfg CC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfePrach_UpdateCCinCCCfg(const XDfePrach *InstancePtr,
			      XDfePrach_CCCfg *CCCfg, s32 CCID,
			      const XDfePrach_CarrierCfg *CarrierCfg)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Update carrier configuration. */
	CCCfg->CarrierCfg[CCID].SCS = CarrierCfg->SCS;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Writes local CC configuration to the shadow (NEXT) registers and triggers
* copying from shadow to operational registers.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    NextCCCfg CC configuration container.
* @param    NextRCCfg RC configuration container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
****************************************************************************/
u32 XDfePrach_SetNextCfg(const XDfePrach *InstancePtr,
			 const XDfePrach_CCCfg *NextCCCfg,
			 XDfePrach_RCCfg *NextRCCfg)
{
	u32 Index;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(NextCCCfg != NULL);
	Xil_AssertNonvoid(NextRCCfg != NULL);

	/* Set all CCCfg registers */
	XDfePrach_SetNextCCCfg(InstancePtr, NextCCCfg);

	/* Set all RCCfg registers */
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		XDfePrach_SetRC(InstancePtr, NextRCCfg, Index);
	}

	/* Now do trigger, needs to be set after xDFENRPrach_SetNextCCCfg()
	   been written. */
	if (XST_FAILURE == XDfePrach_EnableUpdateTrigger(InstancePtr)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}
	/* Enable the frame marker trigger too. */
	XDfePrach_EnableFrameMarkerTrigger(InstancePtr);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Adds specified CCID, with specified configuration.
* If there is insufficient capacity for the new CC the function will return
* an error.
* Initiates CC update (enable CCUpdate trigger TUSER Single Shot).
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC ID [0-15].
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
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_AddCC(XDfePrach *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		    const XDfePrach_CarrierCfg *CarrierCfg)
{
	u32 AddSuccess;
	XDfePrach_CCCfg CCCfg;
	XDfePrach_RCCfg RCCfg;
	u32 Rate;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);
	Xil_AssertNonvoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(CarrierCfg != NULL);

	/* Read current CC configuration. Note that XDfePrach_Initialise writes
	   a NULL CC sequence to H/W */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &RCCfg);

	/* Try to add CC to sequence and update carrier configuration */
	AddSuccess = XDfePrach_AddCCIDAndTranslateSeq(
		InstancePtr, CCID, CCSeqBitmap, &CCCfg.Sequence);
	if (AddSuccess == (u32)XST_FAILURE) {
		metal_log(METAL_LOG_ERROR, "CC not added to a sequence in %s\n",
			  __func__);
		return XST_FAILURE;
	}

	/* Update carrier configuration, first detect rate value */
	if (XST_FAILURE ==
	    XDfePrach_FindRate(InstancePtr, CCSeqBitmap, &Rate)) {
		metal_log(METAL_LOG_ERROR, "Rate cannot be detected\n");
		return XST_FAILURE;
	}
	CCCfg.CarrierCfg[CCID].Enable = XDFEPRACH_CC_MAPPING_ENABLED;
	CCCfg.CarrierCfg[CCID].CCRate = Rate;
	CCCfg.CarrierCfg[CCID].SCS = CarrierCfg->SCS;

	/* Update next configuration and trigger update. */
	if (XST_FAILURE == XDfePrach_SetNextCfg(InstancePtr, &CCCfg, &RCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes CCID from sequence.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC ID [0-15].
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_RemoveCC(XDfePrach *InstancePtr, s32 CCID)
{
	XDfePrach_CCCfg CCCfg;
	XDfePrach_RCCfg RCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Read current CC configuration. */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &RCCfg);

	/* Remove CCID from sequence and mark carrier configuration as
	   disabled */
	XDfePrach_RemoveCCID(InstancePtr, CCID, &CCCfg.Sequence);
	CCCfg.CarrierCfg[CCID].Enable = 0U;

	/* Update next configuration and trigger update. */
	if (XST_FAILURE == XDfePrach_SetNextCfg(InstancePtr, &CCCfg, &RCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Updates a CCID sequence.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC ID [0-15].
* @param    CarrierCfg Carrier data container.
*
* @return
*           - XST_SUCCESS if successful.
*           - XST_FAILURE if error occurs.
*
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_UpdateCC(const XDfePrach *InstancePtr, s32 CCID,
		       const XDfePrach_CarrierCfg *CarrierCfg)
{
	XDfePrach_CCCfg CCCfg;
	XDfePrach_RCCfg RCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(CarrierCfg != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Read current CC configuration. */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &RCCfg);

	/* Update carrier configuration. */
	CCCfg.CarrierCfg[CCID].SCS = CarrierCfg->SCS;

	/* Update next configuration and trigger update. */
	if (XST_FAILURE == XDfePrach_SetNextCfg(InstancePtr, &CCCfg, &RCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Reads all of the RC configuration back.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC configuration container.
*
****************************************************************************/
void XDfePrach_GetCurrentRCCfg(const XDfePrach *InstancePtr,
			       XDfePrach_RCCfg *RCCfg)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RCCfg != NULL);
	/* Read each RC config entry */
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		XDfePrach_GetRC(InstancePtr, false, Index, RCCfg);
	}

	/* Preserve all of the existing channels so that they are
	   not reset/flushed. */
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		XDfePrach_AddRCRestart(
			XDFEPRACH_RCID_MAPPING_CHANNEL_RESTART_NO, RCCfg,
			Index);
	}
}

/****************************************************************************/
/**
*
* Returns the empty CC configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC configuration container.
*
****************************************************************************/
void XDfePrach_GetEmptyRCCfg(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *RCCfg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RCCfg != NULL);

	memset(&RCCfg, 0, sizeof(XDfePrach_RCCfg));
}

/****************************************************************************/
/**
*
* Gets RACH channel configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCCfg RC configuration container.
* @param    RCId Chosen RACH channel Id [0-15].
* @param    ChannelCfg RACH channel container.
*
****************************************************************************/
void XDfePrach_GetChannelCfg(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *RCCfg, s32 RCId,
			     XDfePrach_ChannelCfg *ChannelCfg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RCCfg != NULL);
	Xil_AssertVoid(ChannelCfg != NULL);

	ChannelCfg->RCId = RCCfg->InternalRCCfg[RCId].RCId;
	ChannelCfg->RachChannel = RCCfg->InternalRCCfg[RCId].RachChannel;
	ChannelCfg->CCID = RCCfg->InternalRCCfg[RCId].CCID;
}

/****************************************************************************/
/**
*
* Adds a new RC entry to the RC_CONFIGURATION. RCId must be same as the
* physical channel RachChan.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CurrentRCCfg Current RACH configuration container.
* @param    CCID CC ID [0-15].
* @param    RCId RC Id [0-15].
* @param    RachChan RACH channel [0-15].
* @param    DdcCfg DDC data container.
* @param    NcoCfg NCO data container.
* @param    StaticSchedule Schedule data container.
*
* @return
*	- XST_SUCCESS on succes
*	- XST_FAILURE on failure
*
****************************************************************************/
u32 XDfePrach_AddRCtoRCCfg(const XDfePrach *InstancePtr,
			   XDfePrach_RCCfg *CurrentRCCfg, s32 CCID, u32 RCId,
			   u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			   XDfePrach_NCO *NcoCfg,
			   XDfePrach_Schedule *StaticSchedule)
{
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CurrentRCCfg != NULL);
	Xil_AssertNonvoid(CurrentRCCfg->InternalRCCfg[RCId].CCID <
			  XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(CurrentRCCfg->InternalRCCfg[RCId].RCId <
			  XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(RachChan < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(DdcCfg != NULL);
	Xil_AssertNonvoid(NcoCfg != NULL);
	Xil_AssertNonvoid(StaticSchedule != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Check  RachChan" is not in use. */
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		if ((CurrentRCCfg->InternalRCCfg[Index].Enable ==
		     XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLED) &&
		    (CurrentRCCfg->InternalRCCfg[Index].RachChannel ==
		     RachChan) &&
		    (Index != RCId)) {
			metal_log(METAL_LOG_ERROR, "RC failure, %s\n",
				  __func__);
			/* Error: a different RCID is using this RachChan. */
			return XST_FAILURE;
		}
	}

	/* Load the new channel's data into the RCID configuration, will be
	   marked as needing a restart. */
	XDfePrach_AddRC(RCId, RachChan, CCID, CurrentRCCfg, DdcCfg, NcoCfg,
			StaticSchedule);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes an RC configuration entry from the RC_CONFIGURATION. RCId must be
* same as the physical channel RachChan.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CurrentRCCfg Current RACH configuration container.
* @param    RCId RC Id [0-15].
*
* @return
*	- XST_SUCCESS on succes
*	- XST_FAILURE on failure
*
****************************************************************************/
u32 XDfePrach_RemoveRCfromRCCfg(const XDfePrach *InstancePtr,
				XDfePrach_RCCfg *CurrentRCCfg, u32 RCId)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Remove the channel's data into the RCID configuration. */
	XDfePrach_RemoveOneRC(&CurrentRCCfg->InternalRCCfg[RCId]);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Updates an RC entry to the RC_CONFIGURATION.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CurrentRCCfg Current RACH configuration container.
* @param    CCID CC Id [0-15].
* @param    RCId RC Id [0-15].
* @param    RachChan RACH channel [0-15].
* @param    DdcCfg DDC data container.
* @param    NcoCfg NCO data container.
* @param    StaticSchedule Schedule data container.
*
****************************************************************************/
void XDfePrach_UpdateRCinRCCfg(const XDfePrach *InstancePtr,
			       XDfePrach_RCCfg *CurrentRCCfg, s32 CCID,
			       u32 RCId, u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			       XDfePrach_NCO *NcoCfg,
			       XDfePrach_Schedule *StaticSchedule)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CurrentRCCfg != NULL);
	Xil_AssertVoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertVoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertVoid(RachChan < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertVoid(DdcCfg != NULL);
	Xil_AssertVoid(NcoCfg != NULL);
	Xil_AssertVoid(StaticSchedule != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Load the new channel's data into the RCID configuration, will be
	   marked as needing a restart. */
	XDfePrach_AddRC(RCId, RachChan, CCID, CurrentRCCfg, DdcCfg, NcoCfg,
			StaticSchedule);
}

/****************************************************************************/
/**
*
* Adds a new RC entry to the RC_CONFIGURATION. RCId must be same as the
* physical channel RachChan.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC Id [0-15].
* @param    RCId RC Id [0-15].
* @param    RachChan RACH channel [0-15].
* @param    DdcCfg DDC data container.
* @param    NcoCfg NCO data container.
* @param    StaticSchedule Schedule data container.
*
* @return
*	- XST_SUCCESS on succes
*	- XST_FAILURE on failure
*
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_AddRCCfg(const XDfePrach *InstancePtr, s32 CCID, u32 RCId,
		       u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
		       XDfePrach_NCO *NcoCfg,
		       XDfePrach_Schedule *StaticSchedule)
{
	u32 Index;
	XDfePrach_CCCfg CurrentCCCfg;
	XDfePrach_RCCfg CurrentRCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(RachChan < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(DdcCfg != NULL);
	Xil_AssertNonvoid(NcoCfg != NULL);
	Xil_AssertNonvoid(StaticSchedule != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Read current CC configuration. */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CurrentCCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &CurrentRCCfg);

	/* Check  RachChan" is not in use. */
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		if ((CurrentRCCfg.InternalRCCfg[Index].Enable ==
		     XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLED) &&
		    (CurrentRCCfg.InternalRCCfg[Index].RachChannel ==
		     RachChan) &&
		    (Index != RCId)) {
			metal_log(METAL_LOG_ERROR, "RC failure, %s\n",
				  __func__);
			/* Error: a different RCID is using this RachChan. */
			return XST_FAILURE;
		}
	}

	/* Load the new channel's data into the RCID configuration, will be
	   marked as needing a restart. */
	XDfePrach_AddRC(RCId, RachChan, CCID, &CurrentRCCfg, DdcCfg, NcoCfg,
			StaticSchedule);

	/* Update next configuration and trigger update. */
	if (XST_FAILURE ==
	    XDfePrach_SetNextCfg(InstancePtr, &CurrentCCCfg, &CurrentRCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Removes an RC configuration entry from the RC_CONFIGURATION.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCId RC Id [0-15].
*
* @return
*	- XST_SUCCESS on succes
*	- XST_FAILURE on failure
*
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_RemoveRC(const XDfePrach *InstancePtr, u32 RCId)
{
	XDfePrach_CCCfg CurrentCCCfg;
	XDfePrach_RCCfg CurrentRCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Read current CC configuration. */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CurrentCCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &CurrentRCCfg);

	/* Remove the channel's data into the RCID configuration. */
	XDfePrach_RemoveOneRC(&CurrentRCCfg.InternalRCCfg[RCId]);

	/* Update next configuration and trigger update. */
	if (XST_FAILURE ==
	    XDfePrach_SetNextCfg(InstancePtr, &CurrentCCCfg, &CurrentRCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Updates an RC entry to the RC_CONFIGURATION. RCId must be same as the
* physical channel RachChan.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    CCID CC Id [0-15].
* @param    RCId RC Id [0-15].
* @param    RachChan RACH channel [0-15].
* @param    DdcCfg DDC data container.
* @param    NcoCfg NCO data container.
* @param    StaticSchedule Schedule data container.
*
* @return
*	- XST_SUCCESS on succes
*	- XST_FAILURE on failure
*
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_UpdateRCCfg(const XDfePrach *InstancePtr, s32 CCID, u32 RCId,
			  u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			  XDfePrach_NCO *NcoCfg,
			  XDfePrach_Schedule *StaticSchedule)
{
	XDfePrach_CCCfg CurrentCCCfg;
	XDfePrach_RCCfg CurrentRCCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertNonvoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(RachChan < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(DdcCfg != NULL);
	Xil_AssertNonvoid(NcoCfg != NULL);
	Xil_AssertNonvoid(StaticSchedule != NULL);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Read current CC configuration. */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CurrentCCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &CurrentRCCfg);

	/* Load the new channel's data into the RCID configuration, will be
	   marked as needing a restart. */
	XDfePrach_AddRC(RCId, RachChan, CCID, &CurrentRCCfg, DdcCfg, NcoCfg,
			StaticSchedule);

	/* Update next configuration and trigger update. */
	if (XST_FAILURE ==
	    XDfePrach_SetNextCfg(InstancePtr, &CurrentCCCfg, &CurrentRCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Moves the specified RCID from one NCO & Decimation Channel to another NCO
* Decimation Channel.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RCId RC Id [0-15].
* @param    ToChannel Destination channel Id [0-15].
*
* @return
*	- XST_SUCCESS on succes
*	- XST_FAILURE on failure
*
* @note     Clear event status with XDfePrach_ClearEventStatus() before
*           running this API.
*
****************************************************************************/
u32 XDfePrach_MoveRC(const XDfePrach *InstancePtr, u32 RCId, u32 ToChannel)
{
	u32 Index;
	XDfePrach_CCCfg CurrentCCCfg;
	XDfePrach_RCCfg CurrentRCCfg;
	XDfePrach_NCO NCOCfgOld;
	XDfePrach_DDCCfg DDCCfgOld;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(RCId < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(ToChannel < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertNonvoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Read current CC configuration. */
	XDfePrach_GetCurrentCCCfg(InstancePtr, &CurrentCCCfg);
	/* Read current RC configuration. */
	XDfePrach_GetCurrentRCCfg(InstancePtr, &CurrentRCCfg);

	/* Check  "ToChannel" is not in use. */
	for (Index = 0; Index < XDFEPRACH_RC_NUM_MAX; Index++) {
		if ((CurrentRCCfg.InternalRCCfg[Index].Enable ==
		     XDFEPRACH_RCID_MAPPING_CHANNEL_ENABLED) &&
		    (CurrentRCCfg.InternalRCCfg[Index].RachChannel ==
		     ToChannel)) {
			metal_log(METAL_LOG_ERROR, "RC failure, %s\n",
				  __func__);
			/* Error we are trying to load a running channel,
			   remove it first. */
			return XST_FAILURE;
		}
	}

	/* Phase transfer is transferred internally by the Core. There is no
	   external intervention required beyond initiating the transfer. */
	/* Load the old physical config onto the new physical config. */
	/* First, copy the old NCO and DDC CFG. */
	NCOCfgOld = CurrentRCCfg.NcoCfg[RCId];
	DDCCfgOld = CurrentRCCfg.DdcCfg[RCId];

	/* Set the physical channel to the new destination - changing this
	   changes the destination address for NCO and DDC, making two channels
	   with the same config after they are added. */
	XDfePrach_AddRachChannel(ToChannel, &CurrentRCCfg.InternalRCCfg[RCId]);

	/* Load the NCO and DDC onto the new physical channel. */
	XDfePrach_AddNCO(&CurrentRCCfg, &NCOCfgOld, RCId);
	XDfePrach_AddDDC(&CurrentRCCfg, &DDCCfgOld, RCId);

	/* All other fields remain the same - move is recognised by the change
	   in RACH_CHANNNEL and the RESTART==0 */

	/* Update next configuration and trigger update. */
	if (XST_FAILURE ==
	    XDfePrach_SetNextCfg(InstancePtr, &CurrentCCCfg, &CurrentRCCfg)) {
		metal_log(METAL_LOG_ERROR, "Trigger failure, %s\n", __func__);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Returns the current trigger configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfePrach_GetTriggersCfg(const XDfePrach *InstancePtr,
			      XDfePrach_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEPRACH_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read ACTIVATE triggers */
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET);
	TriggerCfg->Activate.TriggerEnable =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				     XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				     Val);
	TriggerCfg->Activate.Mode =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				     XDFEPRACH_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.TuserEdgeLevel =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				     Val);
	TriggerCfg->Activate.StateOutput =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				     XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				     Val);

	/* Read LOW_POWER triggers */
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET);
	TriggerCfg->LowPower.TriggerEnable =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				     XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				     Val);
	TriggerCfg->LowPower.Mode =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				     XDFEPRACH_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.TuserEdgeLevel =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				     Val);
	TriggerCfg->LowPower.StateOutput =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				     XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				     Val);

	/* Read RACH_UPDATE triggers */
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET);
	TriggerCfg->RachUpdate.TriggerEnable =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				     XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				     Val);
	TriggerCfg->RachUpdate.Mode =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				     XDFEPRACH_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->RachUpdate.TUSERBit =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->RachUpdate.TuserEdgeLevel =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				     Val);
	TriggerCfg->RachUpdate.StateOutput =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				     XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				     Val);

	/* Read FRAME_INIT triggers */
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_FRAME_INIT_OFFSET);
	TriggerCfg->FrameInit.TriggerEnable =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				     XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				     Val);
	TriggerCfg->FrameInit.Mode =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				     XDFEPRACH_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->FrameInit.TUSERBit =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->FrameInit.TuserEdgeLevel =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				     XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				     Val);
	TriggerCfg->FrameInit.StateOutput =
		XDfePrach_RdBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				     XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET,
				     Val);
}

/****************************************************************************/
/**
*
* Sets trigger configuration.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    TriggerCfg Trigger configuration container.
*
****************************************************************************/
void XDfePrach_SetTriggersCfg(const XDfePrach *InstancePtr,
			      XDfePrach_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);
	Xil_AssertVoid(TriggerCfg->RachUpdate.Mode !=
		       XDFEPRACH_TRIGGERS_MODE_TUSER_CONTINUOUS);
	Xil_AssertVoid(TriggerCfg->FrameInit.Mode !=
		       XDFEPRACH_TRIGGERS_MODE_IMMEDIATE);

	/* Write public trigger configuration members and ensure private members
	  (TriggerEnable & Immediate) are set appropriately */

	/* Activate defined as Single Shot/Immediate (as per the programming model) */
	TriggerCfg->Activate.TriggerEnable =
		XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->Activate.StateOutput =
		XDFEPRACH_TRIGGERS_STATE_OUTPUT_ENABLED;
	/* Read/set/write ACTIVATE triggers */
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Val, TriggerCfg->Activate.TriggerEnable);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				   XDFEPRACH_TRIGGERS_MODE_OFFSET, Val,
				   TriggerCfg->Activate.Mode);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val, TriggerCfg->Activate.TuserEdgeLevel);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val,
				   TriggerCfg->Activate.TUSERBit);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				   TriggerCfg->Activate.StateOutput);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_ACTIVATE_OFFSET,
			   Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.TriggerEnable =
		XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	TriggerCfg->LowPower.Mode = XDFEPRACH_TRIGGERS_MODE_TUSER_CONTINUOUS;
	/* Read/set/write LOW_POWER triggers */
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Val, TriggerCfg->LowPower.TriggerEnable);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				   XDFEPRACH_TRIGGERS_MODE_OFFSET, Val,
				   TriggerCfg->LowPower.Mode);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val, TriggerCfg->LowPower.TuserEdgeLevel);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val,
				   TriggerCfg->LowPower.TUSERBit);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				   TriggerCfg->LowPower.StateOutput);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_LOW_POWER_OFFSET,
			   Val);

	/* RachUpdate defined as OneShot or Immediate */
	TriggerCfg->RachUpdate.TriggerEnable =
		XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Val, TriggerCfg->RachUpdate.TriggerEnable);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				   XDFEPRACH_TRIGGERS_MODE_OFFSET, Val,
				   TriggerCfg->RachUpdate.Mode);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val, TriggerCfg->RachUpdate.TuserEdgeLevel);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val,
				   TriggerCfg->RachUpdate.TUSERBit);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				   TriggerCfg->RachUpdate.StateOutput);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_RACH_UPDATE_OFFSET,
			   Val);

	/* Frame_Init defined as OneShot and Continuous */
	TriggerCfg->FrameInit.TriggerEnable =
		XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	Val = XDfePrach_ReadReg(InstancePtr,
				XDFEPRACH_TRIGGERS_FRAME_INIT_OFFSET);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEPRACH_TRIGGERS_TRIGGER_ENABLE_OFFSET,
				   Val, TriggerCfg->FrameInit.TriggerEnable);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_MODE_WIDTH,
				   XDFEPRACH_TRIGGERS_MODE_OFFSET, Val,
				   TriggerCfg->FrameInit.Mode);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val, TriggerCfg->FrameInit.TuserEdgeLevel);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEPRACH_TRIGGERS_TUSER_BIT_OFFSET, Val,
				   TriggerCfg->FrameInit.TUSERBit);
	Val = XDfePrach_WrBitField(XDFEPRACH_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEPRACH_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				   TriggerCfg->FrameInit.StateOutput);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_TRIGGERS_FRAME_INIT_OFFSET,
			   Val);
}

/****************************************************************************/
/**
*
* Gets the specified CCID carrier configuration from either Current or Next.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Next Boolean flag indicating NEXT or CURRENT register.
* @param    CCID CC Id [0-15].
* @param    CarrierCfg Carrier configuration container.
*
*
****************************************************************************/
void XDfePrach_GetCC(const XDfePrach *InstancePtr, bool Next, s32 CCID,
		     XDfePrach_CarrierCfg *CarrierCfg)
{
	u32 Offset;
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CCID < XDFEPRACH_CC_NUM_MAX);
	Xil_AssertVoid(CarrierCfg != NULL);

	/* Read specified next CCID carrier configuration */
	if (Next == true) {
		Offset = XDFEPRACH_CC_MAPPING_NEXT + (CCID * sizeof(u32));
	} else {
		Offset = XDFEPRACH_CC_MAPPING_CURRENT + (CCID * sizeof(u32));
	}

	Data = XDfePrach_ReadReg(InstancePtr, Offset);
	CarrierCfg->SCS =
		XDfePrach_RdBitField(XDFEPRACH_CC_MAPPING_SCS_WIDTH,
				     XDFEPRACH_CC_MAPPING_SCS_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Gets the PRACH Status.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Status Status data container.
*
*
****************************************************************************/
void XDfePrach_GetStatus(const XDfePrach *InstancePtr, XDfePrach_Status *Status)
{
	u32 Offset;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Mixer Overflow */
	Offset = XDFEPRACH_ISR;
	Status->MixerOverflow.MixerOverflow =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_MIXER_OVERFLOW_WIDTH,
					XDFEPRACH_MIXER_OVERFLOW_OFFSET);
	Offset = XDFEPRACH_STATUS_MIXER_OVERFLOW;
	Status->MixerOverflow.FirstAntennaOverflowing =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_ANTENNA_WIDTH,
					XDFEPRACH_STATUS_ANTENNA_OFFSET);
	Status->MixerOverflow.FirstRCIdOverflowing =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_CHANNEL_WIDTH,
					XDFEPRACH_STATUS_CHANNEL_OFFSET);

	/* Decimation Overflow */
	Offset = XDFEPRACH_ISR;
	Status->DecimatorOverflow.DecimatorOverflow =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
					XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET);
	Offset = XDFEPRACH_STATUS_DECIMATOR_OVERFLOW;
	Status->DecimatorOverflow.FirstAntennaOverflowing =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_ANTENNA_WIDTH,
					XDFEPRACH_STATUS_ANTENNA_OFFSET);
	Status->DecimatorOverflow.FirstRCIdOverflowing =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_CHANNEL_WIDTH,
					XDFEPRACH_STATUS_CHANNEL_OFFSET);

	/* Mixer Overrun */
	Offset = XDFEPRACH_ISR;
	Status->MixerOverrun.MixerOverrun =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
					XDFEPRACH_SELECTOR_OVERRUN_OFFSET);
	Offset = XDFEPRACH_STATUS_SELECTOR_OVERRUN;
	Status->MixerOverrun.FirstAntennaOverruning =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_ANTENNA_WIDTH,
					XDFEPRACH_STATUS_ANTENNA_OFFSET);
	Status->MixerOverrun.FirstRCIdOverruning =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_CHANNEL_WIDTH,
					XDFEPRACH_STATUS_CHANNEL_OFFSET);

	/* Decimation Overrun */
	Offset = XDFEPRACH_ISR;
	Status->DecimatorOverrun.DecimatorOverrun =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
					XDFEPRACH_DECIMATOR_OVERRUN_OFFSET);
	Offset = XDFEPRACH_STATUS_DECIMATOR_OVERRUN;
	Status->DecimatorOverrun.FirstAntennaOverruning =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_ANTENNA_WIDTH,
					XDFEPRACH_STATUS_ANTENNA_OFFSET);
	Status->DecimatorOverrun.FirstRCIdOverruning =
		XDfePrach_RdRegBitField(InstancePtr, Offset,
					XDFEPRACH_STATUS_CHANNEL_WIDTH,
					XDFEPRACH_STATUS_CHANNEL_OFFSET);
}

/****************************************************************************/
/**
*
* Clears the PRACH status registers.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
*
****************************************************************************/
void XDfePrach_ClearStatus(const XDfePrach *InstancePtr)
{
	u32 Offset;
	XDfePrach_InterruptMask Flags = { 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U };
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_OPERATIONAL);

	/* Clear IRQ statuses */
	XDfePrach_ClearEventStatus(InstancePtr, &Flags);

	/* Clear RACH_MIXER.STATUS */
	Offset = XDFEPRACH_STATUS_DECIMATOR_OVERFLOW;
	XDfePrach_WriteReg(InstancePtr, Offset, 0U);
	Offset = XDFEPRACH_STATUS_DECIMATOR_OVERRUN;
	XDfePrach_WriteReg(InstancePtr, Offset, 0U);
	Offset = XDFEPRACH_STATUS_MIXER_OVERFLOW;
	XDfePrach_WriteReg(InstancePtr, Offset, 0U);
	Offset = XDFEPRACH_STATUS_SELECTOR_OVERRUN;
	XDfePrach_WriteReg(InstancePtr, Offset, 0U);
}

/****************************************************************************/
/**
*
* Captures phase for all phase accumulators in associated AXI-lite registers.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
*
****************************************************************************/
void XDfePrach_CapturePhase(const XDfePrach *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_RACH_MIXER_PHASE_CAPTURE, 1U);
}

/****************************************************************************/
/**
*
* Reads the captured phase for a given Rach Channel.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    RachChan RACH channel Id [0-15].
* @param    CapturedPhase NCO data container.
*
*
****************************************************************************/
void XDfePrach_GetCapturePhase(const XDfePrach *InstancePtr, u32 RachChan,
			       XDfePrach_NCO *CapturedPhase)
{
	u32 Offset;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RachChan < XDFEPRACH_RC_NUM_MAX);
	Xil_AssertVoid(CapturedPhase != NULL);

	Offset = XDFEPRACH_CAPTURED_PHASE_PHASE_ACC +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	CapturedPhase->PhaseAcc = XDfePrach_ReadReg(InstancePtr, Offset);

	Offset = XDFEPRACH_CAPTURED_PHASE_DUAL_MOD_COUNT +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	CapturedPhase->DualModCount = XDfePrach_ReadReg(InstancePtr, Offset);

	Offset = XDFEPRACH_CAPTURED_PHASE_DUAL_MOD_SEL +
		 (RachChan * XDFEPRACH_NCO_CTRL_ADDR_STEP);
	CapturedPhase->DualModSel = XDfePrach_RdRegBitField(
		InstancePtr, Offset,
		XDFEPRACH_CAPTURED_PHASE_DUAL_MOD_SEL_WIDTH,
		XDFEPRACH_CAPTURED_PHASE_DUAL_MOD_SEL_OFFSET);
}

/****************************************************************************/
/**
*
* Sets the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Delay Requested delay variable.
*
****************************************************************************/
void XDfePrach_SetTUserDelay(const XDfePrach *InstancePtr, u32 Delay)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEPRACH_STATE_INITIALISED);
	Xil_AssertVoid(Delay < (1U << XDFEPRACH_DELAY_VALUE_WIDTH));

	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_DELAY_OFFSET, Delay);
}

/****************************************************************************/
/**
*
* Reads the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr Pointer to the PRACH instance.
*
* @return   Delay value
*
****************************************************************************/
u32 XDfePrach_GetTUserDelay(const XDfePrach *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfePrach_RdRegBitField(InstancePtr, XDFEPRACH_DELAY_OFFSET,
				       XDFEPRACH_DELAY_VALUE_WIDTH,
				       XDFEPRACH_DELAY_VALUE_OFFSET);
}

/****************************************************************************/
/**
*
* Returns data latency.
*
* @param    InstancePtr Pointer to the PRACH instance.
*
* @return   Data latency value.
*
****************************************************************************/
u32 XDfePrach_GetTDataDelay(const XDfePrach *InstancePtr)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Data = XDfePrach_RdRegBitField(InstancePtr, XDFEPRACH_LATENCY_OFFSET,
				       XDFEPRACH_LATENCY_VALUE_WIDTH,
				       XDFEPRACH_LATENCY_VALUE_OFFSET);
	return Data;
}

/*****************************************************************************/
/**
*
* This API gets the driver and HW design version.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    SwVersion Driver version number.
* @param    HwVersion HW version number.
*
*
******************************************************************************/
void XDfePrach_GetVersions(const XDfePrach *InstancePtr,
			   XDfePrach_Version *SwVersion,
			   XDfePrach_Version *HwVersion)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr->StateId != XDFEPRACH_STATE_NOT_READY);

	/* Driver version */
	SwVersion->Major = XDFEPRACH_DRIVER_VERSION_MAJOR;
	SwVersion->Minor = XDFEPRACH_DRIVER_VERSION_MINOR;

	/* Component HW version */
	Version = XDfePrach_ReadReg(InstancePtr, XDFEPRACH_VERSION_OFFSET);
	HwVersion->Patch =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_PATCH_WIDTH,
				     XDFEPRACH_VERSION_PATCH_OFFSET, Version);
	HwVersion->Revision =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_REVISION_WIDTH,
				     XDFEPRACH_VERSION_REVISION_OFFSET,
				     Version);
	HwVersion->Minor =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_MINOR_WIDTH,
				     XDFEPRACH_VERSION_MINOR_OFFSET, Version);
	HwVersion->Major =
		XDfePrach_RdBitField(XDFEPRACH_VERSION_MAJOR_WIDTH,
				     XDFEPRACH_VERSION_MAJOR_OFFSET, Version);
}
/** @} */
