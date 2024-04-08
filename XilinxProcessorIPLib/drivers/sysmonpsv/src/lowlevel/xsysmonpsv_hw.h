/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_hw.h
* @addtogroup sysmonpsv_api SYSMONPSV APIs
* @{
*
* The xsysmonpsv_hw.h header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    11/09/18 First release
* 1.1   aad    07/11/19 Fixed NEW_DATA_MASK
* 2.0   aad    10/12/20 MISRAC Violations
* 2.1   aad    02/24/21 Added additional macro to support production silicon.
* 2.3   aad    07/26/21 Added doxygen comments for Macros
*       aad    07/26/21 Removed unsed Macros
* 3.0   cog    03/25/21 Driver Restructure
* 5.0   se     08/01/24 Added new APIs to enable, set and get averaging for
*                       voltage supplies and temperature satellites.
*
* </pre>
*
******************************************************************************/

#ifndef XSYSMONPSV_HW_H_
#define XSYSMONPSV_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name XSYSMONPSV Base Address
 * @{
 */
#define XSYSMONPSV_BASEADDR 0XF1270000U /**< Sysmon Base Address */

/*@}*/

/**
 * @name Register: XSYSMONPSV_PCSR_MASK
 * @{
 */
#define XSYSMONPSV_PCSR_MASK 0X00000000U /**< Sysmon PCSR Mask Reg Offset */

#define XSYSMONPSV_PCSR_MASK_SYS_RST_MASK_SHIFT                                \
	15U /**< PCSR Mask
                                                          Reset Shift */
#define XSYSMONPSV_PCSR_MASK_SYS_RST_MASK_MASK 0X00038000U
/**< PCSR Mask Reset
                                                           Mask Value */
#define XSYSMONPSV_PCSR_MASK_GATEREG_SHIFT 1U /**< PCSR Mask Gate Reg Shift */
#define XSYSMONPSV_PCSR_MASK_GATEREG_MASK 0X00000002U
/**< PCSR Mask Gate Reg Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_PCSR_CONTROL
 * @{
 */
#define XSYSMONPSV_PCSR_CONTROL 0X00000004U /**< PCSR Control Reg Offset */

#define XSYSMONPSV_PCSR_CONTROL_SYS_RST_MASK_SHIFT                             \
	15U /**< PCSR Control Reset
                                                           Shift */
#define XSYSMONPSV_PCSR_CONTROL_SYS_RST_MASK_MASK 0X00038000U
/**< PCSR Control Reset
                                                           Mask */
#define XSYSMONPSV_PCSR_CONTROL_GATEREG_SHIFT                                  \
	1U /**< PCSR Control Gate Reg
                                                     Shift */
#define XSYSMONPSV_PCSR_CONTROL_GATEREG_MASK 0X00000002U
/**< PCSR Control Gate Reg
                                                     Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_PCSR_LOCK
 * @{
 */
#define XSYSMONPSV_PCSR_LOCK 0X0000000CU /**< PCSR Lock Register Offset */

#define XSYSMONPSV_PCSR_LOCK_STATE_SHIFT 0U /**< PCSR Lock State Shift */
#define XSYSMONPSV_PCSR_LOCK_STATE_MASK 0X00000001U
/**< PCSR Lock State Mask */
/*@}*/

#define XSYSMONPSV_INTR_MASK 0xFFFFFFFF /**< All interrupt Mask */

/**
 * @name Register: XSYSMONPSV_ISR
 * @{
 */
#define XSYSMONPSV_ISR_OFFSET                                                  \
	0X00000044U /**< Interrupt Status Register
                                                Offset */

#define XSYSMONPSV_ISR_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_ISR_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_ISR_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_ISR_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_ISR_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_ISR_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_ISR_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_ISR_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_ISR_TEMP_SHIFT                                              \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_ISR_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_ISR_OT_SHIFT                                                \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_ISR_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_ISR_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_ISR_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_ISR_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_ISR_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_ISR_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_ISR_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_ISR_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_ISR_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_ISR_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_ISR_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_IMR0
 * @{
 */
#define XSYSMONPSV_IMR0_OFFSET                                                 \
	0X00000048U /**< Interrupt Mask Reg 0
                                                 Offset */

#define XSYSMONPSV_IMR0_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_IMR0_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_IMR0_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_IMR0_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_IMR0_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_IMR0_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_IMR0_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_IMR0_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_IMR0_TEMP_SHIFT                                             \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IMR0_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IMR0_OT_SHIFT                                               \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IMR0_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IMR0_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_IMR0_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_IMR0_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_IMR0_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_IMR0_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_IMR0_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_IMR0_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_IMR0_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_IMR0_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_IMR0_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_IER0
 * @{
 */
#define XSYSMONPSV_IER0_OFFSET 0X0000004CU /**< Interrupt Enable 0 Offset */

#define XSYSMONPSV_IER0_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_IER0_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_IER0_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_IER0_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_IER0_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_IER0_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_IER0_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_IER0_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_IER0_TEMP_SHIFT                                             \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IER0_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IER0_OT_SHIFT                                               \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IER0_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IER0_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_IER0_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_IER0_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_IER0_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_IER0_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_IER0_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_IER0_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_IER0_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_IER0_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_IER0_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_IDR0
 * @{
 */
#define XSYSMONPSV_IDR0_OFFSET                                                 \
	0X00000050U /**< Interrupt Disable Reg 0
                                                 Offset */

#define XSYSMONPSV_IDR0_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_IDR0_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_IDR0_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_IDR0_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_IDR0_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_IDR0_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_IDR0_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_IDR0_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_IDR0_TEMP_SHIFT                                             \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IDR0_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IDR0_OT_SHIFT                                               \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IDR0_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IDR0_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_IDR0_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_IDR0_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_IDR0_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_IDR0_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_IDR0_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_IDR0_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_IDR0_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_IDR0_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_IDR0_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_IMR1
 * @{
 */
#define XSYSMONPSV_IMR1_OFFSET                                                 \
	0X00000054U /**< Interrupt Mask Reg 1
                                                 Offset */

#define XSYSMONPSV_IMR1_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_IMR1_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_IMR1_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_IMR1_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_IMR1_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_IMR1_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_IMR1_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_IMR1_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_IMR1_TEMP_SHIFT                                             \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IMR1_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IMR1_OT_SHIFT                                               \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IMR1_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IMR1_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_IMR1_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_IMR1_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_IMR1_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_IMR1_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_IMR1_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_IMR1_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_IMR1_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_IMR1_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_IMR1_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_IER1
 * @{
 */
#define XSYSMONPSV_IER1_OFFSET                                                 \
	0X00000058U /**< Interrupt Enable Reg 1
                                                 Offset */

#define XSYSMONPSV_IER1_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_IER1_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_IER1_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_IER1_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_IER1_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_IER1_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_IER1_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_IER1_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_IER1_TEMP_SHIFT                                             \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IER1_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IER1_OT_SHIFT                                               \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IER1_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IER1_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_IER1_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_IER1_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_IER1_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_IER1_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_IER1_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_IER1_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_IER1_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_IER1_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_IER1_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_IDR1
 * @{
 */
#define XSYSMONPSV_IDR1_OFFSET                                                 \
	0X0000005CU /**< Interrupt Disable Reg 1
                                                 Offset */

#define XSYSMONPSV_IDR1_NEW_DATA3_SHIFT 15U /**< New Data 3 Shift */
#define XSYSMONPSV_IDR1_NEW_DATA3_MASK 0X00008000U
/**< New Data 3 Mask */
#define XSYSMONPSV_IDR1_NEW_DATA2_SHIFT 14U /**< New Data 2 Shift */
#define XSYSMONPSV_IDR1_NEW_DATA2_MASK 0X00004000U
/**< New Data 2 Mask */
#define XSYSMONPSV_IDR1_NEW_DATA1_SHIFT 13U /**< New Data 1 Shift */
#define XSYSMONPSV_IDR1_NEW_DATA1_MASK 0X00002000U
/**< New Data 1 Mask */
#define XSYSMONPSV_IDR1_NEW_DATA0_SHIFT 12U /**< New Data 0 Shift */
#define XSYSMONPSV_IDR1_NEW_DATA0_MASK 0X00001000U
/**< New Data 0 Mask */
#define XSYSMONPSV_IDR1_TEMP_SHIFT                                             \
	9U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IDR1_TEMP_MASK 0X00000200U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IDR1_OT_SHIFT                                               \
	8U /**< Device Temperature Alarm
                                               Shift */
#define XSYSMONPSV_IDR1_OT_MASK 0X00000100U
/**< Device Temperature Alarm
                                               Mask */
#define XSYSMONPSV_IDR1_ALARM4_SHIFT 4U /**< Supply[128-159] Alarm Shift */
#define XSYSMONPSV_IDR1_ALARM4_MASK 0X00000010U
/**< Supply[128-159] Alarm Mask */
#define XSYSMONPSV_IDR1_ALARM3_SHIFT 3U /**< Supply[96-127] Alarm Shift */
#define XSYSMONPSV_IDR1_ALARM3_MASK 0X00000008U
/**< Supply[96-127] Alarm Mask */

#define XSYSMONPSV_IDR1_ALARM2_SHIFT 2U /**< Supply[64-95] Alarm Shift */
#define XSYSMONPSV_IDR1_ALARM2_MASK 0X00000004U
/**< Supply[64-95] Alarm Mask */

#define XSYSMONPSV_IDR1_ALARM1_SHIFT 1U /**< Supply[32-63] Alarm Shift */
#define XSYSMONPSV_IDR1_ALARM1_MASK 0X00000002U
/**< Supply[32-63] Alarm Mask */

#define XSYSMONPSV_IDR1_ALARM0_SHIFT 0U /**< Supply[0-31] Alarm Shift */
#define XSYSMONPSV_IDR1_ALARM0_MASK 0X00000001U
/**< Supply[0-31] Alarm Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_CONFIG0
 * @{
 */
#define XSYSMONPSV_CONFIG0 0X00000100U /**< Config0 register */

#define XSYSMONPSV_CONFIG0_TEMP_AVERAGE_SHIFT 24U /**< Temp Average Shift */
#define XSYSMONPSV_CONFIG0_TEMP_AVERAGE_MASK                                   \
        0x0F000000U /**< Temp Average Mask */
#define XSYSMONPSV_CONFIG0_SUPPLY_AVERAGE_SHIFT                                \
        14U /**< Supply Average Shift */
#define XSYSMONPSV_CONFIG0_SUPPLY_AVERAGE_MASK                                 \
        0x0001C000U /**< Supply Average Mask */

#define XSYSMONPSV_AVERAGE_0_SAMPLES                                           \
        0U /**< No Averaging, Full sample rate */
#define XSYSMONPSV_AVERAGE_2_SAMPLES                                           \
        1U /**< Average 2 samples, Full sample rate/2 */
#define XSYSMONPSV_AVERAGE_4_SAMPLES                                           \
        2U /**< Average 4 samples, Full sample rate/4 */
#define XSYSMONPSV_AVERAGE_8_SAMPLES                                           \
        4U /**< Average 8 samples, Full sample rate/8 */
#define XSYSMONPSV_AVERAGE_16_SAMPLES                                          \
        8U /**< Average 16 samples, Full sample rate/16 */

#define XSYSMONPSV_CONFIG0_I2C_NOT_PMBUS_SHIFT 9U /**< I2C Select Shift */
#define XSYSMONPSV_CONFIG0_I2C_NOT_PMBUS_MASK 0X00000200U
/**< I2C Select Mask */
#define XSYSMONPSV_CONFIG0_PMBUS_UNRESTRICTED_SHIFT                            \
	8U /**< Unrestrict PMBus
                                                           Shift */
#define XSYSMONPSV_CONFIG0_PMBUS_UNRESTRICTED_MASK 0X00000100U
/**< Unrestrict PMBus
                                                           Mask */
#define XSYSMONPSV_CONFIG0_PMBUS_ENABLE_SHIFT                                  \
	7U /**< PMBus Enable
                                                          Shift */
#define XSYSMONPSV_CONFIG0_PMBUS_ENABLE_MASK 0X00000080U
/**< PMBus Enable
                                                          Mask */
#define XSYSMONPSV_CONFIG0_PMBUS_ADDRESS_SHIFT                                 \
	0U /**< PMBus Address
                                                          Shift */
#define XSYSMONPSV_CONFIG0_PMBUS_ADDRESS_MASK 0X0000007FU
/**< PMBus Address
                                                          Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_NEW_DATA_FLAG0
 * @{
 */
#define XSYSMONPSV_NEW_DATA_FLAG0                                              \
	0X00001000U /**< New Data Flag
                                                    Supply[0-31] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_NEW_DATA_FLAG1
 * @{
 */
#define XSYSMONPSV_NEW_DATA_FLAG1                                              \
	0X00001004U /**< New Data Flag
                                                    Supply[32-63] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_NEW_DATA_FLAG2
 * @{
 */
#define XSYSMONPSV_NEW_DATA_FLAG2                                              \
	0X00001008U /**< New Data Flag
                                                    Supply[64-95] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_NEW_DATA_FLAG3
 * @{
 */
#define XSYSMONPSV_NEW_DATA_FLAG3                                              \
	0X0000100CU /**< New Data Flag
                                                    Supply[96-127] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_NEW_DATA_FLAG4
 * @{
 */
#define XSYSMONPSV_NEW_DATA_FLAG4                                              \
	0X00001010U /**< Alarm Flag
                                                    Supply[128-159] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_FLAG0
 * @{
 */
#define XSYSMONPSV_ALARM_FLAG0                                                 \
	0X00001018U /**< Alarm Flag
                                                    Supply[0-31] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_FLAG1
 * @{
 */
#define XSYSMONPSV_ALARM_FLAG1                                                 \
	0X0000101CU /**< Alarm Flag
                                                    Supply[32-63] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_FLAG2
 * @{
 */
#define XSYSMONPSV_ALARM_FLAG2                                                 \
	0X00001020U /**< Alarm Flag
                                                    Supply[63-95] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_FLAG3
 * @{
 */
#define XSYSMONPSV_ALARM_FLAG3                                                 \
	0X00001024U /**< Alarm Flag
                                                    Supply[96-127] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_FLAG4
 * @{
 */
#define XSYSMONPSV_ALARM_FLAG4                                                 \
	0X00001028U /**< Alarm Flag
                                                    Supply[128-159] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_MAX
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_MAX                                             \
	0X00001030U /**< Max Device Temperature
                                                    Offset (Only for
                                                    SE1 Silicon) */
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP                                                 \
	0X00001030U /**< Device Temperature
                                                  Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_MIN
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_MIN                                             \
	0X00001034U /**< Min Device Temperature
                                                    Offset (Only for
                                                    SE1 Silicon) */
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_MIN
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_MAX_MAX 0x00001F90
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_MIN
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_MIN_MIN 0x00001F8C
/*@}*/

/**
 * @name Supply format macros
 * @{
 */
#define XSYSMONPSV_SUPPLY_MANTISSA_MASK                                        \
	0x0000FFFFU /**< Supply Mantissa
                                                              Mask*/
#define XSYSMONPSV_SUPPLY_FMT_MASK                                             \
	0x00010000U /**< Supply Format
                                                              Mask*/
#define XSYSMONPSV_SUPPLY_FMT_SHIFT 16U /**< Supply Format Shift*/

#define XSYSMONPSV_SUPPLY_MODE_MASK                                            \
	0x00060000U /**< Supply Mode
                                                              Mask*/

#define XSYSMONPSV_SUPPLY_MODE_SHIFT 17U /**< Supply Mode Shift*/

#define XSYSMONPSV_SUPPLY_MANTISSA_SIGN 15U /**< Supply Mantissa Shift*/

/*@}*/

/**
 * @name Register: XSYSMONPSV_SUPPLY
 * @{
 */
#define XSYSMONPSV_SUPPLY 0X00001040U /**< Supply Base Register Offset */

/*@}*/

/**
 * @name Register: XSYSMONPSV_SUPPLY_MIN
 * @{
 */
#define XSYSMONPSV_SUPPLY_MIN                                                  \
	0X00001340U /**< Min Supply Base Register
                                               Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_SUPPLY_MAX
 * @{
 */
#define XSYSMONPSV_SUPPLY_MAX                                                  \
	0X00001640U /**< Max Supply Base Register
                                               Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_REG0
 * @{
 */
#define XSYSMONPSV_ALARM_REG0                                                  \
	0X00001940U /**< Alarm Reg
                                                Supply[0-31] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_REG1
 * @{
 */
#define XSYSMONPSV_ALARM_REG1                                                  \
	0X00001944U /**< Alarm Reg
                                                Supply[32-63] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_REG2
 * @{
 */
#define XSYSMONPSV_ALARM_REG2                                                  \
	0X00001948U /**< Alarm Reg
                                                Supply[64-95] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_REG3
 * @{
 */
#define XSYSMONPSV_ALARM_REG3                                                  \
	0X0000194CU /**< Alarm Reg
                                                Supply[96-127] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_REG4
 * @{
 */
#define XSYSMONPSV_ALARM_REG4                                                  \
	0X00001950U /**< Alarm Reg
                                                Supply[128-159] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG0
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG0                                                  \
	0X00001958U /**< Enable Average for Supply Reg
                                                Supply[0-31] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG1
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG1                                                  \
	0X0000195CU /**< Enable Average for Supply Reg
                                                Supply[32-63] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG2
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG2                                                  \
	0X00001960U /**< Enable Average for Supply Reg
                                                Supply[64-95] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG3
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG3                                                  \
	0X00001964U /**< Enable Average for Supply Reg
                                                Supply[96-127] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG4
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG4                                                  \
	0X00001968U /**< Enable Average for Supply Reg
                                                Supply[127-159] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_TH
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_TH                                              \
	0X00001970U /**< Device Temperature
                                                   Threshold Falling*/
/*@}*/
/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_TH_FALLING
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_TH_FALLING                                      \
	0X00001970U /**< Device Temperature
							    Threshold Falling*/
/*@}*/

/**
 * @name Register: XSYSMONPSV_DEVICE_TEMP_TH_RISING
 * @{
 */
#define XSYSMONPSV_DEVICE_TEMP_TH_RISING                                       \
	0X00001974U /**< Device Temperature
                                                           Threshold Rising*/
/*@}*/

/**
 * @name Register: XSYSMONPSV_OT_TEMP_TH
 * @{
 */
#define XSYSMONPSV_OT_TEMP_TH                                                  \
	0X00001978U /**< OT Temperature
                                                   Threshold Base Offset*/
/*@}*/

/**
 * @name Register: XSYSMONPSV_OT_TEMP_TH_FALLING
 * @{
 */
#define XSYSMONPSV_OT_TEMP_TH_FALLING                                          \
	0X00001978U /**< OT Temperature
                                                        Threshold Falling*/
/*@}*/

/**
 * @name Register: XSYSMONPSV_OT_TEMP_TH_RISING
 * @{
 */
#define XSYSMONPSV_OT_TEMP_TH_RISING                                           \
	0X0000197CU /**< OT Temperature
                                                   Threshold Rising*/
/*@}*/

/**
 * @name Register: XSYSMONPSV_SUPPLY_TH_LOWER
 * @{
 */
#define XSYSMONPSV_SUPPLY_TH_LOWER                                             \
	0X00001980U /**< Supply Lower Threshold
                                                    Offset */

#define XSYSMONPSV_SUPPLY_TH_LOWER_VAL_RO_SHIFT                                \
	16U /**< Read-Only
                                                        Format Shift */
#define XSYSMONPSV_SUPPLY_TH_LOWER_VAL_RO_MASK 0X00070000U
/**< Read-only Format
                                                        Mask */
#define XSYSMONPSV_SUPPLY_TH_LOWER_VAL_SHIFT                                   \
	0U /**< Threhsold Value
                                                        Shift */
#define XSYSMONPSV_SUPPLY_TH_LOWER_VAL_MASK 0X0000FFFFU
/**< Threshold Value
                                                        Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_SUPPLY_TH_UPPER
 * @{
 */
#define XSYSMONPSV_SUPPLY_TH_UPPER                                             \
	0X00001C80U /**< Supply Upper Threshold
                                                    Offset */

#define XSYSMONPSV_SUPPLY_TH_UPPER_VAL_RO_SHIFT                                \
	16U /**< Read-Only
                                                        Format Shift */
#define XSYSMONPSV_SUPPLY_TH_UPPER_VAL_RO_MASK 0X00070000U
/**< Read-only Format
                                                        Mask */
#define XSYSMONPSV_SUPPLY_TH_UPPER_VAL_SHIFT                                   \
	0U /**< Threhsold Value
                                                        Shift */
#define XSYSMONPSV_SUPPLY_TH_UPPER_VAL_MASK 0X0000FFFFU
/**< Threshold Value
                                                        Mask */

/*@}*/

/**
 * @name Register: XSYSMONPSV_NEW_DATA_INT_SRC
 * @{
 */
#define XSYSMONPSV_NEW_DATA_INT_SRC                                            \
	0X00001F80U /**< New Data Interrupt
                                                      Supply Source */

#define XSYSMONPSV_INTR_NEW_DATA_MASK                                          \
	0x0000F000U /**< New Data Interrupt
                                                      Mask */
#define XSYSMONPSV_INTR_NEW_DATA_SHIFT                                         \
	12U /**< New Data Interrupt
                                                      Shift */

#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID3_SHIFT                             \
	24U /**< New Data Interrupt
                                                           3 Source shift */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID3_MASK 0XFF000000U
/**< New Data Interrupt
                                                           3 Source Mask */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID2_SHIFT                             \
	16U /**< New Data Interrupt
                                                           2 Source shift */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID2_MASK 0X00FF0000U
/**< New Data Interrupt
                                                           2 Source Mask */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID1_SHIFT                             \
	8U /**< New Data Interrupt
                                                           1 Source shift */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID1_MASK 0X0000FF00U
/**< New Data Interrupt
                                                           1 Source Mask */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID0_SHIFT                             \
	0U /**< New Data Interrupt
                                                           0 Source shift */
#define XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID0_MASK 0X000000FFU
/**< New Data Interrupt
                                                           0 Source Mask */
/*@}*/

/**
 * @name Register: XSYSMONPSV_ALARM_CONFIG
 * @{
 */
#define XSYSMONPSV_ALARM_CONFIG                                                \
	0X00001F84U /**< Alarm Mode
                                                          Config Reg*/

#define XSYSMONPSV_ALARM_CONFIG_DEV_ALARM_MODE_SHIFT                           \
	1U /**< Device Temp Alarm
                                                            Mode Shift */
#define XSYSMONPSV_ALARM_CONFIG_DEV_ALARM_MODE_MASK 0X00000002U
/**< Device Temp Alarm
                                                            Mask Shift */

#define XSYSMONPSV_ALARM_CONFIG_OT_ALARM_MODE_SHIFT                            \
	0U /**< OT Temp Alarm
                                                            Mode Shift */
#define XSYSMONPSV_ALARM_CONFIG_OT_ALARM_MODE_MASK 0X00000001U
/**< OT Temp Alarm */
/**
 * @name Register: XSYSMONPSV_STATUS_RESET
 * @{
 */
#define XSYSMONPSV_STATUS_RESET 0X00001F94U /**< Status Reset Offset */

#define XSYSMONPSV_STATUS_RESET_SUPPLY_SHIFT 1U /**< Supply Reset Shift */
#define XSYSMONPSV_STATUS_RESET_SUPPLY_MASK 0X00000002U
/**< Supply Reset Mask */

#define XSYSMONPSV_STATUS_RESET_DEVICE_TEMP_SHIFT                              \
	0U /**< Temperature Reset
                                                         Mask */
#define XSYSMONPSV_STATUS_RESET_DEVICE_TEMP_MASK 0X00000001U
/**< Temperature Reset
						        Mask */

/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG8
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG8                                                  \
	0X000024B4U /**< Enable Average for Temp Satellite Reg
                                                Satellite[1-32] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_EN_AVG_REG9
 * @{
 */
#define XSYSMONPSV_EN_AVG_REG9                                                  \
	0X000024B8U /**< Enable Average for Temp Satellite Reg
                                                Satellite[33-64] Offset */
/*@}*/

/**
 * @name Register: XSYSMONPSV_TEMP_SAT
 */
#define XSYSMONPSV_TEMP_SAT 0X00001FACU /**< Internal Temp Reg */

/*@}*/

#define XSYSMONPSV_INVALID_SUPPLY 160U /**< Invalid Supply */
#define XSYSMONPSV_PMBUS_INTERFACE 0U /**< PMBus interface select */
#define XSYSMONPSV_I2C_INTERFACE 1U /**< I2C interface select */
#define XSYSMONPSV_INVALID 0x80000000U /**< Invalid Val */
#define XSYSMONPSV_EXPONENT_RANGE_16 16U /**< Voltage exponent val bit */
#define XSYSMONPSV_QFMT_SIGN 15U /**< Q format signed bit */
#define XSYSMONPSV_QFMT_FRACTION 128 /**< Q format fractional val */
#define XSYSMONPSV_UP_SAT_SIGNED                                               \
	32767 /**< Upper limit staurated
                                                  signed val */
#define XSYSMONPSV_UP_SAT                                                      \
	65535 /**< Upper limit saturated
                                                  unsigned val */
#define XSYSMONPSV_LOW_SAT_SIGNED                                              \
	-32767 /**< Lower limit signed
                                                  saturated val */
#define XSYSMONPSV_LOW_SAT                                                     \
	0 /**< Lower limit unsigned
                                                  saturated val */
#define XSYSMONPSV_BIPOLAR_UP_SAT                                              \
	0x7FFF /**< Upper limit bipolar
                                                  saturated val */
#define XSYSMONPSV_BIPOLAR_LOW_SAT                                             \
	0x8000 /**< Lower limit bipolar
                                                  saturated val */
#define XSYSMONPSV_UNIPOLAR_UP_SAT                                             \
	0xFFFF /**< Upper limit unipolar
                                                  saturated val */
#define XSYSMONPSV_UNIPOLAR_LOW_SAT                                            \
	0x0000 /**< Lower limit unipolar
                                                  saturated val */
#define XSYSMONPSV_ENABLE 1U /**< Enable */
#define XSYSMONPSV_DISABLE 0U /**< Disable */
#define XSYSMONPSV_HYSTERESIS 1U /**< Hysteresis Mode */
#define XSYSMONPSV_WINDOW 0U /**< Window Mode */

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param        RegisterAddr Register address in the address
*               space of the SYSMONPSV device.
*
* @return       The 32-bit value of the register
*
*
*****************************************************************************/
#define XSysMonPsv_ReadReg(RegisterAddr) Xil_In32(RegisterAddr)

/****************************************************************************/
/**
*
* This macro writes the given register.
*
* @param        RegisterAddr Register address in the address
*               space of the SYSMONPSV device.
* @param        Data 32-bit value to write to the register.
*
* @return       None.
*
*
*****************************************************************************/
#define XSysMonPsv_WriteReg(RegisterAddr, Data)                 \
                                Xil_Out32(RegisterAddr, (u32)(Data))

#ifdef __cplusplus
}
#endif

#endif /* XSYSMONPSV_HW_H_ */
/** @} */
