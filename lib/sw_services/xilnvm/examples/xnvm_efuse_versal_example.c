/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xnvm_efuse_versal_example.c
 * @addtogroup xnvm_efuse_versal_example	XilNvm eFuse API Usage
 * @{
 *
 * This file illustrates Basic eFuse read/write using rows.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0	kal   08/16/2019 Initial release of xnvm_efuse_versal_example
 *
 * @note
 *	This example is using low level API's and this will be replaced by
 *	example using High level API's in future.
 *
 * User configurable parameters for eFuse
 *------------------------------------------------------------------------------
 *	#define 	XNVM_PROGRAM_ROWS		(FALSE)
 *	Default value is FALSE
 *	User should make this #define to TRUE if he wants to program any efuse
 *	Note : Please provide required inputs to program efuses. Those are
 *	XNVM_PGM_START_ROW, XNVM_PGM_NUM_OF_ROWS, XNVM_PGM_PAGE_NUM
 *	and define PgmUsrData with required pattern.
 *
 *	#define 	XNVM_PGM_START_ROW		(0x0U)
 *	This is the starting row of the eFuse that has to be written with
 *	User Data. Provide Row number in Hexa decimal value.
 *
 *	#define		XNVM_PGM_PAGE_NUM		(XNVM_EFUSE_PAGE_0)
 *	Default value is XNVM_EFUSE_PAGE_0
 *	User should provide the eFuse Page number.
 *
 *	#define 	XNVM_PGM_NUM_OF_ROWS		(0x0U)
 *	This is number of efuse rows to be programmed in single instance.
 *	Provide value in Hexa decimal value.
 *
 * 	#define 	XNVM_RD_START_ROW		(XNVM_DNA_ROW)
 * 	Default value is XNVM_DNA_ROW
 * 	User should provide the Read row number.
 *
 *	#define		XNVM_RD_PAGE_NUM		(XNVM_EFUSE_PAGE_0)
 *	Default value is XNVM_EFUSE_PAGE_0
 *	User should provide the eFuse Page number.
 *
 *	#define		XNVM_RD_NUM_OF_ROWS		(0x0U)
 *	This is the number of eFuse rows to be read in a single instance.
 *
 *	PgmUsrData - Pattern to be written in each one of
 *	the eFuse row. Provide start row value to be programmed in
 *	UsrDataBitMap_Wr[0] and next value to be programmed in
 *	UsrDataBitMap_Wr[1] etc.
 *	Example :
 *	PgmUsrData[XNVM_PGM_NUM_OF_ROWS] = {0x00000003, 0x00000040,...etc},
 *	Provide bit pattern to be programmed in Hexa decimal value.
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xnvm_efuse.h"

/* Configurable parameters for eFuse programming */
#define	XNVM_PROGRAM_ROWS		(FALSE)
#define XNVM_PGM_PAGE_NUM		(XNVM_EFUSE_PAGE_0)
#define XNVM_PGM_START_ROW		(0x0U)
#define XNVM_PGM_NUM_OF_ROWS		(0x0U)

#if defined(XNVM_PROGRAM_ROWS)  && defined (XNVM_PGM_NUM_OF_ROWS)
/* Provide bits to be written in Bitmap array */
u32 PgmUsrData[XNVM_PGM_NUM_OF_ROWS];
u32 ReadUsrData[XNVM_PGM_NUM_OF_ROWS];

#endif

/* Configurable parameters for eFuse read */
#define XNVM_DNA_ROW			(0x8U)
#define XNVM_NUM_DNA_ROWS		(0x4U)
#define XNVM_RD_PAGE_NUM		(XNVM_EFUSE_PAGE_0)
#define XNVM_RD_START_ROW		(XNVM_DNA_ROW)
#define XNVM_RD_NUM_OF_ROWS		(XNVM_NUM_DNA_ROWS)

u32 ReadEfuseData[XNVM_RD_NUM_OF_ROWS];

/************************** Constant Definitions ****************************/

#define XNVM_EFUSE_DEBUG_INFO		(1U)

/************************** Function Prototypes *****************************/

static u32 XNvm_EfuseReadExample(u8 ReadOption, u8 Row, u8 RowCount,
                        XNvm_EfuseType EfuseType, u32* RowData);

/************************** Function Definitions *****************************/

int main()
{
	u32 Status = XST_FAILURE;

	XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
			"App: Versal Efuse Example Start!!\r\n");
#if XNVM_PROGRAM_ROWS

	/* Program User data */
	Status = XNvm_EfusePgmRows(XNVM_PGM_START_ROW,
				XNVM_PGM_NUM_OF_ROWS,
				XNVM_PGM_PAGE_NUM,
				PgmUsrData);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
			"App: Program User Data Failed : 0x%04X\r\f", Status);
		goto END;
	}

	XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
		"App: Efuse Programming successful: 0x%04X\r\n", Status);

	XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
		"App: Read Programmed Efuse rows\r\n");
	/* Readback User data written */
	Status = XNvm_EfuseReadExample(XNVM_EFUSE_RD_FROM_CACHE,
					XNVM_PGM_START_ROW,
					XNVM_PGM_NUM_OF_ROWS,
					XNVM_PGM_PAGE_NUM,
					ReadUsrData);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
			"App: Read Programmed Efuse rows Failed : 0x%04X\r\f",
			Status);
		goto END;
	}
	XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,"\r\n");

#endif
	XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
			"App: Read Requested Efuse rows\r\n");

	Status = XNvm_EfuseReadExample(XNVM_EFUSE_RD_FROM_CACHE,
					XNVM_RD_START_ROW,
					XNVM_RD_NUM_OF_ROWS,
					XNVM_RD_PAGE_NUM,
					ReadEfuseData);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
			"App: Read Requested Efuse rows  Failed : 0x%04X\r\f",
			Status);
		goto END;
	}
	XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
		"App: Versal Efuse Example run successfully : 0x%04X\r\n",
		Status);
END:
	return Status;
}
/*****************************************************************************/
/**
* XNvm_EfuseReadExample function read requested Efuse rows and display the
* content.
*
* @return
*		- XST_SUCCESS if the eFuse read is successful
*		- XST_FAILURE if there is failure in eFuse read.
*
* @note		None.
*
******************************************************************************/
/** //! [XNvm eFuse example] */

static u32 XNvm_EfuseReadExample(u8 ReadOption, u8 Row,
				u8 RowCount, XNvm_EfuseType EfuseType,
				u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 Idx;

	Status = XNvm_EfuseReadRows(ReadOption, Row,
				RowCount, EfuseType,
				RowData);
	if (Status != XST_SUCCESS) {
                goto END;
        }

	for (Idx = 0; Idx < RowCount; Idx++) {

                XNvm_Printf(XNVM_EFUSE_DEBUG_INFO,
                                "Efuse Row %x : 0x%08X\r\n",
                                (Row + Idx),
                                RowData[Idx]);
        }
END:
	return Status;
}

/** //! [XNvm eFuse example] */
/** @} */
