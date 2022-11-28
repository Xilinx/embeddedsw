/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_singleimage.c
*
* This is the file which contains code for Versal Image Selector single image.
*
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  bsv  10/03/22 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_main.h"
#include "xplmi_err_common.h"
#include "xplmi_util.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_MAX_BOARDS 				(3U)
#define XIS_BOARDNAME_SIZE 			(6U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
Boards_List Board[XIS_MAX_BOARDS] = {
	{"VPK120" , 120},
	{"VPK180" , 180},
	{"VCK190",  190},
};

static u8 ReadBuffer[256]; /* Read buffer for reading a page. */

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
int XIs_GetBoardName(u16 ReadAddress, u16 *Offset,u32 WrBfrOffset)
{
	int Status = XST_FAILURE;
	u32 BoardIndex;

	/*
	 * Read from the EEPROM.
	 */
	Status = XIs_EepromReadData(ReadBuffer, ReadAddress,
					XIS_PAGE_SIZE_16, WrBfrOffset);
	if (Status != XST_SUCCESS) {
		Status = XIS_EEPROM_READ_ERROR;
		goto END;
	}

	for(BoardIndex = 0U; BoardIndex < XIS_MAX_BOARDS; BoardIndex++) {
		if(strncmp((char *)(ReadBuffer + 0x16U), Board[BoardIndex].Name,
				XIS_BOARDNAME_SIZE) == 0U) {
			*Offset = Board[BoardIndex].Offset;
			goto END;
		}
	}
	Status = XIS_BOARD_NAME_NOTFOUND_ERROR;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to update the multiboot value.
 *
 * @return	returns XIS_BOARD_NAME_NOTFOUND on failure
 *				returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
int XIs_ImageSelBoardParam(void)
{
	int Status = XST_FAILURE;
	u16 BoardOffset;

	Status = XIs_IicPsMuxInit();
	if (Status != XST_SUCCESS) {
		Status = XIS_MUX_INIT_ERROR;
		goto END;
	}

	Status = XIs_GetBoardName(XIS_EEPROM_BOARD_ADDR_OFFSET_1,
					&BoardOffset, XIS_EEPROM_OFFSET_1_WRITE_BYTES);
	if(Status != XST_SUCCESS) {
		Status = XIs_GetBoardName(XIS_EEPROM_BOARD_ADDR_OFFSET_2,
					&BoardOffset, XIS_EEPROM_OFFSET_2_WRITE_BYTES);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "Board Name NotFound\r\n");
			goto END;
		}
	}

	sleep(1);
	XIs_UpdateMultiBootValue(BoardOffset);

	if (Status == XST_SUCCESS) {
		XPlmi_SoftResetHandler();
	}

END:
	/* Ideally code should not reach here */
	return Status;
}

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
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_MULTI_BOOT, 0xFFFFFFFU, Offset);
}
