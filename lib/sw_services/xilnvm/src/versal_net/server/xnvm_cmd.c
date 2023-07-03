/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/server/xnvm_cmd.c
*
* This file contains the xilnvm IPI/CDO handler implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0   kal  07/05/2022 Initial release
*       har  07/19/2022 Added support for writing keys, PPK hash,
*                       IV and reading eFUSE cache via IPI
* 3.1   skg  10/17/2022 Added Null to invalid command handler of xilnvm command module
* 3.2   har  02/21/2023 Added support for writing ROM Rsvd bits
*       vek  05/31/2023 Added support for Programming PUF secure control bits
*       bm   06/23/2023 Added access permissions for IPI commands
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xnvm_defs.h"
#include "xnvm_bbram_cdohandler.h"
#include "xnvm_efuse_cdohandler.h"
#include "xnvm_cmd.h"

/************************** Function Prototypes ******************************/

/************************** Constant Definitions *****************************/
static XPlmi_ModuleCmd XNvm_Cmds[XNVM_API_MAX];

/* Structure holding access permissions of nvm module commands */
static XPlmi_AccessPerm_t XNvm_AccessPermBuff[XNVM_API_MAX] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_WRITE_AES_KEY),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_ZEROIZE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_WRITE_USER_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_READ_USER_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_BBRAM_WRITE_AES_KEY_FROM_PLOAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_AES_KEY),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_EFUSE_WRITE_AES_KEY_FROM_PLOAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PPK_HASH),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_IV),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_EFUSE_WRITE_IV_FROM_PLOAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_DEC_ONLY),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_OFFCHIP_REVOKE_ID),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_FIPS_INFO),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_EFUSE_WRITE_UDS_FROM_PLOAD),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_EFUSE_WRITE_DME_KEY_FROM_PLOAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_DME_REVOKE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_CRC),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_DME_MODE),
	XPLMI_ALL_IPI_NO_ACCESS(XNVM_API_ID_EFUSE_WRITE_PUF_HD_FROM_PLOAD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PUF),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_ROM_RSVD),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_WRITE_PUF_CTRL_BITS),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_READ_CACHE),
	XPLMI_ALL_IPI_FULL_ACCESS(XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS),
};

static XPlmi_Module XPlmi_Nvm =
{
	XPLMI_MODULE_XILNVM_ID,
	XNvm_Cmds,
	XNVM_API(XNVM_API_MAX),
	NULL,
	XNvm_AccessPermBuff,
	NULL,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function checks for the supported features based on the
 * 		requested API ID
 *
 * @param	ApiId	ApiId to check the supported features
 *
 * @return 	XST_SUCCESS		if the requested API ID is supported
 * 		XST_INVALID_PARAM	On invalid command
 *
 *****************************************************************************/
static int XNvm_FeaturesCmd(u32 ApiId)
{
	int Status = XST_INVALID_PARAM;

	switch (ApiId) {
	case XNVM_API_ID_BBRAM_WRITE_AES_KEY:
	case XNVM_API_ID_BBRAM_ZEROIZE:
	case XNVM_API_ID_BBRAM_WRITE_USER_DATA:
	case XNVM_API_ID_BBRAM_READ_USER_DATA:
	case XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA:
	case XNVM_API_ID_EFUSE_WRITE_AES_KEY:
	case XNVM_API_ID_EFUSE_WRITE_PPK_HASH:
	case XNVM_API_ID_EFUSE_WRITE_IV:
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_ROM_RSVD):
		Status = XST_SUCCESS;
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Cmd not supported\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function processes XilNvm IPI commands
 *
 * @param	Cmd 	Pointer to the XPlmi_Cmd structure
 *
 * @return 	XST_SUCCESS		On successful IPI processing
 * 		XST_INVALID_PARAM	On invalid command
 * 		Error Code 		On Failure
 *****************************************************************************/
static int XNvm_ProcessCmd(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_FEATURES):
		Status = XNvm_FeaturesCmd(Pload[0]);
                break;
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_AES_KEY):
	case XNVM_API(XNVM_API_ID_BBRAM_ZEROIZE):
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_USER_DATA):
	case XNVM_API(XNVM_API_ID_BBRAM_READ_USER_DATA):
	case XNVM_API(XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA):
	case XNVM_API(XNVM_API_ID_BBRAM_WRITE_AES_KEY_FROM_PLOAD):
		Status = XNvm_BbramCdoHandler(Cmd);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY_FROM_PLOAD):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV_FROM_PLOAD):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DEC_ONLY):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_OFFCHIP_REVOKE_ID):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_FIPS_INFO):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_UDS_FROM_PLOAD):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_KEY_FROM_PLOAD):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_REVOKE):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE):
	case XNVM_API(XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_HD_FROM_PLOAD):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV):
	case XNVM_API(XNVM_API_ID_EFUSE_READ_CACHE):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_CTRL_BITS):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_CRC):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_MODE):
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_ROM_RSVD):
		Status = XNvm_EfuseCdoHandler(Cmd);
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the XilNvm commands to the PLMI.
 *
 *****************************************************************************/
void XNvm_CmdsInit(void)
{
	u32 Idx;

	/* Register command handlers with XilPlmi */
	for (Idx = 0U; Idx < XPlmi_Nvm.CmdCnt; Idx++) {
		XNvm_Cmds[Idx].Handler = XNvm_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Nvm);
}

#endif
