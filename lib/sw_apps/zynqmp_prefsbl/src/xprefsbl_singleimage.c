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
#define XPREFSBL_MAX_BOARDS 				(6U)
#define XPREFSBL_BOARDNAME_SIZE 			(6U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
Boards_List Board[XPREFSBL_MAX_BOARDS] = {
	{"ZCU102" , 1020U},

	{"ZCU104" , 1040U},

	{"ZCU106" , 1060U},

	{"ZCU111" , 1110U},

	{"ZCU208" , 2080U},

	{"ZCU216" , 2160U}

};

static u8 ReadBuffer[XPREFSBL_MAX_SIZE]; /* Read buffer for reading a page. */

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * This function is used to Get the board specific information from IIC EEPROM
 * @param	ReadAddress  Board specific parameters address location
 * 			Offset		 Multiboot offset value
 *			WrBfrOffset  No.Of IIC Address bytes
 *
 * @return	returns the error codes described in xpfsbl_error.h on any error
 *				returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
int XPreFsbl_GetBoardName(u16 ReadAddress, u16 *Offset,u32 WrBfrOffset)
{
	int Status = XST_FAILURE;
	u32 BoardIndex;

	/*
	 * Read from the EEPROM.
	 */
	Status = XPrefsbl_EepromReadData(ReadBuffer, ReadAddress,
					XPREFSBL_PAGE_SIZE_16, WrBfrOffset);
	if (Status != XST_SUCCESS) {
		Status = XPREFSBL_EEPROM_READ_ERROR;
		goto END;
	}

	for(BoardIndex = 0U; BoardIndex < XPREFSBL_MAX_BOARDS; BoardIndex++) {
		if(strncmp((char *)ReadBuffer, Board[BoardIndex].Name,
				XPREFSBL_BOARDNAME_SIZE) == 0U) {
			*Offset = Board[BoardIndex].Offset;
			goto END;
		}
	}
	Status = XPREFSBL_BOARD_NAME_NOTFOUND;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to update the multiboot value
 * @param	None.
 *
 * @return	returns XPREFSBL_BOARD_NAME_NOTFOUND on failure
 *				returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
int XPrefsbl_ImageSelBoardParam(void)
{
	int Status = XST_FAILURE;
	u16 BoardOffset;

	Status = XPrefsbl_IicPsMuxInit();
	if (Status != XST_SUCCESS) {
		Status = XPREFSBL_MUX_INIT_ERROR;
		goto END;
	}

	Status = XPreFsbl_GetBoardName(XPREFSBL_EEPROM_BOARD_ADDR_OFFSET_1,
					&BoardOffset, XPREFSBL_EEPROM_OFFSET_1_WRITE_BYTES);
	if(Status != XST_SUCCESS) {
		Status = XPreFsbl_GetBoardName(XPREFSBL_EEPROM_BOARD_ADDR_OFFSET_2,
					&BoardOffset, XPREFSBL_EEPROM_OFFSET_2_WRITE_BYTES);
		if(Status != XST_SUCCESS) {
			XPreFsbl_Printf(DEBUG_GENERAL, "Pre-FSBL Board Name NotFound\r\n");
			goto END;
		}
	}
	XPrefsbl_UpdateMultiBootValue(BoardOffset);

END:
	return Status;
}
#endif