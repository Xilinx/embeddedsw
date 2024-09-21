/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/server/xnvm_efuse_ipihandler.c
*
* This file contains the XilNvm eFUSE IPI Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   07/30/2021 Initial release
*       kpt   08/27/2021 Added server API's to support puf helper data efuse
*                        programming
* 2.4   bsv  09/09/2021 Added PLM_NVM macro
* 2.5   kpt  12/09/2021 Replaced magic number with XNVM_API_ID_MASK
*       har  01/03/2022 Renamed NumOfPufFuses as NumOfPufFusesRows
*       kpt  01/19/2022 Cleared AesKeys structure and added redundancy
*       kpt  01/31/2022 Removed redundant code in XNvm_EfuseMemCopy
*       am   02/28/2022 Fixed MISRA C violation rule 4.5
* 3.0   kal  08/01/2022 Added volatile keyword to ClearStatus and
* 			ClearStatusTmp
*       dc   08/29/2022 Removed initializations for optimization
* 3.1   skg  10/25/2022 Added in body comments for APIs
*       skg  12/07/2022 Added Additional PPKs support
* 3.3	vss  02/23/2024	Added IPI support for eFuse read and write
*	vss  05/20/2024 Added IPI support for AES key write
*       ng   11/22/2023 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xnvm_efuse_server_apis XilNvm eFUSE Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xnvm_efuse.h"
#include "xnvm_efuse_ipihandler.h"
#include "xnvm_defs.h"
#include "xnvm_init.h"
#include "xplmi_dma.h"
#include "xplmi_sysmon.h"
#include "xnvm_utils.h"
#include "xplmi_hw.h"
#include "xplmi_plat.h"

/************************** Constant Definitions *****************************/
#ifdef XNVM_ACCESS_PUF_USER_DATA
#define XNVM_EFUSE_NUM_OF_PUF_FUSES	(128U)
#endif
/**< eFuse word length */
#define XNVM_EFUSE_WORD_LEN			(4U)

