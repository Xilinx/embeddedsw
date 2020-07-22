/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xprefsbl_singleimage.c
*
* This is the file which contains code for the Pre-FSBL single image.
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
* 1.00  Ana   	24/06/20 	First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xprefsbl_main.h"

#ifdef XPREFSBL_GET_BOARD_PARAMS
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPREFSBL_MAX_BOARDS 				(4U)
#define XPREFSBL_BOARDNAME_SIZE 			(6U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
Boards_List Board[4U] = {
	{"ZCU102" , 1020U},

	{"ZCU104" , 1040U},

	{"ZCU106" , 1060U},

	{"ZCU111" , 1110U}
};

/************************** Function Definitions *****************************/
int XPrefsbl_UpdateMultiBootRegister(u8 *ReadBuffer)
{
	int Status = XPREFSBL_BOARD_NAME_NOTFOUND;
	u32 BoardIndex;

	for(BoardIndex = 0U; BoardIndex < XPREFSBL_MAX_BOARDS; BoardIndex++) {
		if(strncmp((char *)ReadBuffer, Board[BoardIndex].Name,
				XPREFSBL_BOARDNAME_SIZE) == 0U) {
			XPrefsbl_UpdateMultiBootValue(Board[BoardIndex].Offset);
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}
#endif