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
* Contains the APIs for the DFE Equalizer Filter component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/22/21 align driver to current specification
*       dc     03/15/21 Add data latency api
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/20/21 Doxygen documentation update
*       dc     05/08/21 Update to common trigger
*
* </pre>
*
******************************************************************************/

#include "xdfeequ.h"
#include "xdfeequ_hw.h"
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
#define XDFEEQU_SEQUENCE_ENTRY_NULL 8U /**< Null sequence entry flag */
#define XDFEEQU_NO_EMPTY_CCID_FLAG 0xFFFFU /**< Not Empty CCID flag */
#define XDFEEQU_U32_NUM_BITS 32U
#define XDFEEQU_COEFF_LOAD_TIMEOUT                                             \
	1000U /**< Units of us declared in XDFEEQU_WAIT */
#define XDFEEQU_WAIT 10U /**< Units of us */
#define XDFEEQU_DELAY_MAX                                                      \
	(1U << XDFEEQU_DELAY_WIDTH) /**< Maximum delay value */
#define XDFEEQU_TAP_MAX 24U /**< Maximum tap value */

#define XDFEEQU_DRIVER_VERSION_MINOR 0U
#define XDFEEQU_DRIVER_VERSION_MAJOR 1U

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
extern struct metal_device CustomDevice[XDFEEQU_MAX_NUM_INSTANCES];
extern metal_phys_addr_t metal_phys[XDFEEQU_MAX_NUM_INSTANCES];
#endif
extern XDfeEqu XDfeEqu_Equalizer[XDFEEQU_MAX_NUM_INSTANCES];
static u32 XDfeEqu_DriverHasBeenRegisteredOnce = 0U;

/************************** Function Definitions ****************************/
extern s32 XDfeEqu_RegisterMetal(XDfeEqu *InstancePtr,
				 struct metal_device **DevicePtr,
				 const char *DeviceNodeName);
extern s32 XDfeEqu_LookupConfig(XDfeEqu *InstancePtr);
extern void XDfeEqu_CfgInitialize(XDfeEqu *InstancePtr);

/************************** Register Access Functions ***********************/

/****************************************************************************/
/**
*
* Writes value to register in an Equalizer instance.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    AddrOffset is address offset relative to instance base address.
* @param    Data is value to be written.
*
*
****************************************************************************/
void XDfeEqu_WriteReg(const XDfeEqu *InstancePtr, u32 AddrOffset, u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	metal_io_write32(InstancePtr->Io, (unsigned long)AddrOffset, Data);
}

/****************************************************************************/
/**
*
* Reads a value from the register using an Equalizer instance.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    AddrOffset is address offset relative to instance base address.
*
* @return   Register value.
*
*
****************************************************************************/
u32 XDfeEqu_ReadReg(const XDfeEqu *InstancePtr, u32 AddrOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return metal_io_read32(InstancePtr->Io, (unsigned long)AddrOffset);
}

/****************************************************************************/
/**
*
* Writes a bit field value to register.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
* @param    FieldData is a bit field data.
*
****************************************************************************/
void XDfeEqu_WrRegBitField(const XDfeEqu *InstancePtr, u32 Offset,
			   u32 FieldWidth, u32 FieldOffset, u32 FieldData)
{
	u32 Data;
	u32 Tmp;
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((FieldOffset + FieldWidth) <= XDFEEQU_U32_NUM_BITS);

	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
	Val = (FieldData & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	Tmp = ~((((u32)1U << FieldWidth) - 1U) << FieldOffset);
	Data = (Data & Tmp) | Val;
	XDfeEqu_WriteReg(InstancePtr, Offset, Data);
}

/****************************************************************************/
/**
*
* Reads a bit field value from the register.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
* @param    Offset is address offset relative to instance base address.
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset.
*
* @return   Bit field data.
*
****************************************************************************/
u32 XDfeEqu_RdRegBitField(const XDfeEqu *InstancePtr, u32 Offset,
			  u32 FieldWidth, u32 FieldOffset)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEEQU_U32_NUM_BITS);

	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
	return ((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U));
}

/****************************************************************************/
/**
*
* Reads a bit field value from the u32 variable.
*
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset in bits number.
* @param    Data is a u32 data which bit field this function reads.
*
* @return   Bit field value.
*
****************************************************************************/
u32 XDfeEqu_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data)
{
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEEQU_U32_NUM_BITS);
	return (((Data >> FieldOffset) & (((u32)1U << FieldWidth) - 1U)));
}
/****************************************************************************/
/**
*
* Writes a bit field value to the u32 variable.
*
* @param    FieldWidth is a bit field width.
* @param    FieldOffset is a bit field offset in bits number.
* @param    Data is a u32 data which bit field this function reads.
* @param    Val is a u32 value to be written in the bit field.
*
* @return   Data with a bit field written.
*
****************************************************************************/
u32 XDfeEqu_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data, u32 Val)
{
	u32 BitFieldSet;
	u32 BitFieldClear;
	Xil_AssertNonvoid((FieldOffset + FieldWidth) <= XDFEEQU_U32_NUM_BITS);

	BitFieldSet = (Val & (((u32)1U << FieldWidth) - 1U)) << FieldOffset;
	BitFieldClear =
		Data & (~((((u32)1U << FieldWidth) - 1U) << FieldOffset));
	return (BitFieldSet | BitFieldClear);
}

