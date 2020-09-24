/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilskey_puf_registration_example.c
 *
 * This file illustrates how to do PUF registration and generating syndrome
 * data and obtaining helper data, debug 2 result, Auxiliary and CHash values
 * from generated syndrome data.
 * This example also encrypts the provided AES key with PUF and generates
 * the black key.
 * Finally programs the PUF code and black key into eFUSE on user request
 * (provided in xilskey_puf_registration.h)
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ---  -------- -------------------------------------------------------
 * 6.1   rp   17/10/16 First release.
 * 6.2   vns  02/18/17 Modified Encrypt API call due to change in Xilsecure
 *            03/10/17 Added Support for programming and reading PUF reserved
 *                     bit
 * 6.7   mmd  03/17/19 Ignored PUF data on overflow
 * 6.9   kpt  02/27/20 Removed XilSKey_Puf_Fetch_Dbg_Mode2_result
 *                     which is used only for debug purpose
 *       har  03/09/20 Fixed array overrun condition in
 *                     XilSKey_Generate_FuseFormat function.
 *       kal  05/15/20 Replace all eFuse read with Cache reads.
 * 7.0   kpt  09/02/20 Added successfully ran print to the example in case of
 *                     success
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xilskey_puf_registration.h"
#include "xplatform_info.h"

/************************** Constant Definitions ****************************/

#define	XSK_PUF_SYN_DATA_BYTES_FOR_4K			(560U)
#define	XSK_PUF_LAST_WORD_OFFSET			(126U)
#define	XSK_PUF_FORMATTED_SYN_SIZE_WORDS		(127U)
#define	XSK_PUF_DEVICE_KEY				(1U)
#define	XSK_PUF_EFUSE_TRIM_MASK				(0xFFFFF000U)
#define	XSK_PUF_LAST_WORD_MASK				(0xFFFFFFF0U)

#define XSK_AES_KEY_LENGTH				(64)
#define XSK_IV_LENGTH					(24)

#define XSK_EFUSEPS_BASEADDR				0xFFCC0000U
#define XSK_EFUSEPS_SYSOSC_OFFSET			0x0000101CU
#define XSK_EFUSEPS_SYSOSC_EN_MASK			0x00000001U

/************************** Type Definitions ********************************/

/* Error codes */
typedef enum {
	XPUF_REGISTRATION_WO_AUTH_ERROR = (0x2000U),
	XPUF_FUSE_12K_NOT_ALLOWED_ERROR = (0x2001U),
	XPUF_STRING_INVALID_ERROR = (0x2002U),
	XPUF_PARAMETER_NULL_ERROR= (0x2003U),

	XPUF_DMA_CONFIG_FAILED_ERROR = (0x2004U),
	XPUF_DMA_INIT_FAILED_ERROR = (0x20005U),
	XPUF_AES_INIT_FAILED_ERROR = (0x2006U),
	XPUF_ENCRYPTION_FAILED_ERROR = (0x2007U),
	XPUF_ZERO_BLACK_KEY_ERROR = (0x2008U)
}Xsk_PufErrocodes;

/************************** Function Prototypes ******************************/

#if defined XPUF_FUSE_SYN_DATA
static void XilSKey_Generate_FuseFormat(XilSKey_Puf *InstancePtr);
static u32 XilSkey_Program_Black_Key();
#endif
static u32 XilSKey_Puf_ConvertStringToHexBE(const char * Str,
					u8 * Buf, u32 Len);
static u32 XilSKey_Puf_Encrypt_Key();

/************************** Function Definitions *****************************/

int main() {

	u32 Status;
	u32 Index;
	u32 IsBlackKeyZero = TRUE;
	u32 SiliconVer = XGetPSVersion_Info();

	xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: Example is running on Silicon version %d.0\n\r",
						(SiliconVer + 1));

	PufInstance.RegistrationMode = XSK_PUF_REG_MODE;

