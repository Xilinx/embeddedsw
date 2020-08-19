/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_util.c
*
* This file provides utility functions
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "string.h"
#include "xbir_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
 * This function extracts the extension of the input file name
 *
 * @param	FileName	Pointer to file name
 *
 * @return	Pointer to extension in file name
 *
 *****************************************************************************/
const char* Xbir_UtilGetFileExt (const char *FileName)
{
	const char *Ext = NULL;
	const char *SearchPtr = FileName + strlen(FileName) - 1U;

	while (SearchPtr > FileName) {
		if (*SearchPtr == '.') {
			Ext = SearchPtr + 1U;
			break;
		}
		SearchPtr--;
	}

	return Ext;
}

/*****************************************************************************/
/**
 * @brief
 * This function validates input string represents number in string format
 *
 * @param	FileName	Pointer to input string
 *
 * @return	XST_SUCCESS if input string represents number
 * 		XST_FAILURE if input string do not represent number
 *
 *****************************************************************************/
u8 Xbir_UtilIsNumber (const char *Str)
{
	u8 Status = FALSE;
	const char *NumStr = Str;

	while ((*NumStr) != 0U) {
		if (((*NumStr) < '0') || ((*NumStr) > '9')) {
			goto END;
		}
		NumStr++;
	}
	Status = TRUE;

END:
	return Status;
}
