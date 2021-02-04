/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ.c
* @addtogroup xdfeequ_v1_0
* @{
*
* Contains the APIs for DFE Equlizer Filter component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
* </pre>
*
******************************************************************************/

#include "xdfeequ.h"
#include "xdfeequ_hw.h"
#include <math.h>
#include <metal/io.h>
#include <metal/device.h>

#ifdef __BAREMETAL__
#include "sleep.h"
#else
#include <unistd.h>
#endif

/**************************** Macros Definitions ****************************/
#define XDFEEQU_SEQUENCE_ENTRY_NULL 8U /**< Null sequence entry flag */
#define XDFEEQU_CEILING(x, y) (((x) + (y)-1U) / (y)) /**< U32 ceiling */
#define XDFEEQU_NO_EMPTY_CCID_FLAG 0xFFFFU /**< Not Empty CCID flag */
#define XDFEEQU_TIMEOUT 1000 /**< Units of us declared in XDFEEQU_WAIT */
#define XDFEEQU_WAIT 10 /**< Units of us */

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFEEQU_MAX_NUM_INSTANCES];
#endif
static struct metal_device *DevicePtrStorage[XDFEEQU_MAX_NUM_INSTANCES];
extern XDfeEqu XDfeEqu_Equalizer[XDFEEQU_MAX_NUM_INSTANCES];

