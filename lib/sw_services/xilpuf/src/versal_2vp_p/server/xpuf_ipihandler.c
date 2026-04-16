/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_ipihandler.c
*
* This file contains the XilPuf IPI Handler definition for VERSAL_2VP_P.
* It implements the new PUF IP IPI handler for the VERSAL_2VP_P platform.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  sd  04/13/2026 Initial release
*
* </pre>
*
******************************************************************************/
/**
 * @addtogroup xpuf_server_apis XilPuf Server APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xplmi_plat.h"
#ifdef PLM_PUF
#include "xpuf.h"
#include "xpuf_ipihandler.h"
#include "xpuf_defs.h"
#include "xplmi_dma.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

#define XPUF_SYNDROME_ADDRESS		(0xF2008000U) /**< Address to copy PUF syndrome data
                                                       * when the provided SyndromeAddr is 64-bit */

/************************** Function Prototypes *****************************/
static int XPuf_PufRegistration(u32 SubsystemId, u32 AddrLow, u32 AddrHigh);
static int XPuf_PufRegeneration(u32 SubsystemId, u32 AddrLow, u32 AddrHigh);
static INLINE int XPuf_MemCopy(u64 SourceAddr, u64 DestAddr, u32 Len);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	    Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS - When PUF operation completes successfully
 * 		- XST_INVALID_PARAM - When Cmd or Payload is NULL, or unsupported API ID
 *
 ******************************************************************************/
