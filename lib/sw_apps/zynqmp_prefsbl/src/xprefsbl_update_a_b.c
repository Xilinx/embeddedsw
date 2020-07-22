/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xprefsbl_update_a_b.c
*
* This is the file which contains code for the Pre-FSBL update A/B mechanism.
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
* 1.00  Ana	   24/06/20 	First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xprefsbl_main.h"

#ifdef XPREFSBL_UPDATE_A_B_MECHANISM
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XPrefsbl_UpdateABMultiBootValue(void)
{
	int Status = XST_FAILURE;
	u32 *PerstRegPtr;
	u32 Offset;
	u8 CurrentImage;
	u8 ReadDataBuffer[XPREFSBL_SIZE_4K] __attribute__ ((aligned(32U)));

	Status = XPreFsbl_QspiInit();
	if (Status != XST_SUCCESS) {
		XPreFsbl_Printf(DEBUG_GENERAL, "QSPI Init failed\r\n");
		goto END;
	}

	Status = XPreFsbl_QspiRead(XPREFSBL_PERS_REGISTER_BASE_ADDRESS,
					(u8 *)ReadDataBuffer, XPREFSBL_SIZE_4K);
	if (Status != XST_SUCCESS) {
		XPreFsbl_Printf(DEBUG_GENERAL, "QSPI Read failed\r\n");
		goto END;
	}

	/**
	 * If Image A and Image B are both are marked as bootable,
	 * Requested image will be loaded
	 * If Image A or Image B any one of the image is marked as bootable,
	 * Requested image is not bootable bootable image is loaded
	 * If Last Image booted is non-bootable, bootable image will be loaded
	 * If None of the Image is marked as bootable, recovery image is loaded
	 */
	if((ReadDataBuffer[XPREFSBL_IMAGE_A_BOOTABLE] == TRUE) ||
			(ReadDataBuffer[XPREFSBL_IMAGE_B_BOOTABLE] == TRUE)) {

		if((ReadDataBuffer[XPREFSBL_LAST_BOOTED_IMAGE] == XPREFSBL_IMAGE_A) &&
					(ReadDataBuffer[XPREFSBL_IMAGE_A_BOOTABLE] == FALSE)) {
			CurrentImage = XPREFSBL_IMAGE_B;
			PerstRegPtr = (u32 *)&ReadDataBuffer[XPREFSBL_IMAGE_B_OFFSET];
		}
		else if((ReadDataBuffer[XPREFSBL_LAST_BOOTED_IMAGE]
					== XPREFSBL_IMAGE_B) &&
					(ReadDataBuffer[XPREFSBL_IMAGE_B_BOOTABLE] == FALSE)){
			CurrentImage = XPREFSBL_IMAGE_A;
			PerstRegPtr = (u32 *)&ReadDataBuffer[XPREFSBL_IMAGE_A_OFFSET];
		}
		else {
			if(ReadDataBuffer[XPREFSBL_REQUESTED_BOOT_IMAGE]
					== XPREFSBL_IMAGE_A) {
				CurrentImage = XPREFSBL_IMAGE_A;
				PerstRegPtr = (u32 *)&ReadDataBuffer[XPREFSBL_IMAGE_A_OFFSET];
			}
			else {
				CurrentImage = XPREFSBL_IMAGE_B;
				PerstRegPtr = (u32 *)&ReadDataBuffer[XPREFSBL_IMAGE_B_OFFSET];
			}
		}
	}
	else {
		PerstRegPtr = (u32 *)&ReadDataBuffer[XPREFSBL_RECOVERY_IMAGE_OFFSET];
		Offset = (u32)(*PerstRegPtr / XPREFSBL_SIZE_32KB);
		XPrefsbl_UpdateMultiBootValue(Offset);
		goto END;
	}

	Offset = (u32)(*PerstRegPtr / XPREFSBL_SIZE_32KB);
	XPrefsbl_UpdateMultiBootValue(Offset);

	if(ReadDataBuffer[XPREFSBL_LAST_BOOTED_IMAGE] != CurrentImage) {
		ReadDataBuffer[XPREFSBL_LAST_BOOTED_IMAGE] = CurrentImage;
		Status = XPreFsbl_QspiWrite(XPREFSBL_PERS_REGISTER_BASE_ADDRESS,
						(u8 *)ReadDataBuffer, XPREFSBL_SIZE_4K);
		if(Status != XST_SUCCESS) {
			XPreFsbl_Printf(DEBUG_GENERAL, "QSPI Last image booted"
                                  " Write failed\r\n");
			goto END;
		}

		Status = XPreFsbl_QspiWrite(XPREFSBL_PERS_REGISTER_BACKUP_ADDRESS,
						(u8 *)ReadDataBuffer, XPREFSBL_SIZE_4K);
		if(Status != XST_SUCCESS) {
			XPreFsbl_Printf(DEBUG_GENERAL, "QSPI Last image booted"
                                  " Backup Write failed\r\n");
			goto END;
		}
	}

END:
	return Status;
}
#endif