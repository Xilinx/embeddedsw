/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xaudioformatter_hw.h
* @addtogroup audioformatter_v1_0
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx audio_formatter core.
*
******************************************************************************/

#ifndef XAUDFMT_HW_H_
#define XAUDFMT_HW_H_	/**< Prevent circular inclusions
			  *  by using protection macros	*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

// 0x000 : Core Version register
//         bit [15:8]   - Revision of minor version (Read)
//         bit [23:16]  - IP minor revision value (Read)
//         bit [31:24]  - IP major revision value (Read)
//         others       - reserved
// 0x004 : Core_Configuration Register
//         bit [11:8]   - Max_channels_MM2S (Read)
//         bit 12       - MM2S_packaging_mode (Read)
//         bit [14:13]  - MM2S_Dataformat (Read)
//         bit 15       - MM2S_Included (Read)
//         bit [27:24]  - Max_channels_S2MM (Read)
//         bit 28       - S2MM_packaging_mode (Read)
//         bit [30:29]  - S2MM_Dataformat (Read)
//         bit 31       - S2MM_Included (Read)
//         others       - reserved
// 0x010 : S2MM_Control Register (Read/Write)
//         bit 0        - Run_Stop
//         bit 1        - Reset
//         bit 12       - Err_IrqEn
//         bit 13       - IOC_IrqEn
//         bit 14       - Timeout_IrqEn
//         bit [18:16]  - PCM_data_width
//         bit [22:19]  - No_of_valid_channels
//         others       - reserved
// 0x014 : S2MM Status Register
//         bit 0        - Halt_in_process (Read)
//         bit 17       - S2MM_Slave_Error (Read)
//         bit 18       - S2MM_Decode_Error (Read)
//         bit 19       - Timeout_Error (Read)
//         bit 30       - Err_Irq (Read)
//         bit 31       - IOC_Irq (Read)
//         others       - reserved
// 0x018 : S2MM_Timeout Register (Read/Write)
//         bit [31:0]   - Timeout_counter
// 0x01C : S2MM Period Config  Register (Read/Write)
//         bit [15:0]   - Period_size
//         bit [23:16]  - No_of_periods
//         others       - reserved
// 0x020 : S2MM Buffer Address LSB Register (Read/Write)
//         bit [31:0]   - Buffer_start_address_LSB
// 0x024 : S2MM Buffer Address MSB Register (Read/Write)
//         bit [31:0]   - Buffer_start_address_MSB
// 0x028 : S2MM DMA Transfer Count Register (Read/Write)
//         bit [24:0]   - S2MM DMA Transfer Count
//         others       - reserved
// 0x02C : AES_channel_status_value_0 Register (Read/Write)
//         bit [31:0]   - S2MM Channel Status 1
// 0x030 : AES_channel_status_value_1 Register (Read/Write)
//         bit [31:0]   - S2MM Channel Status 2
// 0x034 : AES_channel_status_value_2 Register (Read/Write)                                                           //         bit [31:0]   - S2MM Channel Status 3
// 0x038 : AES_channel_status_value_3 Register (Read/Write)
//         bit [31:0]   - S2MM Channel Status 4
// 0x03C : AES_channel_status_value_4 Register (Read/Write)
//         bit [31:0]   - S2MM Channel Status 5
// 0x040 : AES_channel_status_value_5 Register (Read/Write)
//         bit [31:0]   - S2MM Channel Status 6
// 0x044 : S2MM_Channel_Offset Register (Read/Write)
//         bit [15:0]   - channel_offset
//         others       - reserved
// 0x110 : MM2S_Control Register (Read/Write)
//         bit 0        - Run_Stop
//         bit 1        - Reset
//         bit 12       - Err_IrqEn
//         bit 13       - IOC_IrqEn
//         bit 14       - Timeout_IrqEn
//         bit [18:16]  - PCM_data_width
//         bit [22:19]  - No_of_valid_channels
//         others       - reserved
// 0x114 : MM2S Status Register
//         bit 0        - Halt_in_process (Read)
//         bit 17       - S2MM_Slave_Error (Read)
//         bit 18       - S2MM_Decode_Error (Read)
//         bit 19       - Timeout_Error (Read)
//         bit 30       - Err_Irq (Read)
//         bit 31       - IOC_Irq (Read)
//         others       - reserved
// 0x118 : MM2S Fs multiplier  (Read/Write)
//         bit [15:0]   - Fs_multiplier_value
// 0x11C : MM2S Period Config  Register (Read/Write)
//         bit [15:0]   - Period_size
//         bit [23:16]  - No_of_periods
//         others       - reserved
// 0x120 : MM2S Buffer Address LSB Register (Read/Write)
//         bit [31:0]   - Buffer_start_address_LSB
// 0x124 : MM2S Buffer Address MSB Register (Read/Write)
//         bit [31:0]   - Buffer_start_address_MSB
// 0x128 : MM2S DMA Transfer Count Register (Read/Write)
//         bit [24:0]   - MM2S DMA Transfer Count
//         others       - reserved
// 0x12C : AES_channel_status_value_0 Register (Read/Write)
//         bit [31:0]   - MM2S Channel Status 1
// 0x130 : AES_channel_status_value_1 Register (Read/Write)
//         bit [31:0]   - MM2S Channel Status 2
// 0x134 : AES_channel_status_value_2 Register (Read/Write)
//         bit [31:0]   - MM2S Channel Status 3
// 0x138 : AES_channel_status_value_3 Register (Read/Write)
//         bit [31:0]   - MM2S Channel Status 4
// 0x13C : AES_channel_status_value_4 Register (Read/Write)
//         bit [31:0]   - MM2S Channel Status 5
// 0x140 : AES_channel_status_value_5 Register (Read/Write)
//         bit [31:0]   - MM2S Channel Status 6
// 0x144 : MM2S_Channel_Offset Register (Read/Write)
//         bit [15:0]   - channel_offset
//         others       - reserved

