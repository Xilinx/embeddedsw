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
 * </pre>
 *
 ******************************************************************************/

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
#define XPLM_METAHEADER_LEN		(0x140)
#define XPLM_SECURE_CHUNK_BUFFER_ADDR		(0x0402C000U)
#define XPLM_CHUNK_SIZE			(0x2000U)
#define XPLM_AES_RED_KEY_SEL_MASK (0x0000FF00U) /** AES Red key selection mask */
#define XPLM_AES_RED_KEY_SEL_VAL (0x00008200U)	/** AES Red key selection value */
#define XPLM_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK	(0x00288003U)
#define XPLM_BH_IMAGE_DETECTION_KEY_WORD	(0x584C4E58U) /* XLNX */
#define XPLM_SIGNED_IMAGE_MASK		(0x000C0000U)
#define XPLM_PUF_IMAGE_DETECTION_KEY_WORD	(0x50554649U) /* PUFI */

#define XPLM_SEC_LOAD_MIN_ERR_MASK	(0xF00U)
#define XPLM_PUFHD_AUX_CHASH_SIZE		(0x204U)
#define XPLM_PUFHD_TOTAL_SIZE			(0x210U)
#define XPLM_PUFHD_AUX_CHASH_SIZE_WORDS		(XPLM_PUFHD_AUX_CHASH_SIZE / XPLM_WORD_LEN)
#define XPLM_PUFHD_HASH_SET			(EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK)

/* Using chunk buffer as temporary buffer to store PUF HD for digest */
#define XPLM_TEMP_PUF_HD_BUFF		(XPLM_SECURE_CHUNK_BUFFER_ADDR)

/* OSPI configuration */
#define XPLM_OSPI_PHY_MODE	(XPLM_ONE)
#define XPLM_OSPI_NON_PHY_MODE	(XPLM_ZERO)
#define XPLM_OSPI_SDR_MODE	(XPLM_ZERO)
#define XPLM_OSPI_DDR_MODE	(XPLM_ONE)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
static u32 (*XPlm_PullKeySourceRomHook)(const XRomBootRom* InstancePtr, u32* KeySource);
static u32 XPlm_SetPullKeySource(u32 PdiType);
static u32 XPlm_ValidateHeaderChksum(const XRomBootRom* InstancePtr);
u32 XPlm_LoadBitstreamCdo(XRomBootRom *InstancePtr, u32 PdiType);
static u32 XPlm_ReadBootHeader(const XRomBootRom* InstancePtr);
static u32 XPlm_ValidateBootHeader(XRomBootRom* const InstancePtr);
static u32 XPlm_ValidateHashBlock(XRomBootRom* const InstancePtr);
static u32 XPlm_ProcessRtca(void);
static u32 XPlm_SecureClear(XRomBootRom *InstancePtr);
static void XPlm_ZeroizeCriticalMemory(const XRomBootRom* InstancePtr, u32 PdiType);
static u32 XPlm_SecureValidations(XRomBootRom* const InstancePtr);

/************************** Variable Definitions *****************************/
static struct XPlm_FullPdiCrticalInfo_t {
	u32 PUFMode;
} FullPdiCriticalInfo;
static u32 RedKeyCleared = XPLM_ZERO;

/******************************************************************************************/

void XPlm_CaptureCriticalInfo(void)
{
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;

	FullPdiCriticalInfo.PUFMode = InstancePtr->PUFMode;
}

static void XPlm_DumpChunkDebugInfo(XRomSecureChunk* ChunkInstPtr)
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
	XPlm_Printf(DEBUG_INFO, "ScratchPadBuf    : 0x%x\n\r", (UINTPTR)(u32 *)ChunkInstPtr->ScratchPadBuf);
}

