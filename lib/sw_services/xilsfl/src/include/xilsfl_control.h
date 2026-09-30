/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XILSFL_CONTROL_H
#define XILSFL_CONTROL_H

/*****************************************************************************/
/**
 *
 * @file xilsfl_control.h
 * @brief Declares the flash control functions of the serial flash library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.0   sb  9/25/24  Add XSfl_FlashNonBlockingReadProcess API.
 * 1.3   zm  03/09/26 Add support for W25Q128JW Winbond flash.
 *       zm  08/03/26 Add support for MX25U6432F Macronix flash.
 * 1.4   vn  08/26/26 Fix Doxygen warnings.
 * 1.4   vn  09/30/26 Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

/***************************** Include Files *********************************/
#include "xilsfl_flashconfig.h"

/************************** Constant Definitions *****************************/
/**
 * @name CmdBuffer indexes
 * @{
 */
#define XSFL_COMMAND_OFFSET		0 /**< Flash instruction. */
/** MSB byte of address to read or write. */
#define XSFL_ADDRESS_OFFSET	        1
#define XSFL_ADDRESS_SIZE_OFFSET	2 /**< Address size offset index. */
#define XSFL_DUMMY_OFFSET            3 /**< Dummy cycles. */
/**
 * @def XSFL_BYTE_COUNT_OFFSET
 * @brief CmdBuffer index for register read/write byte count.
 */
#define XSFL_BYTE_COUNT_OFFSET       4 /* Number of bytes to read or write */
/** @} */

/**
 * CmdBuffer array length: command, address, address-size, dummy, and
 * byte-count slots.
 */
#define XSFL_BUFFER_ARRAY_LEN        5

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
 *
 * @brief Initializes an XOspiPsv instance so that the driver is ready to use.
 *
 * @param[out]  Ptr Pointer to the XSfl_Interface instance.
 * @param[in]   UserConfig Structure that contains information about a specific
 *              OSPI device. This function initializes an instance object for
 *              the specific device that the contents of UserConfig specify.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if initialization fails.
 *
 ******************************************************************************/
u32 XSfl_OspiInit(XSfl_Interface *Ptr, const XSfl_UserConfig *UserConfig);
u32 XSfl_FlashIdRead(XSfl_Interface *SflInstnacePtr, u8 ChipSelect, u8 *SflReadBuffer );
u32 XSfl_CalculateFCTIndex(u32 ReadId, u32 *FCTIndex);
u32 XSfl_FlashSetSDRDDRMode(XSfl_Interface *SflInstnacePtr, int Mode);
u32 XSfl_FlashEnterExit4BAddMode(XSfl_Interface *SflInstnacePtr, int Enable, u8 ChpiSelect);
u32 XSfl_GetRealAddr(XSfl_Interface *SflInstnacePtr, u32 Address);
u32 XSfl_SectorErase(XSfl_Interface *SflInstnacePtr, u32 Address);
u32 XSfl_FlashPageWrite(XSfl_Interface *SflInstnacePtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr);
u32 XSfl_FlashReadProcess(XSfl_Interface *SflInstnacePtr, u32 Address, u32 ByteCount,
		u8 *ReadBfrPtr, u64 RxAddr64bit);
u32 XSfl_FlashTransferDone(XSfl_Interface *SflInstnacePtr);

u32 XSfl_WaitforStatusDone(XSfl_Interface *SflInstnacePtr);
u32 XSfl_FlashRegisterReadWrite(XSfl_Interface *SflInstnacePtr,
		u8 *RxBfrPtr,u8 *TxBfrPtr, u32 *CmdBufferPtr,u8 Addrvalid);
u32 XSfl_FlashCmdTransfer(XSfl_Interface *SflInstnacePtr,u8 Cmd);
u32 XSfl_GetRealAddr(XSfl_Interface *SflInstnacePtr, u32 Address);
u32 XSfl_FlashNonBlockingReadProcess(XSfl_Interface *SflInstancePtr, u32 Address, u32 ByteCount,
                u8 *ReadBfrPtr, u64 RxAddr64bit);
u32 XSfl_FlashEnableQuadMode(XSfl_Interface *SflInstancePtr, u8 ChipSelNum);
/************************** Variable Definitions *****************************/

/*****************************************************************************/

/** @} */
#endif /* XILSFL_CONTROL_H */
