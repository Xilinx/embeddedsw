/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilsem.h
*
* This is the file which contains xilsem related interface code.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   09/22/2019 Initial release
* 1.01  rb   10/30/2020 Added XSem_Init declaration
* 1.02  gm   11/30/2020 Added SEM Start and Stop API declaration
* 1.03  rb   01/28/2021 Added SEM Pre Init API declaration
* 1.04  rb   03/09/2021 Updates Sem Init API call, removed unused APIs
* 1.05  rb   03/31/2021 Provided XMPU register update config and API
* 1.06	hv   09/03/2021 Fix Doxygen warnings
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XIL_SEM_H
#define XIL_SEM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
/***************************** Global variables ******************************/

/**
 * XMPU Config Database
 */
typedef struct {
	u32 DdrmcNocAddr; /**< DDRMC NOC address */
	u32 XmpuRegionNum; /**< XMPU region number */
	u32 XmpuConfig; /**< XMPU entry config */
	u32 XmpuStartAddrLo; /**< XMPU start address lower portion */
	u32 XmpuStartAddrUp; /**< XMPU start address upper portion */
	u32 XmpuEndAddrLo; /**< XMPU end address lower portion */
	u32 XmpuEndAddrUp; /**< XMPU end address upper portion */
	u32 XmpuMaster; /**< XMPU master ID and mask */
}XSem_XmpuCfg;

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
/** @cond xilsem_internal
 * @{
 */
/**< XilSEM common function prototypes */
int XSem_Init (void);
int XSem_InitScan (void);
int XSem_StartScan (void);
int XSem_StopScan (void);
int XSem_XmpuRegUpdate(XSem_XmpuCfg *XmpuCfg);
/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif	/* XIL_SEM_H */
