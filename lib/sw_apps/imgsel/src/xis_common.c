/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_common.c
*
* This is the file which contains error update functionality,
* multiboot value update functionality and soft reset functionality
*
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  		Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana   	18/06/20 	First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_main.h"

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This functions updates the multiboot value into CSU_MULTIBOOT register.
 *
 * @param	Multiboot offset Value.
 *
 * @return	None.
 *
 ******************************************************************************/
void XIs_UpdateMultiBootValue(u32 Offset)
{
	XIs_Out32(XIS_CSU_MULTI_BOOT, Offset);
}

/*****************************************************************************/
/**
 * This function will do soft reset.
 *
 * @param	None.
 *
 * @return	None.
 *
 ******************************************************************************/
void XIs_Softreset(void)
{
	XIs_Out32(XIS_CRL_APB_RESET_CTRL, XIS_CSU_APB_RESET_VAL);
}

/*****************************************************************************/
/**
 * This function updates the error value into error status register.
 *
 * @param	Error Value.
 *
 * @return	None.
 *
 ******************************************************************************/
void XIs_UpdateError(int Error)
{
	XIs_Out32(XIS_ERROR_STATUS_REGISTER_OFFSET, ((u32)Error << 16U));
}