#if defined XPUF_CONTRACT_MANUFACTURER
	XilSKey_SecCtrlBits Efuse_SecCtrlBits;
	/* Read use RSA bits value from eFUSE */
	Status = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(&Efuse_SecCtrlBits, 0);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL, "App: Error occurred"
			"while reading secure control bits of eFUSE\n\r");
		/* If reading secure control bits of eFUSE is failed */
		goto ENDF;
	}
	/* Check whether RSA authentication is enabled or not */
	if (Efuse_SecCtrlBits.RSAEnable == 0) {
		Status = XPUF_REGISTRATION_WO_AUTH_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App:Registration not allowed w/o "
			"Authentication:0x%08x\r\n", Status);
		goto ENDF;

	}
	else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App:Registration at Contract Manufacturer's site!!!\r\n");
	}
#endif /* XPUF_CONTRACT_MANUFACTURER */

	/* Request PUF for registration */
	Status = XilSKey_Puf_Registration(&PufInstance);
	if (Status == XSK_EFUSEPS_ERROR_PUF_DATA_OVERFLOW) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App:PUF Registration completed with an overflow:0x%08x\r\n",
								Status);
		goto ENDF;
	}
	else if ((Status != XST_SUCCESS)) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App:PUF Registration Failed:0x%08x\r\n", Status);
		goto ENDF;
	} else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App:PUF Registration Completed:0x%08x\r\n", Status);
	}

#if defined XPUF_INFO_ON_UART
	/* Send syndrome data on UART (if enabled) */
	u32 Subindex;
	u8 *Buffer;
	u32 SynIndex;

	xPuf_printf(XPUF_DEBUG_GENERAL, "App:PUF Syndrome data Start!!!\r\n");
	for (SynIndex = 0; SynIndex < XSK_ZYNQMP_PUF_SYN_LEN; SynIndex++) {
		Buffer = (u8*) &(PufInstance.SyndromeData[SynIndex]);

		for (Subindex = 0; Subindex < 4; Subindex++) {
			xPuf_printf(XPUF_DEBUG_GENERAL, "%02x",
						Buffer[Subindex]);
		}
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
	xPuf_printf(XPUF_DEBUG_GENERAL, "App:PUF Syndrome data End!!!\r\n");
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AUX-%08x\r\n", PufInstance.Aux);
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: CHASH -%08x\r\n",
					PufInstance.Chash);
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: ShutterValue -%08x\r\n",
					PufInstance.ShutterValue);

#endif /*XPUF_INFO_ON_UART*/

	/* Request AES engine to encrypt the AES key using PUF Key */
	Status = XilSKey_Puf_Encrypt_Key();
	if (Status != XST_SUCCESS) {
		Status = XPUF_ENCRYPTION_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App:Key encryption failed:0x%08x\r\n", Status);
		goto ENDF;
	}
	for (Index = 0; Index < XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES;
								Index++) {
		if (PufInstance.BlackKey[Index] != 0x00U) {
			IsBlackKeyZero = FALSE;
			break;
		}
	}
	/* If Black key is zero, don't program eFuse with helper data */
	if (IsBlackKeyZero) {
		Status = XPUF_ZERO_BLACK_KEY_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: Zero Black Key:0x%08x\r\n", Status);
		goto ENDF;
	}
