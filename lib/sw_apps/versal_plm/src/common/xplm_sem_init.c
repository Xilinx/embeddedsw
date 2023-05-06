/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_sem_init.c
*
* This file contains the SEM Init APIs.
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
*       hb   10/29/2020 Updated OwnerId for NPI scan scheduler
* 1.02  rb   10/30/2020 Updated XilSEM Init API
* 1.03  rb   01/28/2021 Added Sem PreInit API to have CDO command handler
*                       initialization, removed unused header file
*       rb   03/09/2021 Updated Sem Init API call
* 1.03  ga   05/03/2023 Removed XPlm_SemInit function and updated
*                       copyright information
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xplm_sem_init.h"

#ifdef XPLM_SEM
#include "xilsem.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function calls the scan init functions of XilSEM.
 *		As a part of this, XilSEM library can initiate
 *		scan on both CRAM and NPI.
 *		Scan will be decided based on the CIPS params in xparameters.h.
 *
 * @param	Arg is not used
 *
 * @return	Status as defined in XilSEM library
 *
 *****************************************************************************/
int XPlm_SemScanInit(void *Arg)
{
	int Status = XST_FAILURE;
	(void)Arg;

	Status = XSem_InitScan();

	return Status;
}
#endif
