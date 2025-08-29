/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xsysmonpsv.h
* @addtogroup sysmonpsv_api SYSMONPSV APIs
* @{
*
*
*
*
* <br><br>
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    02/08/18 First release
* 1.2   aad    06/14/20 Fixed temperature calculation for negative temp
* 2.0   aad    07/31/20 Added new APIs to set threshold values, alarm
*                       config and modes for temperature and voltages.
*                       Added new interrupt handling structure.
*       aad    10/12/20 Fixed MISRAC violations
* 2.1   aad    02/24/21 Added additional documentation to support production
*                       silicon.
* 2.2   aad    03/29/21 Added an array that contains the supply names in
*                       strings.
* 2.3   aad    04/30/21 Size optimization for PLM code.
* 2.3   aad    07/26/21 Added doxygen comments.
* 2.3   aad    09/01/21 Fixed compilation warning.
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
* 4.0   se     10/04/22 Update return value definitions
* 5.0   se     08/01/24 Added new APIs to enable, set and get averaging for
*                       voltage supplies and temperature satellites.
* 5.2   se     08/24/25 Microblaze support added
*
* </pre>
*
******************************************************************************/

#ifndef XSYSMONPSV_H_ /**< prevent circular inclusions */
#define XSYSMONPSV_H_ /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv_common.h"
#include "xsysmonpsv_services.h"
#if defined (ARMR5) || defined (__aarch64__)
#include "xscugic.h"
#endif
#if defined (PLATFORM_MB) && defined (XIL_INTERRUPT) && defined (SDT)
#include "xinterrupt_wrap.h"
#endif
/**************************** Type Definitions *******************************/

/*@}*/

/**
 * @brief This typedef defines Threshold types.
 * @{
 */
typedef enum {
	XSYSMONPSV_TH_LOWER, /**< Lower Threshold */
	XSYSMONPSV_TH_UPPER, /**< Upper Threshold */
} XSysMonPsv_Threshold;

/*@}*/

/**
 * @name This array contains the names of the supplies
 * @{
 */
extern const char *XSysMonPsv_Supply_Arr[]; /**< Names of the supplies
                                                enabled in the design */
/*@}*/

/**
 * @brief This typedef defines value types for voltage readings.
 * @{
 */
typedef enum {
	XSYSMONPSV_VAL, /**< Current Value for temperature
                                         (for production silicon only)
                                          and supply */
	XSYSMONPSV_VAL_MIN, /**< Minimum Value reached since
                                          reset */
	XSYSMONPSV_VAL_MAX, /**< Maximum Value reached since
                                          reset */
	XSYSMONPSV_VAL_VREF_MIN, /**< Minimum value reached for a
                                          temperature since last VRef */
	XSYSMONPSV_VAL_VREF_MAX, /**< Maximum value reached for a
                                          temperature since last VRef */
} XSysMonPsv_Val;

/*@}*/

/**
 * @brief This typedef defines types of supply values.
 * @{
 */
typedef enum {
	XSYSMONPSV_1V_UNIPOLAR, /**< 1V unipolar fmt */
	XSYSMONPSV_2V_UNIPOLAR, /**< 2V unipolar fmt */
	XSYSMONPSV_4V_UNIPOLAR, /**< 4V unipolar fmt */
	XSYSMONPSV_1V_BIPOLAR, /**< 1V bipolar fmt */
} XSysMonPsv_VoltageScale;
/*@}*/

/************************* Variable Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
*
* This macro returns the XSYSMONPSV_NEW_ALARMn_MASK for a configured supply.
*
* @param        InstancePtr Instance of sysmon driver
* @param        Supply Type enum of supply enabled
*
* @return       A 32 bit mask to be used for configuring interrupts
*
*
*****************************************************************************/
#define XSysMonPsv_GetAlarmMask(InstancePtr, Supply)                           \
	Mask = 1U << (InstancePtr->Supply_List[Supply] / 32U)

/****************************************************************************/
/**
*
* Converts raw AdcData into Voltage value.
*
* @param        AdcData System Monitor ADC Raw Data.
*
* @return       The Voltage in volts.
*
*
*****************************************************************************/
static inline float XSysMonPsv_RawToVoltage(u32 AdcData)
{
	u32 Mantissa, Scale, Format, Exponent;

	Mantissa = AdcData & XSYSMONPSV_SUPPLY_MANTISSA_MASK;
	Exponent = (AdcData & XSYSMONPSV_SUPPLY_MODE_MASK) >>
		   XSYSMONPSV_SUPPLY_MODE_SHIFT;
	Format = (AdcData & XSYSMONPSV_SUPPLY_FMT_MASK) >>
		 XSYSMONPSV_SUPPLY_FMT_SHIFT;

	/* Calculate the exponent
         * 2^(16-Exponent)
         */
	Scale = ((u32)1U << (XSYSMONPSV_EXPONENT_RANGE_16 - Exponent));
	if ((Format & (Mantissa >> XSYSMONPSV_SUPPLY_MANTISSA_SIGN)) == 1U) {
		return ((float)Mantissa / (float)Scale) - 1.0f;
	} else {
		return (float)Mantissa / (float)Scale;
	}
}

/****************************************************************************/
/**
*
* Converts raw AdcData into Voltage value.
*
* @param        Volts Voltage value to be converted.
* @param        Type Type of supply:
*               Type = 0, Unipolar
*               Type = 1, Bipolar
*
* @return       The Voltage in Raw ADC format.
*
*
*****************************************************************************/
static inline u16 XSysMonPsv_VoltageToRaw(float Volts,
					  XSysMonPsv_VoltageScale Type)
{
	u32 Format = 0;
	u32 Exponent = 16U;
	u32 Scale;
	int TmpVal;
	float TmpFloat;

	if (Type != XSYSMONPSV_1V_BIPOLAR) {
		Exponent -= (u32)Type;
	} else {
		Format = 1U;
	}

	Scale = ((u32)1U << (16U - Exponent));
	TmpFloat = (Volts * (float)Scale);

	TmpVal = (int)TmpFloat;

	if (Format == 1U) {
		if (TmpVal > XSYSMONPSV_UP_SAT_SIGNED) {
			TmpVal = XSYSMONPSV_BIPOLAR_UP_SAT;
		} else if (TmpVal < XSYSMONPSV_LOW_SAT_SIGNED) {
			TmpVal = XSYSMONPSV_BIPOLAR_LOW_SAT;
		}
	} else {
		if (TmpVal > XSYSMONPSV_UP_SAT) {
			TmpVal = XSYSMONPSV_UNIPOLAR_UP_SAT;
		} else if (TmpVal < XSYSMONPSV_LOW_SAT) {
			TmpVal = XSYSMONPSV_UNIPOLAR_LOW_SAT;
		}
	}

	return (u16)((u32)TmpVal & 0xFFFFU);
}

/****************************************************************************/
/**
*
* Converts the fixed point to degree celsius.
*
* @param        FixedQFmt Q8.7 representation of temperature value.
*
* @return       The Temperature in degree celsisus.
*
*
*****************************************************************************/
static inline float XSysMonPsv_FixedToFloat(u32 FixedQFmt)
{
	u32 TwosComp;
	float Temperature;
	if ((FixedQFmt >> XSYSMONPSV_QFMT_SIGN) > 0U) {
		TwosComp = (~(FixedQFmt) + 1U) & (0x000FFFFU);
		Temperature = (float)(TwosComp) /
			      ((float)XSYSMONPSV_QFMT_FRACTION * (-1.0f));
	} else {
		Temperature = (float)FixedQFmt /
			      ((float)XSYSMONPSV_QFMT_FRACTION * (1.0f));
	}
	return Temperature;
}

/****************************************************************************/
/**
*
* Converts the floating point to Fixed Q8.7 format.
*
* @param        Temp Temperature value in Deg Celsius.
*
* @return       Temperature value in fixed Q8.7 format.
*
*
*****************************************************************************/
static inline u16 XSysMonPsv_FloatToFixed(float Temp)
{
	float ScaledDown;
	int RawAdc;

	ScaledDown = (float)(Temp * 128.0f);
	RawAdc = (int)ScaledDown;

	return (u16)RawAdc;
}

/************************** Function Prototypes ******************************/

/* Functions in xsysmonpsv.c */
s64 XSysMonPsv_CfgInitialize(XSysMonPsv *InstancePtr,
			     XSysMonPsv_Config *CfgPtr);
void XSysMonPsv_SystemReset(XSysMonPsv *InstancePtr);
void XSysMonPsv_EnRegGate(XSysMonPsv *InstancePtr, u8 Enable);
void XSysMonPsv_SetPMBusAddress(XSysMonPsv *InstancePtr, u8 Address);
void XSysMonPsv_PMBusEnable(XSysMonPsv *InstancePtr, u8 Enable);
void XSysMonPsv_PMBusEnableCmd(XSysMonPsv *InstancePtr, u8 Enable);
void XSysMonPsv_SelectExtInterface(XSysMonPsv *InstancePtr, u8 Interface);
void XSysMonPsv_StatusReset(XSysMonPsv *InstancePtr, u8 ResetSupply,
			    u8 ResetTemperature);
u16 XSysMonPsv_ReadDevTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType);
void XSysMonPsv_SetDevTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType,
				    u16 Value);
