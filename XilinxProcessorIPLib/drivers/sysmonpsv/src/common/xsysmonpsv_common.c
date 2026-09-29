/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_common.c
*
* @brief Implements the basic driver functions for the XSysMonPsv driver.
*
* The xsysmonpsv_common.c file implements basic driver functions used in the
* services or directly by the application.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 4.0   se     10/04/22 Update return value definitions
*       se     10/27/22 Secure and Non-Secure mode integration
* 5.1   se     03/03/25 Compiler warnings fixed
* 5.3   dc     02/18/26 Correct spelling errors
*       se     03/12/26 Enhancements: NULL checks, range validation,
*                       standardize return values to XST_FAILURE
*       se     03/25/26 Fix Coverity/MISRA-C violations: U suffix,
*                       (void) cast, sign conversion, parentheses
* 5.4   vn     07/30/26 Fix Doxygen warnings
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv_lowlevel.h"
#include "xsysmonpsv_common.h"
#include "xil_util.h"

/****************************************************************************/
/**
*
* @brief Provides the register offset for the given type.
*
* @param[in]   Type Max or Min temperature type.
*
* @return      - -XST_FAILURE when it fails.
*              - Register offset otherwise.
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
* @brief Provides the temperature upper or lower threshold offset.
*
* @param[in]   Event Temp or OT event type.
* @param[in]   Dir Falling or rising direction.
* @param[out]  Offset Upper or lower threshold offset.
*
* @return      - XST_FAILURE when it fails.
*              - XST_SUCCESS when correct params are used.
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
		RetValue = XST_FAILURE;
		break;
	}
	return RetValue;
}

/****************************************************************************/
/**
*
* @brief Provides the supply offset.
*
* @param[in]   InstancePtr Pointer to the XSysMonPsv instance.
* @param[in]   Supply Enum which indicates the desired supply.
*
* @return      Supply offset.
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
* @brief Provides the supply upper or lower threshold offset.
*
* @param[in]   InstancePtr Pointer to the XSysMonPsv instance.
* @param[in]   Supply Enum which indicates the desired supply.
* @param[in]   Dir Type of the direction, falling or rising.
*
* @return      Upper or lower threshold offset.
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
* @brief Converts Q8P7 format to Celsius.
*
* @param[in]   RawData Raw temperature data.
* @param[out]  Val Value in Q8P7 format.
* @param[out]  Val2 Constant 128.
*
***************************************************************************/
void XSysMonPsv_Q8P7ToCelsius(u32 RawData, int *Val, int *Val2)
{
	*Val = (RawData & 0x8000U) ? -(int)twoscomp(RawData) : (int)RawData;
	*Val2 = 128;
}

/****************************************************************************/
/**
*
* @brief Converts Celsius to Q8P7 format.
*
* @param[out]  RawData Raw temperature data.
* @param[in]   Val Numerator for conversion to degrees Celsius.
* @param[in]   Val2 Denominator for conversion to degrees Celsius in Q8.7
*              format.
*
***************************************************************************/
void XSysMonPsv_CelsiusToQ8P7(u32 *RawData, int Val, int Val2)
{
	int Scale = (1 << 7);

	Val2 = Val2 / 1000;
	*RawData = (u32)((Val * Scale) + ((Val2 * Scale) / 1000));
}

/****************************************************************************/
/**
*
* @brief Converts raw data to processed data.
*
* @param[in]   RawData Raw voltage data.
* @param[out]  Val Numerator for conversion to volts.
* @param[out]  Val2 Denominator for conversion to volts.
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
		*Val = -(int)(~(Mantissa) & XSYSMONPSV_SUPPLY_MANTISSA_MASK);
	}
}

/****************************************************************************/
/**
*
* @brief Converts processed data to raw data.
*
* @param[in]   Val Numerator for conversion to volts.
* @param[in]   Val2 Denominator for conversion to volts.
* @param[in]   RegVal Register value.
* @param[out]  RawData Converted raw data.
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

	*RawData = Tmp & 0xFFFFU;
}

/*****************************************************************************/
/**
*
* @brief Checks if the supply value has exceeded the set threshold values.
*
* @param[in]   InstancePtr Pointer to the driver instance.
* @param[in]   Supply Enum which indicates the desired supply.
*
* @return      True if new data available.
*              False if new data is not available.
*              Invalid if the supply is not configured.
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
* @brief Clears the alarm status.
*
* @param[in]   InstancePtr Pointer to the driver instance.
* @param[in]   Supply Enum which indicates the desired supply.
*
* @return      XST_SUCCESS if the alarm was cleared.
*              XSYSMONPSV_INVALID if the supply is not configured.
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
* @brief Enables the specified interrupts in the device.
*
* @param[in]   InstancePtr Pointer to the driver instance.
* @param[in]   Mask 32-bit mask of the interrupts to be enabled.
*              Bit positions of 1 are enabled. Bit positions of 0
*              keep the previous setting. This mask is formed by OR'ing
*              XSYSMON_IER_*  bits defined in xsysmonpsv_hw.h.
* @param[in]   IntrNum Interrupt enable register to be used.
*
* @return      - XST_FAILURE if error.
*              - XST_SUCCESS if successful.
*
*****************************************************************************/
int XSysMonPsv_InterruptEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
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
* @brief Disables the specified interrupts in the device.
*
* @param[in]   InstancePtr Pointer to the driver instance.
* @param[in]   Mask 32-bit mask of the interrupts to be disabled.
*              Bit positions of 1 are disabled. Bit positions of 0
*              keep the previous setting. This mask is formed by OR'ing
*              XSYSMONPSV_IDR_*  bits defined in xsysmonpsv_hw.h.
* @param[in]   IntrNum Interrupt disable register to be used.
*
* @return      - XST_FAILURE if error.
*              - XST_SUCCESS if successful.
*
*****************************************************************************/
int XSysMonPsv_InterruptDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
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
* @brief Returns the interrupt status read from the Interrupt Status Register
* (ISR).
*
* Use the XSYSMONPSV_ISR* constants defined in xsysmonpsv_hw.h to interpret
* the returned value.
*
* @param[in]   InstancePtr Pointer to the driver instance.
* @param[out]  IntrStatus 32-bit value representing the contents of the
*              Interrupt Status Register (ISR).
*
* @return      - XST_FAILURE when a NULL instance is passed.
*              - XST_SUCCESS if it succeeds.
*
*****************************************************************************/
int XSysMonPsv_InterruptGetStatus(XSysMonPsv *InstancePtr, u32 *IntrStatus)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}
	/* Return the value read from the AMS ISR. */
	XSysMonPsv_ReadReg32(InstancePtr, XSYSMONPSV_ISR_OFFSET, IntrStatus);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* @brief Clears the specified interrupts in the Interrupt Status Register.
*
* @param[in]   InstancePtr Pointer to the driver instance.
* @param[in]   Mask 32-bit mask of the interrupts to be cleared.
*              Bit positions of 1 are cleared. Bit positions of 0 do not
*              change the previous interrupt status.
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
* @brief Unlocks the register space of the SysMon hardware.
*
* @param[in]   InstancePtr Pointer to the driver instance.
*
*****************************************************************************/
void XSysMonPsv_UnlockRegspace(XSysMonPsv *InstancePtr)
{
	/* Assert the input arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_PCSR_LOCK,
			     XSYSMONPSV_LOCK_CODE);
}

/** @} */
