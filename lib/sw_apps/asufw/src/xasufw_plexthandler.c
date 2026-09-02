/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_plexthandler.c
 *
 * This file contains the PL extension module commands supported by ASUFW. The module streams data
 * between ASU memory and a crypto block instantiated in the PL over the secure stream switch PL
 * interface.
 *
 * The command performs the whole operation in a single request. The key is taken from user memory
 * and the request uses XAsu_AesParams and XAsu_AesKeyObject, so the same parameters that drive the
 * ASU AES engine drive the PL crypto block.
 *
 * The key consumed by the PL is written to the ASU AES key vault USER_KEY_7 register and is
 * selected by the PL key select register in the same key vault.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_plexthandler.h"

#ifdef XASU_PLEXT_ENABLE

#include "xasufw_hw.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_sss.h"
#include "xasufw_dma.h"
#include "xasufw_debug.h"
#include "xasu_aesinfo.h"
#include "xasu_plextinfo.h"
#include "xasu_sharedmem.h"
#include "xasu_def.h"
#include "xaes_hw.h"
#include "xil_sutil.h"

/************************************ Constant Definitions ***************************************/
/**
 * PL AES mode configuration register offset within the PL configuration window, whose base address
 * and size are published by the PLM in the RTCA.
 */
#define XASUFW_PL_AES_MODE_CFG_OFFSET	(0x00000004U)

/**
 * PL AES mode configuration register value for AES-256 ECB encrypt operation. The register is a
 * single 32-bit word whose field layout is defined by the PL design, hence the value is programmed
 * as published. A different engine mode, key size or operation type needs the corresponding value
 * from the PL design.
 */
#define XASUFW_PL_AES_CFG_ECB_ENCRYPT_256	(0x000A0004U)

/** PL extension maximum wait timeout value in micro seconds (5000us/5ms). */
#define XASUFW_PLEXT_TIMEOUT_MAX	(5000U)

/**
 * Offset of the last word of the USER_KEY_7 register block, from where
 * XAsufw_WriteDataToRegsWithEndianSwap() writes towards decreasing offsets.
 */
#define XASUFW_PL_KEY_LAST_WORD_OFFSET	(XAES_USER_KEY_7_0_OFFSET + \
					((XASU_AES_KEY_SIZE_256BIT_IN_WORDS - 1U) * \
					XASUFW_WORD_LEN_IN_BYTES))

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_PlExtOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_PlExtResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_PlExtValidateParams(const XAsu_AesParams *ParamsPtr);
static s32 XAsufw_PlExtCfgIsolation(u32 Mask, u32 Value);
static s32 XAsufw_PlExtConfigure(XAsufw_Dma *DmaPtr, const XAsu_AesParams *ParamsPtr);
static s32 XAsufw_PlExtKeyZeroize(void);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_PlExtModule; /**< ASUFW PL extension module ID and commands array */
static u32 XAsufw_PlCfgBase; /**< PL configuration window base address */
static u32 XAsufw_PlCfgSize; /**< PL configuration window size in bytes */

