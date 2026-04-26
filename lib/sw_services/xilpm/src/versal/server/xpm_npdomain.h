/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_NPDOMAIN_H_
#define XPM_NPDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The NOC power domain node class.
 */
typedef struct XPm_NpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_NpDomain;

/**
 * @def NPD_MEMIC_NODETYPE_SHIFT
 * @brief Bit position of the MEMIC node-type nibble inside an
 *        NpdMemIcAddresses[] entry.
 *
 * The upper 4 bits of each NpdMemIcAddresses[] entry encode the MEMIC
 * node type (NPS / NMU / etc.), allowing the address table to recover
 * the node type without a parallel array.
 */
#define NPD_MEMIC_NODETYPE_SHIFT	(28U)

/**
 * @def NPD_MEMIC_NODETYPE_MASK
 * @brief Mask of the MEMIC node-type nibble inside an
 *        NpdMemIcAddresses[] entry. Used to encode the type into the
 *        upper 4 bits and to extract it via (addr & MASK) >> SHIFT.
 */
#define NPD_MEMIC_NODETYPE_MASK		((u32)0xFU << NPD_MEMIC_NODETYPE_SHIFT)

/************************** Function Prototypes ******************************/
XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);
XStatus XPmNpDomain_MemIcInit(u32 DeviceId, u32 BaseAddr);
XStatus XPmNpDomain_IsNpdIdle(const XPm_Node *Node);
XStatus XPmNpDomain_ClockGate(const XPm_Node *Node, u8 State);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_NPDOMAIN_H_ */
