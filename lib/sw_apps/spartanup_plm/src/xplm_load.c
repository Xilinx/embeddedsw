/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_load.c
 *
 * This is the file which contains code loading of PDI.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  bm   05/31/24 Initial release
 *       ng   09/17/24 Updated minor error mask for secure rom load api
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xplm_util.h"
#include "xstatus.h"
#include "xplm_hooks.h"
#include "xplm_load.h"
#include "xplm_cdo.h"
#include "xplm_dma.h"
#include "xplm_ospi.h"
#include "xplm_qspi.h"
#include "xplm_hw.h"
#include "xplm_error.h"

/************************** Constant Definitions *****************************/
/** @cond spartanup_plm_internal */
#define XPLM_METAHEADER_LEN		(0x140)
#define XPLM_SECURE_CHUNK_BUFFER_ADDR		(0x0402C000U)
#define XPLM_CHUNK_SIZE			(0x2000U)
#define XPLM_AES_RED_KEY_SEL_MASK (0x0000FF00U) /** AES Red key selection mask */
#define XPLM_AES_RED_KEY_SEL_VAL (0x00008200U)	/** AES Red key selection value */
#define XPLM_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK	(0x00288003U)
#define XPLM_BH_IMAGE_DETECTION_KEY_WORD	(0x584C4E58U) /* XLNX */
#define XPLM_SIGNED_IMAGE_MASK		(0x000C0000U)
#define XPLM_PUF_IMAGE_DETECTION_KEY_WORD	(0x50554649U) /* PUFI */

#define XPLM_SEC_LOAD_MIN_ERR_MASK	(0x3F00U)
#define XPLM_PUFHD_AUX_CHASH_SIZE		(0x204U)
#define XPLM_PUFHD_TOTAL_SIZE			(0x210U)
#define XPLM_PUFHD_AUX_CHASH_SIZE_WORDS		(XPLM_PUFHD_AUX_CHASH_SIZE / XPLM_WORD_LEN)
#define XPLM_PUFHD_HASH_SET			(EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK)

/* Using chunk buffer as temporary buffer to store PUF HD for digest */
#define XPLM_TEMP_PUF_HD_BUFF		(XPLM_SECURE_CHUNK_BUFFER_ADDR)
/** @endcond */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 (*XPlm_PullKeySourceRomHook)(const XRomBootRom *InstancePtr, u32 *KeySource);
static u32 XPlm_SetPullKeySource(u32 PdiType);
static u32 XPlm_ValidateHeaderChksum(const XRomBootRom *InstancePtr);
u32 XPlm_LoadBitstreamCdo(XRomBootRom *InstancePtr, u32 PdiType);
static u32 XPlm_ValidateBootHeader(XRomBootRom *const InstancePtr);
static u32 XPlm_ValidateHashBlock(XRomBootRom *const InstancePtr);
static u32 XPlm_ProcessRtca(void);
static u32 XPlm_SecureClear(XRomBootRom *InstancePtr);
static void XPlm_ZeroizeCriticalMemory(const XRomBootRom *InstancePtr, u32 PdiType);
static u32 XPlm_SecureValidations(XRomBootRom *const InstancePtr);

/************************** Variable Definitions *****************************/
/**
 * Structure to hold the information about PUF modes from Full boot PDI.
 */
/*****************************************************************************/
static struct XPlm_FullPdiCrticalInfo_t {
	u32 PUFMode; /**< PUF mode. */
} FullPdiCriticalInfo;

/** Holds the state of Red key. */
static u32 RedKeyCleared = XPLM_ZERO;

/*****************************************************************************/
/**
 * @brief	Store the PUF mode used during boot for validating partial PDIs.
 */
/*****************************************************************************/
void XPlm_CaptureCriticalInfo(void)
{
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;

	/** - Store the PUF mode from full boot to @ref FullPdiCriticalInfo. */
	FullPdiCriticalInfo.PUFMode = InstancePtr->PUFMode;
}

/** @cond spartanup_plm_internal */
static void XPlm_DumpChunkDebugInfo(XRomSecureChunk *ChunkInstPtr)
{
	XPlm_Printf(DEBUG_INFO, "IsLastChunk      : 0x%x\n\r", ChunkInstPtr->IsLastChunk);
	XPlm_Printf(DEBUG_INFO, "PartitionType    : 0x%x\n\r", ChunkInstPtr->PartitionType);
	XPlm_Printf(DEBUG_INFO, "ChunkNum         : 0x%x\n\r", ChunkInstPtr->ChunkNum);
	XPlm_Printf(DEBUG_INFO, "SrcAddr          : 0x%x\n\r", (u32)ChunkInstPtr->SrcAddr);
	XPlm_Printf(DEBUG_INFO, "RemainingDataLen : 0x%x\n\r", ChunkInstPtr->RemainingDataLen);
	XPlm_Printf(DEBUG_INFO, "DstAddr          : 0x%x\n\r", ChunkInstPtr->DstAddr);
	XPlm_Printf(DEBUG_INFO, "SecureDataLen    : 0x%x\n\r", ChunkInstPtr->SecureDataLen);
	XPlm_Printf(DEBUG_INFO, "KeySource        : 0x%x\n\r", ChunkInstPtr->KeySource);
	XPlm_Printf(DEBUG_INFO, "DataCopied       : 0x%x\n\r", ChunkInstPtr->DataCopied);
	XPlm_Printf(DEBUG_INFO, "ScratchPadBuf    : 0x%x\n\r", ChunkInstPtr->ScratchPadBuf);
}
/** @endcond */

/*****************************************************************************/
/**
 * @brief	This is the handler function to process the bitstream.
 *
 * @param	InstancePtr is the pointer to the ROM instance
 * @param	ChunkInstPtr is the pointer to the chunk instance
 *
 * @returns
 *		- Errors from @ref XPlm_Status_t.
 */
/*****************************************************************************/
static u32 XPlm_ProcessBitstream(const XRomBootRom *InstancePtr, XRomSecureChunk *ChunkInstPtr)
{
	u32 Status = (u32)XST_FAILURE;
	XPlmCdo *CdoPtr = (XPlmCdo *)InstancePtr->FwData;

	/* Save the stage information. */
	XPlm_Stages prev_stage = (XPlm_Stages)(Xil_In32(PMC_GLOBAL_PMC_FW_STATUS) &
					       XPLM_FW_STATUS_STAGE_MASK);

	if (CdoPtr->PdiType == XPLM_PDI_TYPE_FULL) {
		/* Full PDI stage */
		XPlm_LogPlmStage(XPLM_FPDI_CDO_PROCESS_STAGE);
	} else {
		/* Partial PDI stage */
		XPlm_LogPlmStage(XPLM_PPDI_CDO_PROCESS_STAGE);
	}

	XPlm_DumpChunkDebugInfo(ChunkInstPtr);

	CdoPtr->BufPtr = (u32 *)ChunkInstPtr->DstAddr;
	CdoPtr->BufLen = ChunkInstPtr->SecureDataLen / 4U;
	CdoPtr->NextChunkAddr = XPLM_SECURE_CHUNK_BUFFER_ADDR + 0x20U;

	/** - Process CDO. */
	Status = XPlm_ProcessCdo(CdoPtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Process RTCA if it's first chunk during full boot, otherwise skip processing RTCA. */
	if ((CdoPtr->PdiType == XPLM_PDI_TYPE_FULL) && (ChunkInstPtr->ChunkNum == 0U)) {
		Status = XPlm_ProcessRtca();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	ChunkInstPtr->DstAddr = CdoPtr->NextChunkAddr;
	ChunkInstPtr->ScratchPadBuf = (u8 *)ChunkInstPtr->DstAddr;
	ChunkInstPtr->SecureDataLen = 0U;

	/* Restore the previous stage. */
	XPlm_LogPlmStage(prev_stage);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Read the OSPI configuration from RTCA and initialize the OSPI driver accordingly.
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_ProcessRtca(void)
{
	u32 Status = (u32)XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	u32 OspiCfg;
	u32 RefClkDiv;

	XPlm_LogPlmStage(XPLM_RTCA_PROCESS_STAGE);

	OspiCfg = Xil_In32(XPLM_RTCFG_OSPI_CLK_CFG);
	/** - Skip OSPI driver initialization if OSPI is not configured in RTCA. */
	if (OspiCfg == XPLM_ZERO) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** - Assert OSPI reset. */
	XPlm_UtilRMW(PMC_GLOBAL_RST_OSPI, PMC_GLOBAL_RST_OSPI_RESET_MASK, PMC_GLOBAL_RST_OSPI_RESET_MASK);

	/** - Set the OSPI clock source to external or the clock divisor if configured in RTCA. */
	if ((OspiCfg & XPLM_RTCFG_OSPI_EMCCLK_MASK) == XPLM_RTCFG_OSPI_EMCCLK_MASK) {
		XPlm_UtilRMW(PMC_GLOBAL_OSPI_CLK_CTRL,
			     PMC_GLOBAL_OSPI_CLK_CTRL_SRCSEL_MASK | PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK,
			     PMC_GLOBAL_OSPI_CLK_CTRL_SRCSEL_MASK | PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK);
	} else {
		RefClkDiv = OspiCfg & XPLM_RTCFG_OSPI_REF_CLK_DIV_MASK;

		if ((RefClkDiv & XPLM_RTCFG_OSPI_REF_CLK_DIV_UNUSED_BITS_MASK) != 0) {
			Status = (u32)XPLM_ERR_RTCA_OSPI_CLK_DIV_MAX;
			goto END;
		}

		XPlm_UtilRMW(PMC_GLOBAL_OSPI_CLK_CTRL, PMC_GLOBAL_OSPI_CLK_CTRL_DIVISOR_MASK,
			     (RefClkDiv << PMC_GLOBAL_OSPI_CLK_CTRL_DIVISOR_SHIFT));
		XPlm_UtilRMW(PMC_GLOBAL_OSPI_CLK_CTRL, PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK,
			     PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK);
	}

	/** - De-Assert OSPI reset. */
	XPlm_UtilRMW(PMC_GLOBAL_RST_OSPI, PMC_GLOBAL_RST_OSPI_RESET_MASK, XPLM_ZERO);

	/** - Initialize OSPI/QSPI driver based on the boot mode. */
	if (InstancePtr->BootMode == XPLM_BOOT_MODE_OSPI) {
		Status = XPlm_OspiInit();
		if (Status != (u32)XST_SUCCESS) {
			XPlm_Printf(DEBUG_INFO, "XPlm_OspiInit failed with Status: 0x%x\n\r", Status);
			goto END;
		}
		InstancePtr->DeviceRead = XPlm_OspiCopy;
	} else if ((InstancePtr->BootMode == XPLM_BOOT_MODE_QSPI24) ||
		   (InstancePtr->BootMode == XPLM_BOOT_MODE_QSPI32)) {
		Status = XPlm_QspiInit((XPlm_BootModes)InstancePtr->BootMode);
		if (Status != (u32)XST_SUCCESS) {
			XPlm_Printf(DEBUG_INFO, "XPlm_QspiInit failed with Status: 0x%x\n\r", Status);
			goto END;
		}
		InstancePtr->DeviceRead = XPlm_QspiCopy;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Validate and load bitstream CDO.
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_LoadFullPdi(void)
{
	volatile u32 Status = (u32)XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;

	XPlm_LogPlmStage(XPLM_FULL_PDI_INIT_STAGE);

	XPlm_PullKeySourceRomHook = HooksTbl->XRom_PullKeySource;

	/** - Validate key source. */
	Status = XPlm_SetPullKeySource(XPLM_PDI_TYPE_FULL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPlm_LogPlmStage(XPLM_FPDI_CDO_LOAD_STAGE);
	/** - Load bitstream CDO. */
	Status = XPlm_LoadBitstreamCdo(InstancePtr, XPLM_PDI_TYPE_FULL);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Validate parital PDI and process bitstream CDO.
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_LoadPartialPdi(void)
{
	volatile u32 Status = (u32)XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	u32 WidthDetect;
	u32 SbiFifoRemainingDataLen;

	/**
	 * - Read one word from SBI FIFO buffer, if it matches with 0x665599AA, then proceed.
	 * Otherwise discard it and read again, until buffer is empty.
	 */
	while (TRUE) {
		Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, (u32)&WidthDetect, WIDTH_DETECT_WORD_LEN_B,
				      XPLM_DMA_INCR_MODE);
		if (WidthDetect == WIDTH_DETECT_WORD) {
			break;
		}

		SbiFifoRemainingDataLen = Xil_In32(SLAVE_BOOT_SBI_STATUS2) &
					  SLAVE_BOOT_SBI_STATUS2_WRITE_BUF_SPACE_MASK;
		if (SbiFifoRemainingDataLen == SLAVE_BOOT_SBI_STATUS2_WRITE_BUF_SPACE_DEFVAL) {
			Status = XST_SUCCESS;
			goto END;
		}
	}

	XPlm_LogPlmStage(XPLM_PPDI_EVENT_STAGE);

	/** - Clear CR bit in FW_ERR register. */
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_CR_MASK, XPLM_ZERO);

	/** - Clear zeroization pass/fail and completion status bits in FW_STATUS register. */
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK |
		     XPLM_FW_STATUS_ZEROIZE_FAIL_MASK, XPLM_ZERO);

	XPlm_LogPlmStage(XPLM_PPDI_INIT_STAGE);

	/** - Zeroize ROM Instance and re-initialize it. */
	Status = Xil_SecureZeroize((u8 *)InstancePtr, sizeof(XRomBootRom));
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PPDI_ROM_INST_ZEROIZE;
		goto END;
	}
	HooksTbl->XRom_Initialize_Instance(InstancePtr);
	InstancePtr->ImageHeader = (XRomBootHeader *)XPLM_BOOT_HEADER_START_ADDR;
	InstancePtr->DeviceRead = XPlm_SbiRead;
	InstancePtr->PUFMode = FullPdiCriticalInfo.PUFMode;

	/** - Capture Efuse Attributes. */
	HooksTbl->XRom_CaptureeFUSEAttribute(InstancePtr);

	/* Update the first word read from buffer to image header */
	InstancePtr->ImageHeader->WidthDetection = WidthDetect;

	/** - Read boot header from SBI buffer. */
	Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, ((u32)InstancePtr->ImageHeader) + WIDTH_DETECT_WORD_LEN_B,
			      XPLM_BOOT_HDR_SIZE_WITHOUT_SMAP_WIDTH - WIDTH_DETECT_WORD_LEN_B,
			      XPLM_DMA_INCR_MODE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PPDI_SBI_BUF_READ_BH;
		goto END;
	}

	XPlm_LogPlmStage(XPLM_PPDI_VALIDATION_STAGE);

	/** - Validate boot header. */
	Status = XPlm_ValidateBootHeader(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Perform secure validations. */
	Status = XPlm_SecureValidations(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Pull and validate Key source. */
	Status = XPlm_SetPullKeySource(XPLM_PDI_TYPE_PARTIAL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPlm_LogPlmStage(XPLM_PPDI_HB_VALIDATION_STAGE);

	/** - Validate hash block. */
	Status = XPlm_ValidateHashBlock(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPlm_LogPlmStage(XPLM_PPDI_CDO_LOAD_STAGE);

	/** - Validate if Partition is revoked. */
	Status = HooksTbl->XRom_CheckRevocationID(InstancePtr->ImageHeader->PartRevokeId);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Load bitstream CDO. */
	Status = XPlm_LoadBitstreamCdo(InstancePtr, XPLM_PDI_TYPE_PARTIAL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPlm_Printf(DEBUG_GENERAL, "Partial PDI Loaded Successfully\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Load bitstream CDO.
 *
 * @param	InstancePtr is the pointer to the ROM instance
 * @param	PdiType is used to indicate whether it is a Full or Partial PDI.
 *		Expected values are @ref XPLM_PDI_TYPE_FULL or @ref XPLM_PDI_TYPE_PARTIAL.
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_LoadBitstreamCdo(XRomBootRom *InstancePtr, u32 PdiType)
{
	volatile u32 Status = (u32)XST_FAILURE;
	volatile u32 StatusTmp = (u32)XST_FAILURE;
	volatile u32 SStatusTmp = (u32)XST_FAILURE;
	static XRomSecureChunk ChunkInstance;
	static XPlmCdo Cdo;

	/** - Capture User define revision into RTCA register */
	XSECURE_REDUNDANT_IMPL(Xil_Out32, XPLM_RTCFG_USER_DEF_REV,
			       InstancePtr->ImageHeader->UserDefRev);

	/** - Initialize CDO instance. */
	Status = XPlm_InitCdo(&Cdo);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_INIT_CDO_INSTANCE;
		goto END;
	}

	Cdo.PdiType = PdiType;
	InstancePtr->FwData = &Cdo;

	/** - Initialize chunk instance. */
	Status = (u32)XST_FAILURE;
	Status = HooksTbl->XRom_InitChunkInstance(InstancePtr, &ChunkInstance, XROM_DATA_PARTITION);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_INIT_CHUNK_INST;
		goto END;
	}

	XPlm_Printf(DEBUG_INFO, "ChunkInstPtr->SrcAddr : 0x%x\n\r", (u32)ChunkInstance.SrcAddr);

	/**
	 * - Update the pointer to bitstream processing in @ref HooksTbl to @ref XPlm_ProcessBitstream
	 * and validate if it's set correctly.
	 */
	HooksTbl->XRom_ProcessChunk = XPlm_ProcessBitstream;
	if (HooksTbl->XRom_ProcessChunk != XPlm_ProcessBitstream) {
		Status = (u32)XPLM_ERR_GLITCH_DETECTED;
		goto END;
	}

	/**
	 * - Start loading the bitstream using ROM hooks. If failed to load bitstream clear critical
	 *   memory regions using @ref XPlm_ZeroizeCriticalMemory.
	 */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, HooksTbl->XRom_SecureLoad, InstancePtr, &ChunkInstance);
	if ((Status != (u32)XST_SUCCESS) || (StatusTmp != (u32)XST_SUCCESS) || (Status != StatusTmp)) {
		XSECURE_REDUNDANT_IMPL(XPlm_ZeroizeCriticalMemory, InstancePtr, PdiType);
		XPlm_Printf(DEBUG_INFO, "Failed to load CDO\r\n");
		if ((Status & XPLM_SEC_LOAD_MIN_ERR_MASK) == XPLM_ZERO) {
			Status |= (u32)XPLM_ERR_SEC_LOAD;
		}
		goto END;
	}

	/** - Clear the last executed CDO command offset on successful boot. */
	Xil_Out32(PMC_GLOBAL_PMC_FW_DATA, 0x0U);

END:
	/** - Clear secure information and reset crypto engines using @ref XPlm_SecureClear. */
	XSECURE_TEMPORAL_IMPL(StatusTmp, SStatusTmp, XPlm_SecureClear, InstancePtr);
	if (Status == (u32)XST_SUCCESS) {
		if ((StatusTmp != (u32)XST_SUCCESS) || (SStatusTmp != (u32)XST_SUCCESS)
		    || (StatusTmp != SStatusTmp)) {
			Status = (u32)XPLM_ERR_SECURE_CLR;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Clear secure information from RTCA and reset crypto engines.
 *
 * @param	InstancePtr is the pointer to the ROM instance
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_SecureClear(XRomBootRom *InstancePtr)
{
	u32 Status = (u32)XST_SUCCESS;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	u32 SecureCtrl;
	u32 SecureCtrlTmp;

	/**
	 * - If encryption is enabled,
	 *	- clear RED keys if configured in RTCA,
	 *		- assert AES soft reset,
	 *		- assert AES key clear and validate it.
	 *	- assert AES reset.
	 * - Assert SHA reset.
	 */
	if ((InstancePtr->Encstatus != XPLM_ZERO) || (TmpInstancePtr->EncstatusTmp != XPLM_ZERO)) {
		/* Clear Red Keys if configured in RTCA */
		SecureCtrl = Xil_In32(XPLM_RTCFG_SECURE_CTRL_ADDR) & XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK;
		SecureCtrlTmp = Xil_In32(XPLM_RTCFG_SECURE_CTRL_ADDR) & XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK;
		if ((SecureCtrl == XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK) ||
		    (SecureCtrlTmp == XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK)) {
			XSECURE_REDUNDANT_IMPL(Xil_Out32, AES_SOFT_RST_ADDR, XPLM_ZERO);
			XSECURE_REDUNDANT_IMPL(Xil_Out32, AES_KEY_CLEAR_ADDR,
					       XPLM_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK);
			Status = XPlm_UtilPollForMask(AES_KEY_ZEROED_STATUS_ADDR,
						      XPLM_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK,
						      XPLM_TIME_OUT_DEFAULT);
			RedKeyCleared = XPLM_ALLFS;
		}
		/* Assert AES Reset */
		Status |= Xil_SecureOut32(AES_SOFT_RST_ADDR, AES_SOFT_RST_VAL_MASK);
	}
	/* Assert SHA Reset */
	Status |= Xil_SecureOut32(SHA_RESET_ADDR, SHA_RESET_VALUE_MASK);
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	Get the encryption keysource type and update the output parameter with keysource.
 *
 * @param	InstancePtr is the pointer to the ROM instance
 * @param	KeySource is the output to save the key source
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_PullKeySource(const XRomBootRom *InstancePtr, u32 *KeySource)
{
	u32 Status = (u32)XST_FAILURE;

	Status = XPlm_PullKeySourceRomHook(InstancePtr, KeySource);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	switch (InstancePtr->ImageHeader->EncryptionStatus) {
		case XPLM_ENC_STATUS_eFUSE_PUF_KEK:
		case XPLM_ENC_STATUS_eFUSE_FAMILY_KEK:
		case XPLM_ENC_STATUS_BH_PUF_KEK:
		case XPLM_ENC_STATUS_BH_FAMILY_KEK:
			*KeySource = ((*KeySource & ~XPLM_AES_RED_KEY_SEL_MASK) | XPLM_AES_RED_KEY_SEL_VAL);
			break;
		default:
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Validate PUF digest.
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_ValidatePufDigest(void)
{
	volatile u32 Status = (u32)XST_FAILURE;
	static u8 PufHash[XSECURE_SHA3_256_HASH_LEN] __attribute__((aligned(16)));
	volatile u32 EfuseCtrl	= XPLM_PUFHD_HASH_SET;
	volatile u32 EfuseCtrlTmp = XPLM_PUFHD_HASH_SET;

	/** - Skip validation if PUF HD Hash is not set in efuse. */
	EfuseCtrl = Xil_In32(EFUSE_CONTROLS) & EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK;
	EfuseCtrlTmp = Xil_In32(EFUSE_CONTROLS) & EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK;
	if ((EfuseCtrl != XPLM_PUFHD_HASH_SET) && (EfuseCtrlTmp != XPLM_PUFHD_HASH_SET)) {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	/** - Clear local PUF HD buffer. */
	Status = Xil_SecureZeroize((u8 *)(UINTPTR)XPLM_TEMP_PUF_HD_BUFF, XPLM_PUFHD_TOTAL_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PPDI_PUF_HD_BUFF_ZEROIZE;
		goto END;
	}

	/** - Store PUF HD Syndrome data from PMC_GLOBAL registers to local PUF HD buffer. */
	XPlm_MemCpy32((u32 *)XPLM_TEMP_PUF_HD_BUFF, (u32 *)PMC_GLOBAL_PUF_SYN_0,
		      XPLM_PUFHD_AUX_CHASH_SIZE_WORDS);

	/** - Calculate SHA Digest on the PUF HD. */
	Status = (u32)XST_FAILURE;
	Status = HooksTbl->XRom_ShaDigestCalculation((u8 *)XPLM_TEMP_PUF_HD_BUFF,
		 XPLM_PUFHD_TOTAL_SIZE, XSECURE_SHA3_256, PufHash);
	if (Status != (u32)XST_SUCCESS) {
		Status |= (u32)XPLM_ERR_PUF_SHA_DIGEST;
		goto END;
	}

	/** - Compare the calculated SHA Digest with the Efuse Digest. */
	Status = (u32)XST_FAILURE;
	Status = Xil_SMemCmp_CT((const u8 *)PufHash, XSECURE_SHA3_256_HASH_LEN,
				(u32 *)(UINTPTR)EFUSE_PPK2_0, XSECURE_SHA3_256_HASH_LEN,
				XSECURE_SHA3_256_HASH_LEN);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PUF_HD_DIGEST_VALIDATION;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Update the ROM hook to use @ref XPlm_PullKeySource for Full PDI or ROM implementation
 * of PullKeySource for partial PDI.
 *
 * @param	PdiType is used to indicate whether it is a Full or Partial PDI.
 *		Expected values are @ref XPLM_PDI_TYPE_FULL or @ref XPLM_PDI_TYPE_PARTIAL.
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_SetPullKeySource(u32 PdiType)
{
	volatile u32 Status = (u32)XST_FAILURE;
	volatile u32 StatusTmp = (u32)XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	u32 FullPdiEncryptionStatus = XPLM_ALLFS;

	/* Indicate ROM Hooks that Pull Key Source has to be done again */
	InstancePtr->PullKeysourceStatus = XPLM_ZERO;
	TmpInstancePtr->PullKeysourceStatusTmp = XPLM_ZERO;

	if (PdiType == XPLM_PDI_TYPE_FULL) {
		/* Update Hooks to use wrapped PullKeysource PLM API */
		HooksTbl->XRom_PullKeySource = XPlm_PullKeySource;
		/*
		 * Clear KEK value even though it's present as PLM uses
		 * the RED key decrypted by ROM
		 */
		InstancePtr->KEK = XPLM_ZERO;
		TmpInstancePtr->KEKTmp = XPLM_ZERO;
	} else {
		FullPdiEncryptionStatus = Xil_In32(PMC_GLOBAL_ENC_STATUS);
		/* If Red Keys are cleared or key is different, specify ROM hook to regenerate KEK */
		if ((FullPdiEncryptionStatus != InstancePtr->ImageHeader->EncryptionStatus) ||
		    (RedKeyCleared == XPLM_ALLFS)) {
			HooksTbl->XRom_PullKeySource = XPlm_PullKeySourceRomHook;
			if ((InstancePtr->ImageHeader->EncryptionStatus != XPLM_ENC_STATUS_eFUSE_PUF_KEK) &&
			    (InstancePtr->ImageHeader->EncryptionStatus != XPLM_ENC_STATUS_BH_PUF_KEK)) {
				InstancePtr->KEK = XPLM_ZERO;
				TmpInstancePtr->KEKTmp = XPLM_ZERO;
			} else {
				/* Check if PUF is Disabled */
				if ((Xil_In32(EFUSE_XILINX_CTRL) & EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK) ==
				    EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK) {
					Status = (u32)XPLM_ERR_PPDI_PUF_DISABLED;
					goto END;
				}
				/* Validate PUF HD Digest if programmed */
				XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XPlm_ValidatePufDigest);
				if ((Status != (u32)XST_SUCCESS) || (StatusTmp != (u32)XST_SUCCESS) || (Status != StatusTmp)) {
					Status = (u32)XPLM_ERR_PPDI_INVLD_PUF_DIGEST;
					goto END;
				}
			}
		} else {
			HooksTbl->XRom_PullKeySource = XPlm_PullKeySource;
			InstancePtr->KEK = XPLM_ZERO;
			TmpInstancePtr->KEKTmp = XPLM_ZERO;
		}
	}

	Status = (u32)XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Validate Boot header
 *
 * @param	InstancePtr is the pointer to the ROM instance
 *
 * @return
 * 		- XST_SUCCESS on success.
 *		- XPLM_ERR_PPDI_INVALID_IMG_DET_WORD if failed to validate XLNX identification word.
 *		- XPLM_ERR_PPDI_PMCFWLEN_NON_ZERO if failed to validate PMC firmware length.
 *		- XPLM_ERR_PPDI_CDO_LEN_ALIGN if failed to validate alignment of CDO length.
 *		- XPLM_ERR_PPDI_PUF_IMG_NOT_SUPPORTED if PUF image ID is set for partial PDI.
 *		- and errors from @ref XPlm_Status_t.
 *
 ******************************************************************************/
static u32 XPlm_ValidateBootHeader(XRomBootRom *const InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;

	/** - Check for XLNX identification word. */
	if (InstancePtr->ImageHeader->ImageId != XPLM_BH_IMAGE_DETECTION_KEY_WORD) {
		Status = (u32)XPLM_ERR_PPDI_INVALID_IMG_DET_WORD;
		goto END;
	}

	/** - Calculate header checksum. */
	Status = XPlm_ValidateHeaderChksum(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Capture image attributes. */
	HooksTbl->XRom_CaptureImageAttributes(InstancePtr);

	/** - Validate PMCFW length. */
	if ((InstancePtr->ImageHeader->PMCFWLen != XPLM_ZERO)
	    || (InstancePtr->ImageHeader->TotalPMCFWLen != XPLM_ZERO)) {
		Status = (u32)XPLM_ERR_PPDI_PMCFWLEN_NON_ZERO;
		goto END;
	}

	/** - Validate alignment of CDO length. */
	if (((InstancePtr->ImageHeader->TotalDataPartititonLen % 4U) != XPLM_ZERO)
	    || ((InstancePtr->ImageHeader->DataPartititonLen % 4U) != XPLM_ZERO)) {
		Status = (u32)XPLM_ERR_PPDI_CDO_LEN_ALIGN;
		goto END;
	}

	if (InstancePtr->ImageHeader->PUFImageId == XPLM_PUF_IMAGE_DETECTION_KEY_WORD) {
		Status = (u32)XPLM_ERR_PPDI_PUF_IMG_NOT_SUPPORTED;
	}

END:
	return Status;
}

/** @cond spartanup_plm_internal */
static u32 XPlm_EncStatusCheck(XRomBootRom *InstancePtr)
{
	u32 Status = (u32)XPLM_ERR_PPDI_INVALID_BH_ENC_STATUS;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();

	switch (InstancePtr->ImageHeader->EncryptionStatus) {
		case XPLM_ENC_STATUS_UN_ENCRYPTED:
		case XPLM_ENC_STATUS_eFUSE_KEY:
			Status = (u32)XST_SUCCESS;
			break;
		case XPLM_ENC_STATUS_eFUSE_PUF_KEK:
		case XPLM_ENC_STATUS_BH_PUF_KEK:
			InstancePtr->KEK = XPLM_ALLFS;
			TmpInstancePtr->KEKTmp = XPLM_ALLFS;
			Status = (u32)XST_SUCCESS;
			break;
		case XPLM_ENC_STATUS_eFUSE_FAMILY_KEK:
		case XPLM_ENC_STATUS_BH_FAMILY_KEK:
			if (RedKeyCleared != XPLM_ZERO) {
				Status = (u32)XPLM_ERR_PPDI_FAMILY_KEY_NOT_ALLOWED;
			} else {
				InstancePtr->KEK = XPLM_ALLFS;
				TmpInstancePtr->KEKTmp = XPLM_ALLFS;
				Status = (u32)XST_SUCCESS;
			}
			break;
		default:
			Status = (u32)XPLM_ERR_PPDI_INVALID_BH_ENC_STATUS;
			break;
	}
	return Status;
}
/** @endcond */

/*****************************************************************************/
/**
 * @brief	Perform secure validations.
 *
 * @param	InstancePtr is the pointer to the ROM instance
 *
 * @return
 *		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_SecureValidations(XRomBootRom *const InstancePtr)
{
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	volatile u32 Status = (u32)XST_FAILURE;
	volatile u32 EncStatus = XPLM_ALLFS;
	volatile u32 EncStatusTmp = XPLM_ALLFS;
	volatile u32 AuthStatus = XPLM_ALLFS;
	volatile u32 AuthStatusTmp = XPLM_ALLFS;

	/* Validate EncryptionStatus of Boot Header */
	Status = XPlm_EncStatusCheck(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* DEC_EFUSE_ONLY Check */
	if ((InstancePtr->eFUSEEncstatus != XPLM_ZERO) ||
	    (TmpInstancePtr->eFUSEEncstatusTmp != XPLM_ZERO)) {
		if (InstancePtr->ImageHeader->EncryptionStatus !=
		    XPLM_ENC_STATUS_eFUSE_PUF_KEK) {
			Status = (u32)XPLM_ERR_PPDI_DEC_EFUSE_ONLY;
			goto END;
		}
	}

	/* A-HWROT Check */
	if ((InstancePtr->eFUSEAuthStatus != XPLM_ZERO) ||
	    (TmpInstancePtr->eFUSEAuthStatusTmp != XPLM_ZERO)) {
		if ((InstancePtr->ImageHeader->ImageAttributes &
		     XPLM_SIGNED_IMAGE_MASK) != XPLM_SIGNED_IMAGE_MASK) {
			Status = (u32)XPLM_ERR_PPDI_AHWROT_UNSIGNED_IMG;
			goto END;
		}
	}

	EncStatus = Xil_In32(PMC_GLOBAL_ENC_STATUS);
	EncStatusTmp = Xil_In32(PMC_GLOBAL_ENC_STATUS);
	AuthStatus = Xil_In32(PMC_GLOBAL_AUTH_STATUS);
	AuthStatusTmp = Xil_In32(PMC_GLOBAL_AUTH_STATUS);

	if ((EncStatus != EncStatusTmp) || (AuthStatus != AuthStatusTmp)) {
		Status = (u32)XPLM_ERR_PPDI_TEMPORAL_ERR_AUTH_ENC_STATUS;
		goto END;
	}

	/* Check if FPDI Auth Status is same as PPDI Auth Status */
	if ((AuthStatus != InstancePtr->AuthEnabled) ||
	    (AuthStatusTmp != TmpInstancePtr->AuthEnabledTmp)) {
		Status = (u32)XPLM_ERR_PPDI_SEC_TRANSITION_AUTH;
		goto END;
	}

	/*
	 * If Encryption Status is not matching with FPDI,
	 * Do not allow boot unless the PPDI is authenticated
	 */
	if ((EncStatus != (u32)InstancePtr->ImageHeader->EncryptionStatus) ||
	    (EncStatusTmp != (u32)InstancePtr->ImageHeader->EncryptionStatus)) {
		if ((InstancePtr->AuthEnabled != XPLM_ALLFS) ||
		    (TmpInstancePtr->AuthEnabledTmp != XPLM_ALLFS)) {
			Status = (u32)XPLM_ERR_PPDI_SEC_TO_SEC_KEY_MISMATCH;
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	Checksum calculation & comparison of boot header
 *
 * @param	InstancePtr is the pointer to the ROM instance
 *
 * @return
 *		- XPLM_ERR_CALC_BH_CHECKSUM - Error if failed to verify calcualted Boot Header checksum
 *		- XST_SUCCESS - If BootHeader Checksum verified Successfully
 *
 ******************************************************************************/
static u32 XPlm_ValidateHeaderChksum(const XRomBootRom *InstancePtr)
{
	u32 Status = (u32)XPLM_ERR_CALC_BH_CHECKSUM;
	u32 Index;
	u32 ChkSum;
	const u32 *LocalPtr;

	LocalPtr = &(InstancePtr->ImageHeader->WidthDetection);
	ChkSum = XPLM_ZERO;

	/** - Calculate the sum of all words in boot header. */
	for (Index = XPLM_ZERO; Index < (XPLM_BOOT_HDR_CHECKSUM_END_OFFSET / XPLM_WORD_LEN); Index++) {
		ChkSum += LocalPtr[Index];
	}

	/** - Invert the calculated checksum and verify with the checksum provided in boot header. */
	ChkSum = (~(ChkSum)&XPLM_ALLFS);
	if (ChkSum != InstancePtr->ImageHeader->HeaderChecksum) {
		XPlm_Printf(DEBUG_INFO, "Chksum:0x%08x", ChkSum);
		Status = (u32)XPLM_ERR_CALC_BH_CHECKSUM;
	} else {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validate hash block
 *
 * @param	InstancePtr is the pointer to the ROM instance
 *
 * @return
 *		- XPLM_ERR_HASH_BLOCK_ZEROIZE if failed to zeroize hash block in PMC RAM.
 *		- XPLM_ERR_HASH_BLOCK_READ_SBI_BUF if failed to read hash block data from SBI
 *		buffer.
 *		- XPLM_ERR_GCM_TAG_READ_SBI_BUF if failed to read GCM tag from SBI buffer.
 *		- XPLM_ERR_HASH_BLOCK_AUTHENTICATION if failed to authenticate hash block
 *		- and errors from @ref XPlm_Status_t.
 *
 ******************************************************************************/
static u32 XPlm_ValidateHashBlock(XRomBootRom *const InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	static u8 HashBlockGcmTag[XPLM_SECURE_GCM_TAG_SIZE] __attribute__((aligned(16)));

	/** - Update hash algorithm for authentication. */
	Status = HooksTbl->XRom_HashAlgoSelectValidation(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Zeroize hash block in RAM memory. */
	Status = Xil_SecureZeroize((u8 *)XPLM_HASH_BLOCK_ADDR_IN_RAM,
				   InstancePtr->ImageHeader->HashBlockSize);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_HASH_BLOCK_ZEROIZE;
		goto END;
	}

	/**
	 * - Validate authentication if authentication is enabled. Otherwise, if encryption is
	 *   enabled, copy hash block and GCM tag and validate AAD.
	 */
	if ((InstancePtr->AuthEnabled != XPLM_ZERO) || (TmpInstancePtr->AuthEnabledTmp != XPLM_ZERO)) {
		Status = HooksTbl->XRom_AuthDataValidation(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	} else {
		// copy hash block from buffer to PMC RAM
		Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, (u32)XPLM_HASH_BLOCK_ADDR_IN_RAM,
				      InstancePtr->ImageHeader->HashBlockSize, XPLM_DMA_INCR_MODE);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_HASH_BLOCK_READ_SBI_BUF;
			goto END;
		}

		if ((InstancePtr->Encstatus != XPLM_ZERO) || (TmpInstancePtr->EncstatusTmp != XPLM_ZERO)) {
			/* Read the GCM Tag from PDI */
			Status = (u32)XST_FAILURE;
			Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, (u32)HashBlockGcmTag,
					      XPLM_SECURE_GCM_TAG_SIZE, XPLM_DMA_INCR_MODE);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)XPLM_ERR_GCM_TAG_READ_SBI_BUF;
				goto END;
			}

			Status = HooksTbl->XRom_ValidateHBAad(InstancePtr, HashBlockGcmTag);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
		}
	}

	/* Validate Hash Block Authentication Status (Glitch detection) */
	if (((InstancePtr->AuthEnabled != XPLM_ZERO) || (TmpInstancePtr->AuthEnabledTmp != XPLM_ZERO)) &&
	    (InstancePtr->HashBlockAuthStatus != XPLM_ALLFS)) {
		Status = (u32)XPLM_ERR_HASH_BLOCK_AUTHENTICATION;
		goto END;
	}

	/** - Validate boot header integrity. */
	Status = HooksTbl->XRom_ValidateBootheaderIntegrity();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

END:
	/** - In case of failure in validation, zeroize critical memory regions. */
	if (Status != (u32)XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlm_ZeroizeCriticalMemory, InstancePtr, XPLM_PDI_TYPE_PARTIAL);
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Zeroize critical memory regions.
 *
 * @param	InstancePtr is the pointer to the ROM instance
 * @param	PdiType is used to indicate whether it is a Full or Partial PDI.
 *		Expected values are @ref XPLM_PDI_TYPE_FULL or @ref XPLM_PDI_TYPE_PARTIAL.
 *
 *****************************************************************************/
static void XPlm_ZeroizeCriticalMemory(const XRomBootRom *InstancePtr, u32 PdiType)
{
	volatile u32 Status = (u32)XST_FAILURE;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	u32 ZeroizeStatus = 0U;

	/* Skip updating the status and clearing secure memory regions in non-secure boot. */
	if (((InstancePtr->AuthEnabled == XPLM_ZERO) && (TmpInstancePtr->AuthEnabledTmp == XPLM_ZERO)) &&
	    ((InstancePtr->Encstatus == XPLM_ZERO) && (TmpInstancePtr->EncstatusTmp == XPLM_ZERO))) {
		goto END;
	}

	/** - Zeroize Chunk Buffer. */
	Status = (u32)XST_FAILURE;
	Status = Xil_SecureZeroize((u8 *)XPLM_SECURE_CHUNK_BUFFER_ADDR, XPLM_CHUNK_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
	}

	/** - Zeroize Boot Header stored in RAM memory. */
	Status = (u32)XST_FAILURE;
	Status = Xil_SecureZeroize((u8 *)InstancePtr->ImageHeader, XPLM_BOOT_HDR_TOTAL_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
	}
	/** - Zeroize hash block in RAM memory. */
	Status = (u32)XST_FAILURE;
	Status = Xil_SecureZeroize((u8 *)XPLM_HASH_BLOCK_ADDR_IN_RAM,
				   InstancePtr->ImageHeader->HashBlockSize);
	if (Status != (u32)XST_SUCCESS) {
		ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
	}

	/** - Zeroize RTCA Area if it's full boot PDI. */
	if (PdiType != XPLM_PDI_TYPE_PARTIAL) {
		Status = (u32)XST_FAILURE;
		Status = Xil_SecureZeroize((u8 *)XPLM_RTCFG_BASEADDR, XPLM_RTCFG_LENGTH_BYTES);
		if (Status != (u32)XST_SUCCESS) {
			ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
		}
	}

	/** - Update the zeroization status to FW_STATUS register. */
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK,
		     XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK);
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_ZEROIZE_FAIL_MASK, ZeroizeStatus);
END:
	return;
}

/** @} end of spartanup_plm_apis group*/
