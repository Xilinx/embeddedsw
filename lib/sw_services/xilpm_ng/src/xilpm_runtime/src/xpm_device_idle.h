/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_DEVICE_IDLE_H_
#define XPM_DEVICE_IDLE_H_

#include "xparameters.h"
#include "xpm_node.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPmDevice_SoftResetInfo {
	u32 DeviceId;
	XStatus (*IdleHook)(u16 DeviceId, u32 BaseAddress);	/**< Hook function for idling */
	u16 IdleHookArgs;
} XPmDevice_SoftResetInfo;

#ifdef SDT
#define DUMMY_DEVICE_ID 0xFFFF
#endif

/*
 * Wrapper macros for device Ids:
 * These macros have been introduced to address the exclusion of macro-protected
 * blocks during compilation in the SDT flow, where device IDs are not supposed
 * to be defined.
 *
 * 1) In the SDT flow, it checks if the base address is defined for certain
 *    peripheral. If the base address is defined, it assigns a dummy device ID
 *    to its wrapper device ID macro.
 * 2) In the typical flow, it checks if the actual device ID is defined before
 *    assigning the device ID to its wrapper macros.
 */

#ifdef SDT
    #ifdef XPAR_XUSBPSU_0_BASEADDR
        #define WRAPPER_XUSBPSU_0_DEVICE_ID     DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XQSPIPSU_0_BASEADDR
        #define WRAPPER_XQSPIPSU_0_DEVICE_ID    DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XOSPIPSV_0_BASEADDR
        #define WRAPPER_XOSPIPSV_0_DEVICE_ID    DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XSDPS_0_BASEADDR
        #define WRAPPER_XSDPS_0_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XSDPS_1_BASEADDR
        #define WRAPPER_XSDPS_1_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XEMACPS_0_BASEADDR
        #define WRAPPER_XEMACPS_0_DEVICE_ID     DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XEMACPS_1_BASEADDR
        #define WRAPPER_XEMACPS_1_DEVICE_ID     DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_0_BASEADDR
        #define WRAPPER_XZDMA_0_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_1_BASEADDR
        #define WRAPPER_XZDMA_1_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_2_BASEADDR
        #define WRAPPER_XZDMA_2_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_3_BASEADDR
        #define WRAPPER_XZDMA_3_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_5_BASEADDR
        #define WRAPPER_XZDMA_5_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_6_BASEADDR
        #define WRAPPER_XZDMA_6_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_7_BASEADDR
        #define WRAPPER_XZDMA_7_DEVICE_ID       DUMMY_DEVICE_ID
    #endif
#else
    #ifdef XPAR_XUSBPSU_0_DEVICE_ID
        #define WRAPPER_XUSBPSU_0_DEVICE_ID    XPAR_XUSBPSU_0_DEVICE_ID
    #endif
    #ifdef XPAR_XQSPIPSU_0_DEVICE_ID
        #define WRAPPER_XQSPIPSU_0_DEVICE_ID   XPAR_XQSPIPSU_0_DEVICE_ID
    #endif
    #ifdef XPAR_XOSPIPSV_0_DEVICE_ID
        #define WRAPPER_XOSPIPSV_0_DEVICE_ID   XPAR_XOSPIPSV_0_DEVICE_ID
    #endif
    #ifdef XPAR_XSDPS_0_DEVICE_ID
        #define WRAPPER_XSDPS_0_DEVICE_ID      XPAR_XSDPS_0_DEVICE_ID
    #endif
    #ifdef XPAR_XSDPS_1_DEVICE_ID
        #define WRAPPER_XSDPS_1_DEVICE_ID      XPAR_XSDPS_1_DEVICE_ID
    #endif
    #ifdef XPAR_XEMACPS_0_DEVICE_ID
        #define WRAPPER_XEMACPS_0_DEVICE_ID    XPAR_XEMACPS_0_DEVICE_ID
    #endif
    #ifdef XPAR_XEMACPS_1_DEVICE_ID
        #define WRAPPER_XEMACPS_1_DEVICE_ID    XPAR_XEMACPS_1_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_0_DEVICE_ID
        #define WRAPPER_XZDMA_0_DEVICE_ID      XPAR_XZDMA_0_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_1_DEVICE_ID
        #define WRAPPER_XZDMA_1_DEVICE_ID      XPAR_XZDMA_1_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_2_DEVICE_ID
        #define WRAPPER_XZDMA_2_DEVICE_ID      XPAR_XZDMA_2_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_3_DEVICE_ID
        #define WRAPPER_XZDMA_3_DEVICE_ID      XPAR_XZDMA_3_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_4_DEVICE_ID
        #define WRAPPER_XZDMA_4_DEVICE_ID      XPAR_XZDMA_4_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_5_DEVICE_ID
        #define WRAPPER_XZDMA_5_DEVICE_ID      XPAR_XZDMA_5_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_6_DEVICE_ID
        #define WRAPPER_XZDMA_6_DEVICE_ID      XPAR_XZDMA_6_DEVICE_ID
    #endif
    #ifdef XPAR_XZDMA_7_DEVICE_ID
        #define WRAPPER_XZDMA_7_DEVICE_ID      XPAR_XZDMA_7_DEVICE_ID
    #endif
