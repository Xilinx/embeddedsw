/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_asu_cmd.c
*
* This file contains asu commands code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================
* 1.00  am   02/19/25 Initial release
*       rmv  07/31/25 Add XPlmi_CmdAsuCdiTransfer() function to transfer
*		      ASU CDI to provided address.
*       rmv  07/31/25 Add XPlmi_CmdAsuSubsystemHashTransfer() function to transfer
*		      subsystem hash to provided address.
*       rmv  08/26/25 Use callback functions instead of xilocp library functions
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_modules.h"
#include "xplmi.h"
#include "xplmi_debug.h"
#include "xplmi_asu_cmd.h"

/************************** Constant Definitions *****************************/
/* ASU key transfer command Ids */
#define XPLMI_CMD_ID_ASU_FEATURES	(0U)
				/**< Command Id of ASU module features */
#define XPLMI_CMD_ID_ASU_KEY_TRANSFER	(1U)
				/**< Command Id of ASU key transfer features */
#define XPLMI_CMD_ID_ASU_CDI_TRANSFER	(2U)
				/**< Command Id of ASU CDI transfer features */
#define XPLMI_CMD_ID_SUBSYSTEM_HASH_TRANSFER	(3U)
				/**< Command Id of subsystem hash transfer features */
/* Command Id index */
#define XPLMI_CMD_ID_ASU_FEATURES_INDEX		(0U)
				/**< ASU module features index */
#define XPLMI_CMD_ID_ASU_RESPONSE_INDEX		(1U)
				/**< ASU module response index */
#define XPLMI_RESP_CMD_EXEC_STATUS_INDEX	(0U)
				/**< Response command execution status index */
#define XPLMI_CMD_ID_ASU_CDI_ADDR_INDEX		(0U)
#define XPLMI_CMD_ID_SUBSYSTEM_ID_INDEX		(0U)
#define XPLMI_CMD_ID_SUBSYSTEM_HASH_ADDR_INDEX	(1U)

/**************************** Type Definitions *******************************/
static int (* AsuGeneratePufKEK)(void) = NULL;
	/** Static function pointer which holds address of Xplmi_PufOnDemandRegeneration. */
static int (* AsuInitiateKeyXfer)(void) = NULL;
	/** Static function pointer which holds address of XSecure_InitiateASUKeyTransfer. */
static int (* AsuGetAsuCdiSeed)(u32 CdiAddr) = NULL;
	/** Static function pointer which holds address of XOcp_GetAsuCdiSeed. */
static int (* AsuGetSubsysDigest)(u32 SubsystemId, u32 SubsysHashAddrPtr) = NULL;
	/** Static function pointer which holds address of XOcp_GetSubsysDigest. */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	Contains the module ID and PLM error commands array
 *
 *****************************************************************************/
/**
 * @{
 * @cond xplmi_internal
 */