/************************** Function Prototypes *****************************/
static int XNvm_EfuseDataWrite(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseIvRead(XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseRevocationIdRead(XNvm_RevocationId RevokeIdNum,
	u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseUserFusesRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfusePpkHashRead(XNvm_PpkType PpkHashType, u32 AddrLow,
	u32 AddrHigh);
static int XNvm_EfuseMiscCtrlBitsRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseSecCtrlBitsRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseSecMisc1BitsRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseBootEnvCtrlBitsRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfusePufCtrlBitsRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseOffChipIdRead(XNvm_OffchipId IdNum, u32 AddrLow,
	u32 AddrHigh);
static int XNvm_EfuseDecEfuseOnlyRead(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseDnaRead(u32 AddrLow, u32 AddrHigh);
#ifdef XNVM_ACCESS_PUF_USER_DATA
static int XNvm_EfusePufUserDataWrite(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfusePufUserFusesRead(u32 AddrLow, u32 AddrHigh);
#else
static int XNvm_EfusePufWrite(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfusePufRead(u32 AddrLow, u32 AddrHigh);
#endif
static INLINE int XNvm_EfuseMemCopy(u64 SourceAddr, u64 DestAddr, u32 Len);
#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE))
static int XNvm_EfuseMiscCtrlWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseSecCtrlWriteAccess(u64 Addr, u8 EnvMonitorDis);
#endif
#ifdef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
static int XNvm_EfuseOffChipIdsWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseRevokeIdsWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfusePpkhashWriteAccess(u64 Addr, u8 EnvMonitorDis);
#endif
#ifdef XNVM_WRITE_USER_EFUSE
static int XNvm_EfuseUserFuseWriteAccess(u64 Addr, u8 EnvMonitorDis);
#endif
#ifdef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
static int XNvm_EfuseIvWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseSecMisc1WriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseAnlgTrimWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseBootEnvCtrlWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfusePufWriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseSecMisc0WriteAccess(u64 Addr, u8 EnvMonitorDis);
static int XNvm_EfuseAesKeysWriteAccess(u64 Addr, u8 EnvMonitorDis);
#endif

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	/**
	 *  Validate the input parameters. Return error code if input parameters are not valid
	 */
	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Calls the function according to API ID
	 */
	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE):
		Status = XNvm_EfuseDataWrite(Pload[0U], Pload[1U]);
		break;
#ifdef XNVM_ACCESS_PUF_USER_DATA
	case XNVM_API(XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE):
		Status = XNvm_EfusePufUserDataWrite(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE):
		Status = XNvm_EfusePufUserFusesRead(Pload[0U], Pload[1U]);
		break;
#else
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF):
		Status = XNvm_EfusePufWrite(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_PUF):
		Status = XNvm_EfusePufRead(Pload[0U], Pload[1U]);
		break;
#endif
	case XNVM_API(XNVM_API_ID_EFUSE_READ_IV):
		Status = XNvm_EfuseIvRead((XNvm_IvType)Pload[0U], Pload[1U],
				Pload[2U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_REVOCATION_ID):
		Status = XNvm_EfuseRevocationIdRead((XNvm_RevocationId)Pload[0U],
				Pload[1U], Pload[2U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID):
		Status = XNvm_EfuseOffChipIdRead((XNvm_OffchipId)Pload[0U],
				Pload[1U], Pload[2U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_USER_FUSES):
		Status = XNvm_EfuseUserFusesRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS):
		Status = XNvm_EfuseMiscCtrlBitsRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS):
		Status = XNvm_EfuseSecCtrlBitsRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS):
		Status = XNvm_EfuseSecMisc1BitsRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS):
		Status = XNvm_EfuseBootEnvCtrlBitsRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS):
		Status = XNvm_EfusePufCtrlBitsRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_PPK_HASH):
		Status = XNvm_EfusePpkHashRead((XNvm_PpkType)Pload[0U],
				Pload[1U], Pload[2U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY):
		Status = XNvm_EfuseDecEfuseOnlyRead(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_DNA):
		Status = XNvm_EfuseDnaRead(Pload[0U], Pload[1U]);
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
 * @brief       This function programs eFuses requested by the client with
 * 		the data provided in given address.
 *
 * @param 	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseDataAddr structure
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseDataAddr structure
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseDataWrite(u32 AddrLow, u32 AddrHigh)
{
	int Status = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfuseIvs Ivs __attribute__ ((aligned (32U)));
	XNvm_EfuseGlitchCfgBits GlitchData __attribute__ ((aligned (32U)));
	XNvm_EfuseAesKeys AesKeys __attribute__ ((aligned (32U)));
	XNvm_EfusePpkHash PpkHash __attribute__ ((aligned (32U)));
	XNvm_EfuseDecOnly DecOnly __attribute__ ((aligned (32U)));
	XNvm_EfuseMiscCtrlBits MiscCtrlBits __attribute__ ((aligned (32U)));
	XNvm_EfuseSecCtrlBits SecCtrlBits __attribute__ ((aligned (32U)));
	XNvm_EfuseRevokeIds RevokeIds __attribute__ ((aligned (32U)));
	XNvm_EfuseOffChipIds OffChipIds __attribute__ ((aligned (32U)));
	XNvm_EfuseBootEnvCtrlBits BootEnvCtrl __attribute__ ((aligned (32U)));
	XNvm_EfuseSecMisc1Bits SecMisc1Bits	__attribute__ ((aligned (32U)));
	XNvm_EfuseUserDataAddr UserFusesAddr __attribute__ ((aligned (32U)));
	XNvm_EfuseUserData UserData __attribute__ ((aligned (32U)));
	XNvm_EfuseData EfuseData __attribute__ ((aligned (32U)));
	XNvm_EfuseDataAddr EfuseDataAddr __attribute__ ((aligned (32U)));
	u32 UserFuseArr[XNVM_NUM_OF_USER_FUSES]	__attribute__ ((aligned (32U)));
#ifdef XNVM_EN_ADD_PPKS
	XNvm_EfuseAdditionalPpkHash AdditionalPpkHash __attribute__ ((aligned (32U)));
#endif

	Status = Xil_SMemSet(&EfuseData, sizeof(XNvm_EfuseData), 0U, sizeof(XNvm_EfuseData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&EfuseDataAddr,
			sizeof(EfuseDataAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.EnvMonitorDis = (u8)EfuseDataAddr.EnvMonDisFlag;
	if (EfuseData.EnvMonitorDis == TRUE) {
		EfuseData.SysMonInstPtr = NULL;
	}
	else {
		EfuseData.SysMonInstPtr = XPlmi_GetSysmonInst();
	}

	if (EfuseDataAddr.AesKeyAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.AesKeyAddr,
				(u64)(UINTPTR)&AesKeys, sizeof(AesKeys));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.AesKeys = &AesKeys;
	}

	if (EfuseDataAddr.PpkHashAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.PpkHashAddr,
				(u64)(UINTPTR)&PpkHash, sizeof(PpkHash));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.PpkHash = &PpkHash;
	}

	if (EfuseDataAddr.DecOnlyAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.DecOnlyAddr,
				(u64)(UINTPTR)&DecOnly, sizeof(DecOnly));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.DecOnly = &DecOnly;
	}

	if (EfuseDataAddr.SecCtrlAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.SecCtrlAddr,
				(u64)(UINTPTR)&SecCtrlBits,
				sizeof(SecCtrlBits));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.SecCtrlBits = &SecCtrlBits;
	}

	if (EfuseDataAddr.MiscCtrlAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.MiscCtrlAddr,
				(u64)(UINTPTR)&MiscCtrlBits,
				sizeof(MiscCtrlBits));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.MiscCtrlBits = &MiscCtrlBits;
	}

#ifdef XNVM_EN_ADD_PPKS
	if (EfuseDataAddr.AdditionalPpkHashAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.AdditionalPpkHashAddr,
				(u64)(UINTPTR)&AdditionalPpkHash, sizeof(AdditionalPpkHash));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.AdditionalPpkHash = &AdditionalPpkHash;
	}