#if defined XPUF_FUSE_SYN_DATA
	u32 SynReadData[128];
	u32 Aux;
	u32 Chash;

	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Programming eFUSE \r\n");

	/* Convert the syndrome data into eFUSE writing format */
	if(PufInstance.RegistrationMode == XSK_PUF_MODE4K) {
		/*
		 * Syndrome data generated during PUF registration is
		 * converted into eFUSE format.
		 */
		(void)XilSKey_Generate_FuseFormat(&PufInstance);

		/* Writes PUF helper data into eFUSE */
		Status = XilSKey_ZynqMp_EfusePs_WritePufHelprData(&PufInstance);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: Helper data writing failed\r\n");
			goto ENDF;
		}
		else {
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: Syndrome write successful\r\n");
		}

		/*
		 * Reads PUF helper data from eFUSE and
		 * compares with programmed data
		 */
		Status = XilSKey_ZynqMp_EfusePs_ReadPufHelprData(SynReadData);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: Helper data read failed\r\n");
			goto ENDF;
		}
		for (Index = 0; Index < XSK_PUF_FORMATTED_SYN_SIZE_WORDS;
							Index++) {
			if (PufInstance.EfuseSynData[Index] !=
						SynReadData[Index]) {
				xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: Helper data read compare"
					" failed@%d:W-%08x, R-%08x\r\n",
					Index, PufInstance.EfuseSynData[Index],
						SynReadData[Index]);
				goto ENDF;
			}
		}

		/* Programming Auxiliary data into eFUSE */
		Status = XilSKey_ZynqMp_EfusePs_WritePufAux(&PufInstance);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: Aux write failed\r\n");
			goto ENDF;
		}
		else {
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: Aux write successful\r\n");
		}

		/* programming CHash into eFUSE */
		Status = XilSKey_ZynqMp_EfusePs_WritePufChash(&PufInstance);
		if (Status != XST_SUCCESS)	{
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: CHASH write failed\r\n");
			goto ENDF;
		}
		else {
			xPuf_printf(XPUF_DEBUG_GENERAL,
					"App: CHASH write successful\r\n");
		}

		/* Programs and verifies the black key */
		Status = XilSkey_Program_Black_Key();
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Writing Black key is failed\r\n");
			goto ENDF;
		}

		Status = XilSKey_ZynqMp_EfusePs_ReadPufAux(&Aux,
					XSK_EFUSEPS_READ_FROM_CACHE);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Reading Aux value is failed\r\n");
			goto ENDF;
		}

		/* Comparing Aux value */
		if (PufInstance.Aux != Aux) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Aux read not matched W:%08x,R:%08x\r\n",
					PufInstance.Aux, Aux);
			goto ENDF;
		}
		Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&Chash,
						XSK_EFUSEPS_READ_FROM_CACHE);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Reading Aux value is failed\r\n");
			goto ENDF;
		}
		/* Comparing programmed CHash value */
		if (PufInstance.Chash != Chash) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Chash read not matched W:%08x,R:%08x\r\n",
					PufInstance.Chash, Chash);
			goto ENDF;
		}
	}
	else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: Registration mode should be 4K to fuse\r\n");
		Status = XPUF_FUSE_12K_NOT_ALLOWED_ERROR;
		goto ENDF;
	}
#endif /*XPUF_FUSE_SYN_DATA*/

#if (XSK_PUF_PROGRAM_SECUREBITS == TRUE)
		/* Programs the PUF secure bits */
	XilSKey_Puf_Secure	PgmPufSecureBits;
		xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Writing eFUSE PUF secure bits\r\n");
		PgmPufSecureBits.SynInvalid = XSK_PUF_SYN_INVALID;
		PgmPufSecureBits.SynWrLk = XSK_PUF_SYN_WRLK;
		PgmPufSecureBits.RegisterDis = XSK_PUF_REGISTER_DISABLE;
		PgmPufSecureBits.Reserved = XSK_PUF_RESERVED;
		Status = XilSKey_Write_Puf_EfusePs_SecureBits(
					&(PgmPufSecureBits));
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: Writing eFUSE secure bits of PUF is failed\r\n");
			goto ENDF;
		}
#endif

#if (XSK_PUF_READ_SECUREBITS == TRUE)
	XilSKey_Puf_Secure	PufSecureBits;

	Status = XilSKey_Read_Puf_EfusePs_SecureBits(
			&(PufSecureBits), XSK_EFUSEPS_READ_FROM_CACHE);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: Failed while reading PUF secure bits\r\n");
		goto ENDF;
	}
	if (PufSecureBits.RegisterDis == TRUE) {
		xil_printf("Disabled the ability to"
			" register a new PUF key\n\r");
	}
	else {
		xil_printf("The ability to register a new PUF key"
					" is not disabled \n\r");
	}
	if (PufSecureBits.SynInvalid == TRUE) {
		xil_printf("invalidated PUF syndrome data in the eFUSE\n\r");
	}
	else {
		xil_printf("PUF syndrome data in the eFUSE is not "
						"invalidated\n\r");
	}
	if (PufSecureBits.SynWrLk == TRUE) {
		xil_printf("Writing to the PUF syndrome eFuses is locked\n\r");
	}
	else {
		xil_printf("Writing to the PUF syndrome eFuses is not"
						" locked\n\r");
	}
	if (PufSecureBits.Reserved == TRUE) {
		xil_printf("Reserved bit is programmed\n\r");
	}
	else {
		xil_printf("Reserved bit is not programmed\n\r");
	}

