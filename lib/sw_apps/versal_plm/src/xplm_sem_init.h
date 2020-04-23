/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_sem_init.h
*
* This file contains the xilsem interfaces of PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  rm   09/22/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_SEM_INIT_H
#define XPLM_SEM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_default.h"
#ifdef XPLM_SEM

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_SemInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_INIT_SEM_H */