#endif

	if (EfuseDataAddr.RevokeIdAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.RevokeIdAddr,
				(u64)(UINTPTR)&RevokeIds,
				sizeof(RevokeIds));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.RevokeIds = &RevokeIds;
	}

	if (EfuseDataAddr.IvAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.IvAddr,
				(u64)(UINTPTR)&Ivs, sizeof(Ivs));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.Ivs = &Ivs;
	}

	if (EfuseDataAddr.UserFuseAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.UserFuseAddr,
				(u64)(UINTPTR)&UserFusesAddr,
				sizeof(UserFusesAddr));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		UserData.StartUserFuseNum = UserFusesAddr.StartUserFuseNum;
		UserData.NumOfUserFuses = UserFusesAddr.NumOfUserFuses;

		Status = XNvm_EfuseMemCopy(UserFusesAddr.UserFuseDataAddr,
					(u64)(UINTPTR)&UserFuseArr,
				UserData.NumOfUserFuses * XNVM_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		UserData.UserFuseData = UserFuseArr;
		EfuseData.UserFuses = &UserData;
	}

	if (EfuseDataAddr.GlitchCfgAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.GlitchCfgAddr,
				(u64)(UINTPTR)&GlitchData, sizeof(GlitchData));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.GlitchCfgBits = &GlitchData;
	}

	if (EfuseDataAddr.BootEnvCtrlAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.BootEnvCtrlAddr,
				(u64)(UINTPTR)&BootEnvCtrl,
				sizeof(BootEnvCtrl));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.BootEnvCtrl = &BootEnvCtrl;
	}

	if (EfuseDataAddr.Misc1CtrlAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.Misc1CtrlAddr,
				(u64)(UINTPTR)&SecMisc1Bits,
				sizeof(SecMisc1Bits));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.Misc1Bits = &SecMisc1Bits;
	}

	if (EfuseDataAddr.OffChipIdAddr != 0U) {
		Status = XNvm_EfuseMemCopy(EfuseDataAddr.OffChipIdAddr,
				(u64)(UINTPTR)&OffChipIds, sizeof(OffChipIds));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData.OffChipIds = &OffChipIds;
	}

	Status = XNvm_EfuseWrite(&EfuseData);
	if (Status == (int)XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED) {
		Status = XST_SUCCESS;
	}

END:
	ClearStatus = XNvm_ZeroizeAndVerify((u8 *)&AesKeys, sizeof(AesKeys));
	ClearStatusTmp = XNvm_ZeroizeAndVerify((u8 *)&AesKeys, sizeof(AesKeys));
	if ((ClearStatus != XST_SUCCESS) || (ClearStatusTmp != XST_SUCCESS)) {
		Status |= (ClearStatus | ClearStatusTmp);
	}
	return Status;
}

