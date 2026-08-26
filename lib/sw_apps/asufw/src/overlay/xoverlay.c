/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xoverlay.c
*
* This file contains the implementation of the generic on-demand overlay manager for ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   rmv  06/11/26 Initial release
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xoverlay_server_apis Overlay Manager APIs
* @{
*/
/************************************** Include Files ********************************************/
#include "xoverlay.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_debug.h"
#include "xasufw_dma.h"
#include "xecc.h"
#include "xecc_hw.h"

/************************************ Macro Definitions ******************************************/

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/
extern char __load_start_overlay_rsa_ecc[];		/**< LMA of the RSA-ECC overlay image */
extern char __load_start_overlay_rsa_ecc_data[];	/**< LMA of the RSA-ECC overlay data image */
extern char __overlay_rsa_ecc_img_size[];		/**< Size of the RSA-ECC overlay image */
extern char __overlay_rsa_ecc_txt_size[];		/**< Size of the RSA-ECC overlay text image */
extern char __overlay_rsa_ecc_data_size[];		/**< Size of the RSA-ECC overlay data image */

#ifdef XASU_LMS_ENABLE
extern char __load_start_overlay_lms[];		/**< LMA of the LMS overlay image */
extern char __load_start_overlay_lms_data[];	/**< LMA of the LMS overlay data image */
extern char __overlay_lms_img_size[];		/**< Size of the LMS overlay image */
extern char __overlay_lms_txt_size[];		/**< Size of the LMS overlay text image */
extern char __overlay_lms_data_size[];		/**< Size of the LMS overlay data image */
#endif

/*
 * The ML-DSA and ML-KEM data images hold only the KAT vectors of those algorithms. Their constant
 * tables stay resident: the tables do get per-symbol sections, but the resident .rodata output
 * section is matched ahead of the data overlay block and its .rodata.* catch-all claims them
 * first. See the overlay blocks in lscript.ld.
 */
#ifdef XASU_MLDSA_ENABLE
extern char __load_start_overlay_mldsa[];	/**< LMA of the ML-DSA overlay image */
extern char __load_start_overlay_mldsa_data[];	/**< LMA of the ML-DSA overlay data image */
extern char __overlay_mldsa_img_size[];		/**< Size of the ML-DSA overlay image */
extern char __overlay_mldsa_txt_size[];		/**< Size of the ML-DSA overlay text image */
extern char __overlay_mldsa_data_size[];	/**< Size of the ML-DSA overlay data image */
#endif

#ifdef XASU_MLKEM_ENABLE
extern char __load_start_overlay_mlkem[];	/**< LMA of the ML-KEM overlay image */
extern char __load_start_overlay_mlkem_data[];	/**< LMA of the ML-KEM overlay data image */
extern char __overlay_mlkem_img_size[];		/**< Size of the ML-KEM overlay image */
extern char __overlay_mlkem_txt_size[];		/**< Size of the ML-KEM overlay text image */
extern char __overlay_mlkem_data_size[];	/**< Size of the ML-KEM overlay data image */
#endif

extern char __overlay_window_start[];		/**< Start address of the IRAM overlay window */
extern char __overlay_data_window_start[];	/**< Start address of the data overlay window */

extern char __overlay_window_size[];		/**< Size of the IRAM overlay window */
extern char __overlay_data_window_size[];	/**< Size of the data overlay window */

static u32 ActiveAlgoId = XOVERLAY_ALGO_INVALID;	/**< Currently loaded overlay algorithm */

/************************************ Function Prototypes ****************************************/
static s32 XOverlay_RsaEccOnLoad(void);

/********************************** Constant Definitions *****************************************/
/** Overlay image catalog, one entry per supported algorithm. */
static const XOverlay_CatalogEntry Catalog[] = {
	{
		.AlgoId = XOVERLAY_ALGO_RSA_ECC,
		.ImgLma = (const void *)(UINTPTR)__load_start_overlay_rsa_ecc,
		.DataLma = (const void *)(UINTPTR)__load_start_overlay_rsa_ecc_data,
		.ImgSize = (u32)(UINTPTR)__overlay_rsa_ecc_img_size,
		.TxtSize = (u32)(UINTPTR)__overlay_rsa_ecc_txt_size,
		.DataSize = (u32)(UINTPTR)__overlay_rsa_ecc_data_size,
		.OnLoad = XOverlay_RsaEccOnLoad,
	},
#ifdef XASU_LMS_ENABLE
	{
		.AlgoId = XOVERLAY_ALGO_LMS,
		.ImgLma = (const void *)(UINTPTR)__load_start_overlay_lms,
		.DataLma = (const void *)(UINTPTR)__load_start_overlay_lms_data,
		.ImgSize = (u32)(UINTPTR)__overlay_lms_img_size,
		.TxtSize = (u32)(UINTPTR)__overlay_lms_txt_size,
		.DataSize = (u32)(UINTPTR)__overlay_lms_data_size,
		.OnLoad = NULL,
	},
#endif
#ifdef XASU_MLDSA_ENABLE
	{
		.AlgoId = XOVERLAY_ALGO_MLDSA,
		.ImgLma = (const void *)(UINTPTR)__load_start_overlay_mldsa,
		.DataLma = (const void *)(UINTPTR)__load_start_overlay_mldsa_data,
		.ImgSize = (u32)(UINTPTR)__overlay_mldsa_img_size,
		.TxtSize = (u32)(UINTPTR)__overlay_mldsa_txt_size,
		.DataSize = (u32)(UINTPTR)__overlay_mldsa_data_size,
		.OnLoad = NULL,
	},
#endif
#ifdef XASU_MLKEM_ENABLE
	{
		.AlgoId = XOVERLAY_ALGO_MLKEM,
		.ImgLma = (const void *)(UINTPTR)__load_start_overlay_mlkem,
		.DataLma = (const void *)(UINTPTR)__load_start_overlay_mlkem_data,
		.ImgSize = (u32)(UINTPTR)__overlay_mlkem_img_size,
		.TxtSize = (u32)(UINTPTR)__overlay_mlkem_txt_size,
		.DataSize = (u32)(UINTPTR)__overlay_mlkem_data_size,
		.OnLoad = NULL,
	},
#endif
};

/*************************************************************************************************/
/**
 * @brief	This function issues an instruction-stream fence after overlay code is copied into
 *		IRAM.
 *
 *************************************************************************************************/
static void XOverlay_FenceI(void)
{
	__asm__ __volatile__("fence.i" ::: "memory");
}

/*************************************************************************************************/
/**
 * @brief	This function looks up a catalog entry by algorithm ID.
 *
 * @param	AlgoId	Algorithm identifier.
 *
 * @return
 *	- Pointer to the matching catalog entry, if the algorithm is found.
 *	- NULL, if no catalog entry matches the given algorithm ID.
 *
 *************************************************************************************************/
static const XOverlay_CatalogEntry *XOverlay_LookupCatalog(u32 AlgoId)
{
	const XOverlay_CatalogEntry *Entry = NULL;
	u32 Index;

	/** Iterate through the catalog entries to find the matching algorithm ID. */
	for (Index = 0U; Index < (sizeof(Catalog) / sizeof(Catalog[0U])); Index++) {
		if (Catalog[Index].AlgoId == AlgoId) {
			Entry = &Catalog[Index];
			break;
		}
	}

	return Entry;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the overlay image header.
 *
 * @param	Hdr	Pointer to the overlay header.
 * @param	Entry	Catalog entry for the algorithm.
 *
 * @return
 *	- XASUFW_SUCCESS, if the header is valid.
 *	- XASUFW_OVERLAY_HEADER_INVALID, if the header fields do not match the catalog entry.
 *
 *************************************************************************************************/
static s32 XOverlay_ValidateHeader(const XOverlay_Header *Hdr,
				   const XOverlay_CatalogEntry *Entry)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate image header magic number. */
	if (Hdr->Magic != XOVERLAY_MAGIC) {
		Status = XASUFW_OVERLAY_HEADER_INVALID;
		goto END;
	}

	/** Check the header format version. */
	if (Hdr->Version != XOVERLAY_HEADER_VERSION) {
		Status = XASUFW_OVERLAY_HEADER_INVALID;
		goto END;
	}

	/** Check the algorithm ID matches the catalog entry. */
	if (Hdr->AlgoId != Entry->AlgoId) {
		Status = XASUFW_OVERLAY_HEADER_INVALID;
		goto END;
	}

	/** Check the image sizes are consistent and match the catalog entry. */
	if ((Hdr->ImgSize != (Hdr->TxtSize + Hdr->DataSize)) ||
	    (Hdr->ImgSize != Entry->ImgSize) || (Hdr->TxtSize != Entry->TxtSize) ||
	    (Hdr->DataSize != Entry->DataSize)) {
		Status = XASUFW_OVERLAY_HEADER_INVALID;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs an overlay switch between different algorithms.
 *
 * @param	Entry	Catalog entry describing the target algorithm image.
 * @param	DmaPtr	Pointer to the DMA instance used to copy the overlay image.
 *
 * @return
 *	- XASUFW_SUCCESS, if the overlay switch is successful.
 *	- XASUFW_OVERLAY_HEADER_INVALID, if the image header validation fails.
 *	- XASUFW_DMA_COPY_FAIL, if an image copy fails.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if a window zeroize fails.
 *
 *************************************************************************************************/
static s32 XOverlay_Switch(const XOverlay_CatalogEntry *Entry, XAsufw_Dma *DmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XOverlay_Header *LmaHdr;
	u64 IramWin = (u64)(UINTPTR)__overlay_window_start;
	u64 DataWin = (u64)(UINTPTR)__overlay_data_window_start;
	u32 IramWinSize = (u32)(UINTPTR)__overlay_window_size;
	u32 DataWinSize = (u32)(UINTPTR)__overlay_data_window_size;
	u64 ImgLma = (u64)(UINTPTR)Entry->ImgLma;

	/** Validate header at LMA before touching the overlay windows. */
	LmaHdr = (const XOverlay_Header *)(UINTPTR)ImgLma;
	Status = XOverlay_ValidateHeader(LmaHdr, Entry);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OVERLAY_HEADER_INVALID;
		goto END;
	}

	/** Zeroize the active IRAM window footprint before reload. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_SMemSet((void *)(UINTPTR)IramWin, IramWinSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy header and text into IRAM window. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, ImgLma, IramWin, Entry->TxtSize, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Fence instruction stream after IRAM copy. */
	XOverlay_FenceI();

	/** Zeroize the active data window footprint. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_SMemSet((void *)(UINTPTR)DataWin, DataWinSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	if (Entry->DataSize != 0U) {
		/** Copy the data image into the data window. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)Entry->DataLma, DataWin,
				       Entry->DataSize, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	}

	ActiveAlgoId = Entry->AlgoId;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the RSA-ECC overlay on-load hook. It re-initializes the ECC core
 *		instance after each fresh overlay load, since the instance state is reset on
 *		an overlay switch.
 *
 * @return
 *	- XASUFW_SUCCESS, if the ECC instance is re-initialized successfully.
 *	- Error code from the ECC initialize routine, otherwise.
 *
 *************************************************************************************************/
static s32 XOverlay_RsaEccOnLoad(void)
{
	CREATE_VOLATILE(Status, XASUFW_INVALID_PARAM);

	Status = XEcc_Initialize(XEcc_GetInstance(XASU_XECC_0_DEVICE_ID));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function activates an overlay algorithm.
 *
 * @param	AlgoId		Target algorithm identifier.
 * @param	DmaPtr		Pointer to the DMA instance used to copy the overlay image.
 *
 * @return
 *	- XASUFW_SUCCESS, if the overlay is activated and resident in the overlay window.
 *	- XASUFW_INVALID_PARAM, if DmaPtr is NULL.
 *	- XASUFW_OVERLAY_ALGO_NOT_FOUND, if the requested algorithm is not in the catalog.
 *	- XASUFW_OVERLAY_SWITCH_FAIL, if the overlay switch into the window fails.
 *	- XASUFW_OVERLAY_HEADER_INVALID, if the header validation fails.
 *	- XASUFW_OVERLAY_ON_LOAD_FAIL, if the on-load hook fails.
 *
 *************************************************************************************************/
s32 XOverlay_Activate(u32 AlgoId, XAsufw_Dma *DmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XOverlay_CatalogEntry *Entry;
	const XOverlay_Header *LoadedHdr;
	u64 IramWin;
	u32 OverlaySwitch = (u32)XASU_FALSE;

	/** Validate input parameters. */
	if (DmaPtr == NULL) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Lookup the catalog entry for the requested algorithm. */
	Entry = XOverlay_LookupCatalog(AlgoId);
	if (Entry == NULL) {
		Status = XASUFW_OVERLAY_ALGO_NOT_FOUND;
		goto END;
	}

	/** Perform overlay switch if the algorithm is different from the currently loaded one. */
	if (ActiveAlgoId != AlgoId) {
		Status = XOverlay_Switch(Entry, DmaPtr);
		if (Status != XASUFW_SUCCESS) {
			ActiveAlgoId = XOVERLAY_ALGO_INVALID;
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_SWITCH_FAIL);
			goto END;
		}
		OverlaySwitch = (u32)XASU_TRUE;
	}

	/** Validate the overlay header in IRAM after the switch. */
	IramWin = (u64)(UINTPTR)__overlay_window_start;
	LoadedHdr = (const XOverlay_Header *)(UINTPTR)IramWin;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOverlay_ValidateHeader(LoadedHdr, Entry);
	if (Status != XASUFW_SUCCESS) {
		ActiveAlgoId = XOVERLAY_ALGO_INVALID;
		Status = XASUFW_OVERLAY_HEADER_INVALID;
		goto END;
	}

	/**
	 * On overlay switch, reinitialize overlay state via the catalog on-load hook if present;
	 * skip if already loaded.
	 */
	if ((OverlaySwitch == (u32)XASU_TRUE) && (Entry->OnLoad != NULL)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Entry->OnLoad();
		if (Status != XASUFW_SUCCESS) {
			ActiveAlgoId = XOVERLAY_ALGO_INVALID;
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ON_LOAD_FAIL);
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */
