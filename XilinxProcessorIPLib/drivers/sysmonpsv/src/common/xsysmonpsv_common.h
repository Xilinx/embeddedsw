/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xsysmonpsv_common.h
* @addtogroup Overview
*
* Functions in this file are basic driver functions which will be used in the
* in servies or directly by the user.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
*
* </pre>
*
******************************************************************************/
#ifndef _XSYSMONPSV_COMMON_H_
#define _XSYSMONPSV_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xsysmonpsv_driver.h"
#include "xsysmonpsv_supplylist.h"

/************************** Constant Definitions *****************************/

#define XSYSMONPSV_SUCCESS 0 /**< Success value return for functions */
#define XSYSMONPSV_EINVAL 1 /**< Invalid value return for functions */
#define XSYSMONPSV_INTR_OFFSET 0xCU /**< Interrupt Offset */
#define XSYSMONPSV_LOCK_CODE 0xF9E8D7C6 /**< Lock code value */
#define XSYSMONPSV_INTR_0_ID (144U + 32U) /**< Interrupt unique ID */
#define XSYSMONPSV_INTR_1_ID (145U + 32U) /**< Interrupt unique ID */

#define XSYSMONPSV_BIT_ALARM0 0U /**< Bit for supply[0-31]*/
#define XSYSMONPSV_BIT_ALARM1 1U /**< Bit for supply[32-63]*/
#define XSYSMONPSV_BIT_ALARM2 2U /**< Bit for supply[64-95]*/
#define XSYSMONPSV_BIT_ALARM3 3U /**< Bit for supply[96-127]*/
#define XSYSMONPSV_BIT_ALARM4 4U /**< Bit for supply[128-159]*/
#define XSYSMONPSV_BIT_ALARM5 5U /**< Reserved */
#define XSYSMONPSV_BIT_ALARM6 6U /**< Reserved */
#define XSYSMONPSV_BIT_ALARM7 7U /**< Reserved */
#define XSYSMONPSV_BIT_OT 8U /**< Over Temperature bit*/
#define XSYSMONPSV_BIT_TEMP 9U /**< Temperature bit*/

#define XSYSMONPSV_UP_SAT_SIGNED_VAL                                           \
	0x7FFF /**< Signed upper saturation
						  value */
#define XSYSMONPSV_LOW_SAT_SIGNED_VAL                                          \
	0x8000 /**< Signed lower saturation
						  value */

#define XSYSMONPSV_UP_SAT_VAL 0xFFFF /**< Upper saturation value */
#define XSYSMONPSV_LOW_SAT_VAL 0x0000 /**< Upper saturation value */

#define compare(val, thresh)                                                   \
	(((val)&0x8000) || ((thresh)&0x8000) ?                                 \
		 ((val) < (thresh)) :                                          \
		 ((val) > (thresh))) /**< Macro to compare threshold
									  with current value */

#define twoscomp(val)                                                          \
	((((val) ^ 0xFFFF) + 1) & 0x0000FFFF) /**< Macro for 2's compliment */
#define ALARM_REG(address)                                                     \
	((address) / 32U) /**< Alarm Register offet for supply */
#define ALARM_SHIFT(address)                                                   \
	((address) % 32U) /**< Supply bit in Alarm Register */
#define GET_BIT(nr) (1UL << (nr)) /**< Macro for bit shifter */

typedef enum {
	XSYSMONPSV_EV_DIR_EITHER, /**< Rising or falling both direction */
	XSYSMONPSV_EV_DIR_RISING, /**< Rising both direction */
	XSYSMONPSV_EV_DIR_FALLING, /**< Falling both direction */
	XSYSMONPSV_EV_DIR_NONE, /**< No direction */
} XSysMonPsv_EventDir;

typedef enum {
	XSYSMONPSV_TEMP, /**< Current temperature */
	XSYSMONPSV_TEMP_MAX, /**< Maximum temperature reached since reset */
	XSYSMONPSV_TEMP_MIN, /**< Minimum temperature reached since reset */
} XSysMonPsv_TempType;

typedef enum {
	XSYSMONPSV_TEMP_EVENT, /**< Temperature event */
	XSYSMONPSV_OT_EVENT, /**< Over Temperature event */
} XSysMonPsv_TempEvt;

/************************** Function Prototypes ******************************/

/* Functions in xsysmonpsv_common.c */
int XSysMonPsv_TempOffset(XSysMonPsv_TempType Type);
int XSysMonPsv_TempThreshOffset(XSysMonPsv_TempEvt Event,
				XSysMonPsv_EventDir Dir, u32 *Offset);
u32 XSysMonPsv_SupplyOffset(XSysMonPsv *InstancePtr, int Supply);
u32 XSysMonPsv_SupplyThreshOffset(XSysMonPsv *InstancePtr, int Supply,
				  XSysMonPsv_EventDir Dir);
void XSysMonPsv_Q8P7ToCelsius(u32 RawData, int *Val, int *Val2);
void XSysMonPsv_CelsiusToQ8P7(u32 *RawData, int Val, int Val2);
void XSysMonPsv_SupplyRawToProcessed(int RawData, int *Val, int *Val2);
void XSysMonPsv_SupplyProcessedToRaw(int Val, int Val2, u32 RegVal,
				     u32 *RawData);

u32 XSysMonPsv_IsAlarmPresent(XSysMonPsv *InstancePtr,
			      XSysMonPsv_Supply Supply);
u32 XSysMonPsv_ClearAlarm(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply);

int XSysMonPsv_InterruptEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
int XSysMonPsv_InterruptDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
int XSysMonPsv_InterruptGetStatus(XSysMonPsv *InstancePtr, u32 *IntrStatus);
void XSysMonPsv_InterruptClear(XSysMonPsv *InstancePtr, u32 Mask);
void XSysMonPsv_UnlockRegspace(XSysMonPsv *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif /* _XSYSMONPSV_COMMON_H_ */
