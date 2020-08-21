/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xstl_plminterface.h
*
* This file contains the header functions of STL PLM interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rama 08/20/2020 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifdef VERSAL_PLM
#ifndef XSTL_PLMINTERFACE_H
#define XSTL_PLMINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define XSTL_ID_REGINIT		0x0U
#define XSTL_ID_REGCHECK	0x1U
#define XSTL_ID_INTCCHECK	0x2U

#define XSTL_ARGS_LEN 			4U
#define XSTL_NUM_STLS_ON_PMC	4U
#define XSTL_EXECUTION_EN		1U
#define XSTL_EXECUTION_DIS		0U
#define XSTL_PERIODICTY			100U

/**************************** Type Definitions *******************************/
typedef struct {
	int (*StlFuncPtr)(void);
	u32 *ArgListPtr;
	u32 Enable;
} XStl_PmcStlList;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XStl_Init();

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XSTL_PLMINTERFACE_H */
#endif /* End of __MICROBLAZE__ macro */
