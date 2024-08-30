/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_control.h
 * @addtogroup xilsfl overview
 * @{
 * @details
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#ifndef XILSFL_CONTROL_H	/**< prevent circular inclusions */
#define XILSFL_CONTROL_H	/**< by using protection macros */

/***************************** Include Files *********************************/
#include "xilsfl_flashconfig.h"

/************************** Constant Definitions *****************************/
#define XSFL_COMMAND_OFFSET		0 /* Flash instruction */
#define XSFL_ADDRESS_OFFSET	        1 /* MSB byte of address to read or write */
#define XSFL_ADDRESS_SIZE_OFFSET	2 /* Address size Offset index */
#define XSFL_DUMMY_OFFSET            3 /* Dummy cycles */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XSfl_OspiInit(XSfl_Interface *Ptr, const XSfl_UserConfig *UserConfig);
u32 XSfl_FlashIdRead(XSfl_Interface *SflInstnacePtr, u8 ChipSelect, u8 *SflReadBuffer );
u32 XSfl_CalculateFCTIndex(u32 ReadId, u32 *FCTIndex);
u32 XSfl_FlashSetSDRDDRMode(XSfl_Interface *SflInstnacePtr, int Mode, u8 *SflReadBuffer);
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
/************************** Variable Definitions *****************************/

/*****************************************************************************/

#endif /* XILSFL_CONTROL_H */
/** @endcond */
/** @} */