#ifdef XNVM_ACCESS_PUF_USER_DATA
/*****************************************************************************/
/**
 * @brief       This function programs Puf HD eFuses as user eFuses as requested
 * 		by the client with the data provided in given address.
 *
 * @param 	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfusePufFuseAddr structure
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfusePufFuseAddr structure
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfusePufUserDataWrite(u32 AddrLow, u32 AddrHigh)
{
	int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfusePufFuseAddr PufFuseAddr;
	XNvm_EfusePufFuse PufUserFuse;
	u32 PufFusesArr[XNVM_EFUSE_NUM_OF_PUF_FUSES];

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&PufFuseAddr,
			sizeof(PufFuseAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufUserFuse.EnvMonitorDis = PufFuseAddr.EnvMonitorDis;
	PufUserFuse.PrgmPufFuse = PufFuseAddr.PrgmPufFuse;
	PufUserFuse.StartPufFuseRow = PufFuseAddr.StartPufFuseRow;
	PufUserFuse.NumOfPufFusesRows = PufFuseAddr.NumOfPufFusesRows;

	if (PufUserFuse.EnvMonitorDis == TRUE) {
		 PufUserFuse.SysMonInstPtr = NULL;
	}
	else {
		 PufUserFuse.SysMonInstPtr = XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(PufFuseAddr.PufFuseDataAddr,
			(u64)(UINTPTR)&PufFusesArr,
			PufUserFuse.NumOfPufFusesRows * XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	PufUserFuse.PufFuseData = PufFusesArr;

	Status = XNvm_EfuseWritePufAsUserFuses(&PufUserFuse);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the Puf User eFuses requested by the client
 *
 * @param	AddrLow		Lower 32 bit address of the PufUserFuseAddr
 * 				structure
 *
 * @param	AddrHigh	Higher 32 bit address of the PufUserFuseAddr
 *				structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfusePufUserFusesRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfusePufFuseAddr PufFusesAddr;
	u32 PufFusesArr[XNVM_EFUSE_NUM_OF_PUF_FUSES];
	XNvm_EfusePufFuse PufUserFuse;

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&PufFusesAddr,
			sizeof(PufFusesAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufUserFuse.StartPufFuseRow = PufFusesAddr.StartPufFuseRow;
	PufUserFuse.NumOfPufFusesRows = PufFusesAddr.NumOfPufFusesRows;
	PufUserFuse.PufFuseData = PufFusesArr;
	Status = XNvm_EfuseReadPufAsUserFuses(&PufUserFuse);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)PufUserFuse.PufFuseData,
			PufFusesAddr.PufFuseDataAddr,
			PufUserFuse.NumOfPufFusesRows * XNVM_WORD_LEN);

END:
	return Status;
}
#else
/*****************************************************************************/
/**
 * @brief       This function programs Puf helper data requested by the client with
 * 		the data provided in given address.
 *
 * @param 	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfusePufHdAddr structure
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfusePufHdAddr structure
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfusePufWrite(u32 AddrLow, u32 AddrHigh) {
	int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfusePufHdAddr PufHdAddr;
	XNvm_EfusePufHd PufHd;

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&PufHdAddr,
			sizeof(PufHdAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PufHdAddr.PufSecCtrlBits,
				(u64)(UINTPTR)&PufHd.PufSecCtrlBits,
				sizeof(XNvm_EfusePufSecCtrlBits));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufHd.Aux = PufHdAddr.Aux;
	PufHd.Chash = PufHdAddr.Chash;
	PufHd.EnvMonitorDis = (u8)PufHdAddr.EnvMonitorDis;
	PufHd.PrgmPufHelperData = PufHdAddr.PrgmPufHelperData;
	PufHd.PrgmPufSecCtrlBits = PufHdAddr.PrgmPufSecCtrlBits;
	if (PufHd.EnvMonitorDis == TRUE) {
		PufHd.SysMonInstPtr = NULL;
	}
	else {
		PufHd.SysMonInstPtr = XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PufHdAddr.EfuseSynData,
				(u64)(UINTPTR)&PufHd.EfuseSynData,
				XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS *
				XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWritePuf(&PufHd);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads Puf helper data to given address
 *              requested by the client.
 *
 * @param 	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfusePufHdAddr structure
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfusePufHdAddr structure
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfusePufRead(u32 AddrLow, u32 AddrHigh) {
	int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32) | (u64)AddrLow;
	XNvm_EfusePufHdAddr PufHdAddr = {0U};
	XNvm_EfusePufHd PufHd;

	Status = XNvm_EfuseReadPuf(&PufHd);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufHdAddr.Aux = PufHd.Aux;
	PufHdAddr.Chash = PufHd.Chash;

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PufHd.EfuseSynData,
				(u64)(UINTPTR)&PufHdAddr.EfuseSynData,
				XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS *
				XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PufHdAddr, Addr,
				sizeof(PufHdAddr));

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief       This function reads the IV requested by the client
 *
 * @param	IvType	Type of the IV to be read
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseIv structure
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseIv structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseIvRead(XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_Iv Iv;

	Status = XNvm_EfuseReadIv(&Iv, IvType);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&Iv, Addr, sizeof(Iv));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the Revocation ID requested by the client
 *
 * @param	RevokeIdNum	Revocation ID number to be read
 *
 * @param	AddrLow		Lower 32 bit address of the Revocation ID
 *
 * @param	AddrHigh	Higher 32 bit address of the Revocation ID
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseRevocationIdRead(XNvm_RevocationId RevokeIdNum,
	u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	u32 RevocationId;

	Status = XNvm_EfuseReadRevocationId(&RevocationId, RevokeIdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&RevocationId, Addr,
			sizeof(RevocationId));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the User eFuses requested by the client
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseUserDataAddr structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseUserDataAddr structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseUserFusesRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfuseUserDataAddr UserFuses;
	u32 UserFuseData[XNVM_NUM_OF_USER_FUSES];
	XNvm_EfuseUserData UserData;

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&UserFuses,
			sizeof(UserFuses));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UserData.StartUserFuseNum = UserFuses.StartUserFuseNum;
	UserData.NumOfUserFuses = UserFuses.NumOfUserFuses;
	UserData.UserFuseData = UserFuseData;
	Status = XNvm_EfuseReadUserFuses(&UserData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)UserData.UserFuseData,
			UserFuses.UserFuseDataAddr,
			UserData.NumOfUserFuses * XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the PpkHash requested by the client
 *
 * @param	PpkHashType	PpkHash type to be read
 *
 * @param	AddrLow		Lower 32 bit address of the PpkHash array
 *
 * @param	AddrHigh	Higher 32 bit address of the PpkHash array
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfusePpkHashRead(XNvm_PpkType PpkHashType,
		u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_PpkHash PpkHash;

	Status = XNvm_EfuseReadPpkHash(&PpkHash, PpkHashType);
	if (Status != XST_SUCCESS) {
		goto END;
	  }

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PpkHash, Addr,
			sizeof(PpkHash));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the MiscCtrlBits eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseMiscCtrlBits structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseMiscCtrlBits structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseMiscCtrlBitsRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfuseMiscCtrlBits MiscCtrlBits;

	Status = XNvm_EfuseReadMiscCtrlBits(&MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&MiscCtrlBits, Addr,
			sizeof(MiscCtrlBits));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the SecCtrlBits eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseSecCtrlBits structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseSecCtrlBits structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseSecCtrlBitsRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfuseSecCtrlBits SecCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(&SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&SecCtrlBits, Addr,
			sizeof(SecCtrlBits));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the Misc1CtrlBits eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseMisc1Bits structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseMisc1Bits structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseSecMisc1BitsRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfuseSecMisc1Bits SecMisc1Bits;

	Status = XNvm_EfuseReadSecMisc1Bits(&SecMisc1Bits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&SecMisc1Bits, Addr,
			sizeof(SecMisc1Bits));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the BootEnvCtrlBits eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfuseBootEnvCtrlBits structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfuseBootEnvCtrlBits structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseBootEnvCtrlBitsRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfuseBootEnvCtrlBits BootEnvCtrlBits;

	Status = XNvm_EfuseReadBootEnvCtrlBits(&BootEnvCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&BootEnvCtrlBits, Addr,
			sizeof(BootEnvCtrlBits));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the PufSecCtrlBits eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_EfusePufSecCtrlBits structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_EfusePufSecCtrlBits structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfusePufCtrlBitsRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;

	Status = XNvm_EfuseReadPufSecCtrlBits(&PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PufSecCtrlBits, Addr,
			sizeof(PufSecCtrlBits));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the OffChip ID requested by the client
 *
 * @param	IdNum	OffChip ID number to be read
 *
 * @param	AddrLow		Lower 32 bit address of the OffChip ID
 *
 * @param	AddrHigh	Higher 32 bit address of the OffChip ID
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseOffChipIdRead(XNvm_OffchipId IdNum, u32 AddrLow,
		u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	u32 OffChipId;

	Status = XNvm_EfuseReadOffchipRevokeId(&OffChipId, IdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&OffChipId, Addr,
		sizeof(OffChipId));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the DecEfuseOnly eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the DecOnly value
 *
 * @param	AddrHigh	Higher 32 bit address of the DecOnly value
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseDecEfuseOnlyRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	u32 DecOnly;

	Status = XNvm_EfuseReadDecOnly(&DecOnly);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&DecOnly, Addr,
			sizeof(DecOnly));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reads the DNA eFuse cache data
 *
 * @param	AddrLow		Lower 32 bit address of the
 * 				XNvm_Dna structure
 *
 * @param	AddrHigh	Higher 32 bit address of the
 *				XNvm_Dna structure
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseDnaRead(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XNvm_Dna EfuseDna;

	Status = XNvm_EfuseReadDna(&EfuseDna);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&EfuseDna, Addr,
			sizeof(EfuseDna));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies word aligned or non word aligned data
 * 		from source address to destination address.
 *
 * @param 	SourceAddr 	From where the buffer data is read
 *
 * @param 	DestAddr 	To which the buffer data is copied
 *
 * @param 	Len 		Length of data to be copied in bytes
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
static INLINE int XNvm_EfuseMemCopy(u64 SourceAddr, u64 DestAddr, u32 Len)
{
	int Status = XST_FAILURE;

	Status = XPlmi_MemCpy64(DestAddr, SourceAddr, Len);

	return Status;
}

#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE)) || \
	(defined (XNVM_WRITE_USER_EFUSE))
/*****************************************************************************/
/**
 * * @brief	This function is used to program eFuses
 *
 *
 * @param	Cmd - is pointer to the command structure.
 * @param 	AddrLow		Lower 32 bit address of the
 * 				Payload
 * @param	AddrHigh	Higher 32 bit address of the
 *				Payload
 *@param	EnvMonitorDis	Environment monitor disable variable.
 *
 * @return	- XST_SUCCESS - On Specified data write.
 *		- Error Code - On corresponding failure
 *
 *
 ******************************************************************************/
