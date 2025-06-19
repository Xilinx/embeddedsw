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
*
* </pre>
*
**************************************************************************************************/

/************************************** Include Files ********************************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_ASUFW_KEY_MGMT
#include "xocp.h"
#include "xocp_plat.h"
#include "xocp_hw.h"

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

/************************************ Function Prototypes ****************************************/
static int XOcp_GetSubsysDigestAddr(u32 SubsystemId, u32 *SubsysHashAddrPtr);
static XOcp_SubsysInfo *XOcp_GetOcpSubsysInfoDb(void);

/********************************** Variable Definitions *****************************************/

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

	/* Copy hash if OCP support is required for the subsystem, else ignore it. */
	for (Idx = 0; Idx < XOCP_ASUFW_MAX_SUBSYS_SUPPORT; Idx++) {
		if (OcpSubsysInfo[Idx].SubsystemId == SubsystemId) {
			Status = XPlmi_MemCpy64((u64)(UINTPTR)OcpSubsysInfo[Idx].SubsystemHash,
						Hash, XOCP_SHA3_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		}
	}
	Status = XST_SUCCESS;

END:
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
