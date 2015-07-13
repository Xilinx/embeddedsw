/******************************************************************************
*
* Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xsdps_hw.h
* @addtogroup sdps_v2_5
* @{
*
* This header file contains the identifiers and basic HW access driver
* functions (or  macros) that can be used to access the device. Other driver
* functions are defined in xsdps.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.00a hk/sg  10/17/13 Initial release
* 2.5 	sg	   07/09/15 Added SD 3.0 features
* </pre>
*
******************************************************************************/

#ifndef SD_HW_H_
#define SD_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets from the base address of an SD device.
 * @{
 */

#define XSDPS_SDMA_SYS_ADDR_OFFSET	0x00	/**< SDMA System Address
							Register */
#define XSDPS_SDMA_SYS_ADDR_LO_OFFSET	XSDPS_SDMA_SYS_ADDR_OFFSET
						/**< SDMA System Address
							Low Register */
#define XSDPS_ARGMT2_LO_OFFSET		0x00	/**< Argument2 Low Register */
#define XSDPS_SDMA_SYS_ADDR_HI_OFFSET	0x02	/**< SDMA System Address
							High Register */
#define XSDPS_ARGMT2_HI_OFFSET		0x02	/**< Argument2 High Register */

#define XSDPS_BLK_SIZE_OFFSET		0x04	/**< Block Size Register */
#define XSDPS_BLK_CNT_OFFSET		0x06	/**< Block Count Register */
#define XSDPS_ARGMT_OFFSET		0x08	/**< Argument Register */
#define XSDPS_ARGMT1_LO_OFFSET		XSDPS_ARGMT_OFFSET
						/**< Argument1 Register */
#define XSDPS_ARGMT1_HI_OFFSET		0x0A	/**< Argument1 Register */

#define XSDPS_XFER_MODE_OFFSET		0x0C	/**< Transfer Mode Register */
#define XSDPS_CMD_OFFSET		0x0E	/**< Command Register */
#define XSDPS_RESP0_OFFSET		0x10	/**< Response0 Register */
#define XSDPS_RESP1_OFFSET		0x14	/**< Response1 Register */
#define XSDPS_RESP2_OFFSET		0x18	/**< Response2 Register */
#define XSDPS_RESP3_OFFSET		0x1C	/**< Response3 Register */
#define XSDPS_BUF_DAT_PORT_OFFSET	0x20	/**< Buffer Data Port */
#define XSDPS_PRES_STATE_OFFSET		0x24	/**< Present State */
#define XSDPS_HOST_CTRL1_OFFSET		0x28	/**< Host Control 1 */
#define XSDPS_POWER_CTRL_OFFSET		0x29	/**< Power Control */
#define XSDPS_BLK_GAP_CTRL_OFFSET	0x2A	/**< Block Gap Control */
#define XSDPS_WAKE_UP_CTRL_OFFSET	0x2B	/**< Wake Up Control */
#define XSDPS_CLK_CTRL_OFFSET		0x2C	/**< Clock Control */
#define XSDPS_TIMEOUT_CTRL_OFFSET	0x2E	/**< Timeout Control */
#define XSDPS_SW_RST_OFFSET		0x2F	/**< Software Reset */
#define XSDPS_NORM_INTR_STS_OFFSET 	0x30	/**< Normal Interrupt
							Status Register */
#define XSDPS_ERR_INTR_STS_OFFSET 	0x32	/**< Error Interrupt
							Status Register */
#define XSDPS_NORM_INTR_STS_EN_OFFSET	0x34	/**< Normal Interrupt
						Status Enable Register */
#define XSDPS_ERR_INTR_STS_EN_OFFSET	0x36	/**< Error Interrupt
						Status Enable Register */
#define XSDPS_NORM_INTR_SIG_EN_OFFSET	0x38	/**< Normal Interrupt
						Signal Enable Register */
#define XSDPS_ERR_INTR_SIG_EN_OFFSET	0x3A	/**< Error Interrupt
						Signal Enable Register */

#define XSDPS_AUTO_CMD12_ERR_STS_OFFSET	0x3C	/**< Auto CMD12 Error Status
							Register */
#define XSDPS_HOST_CTRL2_OFFSET		0x3E	/**< Host Control2 Register */
#define XSDPS_CAPS_OFFSET 		0x40	/**< Capabilities Register */
#define XSDPS_CAPS_EXT_OFFSET 		0x44	/**< Capabilities Extended */
#define XSDPS_MAX_CURR_CAPS_OFFSET	0x48	/**< Maximum Current
						Capabilities Register */
#define XSDPS_MAX_CURR_CAPS_EXT_OFFSET	0x4C	/**< Maximum Current
						Capabilities Ext Register */
#define XSDPS_FE_ERR_INT_STS_OFFSET	0x52	/**< Force Event for
						Error Interrupt Status */
#define XSDPS_FE_AUTO_CMD12_EIS_OFFSET	0x50	/**< Auto CM12 Error Interrupt
							Status Register */
#define XSDPS_ADMA_ERR_STS_OFFSET	0x54	/**< ADMA Error Status
							Register */
#define XSDPS_ADMA_SAR_OFFSET		0x58	/**< ADMA System Address
							Register */
#define XSDPS_ADMA_SAR_EXT_OFFSET	0x5C	/**< ADMA System Address
							Extended Register */
#define XSDPS_PRE_VAL_1_OFFSET		0x60	/**< Preset Value Register */
#define XSDPS_PRE_VAL_2_OFFSET		0x64	/**< Preset Value Register */
#define XSDPS_PRE_VAL_3_OFFSET		0x68	/**< Preset Value Register */
#define XSDPS_PRE_VAL_4_OFFSET		0x6C	/**< Preset Value Register */
#define XSDPS_BOOT_TOUT_CTRL_OFFSET	0x70	/**< Boot timeout control
							register */

#define XSDPS_SHARED_BUS_CTRL_OFFSET	0xE0	/**< Shared Bus Control
							Register */
#define XSDPS_SLOT_INTR_STS_OFFSET	0xFC	/**< Slot Interrupt Status
							Register */
#define XSDPS_HOST_CTRL_VER_OFFSET	0xFE	/**< Host Controller Version
							Register */

/* @} */

/** @name Control Register - Host control, Power control,
 * 			Block Gap control and Wakeup control
 *
 * This register contains bits for various configuration options of
 * the SD host controller. Read/Write apart from the reserved bits.
 * @{
 */

#define XSDPS_HC_LED_MASK		0x00000001 /**< LED Control */
#define XSDPS_HC_WIDTH_MASK		0x00000002 /**< Bus width */
#define XSDPS_HC_BUS_WIDTH_4		0x00000002
#define XSDPS_HC_SPEED_MASK		0x00000004 /**< High Speed */
#define XSDPS_HC_DMA_MASK		0x00000018 /**< DMA Mode Select */
#define XSDPS_HC_DMA_SDMA_MASK		0x00000000 /**< SDMA Mode */
#define XSDPS_HC_DMA_ADMA1_MASK		0x00000008 /**< ADMA1 Mode */
#define XSDPS_HC_DMA_ADMA2_32_MASK	0x00000010 /**< ADMA2 Mode - 32 bit */
#define XSDPS_HC_DMA_ADMA2_64_MASK	0x00000018 /**< ADMA2 Mode - 64 bit */
#define XSDPS_HC_EXT_BUS_WIDTH		0x00000020 /**< Bus width - 8 bit */
#define XSDPS_HC_CARD_DET_TL_MASK	0x00000040 /**< Card Detect Tst Lvl */
#define XSDPS_HC_CARD_DET_SD_MASK	0x00000080 /**< Card Detect Sig Det */

#define XSDPS_PC_BUS_PWR_MASK		0x00000001 /**< Bus Power Control */
#define XSDPS_PC_BUS_VSEL_MASK		0x0000000E /**< Bus Voltage Select */
#define XSDPS_PC_BUS_VSEL_3V3_MASK	0x0000000E /**< Bus Voltage 3.3V */
#define XSDPS_PC_BUS_VSEL_3V0_MASK	0x0000000C /**< Bus Voltage 3.0V */
#define XSDPS_PC_BUS_VSEL_1V8_MASK	0x0000000A /**< Bus Voltage 1.8V */
#define XSDPS_PC_EMMC_HW_RST_MASK	0x00000010 /**< HW reset for eMMC */

#define XSDPS_BGC_STP_REQ_MASK		0x00000001 /**< Block Gap Stop Req */
#define XSDPS_BGC_CNT_REQ_MASK		0x00000002 /**< Block Gap Cont Req */
#define XSDPS_BGC_RWC_MASK		0x00000004 /**< Block Gap Rd Wait */
#define XSDPS_BGC_INTR_MASK		0x00000008 /**< Block Gap Intr */
#define XSDPS_BGC_SPI_MODE_MASK		0x00000010 /**< Block Gap SPI Mode */
#define XSDPS_BGC_BOOT_EN_MASK		0x00000020 /**< Block Gap Boot Enb */
#define XSDPS_BGC_ALT_BOOT_EN_MASK	0x00000040 /**< Block Gap Alt BootEn */
#define XSDPS_BGC_BOOT_ACK_MASK		0x00000080 /**< Block Gap Boot Ack */

#define XSDPS_WC_WUP_ON_INTR_MASK	0x00000001 /**< Wakeup Card Intr */
#define XSDPS_WC_WUP_ON_INSRT_MASK	0x00000002 /**< Wakeup Card Insert */
#define XSDPS_WC_WUP_ON_REM_MASK	0x00000004 /**< Wakeup Card Removal */

/* @} */

/** @name Control Register - Clock control, Timeout control & Software reset
 *
 * This register contains bits for configuration options of clock, timeout and
 * software reset.
 * Read/Write except for Inter_Clock_Stable bit (read only) and reserved bits.
 * @{
 */

#define XSDPS_CC_INT_CLK_EN_MASK		0x00000001
#define XSDPS_CC_INT_CLK_STABLE_MASK	0x00000002
#define XSDPS_CC_SD_CLK_EN_MASK			0x00000004
#define XSDPS_CC_SD_CLK_GEN_SEL_MASK		0x00000020
#define XSDPS_CC_SDCLK_FREQ_SEL_EXT_MASK	0x000000C0
#define XSDPS_CC_SDCLK_FREQ_SEL_MASK		0x0000FF00
#define XSDPS_CC_SDCLK_FREQ_D256_MASK		0x00008000
#define XSDPS_CC_SDCLK_FREQ_D128_MASK		0x00004000
#define XSDPS_CC_SDCLK_FREQ_D64_MASK		0x00002000
#define XSDPS_CC_SDCLK_FREQ_D32_MASK		0x00001000
#define XSDPS_CC_SDCLK_FREQ_D16_MASK		0x00000800
#define XSDPS_CC_SDCLK_FREQ_D8_MASK		0x00000400
#define XSDPS_CC_SDCLK_FREQ_D4_MASK		0x00000200
#define XSDPS_CC_SDCLK_FREQ_D2_MASK		0x00000100
#define XSDPS_CC_SDCLK_FREQ_BASE_MASK	0x00000000
#define XSDPS_CC_MAX_DIV_CNT			256
#define XSDPS_CC_EXT_MAX_DIV_CNT		2046
#define XSDPS_CC_EXT_DIV_SHIFT			6

#define XSDPS_TC_CNTR_VAL_MASK			0x0000000F

#define XSDPS_SWRST_ALL_MASK			0x00000001
#define XSDPS_SWRST_CMD_LINE_MASK		0x00000002
#define XSDPS_SWRST_DAT_LINE_MASK		0x00000004

#define XSDPS_CC_MAX_NUM_OF_DIV		9
#define XSDPS_CC_DIV_SHIFT		8

/* @} */

/** @name SD Interrupt Registers
 *
 * <b> Normal and Error Interrupt Status Register </b>
 * This register shows the normal and error interrupt status.
 * Status enable register affects reads of this register.
 * If Signal enable register is set and the corresponding status bit is set,
 * interrupt is generated.
 * Write to clear except
 * Error_interrupt and Card_Interrupt bits - Read only
 *
 * <b> Normal and Error Interrupt Status Enable Register </b>
 * Setting this register bits enables Interrupt status.
 * Read/Write except Fixed_to_0 bit (Read only)
 *
 * <b> Normal and Error Interrupt Signal Enable Register </b>
 * This register is used to select which interrupt status is
 * indicated to the Host System as the interrupt.
 * Read/Write except Fixed_to_0 bit (Read only)
 *
 * All three registers have same bit definitions
 * @{
 */

#define XSDPS_INTR_CC_MASK		0x00000001 /**< Command Complete */
#define XSDPS_INTR_TC_MASK		0x00000002 /**< Transfer Complete */
#define XSDPS_INTR_BGE_MASK		0x00000004 /**< Block Gap Event */
#define XSDPS_INTR_DMA_MASK		0x00000008 /**< DMA Interrupt */
#define XSDPS_INTR_BWR_MASK		0x00000010 /**< Buffer Write Ready */
#define XSDPS_INTR_BRR_MASK		0x00000020 /**< Buffer Read Ready */
#define XSDPS_INTR_CARD_INSRT_MASK	0x00000040 /**< Card Insert */
#define XSDPS_INTR_CARD_REM_MASK	0x00000080 /**< Card Remove */
#define XSDPS_INTR_CARD_MASK		0x00000100 /**< Card Interrupt */
#define XSDPS_INTR_INT_A_MASK		0x00000200 /**< INT A Interrupt */
#define XSDPS_INTR_INT_B_MASK		0x00000400 /**< INT B Interrupt */
#define XSDPS_INTR_INT_C_MASK		0x00000800 /**< INT C Interrupt */
#define XSDPS_INTR_RE_TUNING_MASK	0x00001000 /**< Re-Tuning Interrupt */
#define XSDPS_INTR_BOOT_ACK_RECV_MASK	0x00002000 /**< Boot Ack Recv
							Interrupt */
#define XSDPS_INTR_BOOT_TERM_MASK	0x00004000 /**< Boot Terminate
							Interrupt */
#define XSDPS_INTR_ERR_MASK		0x00008000 /**< Error Interrupt */
#define XSDPS_NORM_INTR_ALL_MASK	0x0000FFFF

#define XSDPS_INTR_ERR_CT_MASK		0x00000001 /**< Command Timeout
							Error */
#define XSDPS_INTR_ERR_CCRC_MASK	0x00000002 /**< Command CRC Error */
#define XSDPS_INTR_ERR_CEB_MASK		0x00000004 /**< Command End Bit
							Error */
#define XSDPS_INTR_ERR_CI_MASK		0x00000008 /**< Command Index Error */
#define XSDPS_INTR_ERR_DT_MASK		0x00000010 /**< Data Timeout Error */
#define XSDPS_INTR_ERR_DCRC_MASK	0x00000020 /**< Data CRC Error */
#define XSDPS_INTR_ERR_DEB_MASK		0x00000040 /**< Data End Bit Error */
#define XSDPS_INTR_ERR_CUR_LMT_MASK	0x00000080 /**< Current Limit Error */
#define XSDPS_INTR_ERR_AUTO_CMD12_MASK	0x00000100 /**< Auto CMD12 Error */
#define XSDPS_INTR_ERR_ADMA_MASK	0x00000200 /**< ADMA Error */
#define XSDPS_INTR_ERR_TR_MASK		0x00001000 /**< Tuning Error */
#define XSDPS_INTR_VEND_SPF_ERR_MASK	0x0000E000 /**< Vendor Specific
							Error */
#define XSDPS_ERROR_INTR_ALL_MASK	0x0000F3FF /**< Mask for error bits */
/* @} */

/** @name Block Size and Block Count Register
 *
 * This register contains the block count for current transfer,
 * block size and SDMA buffer size.
 * Read/Write except for reserved bits.
 * @{
 */

#define XSDPS_BLK_SIZE_MASK		0x00000FFF /**< Transfer Block Size */
#define XSDPS_SDMA_BUFF_SIZE_MASK	0x00007000 /**< Host SDMA Buffer Size */
#define XSDPS_BLK_SIZE_1024		0x400
#define XSDPS_BLK_SIZE_2048		0x800
#define XSDPS_BLK_CNT_MASK		0x0000FFFF /**< Block Count for
								Current Transfer */

/* @} */

/** @name Transfer Mode and Command Register
 *
 * The Transfer Mode register is used to control the data transfers and
 * Command register is used for command generation
 * Read/Write except for reserved bits.
 * @{
 */

#define XSDPS_TM_DMA_EN_MASK		0x00000001 /**< DMA Enable */
#define XSDPS_TM_BLK_CNT_EN_MASK	0x00000002 /**< Block Count Enable */
#define XSDPS_TM_AUTO_CMD12_EN_MASK	0x00000004 /**< Auto CMD12 Enable */
#define XSDPS_TM_DAT_DIR_SEL_MASK	0x00000010 /**< Data Transfer
							Direction Select */
#define XSDPS_TM_MUL_SIN_BLK_SEL_MASK	0x00000020 /**< Multi/Single
							Block Select */

#define XSDPS_CMD_RESP_SEL_MASK		0x00000003 /**< Response Type
							Select */
#define XSDPS_CMD_RESP_NONE_MASK	0x00000000 /**< No Response */
#define XSDPS_CMD_RESP_L136_MASK	0x00000001 /**< Response length 138 */
#define XSDPS_CMD_RESP_L48_MASK		0x00000002 /**< Response length 48 */
#define XSDPS_CMD_RESP_L48_BSY_CHK_MASK	0x00000003 /**< Response length 48 &
							check busy after
							response */
#define XSDPS_CMD_CRC_CHK_EN_MASK	0x00000008 /**< Command CRC Check
							Enable */
#define XSDPS_CMD_INX_CHK_EN_MASK	0x00000010 /**< Command Index Check
							Enable */
#define XSDPS_DAT_PRESENT_SEL_MASK	0x00000020 /**< Data Present Select */
#define XSDPS_CMD_TYPE_MASK		0x000000C0 /**< Command Type */
#define XSDPS_CMD_TYPE_NORM_MASK	0x00000000 /**< CMD Type - Normal */
#define XSDPS_CMD_TYPE_SUSPEND_MASK	0x00000040 /**< CMD Type - Suspend */
#define XSDPS_CMD_TYPE_RESUME_MASK	0x00000080 /**< CMD Type - Resume */
#define XSDPS_CMD_TYPE_ABORT_MASK	0x000000C0 /**< CMD Type - Abort */
#define XSDPS_CMD_MASK			0x00003F00 /**< Command Index Mask -
							Set to CMD0-63,
							AMCD0-63 */

/* @} */

/** @name Auto CMD Error Status Register
 *
 * This register is read only register which contains
 * information about the error status of Auto CMD 12 and 23.
 * Read Only
 * @{
 */
#define XSDPS_AUTO_CMD12_NT_EX_MASK	0x0001 /**< Auto CMD12 Not
							executed */
#define XSDPS_AUTO_CMD_TOUT_MASK	0x0002 /**< Auto CMD Timeout
							Error */
#define XSDPS_AUTO_CMD_CRC_MASK		0x0004 /**< Auto CMD CRC Error */
#define XSDPS_AUTO_CMD_EB_MASK		0x0008 /**< Auto CMD End Bit
							Error */
#define XSDPS_AUTO_CMD_IND_MASK		0x0010 /**< Auto CMD Index Error */
#define XSDPS_AUTO_CMD_CNI_ERR_MASK	0x0080 /**< Command not issued by
							Auto CMD12 Error */
/* @} */

/** @name Host Control2 Register
 *
 * This register contains extended configuration bits.
 * Read Write
 * @{
 */
#define XSDPS_HC2_UHS_MODE_MASK		0x0007 /**< UHS Mode select bits */
#define XSDPS_HC2_UHS_MODE_SDR12_MASK	0x0000 /**< SDR12 UHS Mode */
#define XSDPS_HC2_UHS_MODE_SDR25_MASK	0x0001 /**< SDR25 UHS Mode */
#define XSDPS_HC2_UHS_MODE_SDR50_MASK	0x0002 /**< SDR50 UHS Mode */
#define XSDPS_HC2_UHS_MODE_SDR104_MASK	0x0003 /**< SDR104 UHS Mode */
#define XSDPS_HC2_UHS_MODE_DDR50_MASK	0x0004 /**< DDR50 UHS Mode */
#define XSDPS_HC2_1V8_EN_MASK		0x0008 /**< 1.8V Signal Enable */
#define XSDPS_HC2_DRV_STR_SEL_MASK	0x0030 /**< Driver Strength
							Selection */
#define XSDPS_HC2_DRV_STR_B_MASK	0x0000 /**< Driver Strength B */
#define XSDPS_HC2_DRV_STR_A_MASK	0x0010 /**< Driver Strength A */
#define XSDPS_HC2_DRV_STR_C_MASK	0x0020 /**< Driver Strength C */
#define XSDPS_HC2_DRV_STR_D_MASK	0x0030 /**< Driver Strength D */
#define XSDPS_HC2_EXEC_TNG_MASK		0x0040 /**< Execute Tuning */
#define XSDPS_HC2_SAMP_CLK_SEL_MASK	0x0080 /**< Sampling Clock
							Selection */
#define XSDPS_HC2_ASYNC_INTR_EN_MASK	0x4000 /**< Asynchronous Interrupt
							Enable */
#define XSDPS_HC2_PRE_VAL_EN_MASK	0x8000 /**< Preset Value Enable */

/* @} */

/** @name Capabilities Register
 *
 * Capabilities register is a read only register which contains
 * information about the host controller.
 * Sufficient if read once after power on.
 * Read Only
 * @{
 */
#define XSDPS_CAP_TOUT_CLK_FREQ_MASK	0x0000003F /**< Timeout clock freq
							select */
#define XSDPS_CAP_TOUT_CLK_UNIT_MASK	0x00000080 /**< Timeout clock unit -
							MHz/KHz */
#define XSDPS_CAP_MAX_BLK_LEN_MASK	0x00030000 /**< Max block length */
#define XSDPS_CAP_MAX_BLK_LEN_512B_MASK	0x00000000 /**< Max block 512 bytes */
#define XSDPS_CAP_MAX_BL_LN_1024_MASK	0x00010000 /**< Max block 1024 bytes */
#define XSDPS_CAP_MAX_BL_LN_2048_MASK	0x00020000 /**< Max block 2048 bytes */
#define XSDPS_CAP_MAX_BL_LN_4096_MASK	0x00030000 /**< Max block 4096 bytes */

#define XSDPS_CAP_EXT_MEDIA_BUS_MASK	0x00040000 /**< Extended media bus */
#define XSDPS_CAP_ADMA2_MASK		0x00080000 /**< ADMA2 support */
#define XSDPS_CAP_HIGH_SPEED_MASK	0x00200000 /**< High speed support */
#define XSDPS_CAP_SDMA_MASK		0x00400000 /**< SDMA support */
#define XSDPS_CAP_SUSP_RESUME_MASK	0x00800000 /**< Suspend/Resume
							support */
#define XSDPS_CAP_VOLT_3V3_MASK		0x01000000 /**< 3.3V support */
#define XSDPS_CAP_VOLT_3V0_MASK		0x02000000 /**< 3.0V support */
#define XSDPS_CAP_VOLT_1V8_MASK		0x04000000 /**< 1.8V support */

#define XSDPS_CAP_SYS_BUS_64_MASK	0x10000000 /**< 64 bit system bus
							support */
/* Spec 2.0 */
#define XSDPS_CAP_INTR_MODE_MASK	0x08000000 /**< Interrupt mode
							support */
#define XSDPS_CAP_SPI_MODE_MASK		0x20000000 /**< SPI mode */
#define XSDPS_CAP_SPI_BLOCK_MODE_MASK	0x40000000 /**< SPI block mode */


/* Spec 3.0 */
#define XSDPS_CAPS_ASYNC_INTR_MASK	0x20000000 /**< Async Interrupt
							support */
#define XSDPS_CAPS_SLOT_TYPE_MASK	0xC0000000 /**< Slot Type */
#define XSDPS_CAPS_REM_CARD			0x00000000 /**< Removable Slot */
#define XSDPS_CAPS_EMB_SLOT			0x40000000 /**< Embedded Slot */
#define XSDPS_CAPS_SHR_BUS			0x80000000 /**< Shared Bus Slot */

#define XSDPS_ECAPS_SDR50_MASK		0x00000001 /**< SDR50 Mode support */
#define XSDPS_ECAPS_SDR104_MASK		0x00000002 /**< SDR104 Mode support */
#define XSDPS_ECAPS_DDR50_MASK		0x00000004 /**< DDR50 Mode support */
#define XSDPS_ECAPS_DRV_TYPE_A_MASK	0x00000010 /**< DriverType A support */
#define XSDPS_ECAPS_DRV_TYPE_C_MASK	0x00000020 /**< DriverType C support */
#define XSDPS_ECAPS_DRV_TYPE_D_MASK	0x00000040 /**< DriverType D support */
#define XSDPS_ECAPS_TMR_CNT_MASK	0x00000F00 /**< Timer Count for
							Re-tuning */
#define XSDPS_ECAPS_USE_TNG_SDR50_MASK	0x00002000 /**< SDR50 Mode needs
							tuning */
#define XSDPS_ECAPS_RE_TNG_MODES_MASK	0x0000C000 /**< Re-tuning modes
							support */
#define XSDPS_ECAPS_RE_TNG_MODE1_MASK	0x00000000 /**< Re-tuning mode 1 */
#define XSDPS_ECAPS_RE_TNG_MODE2_MASK	0x00004000 /**< Re-tuning mode 2 */
#define XSDPS_ECAPS_RE_TNG_MODE3_MASK	0x00008000 /**< Re-tuning mode 3 */
#define XSDPS_ECAPS_CLK_MULT_MASK	0x00FF0000 /**< Clock Multiplier value
							for Programmable clock
							mode */
#define XSDPS_ECAPS_SPI_MODE_MASK	0x01000000 /**< SPI mode */
#define XSDPS_ECAPS_SPI_BLK_MODE_MASK	0x02000000 /**< SPI block mode */

/* @} */

/** @name Present State Register
 *
 * Gives the current status of the host controller
 * Read Only
 * @{
 */

#define XSDPS_PSR_INHIBIT_CMD_MASK	0x00000001 /**< Command inhibit - CMD */
#define XSDPS_PSR_INHIBIT_DAT_MASK	0x00000002 /**< Command Inhibit - DAT */
#define XSDPS_PSR_DAT_ACTIVE_MASK	0x00000004 /**< DAT line active */
#define XSDPS_PSR_RE_TUNING_REQ_MASK	0x00000008 /**< Re-tuning request */
#define XSDPS_PSR_WR_ACTIVE_MASK	0x00000100 /**< Write transfer active */
#define XSDPS_PSR_RD_ACTIVE_MASK	0x00000200 /**< Read transfer active */
#define XSDPS_PSR_BUFF_WR_EN_MASK	0x00000400 /**< Buffer write enable */
#define XSDPS_PSR_BUFF_RD_EN_MASK	0x00000800 /**< Buffer read enable */
#define XSDPS_PSR_CARD_INSRT_MASK	0x00010000 /**< Card inserted */
#define XSDPS_PSR_CARD_STABLE_MASK	0x00020000 /**< Card state stable */
#define XSDPS_PSR_CARD_DPL_MASK		0x00040000 /**< Card detect pin level */
#define XSDPS_PSR_WPS_PL_MASK		0x00080000 /**< Write protect switch
								pin level */
#define XSDPS_PSR_DAT30_SG_LVL_MASK	0x00F00000 /**< Data 3:0 signal lvl */
#define XSDPS_PSR_CMD_SG_LVL_MASK	0x01000000 /**< Cmd Line signal lvl */
#define XSDPS_PSR_DAT74_SG_LVL_MASK	0x1E000000 /**< Data 7:4 signal lvl */

/* @} */

/** @name Maximum Current Capablities Register
 *
 * This register is read only register which contains
 * information about current capabilities at each voltage levels.
 * Read Only
 * @{
 */
#define XSDPS_MAX_CUR_CAPS_1V8_MASK	0x00000F00 /**< Maximum Current
							Capability at 1.8V */
#define XSDPS_MAX_CUR_CAPS_3V0_MASK	0x000000F0 /**< Maximum Current
							Capability at 3.0V */
#define XSDPS_MAX_CUR_CAPS_3V3_MASK	0x0000000F /**< Maximum Current
							Capability at 3.3V */
/* @} */


/** @name Force Event for Auto CMD Error Status Register
 *
 * This register is write only register which contains
 * control bits to generate events for Auto CMD error status.
 * Write Only
 * @{
 */
#define XSDPS_FE_AUTO_CMD12_NT_EX_MASK	0x0001 /**< Auto CMD12 Not
							executed */
#define XSDPS_FE_AUTO_CMD_TOUT_MASK	0x0002 /**< Auto CMD Timeout
							Error */
#define XSDPS_FE_AUTO_CMD_CRC_MASK	0x0004 /**< Auto CMD CRC Error */
#define XSDPS_FE_AUTO_CMD_EB_MASK	0x0008 /**< Auto CMD End Bit
							Error */
#define XSDPS_FE_AUTO_CMD_IND_MASK	0x0010 /**< Auto CMD Index Error */
#define XSDPS_FE_AUTO_CMD_CNI_ERR_MASK	0x0080 /**< Command not issued by
							Auto CMD12 Error */
/* @} */



/** @name Force Event for Error Interrupt Status Register
 *
 * This register is write only register which contains
 * control bits to generate events of error interrupt status register.
 * Write Only
 * @{
 */
#define XSDPS_FE_INTR_ERR_CT_MASK	0x0001 /**< Command Timeout
							Error */
#define XSDPS_FE_INTR_ERR_CCRC_MASK	0x0002 /**< Command CRC Error */
#define XSDPS_FE_INTR_ERR_CEB_MASK	0x0004 /**< Command End Bit
							Error */
#define XSDPS_FE_INTR_ERR_CI_MASK	0x0008 /**< Command Index Error */
#define XSDPS_FE_INTR_ERR_DT_MASK	0x0010 /**< Data Timeout Error */
#define XSDPS_FE_INTR_ERR_DCRC_MASK	0x0020 /**< Data CRC Error */
#define XSDPS_FE_INTR_ERR_DEB_MASK	0x0040 /**< Data End Bit Error */
#define XSDPS_FE_INTR_ERR_CUR_LMT_MASK	0x0080 /**< Current Limit Error */
#define XSDPS_FE_INTR_ERR_AUTO_CMD_MASK	0x0100 /**< Auto CMD Error */
#define XSDPS_FE_INTR_ERR_ADMA_MASK	0x0200 /**< ADMA Error */
#define XSDPS_FE_INTR_ERR_TR_MASK	0x1000 /**< Target Reponse */
#define XSDPS_FE_INTR_VEND_SPF_ERR_MASK	0xE000 /**< Vendor Specific
							Error */

/* @} */

/** @name ADMA Error Status Register
 *
 * This register is read only register which contains
 * status information about ADMA errors.
 * Read Only
 * @{
 */
#define XSDPS_ADMA_ERR_MM_LEN_MASK	0x04 /**< ADMA Length Mismatch
							Error */
#define XSDPS_ADMA_ERR_STATE_MASK	0x03 /**< ADMA Error State */
#define XSDPS_ADMA_ERR_STATE_STOP_MASK	0x00 /**< ADMA Error State
							STOP */
#define XSDPS_ADMA_ERR_STATE_FDS_MASK	0x01 /**< ADMA Error State
							FDS */
#define XSDPS_ADMA_ERR_STATE_TFR_MASK	0x03 /**< ADMA Error State
							TFR */
/* @} */

/** @name Preset Values Register
 *
 * This register is read only register which contains
 * preset values for each of speed modes.
 * Read Only
 * @{
 */
#define XSDPS_PRE_VAL_SDCLK_FSEL_MASK	0x03FF /**< SDCLK Frequency
							Select Value */
#define XSDPS_PRE_VAL_CLK_GEN_SEL_MASK	0x0400 /**< Clock Generator
							Mode Select */
#define XSDPS_PRE_VAL_DRV_STR_SEL_MASK	0xC000 /**< Driver Strength
							Select Value */

/* @} */

/** @name Slot Interrupt Status Register
 *
 * This register is read only register which contains
 * interrupt slot signal for each slot.
 * Read Only
 * @{
 */
#define XSDPS_SLOT_INTR_STS_INT_MASK	0x0007 /**< Interrupt Signal
							mask */

/* @} */

/** @name Host Controller Version Register
 *
 * This register is read only register which contains
 * Host Controller and Vendor Specific version.
 * Read Only
 * @{
 */
#define XSDPS_HC_VENDOR_VER		0xFF00 /**< Vendor
							Specification
							version mask */
#define XSDPS_HC_SPEC_VER_MASK		0x00FF /**< Host
							Specification
							version mask */
#define XSDPS_HC_SPEC_V3		0x0002
#define XSDPS_HC_SPEC_V2		0x0001
#define XSDPS_HC_SPEC_V1		0x0000

/** @name Block size mask for 512 bytes
 *
 * Block size mask for 512 bytes - This is the default block size.
 * @{
 */

#define XSDPS_BLK_SIZE_512_MASK	0x200

/* @} */

/** @name Commands
 *
 * Constant definitions for commands and response related to SD
 * @{
 */

#define XSDPS_APP_CMD_PREFIX	 0x8000
#define CMD0	 0x0000
#define CMD1	 0x0100
#define CMD2	 0x0200
#define CMD3	 0x0300
#define CMD4	 0x0400
#define CMD5	 0x0500
#define CMD6     0x0600
#define ACMD6    (XSDPS_APP_CMD_PREFIX + 0x0600)
#define CMD7	 0x0700
#define CMD8	 0x0800
#define CMD9	 0x0900
#define CMD10	 0x0A00
#define CMD11	 0x0B00
#define CMD12	 0x0C00
#define ACMD13	 (XSDPS_APP_CMD_PREFIX + 0x0D00)
#define CMD16	 0x1000
#define CMD17	 0x1100
#define CMD18	 0x1200
#define CMD19	 0x1300
#define CMD21	 0x1500
#define CMD23	 0x1700
#define ACMD23	 (XSDPS_APP_CMD_PREFIX + 0x1700)
#define CMD24	 0x1800
#define CMD25	 0x1900
#define CMD41	 0x2900
#define ACMD41	 (XSDPS_APP_CMD_PREFIX + 0x2900)
#define ACMD42	 (XSDPS_APP_CMD_PREFIX + 0x2A00)
#define ACMD51	 (XSDPS_APP_CMD_PREFIX + 0x3300)
#define CMD52	 0x3400
#define CMD55	 0x3700
#define CMD58	 0x3A00

#define RESP_NONE	XSDPS_CMD_RESP_NONE_MASK
#define RESP_R1		XSDPS_CMD_RESP_L48_MASK | XSDPS_CMD_CRC_CHK_EN_MASK | \
			XSDPS_CMD_INX_CHK_EN_MASK

#define RESP_R1B	XSDPS_CMD_RESP_L48_BSY_CHK_MASK | \
			XSDPS_CMD_CRC_CHK_EN_MASK | XSDPS_CMD_INX_CHK_EN_MASK

#define RESP_R2		XSDPS_CMD_RESP_L136_MASK | XSDPS_CMD_CRC_CHK_EN_MASK
#define RESP_R3		XSDPS_CMD_RESP_L48_MASK

#define RESP_R6		XSDPS_CMD_RESP_L48_BSY_CHK_MASK | \
			XSDPS_CMD_CRC_CHK_EN_MASK | XSDPS_CMD_INX_CHK_EN_MASK

/* @} */

/* Card Interface Conditions Definitions */
#define XSDPS_CIC_CHK_PATTERN	0xAA
#define XSDPS_CIC_VOLT_MASK	(0xF<<8)
#define XSDPS_CIC_VOLT_2V7_3V6	(1<<8)
#define XSDPS_CIC_VOLT_LOW	(1<<9)

/* Operation Conditions Register Definitions */
#define XSDPS_OCR_PWRUP_STS	(1<<31)
#define XSDPS_OCR_CC_STS	(1<<30)
#define XSDPS_OCR_S18		(1<<24)
#define XSDPS_OCR_3V5_3V6	(1<<23)
#define XSDPS_OCR_3V4_3V5	(1<<22)
#define XSDPS_OCR_3V3_3V4	(1<<21)
#define XSDPS_OCR_3V2_3V3	(1<<20)
#define XSDPS_OCR_3V1_3V2	(1<<19)
#define XSDPS_OCR_3V0_3V1	(1<<18)
#define XSDPS_OCR_2V9_3V0	(1<<17)
#define XSDPS_OCR_2V8_2V9	(1<<16)
#define XSDPS_OCR_2V7_2V8	(1<<15)
#define XSDPS_OCR_1V7_1V95	(1<<7)
#define XSDPS_OCR_HIGH_VOL	0x00FF8000
#define XSDPS_OCR_LOW_VOL	0x00000080

/* SD Card Configuration Register Definitions */
#define XSDPS_SCR_REG_LEN		8
#define XSDPS_SCR_STRUCT_MASK		(0xF<<28)
#define XSDPS_SCR_SPEC_MASK		(0xF<<24)
#define XSDPS_SCR_SPEC_1V0		0
#define XSDPS_SCR_SPEC_1V1		(1<<24)
#define XSDPS_SCR_SPEC_2V0_3V0		(2<<24)
#define XSDPS_SCR_MEM_VAL_AF_ERASE	(1<<23)
#define XSDPS_SCR_SEC_SUPP_MASK		(7<<20)
#define XSDPS_SCR_SEC_SUPP_NONE		0
#define XSDPS_SCR_SEC_SUPP_1V1		(2<<20)
#define XSDPS_SCR_SEC_SUPP_2V0		(3<<20)
#define XSDPS_SCR_SEC_SUPP_3V0		(4<<20)
#define XSDPS_SCR_BUS_WIDTH_MASK	(0xF<<16)
#define XSDPS_SCR_BUS_WIDTH_1		(1<<16)
#define XSDPS_SCR_BUS_WIDTH_4		(4<<16)
#define XSDPS_SCR_SPEC3_MASK		(1<<12)
#define XSDPS_SCR_SPEC3_2V0		0
#define XSDPS_SCR_SPEC3_3V0		(1<<12)
#define XSDPS_SCR_CMD_SUPP_MASK		0x3
#define XSDPS_SCR_CMD23_SUPP		(1<<1)
#define XSDPS_SCR_CMD20_SUPP		(1<<0)

/* Card Status Register Definitions */
#define XSDPS_CD_STS_OUT_OF_RANGE	(1<<31)
#define XSDPS_CD_STS_ADDR_ERR		(1<<30)
#define XSDPS_CD_STS_BLK_LEN_ERR	(1<<29)
#define XSDPS_CD_STS_ER_SEQ_ERR		(1<<28)
#define XSDPS_CD_STS_ER_PRM_ERR		(1<<27)
#define XSDPS_CD_STS_WP_VIO		(1<<26)
#define XSDPS_CD_STS_IS_LOCKED		(1<<25)
#define XSDPS_CD_STS_LOCK_UNLOCK_FAIL	(1<<24)
#define XSDPS_CD_STS_CMD_CRC_ERR	(1<<23)
#define XSDPS_CD_STS_ILGL_CMD		(1<<22)
#define XSDPS_CD_STS_CARD_ECC_FAIL	(1<<21)
#define XSDPS_CD_STS_CC_ERR		(1<<20)
#define XSDPS_CD_STS_ERR		(1<<19)
#define XSDPS_CD_STS_CSD_OVRWR		(1<<16)
#define XSDPS_CD_STS_WP_ER_SKIP		(1<<15)
#define XSDPS_CD_STS_CARD_ECC_DIS	(1<<14)
#define XSDPS_CD_STS_ER_RST		(1<<13)
#define XSDPS_CD_STS_CUR_STATE		(0xF<<9)
#define XSDPS_CD_STS_RDY_FOR_DATA	(1<<8)
#define XSDPS_CD_STS_APP_CMD		(1<<5)
#define XSDPS_CD_STS_AKE_SEQ_ERR	(1<<2)

/* Switch Function Definitions CMD6 */
#define XSDPS_SWITCH_SD_RESP_LEN	64

#define XSDPS_SWITCH_FUNC_SWITCH	(1<<31)
#define XSDPS_SWITCH_FUNC_CHECK		0

#define XSDPS_MODE_FUNC_GRP1		1
#define XSDPS_MODE_FUNC_GRP2		2
#define XSDPS_MODE_FUNC_GRP3		3
#define XSDPS_MODE_FUNC_GRP4		4
#define XSDPS_MODE_FUNC_GRP5		5
#define XSDPS_MODE_FUNC_GRP6		6

#define XSDPS_FUNC_GRP_DEF_VAL		0xF
#define XSDPS_FUNC_ALL_GRP_DEF_VAL	0xFFFFFF

#define XSDPS_ACC_MODE_DEF_SDR12	0
#define XSDPS_ACC_MODE_HS_SDR25		1
#define XSDPS_ACC_MODE_SDR50		2
#define XSDPS_ACC_MODE_SDR104		3
#define XSDPS_ACC_MODE_DDR50		4

#define XSDPS_CMD_SYS_ARG_SHIFT		4
#define XSDPS_CMD_SYS_DEF		0
#define XSDPS_CMD_SYS_eC		1
#define XSDPS_CMD_SYS_OTP		3
#define XSDPS_CMD_SYS_ASSD		4
#define XSDPS_CMD_SYS_VEND		5

#define XSDPS_DRV_TYPE_ARG_SHIFT	8
#define XSDPS_DRV_TYPE_B		0
#define XSDPS_DRV_TYPE_A		1
#define XSDPS_DRV_TYPE_C		2
#define XSDPS_DRV_TYPE_D		3

#define XSDPS_CUR_LIM_ARG_SHIFT		12
#define XSDPS_CUR_LIM_200		0
#define XSDPS_CUR_LIM_400		1
#define XSDPS_CUR_LIM_600		2
#define XSDPS_CUR_LIM_800		3


/* EXT_CSD field definitions */
#define XSDPS_EXT_CSD_SIZE		512

#define EXT_CSD_WR_REL_PARAM_EN		(1<<2)

#define EXT_CSD_BOOT_WP_B_PWR_WP_DIS    (0x40)
#define EXT_CSD_BOOT_WP_B_PERM_WP_DIS   (0x10)
#define EXT_CSD_BOOT_WP_B_PERM_WP_EN    (0x04)
#define EXT_CSD_BOOT_WP_B_PWR_WP_EN     (0x01)

#define EXT_CSD_PART_CONFIG_ACC_MASK    (0x7)
#define EXT_CSD_PART_CONFIG_ACC_BOOT0   (0x1)
#define EXT_CSD_PART_CONFIG_ACC_RPMB    (0x3)
#define EXT_CSD_PART_CONFIG_ACC_GP0     (0x4)

#define EXT_CSD_PART_SUPPORT_PART_EN    (0x1)

#define EXT_CSD_CMD_SET_NORMAL          (1<<0)
#define EXT_CSD_CMD_SET_SECURE          (1<<1)
#define EXT_CSD_CMD_SET_CPSECURE        (1<<2)

#define EXT_CSD_CARD_TYPE_26    	(1<<0)  /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52    	(1<<1)  /* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_MASK  	0x3F    /* Mask out reserved bits */
#define EXT_CSD_CARD_TYPE_DDR_1_8V	(1<<2)   /* Card can run at 52MHz */
                                             /* DDR mode @1.8V or 3V I/O */
#define EXT_CSD_CARD_TYPE_DDR_1_2V	(1<<3)   /* Card can run at 52MHz */
                                             /* DDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_DDR_52	(EXT_CSD_CARD_TYPE_DDR_1_8V  \
                                        | EXT_CSD_CARD_TYPE_DDR_1_2V)
#define EXT_CSD_CARD_TYPE_SDR_1_8V      (1<<4)  /* Card can run at 200MHz */
#define EXT_CSD_CARD_TYPE_SDR_1_2V      (1<<5)  /* Card can run at 200MHz */
                                                /* SDR mode @1.2V I/O */
#define EXT_CSD_BUS_WIDTH_BYTE			183
#define EXT_CSD_BUS_WIDTH_1_BIT			0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4_BIT			1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8_BIT			2	/* Card is in 8 bit mode */
#define EXT_CSD_BUS_WIDTH_DDR_4_BIT		5	/* Card is in 4 bit DDR mode */
#define EXT_CSD_BUS_WIDTH_DDR_8_BIT		6	/* Card is in 8 bit DDR mode */

#define EXT_CSD_HS_TIMING_BYTE		185
#define EXT_CSD_HS_TIMING_DEF		0
#define EXT_CSD_HS_TIMING_HIGH		1	/* Card is in high speed mode */
#define EXT_CSD_HS_TIMING_HS200		2	/* Card is in HS200 mode */


#define XSDPS_EXT_CSD_CMD_SET		0
#define XSDPS_EXT_CSD_SET_BITS		1
#define XSDPS_EXT_CSD_CLR_BITS		2
#define XSDPS_EXT_CSD_WRITE_BYTE	3

#define XSDPS_MMC_DEF_SPEED_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					| (EXT_CSD_HS_TIMING_BYTE << 16) \
					| (EXT_CSD_HS_TIMING_DEF << 8))

