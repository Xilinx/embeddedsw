/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilsem.h
*
* This is the file which contains xilsem related interafce code.
* This will be inteface for xilsem library
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   09/22/2019 Initial release
* 1.01  rb   10/30/2020 Added XSem_Init declaration
* 1.02  gm   11/30/2020 Added SEM Start and Stop API declaration
* 1.03  rb   01/28/2021 Added SEM Pre Init API declaration
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
int XSem_CfrInit(void);
int XSem_NpiInit(void);
int XSem_NpiRunScan();
int XSem_CfrStopScan(void);
int XSem_CfrStartScan(void);
int XSem_Init(void);
int XSem_PreInit(void);
int XSem_StartScan (void);
int XSem_StopScan (void);

#ifdef __cplusplus
}
#endif

#endif		/* XIL_SEM_H */