/************************ DFE Common functions ******************************/

/************************ Low Level Functions *******************************/

/****************************************************************************/
/**
*
* Sets Equalizer filter coefficients in Real mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag which bits indicate channel is enabled.
* @param    EqCoeffs is an Equalizer coefficients container.
* @param    NumValues is the number of taps.
* @param    Shift is shift value.
*
****************************************************************************/
static void XDfeEqu_LoadRealCoefficients(const XDfeEqu *InstancePtr,
					 u32 ChannelField,
					 const XDfeEqu_Coefficients *EqCoeffs,
					 u32 NumValues, u32 Shift)
{
	u32 Offset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set < (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits <= XDFEEQU_MAX_NUMBER_OF_UNITS_REAL);

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < NumValues; Index++) {
		XDfeEqu_WriteReg(InstancePtr, Offset,
				 (u32)(EqCoeffs->Coefficients[Index]));
		Offset += (u32)sizeof(u32);
	}
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, Shift);

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);
}

/****************************************************************************/
/**
*
* Sets Equalizer filter coefficients in Complex mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag in which bits indicate the channel is
*           enabled.
* @param    EqCoeffs is an Equalizer coefficients container.
* @param    NumValues is the number of taps.
* @param    Shift is shift value.
*
****************************************************************************/
static void
XDfeEqu_LoadComplexCoefficients(const XDfeEqu *InstancePtr, u32 ChannelField,
				const XDfeEqu_Coefficients *EqCoeffs,
				u32 NumValues, u32 Shift)
{
	u32 Offset;
	u32 LoadDone;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set < (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits <= XDFEEQU_MAX_NUMBER_OF_UNITS_COMPLEX);

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < NumValues; Index++) {
		XDfeEqu_WriteReg(InstancePtr, Offset,
				 (u32)(EqCoeffs->Coefficients[Index]));
		XDfeEqu_WriteReg(
			InstancePtr,
			Offset + (XDFEEQU_IM_COEFFICIENT_SET_OFFSET *
				  sizeof(u32)),
			(u32)(EqCoeffs->Coefficients[Index + NumValues]));
		Offset += (u32)sizeof(u32);
	}
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, Shift);

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);

	/* The software should wait for bit 8 in the Channel_Field register
	   to go low before returning. */
	for (Index = 0; Index < XDFEEQU_COEFF_LOAD_TIMEOUT; Index++) {
		LoadDone = XDfeEqu_RdRegBitField(
			InstancePtr, Offset, XDFEEQU_CHANNEL_FIELD_DONE_WIDTH,
			XDFEEQU_CHANNEL_FIELD_DONE_OFFSET);
		if (XDFEEQU_CHANNEL_FIELD_DONE_LOADING_DONE == LoadDone) {
			break;
		}
		usleep(XDFEEQU_WAIT);
		if (Index == (XDFEEQU_COEFF_LOAD_TIMEOUT - 1U)) {
			Xil_AssertVoidAlways();
		}
	}

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < NumValues; Index++) {
		XDfeEqu_WriteReg(
			InstancePtr, Offset,
			(u32)(-EqCoeffs->Coefficients[Index + NumValues]));
		XDfeEqu_WriteReg(InstancePtr,
				 Offset + (XDFEEQU_IM_COEFFICIENT_SET_OFFSET *
					   sizeof(u32)),
				 (u32)(EqCoeffs->Coefficients[Index]));
		Offset += (u32)sizeof(u32);
	}
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set + 1U);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, Shift);

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);
}