/************************** Function Definitions ****************************/
extern u32 XDfeEqu_RegisterMetal(u16 DeviceId, struct metal_device **DevicePtr, const char *DeviceNodeName);
extern u32 XDfeEqu_LookupConfig(u16 DeviceId);
extern u32 XDfeEqu_CfgInitialize(XDfeEqu *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Write value to register in a Equalizer instance.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    AddrOffset is address offset relative to instance base address.
* @param    Data is value to be written.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_WriteReg(const XDfeEqu *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Read a value from register from a Equalizer instance.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    AddrOffset is address offset relativ to instance base address.
*
* @return   Register value.
*
* @note     None
*
****************************************************************************/
u32 XDfeEqu_ReadReg(const XDfeEqu *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, AddrOffset);
}

/****************************************************************************/
/**
*
* Write a bit field value to register.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
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
void XDfeEqu_WrRegBitField(const XDfeEqu *InstancePtr, u32 Offset,
			   u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfeEqu_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Read a bit field value from register.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
*
* @return   Bit field data.
*
* @note     None
*
****************************************************************************/
u32 XDfeEqu_RdRegBitField(const XDfeEqu *InstancePtr, u32 Offset,
			  u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
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
u32 XDfeEqu_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
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
u32 XDfeEqu_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
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
static void XDfeEqu_CreateCCSequence(u32 SeqLen,
				     XDfeEqu_CCSequence *CCIDSequence)
{
	u32 Index;
	Xil_AssertVoid(SeqLen < XDFEEQU_SEQ_LENGTH_MAX);
	Xil_AssertVoid(CCIDSequence != NULL);

	/* Set sequence length and mark all sequence entries as null (8) */
	CCIDSequence->Length = SeqLen;
	for (Index = 0; Index < XDFEEQU_SEQ_LENGTH_MAX; Index++) {
		CCIDSequence->CCID[Index] = XDFEEQU_SEQUENCE_ENTRY_NULL;
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
static u32 XDfeEqu_AddCCID(u32 CCID, u32 Rate, XDfeEqu_CCSequence *CCIDSequence)
{
	u32 Index;
	u32 SeqLen;
	u32 SeqMult = 1;
	u32 CCIDCount;
	u32 EmptyCCID = XDFEEQU_NO_EMPTY_CCID_FLAG;
	Xil_AssertNonvoid(CCIDSequence != NULL);

	if (0 == CCIDSequence->Length) {
		XDfeEqu_CreateCCSequence(Rate, CCIDSequence);
	}
	/* Determine if there is space in the sequence for the new CCID,
	   test for an empty slot. */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		/* search for the first "null" */
		if (CCIDSequence->CCID[Index] == XDFEEQU_SEQUENCE_ENTRY_NULL) {
			EmptyCCID = Index;
			break;
		}
	}

	if (EmptyCCID == XDFEEQU_NO_EMPTY_CCID_FLAG) {
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
		if ((SeqMult * SeqLen) > XDFEEQU_SEQ_LENGTH_MAX) {
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
		if (CCIDSequence->CCID[Index] == XDFEEQU_SEQUENCE_ENTRY_NULL) {
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
		if (CCIDSequence->CCID[Index] == XDFEEQU_SEQUENCE_ENTRY_NULL) {
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
static void XDfeEqu_RemoveCCID(u32 CCID, XDfeEqu_CCSequence *CCIDSequence)
{
	u32 Index;
	Xil_AssertVoid(CCIDSequence != NULL);
	Xil_AssertVoid(CCIDSequence->Length <= XDFEEQU_SEQ_LENGTH_MAX);

	/* Replace each CCID entry with null (8) */
	for (Index = 0; Index < CCIDSequence->Length; Index++) {
		if (CCIDSequence->CCID[Index] == CCID) {
			CCIDSequence->CCID[Index] = XDFEEQU_SEQUENCE_ENTRY_NULL;
		}
	}
}

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Fire a trigger.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerSource is a trigger data container.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeEqu_PressTrigger(XDfeEqu *InstancePtr,
			  const XDfeEqu_Trigger *TriggerSource)
{
	/* Set the Next Control Trigger Source register (0x28).
	   When it is set to TUSER the Trigger_Source.TUSERBit field specifies
	   which bit of TUSER to trigger on (0 to 63) */
	if (TriggerSource->Source == XDFEEQU_TRIGGER_SOURCE_TUSER) {
		XDfeEqu_WrRegBitField(InstancePtr,
				      XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET,
				      XDFEEQU_TUSER_BIT_WIDTH,
				      XDFEEQU_TUSER_BIT_OFFSET,
				      TriggerSource->TUSERBit);
	}

	/* When the Trigger_Source is set to TLAST TUSER the polarity of the
	   trigger source signal is set by the Trigger_Source.Polarity field */
	if ((TriggerSource->Source == XDFEEQU_TRIGGER_SOURCE_TUSER) ||
	    (TriggerSource->Source == XDFEEQU_TRIGGER_SOURCE_TLAST)) {
		XDfeEqu_WrRegBitField(InstancePtr,
				      XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET,
				      XDFEEQU_SIGNAL_INVERSION_WIDTH,
				      XDFEEQU_SIGNAL_INVERSION_OFFSET,
				      TriggerSource->Polarity);
	}
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API Initialise one instancies of a Equalizer driver.
* Traverse "/sys/bus/platform/device" directory, to find Equalizer device id,
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
XDfeEqu *XDfeEqu_InstanceInit(u16 DeviceId, const char *DeviceNodeName)
{
	static u32 XDfeEqu_DriverHasBeenRegisteredOnce = 0U;
	u32 Index;

	Xil_AssertNonvoid(DeviceId < XDFEEQU_MAX_NUM_INSTANCES);

	/* Is for the First initialisation caled ever */
	if (0U == XDfeEqu_DriverHasBeenRegisteredOnce) {
		/* Set up environment environment */
		for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
			XDfeEqu_Equalizer[Index].StateId =
				XDFEEQU_STATE_NOT_READY;
#ifdef __BAREMETAL__
			DevicePtrStorage[Index] = &CustomDevice[Index];
#endif
		}
		XDfeEqu_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check is the instance DeviceID already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	if (XDfeEqu_Equalizer[DeviceId].StateId != XDFEEQU_STATE_NOT_READY) {
		return &XDfeEqu_Equalizer[DeviceId];
	}

	/* Register libmetal for this OS process */
	if (XST_SUCCESS !=
	    XDfeEqu_RegisterMetal(DeviceId, &DevicePtrStorage[DeviceId], DeviceNodeName)) {
		XDfeEqu_Equalizer[DeviceId].StateId = XDFEEQU_STATE_NOT_READY;
		return NULL;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeEqu_LookupConfig(DeviceId)) {
		XDfeEqu_Equalizer[DeviceId].StateId = XDFEEQU_STATE_NOT_READY;
		return NULL;
	}

	/* Configure HW and the driver instance */
	XDfeEqu_CfgInitialize(&XDfeEqu_Equalizer[DeviceId]);

	XDfeEqu_Equalizer[DeviceId].StateId = XDFEEQU_STATE_READY;

	return &XDfeEqu_Equalizer[DeviceId];
}

/*****************************************************************************/
/**
*
* API Close the instancies of a Equalizer driver.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XDfeEqu_InstanceClose(XDfeEqu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Close the instance */
	InstancePtr->StateId = XDFEEQU_STATE_NOT_READY;

	/* Release libmetal */
	metal_device_close(InstancePtr->Device);

	return;
}

/****************************************************************************/
/**
*
* This function puts block into a reset state.
* Sets bit 0 of the Master Reset register (0x4) high.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_Reset(XDfeEqu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEEQU_STATE_NOT_READY);

	/* Put Equalizer in reset */
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_RESET_OFFSET, XDFEEQU_RESET_ON);
	InstancePtr->StateId = XDFEEQU_STATE_RESET;
}

/****************************************************************************/
/**
*
* Read configuration from device tree/xparameters.h and IP registers.
* S/W reset removed.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Cfg is a pointer to the device config structure.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_Configure(XDfeEqu *InstancePtr, XDfeEqu_Cfg *Cfg)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_RESET);

	/* Read vearsion */
	Version = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_VERSION_OFFSET);
	Cfg->Version.Patch =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_PATCH_WIDTH,
				   XDFEEQU_VERSION_PATCH_OFFSET, Version);
	Cfg->Version.Revision =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_REVISION_WIDTH,
				   XDFEEQU_VERSION_REVISION_OFFSET, Version);
	Cfg->Version.Minor =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_MINOR_WIDTH,
				   XDFEEQU_VERSION_MINOR_OFFSET, Version);
	Cfg->Version.Major =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_MAJOR_WIDTH,
				   XDFEEQU_VERSION_MAJOR_OFFSET, Version);

	/* Copy configs model parameters from InstancePtr */
	Cfg->ModelParams.DataIWidth = InstancePtr->Config.DataIWidth;
	Cfg->ModelParams.DataOWidth = InstancePtr->Config.DataOWidth;
	Cfg->ModelParams.NumChannels = InstancePtr->Config.NumChannels;
	Cfg->ModelParams.TuserWidth = InstancePtr->Config.TuserWidth;

	/* Release RESET */
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_RESET_OFFSET, XDFEEQU_RESET_OFF);
	InstancePtr->StateId = XDFEEQU_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* The software sets bit 0 of the Next Control register (0x24) low.