int XNvm_EfuseWriteAccess(const XPlmi_Cmd *Cmd, u32 AddrLow, u32 AddrHigh, u8 EnvMonitorDis)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
#ifdef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
		case XNVM_API_ID_EFUSE_WRITE_IV:
			Status = XNvm_EfuseIvWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC1:
			Status = XNvm_EfuseSecMisc1WriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_ANLG_TRIM:
			Status = XNvm_EfuseAnlgTrimWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL:
			Status = XNvm_EfuseBootEnvCtrlWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_PUF_DATA:
			Status = XNvm_EfusePufWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC0_CTRL:
			Status = XNvm_EfuseSecMisc0WriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEYS):
			Status = XNvm_EfuseAesKeysWriteAccess(Addr, EnvMonitorDis);
			break;
#endif

#ifdef XNVM_WRITE_USER_EFUSE
		case XNVM_API_ID_EFUSE_WRITE_USER_EFUSE:
			Status = XNvm_EfuseUserFuseWriteAccess(Addr, EnvMonitorDis);
			break;
#endif

#ifdef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
		case XNVM_API_ID_EFUSE_WRITE_OFF_CHIP_ID:
			Status = XNvm_EfuseOffChipIdsWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID:
			Status = XNvm_EfuseRevokeIdsWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_PPK_HASH:
			Status = XNvm_EfusePpkhashWriteAccess(Addr, EnvMonitorDis);
			break;