#endif

ENDF:
	if (Status != XST_SUCCESS) {
		xil_printf("xilskey puf registration example failed with"
					"Status:%08x\r\n", Status);
	}
	else {
		xil_printf("Successfully ran xilskey puf registration example...");
	}
	/**
	 * TBD: Move the Status code to the persistent register
	 */
	for (;;)
		;
	return 0;
}

#if defined XPUF_FUSE_SYN_DATA
/*****************************************************************************/
/**
 *
 * Converts the PUF Syndrome data to eFUSE writing format.
 *
 * @param	InstancePtr is a pointer to the XilSKey_Puf instance.
 *
 * @return	None.
 *
 * @note	Formatted data will be available @ InstancePtr->EfuseSynData.
 *
 ******************************************************************************/
static void XilSKey_Generate_FuseFormat(XilSKey_Puf *InstancePtr)
{

	u32 SynData[XSK_ZYNQMP_PUF_FORMATTED_SYN_DATA_LEN_IN_BYTES] = {0};
	u32 SIndex = 0;
	u32 DIndex = 0;
	u32 Index;
	u32 SubIndex;

	memcpy(SynData, InstancePtr->SyndromeData,
				XSK_PUF_SYN_DATA_BYTES_FOR_4K);

	/**
	 * Trimming logic for PUF Syndrome Data:
	 * ------------------------------------
	 * Space allocated in eFUSE for syndrome data = 4060bits
	 * eFUSE02 - 2032bits
	 * eFUSE03 - 2028bits
	 *
	 * PUF Helper data generated for 4K Mode through registration
	 * is 140 Words = 140*32 = 4480 bits.
	 * Remove lower 12 bits of every fourth word of syndrome data.
	 *
	 *  After removing these bits remaining syndrome data will be
	 *  exactly 4060bits which will fit into eFUSE.
	 *
	 *
	 *	Illustration:
	 *
	 *	Input
	 *	-----
	 * 454D025B
	 * CDCB36FC
	 * EE1FE4C5
	 * 3FE53F74 --> F74 has to removed &
	 * 3A0AE7F8	next word upper 12 bits have to be shifted here
	 * 2373F03A
	 * C83188AF
	 * 3A5EB687--> 687 has to be removed
	 * B83E4A1D
	 * D53B5C50
	 * FA8B33D9
	 * 07EEFF43 --> F43 has to be removed
	 * CD01973F
	 * ........
	 * ........
	 * ........
	 */

	for (Index = 0; Index < 5; Index++) {
		for (SubIndex = 0; SubIndex < 4; SubIndex++) {
			if (SubIndex == 3) {
				InstancePtr->EfuseSynData[DIndex] =
				(SynData[SIndex] & XSK_PUF_EFUSE_TRIM_MASK) |
				(SynData[SIndex+1] >> 20);
			}
			else {
				InstancePtr->EfuseSynData[DIndex] =
							SynData[SIndex];
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0; SubIndex < 4; SubIndex++) {
			if (SubIndex == 3) {
				InstancePtr->EfuseSynData[DIndex] =
					(((SynData[SIndex] &
					XSK_PUF_EFUSE_TRIM_MASK) << 12) |
						SynData[SIndex+1] >> 8);
			}
			else {
				InstancePtr->EfuseSynData[DIndex] =
				((SynData[SIndex] << 12) |
						SynData[SIndex+1] >> 20);
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0; SubIndex < 3; SubIndex++) {
			if (SubIndex == 2) {
				InstancePtr->EfuseSynData[DIndex] =
					((SynData[SIndex] << 24) |
					(SynData[SIndex+1] &
						XSK_PUF_EFUSE_TRIM_MASK) >> 8);
				if (DIndex < (XSK_PUF_FORMATTED_SYN_SIZE_WORDS - 1)) {
					InstancePtr->EfuseSynData[DIndex] |=
						(SynData[SIndex+2] >> 28);
				}
			}
			else {
				InstancePtr->EfuseSynData[DIndex]=
					((SynData[SIndex] << 24) |
						SynData[SIndex+1] >> 8);
			}
			SIndex++;
			DIndex++;
		}
		SIndex++;

		if (Index != 4) {
			for (SubIndex = 0; SubIndex < 4; SubIndex++) {
				if (SubIndex == 3) {
					InstancePtr->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
					XSK_PUF_EFUSE_TRIM_MASK) << 4) |
						SynData[SIndex+1] >> 16);

				}
				else {
					InstancePtr->EfuseSynData[DIndex] =
						((SynData[SIndex] << 4) |
						SynData[SIndex+1] >> 28);
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0; SubIndex < 4; SubIndex++) {
				if(SubIndex == 3) {
					InstancePtr->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
					XSK_PUF_EFUSE_TRIM_MASK) << 16) |
						SynData[SIndex+1] >> 4);

				}
				else {
					InstancePtr->EfuseSynData[DIndex]=
						((SynData[SIndex] << 16) |
						SynData[SIndex+1] >> 16);
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0; SubIndex < 3; SubIndex++) {
				if (SubIndex == 2) {
					InstancePtr->EfuseSynData[DIndex] =
						((SynData[SIndex] << 28) |
						(SynData[SIndex+1] &
						XSK_PUF_EFUSE_TRIM_MASK) >> 4);
					InstancePtr->EfuseSynData[DIndex] |=
						(SynData[SIndex+2] >> 24);
				}
				else {
					InstancePtr->EfuseSynData[DIndex]=
						((SynData[SIndex] << 28) |
						SynData[SIndex+1] >> 4);
				}
				SIndex++;
				DIndex++;
			}
			SIndex++;

			for (SubIndex = 0; SubIndex < 4; SubIndex++) {
				if (SubIndex == 3) {
					InstancePtr->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
						XSK_PUF_EFUSE_TRIM_MASK) << 8) |
						SynData[SIndex+1] >> 12);

				}
				else {
					InstancePtr->EfuseSynData[DIndex] =
						((SynData[SIndex] << 8) |
						SynData[SIndex+1] >> 24);
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0; SubIndex < 3; SubIndex++) {
				if (SubIndex == 2) {
					InstancePtr->EfuseSynData[DIndex] =
						((SynData[SIndex] << 20) |
						(SynData[SIndex+1] &
					XSK_PUF_EFUSE_TRIM_MASK) >> 12);

				}
				else {
					InstancePtr->EfuseSynData[DIndex]=
						((SynData[SIndex] << 20) |
						SynData[SIndex+1] >> 12);
				}
				SIndex++;
				DIndex++;
			}
			SIndex++;
		}
	}
	InstancePtr->EfuseSynData[XSK_PUF_LAST_WORD_OFFSET] &=
						XSK_PUF_LAST_WORD_MASK;