/****************************************************************************/
/**
*
* Sets Equalizer filter coefficients in Matrix mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag in which bits indicate channel is enabled.
* @param    EqCoeffs is an Equalizer coefficients container.
* @param    NumValues is the number of taps.
* @param    Shift is shift value.
*
****************************************************************************/
static void XDfeEqu_LoadMatrixCoefficients(const XDfeEqu *InstancePtr,
					   u32 ChannelField,
					   const XDfeEqu_Coefficients *EqCoeffs,
					   u32 NumValues, u32 Shift)
{
	u32 Offset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set < (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits <= XDFEEQU_MAX_NUMBER_OF_UNITS_COMPLEX);

	/* Write the co-efficient set buffer with the following information */
	Offset = XDFEEQU_COEFFICIENT_SET;
	for (Index = 0; Index < NumValues; Index++) {
		XDfeEqu_WriteReg(InstancePtr, Offset,
				 (u32)(EqCoeffs->Coefficients[Index]));
		XDfeEqu_WriteReg(
			InstancePtr,
			Offset + (XDFEEQU_IM_COEFFICIENT_SET_OFFSET *
				  sizeof(u32)),
			(u32)(EqCoeffs->Coefficients[Index + NumValues]));
		Offset += (u32)sizeof(u32);
	}
	Offset = XDFEEQU_SET_TO_WRITE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->Set);
	Offset = XDFEEQU_NUMBER_OF_UNITS_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, EqCoeffs->NUnits);
	Offset = XDFEEQU_SHIFT_VALUE_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, Shift);

	/* Set the Channel_Field register (0x010C) with the value in
	   Channel_Field. This initiates the write of the co-efficients. */
	Offset = XDFEEQU_CHANNEL_FIELD_OFFSET;
	XDfeEqu_WriteReg(InstancePtr, Offset, ChannelField);
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of update trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
****************************************************************************/
static void XDfeEqu_EnableUpdateTrigger(const XDfeEqu *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr,
			       XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET,
			 Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers and sets enable bit of LowPower trigger.
* If Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
****************************************************************************/
static void XDfeEqu_EnableLowPowerTrigger(const XDfeEqu *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr,
			       XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	XDfeEqu_WriteReg(InstancePtr,
			 XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET, Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set enable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
****************************************************************************/
static void XDfeEqu_EnableActivateTrigger(const XDfeEqu *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr,
			       XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  XDFEEQU_TRIGGERS_STATE_OUTPUT_ENABLED);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET,
			 Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers, set disable bit of Activate trigger. If
* Mode = IMMEDIATE, then trigger will be applied immediately.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
****************************************************************************/
static void XDfeEqu_EnableDeactivateTrigger(const XDfeEqu *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr,
			       XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_ENABLED);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				  XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Data,
				  XDFEEQU_TRIGGERS_STATE_OUTPUT_DISABLED);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET,
			 Data);
}

/****************************************************************************/
/**
*
* Reads the Triggers and resets enable a bit of LowPower trigger.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
****************************************************************************/
static void XDfeEqu_DisableLowPowerTrigger(const XDfeEqu *InstancePtr)
{
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XDfeEqu_ReadReg(InstancePtr,
			       XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_DISABLED);
	XDfeEqu_WriteReg(InstancePtr,
			 XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET, Data);
}

/*************************** Init API ***************************************/

/*****************************************************************************/
/**
*
* API initialises one instance of an Equalizer driver.
* Traverse "/sys/bus/platform/device" directory (in Linux), to find registered
* EQU device with the name DeviceNodeName. The first available slot in
* the instances array XDfeEqu_ChFilter[] will be taken as a DeviceNodeName
* object.
*
* @param    DeviceNodeName is device node name.
*
* @return
*           - pointer to instance if successful.
*           - NULL on error.
*
******************************************************************************/
XDfeEqu *XDfeEqu_InstanceInit(const char *DeviceNodeName)
{
	u32 Index;
	XDfeEqu *InstancePtr;
#ifdef __BAREMETAL__
	char Str[XDFEEQU_NODE_NAME_MAX_LENGTH];
	char *AddrStr;
	u32 Addr;
#endif

	Xil_AssertNonvoid(DeviceNodeName != NULL);
	Xil_AssertNonvoid(strlen(DeviceNodeName) <
			  XDFEEQU_NODE_NAME_MAX_LENGTH);

	/* Is this first EQU initialisation ever? */
	if (0U == XDfeEqu_DriverHasBeenRegisteredOnce) {
		/* Set up environment to non-initialized */
		for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
			XDfeEqu_Equalizer[Index].StateId =
				XDFEEQU_STATE_NOT_READY;
			XDfeEqu_Equalizer[Index].NodeName[0] = '\0';
		}
		XDfeEqu_DriverHasBeenRegisteredOnce = 1U;
	}

	/*
	 * Check has DeviceNodeName been already created:
	 * a) if no, do full initialization
	 * b) if yes, skip initialization and return the object pointer
	 */
	for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
		if (0U == strncmp(XDfeEqu_Equalizer[Index].NodeName,
				  DeviceNodeName, strlen(DeviceNodeName))) {
			return &XDfeEqu_Equalizer[Index];
		}
	}

	/*
	 * Find the available slot for this instance.
	 */
	for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
		if (XDfeEqu_Equalizer[Index].NodeName[0] == '\0') {
			strncpy(XDfeEqu_Equalizer[Index].NodeName,
				DeviceNodeName, strlen(DeviceNodeName));
			InstancePtr = &XDfeEqu_Equalizer[Index];
			goto register_metal;
		}
	}

	/* Failing as there is no available slot. */
	return NULL;

register_metal:
#ifdef __BAREMETAL__
	memcpy(Str, InstancePtr->NodeName, XDFEEQU_NODE_NAME_MAX_LENGTH);
	AddrStr = strtok(Str, ".");
	Addr = strtol(AddrStr, NULL, 16);
	for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
		if (Addr == metal_phys[Index]) {
			InstancePtr->Device = &CustomDevice[Index];
			goto bm_register_metal;
		}
	}
	return NULL;
bm_register_metal:
#endif

	/* Register libmetal for this OS process */
	if (XST_SUCCESS != XDfeEqu_RegisterMetal(InstancePtr,
						 &InstancePtr->Device,
						 DeviceNodeName)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to register device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Setup config data */
	if (XST_FAILURE == XDfeEqu_LookupConfig(InstancePtr)) {
		metal_log(METAL_LOG_ERROR, "\n Failed to configure device %s",
			  DeviceNodeName);
		goto return_error;
	}

	/* Configure HW and the driver instance */
	XDfeEqu_CfgInitialize(InstancePtr);

	InstancePtr->StateId = XDFEEQU_STATE_READY;

	return InstancePtr;

return_error:
	InstancePtr->StateId = XDFEEQU_STATE_NOT_READY;
	InstancePtr->NodeName[0] = '\0';
	return NULL;
}

/*****************************************************************************/
/**
*
* API closes the instance of an Equalizer driver.
*
* @param    InstancePtr is a pointer to the XDfeEqu instance.
*
******************************************************************************/
void XDfeEqu_InstanceClose(XDfeEqu *InstancePtr)
{
	u32 Index;
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; Index < XDFEEQU_MAX_NUM_INSTANCES; Index++) {
		/* Find the instance in XDfeEqu_Equalizer array */
		if (&XDfeEqu_Equalizer[Index] == InstancePtr) {
			/* Release libmetal */
			metal_device_close(InstancePtr->Device);
			InstancePtr->StateId = XDFEEQU_STATE_NOT_READY;
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
* This function puts the block into a reset state.
* Sets bit 0 of the Master Reset register (0x4) high.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
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
* Reads configuration from device tree/xparameters.h and IP registers. S/W
* reset removed by setting bit 0 of the Master Reset register (0x4) low.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Cfg is a pointer to the device config structure.
*
****************************************************************************/
void XDfeEqu_Configure(XDfeEqu *InstancePtr, XDfeEqu_Cfg *Cfg)
{
	u32 Version;
	u32 ModelParam;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_RESET);
	Xil_AssertVoid(Cfg != NULL);

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
	ModelParam = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_MODEL_PARAM_OFFSET);
	InstancePtr->Config.NumChannels =
		XDfeEqu_RdBitField(XDFEEQU_MODEL_PARAM_NUM_CHANNELS_WIDTH,
				   XDFEEQU_MODEL_PARAM_NUM_CHANNELS_OFFSET,
				   ModelParam);
	InstancePtr->Config.SampleWidth =
		XDfeEqu_RdBitField(XDFEEQU_MODEL_PARAM_SAMPLE_WIDTH_WIDTH,
				   XDFEEQU_MODEL_PARAM_SAMPLE_WIDTH_OFFSET,
				   ModelParam);
	InstancePtr->Config.ComplexModel =
		XDfeEqu_RdBitField(XDFEEQU_MODEL_PARAM_COMPLEX_MODE_WIDTH,
				   XDFEEQU_MODEL_PARAM_COMPLEX_MODE_OFFSET,
				   ModelParam);
	InstancePtr->Config.TuserWidth =
		XDfeEqu_RdBitField(XDFEEQU_MODEL_PARAM_TUSER_WIDTH_WIDTH,
				   XDFEEQU_MODEL_PARAM_TUSER_WIDTH_OFFSET,
				   ModelParam);
	Cfg->ModelParams.SampleWidth = InstancePtr->Config.SampleWidth;
	Cfg->ModelParams.ComplexModel = InstancePtr->Config.ComplexModel;
	Cfg->ModelParams.NumChannels = InstancePtr->Config.NumChannels;
	Cfg->ModelParams.TuserWidth = InstancePtr->Config.TuserWidth;

	/* Release RESET */
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_RESET_OFFSET, XDFEEQU_RESET_OFF);
	InstancePtr->StateId = XDFEEQU_STATE_CONFIGURED;
}

/****************************************************************************/
/**
*
* DFE Equalizer driver one time initialisation.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Config is a configuration data container.
*
****************************************************************************/
void XDfeEqu_Initialize(XDfeEqu *InstancePtr, const XDfeEqu_EqConfig *Config)
{
	XDfeEqu_Trigger Update;
	u32 Data;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_CONFIGURED);
	Xil_AssertVoid(Config != NULL);
	Xil_AssertVoid(Config->DatapathMode <= XDFEEQU_DATAPATH_MODE_MATRIX);

	Data = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET);
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

	/* Trigger NEXT_CONTROL (UPDATE) immediately using Register source to
	   update CURRENT from NEXT */
	Update.TriggerEnable = XDFEEQU_TRIGGERS_TRIGGER_ENABLE_ENABLED;
	Update.Mode = XDFEEQU_TRIGGERS_MODE_IMMEDIATE;
	Data = XDfeEqu_ReadReg(InstancePtr,
			       XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				  XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Data,
				  Update.TriggerEnable);
	Data = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_MODE_WIDTH,
				  XDFEEQU_TRIGGERS_MODE_OFFSET, Data,
				  Update.Mode);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET,
			 Data);

	InstancePtr->StateId = XDFEEQU_STATE_INITIALISED;
}

