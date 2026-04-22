/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* (c) Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   09/22/2019 Initial release
* 1.01  kc   02/10/2020 Updated scheduler to add/remove tasks
*       kc   02/17/2020 Added configurable priority for scheduler tasks
*       kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*                       functionality
*       kc   03/23/2020 Minor code cleanup
* 1.02  rb   01/28/2021 Added Sem PreInit prototype, updated header file
*       rb   03/09/2021 Updated Sem Init API call
* 1.03  ga   05/03/2023 Removed XPlm_SemInit prototype and updated
*                       copyright information
* 1.04  gam  04/08/2026 Add prototype for Beam testing and protect the code
*                       with macro PLM_SEM_BEAM_TESTING.
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
#include "xplmi_hw.h"

#ifdef XPLM_SEM

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef PLM_SEM_BEAM_TESTING

#define PMC_RAM_SEM_CRAM_STATUS         (XPLMI_PMCRAM_BASEADDR + 0X00014084U)

#define PMC_RAM_SEM_CRAM_COR_BITCNT     (XPLMI_PMCRAM_BASEADDR + 0X000140C0U)

#define PMC_RAM_CRAM_REGCNT  (15)

#define ERR_PRINT_TIMEOUT    (20000U)

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
#define PMC_SLR_MASTER      (0x6U)
#ifdef SDT
#define XSEM_SSIT_MAX_SLR_CNT	NUMBER_OF_SLRS
#endif /* End of SDT */
#else
#define PMC_SLR_MASTER       (0x7U)
#endif /* End of PLM_ENABLE_PLM_TO_PLM_COMM */

#define PMC_TAP_SLR_TYPE_ADDR    (0xF11A0024U)

#endif /* End of PLM_SEM_BEAM_TESTING */
/************************** Function Prototypes ******************************/
int XPlm_SemScanInit(void *Arg);
#ifdef PLM_SEM_BEAM_TESTING
int XPlm_SemCfrErrCntPrint(void *Data);
#endif /* End of PLM_SEM_BEAM_TESTING */
/************************** Variable Definitions *****************************/

/*****************************************************************************/

#endif /* End of XPLM_SEM */

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_INIT_SEM_H */