static XPlmi_Module XPlmi_AsuModule;
/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief	This function is reserved to get the supported features for this
 * 		module.
 *
 * @param	Cmd	is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_CmdAsuFeatures(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_GENERAL, "%s %p\n\r", __func__, Cmd);

	if (Cmd->Payload[XPLMI_CMD_ID_ASU_FEATURES_INDEX] <
			XPlmi_AsuModule.CmdCnt) {
		Cmd->Response[XPLMI_CMD_ID_ASU_RESPONSE_INDEX] = (u32)XST_SUCCESS;
	}
	else {
		Cmd->Response[XPLMI_CMD_ID_ASU_RESPONSE_INDEX] = (u32)XST_FAILURE;
	}

	Status = XST_SUCCESS;

	Cmd->Response[XPLMI_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function transfers keys as prescribed by the command.

 * @param	Cmd	is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 *
 *****************************************************************************/
static int XPlmi_CmdAsuKeyTransfer(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	if (Cmd == NULL) {
		goto RET;
	}

	if ((AsuInitiateKeyXfer == NULL) || (AsuGeneratePufKEK == NULL)) {
		goto END;
	}

	Status = AsuGeneratePufKEK();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = AsuInitiateKeyXfer();

END:
	Cmd->Response[XPLMI_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;

RET:
	return Status;
}

#ifdef PLM_OCP_ASUFW_KEY_MGMT
/*****************************************************************************/
/**
 * @brief	This function transfers the ASU CDI to provided address.
 *
 * @param	Cmd	Pointer to the command structure
 *
 * @return
 *	- XST_SUCCESS, if ASU CDI is transferred successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *****************************************************************************/
static int XPlmi_CmdAsuCdiTransfer(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 CdiAddr;

	/* Validate input parameter. */
	if (Cmd == NULL) {
		goto END;
	}

	CdiAddr = Cmd->Payload[XPLMI_CMD_ID_ASU_CDI_ADDR_INDEX];

	/* Copy ASU CDI to provided address. */
	if (AsuGetAsuCdiSeed != NULL) {
		Status = AsuGetAsuCdiSeed(CdiAddr);
	}

	Cmd->Response[XPLMI_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function transfers the subsystem hash to provided address.
 *
 * @param	Cmd	Pointer to the command structure
 *
 * @return
 *	- XST_SUCCESS, if subsystem hash is transferred successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *****************************************************************************/
static int XPlmi_CmdAsuSubsystemHashTransfer(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 HashAddr;
	u32 SubsystemId;

	/* Validate input parameter. */
	if (Cmd == NULL) {
		goto END;
	}

	SubsystemId = Cmd->Payload[XPLMI_CMD_ID_SUBSYSTEM_ID_INDEX];
	HashAddr = Cmd->Payload[XPLMI_CMD_ID_SUBSYSTEM_HASH_ADDR_INDEX];

	/* Copy subsystem hash to provided address. */
	if (AsuGetSubsysDigest != NULL) {
		Status = AsuGetSubsysDigest(SubsystemId, HashAddr);
	}

	Cmd->Response[XPLMI_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;

END:
	return Status;
}
#endif

/**
 * @{
 * @cond xplmi_internal
 */
/*****************************************************************************/
/**
 * @brief	Contains the array of PLM ASU commands
 *
 *****************************************************************************/
static const XPlmi_ModuleCmd XPlmi_AsuCmds[] =
{
	XPLMI_MODULE_COMMAND(XPlmi_CmdAsuFeatures),
	XPLMI_MODULE_COMMAND(XPlmi_CmdAsuKeyTransfer),
#ifdef PLM_OCP_ASUFW_KEY_MGMT
	XPLMI_MODULE_COMMAND(XPlmi_CmdAsuCdiTransfer),
	XPLMI_MODULE_COMMAND(XPlmi_CmdAsuSubsystemHashTransfer),
#else
	NULL,
	NULL,
#endif
};

/*****************************************************************************/
/**
 * @brief	Contains the array of ASU commands access permissions
 *
 *****************************************************************************/
static XPlmi_AccessPerm_t XPlmi_AsuAccessPermBuff[XPLMI_ARRAY_SIZE(XPlmi_AsuCmds)] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_CMD_ID_ASU_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_CMD_ID_ASU_KEY_TRANSFER),
#ifdef PLM_OCP_ASUFW_KEY_MGMT
	XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_CMD_ID_ASU_CDI_TRANSFER),
	XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_CMD_ID_SUBSYSTEM_HASH_TRANSFER),
#else
	0U,
	0U,
#endif
};

/*****************************************************************************/
/**
 * @brief	Contains the module ID and ASU commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_AsuModule =
{
	XPLMI_MODULE_ASU_ID,
	XPlmi_AsuCmds,
	XPLMI_ARRAY_SIZE(XPlmi_AsuCmds),
	NULL,
	XPlmi_AsuAccessPermBuff,
	NULL
};

/*****************************************************************************/
/**
 * @brief	This function registers the ASU commands to the PLMI and
 * 		initialize the PUF and Key transfer function pointers during
 * 		module initialization.
 *
 * @param	GeneratePufKEK - Pointer to the PUF on demand regeneration function.
 * @param	InitiateKeyXfer - Pointer to the secure key transfer function.
 * @param	GetAsuCdiSeed - Pointer to the function which provides ASU CDI.
 * @param	GetSubsysDigest - Pointer to the function which provides subsystem digest.
 *
 *****************************************************************************/
void XPlmi_AsuModuleInit(int (* const GeneratePufKEK)(void),
	int (* const InitiateKeyXfer)(void),
	int (* const GetAsuCdiSeed)(u32 CdiAddr),
	int (* const GetSubsysDigest)(u32 SubsystemId, u32 SubsysHashAddrPtr))
{
	XPlmi_ModuleRegister(&XPlmi_AsuModule);

	/** Initialize the PUF and Key transfer function pointers */
	AsuGeneratePufKEK = GeneratePufKEK;
	AsuInitiateKeyXfer = InitiateKeyXfer;
	AsuGetAsuCdiSeed = GetAsuCdiSeed;
	AsuGetSubsysDigest = GetSubsysDigest;
}

/**
 * @}
 * @endcond
 */

/** @} */
