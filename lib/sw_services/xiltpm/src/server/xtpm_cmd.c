/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_cmd.c
*
* This is the file which contains TPM command related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.3   pre  03/09/26 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/**
 * @addtogroup xtpm_apis XilTPM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xil_util.h"
#include "xplmi_modules.h"
#include "xplmi_cmd.h"
#include "xtpm_cmd.h"
#include "xtpm_error.h"
#include "xtpm_hw.h"
#include "xtpm.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_ADDR_HIGH_SHIFT	(32U)	/**< Shift to get upper 32 bits of address */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XPlmi_Module XPlmi_Tpm;
static u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};

/*****************************************************************************/
/**
 * @brief	This function checks for the supported features based on the
 * 			requested API ID
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS
 *
 *****************************************************************************/
static int XilTpm_Features(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
#ifdef PLM_TPM
	if (Cmd->Payload[XTPM_INDEX_0] < XPlmi_Tpm.CmdCnt) {
		Cmd->Response[XTPM_INDEX_1] = (u32)XST_SUCCESS;
	} else {
		Cmd->Response[XTPM_INDEX_1] = (u32)XST_FAILURE;
	}
	Cmd->Response[XTPM_INDEX_0] = (u32)XST_SUCCESS;
#else
	(void)Cmd;
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
#endif
	return XST_SUCCESS;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the data from the input address and extends it to the PCR index
 * 			specified in the input with PCR event command.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- High DataAddr - Upper 32 bit value of Data address
 *			- Low DataAddr - Lower 32 bit value of Data address
 *			- Data Size - Size of the data to be extended using PCR event command
 *			- Pcr Index - PCR index to which data needs to be extended
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *************************************************************************************************/
static int XilTpm_Pcr_Event(XPlmi_Cmd *Cmd)
{
#ifdef PLM_TPM
	int Status = XST_FAILURE;
	u64 Addr = (((u64)Cmd->Payload[XTPM_INDEX_1] << XTPM_ADDR_HIGH_SHIFT) | Cmd->Payload[XTPM_INDEX_0]);
	u32 PcrIndex = Cmd->Payload[XTPM_INDEX_3];

	/** Validate input parameters. PCR0 and PCR1 are reserved for ROM and PLM measurements */
	if (PcrIndex < XTPM_PCR_2 || PcrIndex > XTPM_PCR_23) {
		Status = (int)XTPM_ERR_PCR_INDEX_INVALID;
		goto END;
	}

	Status = (int)XTpm_Event(PcrIndex, (u16)Cmd->Payload[XTPM_INDEX_2], (const u8 *)(UINTPTR)Addr, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verifies response. Returns XST_SUCCESS on success response reception. Otherwise, returns XTPM_ERR_RESP_POLLING error.
	 */
	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
		Status = (int)XTPM_ERR_RESP_POLLING;
	}

END:
	return Status;
#else
	(void)Cmd;
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	return XST_SUCCESS;
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the TPM
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *************************************************************************************************/
static int XilTpm_Init(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
#ifdef PLM_TPM
	return (int)XTpm_Init();
#else
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	return XST_SUCCESS;
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function starts up the TPM
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *************************************************************************************************/
static int XilTpm_StartUp(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
#ifdef PLM_TPM
	return (int)XTpm_StartUp();
#else
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	return XST_SUCCESS;
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function performs self-test of the TPM
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *************************************************************************************************/
static int XilTpm_SelfTest(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
#ifdef PLM_TPM
	return (int)XTpm_SelfTest();
#else
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	return XST_SUCCESS;
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function reads PCR value for the specified PCR index and hash algorithm using
 *          PCR_READ command.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Pcr Index - PCR index to be read
 *			- Hash Algorithm - Hash algorithm identifier for which PCR value needs to be read
 *			- Response Buffer Addr - Address of the buffer in which PCR read response will be stored
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 *************************************************************************************************/
static int XilTpm_Pcr_Read(XPlmi_Cmd *Cmd)
{
#ifdef PLM_TPM
	int Status = XST_FAILURE;
	u32 PcrIndex = Cmd->Payload[XTPM_INDEX_0];
	u8 HashAlgo = (u8)Cmd->Payload[XTPM_INDEX_1];
	u64 RespBufferAddr = (((u64)Cmd->Payload[XTPM_INDEX_3] << 32) | Cmd->Payload[XTPM_INDEX_2]);
	u8 *RespBufPtr = (u8 *)(UINTPTR)RespBufferAddr;

	Status = (int)XTpm_PcrRead(PcrIndex, HashAlgo, TpmRespBuffer);
	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
		Status = (int)XTPM_ERR_RESP_POLLING;
		goto END;
	}

	/* Copy the PCR read response from local buffer to the response buffer address provided by the client */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)RespBufPtr, (u64)(UINTPTR)&TpmRespBuffer[XTPM_PCR_VALUE_START_INDEX], (u32)(TpmRespBuffer[XTPM_INDEX_5] - XTPM_PCR_VALUE_START_INDEX));

END:
	return Status;
#else
	(void)Cmd;
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	return XST_SUCCESS;
#endif
}

/*************************************************************************************************/
/**
 * @brief	Contains the array of PLM TPM commands
 *
 *************************************************************************************************/
static const XPlmi_ModuleCmd XTpm_Cmds[] =
{
	XPLMI_MODULE_COMMAND(XilTpm_Features),
	XPLMI_MODULE_COMMAND(XilTpm_Init),
	XPLMI_MODULE_COMMAND(XilTpm_StartUp),
	XPLMI_MODULE_COMMAND(XilTpm_SelfTest),
	XPLMI_MODULE_COMMAND(XilTpm_Pcr_Event),
	XPLMI_MODULE_COMMAND(XilTpm_Pcr_Read),
};

/*****************************************************************************/
/**
 * @brief	Contains the array of PLM TPM access permissions
 *
 *****************************************************************************/
static XPlmi_AccessPerm_t XTpm_AccessPermBuff[XPLMI_ARRAY_SIZE(XTpm_Cmds)] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_FEATURES),
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_INIT),
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_STARTUP),
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_SELFTEST),
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_PCR_EVENT),
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_PCR_READ),
};

/*****************************************************************************/
/**
 * @brief	Contains the module ID and loader commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Tpm =
{
	XPLMI_MODULE_TPM_ID,
	XTpm_Cmds,
	XPLMI_ARRAY_SIZE(XTpm_Cmds),
	NULL,
	XTpm_AccessPermBuff,
#ifdef VERSAL_2VE_2VM
	NULL,
#endif
};

/*****************************************************************************/
/**
 * @brief	This function registers the PLM TPM commands to the PLMI.
 *
 * @return  None
 *****************************************************************************/
void XTpm_CmdsInit(void)
{
	XPlmi_ModuleRegister(&XPlmi_Tpm);
}

#endif	/* PLM_TPM */

/** @} End of xtpm_apis group */