#endif
#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE))
		case XNVM_API_ID_EFUSE_WRITE_MISC_CTRL:
			Status = XNvm_EfuseMiscCtrlWriteAccess(Addr, EnvMonitorDis);
			break;
		case XNVM_API_ID_EFUSE_WRITE_SECURITY_CTRL:
			Status = XNvm_EfuseSecCtrlWriteAccess(Addr, EnvMonitorDis);
			break;
#endif
		default :
			XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function is used to read a specific eFuse
 *
 * @param 	Offset 		Offset of the eFuse
 *
 * @param 	AddrLow		Lower 32 bit address of the eFuse row
 *
 * @param	AddrHigh	Higher 32 bit address of the eFuse row
 *
 * @param 	Size 		Length of data to be read
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
int XNvm_EfuseRead(u32 Offset, u32 AddrLow, u32 AddrHigh, u32 Size)
{
	int Status = XST_FAILURE;
	u64 ReadDataAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XPlmi_VerifyAddrRange(ReadDataAddr, ReadDataAddr + Size - 1U);
        if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
                goto END;
        }

	/*< Offset indicates lower nibble of the eFuse cache row offset */
	Status = XNvm_EfuseReadCacheRange(Offset / XNVM_EFUSE_WORD_LEN,
					  Size / XNVM_EFUSE_WORD_LEN, (u32 *)(UINTPTR)ReadDataAddr);

