/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_common.c
* @addtogroup sysmonpsv_api SYSMONPSV APIs
*
* Functions in the xsysmonpsv_common.c file are basic driver functions which will be used in the
* in servies or directly by the user.
*
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 4.0   se     10/04/22 Update return value definitions
*		se	   10/27/22 Secure and Non-Secure mode integration
* 5.1   se     03/03/25 Compiler warnings fixed
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv_lowlevel.h"
#include "xsysmonpsv_common.h"
#include "xil_util.h"

/****************************************************************************/
/**
*
* Provides the register offset for the different type.
*
* @param	Type Max or Min Temperature Type
*
* @return	-XST_FAILURE when fails
*		    -Register offset
*
***************************************************************************/
int XSysMonPsv_TempOffset(XSysMonPsv_TempType Type)
{
	int RetValue;
	switch (Type) {
	case XSYSMONPSV_TEMP:
		RetValue = XSYSMONPSV_DEVICE_TEMP_MAX;
		break;
	case XSYSMONPSV_TEMP_MAX:
		RetValue = XSYSMONPSV_DEVICE_TEMP_MAX_MAX;
		break;
	case XSYSMONPSV_TEMP_MIN:
		RetValue = XSYSMONPSV_DEVICE_TEMP_MIN_MIN;
		break;
	default:
		RetValue = -XST_FAILURE;
		break;
	}
	return RetValue;
}

/****************************************************************************/
/**
*
* Provides the temperature threshold (upper or lower) offset.
*
* @param	Event Temp or OT event type.
* @param	Dir Falling or rising direction
* @param	Offset Upper or lower threshold offset.
*
* @return	- -XST_FAILURE when fails
*		    - XST_SUCCESS when correct params are used.
*
***************************************************************************/
int XSysMonPsv_TempThreshOffset(XSysMonPsv_TempEvt Event,
				XSysMonPsv_EventDir Dir, u32 *Offset)
{
	int RetValue = XST_SUCCESS;
	switch (Event) {
	case XSYSMONPSV_TEMP_EVENT:
		*Offset = (Dir == XSYSMONPSV_EV_DIR_RISING) ?
				  XSYSMONPSV_DEVICE_TEMP_TH_RISING :
				  XSYSMONPSV_DEVICE_TEMP_TH_FALLING;
		break;
	case XSYSMONPSV_OT_EVENT:
		*Offset = (Dir == XSYSMONPSV_EV_DIR_RISING) ?
				  XSYSMONPSV_OT_TEMP_TH_RISING :
				  XSYSMONPSV_OT_TEMP_TH_FALLING;
		break;
	default:
		RetValue = -XST_FAILURE;
		break;
	}
	return RetValue;
}