* The software sets bit 2 of the Next Control register (0x24). This is set
* high if Config.DatapathMode is set to complex or matrix and low if it is
* set to real.
* The software then sets the Next Control Trigger Source register (0x28) to
* IMMEDIATE.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Config is a configuration data container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_Initialize(XDfeEqu *InstancePtr, const XDfeEqu_EqConfig *Config)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_CONFIGURED);
	Xil_AssertVoid(Config != NULL);
	Xil_AssertVoid(Config->DatapathMode <= XDFEEQU_DATAPATH_MODE_MATRIX);

	Data = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET);

	/* Set bit 0 of the Next Control register (0x24) low. */
	Data = XDfeEqu_WrBitField(XDFEEQU_POWERDOWN_MODE_WIDTH,
				  XDFEEQU_POWERDOWN_MODE_OFFSET, Data,
				  XDFEEQU_POWER_UP);

	/* The software sets bit 2 of the Next Control register to:
	   - high if Config.DatapathMode is set to complex or matrix
	   - low if Config.DatapathMode is set to real. */
	if (Config->DatapathMode == XDFEEQU_DATAPATH_MODE_REAL) {
		Data = XDfeEqu_WrBitField(XDFEEQU_COMPLEX_MODE_WIDTH,
					  XDFEEQU_COMPLEX_MODE_OFFSET, Data,
					  XDFEEQU_REAL_MODE);
	} else {
		Data = XDfeEqu_WrBitField(XDFEEQU_COMPLEX_MODE_WIDTH,
					  XDFEEQU_COMPLEX_MODE_OFFSET, Data,
					  XDFEEQU_COMPLEX_MODE);
	}
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET, Data);

	/* Set the Next Control Trigger Source register (0x28) to IMMEDIATE. */
	XDfeEqu_WrRegBitField(InstancePtr, XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET,
			      XDFEEQU_NEXT_TRIGGER_SOURCE_WIDTH,
			      XDFEEQU_NEXT_TRIGGER_SOURCE_OFFSET,
			      XDFEEQU_TRIGGER_SOURCE_IMMEDIATE);

	InstancePtr->StateId = XDFEEQU_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activate chfilter.
