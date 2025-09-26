/**************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_plat.c
*
* This file contains the implementation of the functionalities required for the ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   rmv  07/17/25 Initial release
* 1.6   tvp  05/16/25 Add XOcp_GetRegSpace function
*       tvp  09/13/25 Moved XOcp_ReadSecureConfig from xocp.c to platform file
*
* </pre>
*
**************************************************************************************************/

/************************************** Include Files ********************************************/
#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_plat.h"
#include "xocp_hw.h"

#ifdef PLM_OCP_ASUFW_KEY_MGMT
#include "xplmi_scheduler.h"

/********************************** Constant Definitions *****************************************/

/************************************ Type Definitions *******************************************/
/*
 * Structure to store OCP subsystem's ID and its hash.
 */
typedef struct {
	u32 SubsystemId;				/**< Subsystem ID */
	u8 SubsystemHash[XOCP_SHA3_LEN_IN_BYTES];	/**< Subsystem hash */
} XOcp_SubsysInfo;


/**************************** Macros (Inline Functions) Definitions ******************************/
#define XOCP_ASUFW_MAX_SUBSYS_SUPPORT		(5U)	/**< Maximum number of OCP subsystems
							(including ASU subsystem) */

#define XOCP_ASU_SUBSYSTEM_ID			(0x1C000002U)	/**< ASU subsystem ID */
#define XOCP_INVALID_SUBSYSTEM_ID		(0x00000000U)	/**< Invalid subsystem ID */

#define XOCP_ASUFW_IPI_MASK			(0x00000001U)	/**< ASUFW IPI mask */
#define XOCP_PLM_ASUFW_EVENT_MASK		(0x00000001U)	/**< PLM to ASU event mask */
#define XOCP_ASUFW_EVENT_PAYLOAD_SIZE		(2U)	/**< ASUFW event payload size */
#define XOCP_ASUFW_NOTIFICATION_TASK_DELAY	(100U)	/**< Notify ASUFW task delay in
							milliseconds */
#define XOCP_ASUFW_MAX_TASK_COUNT		(3U)	/**< Maximum count to schedule task if
							ASUFW is not present */
#define XOCP_ASU_GLOBAL_BASEADDR			(0xEBF80000U)
								/** ASU_GLOBAL base address */
#define XOCP_ASU_GLOBAL_GLOBAL_CNTRL			(XOCP_ASU_GLOBAL_BASEADDR + 0x00000000U)
								/** ASU_GLOBAL GLOBAL_CNTRL register
								address */
#define XOCP_ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)
								/** ASU_GLOBAL GLOBAL_CNTRL
								FW_Is_Present mask */

/************************************ Function Prototypes ****************************************/
static int XOcp_NotifyAsuTask(void *Arg);
static int XOcp_GetSubsysDigestAddr(u32 SubsystemId, u32 *SubsysHashAddrPtr);
static int XOcp_GenerateAsuCdiSeed(void);
static XOcp_SubsysInfo *XOcp_GetOcpSubsysInfoDb(void);

/********************************** Variable Definitions *****************************************/
static u32 XOcpPlmAsufwEvent;
static u8 XOcpAsuCdi[XOCP_CDI_SIZE_IN_BYTES];

/********************************** Function Definitions *****************************************/
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

	/* Validate input parameters. */
	if ((SubsystemIdListLen > XOCP_ASUFW_MAX_SUBSYS_SUPPORT) || (SubsystemIdList == NULL)) {
		goto END;
	}

	/* Store subsystem IDs. */
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
 **************************************************************************************************/
