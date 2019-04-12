/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XPM_POWERDOMAIN_H_
#define XPM_POWERDOMAIN_H_

#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PowerDomain XPm_PowerDomain;

#define XPM_POLL_TIMEOUT			(0X1000000U)

/**
 * The power domain node class.  This is the base class for all the power domain
 * classes.
 */
struct XPm_PowerDomainOps {
	XStatus (*PreHouseClean)(u32 *Args, u32 NumOfArgs);
	XStatus (*PostHouseClean)(u32 *Args, u32 NumOfArgs);
	XStatus (*ScanClear)(u32 *Args, u32 NumOfArgs);
	XStatus (*Mbist)(u32 *Args, u32 NumOfArgs);
	XStatus (*Lbist)(u32 *Args, u32 NumOfArgs);
	XStatus (*Bisr)(u32 *Args, u32 NumOfArgs);
	XStatus (*PlHouseclean)(u32 *Args, u32 NumOfArgs);
	XStatus (*MemInit)(u32 *Args, u32 NumOfArgs);
};

struct XPm_PowerDomain {
	XPm_Power Power; /**< Power: Power node base class */
	XPm_Power *Children; /**< List of children power nodes */
	struct XPm_PowerDomainOps *DomainOps; /**< house cleaning operations */
};

/************************** Function Prototypes ******************************/
XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    struct XPm_PowerDomainOps *Ops);
XStatus XPm_PowerUpLPD(XPm_Node *Node);
XStatus XPm_PowerDwnLPD();
XStatus XPm_PowerUpPLD();
XStatus XPm_PowerDwnPLD();
XStatus XPm_PowerUpME();
XStatus XPm_PowerDwnME();
XStatus XPm_PowerUpCPM();
XStatus XPm_PowerDwnCPM();
XStatus XPm_PowerUpNoC();
XStatus XPm_PowerDwnNoC();
XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function,
				  u32 *Args, u32 NumArgs);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWERDOMAIN_H_ */
