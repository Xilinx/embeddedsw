/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*                     Added ZCU106 support
* 3.0	bkm	 18/4/18  Added Board specific code w.r.t VADJ
*
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
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)		\
		|| defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111)
#include "xiicps.h"
/************************** Constant Definitions *****************************/
#define GPIO_MIO31_MASK	0x00000020U

#define IIC_SCLK_RATE_IOEXP		400000U
#define IIC_SCLK_RATE_I2CMUX 	600000U

#define IOEXPANDER1_ADDR		0x20U
#define PCA9544A_ADDR			0x75U
#define MAX15301_ADDR			0x18U
#define TCA9548A_ADDR			0x74U
#define IRPS5401_ADDR			0x44U
#define IRPS5401_SWC_ADDR		0x45U

#define BUF_LEN		10U
#define MAX_SIZE		32U

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

#define DATA_GT_L0_DP_CFG	0x1U
#define DATA_GT_L1_DP_CFG	0x2U
#define DATA_GT_L2_USB_CFG	0x4U
#define DATA_GT_L3_SATA_CFG	0x8U

#define ICM_CFG_VAL_PWRDN	0X0U
#define ICM_CFG_VAL_PCIE	0X1U
#define ICM_CFG_VAL_SATA	0X2U
#define ICM_CFG_VAL_USB		0X3U
#define ICM_CFG_VAL_DP		0X4U

#define NUM_GT_LANES		4U

#define CMD_CH_2_REG		0x06U
#define CMD_ON_OFF_CFG		0x02U
#define ON_OFF_CFG_VAL		0x16U

#define DELAY_1_US			0x1U
#define DELAY_5_US			0x5U

#define CMD_CH_2_REG_IRPS	0x04U
#define CMD_PAGE_CFG		0x00U
#define DATA_SWC_CFG		0x02U
#define DATA_SWD_CFG		0x03U
#define CMD_OPERATION_CFG	0x01U
#define OPERATION_VAL		0x80U
#define ON_OFF_CFG_VAL_IRPS 0x1AU
#define CMD_VOUT_MODE_CFG		0x20U
#define DATA_VOUT_MODE_VAL		0x14U
#define CMD_VOUT_MAX_CFG		0x24U
#define DATA_VOUT_MAX_VAL_L		0x00U
#define DATA_VOUT_MAX_VAL_H		0x80U
#define CMD_VOUT_CMD_CFG		0x21U
#define CMD_VOUT_UV_WARN_LIMIT	0x43U
#define CMD_VOUT_UV_FAULT_LIMIT	0x44U
#define CMD_VOUT_OV_WARN_LIMIT	0x42U
#define CMD_VOUT_OV_FAULT_LIMIT	0x40U

#define MULTIRECORD_HEADER_SIZE				0x5U
#define DC_LOAD								0x2U
#define SET_VADJ_0V0						0x0U
#define SET_VADJ_1V2						0x1U
#define SET_VADJ_1V5						0x2U
#define SET_VADJ_1V8						0x3U
/** Vout values for 1.2V output voltage */
#define VOUT_CMDL_1V2						0x33U
#define VOUT_CMDH_1V2						0x13U
#define VOUT_OV_WARNL_1V2					0xCDU
#define VOUT_OV_WARNH_1V2					0x15U
#define VOUT_OV_FAULTL_1V2					0x66U
#define VOUT_OV_FAULTH_1V2					0x16U
#define VOUT_UV_WARNL_1V2					0xCDU
#define VOUT_UV_WARNH_1V2					0x10U
#define VOUT_UV_FAULTL_1V2					0x00U
#define VOUT_UV_FAULTH_1V2					0x10U
/** Vout values for 1.5V output voltage */
#define VOUT_CMDL_1V5						0x00U
#define VOUT_CMDH_1V5						0x18U
#define VOUT_OV_WARNL_1V5					0x66U
#define VOUT_OV_WARNH_1V5					0x1AU
#define VOUT_OV_FAULTL_1V5					0x33U
#define VOUT_OV_FAULTH_1V5					0x1BU
#define VOUT_UV_WARNL_1V5					0x9AU
#define VOUT_UV_WARNH_1V5					0x15U
#define VOUT_UV_FAULTL_1V5					0xCDU
#define VOUT_UV_FAULTH_1V5					0x14U
/** Vout values for 1.8V output voltage */
#define VOUT_CMDL_1V8						0xCDU
#define VOUT_CMDH_1V8						0x1CU
#define VOUT_OV_WARNL_1V8					0x33U
#define VOUT_OV_WARNH_1V8					0x1FU
#define VOUT_OV_FAULTL_1V8					0x00U
#define VOUT_OV_FAULTH_1V8					0x20U
#define VOUT_UV_WARNL_1V8					0x66U
#define VOUT_UV_WARNH_1V8					0x1AU
#define VOUT_UV_FAULTL_1V8					0x33U
#define VOUT_UV_FAULTH_1V8					0x1BU

/**************************** Type Definitions *******************************/
typedef struct {
	u32 VadjRecordFound;
	u32 MultirecordHdrOff;
	u32 MultirecordHdrEol;
	u32 RecordType;
	u32 RecordLength;
	u32 VadjHdrOffset;
	u32 VadjDataOffset;
	u32 OutputNumber;
} XMultipleRecord;

typedef struct XVoutCommands {
	u32 VoutCmdL;
	u32 VoutCmdH;
	u32 VoutOvWarnL;
	u32 VoutOvWarnH;
	u32 VoutOvFaultL;
	u32 VoutOvFaultH;
	u32 VoutUvWarnL;
	u32 VoutUvWarnH;
	u32 VoutUvFaultL;
	u32 VoutUvFaultH;
} XVoutCommands;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#endif
u32 XFsbl_BoardInit(void);

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_BOARD_H */
