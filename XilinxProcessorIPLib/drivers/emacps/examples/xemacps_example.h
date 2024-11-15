/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xemacps_example.h
*
* Defines common data types, prototypes, and includes the proper headers
* for use with the EMACPS example code residing in this directory.
*
* This file along with xemacps_example_util.c are utilized with the specific
* example code in the other source code files provided.
* These examples are designed to be compiled and utilized within the SDK
* standalone BSP development environment.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  01/10/10 First release
* 1.01a asa  02/27/12 Hash define added for EMACPS_SLCR_DIV_MASK.
* 1.05a asa  09/22/13 The EthernetFrame is made cache line aligned (32 bytes).
*					  Fix for CR #663885.
* 3.0   hk   02/20/15 Increase array sizes to add support for jumbo frames.
* 3.2   mus  02//16 Added support support to INTC controller
* 3.3   kpc  12/09/16 Fixed issue when -O2 is enabled
* 3.9   hk   01/23/19 Update versal emulation specific fixes.
*            03/20/19 Fix alignment pragmas for IAR compiler.
* </pre>
*
*****************************************************************************/
#ifndef XEMACPS_EXAMPLE_H
#define XEMACPS_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include <stdio.h>
#include <stdlib.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xemacps.h"		/* defines XEmacPs API */
#ifdef SDT
#include "xinterrupt_wrap.h"
#else
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#endif

#ifndef __MICROBLAZE__
#include "sleep.h"
#include "xparameters_ps.h"	/* defines XPAR values */
#include "xpseudo_asm.h"
#endif

/************************** Constant Definitions ****************************/

#define EMACPS_LOOPBACK_SPEED    	100	/* 100Mbps */
#define EMACPS_LOOPBACK_SPEED_1G 	1000	/* 1000Mbps */
#define EMACPS_LOOPBACK_SPEED_2_5G 	2500	/* 5000Mbps */
#define EMACPS_LOOPBACK_SPEED_5G 	5000	/* 5000Mbps */
#define EMACPS_LOOPBACK_SPEED_10G 	10000	/* 10000Mbps */
#define EMACPS_PHY_DELAY_SEC     4	/* Amount of time to delay waiting on
					   PHY to reset */
#define EMACPS_SLCR_DIV_MASK	0xFC0FC0FF

#define CSU_VERSION		0xFFCA0044
#define PLATFORM_MASK		0xF000
#define PLATFORM_SILICON	0x0000
#define VERSAL_VERSION		0xF11A0004
#define PLATFORM_MASK_VERSAL	0xF000000
#define PLATFORM_VERSALEMU	0x1000000
#define PLATFORM_VERSALSIL	0x0000000

/***************** Macros (Inline Functions) Definitions ********************/


/**************************** Type Definitions ******************************/

/*
 * Define an aligned data type for an ethernet frame. This declaration is
 * specific to the GNU compiler
 */
#ifdef __ICCARM__
typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE_JUMBO];
#else
typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE_JUMBO]
__attribute__ ((aligned(64)));
#endif

/************************** Function Prototypes *****************************/



/*
 * Utility functions implemented in xemacps_example_util.c
 */
void EmacPsUtilSetupUart(void);
void EmacPsUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr);
void EmacPsUtilFrameHdrFormatType(EthernetFrame *FramePtr, u16 FrameType);
void EmacPsUtilFrameSetPayloadData(EthernetFrame *FramePtr, u32 PayloadSize);
LONG EmacPsUtilFrameVerify(EthernetFrame *CheckFrame,
			   EthernetFrame *ActualFrame);
void EmacPsUtilFrameMemClear(EthernetFrame *FramePtr);
LONG EmacPsUtilEnterLoopback(XEmacPs *XEmacPsInstancePtr, u32 Speed);
void EmacPsUtilstrncpy(char *Destination, const char *Source, u32 n);
void EmacPsUtilErrorTrap(const char *Message);
void EmacpsDelay(u32 delay);

/************************** Variable Definitions ****************************/

extern XEmacPs EmacPsInstance;	/* Device instance used throughout examples */
extern char EmacPsMAC[];	/* Local MAC address */
extern u32 Platform;

#ifdef __cplusplus
}
#endif

#endif /* XEMACPS_EXAMPLE_H */