/****************************************************************************/
/**
*
* Provides the supply offset.
*
* @param	InstancePtr Pointer to the XSysMonPsv instance.
* @param	Supply Enum which indicates the desired supply.
*
* @return	Offset supply.
*
***************************************************************************/
u32 XSysMonPsv_SupplyOffset(XSysMonPsv *InstancePtr, int Supply)
{
	u32 Offset;
	u8 SupplyReg;

	/* Assert the input arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Offset = XSYSMONPSV_SUPPLY;

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset += ((u32)SupplyReg * 4U);
	return Offset;
}

/****************************************************************************/
/**
*
* Provides the supply threshold (upper or lower) offset.
*
* @param	InstancePtr Pointer to the XSysMonPsv instance.
* @param    Supply Enum which indicates the desired supply.
* @param	Dir Type of the direction - falling or rising.
*
* @return	Upper or lower threshold offset.
*
***************************************************************************/
u32 XSysMonPsv_SupplyThreshOffset(XSysMonPsv *InstancePtr, int Supply,
				  XSysMonPsv_EventDir Dir)
{
	u32 Offset;
	u8 SupplyReg;

	/* Assert the input arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (Dir == XSYSMONPSV_EV_DIR_RISING) {
		Offset = XSYSMONPSV_SUPPLY_TH_UPPER;
	} else {
		Offset = XSYSMONPSV_SUPPLY_TH_LOWER;
	}
	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset += ((u32)SupplyReg * 4U);

	return Offset;
}

/****************************************************************************/
/**
*
* Converts Q8P7 To Celsius.
*
* @param	RawData Raw temperature Data.
* @param	Val Value in Q8P7 format.
* @param	Val2 Constant 128.
*
* @return	None.
*
***************************************************************************/
void XSysMonPsv_Q8P7ToCelsius(u32 RawData, int *Val, int *Val2)
{
	*Val = (RawData & 0x8000) ? -(twoscomp(RawData)) : RawData;
	*Val2 = 128;
}

/****************************************************************************/
/**
*
* Converts Celsius To Q8P7.
*
* @param	RawData Raw Temperature Data.
* @param	Val Numerator for covnersion to deg C.
* @param	Val2 Denominator for conversion to deg C in Q8.7 format.
*
* @return	None.
*
***************************************************************************/
void XSysMonPsv_CelsiusToQ8P7(u32 *RawData, int Val, int Val2)
{
	int Scale = 1 << 7;

	Val2 = Val2 / 1000;
	*RawData = (u32)((Val * Scale) + ((Val2 * Scale) / 1000));
}

/****************************************************************************/
/**
*
* Converts raw data to processed.
*
* @param	RawData Raw voltage data.
* @param	Val Numerator for conversion to volts.
* @param	Val2 Denominator for conversion to volts.
*
* @return	None.
*
***************************************************************************/
void XSysMonPsv_SupplyRawToProcessed(int RawData, int *Val, int *Val2)
{
	int Mantissa, Format, Exponent;

	Mantissa = RawData & XSYSMONPSV_SUPPLY_MANTISSA_MASK;
	Exponent = (RawData & XSYSMONPSV_SUPPLY_MODE_MASK) >>
		   XSYSMONPSV_SUPPLY_MODE_SHIFT;
	Format = (RawData & XSYSMONPSV_SUPPLY_FMT_MASK) >>
		 XSYSMONPSV_SUPPLY_FMT_SHIFT;

	*Val2 = 1 << (16 - Exponent);
	*Val = Mantissa;
	if (Format && (Mantissa >> XSYSMONPSV_SUPPLY_MANTISSA_SIGN)) {
		*Val = (~(Mantissa)&XSYSMONPSV_SUPPLY_MANTISSA_MASK) * -1;
	}
}

/****************************************************************************/
/**
*
* Converts processed data to raw.
*
* @param	Val Numerator for conversion to volts.
* @param	Val2 Denominator for conversion to volts.
* @param	RegVal Register Value.
* @param	RawData Converted Raw data.
*
* @return	None.
*
***************************************************************************/
void XSysMonPsv_SupplyProcessedToRaw(int Val, int Val2, u32 RegVal,
				     u32 *RawData)
{
	int Exponent = (RegVal & XSYSMONPSV_SUPPLY_MODE_MASK) >>
		       XSYSMONPSV_SUPPLY_MODE_SHIFT;
	int Format = (RegVal & XSYSMONPSV_SUPPLY_FMT_MASK) >>
		     XSYSMONPSV_SUPPLY_FMT_SHIFT;
	int Scale = 1 << (16 - Exponent);
	int Tmp;

	Val2 = Val2 / 1000;
	Tmp = (Val * Scale) + ((Val2 * Scale) / 1000);

	/* Set out of bound values to saturation levels */
	if (Format) {
		if (Tmp > XSYSMONPSV_UP_SAT_SIGNED) {
			Tmp = XSYSMONPSV_UP_SAT_SIGNED_VAL;
		} else if (Tmp < XSYSMONPSV_LOW_SAT_SIGNED) {
			Tmp = XSYSMONPSV_LOW_SAT_SIGNED_VAL;
		}
	} else {
		if (Tmp > XSYSMONPSV_UP_SAT) {
			Tmp = XSYSMONPSV_UP_SAT_VAL;
		} else if (Tmp < XSYSMONPSV_LOW_SAT) {
			Tmp = XSYSMONPSV_LOW_SAT_VAL;
		}
	}

	*RawData = Tmp & 0xFFFF;
}

/*****************************************************************************/
/**
*
* Checks if the supply value has exceeded the set
* threshold values.
*
* @param        InstancePtr Pointer to the driver instance.
* @param        Supply Enum which indicates the desired supply.
*
* @return       True if new data available.
*               False if new data is not available.
*               Invalid if the Supply has not been configured.
*
******************************************************************************/
u32 XSysMonPsv_IsAlarmPresent(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply)
{
	u8 Offset;
	u8 Shift;
	u8 SupplyReg;
	u32 Status;

	/* Assert the input arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] ==
	    XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset = 4U * (SupplyReg / 32U);
	Shift = SupplyReg % 32U;

	/* Read the New data flag */
	XSysMonPsv_ReadReg32(InstancePtr, Offset + XSYSMONPSV_ALARM_FLAG0,
			    &Status);

	Status &= ((u32)1U << Shift);

	return ((Status > 0U) ? 1U : 0U);
}

/*****************************************************************************/
/**
*
* Clears alarm status.
*
* @param        InstancePtr Pointer to the driver instance.
* @param        Supply Enum which indicates the desired supply.
*
* @return       True if new data available.
*               False if new data isn't available.
*               Invalid if the Supply hasn't been configured.
*
******************************************************************************/
u32 XSysMonPsv_ClearAlarm(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply)
{
	u8 Offset;
	u8 Shift;
	u8 SupplyReg;
	u32 Status;

	/* Assert the input arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] ==
	    XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset = 4U * (SupplyReg / 32U);
	Shift = SupplyReg % 32U;

	/* Read the New data flag */
	XSysMonPsv_ReadReg32(InstancePtr, Offset + XSYSMONPSV_ALARM_FLAG0,
			    &Status);

	Status &= ((u32)1U << Shift);

	/* Clear the New data flag if its set */
	XSysMonPsv_WriteReg32(InstancePtr, Offset + XSYSMONPSV_ALARM_FLAG0,
			     Status);

	return (u32)XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Enables the specified interrupts in the device.
*
* @param	InstancePtr Pointer to the driver instance.
* @param	Mask 32 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMON_IER_*  bits defined in InstancePtr.h.
* @param	IntrNum Interrupt enable register to be used
*
* @return	- XST_FAILURE if error
*		    - XST_SUCCESS if successful
*
*****************************************************************************/
int XSysMonPsv_InterruptEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	if (InstancePtr == NULL) {
		return -XST_FAILURE;
	}

	/* Calculate the offset of the IER register to be written to */
	Offset = (XSYSMONPSV_IER0_OFFSET +
		  ((u32)IntrNum * XSYSMONPSV_PCSR_LOCK));

	/* Enable the specified interrupts in the AMS Interrupt Enable Register. */
	XSysMonPsv_WriteReg32(InstancePtr, Offset, Mask);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Disables the specified interrupts in the device.
*
* @param	InstancePtr Pointer to the driver instance.
* @param	Mask 32 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMONPSV_IDR_*  bits defined in InstancePtr.h.
* @param	IntrNum Interrupt disable register to be used
*
* @return	- XST_FAILURE if error
*		    - XST_SUCCESS if successful
*
*****************************************************************************/
int XSysMonPsv_InterruptDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	if (InstancePtr == NULL) {
		return -XST_FAILURE;
	}

	/* Calculate the offset of the IDR register to be written to */
	Offset = (XSYSMONPSV_IDR0_OFFSET +
		  ((u32)IntrNum * XSYSMONPSV_PCSR_LOCK));

	/* Disable the specified interrupts in the AMS Interrupt Disable Register. */
	XSysMonPsv_WriteReg32(InstancePtr, Offset, Mask);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Returns the interrupt status read from Interrupt Status
* Register(ISR). Use the XSYSMONPSV_ISR* constants defined in InstancePtr.h
* to interpret the returned value.
*
* @param	InstancePtr Pointer to the driver instance.
* @param	IntrStatus 32-bit value representing the contents of the Interrupt Status
*		Register (ISR).
*
* @return	- -XST_FAILURE when NULL Instance is passed
*           - XST_SUCCESS if succeeds
*
*****************************************************************************/
int XSysMonPsv_InterruptGetStatus(XSysMonPsv *InstancePtr, u32 *IntrStatus)
{
	if (InstancePtr == NULL) {
		return -XST_FAILURE;
	}
	/* Return the value read from the AMS ISR. */
	XSysMonPsv_ReadReg32(InstancePtr, XSYSMONPSV_ISR_OFFSET, IntrStatus);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Clears the specified interrupts in the Interrupt Status
* Register(ISR).
*
* @param	InstancePtr Pointer to the struct InstancePtr.
* @param	Mask 32 bit-mask of the interrupts to be cleared.
*		Bit positions of 1 will be cleared. Bit positions of 0 will not
*		change the previous interrupt status.*
* @return	None.
*
*****************************************************************************/
void XSysMonPsv_InterruptClear(XSysMonPsv *InstancePtr, u32 Mask)
{
	/* Assert the input arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_ISR_OFFSET, Mask);
}

/****************************************************************************/
/**
*
* Unlocks the register space of InstancePtr hardware.
*
* @param	InstancePtr Pointer to the driver instance.
*
* @return	None.
*
*****************************************************************************/
void XSysMonPsv_UnlockRegspace(XSysMonPsv *InstancePtr)
{
	/* Assert the input arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_PCSR_LOCK,
			     XSYSMONPSV_LOCK_CODE);
}
