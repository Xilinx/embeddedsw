/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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


#ifdef __cplusplus
}
#endif

#endif		/* XIL_SEM_H */
