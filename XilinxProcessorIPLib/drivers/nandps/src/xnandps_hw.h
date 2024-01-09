/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps_hw.h
* @addtogroup nandps Overview
* @{
*
* This file contains identifiers and low-level macros/functions for the NAND
* Flash controller driver.
* See xnandps.h for more information.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* </pre>
*
******************************************************************************/

#ifndef XNANDPS_HW_H		/* prevent circular inclusions */
#define XNANDPS_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/

/*
 * Memory controller configuration register offset
 */
#define XNANDPS_MEMC_STATUS_OFFSET		0x000	/**< Controller status
							  reg, RO */
#define XNANDPS_MEMC_IF_CONFIG_OFFSET		0x004	/**< Interface config
							  reg, RO */
#define XNANDPS_MEMC_SET_CONFIG_OFFSET		0x008	/**< Set configuration
							  reg, WO */
#define XNANDPS_MEMC_CLR_CONFIG_OFFSET		0x00C	/**< Clear config reg,
							  WO */
#define XNANDPS_DIRECT_CMD_OFFSET		0x010	/**< Direct command
							  reg, WO */
#define XNANDPS_SET_CYCLES_OFFSET		0x014	/**< Set cycles
							  register, WO */
#define XNANDPS_SET_OPMODE_OFFSET		0x018	/**< Set opmode
							  register, WO */
#define XNANDPS_REFRESH_PERIOD_0_OFFSET	0x020	/**< Refresh period_0
							  reg, RW */
#define XNANDPS_REFRESH_PERIOD_1_OFFSET	0x024	/**< Refresh period_1
							  reg, RW */

/*
 * Chip configuration register offset
 */
#define XNANDPS_IF0_CHIP_0_CONFIG_OFFSET	0x100	/**< Interface 0 chip 0
							  config */
#define XNANDPS_IF0_CHIP_1_CONFIG_OFFSET	0x120	/**< Interface 0 chip 1
							  config */
#define XNANDPS_IF0_CHIP_2_CONFIG_OFFSET	0x140	/**< Interface 0 chip 2
							  config */
#define XNANDPS_IF0_CHIP_3_CONFIG_OFFSET	0x160	/**< Interface 0 chip 3
							  config */
#define XNANDPS_IF1_CHIP_0_CONFIG_OFFSET	0x180	/**< Interface 1 chip 0
							  config */
#define XNANDPS_IF1_CHIP_1_CONFIG_OFFSET	0x1A0	/**< Interface 1 chip 1
							  config */
#define XNANDPS_IF1_CHIP_2_CONFIG_OFFSET	0x1C0	/**< Interface 1 chip 2
							  config */
#define XNANDPS_IF1_CHIP_3_CONFIG_OFFSET	0x1E0	/**< Interface 1 chip 3
							  config */

/*
 * nand_cycles (RO), sram_cycles (RO) and opmode_x_n (RO) registers offsets
 */
#define XNANDPS_FLASH_CYCLES(addr)		(0x000 + addr) /**< NAND & SRAM
								 cycle,RO*/
#define XNANDPS_OPMODE(addr)			(0x004 + addr) /**< Chip opmode
								 reg, RO */

/*
 * User configuration register offset
 */
#define XNANDPS_USER_STATUS_OFFSET		0x200	/**< User status reg,
							  RO */
#define XNANDPS_USER_CONFIG_OFFSET		0x204	/**< User config reg,
							  WO */

/*
 * ECC register offset
 */
#define XNANDPS_IF0_ECC_OFFSET			0x300	/**< Interface 0 ECC
							  register */
#define XNANDPS_IF1_ECC_OFFSET			0x400	/**< Interface 1 ECC
							  register */
#define XNANDPS_ECC_STATUS_OFFSET(addr)	(0x000 + addr) /**< ECC status
								 register */
#define XNANDPS_ECC_MEMCFG_OFFSET(addr)	(0x004 + addr) /**< ECC mem
								 config reg */
#define XNANDPS_ECC_MEMCMD1_OFFSET(addr)	(0x008 + addr) /**< ECC mem
								 com1 reg*/
#define XNANDPS_ECC_MEMCMD2_OFFSET(addr)	(0x00C + addr) /**< ECC mem
								 com2 reg*/
#define XNANDPS_ECC_ADDR0_OFFSET(addr)		(0x010 + addr) /**< ECC
								 address0 reg
								 */
#define XNANDPS_ECC_ADDR1_OFFSET(addr)		(0x014 + addr) /**< ECC
								 address1 reg
								 */
#define XNANDPS_ECC_VALUE0_OFFSET(addr)	(0x018 + addr) /**< ECC value 0
								 reg */
#define XNANDPS_ECC_VALUE1_OFFSET(addr)	(0x01C + addr) /**< ECC value 1
								 reg */
#define XNANDPS_ECC_VALUE2_OFFSET(addr)	(0x020 + addr) /**< ECC value 2
								 reg */
#define XNANDPS_ECC_VALUE3_OFFSET(addr)	(0x024 + addr) /**< ECC value 3
								 reg */
#define XNANDPS_ECC_VALUE4_OFFSET(addr)	(0x028 + addr) /**< ECC value 4
								 reg */

/*
 * Integration test register offset
 */
#define XNANDPS_INTGTEST_OFFSET		0xE00	/**< Integration test
								offset */

/*
 * ID configuration register offset
 */
#define XNANDPS_PERIPH_ID0_OFFSET		0xFE0	/**< Peripheral id0
							  register */
#define XNANDPS_PERIPH_ID1_OFFSET		0xFE4	/**< Peripheral id1
							  register */
#define XNANDPS_PERIPH_ID2_OFFSET		0xFE8	/**< Peripheral id2
							  register */
#define XNANDPS_PERIPH_ID3_OFFSET		0xFEC	/**< Peripheral id3
							  register */
#define XNANDPS_PCELL_ID0_OFFSET		0xFF0	/**< Primecell id0
							  register */
#define XNANDPS_PCELL_ID1_OFFSET		0xFF4	/**< Primecell id1
							  register */
#define XNANDPS_PCELL_ID2_OFFSET		0xFF8	/**< Primecell id2
							  register */
#define XNANDPS_PCELL_ID3_OFFSET		0xFFC	/**< Primecell id3
							  register */

/** @name Memory controller status register bit definitions and masks
 *  @{
 */
#define XNANDPS_MEMC_STATUS_STATE_MASK			0x00000001 /**< Memory
				controller operating state mask */
#define XNANDPS_MEMC_STATUS_INT_EN0_MASK		0x00000002 /**< Memory
				controller interface 0 interrupt enable mask */
#define XNANDPS_MEMC_STATUS_INT_EN1_MASK		0x00000004 /**< Memory
				controller interface 1 interrupt enable mask */
#define XNANDPS_MEMC_STATUS_INT_STATUS0_MASK		0x00000008 /**< Memory
				controller interface 0 interrupt status mask */
#define XNANDPS_MEMC_STATUS_INT_STATUS1_MASK		0x00000010 /**< Memory
				controller interface 1 interrupt status mask */
#define XNANDPS_MEMC_STATUS_RAW_INT_STATUS0_MASK	0x00000020 /**< Memory
			controller interface 0 raw interrupt status mask */
#define XNANDPS_MEMC_STATUS_RAW_INT_STATUS1_MASK	0x00000040 /**< Memory
			controller interface 1 raw interrupt status mask */
#define XNANDPS_MEMC_STATUS_ECC_INT_EN0_MASK		0x00000080 /**< Memory
			controller interface 0 ECC interrupt enable mask */
#define XNANDPS_MEMC_STATUS_ECC_INT_EN1_MASK		0x00000100 /**< Memory
			controller interface 1 ECC interrupt enable mask */
#define XNANDPS_MEMC_STATUS_ECC_INT0_MASK		0x00000200 /**< Memory
			controller interface 0 ECC interrupt status mask */
#define XNANDPS_MEMC_STATUS_ECC_INT1_MASK		0x00000400 /**< Memory
			controller interface 1 ECC interrupt status mask */
#define XNANDPS_MEMC_STATUS_RAW_ECC_INT0_MASK		0x00000800 /**< Memory
			controller interface 0 raw ECC interrupt status mask */
#define XNANDPS_MEMC_STATUS_RAW_ECC_INT1_MASK		0x00001000 /**< Memory
			controller interface 1 raw ECC interrupt status mask */
/* @} */

/** @name Memory interface configurartion register bit definitions and masks
 *  @{
 */
#define XNANDPS_MEMC_IF_CONFIG_MEMORY_TYPE0_MASK	0x00000003 /**< Memory
				controller interface 0 type mask */
#define XNANDPS_MEMC_IF_CONFIG_MEMORY_CHIPS0_MASK	0x0000000C /**< Memory
				controller interface 0 chip select mask */
#define XNANDPS_MEMC_IF_CONFIG_MEMORY_WIDTH0_MASK	0x00000030 /**< Memory
				controller interface 0 data width mask */
#define XNANDPS_MEMC_IF_CONFIG_REMAP0_MASK		0x00000040 /**< Memory
				controller interface 0 remap0 mask */
#define XNANDPS_MEMC_IF_CONFIG_MEMORY_TYPE1_MASK	0x00000300 /**< Memory
				controller interface 1 type mask */
#define XNANDPS_MEMC_IF_CONFIG_MEMORY_CHIPS1_MASK	0x00000C00 /**< Memory
				controller interface 1 chip select mask */
#define XNANDPS_MEMC_IF_CONFIG_MEMORY_WIDTH1_MASK	0x00003000 /**< Memory
				controller interface 1 data width mask */
#define XNANDPS_MEMC_IF_CONFIG_REMAP1_MASK		0x00004000 /**< Memory
				controller interface 1 remap0 mask */
#define XNANDPS_MEMC_IF_CONFIG_EX_MONITORS_MASK	0x00030000 /**< Memory
				controller interface exclusive masks mask */
/* @} */

/** @name Set configuration register bit definitions and masks
 *  @{
 */
#define XNANDPS_MEMC_SET_CONFIG_INT_ENABLE0_MASK	0x00000001 /**< Memory
			controller interfce0 interrupt enable mask */
#define XNANDPS_MEMC_SET_CONFIG_INT_ENABLE1_MASK	0x00000002 /**< Memory
			controller interfce1 interrupt enable mask */
#define XNANDPS_MEMC_SET_CONFIG_LOW_POWER_REQ_MASK	0x00000004 /**< Memory
			controller low power state mask */
#define XNANDPS_MEMC_SET_CONFIG_ECC_INT_ENABLE0_MASK	0x00000020 /**< Memory
		controller interfce0 ECC interrupt enable mask */
#define XNANDPS_MEMC_SET_CONFIG_ECC_INT_ENABLE1_MASK	0x00000040 /**< Memory
		controller interfce1 ECC interrupt enable mask */
/* @} */

/** @name Clear configuration register bit definitions and masks
 *  @{
 */
#define XNANDPS_MEMC_CLR_CONFIG_INT_DISABLE0_MASK	0x00000001 /**< Memory
			controller interface 0 interrupt disable mask */
#define XNANDPS_MEMC_CLR_CONFIG_INT_DISABLE1_MASK	0x00000002 /**< Memory
			controller interface 1 interrupt disable mask */
#define XNANDPS_MEMC_CLR_CONFIG_LOW_POWER_EXIT_MASK	0x00000004 /**< Memory
			controller low power exit mask */
#define XNANDPS_MEMC_CLR_CONFIG_INT_CLR0_MASK		0x00000008 /**< Memory
			controller interface0 interrupt clear mask */
#define XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK		0x00000010 /**< Memory
			controller interface1 interrupt clear mask */
#define XNANDPS_MEMC_CLR_CONFIG_ECC_INT_DISABLE0_MASK	0x00000020 /**< Memory
			controller interface0 ECC interrupt disable mask */
#define XNANDPS_MEMC_CLR_CONFIG_ECC_INT_DISABLE1_MASK	0x00000040 /**< Memory
			controller interface1 ECC interrupt disable mask */
/* @} */

/** @name Clear configuration register bit definitions and masks and shift
 *  @{
 */
#define XNANDPS_DIRECT_CMD_ADDR_MASK			0x000FFFFF /**< Direct
						command address mask */
#define XNANDPS_DIRECT_CMD_SET_CRE_MASK		0x00100000 /**< Direct
						command set cre mask */
#define XNANDPS_DIRECT_CMD_TYPE_MASK			0x00600000 /**< Direct
						command type mask */
#define XNANDPS_DIRECT_CMD_CHIP_SELECT_MASK		0x03800000 /**< Direct
						command chip select mask */

#define XNANDPS_DIRECT_CMD_SET_CRE_SHIFT	20	/**< Direct command
							  set_cre shift */
#define XNANDPS_DIRECT_CMD_CMD_TYPE_SHIFT	21	/**< Direct command
							  cmd_type shift */
#define XNANDPS_DIRECT_CMD_CHIP_SELECT_SHIFT	23	/**< Direct command
							  chip select shift */
/* @} */

/** @name Set cycles register bit definitions and masks and shift
 *  @{
 */
#define XNANDPS_SET_CYCLES_SET_T0_MASK			0x0000000F /**< Set
						cycles set_t0 mask */
#define XNANDPS_SET_CYCLES_SET_T1_MASK			0x000000F0 /**< Set
						cycles set_t1 mask */
#define XNANDPS_SET_CYCLES_SET_T2_MASK			0x00000700 /**< Set
						cycles set_t2 mask */
#define XNANDPS_SET_CYCLES_SET_T3_MASK			0x00003800 /**< Set
						cycles set_t3 mask */
#define XNANDPS_SET_CYCLES_SET_T4_MASK			0x0001C000 /**< Set
						cycles set_t4 mask */
#define XNANDPS_SET_CYCLES_SET_T5_MASK			0x000E0000 /**< Set
						cycles set_t5 mask */
#define XNANDPS_SET_CYCLES_SET_T6_MASK			0x00F00000 /**< Set
						cycles set_t6 mask */

#define XNANDPS_SET_CYCLES_SET_T0_SHIFT	0	/**< Set cycles set_t0
							  shift */
#define XNANDPS_SET_CYCLES_SET_T1_SHIFT	4	/**< Set cycles set_t1
							  shift */
#define XNANDPS_SET_CYCLES_SET_T2_SHIFT	8	/**< Set cycles set_t2
							  shift */
#define XNANDPS_SET_CYCLES_SET_T3_SHIFT	11	/**< Set cycles set_t3
							  shift */
#define XNANDPS_SET_CYCLES_SET_T4_SHIFT	14	/**< Set cycles set_t4
							  shift */
#define XNANDPS_SET_CYCLES_SET_T5_SHIFT	17	/**< Set cycles set_t5
							  shift */
#define XNANDPS_SET_CYCLES_SET_T6_SHIFT	20	/**< Set cycles set_t6
							  shift */

/* @} */

/** @name Set opmode register bit definitions and masks
 *  @{
 */
#define XNANDPS_SET_OPMODE_SET_MW_MASK			0x00000003 /**< Set
					opmode set memory width mask */
#define XNANDPS_SET_OPMODE_SET_RD_SYNC_MASK		0x00000004 /**< Set
					opmode set rd_sync mask */
#define XNANDPS_SET_OPMODE_SET_RD_BL_MASK		0x00000038 /**< Set
					opmode set rd_bl mask */
#define XNANDPS_SET_OPMODE_SET_WR_SYNC_MASK		0x00000040 /**< Set
					opmode set wr_sync mask */
#define XNANDPS_SET_OPMODE_SET_WR_BL_MASK		0x00000380 /**< Set
					opmode set wr_bl mask */
#define XNANDPS_SET_OPMODE_SET_BAA_MASK		0x00000400 /**< Set
					opmode set baa mask */
#define XNANDPS_SET_OPMODE_SET_ADV_MASK		0x00000800 /**< Set
					opmode set adv mask */
#define XNANDPS_SET_OPMODE_SET_BLS_MASK		0x00001000 /**< Set
					opmode set bls mask */
#define XNANDPS_SET_OPMODE_SET_BURST_ALIGN_MASK	0x0000E000 /**< Set
					opmode set burst align mask */

#define XNANDPS_SET_OPMODE_MW_8_BITS			0x0	/**< Set opmode
				memory width value for 8-bit flash */
#define XNANDPS_SET_OPMODE_MW_16_BITS			0x1	/**< Set opmode
				memory width value for 16-bit flash */
#define XNANDPS_SET_OPMODE_MW_32_BITS			0x2	/**< Set opmode
				memory width value for 32-bit flash */
/* @} */

/** @name Refresh period register bit definitions and masks
 *  @{
 */
#define XNANDPS_REFRESH_PERIOD_0_MASK			0x0000000F
/**< Interface 0 refresh period mask */
#define XNANDPS_REFRESH_PERIOD_1_MASK			0x0000000F
/**< Interface 1 refresh period mask */
/* @} */

/** @name Opmode register bit definitions and masks
 *  @{
 */
#define XNANDPS_OPMODE_MW_MASK				0x00000003
/**< Opmode Memory width mask */
#define XNANDPS_OPMODE_RD_SYNC_MASK			0x00000004
/**< Opmode rd_sync mask */
#define XNANDPS_OPMODE_RD_BL_MASK			0x00000038
/**< Opmode rd_bl mask */
#define XNANDPS_OPMODE_WR_SYNC_MASK			0x00000040
/**< Opmode wr_sync mask */
#define XNANDPS_OPMODE_WR_BL_MASK			0x00000380
/**< Opmode BAA mask */
#define XNANDPS_OPMODE_BAA_MASK			0x00000400
/**< Opmode ADV mask */
#define XNANDPS_OPMODE_ADV_MASK			0x00000800
/**< Opmode BLS mask */
#define XNANDPS_OPMODE_BLS_MASK			0x00001000
/**< Opmode Burst align mask */
#define XNANDPS_OPMODE_BURST_ALIGN_MASK		0x0000E000
/**< Opmode Address mask */
#define XNANDPS_OPMODE_ADDRESS_MASK			0x00FF0000
/**< Opmode Address match mask */
#define XNANDPS_OPMODE_ADDRESS_MATCH_MASK		0xFF000000
/* @} */

/** @name User status register bit definitions and masks
 *  @{
 */
#define XNANDPS_USER_STATUS_MASK			0x000000FF /**< User
							status mask */
/* @} */

/** @name User config register bit definitions and masks
 *  @{
 */
#define XNANDPS_USER_CONFIG_MASK			0x000000FF /**< User
							config mask */
/* @} */

/** @name ECC status register bit definitions and masks
 *  @{
 */
#define XNANDPS_ECC_STATUS_RAW_INT_STATUS_MASK		0x0000003F /**< Ecc
						status raw_int_status mask */
#define XNANDPS_ECC_STATUS_MASK			0x00000040 /**< Ecc
						status ecc_status mask */
#define XNANDPS_ECC_LAST_MASK				0x00000180 /**< Ecc
						status ecc_last mask */
#define XNANDPS_ECC_READ_NOT_WRITE_MASK		0x00000200 /**< Ecc
					status ecc_read_not_write mask */
#define XNANDPS_ECC_VALID_MASK				0x00007C00 /**< Ecc
						status ecc_valid mask */
#define XNANDPS_ECC_FAIL_MASK				0x000F8000 /**< Ecc
						status ecc_fail mask */
#define XNANDPS_ECC_CAN_CORRECT_MASK			0x01F00000 /**< Ecc
					status ecc_can_correct mask */
#define XNANDPS_ECC_READ_MASK				0x37000000 /**< Ecc
						status ecc_read mask */
/* @} */

/** @name ECC mem config register bit definitions and masks and shifts
 *  @{
 */
#define XNANDPS_ECC_MEMCFG_PAGE_SIZE_MASK		0x00000003
/**< Ecc cfg page_size mask */
#define XNANDPS_ECC_MEMCFG_ECC_MODE_MASK		0x0000000C
/**< Ecc cfg ecc_mode mask */
#define XNANDPS_ECC_MEMCFG_ECC_READ_END_MASK		0x00000010
/**< Ecc cfg ecc_read_end mask */
#define XNANDPS_ECC_MEMCFG_ECC_JUMP_MASK		0x00000060
/**< Ecc cfg ecc_jump mask */
#define XNANDPS_ECC_MEMCFG_IGNORE_ADD8_MASK		0x00000080
/**< Ecc cfg ecc_ignore_add_eight mask */
#define XNANDPS_ECC_MEMCFG_ECC_INT_PASS_MASK		0x00000100
/**< Ecc cfg ecc_int_pass mask */
#define XNANDPS_ECC_MEMCFG_ECC_INT_ABORT_MASK		0x00000200
/**< Ecc cfg ecc_int_abort mask */
#define XNANDPS_ECC_MEMCFG_ECC_EXTRA_BLOCK_MASK	0x00000400
/**< Ecc cfg ecc_extra_block mask */
#define XNANDPS_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_MASK	0x00001800
/**< Ecc cfg ecc_extra_block_size mask */

#define XNANDPS_ECC_MEMCFG_PAGE_SIZE_SHIFT		0
/**< Ecc cfg page_size shift */
#define XNANDPS_ECC_MEMCFG_ECC_MODE_SHIFT		2
/**< Ecc cfg ecc_mode shift */
#define XNANDPS_ECC_MEMCFG_ECC_READ_END_SHIFT		4
/**< Ecc cfg ecc_read_end shift */
#define XNANDPS_ECC_MEMCFG_ECC_JUMP_SHIFT		5
/**< Ecc cfg ecc_jump shift */
#define XNANDPS_ECC_MEMCFG_IGNORE_ADD8_SHIFT		7
/**< Ecc cfg ecc_ignore_add_eight shift */
#define XNANDPS_ECC_MEMCFG_ECC_INT_PASS_SHIFT		8
/**< Ecc cfg ecc_int_pass shift */
#define XNANDPS_ECC_MEMCFG_ECC_INT_ABORT_SHIFT		9
/**< Ecc cfg ecc_int_abort shift */
#define XNANDPS_ECC_MEMCFG_ECC_EXTRA_BLOCK_SHIFT	10
/**< Ecc cfg ecc_extra_block shift */
#define XNANDPS_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_SHIFT	11
/**< Ecc cfg ecc_extra_block_size shift */

#define XNANDPS_ECC_MEMCFG_PAGE_SIZE_512		0x1	/**< ECC cfg
					page size value for 512 byte page */
#define XNANDPS_ECC_MEMCFG_PAGE_SIZE_1024		0x2	/**< ECC cfg
					page size value for 1024 byte page */
#define XNANDPS_ECC_MEMCFG_PAGE_SIZE_2048		0x3	/**< ECC cfg
					page size value for 2048 byte page */
/* @} */
/* @} */

/** @name ECC mem command1 register bit definitions and masks and shifts
 *  @{
 */
#define XNANDPS_ECC_MEMCOMMAND1_WR_CMD_MASK		0x000000FF
/**< Ecc command 1 nand_wr_cmd mask */
#define XNANDPS_ECC_MEMCOMMAND1_RD_CMD_MASK		0x0000FF00
/**< Ecc command 1 nand_rd_cmd mask */
#define XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_MASK	0x00FF0000
/**< Ecc command 1 nand_rd_cmd_end mask */
#define XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_VALID_MASK	0x01000000
/**< Ecc command 1 nand_rd_cmd_end_valid mask */

#define XNANDPS_ECC_MEMCOMMAND1_WR_CMD_SHIFT		0
/**< Ecc command 1 nand_wr_cmd shift */
#define XNANDPS_ECC_MEMCOMMAND1_RD_CMD_SHIFT		8
/**< Ecc command 1 nand_rd_cmd shift */
#define XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_SHIFT	16
/**< Ecc command 1 nand_rd_cmd_end shift */
#define XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_VALID_SHIFT	24
/**< Ecc command 1 nand_rd_cmd_end_valid shift */
/* @} */

/** @name ECC mem command2 register bit definitions and masks
 *  @{
 */
#define XNANDPS_ECC_MEMCOMMAND2_WR_COL_CHANGE_MASK		0x000000FF
/**< Ecc command2 nand_wr_col_change mask */
#define XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_MASK		0x0000FF00
/**< Ecc command2 nand_rd_col_change mask */
#define XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_MASK		0x00FF0000
/**< Ecc command2 nand_rd_col_change_end mask */
#define XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_MASK	0x00FF0000
/**< Ecc command2 nand_rd_col_change_end_valid mask */


#define XNANDPS_ECC_MEMCOMMAND2_WR_COL_CHANGE_SHIFT		0
/**< Ecc command2 nand_wr_col_change shift */
#define XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_SHIFT		8
/**< Ecc command2 nand_rd_col_change shift */
#define XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_SHIFT	16
/**< Ecc command2 nand_rd_col_change_end shift */
#define XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_SHIFT	24
/**< Ecc command2 nand_rd_col_change_end_valid shift */
/* @} */

/** @name ECC value register bit definitions and masks
 *  @{
 */
#define XNANDPS_ECC_VALUE_MASK				0x00FFFFFF
/**< Ecc value ecc_value mask */
#define XNANDPS_ECC_VALUE_CORRECT_MASK			0x08000000
/**< Ecc value ecc_correct mask */
#define XNANDPS_ECC_VALUE_FAIL_MASK			0x10000000
/**< Ecc value ecc_fail mask */
#define XNANDPS_ECC_VALUE_READ_MASK			0x20000000
/**< Ecc value ecc_read mask */
#define XNANDPS_ECC_VALUE_VALID_MASK			0x40000000
/**< Ecc value ecc_valid mask */
#define XNANDPS_ECC_VALUE_INT_MASK			0x80000000
/**< Ecc value ecc_int mask */
/* @} */

/** @name Peripheral ID register bit definitions and masks
 *  @{
 */
#define XNANDPS_PERIPH_ID_PART_NUM_MASK		0x00000FFF
/**< Peripheral ID part_num mask */
#define XNANDPS_PERIPH_ID_DESIGNER_ID_MASK		0x000FF000
/**< Peripheral ID designed id mask */
#define XNANDPS_PERIPH_ID_REVISION_MASK		0x00F00000
/**< Peripheral ID revision mask */
#define XNANDPS_PERIPH_ID_INTG_CFG_MASK		0x01000000
/**< Peripheral ID integration_cfg mask */
/* @} */

/** @name Peripheral ID register bit definitions and masks
 *  @{
 */
#define XNANDPS_PCELL_ID_MASK				0x000000FF
/**< Primecell identification register mask */
/* @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XNandPs_ReadReg Xil_In32	/**< XNandPs Register register */
#define XNandPs_WriteReg Xil_Out32	/**< XNandPs register write */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