#define XSDPS_MMC_HIGH_SPEED_ARG	((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_HS_TIMING_BYTE << 16) \
					 | (EXT_CSD_HS_TIMING_HIGH << 8))

#define XSDPS_MMC_HS200_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_HS_TIMING_BYTE << 16) \
					 | (EXT_CSD_HS_TIMING_HS200 << 8))

#define XSDPS_MMC_1_BIT_BUS_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_BUS_WIDTH_BYTE << 16) \
					 | (EXT_CSD_BUS_WITH_1_BIT << 8))

#define XSDPS_MMC_4_BIT_BUS_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_BUS_WIDTH_BYTE << 16) \
					 | (EXT_CSD_BUS_WIDTH_4_BIT << 8))

#define XSDPS_MMC_8_BIT_BUS_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_BUS_WIDTH_BYTE << 16) \
					 | (EXT_CSD_BUS_WIDTH_8_BIT << 8))

#define XSDPS_MMC_DDR_4_BIT_BUS_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_BUS_WIDTH_BYTE << 16) \
					 | (EXT_CSD_BUS_WIDTH_DDR_4_BIT << 8))

#define XSDPS_MMC_DDR_8_BIT_BUS_ARG		((XSDPS_EXT_CSD_WRITE_BYTE << 24) \
					 | (EXT_CSD_BUS_WIDTH_BYTE << 16) \
					 | (EXT_CSD_BUS_WIDTH_DDR_8_BIT << 8))

