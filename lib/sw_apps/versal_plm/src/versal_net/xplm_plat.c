/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplm_plat.c
*
* This file contains the PLMI versal_net platform specific code.
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
#include "xpm_psm.h"
#include "xilpdi.h"
#include "xloader.h"
#include "xloader_plat.h"
#ifdef PLM_OCP
#ifdef PLM_OCP_KEY_MNGMT
#include "xocp_keymgmt.h"
#endif
#include "xocp.h"
#endif
#ifdef XPLMI_IPI_DEVICE_ID
#include "xplmi_ipi.h"
#endif

/************************** Constant Definitions *****************************/
#define XPLMI_PSM_COUNTER_VER 		(1U) /**< PSM counter version */
#define XPLMI_PSM_COUNTER_LCVER		(1U) /**< PSM counter lowest compatible version */
#define XPLMI_PSM_KEEP_ALIVE_STS_VER 	(1U) /**< PSM keep alive status version */
#define XPLMI_PSM_KEEP_ALIVE_STS_LCVER	(1U) /**< PSM keep alive status lowest compatible version */

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
* @return	None
*
*****************************************************************************/
void XPlm_EnablePlatformSlaveErrors(void)
{
	/* TODO Add versalnet specific slave errors */
	return;
}

#ifdef XPLMI_IPI_DEVICE_ID
/*****************************************************************************/
/**
* @brief	This function updates the keep alive status variable
*
* @param	Val to set the status as started or not started or error
*
* @return	PsmKeepAliveStatus
*
*****************************************************************************/
u32 XPlm_SetPsmAliveStsVal(u32 Val)
{
	static u32 PsmKeepAliveStatus __attribute__ ((aligned(4U)))
			= XPLM_PSM_ALIVE_NOT_STARTED;
	EXPORT_XILPSM_DS(PsmKeepAliveStatus, XPM_PSM_KEEP_ALIVE_STS_DS_ID,
		XPLMI_PSM_KEEP_ALIVE_STS_VER, XPLMI_PSM_KEEP_ALIVE_STS_LCVER,
		sizeof(PsmKeepAliveStatus), (u32)(UINTPTR)&PsmKeepAliveStatus);

	if(Val != XPLM_PSM_ALIVE_RETURN) {
		/* Update the Keep Alive Status */
		PsmKeepAliveStatus = Val;
	}

	return PsmKeepAliveStatus;
}

/*****************************************************************************/
/**
* @brief	This function updates the counter value
*
* @param	Val to Increment or Clear the CounterVal variable
*
* @return	CounterVal
*
*****************************************************************************/
u32 XPlm_UpdatePsmCounterVal(u32 Val)
{
	static u32 CounterVal __attribute__ ((aligned(4U))) = 0U;
	EXPORT_XILPSM_DS(CounterVal, XPM_PSM_COUNTER_DS_ID, XPLMI_PSM_COUNTER_VER,
		XPLMI_PSM_COUNTER_LCVER, sizeof(CounterVal), (u32)(UINTPTR)&CounterVal);

	if(Val == XPLM_PSM_COUNTER_INCREMENT) {
		/* Increment the counter value */
		CounterVal++;
	}else if(Val == XPLM_PSM_COUNTER_CLEAR){
		/* Clear the counter value */
		CounterVal = 0U;
	} else{
		/* To avoid Misra-C violation  */
	}

	return CounterVal;
}
#endif /* XPLMI_IPI_DEVICE_ID */

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
	u32 Checksum;
	u32 Index;
	u32 PdiVersion;
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
	if (PdiVersion < XLOADER_PDI_VERSION_4) {
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
			Offset += ((XPlmi_In32(Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_LEN_SHIFT) << XPLMI_WORD_LEN_SHIFT;
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
 * @brief	This function performs post update activites like restoring
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

	/**
	 * Update KEK red key availability status if PLM is encrypted with
	 * Black key
	 */
#ifndef PLM_SECURE_EXCLUDE
	XSECURE_REDUNDANT_CALL(BootPdiInfo->DecKeySrc, DecKeySrcTmp, XLoader_GetKekSrc);
	if (BootPdiInfo->DecKeySrc != DecKeySrcTmp) {
		Status = XST_GLITCH_ERROR;
		goto END;
	}
	XPlmi_GetBootKatStatus((volatile u32*)&BootPdiInfo->PlmKatStatus);

	/**
	 * Update DecKeySrc after clearing RED keys
	 * when RedKeyClear is set in PMCRAM
	 */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_ClearAesKey, &BootPdiInfo->DecKeySrc);

#endif
	/* Regenerate DEVAK keys of the sub-systems */
#ifdef PLM_OCP
	#ifdef PLM_OCP_KEY_MNGMT
	Status = XOcp_RegenSubSysDevAk();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	#endif
	/* Restore secure state configuration and extend SWPCR */
	Status = XOcp_MeasureSecureStateAndExtendSwPcr();
#else
	Status = XST_SUCCESS;
#endif
	/*
	 * Set FW_IS_PRESENT bit indicating new PLM is ready.
	 * Note that any error before this point in new PLM will be treated as
	 * a fatal error and IPOR is performed.
	 */
	XPlmi_SetBootPdiDone();

	XPlmi_Printf(DEBUG_GENERAL, "In-Place PLM Update Done\n\r");
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