/*****************************************************************************/
/**
*
* Activates mixer.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    EnableLowPower is a flag indicating low power.
*
******************************************************************************/
void XDfeEqu_Activate(XDfeEqu *InstancePtr, bool EnableLowPower)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEEQU_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL));

	/* Do nothing if the block already operational */
	IsOperational =
		XDfeEqu_RdRegBitField(InstancePtr,
				      XDFEEQU_CURRENT_OPERATIONAL_MODE_OFFSET,
				      XDFEEQU_CURRENT_MODE_OPERATIONAL_WIDTH,
				      XDFEEQU_CURRENT_MODE_OPERATIONAL_OFFSET);
	if (IsOperational == XDFEEQU_CURRENT_MODE_OPERATIONAL_ENABLED) {
		return;
	}

	/* Enable the Activate trigger and set to one-shot */
	XDfeEqu_EnableActivateTrigger(InstancePtr);

	/* Enable the LowPower trigger, set to continuous triggering */
	if (EnableLowPower == true) {
		XDfeEqu_EnableLowPowerTrigger(InstancePtr);
	}

	/* Equalizer is operational now, change a state */
	InstancePtr->StateId = XDFEEQU_STATE_OPERATIONAL;
}

/*****************************************************************************/
/**
*
* Deactivates chfilter.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
******************************************************************************/
void XDfeEqu_Deactivate(XDfeEqu *InstancePtr)
{
	u32 IsOperational;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr->StateId == XDFEEQU_STATE_INITIALISED) ||
		       (InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL));

	/* Do nothing if the block already deactivated */
	IsOperational =
		XDfeEqu_RdRegBitField(InstancePtr,
				      XDFEEQU_CURRENT_OPERATIONAL_MODE_OFFSET,
				      XDFEEQU_CURRENT_MODE_OPERATIONAL_WIDTH,
				      XDFEEQU_CURRENT_MODE_OPERATIONAL_OFFSET);
	if (IsOperational == XDFEEQU_CURRENT_MODE_OPERATIONAL_DISABLED) {
		return;
	}

	/* Disable LowPower trigger (may not be enabled) */
	XDfeEqu_DisableLowPowerTrigger(InstancePtr);

	/* Disable Activate trigger */
	XDfeEqu_EnableDeactivateTrigger(InstancePtr);

	InstancePtr->StateId = XDFEEQU_STATE_INITIALISED;
}

/*************************** Component API **********************************/

/****************************************************************************/
/**
*
* The software first sets bits 7:4 of the Next Control register (0x24) with
* the values in Config.Real_Datapath_Set, Config.Im_Datapath_Set.
* In real mode bits, 5:4 are set to the value held in
* Config.Real_Datapath_Set.
* Bits 7:6 are set to the value held in Config.Real_Datapath_Set.
* In complex mode bits, 5:4 are set to the value held in value
* Config.Real_Datapath_Set. Bits 7:6 are set to the value held in
* Config.Real_Datapath_Set plus 1.
* In matrix mode bits 5:4 are set to the value held in
* Config.Real_Datapath_Set. Bits 7:6 are set to the value held in
* Config.Im_Datapath_Set.
* The software sets bit 1 depending on Config.Flush.
* The software then sets the _Enable bit in the Next Control Trigger
* Source register (0x28) high.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Config configuration container.
*
****************************************************************************/
void XDfeEqu_Update(const XDfeEqu *InstancePtr, const XDfeEqu_EqConfig *Config)
{
	u32 Offset;
	u32 Data;
	u32 ReTmp;
	u32 ImTmp;
	u32 CoeffTmp;
	u32 FlushTmp;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Config != NULL);

	Offset = XDFEEQU_CURRENT_CONTROL_OFFSET;
	Data = XDfeEqu_ReadReg(InstancePtr, Offset);

	/* Set bits 7:4 of the Next Control register (0x24) with the values in
	   Config.Real_Datapath_Set, Config.Im_Datapath_Set. */
	ReTmp = Config->RealDatapathSet & XDFEEQU_COEFF_SET_CONTROL_MASK;
	if (XDFEEQU_DATAPATH_MODE_COMPLEX == Config->DatapathMode) {
		ImTmp = Config->RealDatapathSet + 1U;
	} else {
		ImTmp = Config->ImDatapathSet;
	}
	ImTmp &= XDFEEQU_COEFF_SET_CONTROL_MASK;
	CoeffTmp = ReTmp | (ImTmp << XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET);
	Data = XDfeEqu_WrBitField(XDFEEQU_COEFF_SET_CONTROL_WIDTH,
				  XDFEEQU_COEFF_SET_CONTROL_OFFSET, Data,
				  CoeffTmp);

	/* The software sets bit 1 depending on Config.Flush */
	FlushTmp = Config->Flush & XDFEEQU_FLUSH_BUFFERS_WIDTH;
	Data = XDfeEqu_WrBitField(XDFEEQU_FLUSH_BUFFERS_WIDTH,
				  XDFEEQU_FLUSH_BUFFERS_OFFSET, Data, FlushTmp);

	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_NEXT_CONTROL_OFFSET, Data);
	XDfeEqu_EnableUpdateTrigger(InstancePtr);
}