int XOcp_StoreSubsysDigest(u32 SubsystemId, u64 Hash)
{
	volatile int Status = XST_FAILURE;
	XOcp_SubsysInfo *OcpSubsysInfo = XOcp_GetOcpSubsysInfoDb();
	u32 Idx;

	/* Return in case of invalid subsystem ID. */
	if (SubsystemId == XOCP_INVALID_SUBSYSTEM_ID) {
		goto END;
	}

	/* Copy hash if OCP support is required for the subsystem, else ignore it. */
	for (Idx = 0; Idx < XOCP_ASUFW_MAX_SUBSYS_SUPPORT; Idx++) {
		if (OcpSubsysInfo[Idx].SubsystemId == SubsystemId) {
			Status = XPlmi_MemCpy64((u64)(UINTPTR)OcpSubsysInfo[Idx].SubsystemHash,
						Hash, XOCP_SHA3_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			XOcpPlmAsufwEvent |= XPLMI_BIT(Idx);
			break;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a notification to the ASU system.
 *
 * @return
 *	- XST_SUCCESS, if notification is sent to ASUFW or the notification task is scheduled
 *		       successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int XOcp_NotifyAsu(void)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[XOCP_ASUFW_EVENT_PAYLOAD_SIZE] = {0U};
	u32 IsAsuPresent = 0U;
	u32 PlmAsufwEvent = XOcpPlmAsufwEvent;
	static u32 TaskCount = 0U;

	if (XOcpPlmAsufwEvent == 0U) {
		/* No event to notify, return success. */
		Status = XST_SUCCESS;
		goto END;
	}

	/*
	 * Check if ASUFW is present or not. If ASUFW is not present, schedule
	 * task to notify ASUFW later.
	 */
	IsAsuPresent = (XPlmi_In32(XOCP_ASU_GLOBAL_GLOBAL_CNTRL)) &
			XOCP_ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK;
	if (IsAsuPresent == 0U) {
		if (TaskCount < XOCP_ASUFW_MAX_TASK_COUNT) {
			Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_XILOCP_ID, XOcp_NotifyAsuTask,
					NULL, XOCP_ASUFW_NOTIFICATION_TASK_DELAY,
					XPLM_TASK_PRIORITY_0, NULL, XPLMI_NON_PERIODIC_TASK);
		}
		TaskCount++;
		goto END;
	}
	XOcpPlmAsufwEvent = 0U;

	/* If ASUFW event is set, generate ASU CDI seed and copy device DNA. */
	if ((PlmAsufwEvent & XOCP_PLM_ASUFW_EVENT_MASK) != 0U) {
		Status = XOcp_GenerateAsuCdiSeed();
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

	/** Assign PLM event based on subsystem state changes. */
	Payload[0U] = XOCP_PLM_ASUFW_CMD_HDR;
	Payload[1U] = PlmAsufwEvent;

	/* Write the IPI payload buffer. */
	Status = XPlmi_IpiWrite(XOCP_ASUFW_IPI_MASK, Payload, XOCP_ASUFW_EVENT_PAYLOAD_SIZE,
				XIPIPSU_BUF_TYPE_MSG);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/* Trigger the IPI to notify ASUFW. */
	Status = XPlmi_IpiTrigger(XOCP_ASUFW_IPI_MASK);
	if (XST_SUCCESS != Status) {
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function triggers an ASU task notification.
 *
 * @param	Arg
 *
 * @return
 *	- XST_SUCCESS, if the ASU task is triggered successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static int XOcp_NotifyAsuTask(void *Arg)
{
	XStatus Status = XST_FAILURE;

	(void)Arg;

	/* Notify ASUFW with the event mask stored in XOcpPlmAsufwEvent. */
	Status = XOcp_NotifyAsu();

	return Status;
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

	/* Get the address of subsystem digest using subsystem ID. */
	for (Idx = 0; Idx < XOCP_ASUFW_MAX_SUBSYS_SUPPORT; Idx++) {
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

	/* Get the address of the subsystem digest for the required subsystem. */
	Status = XOcp_GetSubsysDigestAddr(SubsystemId, &SubsystemHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy subsystem digest to provided address. */
	Status = Xil_SMemCpy((void *)(UINTPTR)SubsysHashAddrPtr, XOCP_SHA3_LEN_IN_BYTES,
			     (const void *)(UINTPTR)SubsystemHash,
			     XOCP_SHA3_LEN_IN_BYTES,
			     XOCP_SHA3_LEN_IN_BYTES);

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

	/* If CDI is not valid device key generation is skipped */
	if (XPlmi_In32(XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID) == 0x0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	/* Read and validate whether, DICE CDI SEED is valid or not */
	Status = XOcp_ValidateDiceCdi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy CDI from PMC global registers to Seed buffer */
	Status = Xil_SMemCpy((void *)(UINTPTR)Seed, XOCP_CDI_SIZE_IN_BYTES,
			     (const void *)(UINTPTR)XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
			     XOCP_CDI_SIZE_IN_BYTES, XOCP_CDI_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Get the subsystem digest for the ASUFW. */
	Status = XOcp_GetSubsysDigestAddr(XOCP_ASU_SUBSYSTEM_ID, &AsuHashAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate ASUFW CDI. */
	Status = XOcp_KeyGenDevAkSeed(PlmCdiAddr, XOCP_CDI_SIZE_IN_BYTES, (u32)(UINTPTR)AsuHashAddr,
				      XOCP_CDI_SIZE_IN_BYTES,
				      (XSecure_HmacRes *)(UINTPTR)XOcpAsuCdi);

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
 **************************************************************************************************/
int XOcp_GetAsuCdiSeed(u32 CdiAddr)
{
	volatile int Status = XST_FAILURE;

	Status = Xil_SMemCpy((void *)(UINTPTR)CdiAddr, XOCP_CDI_SIZE_IN_BYTES,
			     (const void *)(UINTPTR)XOcpAsuCdi, XOCP_CDI_SIZE_IN_BYTES,
			     XOCP_CDI_SIZE_IN_BYTES);

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
	static XOcp_SubsysInfo OcpSubsysInfo[XOCP_ASUFW_MAX_SUBSYS_SUPPORT] = {0U};

	return &OcpSubsysInfo[0];
}

#endif /* PLM_OCP_ASUFW_KEY_MGMT */

/**************************************************************************************************/
/**
 * @brief      This function provides OCP related register space.
 *
 * @return
 * 	- XOcp_RegSpace pointer to register space.
 *
 **************************************************************************************************/
XOcp_RegSpace* XOcp_GetRegSpace(void)
{
	static XOcp_RegSpace RegSpace = {
		.DmeSignRAddr = XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0,
		.DmeSignSAddr = XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0,
		.DiceCdiSeedAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
		.DiceCdiSeedValidAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID,
		.DiceCdiSeedParityAddr = XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY,
		.DevIkPvtAddr = XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0,
		.DevIkPubXAddr = XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0,
		.DevIkPubYAddr = XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0,
	};

	return &RegSpace;
}

/**************************************************************************************************/
/**
 * @brief	This function reads secure efuse configuration.
 *
 * @param	EfuseConfig Pointer to XOcp_SecureConfig.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
void XOcp_ReadSecureConfig(XOcp_SecureConfig* EfuseConfig)
{
	EfuseConfig->BootmodeDis = XPlmi_In32(XOCP_PMC_LOCAL_BOOT_MODE_DIS) &
			XOCP_PMC_LOCAL_BOOT_MODE_DIS_FULLMASK;
	EfuseConfig->BootEnvCtrl = XPlmi_In32(XOCP_EFUSE_CACHE_BOOT_ENV_CTRL);
	EfuseConfig->IpDisable1 = XPlmi_In32(XOCP_EFUSE_CACHE_IP_DISABLE_1);
	EfuseConfig->SecMisc1 = XPlmi_In32(XOCP_EFUSE_CACHE_SECURITY_MISC_1);
	EfuseConfig->Caher1 = XPlmi_In32(XOCP_EFUSE_CACHE_CAHER_1) &
						XOCP_CAHER_1_MEASURED_MASK;
	EfuseConfig->DecOnly = XPlmi_In32(XOCP_EFUSE_CACHE_SECURITY_MISC_0) &
			XOCP_DEC_ONLY_MEASURED_MASK;
	EfuseConfig->SecCtrl = XPlmi_In32(XOCP_EFUSE_CACHE_SECURITY_CONTROL) &
			XOCP_SEC_CTRL_MEASURED_MASK;
	EfuseConfig->MiscCtrl = XPlmi_In32(XOCP_EFUSE_CACHE_MISC_CTRL) &
			XOCP_MISC_CTRL_MEASURED_MASK;
	EfuseConfig->AnlgTrim3 = XPlmi_In32(XOCP_EFUSE_CACHE_ANLG_TRIM_3);
	EfuseConfig->DmeFips = XPlmi_In32(XOCP_EFUSE_CACHE_DME_FIPS) &
			XOCP_DME_FIPS_MEASURED_MASK;
	EfuseConfig->IPDisable0 = XPlmi_In32(XOCP_EFUSE_CACHE_IP_DISABLE_0) &
			XOCP_IP_DISABLE0_MEASURED_MASK;
	EfuseConfig->RomRsvd = XPlmi_In32(XOCP_EFUSE_CACHE_ROM_RSVD) &
			XOCP_ROM_RSVD_MEASURED_MASK;
	EfuseConfig->RoSwapEn = XPlmi_In32(XOCP_EFUSE_CACHE_RO_SWAP_EN);
}

#endif /* PLM_OCP */