static u32 XPlm_ProcessBitstream(const XRomBootRom* InstancePtr, XRomSecureChunk* ChunkInstPtr)
{
	u32 Status = (u32)XST_FAILURE;
	XPlmCdo *CdoPtr = (XPlmCdo *)InstancePtr->FwData;
	u32 CurrentStage = Xil_In32(PMC_GLOBAL_PMC_FW_STATUS) & XPLM_FW_STATUS_STAGE_MASK;

	if (CdoPtr->PdiType == XPLM_PDI_TYPE_FULL) {
		/* Full PDI stage */
		XPlm_LogPlmStage(XPLM_FPDI_CDO_PROCESS_STAGE);
	}
	else {
		/* Partial PDI stage */
		XPlm_LogPlmStage(XPLM_PPDI_CDO_PROCESS_STAGE);
	}

	XPlm_DumpChunkDebugInfo(ChunkInstPtr);

	CdoPtr->BufPtr = (u32 *)ChunkInstPtr->DstAddr;
	CdoPtr->BufLen = ChunkInstPtr->SecureDataLen / 4U;
	CdoPtr->NextChunkAddr = XPLM_SECURE_CHUNK_BUFFER_ADDR + 0x20U;


	Status = XPlm_ProcessCdo(CdoPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((CdoPtr->PdiType == XPLM_PDI_TYPE_FULL) && (ChunkInstPtr->ChunkNum == 0U)) {
		Status = XPlm_ProcessRtca();
		if (Status != XST_SUCCESS){
			goto END;
		}
	}

	ChunkInstPtr->DstAddr = CdoPtr->NextChunkAddr;
	ChunkInstPtr->ScratchPadBuf = (u8 *)ChunkInstPtr->DstAddr;
	ChunkInstPtr->SecureDataLen = 0U;

	XPlm_LogPlmStage(CurrentStage);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Read the OSPI configuration from RTCA and initialize the OSPI driver accordingly.
 *
 * @param	InstancePtr
 *
 * @return
 *
 *****************************************************************************/
static u32 XPlm_ProcessRtca() {
	u32 Status = XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	u32 OspiCfg;
	u32 RefClkDiv;

	XPlm_LogPlmStage(XPLM_RTCA_PROCESS_STAGE);

	OspiCfg = Xil_In32(XPLM_RTCFG_OSPI_CLK_CFG);
	/** - Skip OSPI driver initialization if OSPI is not configured in RTCA. */
	if (OspiCfg == XPLM_ZERO)
	{
		Status = XST_SUCCESS;
		goto END;
	}

	XPlm_UtilRMW(PMC_GLOBAL_RST_OSPI, PMC_GLOBAL_RST_OSPI_RESET_MASK, PMC_GLOBAL_RST_OSPI_RESET_MASK);
	if ((OspiCfg & XPLM_RTCFG_OSPI_EMCCLK_MASK) == XPLM_RTCFG_OSPI_EMCCLK_MASK) {
		XPlm_UtilRMW(PMC_GLOBAL_OSPI_CLK_CTRL,
			     PMC_GLOBAL_OSPI_CLK_CTRL_SRCSEL_MASK | PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK,
			     PMC_GLOBAL_OSPI_CLK_CTRL_SRCSEL_MASK | PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK);
	}
	else {
		RefClkDiv = OspiCfg & XPLM_RTCFG_OSPI_REF_CLK_DIV_MASK;

		if ((RefClkDiv & XPLM_RTCFG_OSPI_REF_CLK_DIV_UNUSED_BITS_MASK) != 0) {
			Status = (u32)XPLM_ERR_RTCA_OSPI_CLK_DIV_MAX;
			goto END;
		}

		XPlm_UtilRMW(PMC_GLOBAL_OSPI_CLK_CTRL, PMC_GLOBAL_OSPI_CLK_CTRL_DIVISOR_MASK, (RefClkDiv << PMC_GLOBAL_OSPI_CLK_CTRL_DIVISOR_SHIFT));
		XPlm_UtilRMW(PMC_GLOBAL_OSPI_CLK_CTRL, PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK, PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK);
	}
	XPlm_UtilRMW(PMC_GLOBAL_RST_OSPI, PMC_GLOBAL_RST_OSPI_RESET_MASK, XPLM_ZERO);

	if (InstancePtr->BootMode == XPLM_BOOT_MODE_OSPI) {
		Status = XPlm_OspiInit();
		if (Status != (u32)XST_SUCCESS) {
			XPlm_Printf(DEBUG_INFO, "XPlm_OspiInit failed with Status: 0x%x\n\r", Status);
			goto END;
		}
		InstancePtr->DeviceRead = XPlm_OspiCopy;
	}
	else if ((InstancePtr->BootMode == XPLM_BOOT_MODE_QSPI24) ||
		(InstancePtr->BootMode == XPLM_BOOT_MODE_QSPI32)) {
		Status = XPlm_QspiInit(InstancePtr->BootMode);
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
 * @brief This is PLM main function
 *
 * @return	Ideally should not return, in case if it reaches end,
 *		error is returned
 *
 *****************************************************************************/
u32 XPlm_LoadFullPdi(void)
{
	volatile u32 Status = (u32)XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	static XRomSecureChunk ChunkInstance;

	XPlm_LogPlmStage(XPLM_FULL_PDI_INIT_STAGE);

	XPlm_PullKeySourceRomHook = HooksTbl->XRom_PullKeySource;
	Status = XPlm_SetPullKeySource(XPLM_PDI_TYPE_FULL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPlm_LogPlmStage(XPLM_FPDI_CDO_LOAD_STAGE);
	Status = XPlm_LoadBitstreamCdo(InstancePtr, XPLM_PDI_TYPE_FULL);

END:
	return Status;
}

u32 XPlm_LoadPartialPdi(void) {
	volatile u32 Status = (u32)XST_FAILURE;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;
	u32 SmapBusWidthCheck[SMAP_BUS_WIDTH_WORD_LEN];
	u32 offset = XPLM_ZERO;
	u32 WidthDetect;
	u32 SbiFifoRemainingDataLen;

	/**
	 * - Read one word from SBI FIFO buffer, if it matches with 0x665599AA, then proceed.
	 * Otherwise discard it and read again, until buffer is empty.
	 */
	while(TRUE) {
		Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, (u32)&WidthDetect, WIDTH_DETECT_WORD_LEN_B, XPLM_SBI_READ_FLAGS_NONE);
		if (WidthDetect == WIDTH_DETECT_WORD)
		{
			break;
		}

		SbiFifoRemainingDataLen = Xil_In32(SLAVE_BOOT_SBI_STATUS2) & SLAVE_BOOT_SBI_STATUS2_WRITE_BUF_SPACE_MASK;
		if (SbiFifoRemainingDataLen == SLAVE_BOOT_SBI_STATUS2_WRITE_BUF_SPACE_DEFVAL)
		{
			Status = XST_SUCCESS;
			goto END;
		}
	}


	XPlm_LogPlmStage(XPLM_PPDI_EVENT_STAGE);

	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_CR_MASK, XPLM_ZERO);

	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK |
			XPLM_FW_STATUS_ZEROIZE_FAIL_MASK, XPLM_ZERO);

	XPlm_LogPlmStage(XPLM_PPDI_INIT_STAGE);

	/* Zeroize ROM Instance */
	Status = Xil_SecureZeroize((u8 *)InstancePtr, sizeof(XRomBootRom));
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PPDI_ROM_INST_ZEROIZE;
		goto END;
	}

	/* Re-initialize ROM Instance */
	HooksTbl->XRom_Initialize_Instance(InstancePtr);
	InstancePtr->ImageHeader = (XRomBootHeader *)XPLM_BOOT_HEADER_START_ADDR;
	InstancePtr->DeviceRead = XPlm_SbiRead;
	InstancePtr->PUFMode = FullPdiCriticalInfo.PUFMode;

	/* Capture Efuse Attributes */
	HooksTbl->XRom_CaptureeFUSEAttribute(InstancePtr);

	/** - Clear previous boot header in PMC RAM. */
	Status = Xil_SecureZeroize((u8 *)InstancePtr->ImageHeader, XPLM_BOOT_HDR_TOTAL_SIZE);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PPDI_BH_ZEROIZE;
		goto END;
	}

	/* Update the first word read from buffer to image header */
	InstancePtr->ImageHeader->WidthDetection = WidthDetect;

	Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, ((u32)InstancePtr->ImageHeader) + WIDTH_DETECT_WORD_LEN_B, XPLM_BOOT_HDR_SIZE_WITHOUT_SMAP_WIDTH - WIDTH_DETECT_WORD_LEN_B, XPLM_SBI_READ_FLAGS_NONE);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_PPDI_SBI_BUF_READ_BH;
		goto END;
	}

	XPlm_LogPlmStage(XPLM_PPDI_VALIDATION_STAGE);

	Status = XPlm_ValidateBootHeader(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_SecureValidations(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_SetPullKeySource(XPLM_PDI_TYPE_PARTIAL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPlm_LogPlmStage(XPLM_PPDI_HB_VALIDATION_STAGE);

	Status = XPlm_ValidateHashBlock(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}


	XPlm_LogPlmStage(XPLM_PPDI_CDO_LOAD_STAGE);

	/* Check if Partition is revoked */
	Status = HooksTbl->XRom_CheckRevocationID(InstancePtr->ImageHeader->PartRevokeId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_LoadBitstreamCdo(InstancePtr, XPLM_PDI_TYPE_PARTIAL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPlm_Printf(DEBUG_GENERAL, "Partial PDI Loaded Successfully\n\r");

END:
	return Status;
}

u32 XPlm_LoadBitstreamCdo(XRomBootRom *InstancePtr, u32 PdiType) {
	volatile u32 Status = (u32)XST_FAILURE;
	u32 StatusTmp = (u32)XST_FAILURE;
	static XRomSecureChunk ChunkInstance;
	static XPlmCdo Cdo;

	/** - Capture User define revision into RTCA register */
	XSECURE_REDUNDANT_IMPL(Xil_Out32, XPLM_RTCFG_USER_DEF_REV,
			InstancePtr->ImageHeader->UserDefRev);

	Status = XPlm_InitCdo(&Cdo);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_INIT_CDO_INSTANCE;
		goto END;
	}

	Cdo.PdiType = PdiType;
	InstancePtr->FwData = &Cdo;

	(void)HooksTbl->XRom_InitChunkInstance(InstancePtr, &ChunkInstance, XROM_DATA_PARTITION);
	XPlm_Printf(DEBUG_INFO, "ChunkInstPtr->SrcAddr : 0x%x\n\r", (u32)ChunkInstance.SrcAddr);

	HooksTbl->XRom_ProcessChunk = XPlm_ProcessBitstream;

	Status = HooksTbl->XRom_SecureLoad(InstancePtr, &ChunkInstance);
	if (Status != XST_SUCCESS) {
		XPlm_ZeroizeCriticalMemory(InstancePtr, PdiType);
		XPlm_Printf(DEBUG_INFO, "Failed to load CDO\r\n");
		if ((Status & XPLM_SEC_LOAD_MIN_ERR_MASK) == XPLM_ZERO)
		{
			Status |= XPLM_ERR_SEC_LOAD;
		}
		goto END;
	}

	/* Clear the last executed CDO command offset on successful boot. */
	Xil_Out32(PMC_GLOBAL_PMC_FW_DATA, 0x0U);

END:
	StatusTmp = XPlm_SecureClear(InstancePtr);
	if (Status == (u32)XST_SUCCESS) {
		Status = StatusTmp;
	}
	return Status;
}

static u32 XPlm_SecureClear(XRomBootRom *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	u32 AesRstVal = 0U;
	if ((InstancePtr->Encstatus != XPLM_ZERO) || (TmpInstancePtr->EncstatusTmp != XPLM_ZERO)) {
		/* Clear Red Keys if configured in RTCA */
		if ((Xil_In32(XPLM_RTCFG_SECURE_CTRL_ADDR) & XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK) ==
				XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK) {
			Xil_Out32(AES_SOFT_RST_ADDR, XPLM_ZERO);
			Xil_Out32(AES_KEY_CLEAR_ADDR, XPLM_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK);
			Status = XPlm_UtilPollForMask(AES_KEY_ZEROED_STATUS_ADDR,
				XPLM_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK, XPLM_TIME_OUT_DEFAULT);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)XPLM_ERR_AES_KEY_CLEAR_TIMEOUT;
				goto END;
			}
			RedKeyCleared = XPLM_ALLFS;
		}
		/* Assert AES Reset */
		Xil_Out32(AES_SOFT_RST_ADDR, AES_SOFT_RST_VAL_MASK);
	}
	/* Assert SHA Reset */
	Xil_Out32(SHA_RESET_ADDR, SHA_RESET_VALUE_MASK);

	Status = (u32)XST_SUCCESS;

END:
	return Status;

}

u32 XPlm_IsSlaveBootMode(u32 BootMode) {
	u32 Status = (u32)XST_FAILURE;

	if ((BootMode == (u32)XPLM_BOOT_MODE_JTAG) ||
		(BootMode == (u32)XPLM_BOOT_MODE_SMAP) ||
		(BootMode == (u32)XPLM_BOOT_MODE_SELECT_SERIAL)) {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

static u32 XPlm_PullKeySource(const XRomBootRom* InstancePtr, u32* KeySource) {
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

static u32 XPlm_ValidatePufDigest(const XRomBootRom* InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	static u8 PufHash[XSECURE_SHA3_256_HASH_LEN] __attribute__((aligned(16)));
	volatile u32 EfuseCtrl	= XPLM_PUFHD_HASH_SET;
	volatile u32 EfuseCtrlTmp = XPLM_PUFHD_HASH_SET;

	/** Do not validate if PUF HD Hash is not set in efuse */
	EfuseCtrl = Xil_In32((EFUSE_CONTROLS) & EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK);
	EfuseCtrlTmp = Xil_In32((EFUSE_CONTROLS) & EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK);
	if ((EfuseCtrl != XPLM_PUFHD_HASH_SET) && (EfuseCtrlTmp != XPLM_PUFHD_HASH_SET)) {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	/** Zeroize local PUF HD buffer */
	Status = Xil_SecureZeroize((u8 *)(UINTPTR)XPLM_TEMP_PUF_HD_BUFF, XPLM_PUFHD_TOTAL_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		Status != (u32)XPLM_ERR_PPDI_PUF_HD_BUFF_ZEROIZE;
	}

	/** Copy PUF HD Syndrome data from PMC_GLOBAL registers */
	XPlm_MemCpy32((u32*)XPLM_TEMP_PUF_HD_BUFF, (u32 *)PMC_GLOBAL_PUF_SYN_0,
			XPLM_PUFHD_AUX_CHASH_SIZE_WORDS);

	/** Calculate SHA Digest over the PUF HD copied into internal RAM */
	Status = HooksTbl->XRom_ShaDigestCalculation((u8 *)XPLM_TEMP_PUF_HD_BUFF,
			XPLM_PUFHD_TOTAL_SIZE, XSECURE_SHA3_256, PufHash);
	if (Status != (u32)XST_SUCCESS) {
		Status |= (u32)XPLM_ERR_PUF_SHA_DIGEST;
		goto END;
	}

	/** Compare the Calculated SHA Digest with the Efuse Digest */
	Status = Xil_SMemCmp_CT((const u8 *)PufHash, XSECURE_SHA3_256_HASH_LEN,
			(u32*)(UINTPTR)EFUSE_PPK2_0, XSECURE_SHA3_256_HASH_LEN,
			XSECURE_SHA3_256_HASH_LEN);
	if (Status != (u32)XST_SUCCESS) {
		Status != (u32)XPLM_ERR_PUF_HD_DIGEST_VALIDATION;
		goto END;
	}

END:
	return Status;
}

static u32 XPlm_SetPullKeySource(u32 PdiType)
{
	u32 Status = (u32)XST_FAILURE;
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
	}
	else {
		FullPdiEncryptionStatus = Xil_In32(PMC_GLOBAL_ENC_STATUS);
		/* If Red Keys are cleared or key is different, specify ROM hook to regenerate KEK */
		if ((FullPdiEncryptionStatus != InstancePtr->ImageHeader->EncryptionStatus) ||
				(RedKeyCleared == XPLM_ALLFS)) {
			HooksTbl->XRom_PullKeySource = XPlm_PullKeySourceRomHook;
			if ((InstancePtr->ImageHeader->EncryptionStatus != XPLM_ENC_STATUS_eFUSE_PUF_KEK) ||
				(InstancePtr->ImageHeader->EncryptionStatus != XPLM_ENC_STATUS_BH_PUF_KEK)) {
				InstancePtr->KEK = XPLM_ZERO;
				TmpInstancePtr->KEKTmp = XPLM_ZERO;
			}
			else {
				/* Check if PUF is Disabled */
				if ((Xil_In32(EFUSE_XILINX_CTRL) & EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK) ==
						EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK) {
					Status = (u32)XPLM_ERR_PPDI_PUF_DISABLED;
					goto END;
				}
				/* Validate PUF HD Digest if programmed */
				Status = XPlm_ValidatePufDigest(InstancePtr);
				if (Status != (u32)XST_SUCCESS) {
					goto END;
				}
			}
		}
		else {
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
 * @return
 *
 ******************************************************************************/
static u32 XPlm_ValidateBootHeader(XRomBootRom* const InstancePtr) {
	u32 Status = (u32)XST_FAILURE;

	/** - Check for XLNX identification word */
	if (InstancePtr->ImageHeader->ImageId != XPLM_BH_IMAGE_DETECTION_KEY_WORD) {
		Status = (u32)XPLM_ERR_PPDI_INVALID_IMG_DET_WORD;
		goto END;
	}

	/** - Calculate header checksum. */
	Status = XPlm_ValidateHeaderChksum(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Capture image attributes. */
	HooksTbl->XRom_CaptureImageAttributes(InstancePtr);

	/** - Validate PMCFW length. */
	if (InstancePtr->ImageHeader->PMCFWLen != XPLM_ZERO)
	{
		Status = (u32)XPLM_ERR_PPDI_PMCFWLEN_NON_ZERO;
		goto END;
	}

	/** - Validate alignment of CDO length. */
	if ((InstancePtr->ImageHeader->TotalDataPartititonLen % 4U) != XPLM_ZERO)
	{
		Status = (u32)XPLM_ERR_PPDI_CDO_LEN_ALIGN;
		goto END;
	}

	if (InstancePtr->ImageHeader->PUFImageId == XPLM_PUF_IMAGE_DETECTION_KEY_WORD) {
		Status = (u32)XPLM_ERR_PPDI_PUF_IMG_NOT_SUPPORTED;
	}

END:
	return Status;
}


static u32 XPlm_EncStatusCheck(XRomBootRom* InstancePtr)
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
			}
			else {
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

static u32 XPlm_SecureValidations(XRomBootRom* const InstancePtr)
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
 * @param	InstancePtr - BootROM Instance Pointer
 *
 * @return
 *		- XPLM_ERR_CALC_BH_CHECKSUM - Error if failed to verify calcualted Boot Header checksum
 *		- XST_SUCCESS - If BootHeader Checksum verified Successfully
 ******************************************************************************/
static u32 XPlm_ValidateHeaderChksum(const XRomBootRom* InstancePtr) {
	u32 Status = (u32)XPLM_ERR_CALC_BH_CHECKSUM;
	u32 Index;
	u32 ChkSum;
	const u32* LocalPtr;

	LocalPtr = &(InstancePtr->ImageHeader->WidthDetection);
	ChkSum = XPLM_ZERO;

	/* Calculate the checksum over the boot header excluding padding */
	for (Index = XPLM_ZERO; Index < (XPLM_BOOT_HDR_CHECKSUM_END_OFFSET / XPLM_WORD_LEN); Index++) {
		ChkSum += LocalPtr[Index];
	}
	ChkSum = (~(ChkSum)&XPLM_ALLFS);

	/* Comparing the calculated checksum with image header checksum in case don't match return error */
	if (ChkSum != InstancePtr->ImageHeader->HeaderChecksum) {
		XPlm_Printf(DEBUG_INFO, "Chksum:0x%08x", ChkSum);
		Status = (u32)XPLM_ERR_CALC_BH_CHECKSUM;
	}
	else {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validate hash block
 *
 * @return
 *
 ******************************************************************************/
static u32 XPlm_ValidateHashBlock(XRomBootRom* const InstancePtr) {
	u32 Status = XST_FAILURE;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	static u8 HashBlockGcmTag[XPLM_SECURE_GCM_TAG_SIZE] __attribute__((aligned(16)));

	/* update hash algorithm for authentication. */
	Status = HooksTbl->XRom_HashAlgoSelectValidation(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* zeroize hash block in RAM memory */
	Status = Xil_SecureZeroize((u8 *)XPLM_HASH_BLOCK_ADDR_IN_RAM, InstancePtr->ImageHeader->HashBlockSize);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_HASH_BLOCK_ZEROIZE;
		goto END;
	}

	if ((InstancePtr->AuthEnabled != XPLM_ZERO) || (TmpInstancePtr->AuthEnabledTmp != XPLM_ZERO)) {
		Status = HooksTbl->XRom_AuthDataValidation(InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		// copy hash block from buffer to PMC RAM
		Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, (u32)XPLM_HASH_BLOCK_ADDR_IN_RAM, InstancePtr->ImageHeader->HashBlockSize, XPLM_SBI_READ_FLAGS_NONE);
		if (Status != XST_SUCCESS) {
			Status = (u32)XPLM_ERR_HASH_BLOCK_READ_SBI_BUF;
			goto END;
		}

		if((InstancePtr->Encstatus != XPLM_ZERO) || (TmpInstancePtr->EncstatusTmp != XPLM_ZERO)) {
			/* Read the GCM Tag from PDI */
			Status = XST_FAILURE;
			Status = XPlm_SbiRead(XPLM_SBI_BUF_ADDR, (u32)HashBlockGcmTag, XPLM_SECURE_GCM_TAG_SIZE, XPLM_SBI_READ_FLAGS_NONE);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)XPLM_ERR_GCM_TAG_READ_SBI_BUF;
				goto END;
			}

			Status = HooksTbl->XRom_ValidateHBAad(InstancePtr, HashBlockGcmTag);
			if (Status != XST_SUCCESS) {
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

	/* Perform Boot Header validation. */
	Status = HooksTbl->XRom_ValidateBootheaderIntegrity();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	if (Status != XST_SUCCESS){
		/* Zeroize Critical Memory in Secure boot failures */
		XPlm_ZeroizeCriticalMemory(InstancePtr, XPLM_PDI_TYPE_PARTIAL);
	}
	return Status;
}


static void XPlm_ZeroizeCriticalMemory(const XRomBootRom* InstancePtr, u32 PdiType)
{
	volatile u32 Status = (u32)XST_FAILURE;
	XRomTmpVar *TmpInstancePtr = HooksTbl->XRom_GetTemporalInstance();
	u32 ZeroizeStatus = 0U;

	/* Clear only in Secure Boot */
	if ((InstancePtr->AuthEnabled == XPLM_ZERO) && (TmpInstancePtr->AuthEnabledTmp == XPLM_ZERO) ||
		(InstancePtr->Encstatus == XPLM_ZERO) && (TmpInstancePtr->EncstatusTmp == XPLM_ZERO)) {
		goto END;
	}

	/* Zeroize Chunk Buffer */
	Status = (u32)XST_FAILURE;
	Status = Xil_SecureZeroize((u8 *)XPLM_SECURE_CHUNK_BUFFER_ADDR, XPLM_CHUNK_SIZE);
	if (Status != XST_SUCCESS) {
		ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
	}

	/* zeroize Boot Header stored in RAM memory */
	Status = (u32)XST_FAILURE;
	Status = Xil_SecureZeroize((u8 *)InstancePtr->ImageHeader, XPLM_BOOT_HDR_TOTAL_SIZE);
	if (Status != XST_SUCCESS) {
		ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
	}
	/* zeroize hash block in RAM memory */
	Status = (u32)XST_FAILURE;
	Status = Xil_SecureZeroize((u8 *)XPLM_HASH_BLOCK_ADDR_IN_RAM, InstancePtr->ImageHeader->HashBlockSize);
	if (Status != XST_SUCCESS) {
		ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
	}

	if (PdiType != XPLM_PDI_TYPE_PARTIAL) {
		/* zeroize RTCA Area for Full PDI Failures */
		Status = (u32)XST_FAILURE;
		Status = Xil_SecureZeroize((u8 *)XPLM_RTCFG_BASEADDR, XPLM_RTCFG_LENGTH_BYTES);
		if (Status != XST_SUCCESS) {
			ZeroizeStatus |= XPLM_FW_STATUS_ZEROIZE_FAIL_MASK;
		}
	}

	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK,
			XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK);
	XPlm_UtilRMW(PMC_GLOBAL_PMC_FW_STATUS, XPLM_FW_STATUS_ZEROIZE_FAIL_MASK,
			ZeroizeStatus);
END:
	return;
}