#define XSDPS_MMC_DELAY_FOR_SWITCH	1000

/* @} */

/* @400KHz, in usec */
#define XSDPS_74CLK_DELAY	2960
#define XSDPS_100CLK_DELAY	4000
#define XSDPS_INIT_DELAY	10000

#define XSDPS_DEF_VOLT_LVL	XSDPS_PC_BUS_VSEL_3V0_MASK
#define XSDPS_CARD_DEF_ADDR	0x1234

#define XSDPS_CARD_SD		1
#define XSDPS_CARD_MMC		2
#define XSDPS_CARD_SDIO		3
#define XSDPS_CARD_SDCOMBO	4
#define XSDPS_CHIP_EMMC		5


/** @name ADMA2 Descriptor related definitions
 *
 * ADMA2 Descriptor related definitions
 * @{
 */

#define XSDPS_DESC_MAX_LENGTH 65536U

#define XSDPS_DESC_VALID     	(0x1 << 0)
#define XSDPS_DESC_END       	(0x1 << 1)
#define XSDPS_DESC_INT       	(0x1 << 2)
#define XSDPS_DESC_TRAN  	(0x2 << 4)

/* @} */

/* For changing clock frequencies */
#define XSDPS_CLK_400_KHZ		400000		/**< 400 KHZ */
#define XSDPS_CLK_50_MHZ		50000000	/**< 50 MHZ */
#define XSDPS_CLK_52_MHZ		52000000	/**< 52 MHZ */
#define XSDPS_SD_VER_1_0		0x1		/**< SD ver 1 */
#define XSDPS_SD_VER_2_0		0x2		/**< SD ver 2 */
#define XSDPS_SCR_BLKCNT	1
#define XSDPS_SCR_BLKSIZE	8
#define XSDPS_1_BIT_WIDTH	0x1
#define XSDPS_4_BIT_WIDTH	0x2
#define XSDPS_8_BIT_WIDTH	0x3
#define XSDPS_UHS_SPEED_MODE_SDR12	0x0
#define XSDPS_UHS_SPEED_MODE_SDR25	0x1
#define XSDPS_UHS_SPEED_MODE_SDR50	0x2
#define XSDPS_UHS_SPEED_MODE_SDR104	0x3
#define XSDPS_UHS_SPEED_MODE_DDR50	0x4
#define XSDPS_SWITCH_CMD_BLKCNT		1
#define XSDPS_SWITCH_CMD_BLKSIZE	64
#define XSDPS_SWITCH_CMD_HS_GET		0x00FFFFF0
#define XSDPS_SWITCH_CMD_HS_SET		0x80FFFFF1
#define XSDPS_SWITCH_CMD_SDR12_SET		0x80FFFFF0
#define XSDPS_SWITCH_CMD_SDR25_SET		0x80FFFFF1
#define XSDPS_SWITCH_CMD_SDR50_SET		0x80FFFFF2
#define XSDPS_SWITCH_CMD_SDR104_SET		0x80FFFFF3
#define XSDPS_SWITCH_CMD_DDR50_SET		0x80FFFFF4
#define XSDPS_EXT_CSD_CMD_BLKCNT	1
#define XSDPS_EXT_CSD_CMD_BLKSIZE	512
#define XSDPS_TUNING_CMD_BLKCNT		1
#define XSDPS_TUNING_CMD_BLKSIZE	64