* First set bit 0 of the Next Control register (0x24) high.
* Then set the Next Control Trigger Source register (0x28) depending on the
* Trigger_Source. Trigger_Source can be set to:
*    - IMMEDIATE,
*    - TLAST_LOW,
*    - TLAST_HIGH,
* When it is set to TUSER the Trigger_Source.TUSERBit field specifies which
* bit of TUSER to trigger on (0 to 63). When the Trigger_Source is set
* to TLAST TUSER the polarity of the trigger source signal is set by
* the Trigger_Source.Polarity field.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerSource is a trigger data container.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeEqu_Activate(XDfeEqu *InstancePtr,
		      const XDfeEqu_Trigger *TriggerSource)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_INITIALISED);

	/* Set bit 0 of the Next Control register (0x24) high. */
	XDfeEqu_WrRegBitField(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET,
			      XDFEEQU_POWERDOWN_MODE_WIDTH,
			      XDFEEQU_POWERDOWN_MODE_OFFSET,
			      XDFEEQU_POWERDOWN_MODE_POWERUP);

	XDfeEqu_PressTrigger(InstancePtr, TriggerSource);

	InstancePtr->StateId = XDFEEQU_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* DeActivate chfilter.
* First set bit 0 of the Next Control register (0x24) low.
* Then set the Next Control Trigger Source register (0x28) depending on the
* Trigger_Source. Trigger_Source can be set to:
*    - IMMEDIATE,
*    - TLAST_LOW,
*    - TLAST_HIGH,
*    - TUSER{x}_LOW or
*    - TUSER{x}_HIGH.
* Where x is a number in the range 0 to 63 that indicates the bit of TUSER to
* use as the trigger.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerSource is a trigger data container.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void XDfeEqu_Deactivate(XDfeEqu *InstancePtr,
			const XDfeEqu_Trigger *TriggerSource)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);

	/* Set bit 0 of the Next Control register (0x24) low. */
	XDfeEqu_WrRegBitField(InstancePtr, XDFEEQU_CURRENT_CONTROL_OFFSET,
			      XDFEEQU_POWERDOWN_MODE_WIDTH,
			      XDFEEQU_POWERDOWN_MODE_OFFSET,
			      XDFEEQU_POWERDOWN_MODE_POWERDOWN);

	XDfeEqu_PressTrigger(InstancePtr, TriggerSource);

	InstancePtr->StateId = XDFEEQU_STATE_INITIALISED;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* Get an used coefficients settings.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Config configuration container.
* @param    TriggerSource sets a trigger source.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_SwitchCoefficientSet(XDfeEqu *InstancePtr,
				  const XDfeEqu_EqConfig *Config,
				  const XDfeEqu_Trigger *TriggerSource)
{
	u32 Offset;
	u32 Data;
	u32 Tmp;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Config != NULL);
	Xil_AssertVoid(TriggerSource != NULL);

	Offset = XDFEEQU_CURRENT_CONTROL_OFFSET;
	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
	if (XDFEEQU_DATAPATH_MODE_REAL == Config->DatapathMode) {
		Tmp = ((Config->RealDatapathSet &
			XDFEEQU_COEFF_SET_CONTROL_MASK)
		       << XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET) |
		      (Config->RealDatapathSet &
		       XDFEEQU_COEFF_SET_CONTROL_MASK);
	} else if (XDFEEQU_DATAPATH_MODE_COMPLEX == Config->DatapathMode) {
		Tmp = (((Config->RealDatapathSet &
			 XDFEEQU_COEFF_SET_CONTROL_MASK) +
			1U)
		       << XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET) |
		      (Config->RealDatapathSet &
		       XDFEEQU_COEFF_SET_CONTROL_MASK);
	} else {
		Tmp = ((Config->ImDatapathSet & XDFEEQU_COEFF_SET_CONTROL_MASK)
		       << XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET) |
		      (Config->RealDatapathSet &
		       XDFEEQU_COEFF_SET_CONTROL_MASK);
	}

	/* Set bits 7:4 of the Next Control register (0x24) with the values in
	   Config.Real_Datapath_Set, Config.Im_Datapath_Set. */
	Data = XDfeEqu_WrBitField(XDFEEQU_COEFF_SET_CONTROL_WIDTH,
				  XDFEEQU_COEFF_SET_CONTROL_OFFSET, Data, Tmp);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET, Data);

	/* Set the Next Control Trigger Source register (0x28) depending on
	   the Trigger_Source. */
	XDfeEqu_PressTrigger(InstancePtr, TriggerSource);
}

