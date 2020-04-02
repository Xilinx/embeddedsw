/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ecdsa.c
*
* This file contains the implementation of the interface functions for ECDSA
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rpo 03/31/2020 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_error.h"
#include "xsecure_ecdsa.h"
#include "xsecure_ecdsa_rsa_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on ECDSA core.
 *
 * @return
 *		- Returns XST_SUCCESS on success.
 *		- Returns error code on failure
 *
 *****************************************************************************/
u32 XSecure_EcdsaKat(void)
{
	u32 Status = XSECURE_ECC_KAT_FAILED_ERROR;
	u32 QxCord[XSECURE_ECC_DATA_SIZE_WORDS] = {
		0x88371BE6U, 0xFD2D8761U, 0x30DA0A10U, 0xEA9DBD2EU,
		0x30FB204AU, 0x1361EFBAU, 0xF9FDF2CEU, 0x48405353U,
		0xDE06D343U, 0x335DFF33U, 0xCBF43FDFU, 0x6C037A0U
	};

	u32 QyCord[XSECURE_ECC_DATA_SIZE_WORDS] = {
		0xEA662A43U, 0xD380E26EU, 0x57AA933CU, 0x4DD77035U,
		0x5891AD86U, 0x7AB634EDU, 0x3E46D080U, 0xD97F2544U,
		0xBF70B8A4U, 0x9204B98FU, 0x940E3467U, 0x360D38F3U
	};

	u32 SignR[XSECURE_ECC_DATA_SIZE_WORDS] = {
		0x52D853B5U, 0x41531533U, 0x2D1B4AA6U, 0x6EAF0088U,
		0x4E88153DU, 0x9F0AB1AAU, 0x12A416D8U, 0x7A50E599U,
		0xB7CA0FA0U, 0x330C7507U, 0x3495767EU, 0x5886078DU
	};

	u32 SignS[XSECURE_ECC_DATA_SIZE_WORDS] = {
		0x7A36E1AAU, 0x329682AEU, 0xE17F691BU, 0xF3869DA0U,
		0xE32BDE69U, 0x6F78CDC4U, 0x89C8FF9FU, 0x449A3523U,
		0x82CC2114U, 0xFD14B06BU, 0xBF1BF8CCU, 0x2CC10023U
	};

	u32 HashVal[XSECURE_ECC_DATA_SIZE_WORDS] = {
		0x925FA874U, 0x331B36FBU, 0x13173C62U, 0x57633F17U,
		0x110BA0CDU, 0x9E3B9A7DU, 0x46DE70D2U, 0xB30870DBU,
		0xF3CA965DU, 0xADAA0A68U, 0x9573A993U, 0x1128C8B0U
	};

	/*
	 * Take the core out of reset
	 */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Status = P384_validatekey((u8 *)QxCord, (u8 *)QyCord);
	if(Status != XST_SUCCESS) {
		Status = XSECURE_ECC_KAT_KEY_NOTVALID_ERROR;
		goto END;
	}
	Status = P384_ecdsaverify((u8 *)HashVal, (u8 *)QxCord, (u8 *)QyCord,
				 (u8 *)SignR, (u8 *)SignS);
	if(Status != XST_SUCCESS) {
		Status = XSECURE_ECC_KAT_FAILED_ERROR;
		goto END;
	}

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}
