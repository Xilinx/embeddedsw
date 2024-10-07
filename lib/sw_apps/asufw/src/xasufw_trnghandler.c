/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
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
#define XASUFW_RESP_TRNG_RANDOM_BYTES_OFFSET	2U /**< TRNG Random numbers start word offset in
													response buffer */

#define XASUFW_MAX_RANDOM_BYTES_ALLOWED			96U /**< Maximum random bytes can be requested */

/************************************** Type Definitions *****************************************/
/* Calculate the structure member address from Item and structure Type */
#define XAsufw_GetRespBuf(Item, Type, Member)    \
	((Type *)(((char *)(Item) - offsetof(Type, Item)) + offsetof(Type, Member)))

#ifdef XASUFW_TRNG_ENABLE_DRBG_MODE
/** @brief This structure contains configuration information for DRBG instantiation. */
typedef struct {
	u32 Header; /**< DRBG Instantiate command header */
	u8 *SeedPtr; /**< Initial seed pointer */
	u32 SeedLen; /**< Seed length */
	u8 *PersStrPtr; /**< Personalization string pointer */
	u32 SeedLife; /**< Seed life */
	u32 DFLen; /**< DF length */
} XAsufw_DrbgInstantiateCmd;

/** @brief This structure contains configuration information for DRBG reseed. */
typedef struct {
	u32 Header; /**< DRBG Reseed command header */
	u8 *ReseedPtr; /**< Reseed pointer */
	u32 DFLen; /**< DF length */
} XAsufw_DrbgReseedCmd;