/****************************************************************************/
/**
*
* Set equalizer filter coefficients in Real mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag whic bits indicate channel is enabled.
* @param    EqCoeffs is equalizer coefficients container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_LoadRealCoefficients(const XDfeEqu *InstancePtr, u32 ChannelField,
				  const XDfeEqu_Coeff *EqCoeffs)
{
	u32 Offset;
	u32 Data;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set > (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits == 0U);

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Shift);
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < XDFEEQU_COEFFICIENT_SET_MAX; Index++) {
		XDfeEqu_WriteReg(InstancePtr, Offset,
				 EqCoeffs->Coefficients[Index]);
		Offset += sizeof(u32);
	}

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);

	/* The software should wait for bit 8 in the Channel_Field register
	   to go low before returning. */
	for (Index = 0; Index < XDFEEQU_TIMEOUT; Index++) {
		Data = XDfeEqu_RdRegBitField(InstancePtr, Offset,
					     XDFEEQU_CHANNEL_FIELD_DONE_WIDTH,
					     XDFEEQU_CHANNEL_FIELD_DONE_OFFSET);
		if (Data == 0U) {
			break;
		}
		usleep(XDFEEQU_WAIT);
		if (Index == (XDFEEQU_TIMEOUT - 1)) {
			Xil_AssertVoidAlways();
		}
	}
}

/****************************************************************************/
/**
*
* Set equalizer filter coefficients in Complex mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag whic bits indicate channel is enabled.
* @param    EqCoeffs is equalizer coefficients container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_LoadComplexCoefficients(const XDfeEqu *InstancePtr,
				     u32 ChannelField,
				     const XDfeEqu_Coeff *EqCoeffs)
{
	u32 Offset;
	u32 Data;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set > (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits == 0U);

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Shift);

	/* TODO */
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < XDFEEQU_COEFFICIENT_SET_MAX; Index++) {
		XDfeEqu_WriteReg(InstancePtr, Offset,
				 EqCoeffs->Coefficients[Index]);
		Offset += sizeof(u32);
	}

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);

	/* The software should wait for bit 8 in the Channel_Field register
	   to go low before returning. */
	for (Index = 0; Index < XDFEEQU_TIMEOUT; Index++) {
		Data = XDfeEqu_RdRegBitField(InstancePtr, Offset,
					     XDFEEQU_CHANNEL_FIELD_DONE_WIDTH,
					     XDFEEQU_CHANNEL_FIELD_DONE_OFFSET);
		if (Data == 0U) {
			break;
		}
		usleep(XDFEEQU_WAIT);
		if (Index == (XDFEEQU_TIMEOUT - 1)) {
			Xil_AssertVoidAlways();
		}
	}
}

/****************************************************************************/
/**
*
* Set equalizer filter coefficients in Matrix mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag whic bits indicate channel is enabled.
* @param    EqCoeffs is equalizer coefficients container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_LoadMatrixCoefficients(const XDfeEqu *InstancePtr,
				    u32 ChannelField,
				    const XDfeEqu_Coeff *EqCoeffs)
{
	u32 Offset;
	u32 Data;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set > (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits == 0U);

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Shift);

	/* TODO */
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < XDFEEQU_COEFFICIENT_SET_MAX; Index++) {
		XDfeEqu_WriteReg(InstancePtr, Offset,
				 EqCoeffs->Coefficients[Index]);
		Offset += sizeof(u32);
	}

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);

	/* The software should wait for bit 8 in the Channel_Field register
	   to go low before returning. */
	for (Index = 0; Index < XDFEEQU_TIMEOUT; Index++) {
		Data = XDfeEqu_RdRegBitField(InstancePtr, Offset,
					     XDFEEQU_CHANNEL_FIELD_DONE_WIDTH,
					     XDFEEQU_CHANNEL_FIELD_DONE_OFFSET);
		if (Data == 0U) {
			break;
		}
		usleep(XDFEEQU_WAIT);
		if (Index == (XDFEEQU_TIMEOUT - 1)) {
			Xil_AssertVoidAlways();
		}
	}
}