u16 XSysMonPsv_ReadOTTempThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Threshold ThresholdType);
void XSysMonPsv_SetOTTempThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Threshold ThresholdType,
				   u16 Value);
u32 XSysMonPsv_ReadDeviceTemp(XSysMonPsv *InstancePtr, XSysMonPsv_Val Value);
u32 XSysMonPsv_ReadSupplyThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Supply Supply,
				   XSysMonPsv_Threshold ThresholdType);
u32 XSysMonPsv_ReadSupplyValue(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply, XSysMonPsv_Val Value);
u32 XSysMonPsv_IsNewData(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply);
u32 XSysMonPsv_IsAlarmCondition(XSysMonPsv *InstancePtr,
				XSysMonPsv_Supply Supply);
u32 XSysMonPsv_SetSupplyUpperThreshold(XSysMonPsv *InstancePtr,
				       XSysMonPsv_Supply Supply, u32 Value);
u32 XSysMonPsv_SetSupplyLowerThreshold(XSysMonPsv *InstancePtr,
				       XSysMonPsv_Supply Supply, u32 Value);
void XSysMonPsv_SetTempMode(XSysMonPsv *InstancePtr, u32 Mode);
void XSysMonPsv_SetOTMode(XSysMonPsv *InstancePtr, u32 Mode);
u32 XSysMonPsv_ReadAlarmConfig(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply);
u32 XSysMonPsv_SetAlarmConfig(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply,
			      u32 Config);