#endif

/* Define the XILPM device macro based on canonical defination. */
/* XILPM_USB_0 */
#ifdef VERSAL_NET
#if (!defined(XILPM_USB_EXCLUDE) && defined(WRAPPER_XUSBPSU_0_DEVICE_ID) && \
	(XPAR_XUSBPSU_0_BASEADDR == 0xF1B00000U))
#define XILPM_USB_0 WRAPPER_XUSBPSU_0_DEVICE_ID
#endif
#else
#if (!defined(XILPM_USB_EXCLUDE) && defined(WRAPPER_XUSBPSU_0_DEVICE_ID) && \
	(XPAR_XUSBPSU_0_BASEADDR == 0xFE200000U))
#define XILPM_USB_0 WRAPPER_XUSBPSU_0_DEVICE_ID
#endif
#endif

/* XILPM_QSPI_0 */
#if (defined(WRAPPER_XQSPIPSU_0_DEVICE_ID) && \
	(XPAR_XQSPIPSU_0_BASEADDR == 0xF1030000U))
#define XILPM_QSPI_0 WRAPPER_XQSPIPSU_0_DEVICE_ID
#endif

/* XILPM_OSPI_0 */
#if (defined(WRAPPER_XOSPIPSV_0_DEVICE_ID) && \
	(XPAR_XOSPIPSV_0_BASEADDR == 0xF1010000U))
#define XILPM_OSPI_0 WRAPPER_XOSPIPSV_0_DEVICE_ID
#endif

/* XILPM_SD_0 and XILPM_SD_1 */
#if (defined(WRAPPER_XSDPS_0_DEVICE_ID))
	#if (XPAR_XSDPS_0_BASEADDR == 0xF1040000U)
		#define XILPM_SD_0 WRAPPER_XSDPS_0_DEVICE_ID
	#elif (XPAR_XSDPS_0_BASEADDR == 0xF1050000U)
		#define XILPM_SD_1 WRAPPER_XSDPS_0_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XSDPS_1_DEVICE_ID))
	#if (XPAR_XSDPS_1_BASEADDR == 0xF1040000U)
		#define XILPM_SD_0 WRAPPER_XSDPS_1_DEVICE_ID
	#elif (XPAR_XSDPS_1_BASEADDR == 0xF1050000U)
		#define XILPM_SD_1 WRAPPER_XSDPS_1_DEVICE_ID
	#endif
#endif

