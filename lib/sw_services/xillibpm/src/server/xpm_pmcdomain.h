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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_PMCDOMAIN_H_
#define XPM_PMCDOMAIN_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PmcDomain XPm_PmcDomain;

/**
 * The PMC power domain node class.
 */
struct XPm_PmcDomain {
	XPm_PowerDomain Domain; /**< Power: Power domain base class */
};

/************************** Function Prototypes ******************************/
XStatus XPmPmcDomain_Init(XPm_PmcDomain *PmcDomain, u32 Id);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PMCDOMAIN_H_ */