/* Bit shift */
#define BIT(n)		             (1 << (n))
#define XAUD_FORMATTER_S2MM_OFFSET        0
#define XAUD_FORMATTER_MM2S_OFFSET        0x100

#define XAUD_FORMATTER_CORE_CONFIG    0x4
#define XAUD_FORMATTER_CTRL           0x10
#define XAUD_FORMATTER_STS            0x14

#define XAUD_CTRL_RESET_MASK     BIT(1)
#define XAUD_CFG_MM2S_MASK       BIT(15)
#define XAUD_CFG_S2MM_MASK       BIT(31)

#define XAUD_FORMATTER_S2MM_TIMEOUT  0x18
#define XAUD_FORMATTER_FS_MULTIPLIER 0x118
#define XAUD_FORMATTER_PERIOD_CONFIG 0x1C
#define XAUD_FORMATTER_BUFF_ADDR_LSB 0x20
#define XAUD_FORMATTER_BUFF_ADDR_MSB 0x24
#define XAUD_FORMATTER_XFER_COUNT    0x28
#define XAUD_FORMATTER_CH_STS_START  0x2C
#define XAUD_FORMATTER_BYTES_PER_CH  0x44

#define XAUD_STS_IOC_IRQ_MASK        BIT(31)
#define XAUD_STS_TIMEOUT_IRQ_MASK    BIT(19)
#define XAUD_STS_ERROR_IRQ_MASK      BIT(30)
#define XAUD_CTRL_IOC_IRQ_MASK       BIT(13)
#define XAUD_CTRL_TIMEOUT_IRQ_MASK   BIT(14)
#define XAUD_CTRL_ERR_IRQ_MASK       BIT(12)
#define XAUD_CTRL_DMA_EN_MASK        BIT(0)

#define XAUD_CTRL_DATA_WIDTH_SHIFT       16
#define XAUD_CTRL_ACTIVE_CH_SHIFT        19
#define XAUD_PERIOD_CFG_PERIODS_SHIFT    16

#define XAUD_CHANNELS_MIN            2
#define XAUD_PERIODS_MIN             2
#define XAUD_PERIODS_MAX             8
#define XAUD_PERIOD_BYTES_MIN        64
#define XAUD_PERIOD_BYTES_MAX        (50 * 1024)

/***************** Macros (Inline Functions) Definitions *********************/

#define XAudioFormatter_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XAudioFormatter_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

#ifdef __cplusplus
}

#endif


#endif /* End of protection macro */
/** @} */