#ifndef VERSAL_NET
/* XILPM_ETH_0 and XILPM_ETH_1 */
#if (defined(WRAPPER_XEMACPS_0_DEVICE_ID))
	#if (XPAR_XEMACPS_0_BASEADDR == 0xFF0C0000U)
		#define XILPM_ETH_0 WRAPPER_XEMACPS_0_DEVICE_ID
	#elif (XPAR_XEMACPS_0_BASEADDR == 0xFF0D0000U)
		#define XILPM_ETH_1 WRAPPER_XEMACPS_0_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XEMACPS_1_DEVICE_ID))
	#if (XPAR_XEMACPS_1_BASEADDR == 0xFF0C0000U)
		#define XILPM_ETH_0 WRAPPER_XEMACPS_1_DEVICE_ID
	#elif (XPAR_XEMACPS_1_BASEADDR == 0xFF0D0000U)
		#define XILPM_ETH_1 WRAPPER_XEMACPS_1_DEVICE_ID
	#endif
#endif

/* XILPM_ZDMA_0 to XILPM_ZDMA_7 */
#if (defined(WRAPPER_XZDMA_0_DEVICE_ID))
	#if (XPAR_XZDMA_0_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_0_DEVICE_ID
	#elif (XPAR_XZDMA_0_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_0_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_1_DEVICE_ID))
	#if (XPAR_XZDMA_1_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_1_DEVICE_ID
	#elif (XPAR_XZDMA_1_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_1_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_2_DEVICE_ID))
	#if (XPAR_XZDMA_2_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_2_DEVICE_ID
	#elif (XPAR_XZDMA_2_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_2_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_3_DEVICE_ID))
	#if (XPAR_XZDMA_3_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_3_DEVICE_ID
	#elif (XPAR_XZDMA_3_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_3_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_4_DEVICE_ID))
	#if (XPAR_XZDMA_4_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_4_DEVICE_ID
	#elif (XPAR_XZDMA_4_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_4_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_5_DEVICE_ID))
	#if (XPAR_XZDMA_5_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_5_DEVICE_ID
	#elif (XPAR_XZDMA_5_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_5_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_6_DEVICE_ID))
	#if (XPAR_XZDMA_6_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_6_DEVICE_ID
	#elif (XPAR_XZDMA_6_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_6_DEVICE_ID
	#endif
#endif
#if (defined(WRAPPER_XZDMA_7_DEVICE_ID))
	#if (XPAR_XZDMA_7_BASEADDR == 0xFFA80000U)
		#define XILPM_ZDMA_0 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFA90000U)
		#define XILPM_ZDMA_1 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFAA0000U)
		#define XILPM_ZDMA_2 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFAB0000U)
		#define XILPM_ZDMA_3 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFAC0000U)
		#define XILPM_ZDMA_4 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFAD0000U)
		#define XILPM_ZDMA_5 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFAE0000U)
		#define XILPM_ZDMA_6 WRAPPER_XZDMA_7_DEVICE_ID
	#elif (XPAR_XZDMA_7_BASEADDR == 0xFFAF0000U)
		#define XILPM_ZDMA_7 WRAPPER_XZDMA_7_DEVICE_ID
	#endif
#endif
#endif

#if defined(XILPM_QSPI_0)
#include "xqspipsu.h"
XStatus NodeQspiIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XILPM_OSPI_0)
#include "xospipsv.h"
XStatus NodeOspiIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XILPM_SD_0) || defined(XILPM_SD_1)
#include "xsdps.h"
XStatus NodeSdioIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XILPM_USB_0)
#include "xusbpsu.h"
XStatus NodeUsbIdle(u16 DeviceId, u32 BaseAddress);
#endif

#ifndef VERSAL_NET
#if defined(XILPM_ETH_0) || defined(XILPM_ETH_1)
#include "xemacps_hw.h"
XStatus NodeGemIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if (defined(XILPM_ZDMA_0) || \
	defined(XILPM_ZDMA_1) || \
	defined(XILPM_ZDMA_2) || \
	defined(XILPM_ZDMA_3) || \
	defined(XILPM_ZDMA_4) || \
	defined(XILPM_ZDMA_5) || \
	defined(XILPM_ZDMA_6) || \
	defined(XILPM_ZDMA_7))
#include "xzdma_hw.h"
XStatus NodeZdmaIdle(u16 DeviceId, u32 BaseAddr);
#endif
#endif

XStatus XPmDevice_SoftResetIdle(const XPm_Device *Device, const u32 IdleReq);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DEVICE_IDLE_H_ */