/****************************************************************************/
/**
*
* Returns current trigger configuration.
*
* @param    InstancePtr is a pointer to the Ccf instance.
* @param    TriggerCfg is a trigger configuration container.
*
****************************************************************************/
void XDfeEqu_GetTriggersCfg(const XDfeEqu *InstancePtr,
			    XDfeEqu_TriggerCfg *TriggerCfg)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId != XDFEEQU_STATE_NOT_READY);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Read OPERATIONAL (ACTIVATE) triggers */
	Val = XDfeEqu_ReadReg(InstancePtr,
			      XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET);
	TriggerCfg->Activate.TriggerEnable =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->Activate.Mode = XDfeEqu_RdBitField(
		XDFEEQU_TRIGGERS_MODE_WIDTH, XDFEEQU_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Activate.TUSERBit =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Activate.TuserEdgeLevel =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->Activate.StateOutput =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read LOW_POWER triggers */
	Val = XDfeEqu_ReadReg(InstancePtr,
			      XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET);
	TriggerCfg->LowPower.TriggerEnable =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->LowPower.Mode = XDfeEqu_RdBitField(
		XDFEEQU_TRIGGERS_MODE_WIDTH, XDFEEQU_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->LowPower.TUSERBit =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->LowPower.TuserEdgeLevel =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->LowPower.StateOutput =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Val);

	/* Read NEXT_CONTROL (UPDATE) triggers */
	Val = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET);
	TriggerCfg->Update.TriggerEnable =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				   XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val);
	TriggerCfg->Update.Mode = XDfeEqu_RdBitField(
		XDFEEQU_TRIGGERS_MODE_WIDTH, XDFEEQU_TRIGGERS_MODE_OFFSET, Val);
	TriggerCfg->Update.TUSERBit =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH,
				   XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET, Val);
	TriggerCfg->Update.TuserEdgeLevel =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				   XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET,
				   Val);
	TriggerCfg->Update.StateOutput =
		XDfeEqu_RdBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				   XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets trigger configuration.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    TriggerCfg is a trigger configuration container.