#define XSDPS_HIGH_SPEED_MAX_CLK	50000000
#define XSDPS_UHS_SDR104_MAX_CLK	208000000
#define XSDPS_UHS_SDR50_MAX_CLK		100000000
#define XSDPS_UHS_DDR50_MAX_CLK		50000000
#define XSDPS_UHS_SDR25_MAX_CLK		50000000
#define XSDPS_UHS_SDR12_MAX_CLK		25000000

#define SD_DRIVER_TYPE_B	0x01
#define SD_DRIVER_TYPE_A	0x02
#define SD_DRIVER_TYPE_C	0x04
#define SD_DRIVER_TYPE_D	0x08
#define SD_SET_CURRENT_LIMIT_200	0
#define SD_SET_CURRENT_LIMIT_400	1
#define SD_SET_CURRENT_LIMIT_600	2
#define SD_SET_CURRENT_LIMIT_800	3

#define SD_MAX_CURRENT_200	(1 << SD_SET_CURRENT_LIMIT_200)
#define SD_MAX_CURRENT_400	(1 << SD_SET_CURRENT_LIMIT_400)
#define SD_MAX_CURRENT_600	(1 << SD_SET_CURRENT_LIMIT_600)
#define SD_MAX_CURRENT_800	(1 << SD_SET_CURRENT_LIMIT_800)

