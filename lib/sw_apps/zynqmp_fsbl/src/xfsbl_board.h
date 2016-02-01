/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_board.h
*
* This is the header file which contains board specific definitions for FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ssc  01/20/16 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_BOARD_H
#define XFSBL_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#ifdef XPS_BOARD_ZCU102
#include "xiicps.h"
/************************** Constant Definitions *****************************/
#define SERDES_ICM_CFG0_L0_ICM_CFG_MASK	0x00000007U
#define SERDES_ICM_CFG0_L1_ICM_CFG_MASK	0x00000070U
#define SERDES_ICM_CFG1_L2_ICM_CFG_MASK	0x00000007U
#define SERDES_ICM_CFG1_L3_ICM_CFG_MASK	0x00000070U

#define GPIO_MIO31_MASK	0x00000020U

#define CRL_APB_BOOTMODE_1_HI 	0x00000202U
#define CRL_APB_BOOTMODE_1_LO 	0x00000002U

#define ICM_CFG0_PCIE_PCIE	0x00000011U
#define ICM_CFG1_PCIE_PCIE	0x00000011U
#define ICM_CFG0_PCIE_DP	0x00000041U
#define ICM_CFG1_USB_SATA	0x00000023U
#define ICM_CFG0_DP_DP		0x00000044U

#define IIC_SCLK_RATE_IOEXP		400000
#define IIC_SCLK_RATE_I2CMUX 	600000

#define IOEXPANDER1_ADDR		0x20U
#define PCA9544A_ADDR			0x75U
#define MAX15301_ADDR			0x18U

#define BUF_LEN		10U

#define CMD_CFG_0_REG		0x06U
#define CMD_OUTPUT_0_REG	0x02U
#define DATA_OUTPUT			0x0U

/**
 * DATA_COMMON_CFG:
 * GEM3_EXP_RESET_B is High (1)
 * IIC_MUX_RESET_B is High (1)
 * PCIE_CLK_DIR_SEL is High (0) - PCIe RC
 */
#define DATA_COMMON_CFG		0xE0U

#define DATA_GT_0000_CFG	0x00U
#define DATA_GT_1100_CFG	0x0CU
#define DATA_GT_1110_CFG	0x0EU
#define DATA_GT_1111_CFG	0x0FU

#define CMD_CH_2_REG		0x06U
#define CMD_ON_OFF_CFG		0x02U
#define ON_OFF_CFG_VAL		0x16U

#define DELAY_1_US			0x1U
#define DELAY_5_US			0x5U



/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#endif
u32 XFsbl_BoardInit(void);

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_BOARD_H */