#if defined XPUF_INFO_ON_UART
	xPuf_printf(XPUF_DEBUG_GENERAL,
		"App:Formatted syndrome data start!!!\r\n");
	for (Index = 0; Index < XSK_PUF_FORMATTED_SYN_SIZE_WORDS;
						Index++) {
		xPuf_printf(XPUF_DEBUG_GENERAL,"%08x",
			InstancePtr->EfuseSynData[Index]);
	}
	xPuf_printf(XPUF_DEBUG_GENERAL,
		"\r\nApp:Formatted syndrome data End!!!\r\n");
#endif

}

/*****************************************************************************/
/**
 *
 * This API programs the eFUSE with the black key.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS - On successful eFUSE programming
 *		- Error code - on failure
 *
 * @note	None,
 *
 ******************************************************************************/
static u32 XilSkey_Program_Black_Key()
{
	u32 Status;
	u32 Index;
	u32 AesCrc;

	/* Initializing the instance to program AES key */
	EfuseInstance.PrgrmAesKey = TRUE;

	/* Copies Black key into Efuse instance in EFUSE write format */
	for (Index = 0; Index < XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES;
								Index++) {
		EfuseInstance.AESKey[
		(XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES - 1) - Index] =
						PufInstance.BlackKey[Index];
	}
	/* Programs the Black key */
	Status = XilSKey_ZynqMp_EfusePs_Write(&EfuseInstance);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Black key writing is failed \r\n");
		return Status;
	}
	else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Black key writing is passed \r\n");
	}
	Status = XilSKey_ZynqMp_EfusePs_CacheLoad();
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Calculates black key's CRC*/
	AesCrc = XilSkey_CrcCalculation_AesKey(&(EfuseInstance.AESKey[0]));
	xPuf_printf(XPUF_DEBUG_GENERAL,
			"App; CRC caclculated on black key is = %x", AesCrc);

	/* Verifies the black key programmed */
	Status = XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(AesCrc);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"\r\n Black key CRC checK is failed\n\r");
		return Status;
	}
	else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"\r\nBlack key CRC checK is passed\n\r");
	}

	return XST_SUCCESS;

}

