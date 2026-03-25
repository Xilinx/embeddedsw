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
*       pre  03/21/26 Added GetPcrLog server API
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
static int XilTpm_Features(XPlmi_Cmd *Cmd);
static int XilTpm_Init(XPlmi_Cmd *Cmd);
static int XilTpm_StartUp(XPlmi_Cmd *Cmd);
static int XilTpm_SelfTest(XPlmi_Cmd *Cmd);
static int XilTpm_PcrEvent(XPlmi_Cmd *Cmd);
static int XilTpm_PcrRead(XPlmi_Cmd *Cmd);
static int XilTpm_GetPcrLog(XPlmi_Cmd *Cmd);

/************************** Variable Definitions *****************************/

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
	XPLMI_MODULE_COMMAND(XilTpm_PcrEvent),
	XPLMI_MODULE_COMMAND(XilTpm_PcrRead),
	XPLMI_MODULE_COMMAND(XilTpm_GetPcrLog),
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
	XPLMI_ALL_IPI_FULL_ACCESS(XTPM_API_ID_GET_PCR_LOG),
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
	/** - Check if the requested API ID is supported.
	 * If supported, set success in response buffer else set failure.
	 */
	if (Cmd->Payload[XTPM_INDEX_0] < XPlmi_Tpm.CmdCnt) {
		Cmd->Response[XTPM_INDEX_1] = (u32)XST_SUCCESS;
	} else {
		Cmd->Response[XTPM_INDEX_1] = (u32)XST_FAILURE;
	}
	/** - Set success in command response for feature query command */
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
 * 			- Pcr Index - PCR index to which data needs to be extended
 *			- High DataAddr - Upper 32 bit value of Data address
 *			- Low DataAddr - Lower 32 bit value of Data address
 *			- Data Length - Length of the data in bytes to be extended using PCR event command
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *************************************************************************************************/
static int XilTpm_PcrEvent(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
#ifdef PLM_TPM
	/** - Get the PCR index and data address from the command payload */
	u32 PcrIndex = Cmd->Payload[XTPM_INDEX_0];
	u64 Addr = (((u64)Cmd->Payload[XTPM_INDEX_2] << XTPM_ADDR_HIGH_SHIFT) | Cmd->Payload[XTPM_INDEX_1]);

	/** - Verify the address range for the data to be extended */
	XPLMI_VERIFY_ADDR_RANGE(Cmd->SubsystemId, Addr,
							Cmd->Payload[XTPM_INDEX_3], Status,
							XTPM_ERR_INVALID_ADDR_RANGE, END);

	/** - Validate input parameters. PCR0 and PCR1 are reserved for ROM and PLM measurements, PCR17 to PCR22 are not extendable */
	if ((PcrIndex < XTPM_PCR_2) || (PcrIndex > XTPM_PCR_23) || ((PcrIndex > XTPM_PCR_16) && (PcrIndex < XTPM_PCR_23))) {
		Status = (int)XTPM_ERR_PCR_INDEX_INVALID;
		goto END;
	}

	/**
	 * - Get the data from the input address and extend it to PCR using PCR_EVENT command and gets response.
	 * If the command is not successful, returns error code.
	 */
	Status = (int)XTpm_Event(PcrIndex, (u16)Cmd->Payload[XTPM_INDEX_3], (const u8 *)(UINTPTR)Addr);

#ifndef PLM_ENABLE_ADDR_RANGE_VALIDATION
	goto END;
#endif
END:
#else
	(void)Cmd;
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	Status = XST_SUCCESS;
#endif
	return Status;
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
static int XilTpm_PcrRead(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
#ifdef PLM_TPM
	/** - Get the PCR index, hash algorithm and response buffer address from the command payload */
	u32 PcrIndex = Cmd->Payload[XTPM_INDEX_0];
	u8 HashAlgo = (u8)Cmd->Payload[XTPM_INDEX_1];
	u64 RespBufferAddr = (((u64)Cmd->Payload[XTPM_INDEX_3] << XTPM_ADDR_HIGH_SHIFT) |
						Cmd->Payload[XTPM_INDEX_2]);

	/** - Verify the address range for the response buffer to store PCR read response */
	XPLMI_VERIFY_ADDR_RANGE(Cmd->SubsystemId, RespBufferAddr,
							PCR_VALUE_MAX_LEN, Status,
							XTPM_ERR_INVALID_ADDR_RANGE, END);

	/** - Read the PCR value using the specified index and hash algorithm */
	Status = (int)XTpm_PcrRead(PcrIndex, HashAlgo, RespBufferAddr);

#ifndef PLM_ENABLE_ADDR_RANGE_VALIDATION
	goto END;
#endif
END:
#else
	(void)Cmd;
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	Status = XST_SUCCESS;
#endif
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function handler calls XTpm_GetPcrLog server API to get log
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 * 		- PcrEventAddrLow - Lower 32 bit address of the PCR event buffer address
 * 		- PcrEventAddrHigh - Higher 32 bit address of the PCR event buffer address
 * 		- PcrLogInfoAddrLow - Lower 32 bit address of the PCR log info buffer address
 * 		- PcrLogInfoAddrHigh - Higher 32 bit address of the PCR log info buffer address
 * 		- NumOfLogEntries - Number of log entries to read
 *
 * @return
 *          - XST_SUCCESS - If log contents are copied
 *          - ErrorCode - Upon any failure
 *
 *************************************************************************************************/
static int XilTpm_GetPcrLog(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
#ifdef PLM_TPM
	/** - Get the PCR event buffer address and PCR log info buffer address from the command payload */
	u64 PcrEventAddr = ((u64)Cmd->Payload[XTPM_INDEX_1] << XTPM_ADDR_HIGH_SHIFT) |
						(u64)Cmd->Payload[XTPM_INDEX_0];
	u64 PcrLogInfoAddr = ((u64)Cmd->Payload[XTPM_INDEX_3] << XTPM_ADDR_HIGH_SHIFT) |
							(u64)Cmd->Payload[XTPM_INDEX_2];

	/** - Verify the address range for the PCR event buffer and PCR log info buffer */
	XPLMI_VERIFY_ADDR_RANGE(Cmd->SubsystemId, PcrEventAddr,
							(sizeof(XTpm_PcrEvent_t) * Cmd->Payload[XTPM_INDEX_4]),
							Status, XTPM_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(Cmd->SubsystemId, PcrLogInfoAddr,
							sizeof(XTpm_PcrLogInfo_t), Status, XTPM_ERR_INVALID_ADDR_RANGE, END);

	/** - Get the PCR log and write to specified event buffer and log info buffer */
	Status = (int)XTpm_GetPcrLog(PcrEventAddr, PcrLogInfoAddr, Cmd->Payload[XTPM_INDEX_4]);

#ifndef PLM_ENABLE_ADDR_RANGE_VALIDATION
	goto END;
#endif
END:
#else
	(void)Cmd;
	XTpm_Printf(XTPM_DEBUG_GENERAL, "TPM is disabled\r\n");
	Status = XST_SUCCESS;
#endif
	return Status;
}

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