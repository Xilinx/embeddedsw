/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpsmfw_reset.h
*
* This file contains default headers and definitions used by Reset module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	rp	10/4/2018	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_RESET_H_
#define XPSMFW_RESET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define INTLPD_CONFIG_BASE_ADDR					((u32)0xFE600000)

#define INTLPD_CONFIG_RPU0_LPD_AXI				( ( INTLPD_CONFIG_BASE_ADDR ) + ((u32)0x001C0000) )
#define INTLPD_CONFIG_RPU0_LPD_AXI_POWER_IDLEREQ_MASK		((u32)0x00000004)
#define INTLPD_CONFIG_RPU0_LPD_AXI_POWER_IDLEACK_MASK		((u32)0x00000002)

#define INTLPD_CONFIG_RPU1_LPD_AXI				( ( INTLPD_CONFIG_BASE_ADDR ) + ((u32)0x001D0000) )
#define INTLPD_CONFIG_RPU1_LPD_AXI_POWER_IDLEREQ_MASK		((u32)0x00000004)
#define INTLPD_CONFIG_RPU1_LPD_AXI_POWER_IDLEACK_MASK		((u32)0x00000002)

#define INTLPD_CONFIG_INTLPD_RPU0_AXI				( ( INTLPD_CONFIG_BASE_ADDR ) + ((u32)0x00090000) )
#define INTLPD_CONFIG_INTLPD_RPU0_AXI_POWER_IDLEREQ_MASK	((u32)0x00000008)
#define INTLPD_CONFIG_INTLPD_RPU0_AXI_POWER_IDLEACK_MASK	((u32)0x00000004)

#define INTLPD_CONFIG_INTLPD_RPU1_AXI				( ( INTLPD_CONFIG_BASE_ADDR ) + ((u32)0x000A0000) )
#define INTLPD_CONFIG_INTLPD_RPU1_AXI_POWER_IDLEREQ_MASK	((u32)0x00000008)
#define INTLPD_CONFIG_INTLPD_RPU1_AXI_POWER_IDLEACK_MASK	((u32)0x00000004)

/* TODO: TBD */
#define RPU0_LPD_AXI_ISOLATION_TIMEOUT				(1000U)
#define RPU1_LPD_AXI_ISOLATION_TIMEOUT				(1000U)
#define LPD_RPU0_AXI_ISOLATION_TIMEOUT				(1000U)
#define LPD_RPU1_AXI_ISOLATION_TIMEOUT				(1000U)
/* TODO: TBD */
#define XPSMFW_RST_RPU_COMB_PROP_TIME				(1000U)
#define XPSMFW_RST_RPU_SEQ_PROP_TIME				(1000U)
#define XPSMFW_RST_FPD_COMB_PROP_TIME				(1000U)
#define XPSMFW_RST_FPD_SEQ_PROP_TIME				(1000U)

/* Software Reset Handler Table Structure */
typedef XStatus (*HandlerFunction_t)(void);
struct SwResetHandlerTable_t {
        u32 Mask;
        HandlerFunction_t Handler;
};

XStatus XPsmFw_DispatchSwRstHandler(u32 SwRstStatus, u32 SwRstIntMask);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_RESET_H_ */