#define XSDPS_SD_SDR12_MAX_CLK	25000000
#define XSDPS_SD_SDR25_MAX_CLK	50000000
#define XSDPS_SD_SDR50_MAX_CLK	100000000
#define XSDPS_SD_DDR50_MAX_CLK	50000000
#define XSDPS_SD_SDR104_MAX_CLK	208000000
#define XSDPS_MMC_HS200_MAX_CLK	200000000

#define XSDPS_CARD_STATE_IDLE		0
#define XSDPS_CARD_STATE_RDY		1
#define XSDPS_CARD_STATE_IDEN		2
#define XSDPS_CARD_STATE_STBY		3
#define XSDPS_CARD_STATE_TRAN		4
#define XSDPS_CARD_STATE_DATA		5
#define XSDPS_CARD_STATE_RCV		6
#define XSDPS_CARD_STATE_PROG		7
#define XSDPS_CARD_STATE_DIS		8
#define XSDPS_CARD_STATE_BTST		9
#define XSDPS_CARD_STATE_SLP		10

#define XSDPS_SLOT_REM			0
#define XSDPS_SLOT_EMB			1

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSdPs_In64 Xil_In64
#define XSdPs_Out64 Xil_Out64

#define XSdPs_In32 Xil_In32
#define XSdPs_Out32 Xil_Out32