/*************************************************************************************************/
/**
 * @brief	This function initializes the PL extension module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization of PL extension module is successful.
 * 	- XASUFW_PLEXT_MODULE_REGISTRATION_FAILED, if module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_PlExtInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_PlExtCmds array contains the list of commands supported by the module. */
	static const XAsufw_ModuleCmd XAsufw_PlExtCmds[XASU_PLEXT_MAX_CMDS] = {
		[XASU_PLEXT_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_PlExtOperation),
	};

	/**
	 * USER_KEY_7 is reserved for the PL path, so AES resource is not required.
	 */
	static XAsufw_ResourcesRequired XAsufw_PlExtResourcesBuf[XASU_PLEXT_MAX_CMDS] = {
		[XASU_PLEXT_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
						XASUFW_PLI_RESOURCE_MASK,
	};

	/** The XAsufw_PlExtAccessPermBuf contains the IPI access permissions for each command. */
	static XAsufw_AccessPerm_t XAsufw_PlExtAccessPermBuf[XASU_PLEXT_MAX_CMDS] = {
		[XASU_PLEXT_OPERATION_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_PLEXT_OPERATION_CMD_ID),
	};

	XAsufw_PlExtModule.Id = XASU_MODULE_PLEXT_ID;
	XAsufw_PlExtModule.Cmds = XAsufw_PlExtCmds;
	XAsufw_PlExtModule.ResourcesRequired = XAsufw_PlExtResourcesBuf;
	XAsufw_PlExtModule.CmdCnt = XASU_PLEXT_MAX_CMDS;
	XAsufw_PlExtModule.ResourceHandler = XAsufw_PlExtResourceHandler;
	XAsufw_PlExtModule.AsuDmaPtr = NULL;
	XAsufw_PlExtModule.AccessPermBufferPtr = XAsufw_PlExtAccessPermBuf;

	/** Register PL extension module. */
	Status = XAsufw_ModuleRegister(&XAsufw_PlExtModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PLEXT_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Read the PL configuration window published by the PLM in the RTCA. */
	XAsufw_PlCfgBase = XAsufw_ReadReg(XASU_RTCA_PL_CFG_BASE_ADDR);
	XAsufw_PlCfgSize = XAsufw_ReadReg(XASU_RTCA_PL_CFG_SIZE_ADDR);

	/**
	 * Validate the PL configuration window. An unusable window means the design does not
	 * provide a PL crypto block, which must not stop ASUFW from booting. The module stays
	 * registered and rejects the command with XASUFW_PLEXT_WINDOW_NOT_CONFIGURED.
	 */
	if ((XAsufw_PlCfgBase == 0U) ||
			((XAsufw_PlCfgBase % XASUFW_WORD_LEN_IN_BYTES) != 0U) ||
			(XAsufw_PlCfgSize < (XASUFW_PL_AES_MODE_CFG_OFFSET +
			XASUFW_WORD_LEN_IN_BYTES))) {
		XAsufw_Printf(DEBUG_GENERAL, "PL window is not usable, Base = %x Size = %x\r\n",
			XAsufw_PlCfgBase, XAsufw_PlCfgSize);
		XAsufw_PlCfgBase = 0U;
		XAsufw_PlCfgSize = 0U;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for PL extension module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 *	- XASUFW_PLEXT_WINDOW_NOT_CONFIGURED, if the PL configuration window is not usable.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_PlExtResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	if (XAsufw_PlCfgBase == 0U) {
		Status = XASUFW_PLEXT_WINDOW_NOT_CONFIGURED;
		goto END;
	}

	/** Allocate PLI and DMA resources for PL extension operation command. */
	XAsufw_PlExtModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_PLI, ReqId);
	if (XAsufw_PlExtModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	XAsufw_AllocateResource(XASUFW_PLI, XASUFW_PLI, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the PL extension operation parameters.
 *
 * @param	ParamsPtr	Pointer to XAsu_AesParams structure which holds the parameters of PL
 * 				extension input arguments.
 *
 * @return
 * 	- XASUFW_SUCCESS, if validation of input parameters is successful.
 * 	- XASUFW_PLEXT_INVALID_PARAM, if any input parameter is invalid.
 *
 *************************************************************************************************/
static s32 XAsufw_PlExtValidateParams(const XAsu_AesParams *ParamsPtr)
{
	s32 Status = XASUFW_PLEXT_INVALID_PARAM;

	/** Validate the engine mode and the operation type supported by the PL crypto block. */
	if ((ParamsPtr->EngineMode != XASU_AES_ECB_MODE) ||
			(ParamsPtr->OperationType != XASU_AES_ENCRYPT_OPERATION)) {
		goto END;
	}

	/**
	 * Validate the key object address. IV, AAD and tag are not applicable to AES ECB mode on
	 * the PL path.
	 */
	if ((ParamsPtr->KeyObjectAddr == 0U) || (ParamsPtr->IvObj.IvAddr != 0U) ||
			(ParamsPtr->IvObj.IvLen != 0U) || (ParamsPtr->IvObj.IvId != 0U) ||
			(ParamsPtr->AadAddr != 0U) || (ParamsPtr->AadLen != 0U) ||
			(ParamsPtr->TagAddr != 0U) || (ParamsPtr->TagLen != 0U)) {
		goto END;
	}

	/** Validate the input and output data addresses. */
	if ((ParamsPtr->InputDataAddr == 0U) || (ParamsPtr->OutputDataAddr == 0U) ||
			((ParamsPtr->InputDataAddr % XASUFW_WORD_LEN_IN_BYTES) != 0U) ||
			((ParamsPtr->OutputDataAddr % XASUFW_WORD_LEN_IN_BYTES) != 0U)) {
		goto END;
	}

	/**
	 * Validate the input data length. ECB mode input data should be 16Bytes aligned and the
	 * data is transferred in a single DMA transfer.
	 */
	if ((ParamsPtr->DataLen == 0U) ||
			(ParamsPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
			((ParamsPtr->DataLen % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U)) {
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures the ASU to PL isolation control register and validates it
 * 		against the register read-back.
 *
 * @param	Mask	Isolation bits to be updated.
 * @param	Value	Value to be written to the masked bits.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the isolation configuration is successful.
 * 	- XASUFW_PLEXT_CFG_READBACK_FAILED, if the read-back does not match.
 *
 *************************************************************************************************/
static s32 XAsufw_PlExtCfgIsolation(u32 Mask, u32 Value)
{
	s32 Status = XASUFW_FAILURE;
	u32 RegVal;

	XAsufw_RMW(ASU_PL_CSR_ISOLATION_CTRL, Mask, Value);

	RegVal = XAsufw_ReadReg(ASU_PL_CSR_ISOLATION_CTRL);
	if ((RegVal & Mask) != Value) {
		XAsufw_Printf(DEBUG_GENERAL, "PL isolation readback failed, RegVal = %x\r\n",
			RegVal);
		Status = XASUFW_PLEXT_CFG_READBACK_FAILED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function zeroizes the AES key vault USER_KEY_7 register.
 *
 * @return
 * 	- XASUFW_SUCCESS, if key zeroization is successful.
 * 	- XASUFW_AES_KEY_CLEAR_ERROR, if AES key clear fails.
 *
 *************************************************************************************************/
static s32 XAsufw_PlExtKeyZeroize(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Zeroize the USER_KEY_7 key source. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_KEY_CLEAR_OFFSET),
		XAES_KEY_CLEAR_USER_KEY_7_MASK);

	/** Check for key zeroed status within 5ms timeout. */
	Status = (s32)Xil_WaitForEvent((XASU_XKEY_0_BASEADDR + XAES_KEY_ZEROED_STATUS_OFFSET),
		XAES_KEY_ZEROED_STATUS_USER_KEY_7_MASK, XAES_KEY_ZEROED_STATUS_USER_KEY_7_MASK,
		XASUFW_PLEXT_TIMEOUT_MAX);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_KEY_CLEAR_ERROR;
	}

	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_KEY_CLEAR_OFFSET),
		XAES_KEY_CLEAR_REG_CLR_MASK);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the key to the AES key vault, routes it to the PL and
 * 		configures the PL crypto block.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance.
 * @param	ParamsPtr	Pointer to XAsu_AesParams structure which holds the parameters of PL
 * 				extension input arguments.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the PL crypto block configuration is successful.
 * 	- XASUFW_PLEXT_INVALID_PARAM, if the key object parameters are invalid.
 * 	- XASUFW_PLEXT_CFG_READBACK_FAILED, if the isolation, key or mode configuration read-back
 * 	  does not match.
 * 	- XASUFW_DMA_COPY_FAIL, if ASU DMA transfer fails.
 * 	- XASUFW_AES_WRITE_KEY_FAILED, if key write to AES USER key register fails.
 *	- XASUFW_PLEXT_WINDOW_NOT_CONFIGURED, if the PL configuration window is not usable.
 *
 *************************************************************************************************/
static s32 XAsufw_PlExtConfigure(XAsufw_Dma *DmaPtr, const XAsu_AesParams *ParamsPtr)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_AesKeyObject KeyObject;
	u32 Key[XASU_AES_KEY_SIZE_256BIT_IN_WORDS];
	u32 RegVal;

	/** Copy the key object from 64-bit address space to local structure using ASU DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyObjectAddr, (u64)(UINTPTR)&KeyObject,
		(u32)sizeof(XAsu_AesKeyObject), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Validate the key object parameters. */
	if ((KeyObject.KeyAddress == 0U) || (KeyObject.KeyId != 0U) ||
			(KeyObject.KeySize != XASU_AES_KEY_SIZE_256_BITS) ||
			(KeyObject.KeySrc != XASU_AES_USER_KEY_7)) {
		Status = XASUFW_PLEXT_INVALID_PARAM;
		goto END;
	}

	/** Copy the key from 64-bit address space to local array using ASU DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, KeyObject.KeyAddress, (u64)(UINTPTR)Key,
		XASU_AES_KEY_SIZE_256BIT_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Enable the ASU to PL APB and key path. */
	Status = XAsufw_PlExtCfgIsolation(ASU_PL_CSR_ISOLATION_KEY_EN_MASK,
		ASU_PL_CSR_ISOLATION_KEY_EN_MASK);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Write the key to the USER_KEY_7 registers by changing the endianness. */
	Status = XAsufw_WriteDataToRegsWithEndianSwap(XASU_XKEY_0_BASEADDR,
		XASUFW_PL_KEY_LAST_WORD_OFFSET, Key, XASU_AES_KEY_SIZE_256BIT_IN_WORDS);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_WRITE_KEY_FAILED;
		goto END;
	}

	/** Select the key size and route the USER_KEY_7 key source to the PL. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_KEY_SIZE_OFFSET), KeyObject.KeySize);
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_PL_KEY_SEL_OFFSET),
		XAES_KEY_SEL_USER_KEY_7_VALUE);

	/** Validate key size and PL key select against key vault register read-back. */
	RegVal = XAsufw_ReadReg(XASU_XKEY_0_BASEADDR + XAES_KEY_SIZE_OFFSET);
	if ((RegVal != KeyObject.KeySize) ||
			(XAsufw_ReadReg(XASU_XKEY_0_BASEADDR + XAES_PL_KEY_SEL_OFFSET) !=
			XAES_KEY_SEL_USER_KEY_7_VALUE)) {
		XAsufw_Printf(DEBUG_GENERAL, "PL key cfg readback failed, RegVal = %x\r\n", RegVal);
		Status = XASUFW_PLEXT_CFG_READBACK_FAILED;
		goto END;
	}

	/** Enable the ASU to PL data path. */
	Status = XAsufw_PlExtCfgIsolation(ASU_PL_CSR_ISOLATION_DATA_EN_MASK,
		ASU_PL_CSR_ISOLATION_DATA_EN_MASK);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((XASUFW_PL_AES_MODE_CFG_OFFSET + XASUFW_WORD_LEN_IN_BYTES) > XAsufw_PlCfgSize) {
		Status = XASUFW_PLEXT_WINDOW_NOT_CONFIGURED;
		goto END;
	}

	/** Configure the PL crypto block mode and validate it against the register read-back. */
	XAsufw_WriteReg((XAsufw_PlCfgBase + XASUFW_PL_AES_MODE_CFG_OFFSET),
		XASUFW_PL_AES_CFG_ECB_ENCRYPT_256);

	RegVal = XAsufw_ReadReg(XAsufw_PlCfgBase + XASUFW_PL_AES_MODE_CFG_OFFSET);
	if (RegVal != XASUFW_PL_AES_CFG_ECB_ENCRYPT_256) {
		XAsufw_Printf(DEBUG_GENERAL, "PL mode cfg readback failed, RegVal = %x\r\n",
			RegVal);
		Status = XASUFW_PLEXT_CFG_READBACK_FAILED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	/** Clear the local key object and key arrays. */
	Status = XAsufw_UpdateErrorStatus(Status,
		XAsufw_SMemSet(&KeyObject, sizeof(KeyObject)));
	Status = XAsufw_UpdateErrorStatus(Status, XAsufw_SMemSet(Key, sizeof(Key)));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for PL extension operation command. The whole operation
 * 		is performed in a single request, hence OperationFlags and IsLast fields of the
 * 		request are not used.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the PL crypto operation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_PLEXT_INVALID_PARAM, if any input parameter is invalid.
 * 	- XASUFW_PLEXT_WINDOW_NOT_CONFIGURED, if the PL configuration window is not usable.
 * 	- XASUFW_PLEXT_CFG_READBACK_FAILED, if the isolation, key or mode configuration read-back
 * 	  does not match.
 * 	- XASUFW_DMA_COPY_FAIL, if ASU DMA transfer fails.
 * 	- XASUFW_AES_WRITE_KEY_FAILED, if key write to AES USER key register fails.
 * 	- XASUFW_AES_KEY_CLEAR_ERROR, if AES key clear fails.
 * 	- XASUFW_SSS_CFG_CHECK_FAIL, if SSS configuration fails.
 * 	- XASUFW_DMA_WAIT_FOR_DONE_TIMED_OUT, if DMA transfer wait failed in source or destination
 * 	  channel WaitForDone.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA pointer is not allocated.
 *
 *************************************************************************************************/
static s32 XAsufw_PlExtOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_AesParams *ParamsPtr = (const XAsu_AesParams *)ReqBuf->Arg;
	XAsufw_Dma *DmaPtr = XAsufw_PlExtModule.AsuDmaPtr;

	if (DmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_AesParams);

	if (XAsufw_PlCfgBase == 0U) {
		Status = XASUFW_PLEXT_WINDOW_NOT_CONFIGURED;
		goto END;
	}

	/** Validate the input parameters. */
	Status = XAsufw_PlExtValidateParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Configure SSS to route the DMA to and from the PL interface. */
	Status = XAsufw_SssDmaToPli(DmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SSS_CFG_CHECK_FAIL);
		goto END;
	}

	/** Write the key and configure the PL crypto block. */
	Status = XAsufw_PlExtConfigure(DmaPtr, ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Arm the destination channel, then transfer the input data to the PL crypto block. */
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XASUDMA_DST_CHANNEL,
		ParamsPtr->OutputDataAddr, ParamsPtr->DataLen, XASU_FALSE);
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL,
		ParamsPtr->InputDataAddr, ParamsPtr->DataLen, XASU_TRUE);

	/** Wait till the ASU source DMA done bit to set. */
	Status = XAsufw_WaitForDmaDone(DmaPtr, XASUDMA_SRC_CHANNEL);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Wait till the ASU destination DMA done bit to set. */
	Status = XAsufw_WaitForDmaDone(DmaPtr, XASUDMA_DST_CHANNEL);

END:
	/** Reset the PL key select register and zeroize the key. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_PL_KEY_SEL_OFFSET), 0U);
	Status = XAsufw_UpdateErrorStatus(Status, XAsufw_PlExtKeyZeroize());

	/** Restore SSS configuration to loopback mode. */
	if (DmaPtr != NULL) {
		Status = XAsufw_UpdateErrorStatus(Status, XAsufw_SssDmaLoopback(DmaPtr->SssDmaCfg));
	}

	/** Disable both ASU to PL paths. */
	Status = XAsufw_UpdateErrorStatus(Status,
		XAsufw_PlExtCfgIsolation((ASU_PL_CSR_ISOLATION_DATA_EN_MASK |
		ASU_PL_CSR_ISOLATION_KEY_EN_MASK), 0U));

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_PLI, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_PlExtModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_PLEXT_ENABLE */
/** @} */
