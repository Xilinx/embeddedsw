/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_2ve_2vm/xplm_plat.c
*
* This file contains the PLMI versal_2ve_2vm platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       dd   03/28/2023 Updated doxygen comments
* 1.02  sk   05/22/2023 Added redundancy for validate checksum
* 1.10  mss  12/06/2023 Added ErrorCode in XPlm_CompatibilityCheck API
*                       for OptionalDataLen Check
*       bm   02/23/2024 Ack In-Place PLM Update request after complete restore
* 1.11  ng   04/30/2024 Fixed doxygen grouping
*       obs  12/18/2024 Fixed GCC Warnings
*       am   02/22/2025 Added Puf on dmand regeneration support
*       sk   03/25/2025 Updated platform name
*       pre  09/09/2025 Added status reset before reusing
*       pre  03/23/2026 Added PLM hash extension to TPM after PLM update
* 2.0   sd   07/01/2026 Updated logic to handle zero-length optional-data
*       vm   07/14/2026 Start ASUFW keep-alive monitor after in-place PLM update
*       vm   08/22/2026 Added compatibility check for in-place ASU update
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xplm_apis Versal PLM APIs
 * @{
 * @cond xplm_internal
 */

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xparameters.h"
#include "xplmi_update.h"
#include "xplmi.h"
#include "xilpdi.h"
#include "xloader.h"
#include "xloader_plat.h"
#ifdef PLM_OCP
#include "xocp.h"
#endif
#ifdef XPLMI_IPI_DEVICE_ID
#include "xplmi_ipi.h"
#endif
#include "xpuf.h"
#ifdef PLM_TPM
#include "xtpm.h"
#endif
#include "xplmi_asufw_keepalive.h"
#include "xplmi_asu_update.h"

/************************** Constant Definitions *****************************/
#define XPLMI_PSM_COUNTER_VER 		(1U) /**< PSM counter version */
#define XPLMI_PSM_COUNTER_LCVER		(1U) /**< PSM counter lowest compatible version */
#define XPLMI_PSM_KEEP_ALIVE_STS_VER 	(1U) /**< PSM keep alive status version */
#define XPLMI_PSM_KEEP_ALIVE_STS_LCVER	(1U) /**< PSM keep alive status lowest compatible version */

#define XPLMI_BH_PUF_SHUTTER_VALUE_OFFSET (0x60U) /**< BootHeader PUF shutter value offset. */
#define XPLMI_BH_IMAGE_ATTRIBUTE_OFFSET	(0x24U) /**< BootHeader PUF Image attribute offset. */
#define XPLMI_BH_PUF_HD_MASK_BITS	(0x3U) /**< BootHeader PUF HD mask bits. */
#define XPLMI_BH_PUF_HD_MASK_SHIFT	(0x6U) /**< BootHeader PUF HD mask shift. */
#define XPLMI_BH_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT	(31U) /**< BootHeader PUF global variation filter enable shift. */
#define XPLMI_EFUSE_CACHE_ADDRESS	(0xF1250000U) /** eFuse cache address. */
#define XPLMI_EFUSE_CACHE_PUF_CHASH_OFFSET	(0xA8U) /** eFuse cache puf CHASH offset. */

/* ASU firmware image ID */
#define XPLMI_ASUFW_IMAGE_ID			(0x1C000002U) /**< ASUFW image ID in PDI */

#define XPLM_ASU_DS_INFO_MIN_LEN_IN_WORDS	(3U) /**< Shortest usable ASU data structure
						information block: header word, one version
						record and checksum word */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#define XPLMI_DS_CNT			(u32)(__data_struct_end - __data_struct_start) /**< Data structure count */
#define XPLMI_IPU_RESPONSE_CNT		(1U)

/************************** Variable Definitions *****************************/
extern XPlmi_DsEntry __data_struct_start[];
extern XPlmi_DsEntry __data_struct_end[];

/*****************************************************************************/
/**
* @brief	This function enables SLVERR for PMC related modules
*
*****************************************************************************/
void XPlm_EnablePlatformSlaveErrors(void)
{
	/* Add versal_2ve_2vm specific slave errors */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function checks compatibility between data structures of
 * 		old and new PLM
 *
 * @param	PdiAddr is the address of the PDI which has new PLM
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlm_CompatibilityCheck(u32 PdiAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 IdString;
	u32 Offset;
	u32 OptionalDataLen;
	u32 OptionalDataEndAddr;
	u32 Index;
	u32 PdiVersion;
	u32 OptDataIncr;
	XPlmi_DsVer *DsVerList = NULL;
	XPlmi_DsEntry *DsEntry = NULL;

	/* Check if given PDI address is in lower DDR */
	if ((PdiAddr > XPLMI_2GB_END_ADDR) || (PdiAddr + XIH_BH_META_HDR_OFFSET >
		XPLMI_2GB_END_ADDR)) {
		Status = (int)XPLM_ERR_INVALID_UPDATE_ADDR;
		goto END;
	}

	/* Check if given PDI is a full PDI */
	IdString = XPlmi_In32(PdiAddr + XIH_BH_IMAGE_IDENT_OFFSET);
	if (IdString != XIH_BH_IMAGE_IDENT) {
		Status = XPLM_ERR_INVALID_UPDATE_BH_IDENT;
		goto END;
	}

	Offset = PdiAddr + (u32)XPlmi_In32(PdiAddr + XIH_BH_META_HDR_OFFSET);
	if ((PdiAddr > Offset) || (Offset > XPLMI_2GB_END_ADDR)) {
		Status = XPLM_ERR_INVALID_UPDATE_ADDR;
		goto END;

	}
	/* Check if IHT is within lower DDR */
	if (Offset + XIH_IHT_LEN > XPLMI_2GB_END_ADDR) {
		Status = XPLM_ERR_INVALID_UPDATE_ADDR;
		goto END;
	}

	/* Check PDI Version */
	PdiVersion = XPlmi_In32(Offset + XIH_IHT_VERSION_OFFSET);
	if (PdiVersion < XLOADER_VERSAL_2VE_2VM_PDI_VERSION_1) {
		Status = XPLM_ERR_UPDATE_PDI_UNSUPPORTED_VER;
		goto END;
	}
	/* Check ID string for FPDI */
	if (XPlmi_In32(Offset + XIH_IHT_IDENT_STRING_OFFSET) != XIH_IHT_FPDI_IDENT_VAL) {
		Status = XPLM_ERR_UPDATE_INVALID_IDENT_STRING;
		goto END;
	}
	/* Check IdCode */
	if(XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) {
		Status = XLoader_IdCodeCheck((XilPdi_ImgHdrTbl *)(UINTPTR)Offset);
		if (Status != XST_SUCCESS) {
			Status = XPLM_ERR_UPDATE_ID_CODE_CHECK;
			goto END;
		}
	}
	/* Check if Optional data length is non-zero */
	OptionalDataLen = XPlmi_In32(Offset + XIH_OPTIONAL_DATA_LEN_OFFSET);
	if (OptionalDataLen == 0U) {
		Status = XPLMI_ERR_INPLACE_INVALID_OPTIONAL_DATA_LEN;
		goto END;
	}

	Offset += XIH_IHT_LEN;
	OptionalDataEndAddr = Offset + (OptionalDataLen << XPLMI_WORD_LEN_SHIFT);
	/* Check if optional data is within lower DDR */
	if ((Offset > OptionalDataEndAddr) || (OptionalDataEndAddr > XPLMI_2GB_END_ADDR)) {
		Status = XPLM_ERR_INVALID_UPDATE_ADDR;
		goto END;
	}

	/* Search for the data structure information ID in Optional Data */
	while (Offset < OptionalDataEndAddr) {
		if ((XPlmi_In32(Offset) & XIH_OPT_DATA_HDR_ID_MASK) !=
				XIH_OPT_DATA_STRUCT_INFO_ID) {
			/*
			 * Guard against a non-advancing search loop.
			 */
			OptDataIncr = ((XPlmi_In32(Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_LEN_SHIFT) << XPLMI_WORD_LEN_SHIFT;
			if (OptDataIncr == 0U) {
				Offset = OptionalDataEndAddr;
				break;
			}
			Offset += OptDataIncr;
		}
		else {
			break;
		}
	}
	if (Offset >= OptionalDataEndAddr) {
		Status = XPLM_ERR_NO_STRUCT_OPTIONAL_DATA;
		goto END;
	}

	OptionalDataLen = (XPlmi_In32(Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_LEN_SHIFT;

	if (OptionalDataLen > 1U) {
		/* Verify checksum of data structure info */
		XSECURE_REDUNDANT_CALL(Status, StatusTmp, XilPdi_ValidateChecksum, (void *)Offset,
			(OptionalDataLen <<XPLMI_WORD_LEN_SHIFT));
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XPLM_ERR_DS_INFO_CHECKSUM_FAILED;
			goto END;
		}
	}
	else {
		Status = XPLM_ERR_NO_STRUCT_OPTIONAL_DATA;
		goto END;
	}

	/* Check data structures compatibility */
	DsVerList = (XPlmi_DsVer *)(Offset + XPLMI_WORD_LEN);
	for (Index = 0U; Index < OptionalDataLen - 2U; Index++) {
		DsEntry = XPlmi_GetDsEntry(__data_struct_start, XPLMI_DS_CNT,
				&DsVerList[Index]);
		if (DsEntry != NULL) {
			if ((DsEntry->DsHdr.Ver.Version < DsVerList[Index].LowestCompVer) ||
				(DsEntry->DsHdr.Ver.Version > DsVerList[Index].Version)) {
				XPlmi_Printf(DEBUG_GENERAL, "Incompatible data "
				"structure - ModuleId: 0x%x, DsId : 0x%x\n\r",
				DsVerList[Index].ModuleId, DsVerList[Index].DsId);
				Status = XPLMI_ERR_PLM_UPDATE_COMPATIBILITY;
				break;
			}
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs post update activities like restoring
 *		KEK Src, KAT Status, Clearing Aes Key, regenerating DevAk, etc.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlm_PostPlmUpdate(void)
{
	int Status = XST_FAILURE;
	XilBootPdiInfo *BootPdiInfo = XLoader_GetBootPdiInfo();
	u32 PlmUpdateIpiMask = XPlmi_GetPlmUpdateIpiMask();
	#ifndef PLM_SECURE_EXCLUDE
	volatile u32 DecKeySrcTmp;
	#endif
#ifdef PLM_TPM
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	if (LpdInitializedPtr == NULL) {
		goto END;
	}
#endif

	/** - Update KEK red key availability status if PLM is encrypted with Black key */
#ifndef PLM_SECURE_EXCLUDE
	XSECURE_REDUNDANT_CALL(BootPdiInfo->DecKeySrc, DecKeySrcTmp, XLoader_GetKekSrc);
	if (BootPdiInfo->DecKeySrc != DecKeySrcTmp) {
		Status = XST_GLITCH_ERROR;
		goto END;
	}
	XPlmi_GetBootKatStatus((volatile u32*)&BootPdiInfo->PlmKatStatus);

	/** - Update DecKeySrc after clearing RED keys when RedKeyClear is set in PMCRAM */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_ClearAesKey, &BootPdiInfo->DecKeySrc);

#endif
	/* Regenerate DEVAK keys of the sub-systems */
#ifdef PLM_OCP
#ifdef PLM_OCP_ASUFW_KEY_MGMT
	Status = XOcp_PostPlmUpdateNotify();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	/* Restore secure state configuration and extend SWPCR */
	Status = XOcp_MeasureSecureStateAndExtendSwPcr();
#else
	Status = XST_SUCCESS;
#endif

#ifdef PLM_TPM
	if ((*LpdInitializedPtr & LPD_INITIALIZED) != LPD_INITIALIZED) {
		XPlmi_Printf(DEBUG_GENERAL, "%s: Warning: PCR extension failed for "
					"PLM image due to uninitialized LPD\r\n", __func__);
		Status = XST_SUCCESS;
	}
	else {
		/* TPM module is applicable for Versal and Versal_2VE_2VM only */
		Status = (int)XTpm_ModuleInit();
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/**
		 * - Sends PLM digest stored in PMC_GLOBAL_FW_AUTH_HASH_0 register to TPM
		 */
		Status = XTpm_MeasurePlm();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_MEASURE_PLM_FAILURE, Status);
			goto END;
		}
	}
#endif
	/*
	 * Set FW_IS_PRESENT bit indicating new PLM is ready.
	 * Note that any error before this point in new PLM will be treated as
	 * a fatal error and IPOR is performed.
	 */
	XPlmi_SetBootPdiDone();

	XPlmi_Printf(DEBUG_GENERAL, "In-Place PLM Update Done\n\r");

	/** Restart keep-alive monitor after in-place PLM update if ASUFW is present. */
	if ((u8)TRUE == XPlmi_AsuFwIsPresent()) {
		XPlmi_StartAsufwKeepAliveMonitor();
	}
	XPlmi_Out32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_RESP_BUFF, XPLMI_INVALID_RESP_BUFF_ADDR);
	XPlmi_Out32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_MASK, XPLMI_INVALID_IPI_MASK);

	/* Send Response to IPI request indicating PLM Update is Done */
	if (PlmUpdateIpiMask != 0U) {
#ifdef XPLMI_IPI_DEVICE_ID
		/* Fill IPI response */
		(void)XPlmi_IpiWrite(PlmUpdateIpiMask, (u32 *)&Status,
			XPLMI_IPU_RESPONSE_CNT, XIPIPSU_BUF_TYPE_RESP);
		microblaze_disable_interrupts();
		/* Ack the IPI */
		XPlmi_Out32(IPI_PMC_ISR, PlmUpdateIpiMask);
		/* Enable back the IPI used for Update */
		XPlmi_IpiEnable(PlmUpdateIpiMask);
		/* Clear the IPI mask of requestor */
		XPlmi_SetPlmUpdateIpiMask(0U);
		microblaze_enable_interrupts();
#endif
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads PUF attributes from BootHeader and performs
 *              PUF on demand regeneration.
 *
 * @param	StatusFlag	Pointer to PUF regeneration status flag
 *
 * @return
 * 		XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_PufOnDemandRegeneration(u8* StatusFlag)
{
	volatile s32 Status = XST_FAILURE;
	XPuf_Data *PufData = (XPuf_Data *)(UINTPTR)XPLMI_PMCRAM_CHUNK_MEMORY_1;

	Status = XPlmi_MemSetBytes(PufData, sizeof(XPuf_Data), 0U, sizeof(XPuf_Data));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufData->ReadOption = (((XPlmi_In32(XIH_BH_PRAM_ADDR + XPLMI_BH_IMAGE_ATTRIBUTE_OFFSET) >>
		XPLMI_BH_PUF_HD_MASK_SHIFT) & XPLMI_BH_PUF_HD_MASK_BITS));
	PufData->PufOperation = XPUF_REGEN_ON_DEMAND;
	PufData->ShutterValue = XPlmi_In32(XIH_BH_PRAM_ADDR + XPLMI_BH_PUF_SHUTTER_VALUE_OFFSET) &
				~XPLMI_BIT(XPLMI_BH_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT);
	PufData->GlobalVarFilter = (u8)(XPlmi_In32(XIH_BH_PRAM_ADDR + XPLMI_BH_PUF_SHUTTER_VALUE_OFFSET) >>
					XPLMI_BH_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT);

	if (PufData->ReadOption == XLOADER_PUF_HD_BHDR) {
		PufData->ReadOption = XPUF_READ_FROM_RAM;
		PufData->SyndromeAddr = XIH_BH_PRAM_ADDR + XIH_BH_PUF_HD_OFFSET;
		PufData->Chash = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_CHASH_OFFSET);
		PufData->Aux = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_AUX_OFFSET);
		XPlmi_Printf(DEBUG_INFO, "BHDR PUF HELPER DATA with CHASH:"
			"%0x and AUX:%0x\n\r", PufData->Chash, PufData->Aux);
	}
	else {
		/**
		 * Skip PUF regeneration, if CHASH in eFuse cache is zero.
		 */
		if (XPlmi_In32(XPLMI_EFUSE_CACHE_ADDRESS +
				XPLMI_EFUSE_CACHE_PUF_CHASH_OFFSET) == 0U) {
			*StatusFlag = XST_FAILURE;
			Status = XST_SUCCESS;
			goto END;
		}
		XPlmi_Printf(DEBUG_INFO, "EFUSE PUF HELPER DATA\n\r");
		PufData->ReadOption = XPUF_READ_FROM_EFUSE_CACHE;
	}

	Status = XST_FAILURE;
	Status = XPuf_Regeneration(PufData);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Failed at PUF regeneration with status "
			"%0x\n\r", Status);
		XPLMI_STATUS_GLITCH_DETECT(Status);
		*StatusFlag = XST_FAILURE;
	} else {
		*StatusFlag = XST_SUCCESS;
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function checks whether the given PDI can be used for an in-place ASU update.
 * 		It validates the PDI address range, confirms an ASUFW image is present and verifies
 * 		the ASU data structure information optional data. The data structure version records
 * 		themselves are not compared here, as the versions supported by the running ASU
 * 		firmware are known only to it and are checked while it restores its data structures.
 *
 * @param	PdiAddr is the PDI address containing ASUFW.
 *
 * @return
 *		- XST_SUCCESS, if the PDI passes the ASU compatibility checks.
 *		- XPLM_ERR_ASU_UPDATE_INVALID_ADDR, if PDI addresses are invalid.
 *		- Error code from XPlmi_MemSetBytes, if the PDI instance could not be cleared.
 *		- Error code from XLoader_PdiInit, if the meta header of the PDI is not valid.
 *		- XLOADER_ERR_ASU_FW_NOT_FOUND, if ASUFW image is not found in the PDI.
 *		- XPLM_ERR_ASU_UPDATE_INVALID_OPT_DATA_LEN, if the optional data length is zero or
 *		  larger than the optional data copy in PMC RAM.
 *		- XPLM_ERR_ASU_UPDATE_NO_STRUCT_DATA, if ASU data structure information is missing.
 *		- XPLM_ERR_ASU_UPDATE_INVALID_DS_LEN, if the data structure information length is
 *		  too short or reaches beyond the optional data.
 *		- XPLM_ERR_ASU_UPDATE_DS_CHECKSUM_FAILED, if data structure information checksum
 *		  fails.
 *
 * @note	The checksum, the device identification code and, when enabled, the authentication
 *		of the image header table are verified by the meta header read below, so they are
 *		not repeated here. The identification string and the version of the table are not
 *		checked either, because an ASU update PDI may be built as a partial PDI, which
 *		carries neither the full PDI identification string nor the data structure format
 *		the in-place PLM update relies on.
 *		The checks run while the update command is handled, whereas the image is loaded
 *		after ASU has shut down and the loader reads the PDI from DDR again, so the
 *		verdict covers the PDI as it is seen here. The loader authenticates the image it
 *		loads.
 *
 **************************************************************************************************/
int XPlm_AsuCompatibilityCheck(u32 PdiAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 Offset;
	u32 OptionalDataLen;
	u32 OptionalDataEndAddr;
	u32 DsInfoLen;
	u32 PdiId;
	XilPdi *PdiPtr = XLoader_GetPdiInstance();

	/** - Save the PDI ID so that a rejected PDI is not left published in run time config. */
	PdiId = XPlmi_In32(XPLMI_RTCFG_PDI_ID_ADDR);

	/** - Check if the given PDI address and its image header table are within lower DDR. */
	if ((PdiAddr == 0U) || (PdiAddr > XPLMI_2GB_END_ADDR) ||
		((PdiAddr + SMAP_BUS_WIDTH_LENGTH + XIH_IHT_LEN) > XPLMI_2GB_END_ADDR)) {
		Status = (int)XPLM_ERR_ASU_UPDATE_INVALID_ADDR;
		goto END;
	}

	/** - Clear the PDI instance before it is reused for the update PDI. */
	Status = XPlmi_MemSetBytes(PdiPtr, sizeof(XilPdi), 0U, sizeof(XilPdi));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Read the meta header of the update PDI. The loader copies the image header table
	 * optional data into PMC RAM.
	 */
	Status = XST_FAILURE;
	PdiPtr->PdiType = XLOADER_PDI_TYPE_IAU;
	Status = XLoader_PdiInit(PdiPtr, XLOADER_PDI_SRC_DDR, (u64)PdiAddr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL,
			"ASU compatibility check: PDI initialization failed: 0x%x\n\r", Status);
		goto END;
	}

	/** - Confirm ASUFW image is present in the PDI. */
	Status = XST_FAILURE;
	Status = XLoader_GetImageAndPrtnInfo(PdiPtr, XPLMI_ASUFW_IMAGE_ID);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL,
			"ASU compatibility check: ASUFW image not found in PDI\n\r");
		Status = (int)XLOADER_ERR_ASU_FW_NOT_FOUND;
		goto END;
	}

	/**
	 * - Check the optional data length. It has to be non-zero and to fit in the copy the
	 * loader places in PMC RAM, which is what the search below reads.
	 */
	OptionalDataLen = PdiPtr->MetaHdr->ImgHdrTbl.OptionalDataLen;
	if ((OptionalDataLen == 0U) || (OptionalDataLen >
		(XILPDI_OPTIONAL_DATA_MAX_SIZE_16K_BYTES >> XPLMI_WORD_LEN_SHIFT))) {
		XPlmi_Printf(DEBUG_GENERAL,
			"ASU compatibility check: invalid optional data length in PDI\n\r");
		Status = (int)XPLM_ERR_ASU_UPDATE_INVALID_OPT_DATA_LEN;
		goto END;
	}

	/**
	 * - Search the optional data copy the loader placed in PMC RAM for the ASU data structure
	 * information.
	 */
	Offset = XILPDI_PMCRAM_IHT_DATA_ADDR;
	OptionalDataEndAddr = Offset + (OptionalDataLen << XPLMI_WORD_LEN_SHIFT);
	Offset = (u32)XilPdi_SearchOptionalData(Offset, OptionalDataEndAddr,
			XIH_OPT_DATA_ASU_STRUCT_INFO_ID);
	if (Offset >= OptionalDataEndAddr) {
		XPlmi_Printf(DEBUG_GENERAL,
			"ASU compatibility check: data structure information not found\n\r");
		Status = (int)XPLM_ERR_ASU_UPDATE_NO_STRUCT_DATA;
		goto END;
	}

	/**
	 * - The block holds one header word, at least one data structure version record and one
	 * checksum word, and it has to end within the optional data.
	 */
	DsInfoLen = (XPlmi_In32(Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >> XIH_OPT_DATA_LEN_SHIFT;
	if ((DsInfoLen < XPLM_ASU_DS_INFO_MIN_LEN_IN_WORDS) ||
		((DsInfoLen << XPLMI_WORD_LEN_SHIFT) > (OptionalDataEndAddr - Offset))) {
		XPlmi_Printf(DEBUG_GENERAL,
			"ASU compatibility check: invalid data structure information length\n\r");
		Status = (int)XPLM_ERR_ASU_UPDATE_INVALID_DS_LEN;
		goto END;
	}

	/** - Verify checksum of the ASU data structure information. */
	Status = XST_FAILURE;
	StatusTmp = XST_FAILURE;
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, XilPdi_ValidateChecksum, (void *)(UINTPTR)Offset,
		(DsInfoLen << XPLMI_WORD_LEN_SHIFT));
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = (int)XPLM_ERR_ASU_UPDATE_DS_CHECKSUM_FAILED;
	}

END:
	if (Status != XST_SUCCESS) {
		/**
		 * - Restore the PDI ID published in run time configuration and clear the PDI
		 * instance, which is shared with the image load paths. The image header table
		 * and optional data of the rejected PDI stay in the PMC RAM area reserved for
		 * them, which the next meta header read clears before it is used.
		 */
		XPlmi_Out32(XPLMI_RTCFG_PDI_ID_ADDR, PdiId);
		ClearStatus = XPlmi_MemSetBytes(PdiPtr, sizeof(XilPdi), 0U, sizeof(XilPdi));
		if (ClearStatus != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL,
				"ASU compatibility check: PDI instance clear failed: 0x%x\n\r",
				ClearStatus);
		}
	}

	return Status;
}
/** @endcond */
/** @} end of xplm_apis */
