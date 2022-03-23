/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSLPDOMAIN_H_
#define XPM_PSLPDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LPD_BISR_DONE		BIT(0)
#define LPD_BISR_DATA_COPIED	BIT(1)

typedef struct XPm_PsLpDomain XPm_PsLpDomain;

/**
 * The PS low power domain node class.
 */
struct XPm_PsLpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 LpdIouSlcrBaseAddr; /**< LPD IOU SLCR Base address */
	u32 LpdSlcrBaseAddr; /**< LPD SLCR Base address */
	u32 LpdSlcrSecureBaseAddr; /**< LPD SLCR Secure base address */
	u8 LpdBisrFlags;
};

/************************** Function Prototypes ******************************/
XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressesCnt);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSLPDOMAIN_H_ */