END:
	return Status;
}

#ifdef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
/*****************************************************************************/
/**
 * @brief	This function is used to write a Iv eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseIvWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseIvs Ivs = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(Ivs) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&Ivs, sizeof(Ivs));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.Ivs = &Ivs;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write SecMisc1Bits eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseSecMisc1WriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecMisc1Bits SecMisc1Bits = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(SecMisc1Bits) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&SecMisc1Bits, sizeof(SecMisc1Bits));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.Misc1Bits = &SecMisc1Bits;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write GlitchCfgBits eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseAnlgTrimWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseGlitchCfgBits GlitchCfgBits = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(GlitchCfgBits) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&GlitchCfgBits, sizeof(GlitchCfgBits));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.GlitchCfgBits = &GlitchCfgBits;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write BootEnvCtrl eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseBootEnvCtrlWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseBootEnvCtrlBits BootEnvCtrl = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(BootEnvCtrl) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&BootEnvCtrl, sizeof(BootEnvCtrl));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.BootEnvCtrl = &BootEnvCtrl;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write puf eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfusePufWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufData PufHdAddr = {0U};
	XNvm_EfusePufHd PufHd = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(PufHdAddr) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&PufHdAddr, sizeof(PufHdAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufHd.Chash = PufHdAddr.Chash;
	PufHd.Aux = PufHdAddr.Aux;
	PufHd.EnvMonitorDis = EfuseData.EnvMonitorDis;
	PufHd.SysMonInstPtr = EfuseData.SysMonInstPtr;
	PufHd.PrgmPufHelperData = TRUE;

	Status = XNvm_EfuseMemCopy((u64)(UINTPTR)&PufHdAddr.EfuseSynData,
				   (u64)(UINTPTR)&PufHd.EfuseSynData,
				   XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS *
				   XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWritePuf(&PufHd);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write SecMisc0 eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseSecMisc0WriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseDecOnly DecOnly = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(DecOnly) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&DecOnly, sizeof(DecOnly));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.DecOnly = &DecOnly;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write Aeskeys eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseAesKeysWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseAesKeys AesKeys = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(AesKeys) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&AesKeys, sizeof(AesKeys));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.AesKeys = &AesKeys;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}
#endif

#ifdef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
/*****************************************************************************/
/**
 * @brief	This function is used to write OffChipIds eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseOffChipIdsWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseOffChipIds OffChipIds = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(OffChipIds) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&OffChipIds, sizeof(OffChipIds));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.OffChipIds = &OffChipIds;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write RevokeIds eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseRevokeIdsWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseRevokeIds RevokeIds = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(RevokeIds) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&RevokeIds, sizeof(RevokeIds));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.RevokeIds = &RevokeIds;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write PpkHash eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfusePpkhashWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfusePpkHash PpkHash = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(PpkHash) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&PpkHash, sizeof(PpkHash));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.PpkHash = &PpkHash;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}
#endif

#ifdef XNVM_WRITE_USER_EFUSE
/*****************************************************************************/
/**
 * @brief	This function is used to write UserFuses eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseUserFuseWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseUserData UserFuses = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(UserFuses) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&UserFuses, sizeof(UserFuses));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.UserFuses = &UserFuses;
	Status = XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}
#endif

#if (defined(XNVM_WRITE_KEY_MANAGEMENT_EFUSE)) || (defined(XNVM_WRITE_SECURITY_CRITICAL_EFUSE))
/*****************************************************************************/
/**
 * @brief	This function is used to write MiscCtrlBits eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseMiscCtrlWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseMiscCtrlBits MiscCtrlBits = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(MiscCtrlBits) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&MiscCtrlBits, sizeof(MiscCtrlBits));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.MiscCtrlBits = &MiscCtrlBits;

#ifndef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
	if ((EfuseData.MiscCtrlBits->Ppk0Invalid != 0U) || (EfuseData.MiscCtrlBits->Ppk1Invalid != 0U) ||
	    (EfuseData.MiscCtrlBits->Ppk2Invalid != 0U)) {
		Status = XNVM_EFUSE_ERROR_EFUSE_ACCESS_DISABLED;
		goto END;
	}
#endif

#ifndef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
	if ((EfuseData.MiscCtrlBits->GlitchDetRomMonitorEn != 0U)
	    || (EfuseData.MiscCtrlBits->HaltBootError != 0U) ||
	    (EfuseData.MiscCtrlBits->HaltBootEnv != 0U) || (EfuseData.MiscCtrlBits->CryptoKatEn != 0U) ||
	    (EfuseData.MiscCtrlBits->LbistEn != 0U) || (EfuseData.MiscCtrlBits->SafetyMissionEn != 0U)) {
		Status = XNVM_EFUSE_ERROR_EFUSE_ACCESS_DISABLED;
		goto END;
	}
#endif
	Status =  XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to write SecCtrlBits eFuse
 *
 * @param 	Addr 	Address of the data to be written
 *
 * @param 	EnvMonitorDis	Environmental Disable variable
 *
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- Error Code - Corresponding error code on failure
 *
 *****************************************************************************/
