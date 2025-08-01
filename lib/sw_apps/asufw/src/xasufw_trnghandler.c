/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_trnghandler.c
 *
 * This file contains the TRNG module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/20/24 Initial release
 *       ma   07/01/24 Add IPI interface for DRBG mode for CAVP testing
 *       ma   07/08/24 Add task based approach at queue level
 *       ma   07/23/24 Added API to read any number of random bytes from TRNG
 *       ma   07/26/24 Added support for PTRNG GetRandomBytes
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       ma   12/24/24 Disable autoproc mode before running DRBG KAT or DRBG Instantiate commands
 *       ma   02/07/25 Added DRBG support in client
 *       ma   02/11/25 Added redundancy, validations and clearing of local buffer in
 *                     XAsufw_TrngGetRandomNumbers
 *       rmv  08/01/25 Added function call to set TRNG to default mode
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_trnghandler.h"
#include "xasufw_modules.h"
#include "xasufw_status.h"
#include "xtrng.h"
#include "xtrng_hw.h"
#include "xasu_trnginfo.h"
#include "xasufw_hw.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_MAX_RANDOM_BYTES_ALLOWED		510U /**< Maximum random bytes can be requested */
#define XASUFW_GET_RANDOM_BYTES_TIMEOUT_VAL	100000U /**< Maximum timeout in us waiting for TRNG
								random number to be available in FIFO */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_TrngGetRandomBytes(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_TrngKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_TrngDrbgInstantiate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_TrngDrbgReseed(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_TrngDrbgGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_TrngModule; /**< ASUFW TRNG Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the TRNG module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if TRNG module initialization is successful.
 * 	- XASUFW_TRNG_MODULE_REGISTRATION_FAILED, if TRNG module registration fails.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_TrngInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);

	/** The XAsufw_TrngCmds array contains the list of commands supported by TRNG module. */
	static const XAsufw_ModuleCmd XAsufw_TrngCmds[] = {
		[XASU_TRNG_GET_RANDOM_BYTES_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngGetRandomBytes),
		[XASU_TRNG_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngKat),
		[XASU_TRNG_DRBG_INSTANTIATE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngDrbgInstantiate),
		[XASU_TRNG_DRBG_RESEED_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngDrbgReseed),
		[XASU_TRNG_DRBG_GENERATE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngDrbgGenerate),
	};

	/** The XAsufw_TrngResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_TrngResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_TrngCmds)] = {
		[XASU_TRNG_GET_RANDOM_BYTES_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_TRNG_KAT_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
		[XASU_TRNG_DRBG_INSTANTIATE_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
		[XASU_TRNG_DRBG_RESEED_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
		[XASU_TRNG_DRBG_GENERATE_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
	};

	XAsufw_TrngModule.Id = XASU_MODULE_TRNG_ID;
	XAsufw_TrngModule.Cmds = XAsufw_TrngCmds;
	XAsufw_TrngModule.ResourcesRequired = XAsufw_TrngResourcesBuf;
	XAsufw_TrngModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_TrngCmds);
	XAsufw_TrngModule.ResourceHandler = NULL;
	XAsufw_TrngModule.AsuDmaPtr = NULL;

	/** Register TRNG module. */
	Status = XAsufw_ModuleRegister(&XAsufw_TrngModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_TRNG_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize the TRNG crypto engine. */
	Status = XTrng_CfgInitialize(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Perform health test on TRNG. */
	Status = XTrng_PreOperationalSelfTests(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Set TRNG to default mode. */
	Status = XTrng_EnableDefaultMode(XAsufw_Trng);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if random numbers are available in TRNG FIFO or not.
 *
 * @return
 * 	- XASUFW_SUCCESS, if random numbers are available in TRNG FIFO.
 * 	- XASUFW_FAILURE, if random numbers are not available in TRNG FIFO.
 *
 *************************************************************************************************/
s32 XAsufw_TrngIsRandomNumAvailable(void)
{
	s32 Status = XASUFW_FAILURE;
	const XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);

#if !defined(XASUFW_TRNG_ENABLE_PTRNG_MODE)
	Status = XTrng_IsRandomNumAvailable(XAsufw_Trng);
#else
	Status = XASUFW_SUCCESS;
#endif /* XASUFW_TRNG_ENABLE_PTRNG_MODE */

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG Get Random Bytes command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if read TRNG FIFO operation is successful.
 * 	- XASUFW_FAILURE, if there is any other failure.
 *
 * @note	This IPI command must not be called when DRBG mode is enabled using
 * 	XASU_TRNG_ENABLE_DRBG_MODE macro.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngGetRandomBytes(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	u32 *RandomBuf = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;

	(void)ReqId;

#if !defined(XASUFW_TRNG_ENABLE_PTRNG_MODE)
	Status = XTrng_ReadTrngFifo(XAsufw_Trng, RandomBuf, XTRNG_SEC_STRENGTH_IN_BYTES);
#else
	Status = XTrng_Generate(XAsufw_Trng, (u8 *)RandomBuf, XTRNG_SEC_STRENGTH_IN_BYTES, XASU_FALSE);
#endif /* XASUFW_TRNG_ENABLE_PTRNG_MODE */

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code from XAsufw_TrngKat API, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	s32 SStatus = XASUFW_FAILURE;
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);

	(void)ReqBuf;
	(void)ReqId;

	if ((XASUFW_PLATFORM == PMC_TAP_VERSION_PLATFORM_QEMU) ||
		(XASUFW_PLATFORM == PMC_TAP_VERSION_PLATFORM_COSIM)) {
		XAsufw_Printf(DEBUG_INFO, "INFO: DRBG KAT is not supported on QEMU\r\n");
		Status = XASUFW_TRNG_KAT_NOT_SUPPORTED_ON_QEMU;
		goto END;
	}

	/** Disable auto proc mode and uninstantiate TRNG before running KAT */
	Status = XTrng_DisableAutoProcMode(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Run TRNG DRBG KAT */
	Status = XTrng_DrbgKat(XAsufw_Trng);

	/** Instantiate to complete initialization of TRNG in HRNG mode */
	SStatus = XTrng_InitNCfgTrngMode(XAsufw_Trng, XTRNG_HRNG_MODE);
	if (SStatus != XST_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, SStatus);
		goto END;
	}

	/** Enable auto proc mode for TRNG */
	SStatus = XTrng_EnableAutoProcMode(XAsufw_Trng);
	if (SStatus != XST_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, SStatus);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG DRBG instantiate command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if command execution is successful.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngDrbgInstantiate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
#if defined(XASU_TRNG_ENABLE_DRBG_MODE)
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	XTrng_UserConfig UsrCfg;
	XAsu_DrbgInstantiateCmd *Cmd = (XAsu_DrbgInstantiateCmd *)ReqBuf->Arg;

	/* Instantiate TRNG in DRBG mode. */
	UsrCfg.Mode = XTRNG_DRBG_MODE;
	UsrCfg.DFLength = (u8)Cmd->DFLen;
	UsrCfg.SeedLife = Cmd->SeedLife;

	Status = XTrng_Uninstantiate(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XTrng_Instantiate(XAsufw_Trng, (u8 *)(UINTPTR)Cmd->SeedPtr, Cmd->SeedLen,
				(u8 *)(UINTPTR)Cmd->PersStrPtr, &UsrCfg);
	if (Status != XASUFW_SUCCESS) {
		(void)XTrng_Uninstantiate(XAsufw_Trng);
	}

END:
#endif /*XASU_TRNG_ENABLE_DRBG_MODE */

	(void)ReqBuf;
	(void)ReqId;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG DRBG reseed command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if command execution is successful.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngDrbgReseed(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
#if defined(XASU_TRNG_ENABLE_DRBG_MODE)
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	XAsu_DrbgReseedCmd *Cmd = (XAsu_DrbgReseedCmd *)ReqBuf->Arg;

	Status = XTrng_Reseed(XAsufw_Trng, (u8 *)(UINTPTR)Cmd->ReseedPtr, (u8)Cmd->DFLen);
	if (Status != XASUFW_SUCCESS) {
		(void)XTrng_Uninstantiate(XAsufw_Trng);
	}
#endif /* XASU_TRNG_ENABLE_DRBG_MODE */

	(void)ReqBuf;
	(void)ReqId;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG DRBG generate command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if command execution is successful.
 * 	- XASUFW_FAILURE, if tehre is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngDrbgGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
#if defined(XASU_TRNG_ENABLE_DRBG_MODE)
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	XAsu_DrbgGenerateCmd *Cmd = (XAsu_DrbgGenerateCmd *)ReqBuf->Arg;

	Status = XTrng_Generate(XAsufw_Trng, (u8 *)(UINTPTR)Cmd->RandBuf, Cmd->RandBufSize,
			(u8)Cmd->PredResistance);
	if (Status != XASUFW_SUCCESS) {
		(void)XTrng_Uninstantiate(XAsufw_Trng);
	}
#endif /* XASU_TRNG_ENABLE_DRBG_MODE */

	(void)ReqBuf;
	(void)ReqId;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the requested number of random bytes from TRNG AUTOPROC FIFO.
 *
 * @param	RandomBuf	Pointer to the random buffer.
 * @param	Size		Size of the random buffer. The maximum allowed size is 510 Bytes
 *
 * @return
 * 	- XASUFW_SUCCESS, if requested bytes of random number is generated successfully.
 * 	- XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE, if size of random buffer is invalid.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_TrngGetRandomNumbers(u8 *RandomBuf, u32 Size)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
#if !defined(XASUFW_TRNG_ENABLE_PTRNG_MODE) && !defined(XASU_TRNG_ENABLE_DRBG_MODE)
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	const XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	u32 Bytes = Size;
	u8 *BufAddr = RandomBuf;
	u8 LocalBuf[XTRNG_SEC_STRENGTH_IN_BYTES];
	u32 Loop;

	/** Validate the size. */
	if (Size > XASUFW_MAX_RANDOM_BYTES_ALLOWED) {
		Status = XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE;
		goto END;
	}

	while (Bytes != 0U) {
		/** Check if the random number is available in the TRNG FIFO for predefined time. */
		for (Loop = 0x0U; Loop < XASUFW_GET_RANDOM_BYTES_TIMEOUT_VAL; ++Loop) {
			if (XTrng_IsRandomNumAvailable(XAsufw_Trng) == XASUFW_SUCCESS) {
				break;
			}
			usleep(1U);
		}

		/**
		 * Check again if random number is available for redundancy.
		 * Return error if random number is not available.
		 */
		if (XTrng_IsRandomNumAvailable(XAsufw_Trng) != XASUFW_SUCCESS) {
			Status = XASUFW_TRNG_GET_RANDOM_NUMBERS_TIMEDOUT;
			goto END;
		}

		/** Read the random number from the TRNG FIFO to the given buffer. */
		if (Bytes >= XTRNG_SEC_STRENGTH_IN_BYTES) {
			XFIH_CALL_GOTO(XTrng_ReadTrngFifo, XFihVar, Status, END, XAsufw_Trng, (u32 *)BufAddr,
							XTRNG_SEC_STRENGTH_IN_BYTES);
			BufAddr += XTRNG_SEC_STRENGTH_IN_BYTES;
			Bytes -= XTRNG_SEC_STRENGTH_IN_BYTES;
		} else {
			XFIH_CALL_GOTO(XTrng_ReadTrngFifo, XFihVar, Status, END, XAsufw_Trng, (u32 *)LocalBuf,
							XTRNG_SEC_STRENGTH_IN_BYTES);
			XFIH_CALL_GOTO(Xil_SMemCpy, XFihVar, Status, END, BufAddr, Bytes, LocalBuf,
							XTRNG_SEC_STRENGTH_IN_BYTES, Bytes);
			BufAddr += Bytes;
			Bytes = 0U;
		}
	}

	/** Validate if desired number of bytes are copied. */
	if ((RandomBuf + Size) != BufAddr) {
		Status = XASUFW_TRNG_INVALID_RANDOM_NUMBER;
	}

END:
	/** Zeroize local buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, LocalBuf, XTRNG_SEC_STRENGTH_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
#endif /* XASUFW_TRNG_ENABLE_PTRNG_MODE */
	return Status;
}
/** @} */