#endif /*XPUF_FUSE_SYN_DATA*/

/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param	InChar is input character. It has to be between 0-9,a-f,A-F
 * @param	Num is the output nibble.
 *
 * @return
 * 		- XST_SUCCESS no errors occurred.
 *		- ERROR when input parameters are not valid
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 XilSKey_Puf_ConvertCharToNibble(char InChar, u8 *Num) {
	/* Convert the char to nibble */
	if ((InChar >= '0') && (InChar <= '9'))
		*Num = InChar - '0';
	else if ((InChar >= 'a') && (InChar <= 'f'))
		*Num = InChar - 'a' + 10;
	else if ((InChar >= 'A') && (InChar <= 'F'))
		*Num = InChar - 'A' + 10;
	else
		return XPUF_STRING_INVALID_ERROR;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper
 *		case values. Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 *		- XST_SUCCESS no errors occurred.
 *		- ERROR when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 XilSKey_Puf_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len)
{
	u32 ConvertedLen = 0;
	u8 LowerNibble, UpperNibble;

	/* Check the parameters */
	if (Str == NULL)
		return XPUF_PARAMETER_NULL_ERROR;

	if (Buf == NULL)
		return XPUF_PARAMETER_NULL_ERROR;

	/* Len has to be multiple of 2 */
	if ((Len == 0) || (Len % 2 == 1))
		return XPUF_PARAMETER_NULL_ERROR;

	ConvertedLen = 0;
	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		if (XilSKey_Puf_ConvertCharToNibble(Str[ConvertedLen],
				&UpperNibble) ==XST_SUCCESS) {
			/* Convert char to nibble */
			if (XilSKey_Puf_ConvertCharToNibble(
					Str[ConvertedLen + 1],
					&LowerNibble) == XST_SUCCESS) {
				/* Merge upper and lower nibble to Hex */
				Buf[ConvertedLen / 2] =
					(UpperNibble << 4) | LowerNibble;
			} else {
				/* Error converting Lower nibble */
				return XPUF_STRING_INVALID_ERROR;
			}
		} else {
			/* Error converting Upper nibble */
			return XPUF_STRING_INVALID_ERROR;
		}
		ConvertedLen += 2;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Converts the PUF Syndrome data to eFUSE writing format.
 *
 * @param	None
 *
 * @return	None
 *
 * @note	Encrypted key will be available @ PufInstance.BlackKey.
 *
 ******************************************************************************/
static u32 XilSKey_Puf_Encrypt_Key()
{
	u32 Status;
	XCsuDma_Config *Config;
	XCsuDma_Configure ConfigurValues = {0};

	/* Convert the AES key string to hex format */
	Status = XilSKey_Puf_ConvertStringToHexBE(
			(const char *) (XSK_PUF_AES_KEY),
				PufInstance.RedKey, 64);
	if (Status != XST_SUCCESS) {
		Status = XPUF_STRING_INVALID_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: String Conversion error (KEY):%08x !!!\r\n", Status);
		goto ENDENCRYPT;
	} else {
#if defined	XPUF_INFO_ON_UART
	u32 Index;
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Red key - ");
	for (Index = 0; Index < 32; Index++)
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"%02x", PufInstance.RedKey[Index]);
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
#endif
	}

	/*
	 * Convert the IV from string to hex format. This is the IV used to
	 * encrypt the AES key using PUF key
	 */
	Status = XilSKey_Puf_ConvertStringToHexBE(
				(const char *)(XSK_PUF_BLACK_KEY_IV),
			PufInstance.BlackKeyIV, XSK_IV_LENGTH);
	if (Status != XST_SUCCESS) {
		Status = XPUF_STRING_INVALID_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: String Conversion error (IV):%08x !!!\r\n", Status);
		goto ENDENCRYPT;
	} else {
#if defined		XPUF_INFO_ON_UART
	u32 ArrayIn;
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Black key IV - ");
	for (ArrayIn = 0; ArrayIn < XSK_ZYNQMP_PUF_KEY_IV_LEN_IN_BYTES;
			ArrayIn++) {
		xPuf_printf(XPUF_DEBUG_GENERAL, "%02x",
				PufInstance.BlackKeyIV[ArrayIn]);
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
#endif
	}

	/* Flush and disable the cache */
	Xil_DCacheFlush();
	Xil_DCacheDisable();

	xPuf_printf(XPUF_DEBUG_GENERAL, "App: DCache disabled \n\r");

	/* Initialize & configure the DMA */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: DMA config  \n\r");
	Config = XCsuDma_LookupConfig(XSK_CSUDMA_DEVICE_ID);
	if (NULL == Config) {
		Status = XPUF_DMA_CONFIG_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: DMA config failed:%08x \r\n", Status);
		goto ENDENCRYPT;
	}

	xPuf_printf(XPUF_DEBUG_GENERAL, "App: DMA config initialize \n\r");
	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPUF_DMA_INIT_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: DMA Config Initialize failed \n\r");
		goto ENDENCRYPT;
	}

	/* Initialize AES engine */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AES initialize \n\r");
	Status = XSecure_AesInitialize(&AesInstance, &CsuDma,
				XSK_PUF_DEVICE_KEY,
				(u32*)&PufInstance.BlackKeyIV[0], NULL);

	if (Status != XST_SUCCESS) {
		Status = XPUF_AES_INIT_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: AES Initialize failed \n\r");
		goto ENDENCRYPT;
	}
	/* Set the data endianness for IV */
	XCsuDma_GetConfig(&CsuDma, XCSUDMA_SRC_CHANNEL,
				&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(&CsuDma, XCSUDMA_SRC_CHANNEL,
					&ConfigurValues);

	/* Enable CSU DMA Dst channel for byte swapping.*/
	XCsuDma_GetConfig(&CsuDma, XCSUDMA_DST_CHANNEL,
			&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(&CsuDma, XCSUDMA_DST_CHANNEL,
			&ConfigurValues);

	/* Request to encrypt the AES key using PUF Key	 */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AES encryption \r\n");
	XSecure_AesEncryptData(&AesInstance, &PufInstance.BlackKey[0],
			&PufInstance.RedKey[0], 32);
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Encrypted key generated\r\n");
#if defined		XPUF_INFO_ON_UART
	u32 KeyIn;
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Black key - ");
	for (KeyIn = 0; KeyIn < 32; KeyIn++) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"%02x", PufInstance.BlackKey[KeyIn]);
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
#endif

ENDENCRYPT:
	return Status;

}