int XSysMonPsv_ReadTempProcessed(XSysMonPsv *InstancePtr,
				 XSysMonPsv_TempType Type, float *Val);
int XSysMonPsv_ReadTempRaw(XSysMonPsv *InstancePtr, XSysMonPsv_TempType Type,
			   u32 *Val);
int XSysMonPsv_ReadTempProcessedSat(XSysMonPsv *InstancePtr, int SatId,
				    float *Val);
int XSysMonPsv_ReadTempRawSat(XSysMonPsv *InstancePtr, int SatId, u32 *Val);
int XSysMonPsv_SetTempThresholdUpper(XSysMonPsv *InstancePtr,
				     XSysMonPsv_TempEvt Event, u32 Val);
int XSysMonPsv_SetTempThresholdLower(XSysMonPsv *InstancePtr,
				     XSysMonPsv_TempEvt Event, u32 Val);
int XSysMonPsv_GetTempThresholdUpper(XSysMonPsv *InstancePtr,
				     XSysMonPsv_TempEvt Event, u32 *Val);
int XSysMonPsv_GetTempThresholdLower(XSysMonPsv *InstancePtr,
				     XSysMonPsv_TempEvt Event, u32 *Val);
int XSysMonPsv_ReadSupplyProcessed(XSysMonPsv *InstancePtr, int Supply,
				   float *Val);
