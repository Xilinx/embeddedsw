/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_wdt_hw.h
*
* This file owns the PMC WDT hardware definitions and the selection of the PMC
* WDT instance from the design parameters. It is deliberately self contained and
* does not include any platform xplmi_hw.h, so the values it produces cannot
* depend on which platform header a translation unit happens to pull in first.
* Device families without a PMC WWDT (versal, versal_2vp) match no branch below
* and the file is inert for them.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0   sd   08/20/2026 Initial release
*       ng   08/27/2026 Reordered the base address for handling the device family
*
* </pre>
*
******************************************************************************/

#ifndef XPLMI_WDT_HW_H
#define XPLMI_WDT_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "bspconfig.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
/* PMC WWDT address map, one branch per device family that has a PMC WWDT. */
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_NET)
	#define XPLMI_PMC_WDT_BASEADDR		(0xF03F0000U)
	#define XPLMI_PMC_GWDT_CNTRL_STATUS_REG	(0xF03F2000U)
#elif defined(VERSAL_2VP_P)
	#define XPLMI_PMC_WDT_BASEADDR		(0xF1158000U)
	#define XPLMI_PMC_GWDT_CNTRL_STATUS_REG	(0xF115A000U)
#endif

#ifdef XPLMI_PMC_WDT_BASEADDR

#define XPLMI_PMC_WDT_GWCSR_GWEN_MASK	(0x01U)

#ifndef SDT
	#if defined(XPAR_WDTTB_0_DEVICE_ID) && (XPAR_WDTTB_0_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_0_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_0_DEVICE_ID
	#elif defined(XPAR_WDTTB_1_DEVICE_ID) && (XPAR_WDTTB_1_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_1_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_1_DEVICE_ID
	#elif defined(XPAR_WDTTB_2_DEVICE_ID) && (XPAR_WDTTB_2_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_2_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_2_DEVICE_ID
	#elif defined(XPAR_WDTTB_3_DEVICE_ID) && (XPAR_WDTTB_3_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_3_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_3_DEVICE_ID
	#elif defined(XPAR_WDTTB_4_DEVICE_ID) && (XPAR_WDTTB_4_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_4_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_4_DEVICE_ID
	#elif defined(XPAR_WDTTB_5_DEVICE_ID) && (XPAR_WDTTB_5_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_5_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_5_DEVICE_ID
	#elif defined(XPAR_WDTTB_6_DEVICE_ID) && (XPAR_WDTTB_6_BASEADDR == XPLMI_PMC_WDT_BASEADDR) &&\
			(XPAR_WDTTB_6_ENABLE_WINDOW_WDT == 0U)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_WDTTB_6_DEVICE_ID
	#endif
#else
	#if defined(XPAR_XWDTTB_0_BASEADDR) && (XPAR_XWDTTB_0_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_0_BASEADDR
	#elif defined(XPAR_XWDTTB_1_BASEADDR) && (XPAR_XWDTTB_1_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_1_BASEADDR
	#elif defined(XPAR_XWDTTB_2_BASEADDR) && (XPAR_XWDTTB_2_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_2_BASEADDR
	#elif defined(XPAR_XWDTTB_3_BASEADDR) && (XPAR_XWDTTB_3_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_3_BASEADDR
	#elif defined(XPAR_XWDTTB_4_BASEADDR) && (XPAR_XWDTTB_4_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_4_BASEADDR
	#elif defined(XPAR_XWDTTB_5_BASEADDR) && (XPAR_XWDTTB_5_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_5_BASEADDR
	#elif defined(XPAR_XWDTTB_6_BASEADDR) && (XPAR_XWDTTB_6_BASEADDR == XPLMI_PMC_WDT_BASEADDR)
	#define XPLMI_PMC_WDT
	#define XPLMI_PMC_WDT_DEVICE_ID		XPAR_XWDTTB_6_BASEADDR
	#endif
#endif

#endif /* XPLMI_PMC_WDT_BASEADDR */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_WDT_HW_H */