int XPuf_IpiHandler(const XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_INVALID_PARAM;
	u32 *Pload = NULL;

	/**
	 *  Validate the input parameters. Return XST_FAILURE if input parameters are invalid
	 */
	if (NULL == Cmd) {
		goto END;
	}

	Pload = Cmd->Payload;

	if (NULL == Pload) {
		goto END;
	}

	/**
	 *  Calls the respective handler based on the API Id
	 */
	switch (Cmd->CmdId & XPUF_API_ID_MASK) {
		case XPUF_API(XPUF_PUF_REGISTRATION):
			Status = XPuf_PufRegistration(Cmd->SubsystemId, Pload[0], Pload[1]);
			break;
		case XPUF_API(XPUF_PUF_REGENERATION):
			Status = XPuf_PufRegeneration(Cmd->SubsystemId, Pload[0], Pload[1]);
			break;
		case XPUF_API(XPUF_PUF_CLEAR_PUF_ID):
			Status = XPuf_ClearPufID();
			break;
		default:
			XPuf_Printf(XPUF_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function performs PUF registration (enrollment) for VERSAL_2VP_P.
 *          No GlobalVarFilter, AUX, or EfuseSynData for this platform.
 *
 * @param 	SubsystemId	Subsystem ID.
 * @param 	AddrLow		Lower 32 bit address of the XPuf_DataAddr structure
 * @param	AddrHigh	Higher 32 bit address of the XPuf_DataAddr structure
 *
 * @return
 * 		- XST_SUCCESS - When PUF registration completes successfully
 * 		- XST_FAILURE - When any operation in the registration process fails
 *
 ******************************************************************************/
static int XPuf_PufRegistration(u32 SubsystemId, u32 AddrLow, u32 AddrHigh) {
	int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XPuf_DataAddr PufDataAddr __attribute__((aligned(32U))) = {0U};
	XPuf_Data PufData __attribute__((aligned(32U))) = {0U};

	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Addr, sizeof(PufDataAddr), Status, XPUF_ERR_INVALID_ADDR_RANGE, END);

	/**
	 * Copy user configured puf structure to local PufDataAddr structure.
	 */
	Status = XPuf_MemCopy(Addr, (u64)(UINTPTR)&PufDataAddr,
			sizeof(PufDataAddr));
	if  (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Validate internal address fields in the copied structure
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, PufDataAddr.SyndromeDataAddr, XPUF_SYN_LEN_IN_BYTES, Status, XPUF_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, PufDataAddr.ChashAddr, XPUF_WORD_LENGTH, Status, XPUF_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, PufDataAddr.PufIDAddr, XPUF_ID_LEN_IN_BYTES, Status, XPUF_ERR_INVALID_ADDR_RANGE, END);

	/**
	 * Point user configured data from PufDataAddr to PufData structure.
	 * No GlobalVarFilter for VERSAL_2VP_P.
	 */
	PufData.ShutterValue = PufDataAddr.ShutterValue;
	PufData.PufOperation = PufDataAddr.PufOperation;
	PufData.RoSwapVal = PufDataAddr.RoSwapVal;

	/**
	 * Call to server puf registration function.
	 */
	Status = XPuf_Registration(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Copy syndrome data from PufData structure to PufDataAddr structure.
	 * For VERSAL_2VP_P, syndrome length is XPUF_SYN_LEN_IN_BYTES.
	 */
	Status = XPuf_MemCopy((u64)(UINTPTR)&PufData.SyndromeData,
		PufDataAddr.SyndromeDataAddr,
		XPUF_SYN_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Copy Puf Id from PufData structure to PufDataAddr structure.
	 */
	Status = XPuf_MemCopy((u64)(UINTPTR)&PufData.PufID,
		PufDataAddr.PufIDAddr, XPUF_ID_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Write CHash.
	 * No AuxAddr or EfuseSynData for VERSAL_2VP_P.
	 */
	XPlmi_Out64(PufDataAddr.ChashAddr, PufData.Chash);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function performs PUF regeneration for VERSAL_2VP_P.
 *          No GlobalVarFilter or AUX data for this platform.
 *
 * @param 	SubsystemId	Subsystem ID.
 * @param 	AddrLow		Lower 32 bit address of the XPuf_DataAddr structure
 * @param	AddrHigh	Higher 32 bit address of the XPuf_DataAddr structure
 *
 * @return
 * 		- XST_SUCCESS - When PUF regeneration completes successfully
 * 		- XST_FAILURE - When any operation in the regeneration process fails
 *
 ******************************************************************************/
static int XPuf_PufRegeneration(u32 SubsystemId, u32 AddrLow, u32 AddrHigh) {
	int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XPuf_DataAddr PufDataAddr __attribute__((aligned(32U))) = {0U};
	XPuf_Data PufData __attribute__((aligned(32U))) = {0U};

	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Addr, sizeof(PufDataAddr), Status, XPUF_ERR_INVALID_ADDR_RANGE, END);

	/**
	 * Copy user configured puf structure to local PufDataAddr structure.
	 */
	Status = XPuf_MemCopy(Addr, (u64)(UINTPTR)&PufDataAddr, sizeof(PufDataAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Validate internal address fields in the copied structure
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, PufDataAddr.ChashAddr, XPUF_WORD_LENGTH, Status, XPUF_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, PufDataAddr.PufIDAddr, XPUF_ID_LEN_IN_BYTES, Status, XPUF_ERR_INVALID_ADDR_RANGE, END);

	/* Validate syndrome address only if reading from RAM */
	if (PufDataAddr.ReadOption == XPUF_READ_FROM_RAM) {
		XPLMI_VERIFY_ADDR_RANGE(SubsystemId, PufDataAddr.SyndromeAddr, XPUF_SYN_LEN_IN_BYTES, Status, XPUF_ERR_INVALID_ADDR_RANGE, END);
	}

	/**
	 * Point user configured data from PufDataAddr to PufData structure.
	 * No GlobalVarFilter or AUX data for VERSAL_2VP_P.
	 */
	PufData.ShutterValue = PufDataAddr.ShutterValue;
	PufData.PufOperation = PufDataAddr.PufOperation;
	PufData.ReadOption = (XPuf_ReadOption)PufDataAddr.ReadOption;
	PufData.Chash = XPlmi_In64(PufDataAddr.ChashAddr);
	PufData.RoSwapVal = PufDataAddr.RoSwapVal;

	/**
	 * If higher address of syndrome data is non-zero then copy syndrome data at XPUF_SYNDROME_ADDRESS.
	 */
	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		if ((PufDataAddr.SyndromeAddr >> 32) != 0U) {
			Status = XPuf_MemCopy((u64)PufDataAddr.SyndromeAddr,
				(u64)XPUF_SYNDROME_ADDRESS,
				XPUF_SYN_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			PufData.SyndromeAddr = XPUF_SYNDROME_ADDRESS;
		}
		else{
			PufData.SyndromeAddr = (u32)PufDataAddr.SyndromeAddr;
		}
	}

	/**
	 * Call to server puf regeneration function.
	 */
	Status = XPuf_Regeneration(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Copy Puf Id from PufData structure to PufDataAddr structure.
	 */
	Status = XPuf_MemCopy((u64)(UINTPTR)&PufData.PufID,
		PufDataAddr.PufIDAddr, XPUF_ID_LEN_IN_BYTES);

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
 * @return
 * 		- XST_SUCCESS - If the copy is successful
 * 		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
static INLINE int XPuf_MemCopy(u64 SourceAddr, u64 DestAddr, u32 Len)
{
	int Status = XST_FAILURE;

	Status = XPlmi_MemCpy64(DestAddr, SourceAddr, Len);

	return Status;
}
#endif
/** @} */
