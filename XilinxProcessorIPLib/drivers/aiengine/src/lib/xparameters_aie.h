/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xparameters_aie.h
* @{
*
* This file contains stub xparameter definitions for ME.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/27/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Naresh  07/26/2018  Set num instances to 1 to avoid segmentation fault
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
/*********************************************
* 
*********************************************/

#ifndef XPARAMETERSAIE_H   /* prevent circular inclusions */
#define XPARAMETERSAIE_H   /* by using protection macros */

#define XPAR_AIE_NUM_INSTANCES	1
#define XPAR_AIE_DEVICE_ID	1
#define XPAR_AIE_ARRAY_OFFSET	0
#define XPAR_AIE_NUM_ROWS	32      /* 2^5 */
#define XPAR_AIE_NUM_COLUMNS	128     /* 2^7 */     

#endif  /* end of protection macro */