int XSysMonPsv_ReadSupplyRaw(XSysMonPsv *InstancePtr, u32 Supply, u32 *Val);
int XSysMonPsv_SetSupplyThresholdUpper(XSysMonPsv *InstancePtr, u32 Supply,
				       u32 Val);
int XSysMonPsv_SetSupplyThresholdLower(XSysMonPsv *InstancePtr, int Supply,
				       u32 Val);
int XSysMonPsv_GetSupplyThresholdUpper(XSysMonPsv *InstancePtr, u32 Supply,
				       u32 *Val);
int XSysMonPsv_GetSupplyThresholdLower(XSysMonPsv *InstancePtr, u32 Supply,
				       u32 *Val);
void XSysMonPsv_EnableTempAverage(XSysMonPsv *InstancePtr, int SatId, u8 Enable);
void XSysMonPsv_SetTempAverageRate(XSysMonPsv *InstancePtr, u8 AverageRate);
int XSysMonPsv_GetTempAverageRate(XSysMonPsv *InstancePtr, u8 *AverageRate);
int XSysMonPsv_EnableSupplyAverage(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Supply Supply, u8 Enable);
void XSysMonPsv_SetSupplyAverageRate(XSysMonPsv *InstancePtr, u8 AverageRate);
int XSysMonPsv_GetSupplyAverageRate(XSysMonPsv *InstancePtr, u8 *AverageRate);

#if defined (ARMR5) || defined (__aarch64__) || defined  (PLATFORM_MB)
int XSysMonPsv_RegisterDeviceTempOps(XSysMonPsv *InstancePtr,
				     XSysMonPsv_Handler CallbackFunc,
				     void *CallbackRef);
int XSysMonPsv_UnregisterDeviceTempOps(XSysMonPsv *InstancePtr);
int XSysMonPsv_RegisterOTOps(XSysMonPsv *InstancePtr,
			     XSysMonPsv_Handler CallbackFunc,
			     void *CallbackRef);
int XSysMonPsv_UnregisterOTOps(XSysMonPsv *InstancePtr);
int XSysMonPsv_RegisterSupplyOps(XSysMonPsv *InstancePtr,
				 XSysMonPsv_Supply Supply,
				 XSysMonPsv_Handler CallbackFunc,
				 void *CallbackRef);
int XSysMonPsv_UnregisterSupplyOps(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Supply Supply);

int XSysMonPsv_Init(XSysMonPsv *InstancePtr, void *IntcInst);
#endif

/* Interrupt functions in xsysmonpsv_intr.c */
void XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
void XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
u32 XSysMonPsv_IntrGetEnabled(XSysMonPsv *InstancePtr, u8 IntrNum);
u32 XSysMonPsv_IntrGetStatus(XSysMonPsv *InstancePtr);
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask);
void XSysMonPsv_SetNewDataIntSrc(XSysMonPsv *InstancePtr,
				 XSysMonPsv_Supply Supply, u32 Mask);

#if defined (ARMR5) || defined (__aarch64__) || defined (PLATFORM_MB)
void XSysMonPsv_SetTempEventHandler(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Handler CallbackFunc,
				    void *CallbackRef);
void XSysMonPsv_SetOTEventHandler(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Handler CallbackFunc,
				  void *CallbackRef);
void XSysMonPsv_SetSupplyEventHandler(XSysMonPsv *InstancePtr,
				      XSysMonPsv_Supply Supply,
				      XSysMonPsv_Handler CallbackFunc,
				      void *CallbackRef);
void XSysMonPsv_AlarmEventHandler(XSysMonPsv *InstancePtr);
#endif
/* Functions in xsysmonpsv_sinit.c */
XSysMonPsv_Config *XSysMonPsv_LookupConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* XSYSMONPSV_H_ */
/** @} */