static int XNvm_EfuseSecCtrlWriteAccess(u64 Addr, u8 EnvMonitorDis)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits SecCtrlBits = {0U};
	XNvm_EfuseData EfuseData = {0U};

	Status = XPlmi_VerifyAddrRange(Addr, Addr + sizeof(SecCtrlBits) - 1U);
	if(Status != XST_SUCCESS) {
		Status = XNVM_EFUSE_ERROR_INVALID_ADDR_RANGE;
		goto END;
	}

	EfuseData.EnvMonitorDis = EnvMonitorDis;
	if (EfuseData.EnvMonitorDis == FALSE) {
		EfuseData.SysMonInstPtr = (XSysMonPsv *)XPlmi_GetSysmonInst();
	}

	Status = XNvm_EfuseMemCopy(Addr, (u64)(UINTPTR)&SecCtrlBits, sizeof(SecCtrlBits));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.SecCtrlBits = &SecCtrlBits;

#ifndef XNVM_WRITE_KEY_MANAGEMENT_EFUSE
	if ((EfuseData.SecCtrlBits->UserKey0CrcLk != 0U) || (EfuseData.SecCtrlBits->UserKey0WrLk != 0U) ||
	    (EfuseData.SecCtrlBits->UserKey1CrcLk != 0U) || (EfuseData.SecCtrlBits->UserKey1WrLk != 0U) ||
	    (EfuseData.SecCtrlBits->AesWrLk != 0U) || (EfuseData.SecCtrlBits->AesCrcLk != 0U) ||
	    (EfuseData.SecCtrlBits->Ppk2WrLk != 0U) || (EfuseData.SecCtrlBits->Ppk1WrLk != 0U) ||
	    (EfuseData.SecCtrlBits->Ppk0WrLk != 0U)) {
		Status = XNVM_EFUSE_ERROR_EFUSE_ACCESS_DISABLED;
		goto END;
	}
#endif

#ifndef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
	if ((EfuseData.SecCtrlBits->AesDis != 0U) || (EfuseData.SecCtrlBits->JtagErrOutDis != 0U) ||
	    (EfuseData.SecCtrlBits->JtagDis != 0U) || (EfuseData.SecCtrlBits->HwTstBitsDis != 0U) ||
	    (EfuseData.SecCtrlBits->SecDbgDis != 0U) || (EfuseData.SecCtrlBits->SecLockDbgDis != 0U) ||
	    (EfuseData.SecCtrlBits->PmcScEn != 0U) || (EfuseData.SecCtrlBits->BootEnvWrLk != 0U) ||
	    (EfuseData.SecCtrlBits->RegInitDis != 0U)) {
		Status = XNVM_EFUSE_ERROR_EFUSE_ACCESS_DISABLED;
		goto END;
	}
#endif

	Status =  XNvm_EfuseWrite(&EfuseData);

END:
	return Status;
}
#endif
#endif
