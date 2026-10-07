/******************************************************************************
* Copyright (c) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xl2cc_counter.h
*
* @addtogroup l2_event_counter_apis PL310 L2 Event Counters Functions
*
* xl2cc_counter.h contains APIs for configuring and controlling the event
* counters in PL310 L2 cache controller.
* PL310 has two event counters which can be used to count variety of events
* like DRHIT, DRREQ, DWHIT, DWREQ, etc. xl2cc_counter.h contains definitions
* for different configurations which can be used for the event counters to
* count a set of events.
*
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sdm  07/11/11 First release
* 3.07a asa  08/30/12 Updated for CR 675636 to provide the L2 Base Address
*		      inside the APIs
* 6.8   aru  09/06/18 Removed compilation warnings for ARMCC toolchain.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/


#ifndef L2CCCOUNTER_H
#define L2CCCOUNTER_H

/**
*@cond nocomments
*/
/***************************** Include Files ********************************/

#include "xpseudo_asm.h"
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/*
 * The following constants define the event codes for the event counters.
 */
#define XL2CC_CO		0x1
#define XL2CC_DRHIT		0x2
#define XL2CC_DRREQ		0x3
#define XL2CC_DWHIT		0x4
#define XL2CC_DWREQ		0x5
#define XL2CC_DWTREQ		0x6
#define XL2CC_IRHIT		0x7
#define XL2CC_IRREQ		0x8
#define XL2CC_WA		0x9
#define XL2CC_IPFALLOC		0xa
#define XL2CC_EPFHIT		0xb
#define XL2CC_EPFALLOC		0xc
#define XL2CC_SRRCVD		0xd
#define XL2CC_SRCONF		0xe
#define XL2CC_EPFRCVD		0xf

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/**
*@endcond
*/

/************************** Function Prototypes *****************************/

void XL2cc_EventCtrInit(s32 Event0, s32 Event1);
void XL2cc_EventCtrStart(void);
void XL2cc_EventCtrStop(u32 *EveCtr0, u32 *EveCtr1);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @} End of "addtogroup l2_event_counter_apis".
*/
#endif /* L2CCCOUNTER_H */