#define XSdPs_In16 Xil_In16
#define XSdPs_Out16 Xil_Out16

#define XSdPs_In8 Xil_In8
#define XSdPs_Out8 Xil_Out8

/****************************************************************************/
/**
* Read a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XSdPs_ReadReg(XSdPs *InstancePtr. s32 RegOffset)
*
******************************************************************************/
#define XSdPs_ReadReg64(InstancePtr, RegOffset) \
	XSdPs_In64((InstancePtr->Config.BaseAddress) + RegOffset)

/***************************************************************************/
/**
* Write to a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XSdPs_WriteReg(XSdPs *InstancePtr, s32 RegOffset,
*		u64 RegisterValue)
*
******************************************************************************/
#define XSdPs_WriteReg64(InstancePtr, RegOffset, RegisterValue) \
	XSdPs_Out64((InstancePtr->Config.BaseAddress) + (RegOffset), \
		(RegisterValue))

/****************************************************************************/
/**
* Read a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XSdPs_ReadReg(u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XSdPs_ReadReg(BaseAddress, RegOffset) \
	XSdPs_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write to a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XSdPs_WriteReg(u32 BaseAddress, int RegOffset,
*		u32 RegisterValue)
*
******************************************************************************/
#define XSdPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XSdPs_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/****************************************************************************/
/**
* Read a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u16 XSdPs_ReadReg(u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XSdPs_ReadReg16(BaseAddress, RegOffset) \
	XSdPs_In16((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write to a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XSdPs_WriteReg(u32 BaseAddress, int RegOffset,
*		u16 RegisterValue)
*
******************************************************************************/
#define XSdPs_WriteReg16(BaseAddress, RegOffset, RegisterValue) \
	XSdPs_Out16((BaseAddress) + (RegOffset), (RegisterValue))

/****************************************************************************/
/**
* Read a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u8 XSdPs_ReadReg(u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XSdPs_ReadReg8(BaseAddress, RegOffset) \
	XSdPs_In8((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write to a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XSdPs_WriteReg(u32 BaseAddress, int RegOffset,
*		u8 RegisterValue)
*
******************************************************************************/
#define XSdPs_WriteReg8(BaseAddress, RegOffset, RegisterValue) \
	XSdPs_Out8((BaseAddress) + (RegOffset), (RegisterValue))

/***************************************************************************/
/**
* Macro to get present status register
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		C-Style signature:
*		void XSdPs_WriteReg(u32 BaseAddress, int RegOffset,
*		u8 RegisterValue)
*
******************************************************************************/
#define XSdPs_GetPresentStatusReg(BaseAddress) \
		XSdPs_In32((BaseAddress) + (XSDPS_PRES_STATE_OFFSET))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* SD_HW_H_ */
/** @} */
