/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_asufw.c
* @addtogroup xilocp_asufw_apis XilOcp ASUFW APIs
* @{
*
* This file contains the implementation of the functionalities required for the ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*       rmv  02/18/26 Increase value of maximum OCP subsystems
*
* </pre>
*
**************************************************************************************************/

/************************************** Include Files ********************************************/
#include "xplmi_config.h"
#ifdef PLM_OCP_ASUFW_KEY_MGMT
#include "xocp_generic.h"
#include "xocp_common.h"
#include "xocp_hw.h"
#include "xocp_keymgmt_common.h"
#include "xplmi_debug.h"
#include "xocp_asufw.h"
#include "xplmi_ipi.h"
#include "xplmi_modules.h"
#include "xplmi_scheduler.h"
#include "xplmi_err_common.h"
#include "xplmi_update.h"

/********************************** Constant Definitions *****************************************/
#define XOCP_SUBSYS_INFO_VERSION			(1U) /**< Subsystem info version */
#define XOCP_SUBSYS_INFO_LCVERSION			(1U) /**< Subsystem info lowest compatible
								version */
#define XOCP_ACTIVE_SUBSYS_MASK_VERSION			(1U) /**< Active subsystem mask version */
#define XOCP_ACTIVE_SUBSYS_MASK_LCVERSION		(1U) /**< Active subsystem mask lowest
								compatible version */

/************************************ Type Definitions *******************************************/
/*
 * Structure to store OCP subsystem's ID and its hash.
 */
typedef struct {
	u32 SubsystemId;				/**< Subsystem ID */
	u8 SubsystemHash[XOCP_SHA3_LEN_IN_BYTES];	/**< Subsystem hash */
} XOcp_SubsysInfo;


/**************************** Macros (Inline Functions) Definitions ******************************/
/** @cond xocp_internal
 * @{
 */
#define XOCP_MAX_USER_OCP_SUBSYS		(6U)	/**< Maximum number of user OCP subsystem */
#define XOCP_TOTAL_OCP_SUBSYS			(XOCP_MAX_USER_OCP_SUBSYS + 1U)
				/**< Total number of OCP subsystems (including ASU subsystem) */

#define XOCP_ASU_SUBSYSTEM_ID			(0x1C000002U)	/**< ASU subsystem ID */
#define XOCP_PMC_SUBSYSTEM_ID			(0x1C000001U)	/**< PMC subsystem ID */
#define XOCP_INVALID_SUBSYSTEM_ID		(0x00000000U)	/**< Invalid subsystem ID */

#define XOCP_PLM_ASUFW_EVENT_MASK		(0x00000001U)	/**< PLM to ASU event mask */
/** @}
 * @endcond
 */

/************************************ Function Prototypes ****************************************/
static int XOcp_GetSubsysDigestAddr(u32 SubsystemId, u32 *SubsysHashAddrPtr);
static int XOcp_GenerateAsuCdiSeed(void);
static int XOcp_GenerateCdiAndNotify(u32 SubsystemId);
static XOcp_SubsysInfo *XOcp_GetOcpSubsysInfoDb(void);

/********************************** Variable Definitions *****************************************/
static u8 XOcpAsuCdi[XOCP_CDI_SIZE_IN_BYTES];
static u8 IsAsuCdiValid;
static u32 OcpEventMask;

/********************************** Function Definitions *****************************************/
/*************************************************************************************************/
/**
 * @brief	This function provides a pointer to the OCP active subsystem mask.
 *
 * @return
 *	- Pointer to the OCP active subsystem mask.
 *
 *************************************************************************************************/
static u32 *XOcp_OcpActiveSubsysMask(void)
{
	static u32 OcpActiveSubsysMask __attribute__ ((aligned(4U))) = 0U;

	EXPORT_OCP_DS(OcpActiveSubsysMask, XOCP_ACTIVE_SUBSYS_MASK_DS_ID,
		      XOCP_ACTIVE_SUBSYS_MASK_VERSION, XOCP_ACTIVE_SUBSYS_MASK_LCVERSION,
		      sizeof(OcpActiveSubsysMask), (u32)(UINTPTR)&OcpActiveSubsysMask);


	return &OcpActiveSubsysMask;
}

/*************************************************************************************************/
/**
 * @brief	This function stores the subsystem IDs which requires OCP support. Based on the
 *		user configuration, list of subsystems requiring OCP support are populated in CDO
 *		in OCP_SUBSYS_INPUT command. This API is called as part of the OCP_SUBSYS_INPUT
 *		command.
 *
 * @param	SubsystemIdListLen	Length of the subsystem ID list.
 * @param	SubsystemIdList		Pointer to the list of subsystem IDs.
 *
 * @return
 *	- XST_SUCCESS, if subsystem ID is stored successfully.
 *	- XST_FAILURE, if the list length exceeds the maximum supported subsystems.
 *
 *************************************************************************************************/
int XOcp_StoreOcpSubsysIDs(u32 SubsystemIdListLen, const u32 *SubsystemIdList)
{
	volatile int Status = XST_FAILURE;
	XOcp_SubsysInfo *OcpSubsysInfo = XOcp_GetOcpSubsysInfoDb();
	u32 Idx;

	/** Validate input parameters. */
	if ((SubsystemIdListLen > XOCP_TOTAL_OCP_SUBSYS) || (SubsystemIdList == NULL)) {
		goto END;
	}

	/** Store subsystem IDs. */
	for (Idx = 0; Idx < SubsystemIdListLen; Idx++) {
		OcpSubsysInfo[Idx].SubsystemId = SubsystemIdList[Idx];
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function stores the subsystem digest to the OCP subsystem information database.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	Hash		Address of the hash to be stored.
 *
 * @return
 *	- XST_SUCCESS, if subsystem digest is stored successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int XOcp_StoreSubsysDigest(u32 SubsystemId, u64 Hash)
{
	volatile int Status = XST_FAILURE;
	XOcp_SubsysInfo *OcpSubsysInfo = XOcp_GetOcpSubsysInfoDb();
	u32 *OcpActiveSubsysMask = XOcp_OcpActiveSubsysMask();
	u32 Idx;

	/** Validate input parameters. */
	if (SubsystemId == XOCP_INVALID_SUBSYSTEM_ID) {
		goto END;
	}

	/** Copy hash if OCP support is required for the subsystem, else ignore it. */
	for (Idx = 0; Idx < XOCP_TOTAL_OCP_SUBSYS; Idx++) {
		if (OcpSubsysInfo[Idx].SubsystemId == SubsystemId) {
			Status = XPlmi_MemCpy64((u64)(UINTPTR)OcpSubsysInfo[Idx].SubsystemHash,
						Hash, XOCP_SHA3_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			OcpEventMask |= XPLMI_BIT(Idx);
			*OcpActiveSubsysMask |= XPLMI_BIT(Idx);
			break;
		}
	}

	/** Process subsystem and notify ASUFW. */
	Status = XOcp_GenerateCdiAndNotify(SubsystemId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if given subsystem ID is in OCP subsystems list or not.
 *
 * @param	SubsystemId	Subsystem ID.
 *
 * @return
 *	- TRUE, if subsystem is part of OCP subsystems list.
 *	- FALSE, if subsystem is not part of OCP subsystems list.
 *
 *************************************************************************************************/
u8 XOcp_IsOcpSubsystem(u32 SubsystemId)
{
	XOcp_SubsysInfo *OcpSubsysInfo = XOcp_GetOcpSubsysInfoDb();
	u32 Idx;
	u8 IsOcpSubsystem = FALSE;

	/** Validate input parameters. */
	if (SubsystemId == XOCP_INVALID_SUBSYSTEM_ID) {
		goto END;
	}

	/** Check if given subsystem ID is in OCP subsystems list or not. */
	for (Idx = 0U; Idx < XOCP_TOTAL_OCP_SUBSYS; Idx++) {
		if (OcpSubsysInfo[Idx].SubsystemId == SubsystemId) {
			IsOcpSubsystem = TRUE;
			break;
		}
	}

END:
	return IsOcpSubsystem;
}

/*************************************************************************************************/
/**
 * @brief	This function handles post-PLM update operations for OCP subsystem.
 *		It generates CDI seed and sends event notification to ASUFW for PMC subsystem.
 *
 * @return
 *	- XST_SUCCESS, if post-PLM update operations are completed successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int XOcp_PostPlmUpdateNotify(void)
{
	return XOcp_GenerateCdiAndNotify(XOCP_PMC_SUBSYSTEM_ID);
}

/*************************************************************************************************/
/**
 * @brief	This function provides the address of the stored subsystem digest from the OCP
 *		subsystem information database.
 *
 * @param	SubsystemId		Subsystem ID for which the digest is requested.
 * @param	SubsysHashAddrPtr	Pointer to a variable where the address of the subsystem
 *					hash is to be stored.
 *
 * @return
 *	- XST_SUCCESS, if the subsystem digest is retrieved successfully.
 *	- XST_FAILURE, if the subsystem is not found.
 *
 *************************************************************************************************/
static int XOcp_GetSubsysDigestAddr(u32 SubsystemId, u32 *SubsysHashAddrPtr)
{
	volatile int Status = XST_FAILURE;
	const XOcp_SubsysInfo *OcpSubsysInfo = XOcp_GetOcpSubsysInfoDb();
	u32 Idx;

	/** Get the address of subsystem digest using subsystem ID. */
	for (Idx = 0; Idx < XOCP_TOTAL_OCP_SUBSYS; Idx++) {
		if (OcpSubsysInfo[Idx].SubsystemId == SubsystemId) {
			*SubsysHashAddrPtr = (u32)(UINTPTR)(OcpSubsysInfo[Idx].SubsystemHash);
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function copies stored subsystem digest from the OCP subsystem information
 *		database based on subsystem ID.
 *
 * @param	SubsystemId		Subsystem ID for which the digest is requested.
 * @param	SubsysHashAddrPtr	Address to store the subsystem hash.
 *
 * @return
 *	- XST_SUCCESS, if the subsystem digest is retrieved successfully.
 *	- XST_FAILURE, if the subsystem is not found.
 *
 *************************************************************************************************/
int XOcp_GetSubsysDigest(u32 SubsystemId, u32 SubsysHashAddrPtr)
{
	volatile int Status = XST_FAILURE;
	u32 SubsystemHash = 0U;

	/** Get the address of the subsystem digest for the required subsystem. */
	Status = XOcp_GetSubsysDigestAddr(SubsystemId, &SubsystemHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Copy subsystem digest to provided address. */
	Status = Xil_SMemCpy((void *)(UINTPTR)SubsysHashAddrPtr, XOCP_SHA3_LEN_IN_BYTES,
			     (const void *)(UINTPTR)SubsystemHash,
			     XOCP_SHA3_LEN_IN_BYTES,
			     XOCP_SHA3_LEN_IN_BYTES);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides an ASU event mask.
 *
 * @param	EventMask	ASU event mask.
 *
 *************************************************************************************************/
void XOcp_GetOcpEventMask(u32 *EventMask)
{
	*EventMask = OcpEventMask;
	OcpEventMask = 0U;
}

/*************************************************************************************************/
/**
 * @brief	This function generates CDI seed and sends event notification to ASUFW.
 *		For ASU subsystem, it generates CDI seed and copies device DNA.
 *		For all subsystems, it sends event notification if ASUFW is ready.
 *
 * @param	SubsystemId	Subsystem ID.
 *
 * @return
 *	- XST_SUCCESS, if CDI generation and notification is completed successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static int XOcp_GenerateCdiAndNotify(u32 SubsystemId)
{
	volatile int Status = XST_FAILURE;
	u8 ActionId = 0U;
	const u32 *OcpActiveSubsysMask = XOcp_OcpActiveSubsysMask();

	/** For ASU subsystem, generate ASU CDI seed and copy device DNA. */
	if ((SubsystemId == XOCP_ASU_SUBSYSTEM_ID) || (SubsystemId == XOCP_PMC_SUBSYSTEM_ID)) {
		if (((OcpEventMask & XOCP_PLM_ASUFW_EVENT_MASK) == XOCP_PLM_ASUFW_EVENT_MASK) ||
		    (SubsystemId == XOCP_PMC_SUBSYSTEM_ID)) {
			Status = XOcp_GenerateAsuCdiSeed();
			if (XST_SUCCESS != Status) {
				goto END;
			}
		}
		OcpEventMask = *OcpActiveSubsysMask;
	}

	/** Send event notification if error action is CUSTOM. */
	Status = XPlmi_EmGetAction(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
				   XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE, &ActionId);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (ActionId == XPLMI_EM_ACTION_CUSTOM) {
		XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
				    XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates CDI seed for ASUFW.
 *
 * @return
 *	- XST_SUCCESS, if ASU CDI seed is generated successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static int XOcp_GenerateAsuCdiSeed(void)
{
	volatile int Status = XST_FAILURE;
	u32 AsuHashAddr = 0U;
	u8 Seed[XOCP_CDI_SIZE_IN_BYTES];
	u32 PlmCdiAddr = (u32)(UINTPTR)&Seed[0];

	/** Mark ASU CDI as invalid. */
	IsAsuCdiValid = FALSE;

	/** If CDI is not valid device key generation is skipped */
	if (XPlmi_In32(XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID) == 0x0U) {
		XOcp_Printf(DEBUG_GENERAL, "Valid CDI not found, OCP key management functionalities"
					   " will be affected\n\r");
		Status = XST_SUCCESS;
		goto END;
	}

	/** Read and validate whether, DICE CDI SEED is valid or not */
	Status = XOcp_ValidateDiceCdi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Copy CDI from PMC global registers to Seed buffer */
	Status = Xil_SMemCpy((void *)(UINTPTR)Seed, XOCP_CDI_SIZE_IN_BYTES,
			     (const void *)(UINTPTR)XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
			     XOCP_CDI_SIZE_IN_BYTES, XOCP_CDI_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Get the subsystem digest for the ASUFW. */
	Status = XOcp_GetSubsysDigestAddr(XOCP_ASU_SUBSYSTEM_ID, &AsuHashAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate ASUFW CDI. */
	Status = XOcp_KeyGenDevAkSeed(PlmCdiAddr, XOCP_CDI_SIZE_IN_BYTES, (u32)(UINTPTR)AsuHashAddr,
				      XOCP_CDI_SIZE_IN_BYTES,
				      (XSecure_HmacRes *)(UINTPTR)XOcpAsuCdi);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Mark ASU CDI as valid. */
	IsAsuCdiValid = TRUE;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function copies the ASU CDI to provided address.
 *
 * @param	CdiAddr		Address to store ASU CDI.
 *
 * @return
 *	- XST_SUCCESS, if ASU CDI is copied successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int XOcp_GetAsuCdiSeed(u32 CdiAddr)
{
	volatile int Status = XST_FAILURE;

	if (IsAsuCdiValid == TRUE) {
		Status = Xil_SMemCpy((void *)(UINTPTR)CdiAddr, XOCP_CDI_SIZE_IN_BYTES,
				     (const void *)(UINTPTR)XOcpAsuCdi, XOCP_CDI_SIZE_IN_BYTES,
				     XOCP_CDI_SIZE_IN_BYTES);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the pointer to the OCP subsystem information database.
 *
 * @return
 *	- Pointer to the XOcp_SubsysInfo database.
 *
 *************************************************************************************************/
static XOcp_SubsysInfo *XOcp_GetOcpSubsysInfoDb(void)
{
	static XOcp_SubsysInfo OcpSubsysInfo[XOCP_TOTAL_OCP_SUBSYS] __attribute__ ((aligned(4U))) = {0U};

	EXPORT_OCP_DS(OcpSubsysInfo, XOCP_OCP_SUBSYSTEM_INFO_DS_ID,
		      XOCP_SUBSYS_INFO_VERSION, XOCP_SUBSYS_INFO_LCVERSION,
		      sizeof(OcpSubsysInfo), (u32)(UINTPTR)&OcpSubsysInfo[0]);

	return &OcpSubsysInfo[0];
}

#endif /* PLM_OCP_ASUFW_KEY_MGMT */
/** @} */