*
****************************************************************************/
void XDfeEqu_SetTriggersCfg(const XDfeEqu *InstancePtr,
			    XDfeEqu_TriggerCfg *TriggerCfg)
{
	u32 Val;
	u32 TUSERWidth;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_INITIALISED);
	Xil_AssertVoid(TriggerCfg != NULL);

	/* Write trigger configuration members */

	TUSERWidth =
		XDfeEqu_RdRegBitField(InstancePtr,
				      XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET,
				      XDFEEQU_MODEL_PARAM_TUSER_WIDTH_WIDTH,
				      XDFEEQU_MODEL_PARAM_TUSER_WIDTH_OFFSET);

	if (TUSERWidth == 0U) {
		TriggerCfg->Activate.Mode = XDFEEQU_TRIGGERS_MODE_IMMEDIATE;
		TriggerCfg->LowPower.Mode = XDFEEQU_TRIGGERS_MODE_IMMEDIATE;
		TriggerCfg->Update.Mode = XDFEEQU_TRIGGERS_MODE_IMMEDIATE;
	} else {
		TriggerCfg->Activate.Mode =
			XDFEEQU_TRIGGERS_MODE_TUSER_SINGLE_SHOT;
		TriggerCfg->LowPower.Mode =
			XDFEEQU_TRIGGERS_MODE_TUSER_CONTINUOUS;
		TriggerCfg->Update.Mode =
			XDFEEQU_TRIGGERS_MODE_TUSER_SINGLE_SHOT;
	}

	/* Activate defined as SingleShot (as per the programming model) or
	   immediate if there is no TUSER on the IP. */
	/* Read/set/write OPERATIONAL (ACTIVATE) triggers */
	TriggerCfg->Activate.TriggerEnable =
		XDFEEQU_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	Val = XDfeEqu_ReadReg(InstancePtr,
			      XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->Activate.TriggerEnable);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_MODE_WIDTH,
				 XDFEEQU_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->Activate.Mode);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->Activate.TuserEdgeLevel);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->Activate.TUSERBit);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->Activate.StateOutput);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_OPERATIONAL_MODE_TRIGGER_OFFSET,
			 Val);

	/* LowPower defined as Continuous */
	TriggerCfg->LowPower.TriggerEnable =
		XDFEEQU_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	/* Read/set/write LOW_POWER triggers */
	Val = XDfeEqu_ReadReg(InstancePtr,
			      XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->LowPower.TriggerEnable);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_MODE_WIDTH,
				 XDFEEQU_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->LowPower.Mode);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->LowPower.TuserEdgeLevel);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->LowPower.TUSERBit);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->LowPower.StateOutput);
	XDfeEqu_WriteReg(InstancePtr,
			 XDFEEQU_DYNAMIC_POWER_DOWN_MODE_TRIGGER_OFFSET, Val);

	/* Update defined as SingleShot (as per the programming model) */
	/* Read/set/write NEXT_CONTROL (UPDATE) triggers */
	TriggerCfg->Update.TriggerEnable =
		XDFEEQU_TRIGGERS_TRIGGER_ENABLE_DISABLED;
	Val = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TRIGGER_ENABLE_WIDTH,
				 XDFEEQU_TRIGGERS_TRIGGER_ENABLE_OFFSET, Val,
				 TriggerCfg->Update.TriggerEnable);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_MODE_WIDTH,
				 XDFEEQU_TRIGGERS_MODE_OFFSET, Val,
				 TriggerCfg->Update.Mode);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_WIDTH,
				 XDFEEQU_TRIGGERS_TUSER_EDGE_LEVEL_OFFSET, Val,
				 TriggerCfg->Update.TuserEdgeLevel);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_TUSER_BIT_WIDTH,
				 XDFEEQU_TRIGGERS_TUSER_BIT_OFFSET, Val,
				 TriggerCfg->Update.TUSERBit);
	Val = XDfeEqu_WrBitField(XDFEEQU_TRIGGERS_STATE_OUTPUT_WIDTH,
				 XDFEEQU_TRIGGERS_STATE_OUTPUT_OFFSET, Val,
				 TriggerCfg->Update.StateOutput);
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_NEXT_CONTROL_TRIGGER_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Sets Equalizer filter coefficients in Real, Complex or Matrix mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField is a flag in which bits indicate the channel is
*           enabled.
* @param    Mode is an Equalizer mode.
* @param    EqCoeffs is the Equalizer coefficients container.
*
****************************************************************************/
void XDfeEqu_LoadCoefficients(const XDfeEqu *InstancePtr, u32 ChannelField,
			      u32 Mode, const XDfeEqu_Coefficients *EqCoeffs)
{
	u32 NumValues;
	s32 CoeffSum = 0;
	double ScaleFactor;
	u32 Shift;
	u32 LoadDone;
	u32 SumItemNum;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(ChannelField <
		       ((u32)1U << XDFEEQU_CHANNEL_FIELD_FIELD_WIDTH));
	Xil_AssertVoid(Mode <= XDFEEQU_DATAPATH_MODE_MATRIX);
	Xil_AssertVoid(EqCoeffs != NULL);
	Xil_AssertVoid(EqCoeffs->Set < (1U << XDFEEQU_SET_TO_WRITE_SET_WIDTH));
	Xil_AssertVoid(EqCoeffs->NUnits > 0U);

	/* Determine scale shift value using expression given in specification
	   item 10 in complex equalizer. */
	NumValues = EqCoeffs->NUnits * XDFEEQU_TAP_UNIT_SIZE;
	if (Mode == XDFEEQU_DATAPATH_MODE_REAL) {
		SumItemNum = NumValues;
	} else {
		/* Complex is data number x2 */
		SumItemNum = 2U * NumValues;
	}

	for (Index = 0; Index < SumItemNum; Index++) {
		CoeffSum += EqCoeffs->Coefficients[Index];
	}

	/* The following two lines are a formula to claculate a shift value.
	   Coefficient AXI stream will have TLAST to indicate that all
	   coefficients, shift values, num_active_units associated with one
	   co-efficient set have arrived. The first value on co-efficient
	   stream will be a shift value associated with that co-efficient
	   set. */
	ScaleFactor = (double)CoeffSum / (256U * ((u32)1 << 15));
	Shift = (u32)floor(fabs(log2(ScaleFactor)));

	/* Check is load in progress */
	for (Index = 0; Index < XDFEEQU_COEFF_LOAD_TIMEOUT; Index++) {
		LoadDone = XDfeEqu_RdRegBitField(
			InstancePtr, XDFEEQU_CHANNEL_FIELD_OFFSET,
			XDFEEQU_CHANNEL_FIELD_DONE_WIDTH,
			XDFEEQU_CHANNEL_FIELD_DONE_OFFSET);
		if (XDFEEQU_CHANNEL_FIELD_DONE_LOADING_DONE == LoadDone) {
			/* Loading is finished */
			break;
		}
		usleep(XDFEEQU_WAIT);
		if (Index == (XDFEEQU_COEFF_LOAD_TIMEOUT - 1U)) {
			/* Loading still on, this is serious problem block
			   the system */
			Xil_AssertVoidAlways();
		}
	}

	/* Write filter coefficients and initiate new coefficient load */
	if (Mode == XDFEEQU_DATAPATH_MODE_REAL) {
		/* Mode == real */
		XDfeEqu_LoadRealCoefficients(InstancePtr, ChannelField,
					     EqCoeffs, NumValues, Shift);
	} else if (Mode == XDFEEQU_DATAPATH_MODE_COMPLEX) {
		/* Mode == complex */
		XDfeEqu_LoadComplexCoefficients(InstancePtr, ChannelField,
						EqCoeffs, NumValues, Shift);
	} else {
		/* Mode == matrix */
		XDfeEqu_LoadMatrixCoefficients(InstancePtr, ChannelField,
					       EqCoeffs, NumValues, Shift);
	}
}

/****************************************************************************/
/**
*
* Gets used coefficients settings.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    RealSet is a pointer to a real value.
* @param    ImagSet is a pointer to an imaginary value.
*
****************************************************************************/
void XDfeEqu_GetActiveSets(const XDfeEqu *InstancePtr, u32 *RealSet,
			   u32 *ImagSet)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_OPERATIONAL);
	Xil_AssertVoid(RealSet != NULL);
	Xil_AssertVoid(ImagSet != NULL);

	/* Gets the values in bits 7:4 of the Current control register and
	   populates Config.Real_Datapath_Set and Config.Im_Datapath_Set. */
	Data = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_CURRENT_CONTROL_OFFSET);
	if (0U != (Data & (1U << XDFEEQU_COMPLEX_MODE_OFFSET))) {
		*ImagSet = ((Data >> XDFEEQU_COEFF_SET_CONTROL_OFFSET) >>
			    XDFEEQU_COEFF_SET_CONTROL_IM_OFFSET) &
			   XDFEEQU_COEFF_SET_CONTROL_MASK;
		*RealSet = (Data >> XDFEEQU_COEFF_SET_CONTROL_OFFSET) &
			   XDFEEQU_COEFF_SET_CONTROL_MASK;
	} else {
		*ImagSet = 0U;
		*RealSet = (Data >> XDFEEQU_COEFF_SET_CONTROL_OFFSET) &
			   ((1U << XDFEEQU_COEFF_SET_CONTROL_WIDTH) - 1U);
	}
}

/****************************************************************************/
/**
*
* Sets the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Delay is a requested delay variable.
*
****************************************************************************/
void XDfeEqu_SetTUserDelay(const XDfeEqu *InstancePtr, u32 Delay)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEEQU_STATE_INITIALISED);
	Xil_AssertVoid(Delay < XDFEEQU_DELAY_MAX);

	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_DELAY, Delay);
}

/****************************************************************************/
/**
*
* Reads the delay, which will be added to TUSER and TLAST (delay matched
* through the IP).
*
* @param    InstancePtr is a pointer to the Equalizer instance.
*
* @return   Delay value
*
****************************************************************************/
u32 XDfeEqu_GetTUserDelay(const XDfeEqu *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDfeEqu_RdRegBitField(InstancePtr, XDFEEQU_DELAY,
				     XDFEEQU_DELAY_WIDTH, XDFEEQU_DELAY_OFFSET);
}

/****************************************************************************/
/**
*
* Returns CONFIG.DATA_LATENCY.VALUE + tap, where the tap is between 0
* and 23 in real mode and between 0 and 11 in complex/matrix mode.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    Tap is a tap variable.
*
* @return   Data latency value.
*
****************************************************************************/
u32 XDfeEqu_GetTDataDelay(const XDfeEqu *InstancePtr, u32 Tap)
{
	u32 Data;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tap < XDFEEQU_TAP_MAX);

	Data = XDfeEqu_RdRegBitField(InstancePtr, XDFEEQU_DATA_LATENCY,
				     XDFEEQU_DATA_LATENCY_WIDTH,
				     XDFEEQU_DATA_LATENCY_OFFSET);
	return (Data + Tap);
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
void XDfeEqu_GetVersions(const XDfeEqu *InstancePtr, XDfeEqu_Version *SwVersion,
			 XDfeEqu_Version *HwVersion)
{
	u32 Version;

	Xil_AssertVoid(InstancePtr->StateId != XDFEEQU_STATE_NOT_READY);

	/* Driver version */
	SwVersion->Major = XDFEEQU_DRIVER_VERSION_MAJOR;
	SwVersion->Minor = XDFEEQU_DRIVER_VERSION_MINOR;

	/* Component HW version */
	Version = XDfeEqu_ReadReg(InstancePtr, XDFEEQU_VERSION_OFFSET);
	HwVersion->Patch =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_PATCH_WIDTH,
				   XDFEEQU_VERSION_PATCH_OFFSET, Version);
	HwVersion->Revision =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_REVISION_WIDTH,
				   XDFEEQU_VERSION_REVISION_OFFSET, Version);
	HwVersion->Minor =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_MINOR_WIDTH,
				   XDFEEQU_VERSION_MINOR_OFFSET, Version);
	HwVersion->Major =
		XDfeEqu_RdBitField(XDFEEQU_VERSION_MAJOR_WIDTH,
				   XDFEEQU_VERSION_MAJOR_OFFSET, Version);
}
/** @} */