/** @brief This structure contains configuration information for DRBG regenerate. */
typedef struct {
	u32 Header; /**< DRBG Generate command header */
	u8 *RandBuf; /**< Pointer to buffer for storing random data */
	u32 RandBufSize; /**< Size of the random data buffer */
	u32 PredResistance; /**< Prediction resistance flag */
} XAsufw_DrbgGenerateCmd;
#endif /* XASUFW_TRNG_ENABLE_DRBG_MODE */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_TrngGetRandomBytes(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_TrngKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_TrngGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_TrngDrbgInstantiate(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_TrngDrbgReseed(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_TrngDrbgGenerate(const XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_TrngModule; /**< ASUFW TRNG Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the TRNG module.
 *
 * @return
 * 	- On successful initialization of TRNG module, it returns XASUFW_SUCCESS.
 * 	- XASUFW_TRNG_MODULE_REGISTRATION_FAILED, if TRNG module registration fails.
 * 	- XASUFW_FAILURE, if thee is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_TrngInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);

	/** Contains the array of ASUFW TRNG commands. */
	static const XAsufw_ModuleCmd XAsufw_TrngCmds[] = {
		[XASU_TRNG_GET_RANDOM_BYTES_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngGetRandomBytes),
		[XASU_TRNG_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngKat),
		[XASU_TRNG_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngGetInfo),
		[XASU_TRNG_DRBG_INSTANTIATE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngDrbgInstantiate),
		[XASU_TRNG_DRBG_RESEED_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngDrbgReseed),
		[XASU_TRNG_DRBG_GENERATE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_TrngDrbgGenerate),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_TrngResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_TrngCmds)] = {
		[XASU_TRNG_GET_RANDOM_BYTES_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_TRNG_KAT_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
		[XASU_TRNG_GET_INFO_CMD_ID] = 0U,
		[XASU_TRNG_DRBG_INSTANTIATE_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
		[XASU_TRNG_DRBG_RESEED_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
		[XASU_TRNG_DRBG_GENERATE_CMD_ID] = XASUFW_TRNG_RESOURCE_MASK,
	};

	XAsufw_TrngModule.Id = XASU_MODULE_TRNG_ID;
	XAsufw_TrngModule.Cmds = XAsufw_TrngCmds;
	XAsufw_TrngModule.ResourcesRequired = XAsufw_TrngResourcesBuf;
	XAsufw_TrngModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_TrngCmds);

	/** Register TRNG module. */
	Status = XAsufw_ModuleRegister(&XAsufw_TrngModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_TRNG_MODULE_REGISTRATION_FAILED);
	}

	/** Initialize the TRNG instance. */
	Status = XTrng_CfgInitialize(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

#if !defined(XASUFW_TRNG_ENABLE_PTRNG_MODE) || defined(XASUFW_TRNG_ENABLE_DRBG_MODE)
	/** Perform health test on TRNG. */
	Status = XTrng_PreOperationalSelfTests(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Instantiate to complete initialization of TRNG in HRNG mode. */
	Status = XTrng_InitNCfgTrngMode(XAsufw_Trng, XTRNG_HRNG_MODE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Enable auto proc mode for TRNG. */
	Status = XTrng_EnableAutoProcMode(XAsufw_Trng);
#else
	/** Instantiate to complete initialization of TRNG in PTRNG mode. */
	Status = XTrng_InitNCfgTrngMode(XAsufw_Trng, XTRNG_PTRNG_MODE);
#endif /* XASUFW_TRNG_ENABLE_PTRNG_MODE */

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if random numbers are available in TRNG FIFO or not.
 *
 * @return
 * 	- XASUFW_SUCCESS If random numbers are available in TRNG FIFO.
 * 	- XASUFW_FAILURE If random numbers are not available in TRNG FIFO.
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
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS - if read TRNG FIFO operation is successful.
 * 	- XASUFW_FAILURE, if there is any other failure.
 *
 * @note	This IPI command must not be called when DRBG mode is enabled using
 * 	XASUFW_TRNG_ENABLE_DRBG_MODE macro.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngGetRandomBytes(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	u32 *RandomBuf = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
			 XASUFW_RESP_TRNG_RANDOM_BYTES_OFFSET;

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
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_ShaKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	s32 SStatus = XASUFW_FAILURE;
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);

	if (XASUFW_PLATFORM == PMC_TAP_VERSION_PLATFORM_QEMU) {
		XAsufw_Printf(DEBUG_GENERAL, "INFO: DRBG KAT is not supported on QEMU\r\n");
		Status = XASUFW_TRNG_KAT_NOT_SUPPORTED_ON_QEMU;
		goto END;
	}

	/** Uninstantiate TRNG before running KAT */
	Status = XTrng_Uninstantiate(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

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
 * @brief	This function is a handler for TRNG Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful execution of the command.
 * 	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	/** TODO: Need to add support TRNG Get Info command */
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG DRBG instantiate command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS on successful execution of the command.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngDrbgInstantiate(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
#if defined(XASUFW_TRNG_ENABLE_DRBG_MODE)
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	XTrng_UserConfig UsrCfg;
	XAsufw_DrbgInstantiateCmd *Cmd = (XAsufw_DrbgInstantiateCmd *)ReqBuf;

	/* Disable auto proc mode before running KAT */
	Status = XTrng_Uninstantiate(XAsufw_Trng);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/* Initiate TRNG */
	UsrCfg.Mode = XTRNG_DRBG_MODE;
	UsrCfg.DFLength = (u8)Cmd->DFLen;
	UsrCfg.SeedLife = Cmd->SeedLife;
	UsrCfg.IsBlocking = XASU_TRUE;

	Status = XTrng_Instantiate(XAsufw_Trng, Cmd->SeedPtr, Cmd->SeedLen, Cmd->PersStrPtr, &UsrCfg);

END:
	if (Status != XASUFW_SUCCESS) {
		(void)XTrng_Uninstantiate(XAsufw_Trng);
	}
#endif /*XASUFW_TRNG_ENABLE_DRBG_MODE */
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG DRBG reseed command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS on successful execution of the command.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngDrbgReseed(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
#if defined(XASUFW_TRNG_ENABLE_DRBG_MODE)
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	XAsufw_DrbgReseedCmd *Cmd = (XAsufw_DrbgReseedCmd *)ReqBuf;

	Status = XTrng_Reseed(XAsufw_Trng, Cmd->ReseedPtr, (u8)Cmd->DFLen);
#endif /* XASUFW_TRNG_ENABLE_DRBG_MODE */
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for TRNG DRBG generate command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS on successful execution of the command.
 * 	- XASUFW_FAILURE, if tehre is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_TrngDrbgGenerate(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
#if defined(XASUFW_TRNG_ENABLE_DRBG_MODE)
	XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	XAsufw_DrbgGenerateCmd *Cmd = (XAsufw_DrbgGenerateCmd *)ReqBuf;

	Status = XTrng_Generate(XAsufw_Trng, Cmd->RandBuf, Cmd->RandBufSize, (u8)Cmd->PredResistance);
#endif /* XASUFW_TRNG_ENABLE_DRBG_MODE */
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the requested number of random bytes from TRNG AUTOPROC FIFO.
 *
 * @param	RandomBuf	Pointer to the random buffer.
 * @param	Size		Size of the random buffer. The maximum allowed size is 96 Bytes
 * 				(3 256-bit random numbers).
 *
 * @return
 * 	- XASUFW_SUCCESS on successful execution of the command.
 * 	- XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE, if size of random buffer is invalid.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_TrngGetRandomNumbers(u8 *RandomBuf, u32 Size)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
#if !defined(XASUFW_TRNG_ENABLE_PTRNG_MODE) && !defined(XASUFW_TRNG_ENABLE_DRBG_MODE)
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	const XTrng *XAsufw_Trng = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	u32 Bytes = Size;
	u8 *BufAddr = RandomBuf;
	u8 LocalBuf[XTRNG_SEC_STRENGTH_IN_BYTES];

	if (Size > XASUFW_MAX_RANDOM_BYTES_ALLOWED) {
		Status = XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE;
		goto END;
	}

	while (Bytes != 0U) {
		while (XTrng_IsRandomNumAvailable(XAsufw_Trng) != XASUFW_SUCCESS);
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
			Bytes = 0U;
		}
	}

END:
#endif /* XASUFW_TRNG_ENABLE_PTRNG_MODE */
	return Status;
}
/** @} */
