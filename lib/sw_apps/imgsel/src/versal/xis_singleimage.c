/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.00  skd  01/13/23 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_config.h"
#include "xis_i2c.h"
#include "xis_error.h"
#include "xplmi_err_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_MAX_BOARDS 				(5U)
#define XIS_BOARDNAME_SIZE 			(6U)
#define XIS_BOARDNAME_OFFSET		(22U)

/************************** Function Prototypes ******************************/
static void XIs_UpdateMultiBootValue(u32 Offset);

/************************** Variable Definitions *****************************/
Boards_List Board[XIS_MAX_BOARDS] = {
	{"VPK120" , 1200},
	{"VMK180" , 1800},
	{"VCK190" , 1900},
	{"VPK180" , 1810},
	{"VEK280" , 2800}
};

static u8 ReadBuffer[XIS_MAX_SIZE]; /* Read buffer for reading a page. */

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
static void XIs_UpdateMultiBootValue(u32 Offset)
{
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_MULTI_BOOT, 0xFFFFFFFU, Offset);
}

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
		if(strncmp((char *)(ReadBuffer + XIS_BOARDNAME_OFFSET), Board[BoardIndex].Name,
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
 * This function is used to update the multiboot value
 * @param	None.
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

	sleep(1);
	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "***********Versal Image Selector***********\n\r");

	Status = XIs_GetBoardName(XIS_EEPROM_BOARD_ADDR_OFFSET,
					&BoardOffset, XIS_EEPROM_OFFSET_1_WRITE_BYTES);
	if(Status != XST_SUCCESS) {
		Status = XIs_GetBoardName(XIS_EEPROM_BOARD_ADDR_OFFSET,
					&BoardOffset, XIS_EEPROM_OFFSET_2_WRITE_BYTES);
		if(Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Board name not found\r\n");
			goto END;
		}
	}

	ReadBuffer[XIS_BOARDNAME_SIZE + 1U] = '\0';
	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Board name: %s\n\r", &ReadBuffer[XIS_BOARDNAME_OFFSET]);

	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Board offset: %x\r\n", BoardOffset);
	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "*******************************************\n\r");
	sleep(1);

	XIs_UpdateMultiBootValue(BoardOffset);

	if (Status == XST_SUCCESS) {
		XPlmi_SoftResetHandler();
	}

END:
	/* Ideally code should not reach here */
	return Status;
}

