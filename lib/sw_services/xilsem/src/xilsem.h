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
*
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

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSem_Init (void);
int XSem_InitScan (void);
int XSem_StartScan (void);
int XSem_StopScan (void);

#ifdef __cplusplus
}
#endif

#endif	/* XIL_SEM_H */