/****************************************************************************/
/**
*
* Get used coefficients settings.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Config configuration container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_GetCoefficientSet(const XDfeEqu *InstancePtr,
			       XDfeEqu_EqConfig *Config)
{
	u32 Offset;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(Config != NULL);

	/* Gets the values in bits 7:4 of the Current control register and
	   populates Config.Real_Datapath_Set and Config.Im_Datapath_Set. */
	Offset = XDFEEQU_CURRENT_CONTROL_OFFSET;
	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
	if (0U != (Data & (1U << XDFEEQU_COMPLEX_MODE_OFFSET))) {
		Config->ImDatapathSet =
			((Data >> XDFEEQU_COEFF_SET_CONTROL_OFFSET) >>
			 XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET) &
			XDFEEQU_COEFF_SET_CONTROL_MASK;
		Config->RealDatapathSet =
			(Data >> XDFEEQU_COEFF_SET_CONTROL_OFFSET) &
			XDFEEQU_COEFF_SET_CONTROL_MASK;
	} else {
		Config->ImDatapathSet = 0U;
		Config->RealDatapathSet =
			(Data >> XDFEEQU_COEFF_SET_CONTROL_OFFSET) &
			((1U << XDFEEQU_COEFF_SET_CONTROL_WIDTH) - 1U);
	}
}

/****************************************************************************/
/**
*
* Flush buffers.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerSource sets a trigger source.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_FlushBuffers(XDfeEqu *InstancePtr,
			  const XDfeEqu_Trigger *TriggerSource)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(TriggerSource != NULL);

	/* Sets bit 1 of the Next Control register (0x24) high. This bit will
	   self-clear after the buffer flush has been triggered. */
	XDfeEqu_WrRegBitField(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET,
			      XDFEEQU_FLUSH_BUFFERS_WIDTH,
			      XDFEEQU_FLUSH_BUFFERS_OFFSET, 1U);

	/* Set the Next Control Trigger Source register (0x28) depending on
	   the Trigger_Source. */
	XDfeEqu_PressTrigger(InstancePtr, TriggerSource);
}

/****************************************************************************/
/**
*
* Set a data path mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerSource sets a trigger source.
* @param    Mode is a Power down flag.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_SetPowerMode(XDfeEqu *InstancePtr,
			  const XDfeEqu_Trigger *TriggerSource, u32 Mode)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(TriggerSource != NULL);

	/* Set the Dynamic Power Down Mode register (0x2C) depending on
	   Trigger_Source and Mode. Mode can be on or off. */
	XDfeEqu_WrRegBitField(InstancePtr,
			      XDFEEQU_DYNAMIC_POWER_DOWN_MODE_OFFSET,
			      XDFEEQU_DYNAMIC_POWER_DOWN_MODE_NEXT_MODE_WIDTH,
			      XDFEEQU_DYNAMIC_POWER_DOWN_MODE_NEXT_MODE_OFFSET,
			      Mode);

	/* Set the Next Control Trigger Source register (0x28) depending on
	   the Trigger_Source. */
	XDfeEqu_PressTrigger(InstancePtr, TriggerSource);
}

/****************************************************************************/
/**
*
* Get a data path mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerSource sets a trigger source.
* @param    Mode is a pointer to power down status.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeEqu_GetPowerMode(XDfeEqu *InstancePtr, u32 *Mode)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(Mode != NULL);

	/* Set the Dynamic Power Down Mode register (0x2C) depending on
	   Trigger_Source and Mode. Mode can be on or off. */
	*Mode = XDfeEqu_RdRegBitField(
		InstancePtr, XDFEEQU_DYNAMIC_POWER_DOWN_MODE_OFFSET,
		XDFEEQU_DYNAMIC_POWER_DOWN_MODE_CURRENT_MODE_WIDTH,
		XDFEEQU_DYNAMIC_POWER_DOWN_MODE_CURRENT_MODE_OFFSET);
}

/** @} */
