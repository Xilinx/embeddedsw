/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * CORE_AXI_WR_PROTECT register
 */
#define XSDFEC_CORE_AXI_WR_PROTECT_ADDR 0x00000 // Register offset from BaseAddress
#define XSDFEC_CORE_AXI_WR_PROTECT_LSB           0 // LSB for CORE_AXI_WR_PROTECT field
#define XSDFEC_CORE_AXI_WR_PROTECT_MASK 0x00000001 // Bit mask for CORE_AXI_WR_PROTECT field
/**
 * CORE_CODE_WR_PROTECT register
 */
#define XSDFEC_CORE_CODE_WR_PROTECT_ADDR 0x00004 // Register offset from BaseAddress
#define XSDFEC_CORE_CODE_WR_PROTECT_LSB           0 // LSB for CORE_CODE_WR_PROTECT field
#define XSDFEC_CORE_CODE_WR_PROTECT_MASK 0x00000001 // Bit mask for CORE_CODE_WR_PROTECT field
/**
 * CORE_ACTIVE register
 */
#define XSDFEC_CORE_ACTIVE_ADDR 0x00008 // Register offset from BaseAddress
#define XSDFEC_CORE_ACTIVE_LSB           0 // LSB for CORE_ACTIVE field
#define XSDFEC_CORE_ACTIVE_MASK 0x00000001 // Bit mask for CORE_ACTIVE field
/**
 * CORE_AXIS_WIDTH register
 */
#define XSDFEC_CORE_AXIS_WIDTH_ADDR 0x0000c // Register offset from BaseAddress
#define XSDFEC_CORE_AXIS_WIDTH_DIN_LSB           0 // LSB for CORE_AXIS_WIDTH_DIN field
#define XSDFEC_CORE_AXIS_WIDTH_DIN_MASK 0x00000003 // Bit mask for CORE_AXIS_WIDTH_DIN field
#define XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_LSB           2 // LSB for CORE_AXIS_WIDTH_DIN_WORDS field
#define XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK 0x00000004 // Bit mask for CORE_AXIS_WIDTH_DIN_WORDS field
#define XSDFEC_CORE_AXIS_WIDTH_DOUT_LSB           3 // LSB for CORE_AXIS_WIDTH_DOUT field
#define XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK 0x00000018 // Bit mask for CORE_AXIS_WIDTH_DOUT field
#define XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_LSB           5 // LSB for CORE_AXIS_WIDTH_DOUT_WORDS field
#define XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK 0x00000020 // Bit mask for CORE_AXIS_WIDTH_DOUT_WORDS field
/**
 * CORE_AXIS_ENABLE register
 */
#define XSDFEC_CORE_AXIS_ENABLE_ADDR 0x00010 // Register offset from BaseAddress
#define XSDFEC_CORE_AXIS_ENABLE_CTRL_LSB           0 // LSB for CORE_AXIS_ENABLE_CTRL field
#define XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK 0x00000001 // Bit mask for CORE_AXIS_ENABLE_CTRL field
#define XSDFEC_CORE_AXIS_ENABLE_DIN_LSB           1 // LSB for CORE_AXIS_ENABLE_DIN field
#define XSDFEC_CORE_AXIS_ENABLE_DIN_MASK 0x00000002 // Bit mask for CORE_AXIS_ENABLE_DIN field
#define XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_LSB           2 // LSB for CORE_AXIS_ENABLE_DIN_WORDS field
#define XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK 0x00000004 // Bit mask for CORE_AXIS_ENABLE_DIN_WORDS field
#define XSDFEC_CORE_AXIS_ENABLE_STATUS_LSB           3 // LSB for CORE_AXIS_ENABLE_STATUS field
#define XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK 0x00000008 // Bit mask for CORE_AXIS_ENABLE_STATUS field
#define XSDFEC_CORE_AXIS_ENABLE_DOUT_LSB           4 // LSB for CORE_AXIS_ENABLE_DOUT field
#define XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK 0x00000010 // Bit mask for CORE_AXIS_ENABLE_DOUT field
#define XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_LSB           5 // LSB for CORE_AXIS_ENABLE_DOUT_WORDS field
#define XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK 0x00000020 // Bit mask for CORE_AXIS_ENABLE_DOUT_WORDS field
/**
 * CORE_ORDER register
 */
#define XSDFEC_CORE_ORDER_ADDR 0x00018 // Register offset from BaseAddress
#define XSDFEC_CORE_ORDER_LSB           0 // LSB for CORE_ORDER field
#define XSDFEC_CORE_ORDER_MASK 0x00000001 // Bit mask for CORE_ORDER field
/**
 * CORE_ISR register
 */
#define XSDFEC_CORE_ISR_ADDR 0x0001c // Register offset from BaseAddress
#define XSDFEC_CORE_ISR_LSB           0 // LSB for CORE_ISR field
#define XSDFEC_CORE_ISR_MASK 0x0000003f // Bit mask for CORE_ISR field
/**
 * CORE_IER register
 */
#define XSDFEC_CORE_IER_ADDR 0x00020 // Register offset from BaseAddress
#define XSDFEC_CORE_IER_LSB           0 // LSB for CORE_IER field
#define XSDFEC_CORE_IER_MASK 0x0000003f // Bit mask for CORE_IER field
/**
 * CORE_IDR register
 */
#define XSDFEC_CORE_IDR_ADDR 0x00024 // Register offset from BaseAddress
#define XSDFEC_CORE_IDR_LSB           0 // LSB for CORE_IDR field
#define XSDFEC_CORE_IDR_MASK 0x0000003f // Bit mask for CORE_IDR field
/**
 * CORE_IMR register
 */
#define XSDFEC_CORE_IMR_ADDR 0x00028 // Register offset from BaseAddress
#define XSDFEC_CORE_IMR_LSB           0 // LSB for CORE_IMR field
#define XSDFEC_CORE_IMR_MASK 0x0000003f // Bit mask for CORE_IMR field
/**
 * CORE_ECC_ISR register
 */
#define XSDFEC_CORE_ECC_ISR_ADDR 0x0002c // Register offset from BaseAddress
#define XSDFEC_CORE_ECC_ISR_LSB           0 // LSB for CORE_ECC_ISR field
#define XSDFEC_CORE_ECC_ISR_MASK 0x003fffff // Bit mask for CORE_ECC_ISR field
/**
 * CORE_ECC_IER register
 */
#define XSDFEC_CORE_ECC_IER_ADDR 0x00030 // Register offset from BaseAddress
#define XSDFEC_CORE_ECC_IER_LSB           0 // LSB for CORE_ECC_IER field
#define XSDFEC_CORE_ECC_IER_MASK 0x003fffff // Bit mask for CORE_ECC_IER field
/**
 * CORE_ECC_IDR register
 */
#define XSDFEC_CORE_ECC_IDR_ADDR 0x00034 // Register offset from BaseAddress
#define XSDFEC_CORE_ECC_IDR_LSB           0 // LSB for CORE_ECC_IDR field
#define XSDFEC_CORE_ECC_IDR_MASK 0x003fffff // Bit mask for CORE_ECC_IDR field
/**
 * CORE_ECC_IMR register
 */
#define XSDFEC_CORE_ECC_IMR_ADDR 0x00038 // Register offset from BaseAddress
#define XSDFEC_CORE_ECC_IMR_LSB           0 // LSB for CORE_ECC_IMR field
#define XSDFEC_CORE_ECC_IMR_MASK 0x003fffff // Bit mask for CORE_ECC_IMR field
/**
 * CORE_BYPASS register
 */
#define XSDFEC_CORE_BYPASS_ADDR 0x0003c // Register offset from BaseAddress
#define XSDFEC_CORE_BYPASS_LSB           0 // LSB for CORE_BYPASS field
#define XSDFEC_CORE_BYPASS_MASK 0x00000001 // Bit mask for CORE_BYPASS field
/**
 * CORE_VERSION register
 */
#define XSDFEC_CORE_VERSION_ADDR 0x00050 // Register offset from BaseAddress
#define XSDFEC_CORE_VERSION_LSB           0 // LSB for CORE_VERSION field
#define XSDFEC_CORE_VERSION_MASK 0xffffffff // Bit mask for CORE_VERSION field
/**
 * TURBO register
 */
#define XSDFEC_TURBO_ADDR 0x00100 // Register offset from BaseAddress
#define XSDFEC_TURBO_ALG_LSB           0 // LSB for TURBO_ALG field
#define XSDFEC_TURBO_ALG_MASK 0x00000001 // Bit mask for TURBO_ALG field
#define XSDFEC_TURBO_SCALE_FACTOR_LSB           8 // LSB for TURBO_SCALE_FACTOR field
#define XSDFEC_TURBO_SCALE_FACTOR_MASK 0x00000f00 // Bit mask for TURBO_SCALE_FACTOR field
/**
 * LDPC_CODE_REG0 register
 */
#define XSDFEC_LDPC_CODE_REG0_ADDR_BASE 0x02000 // Register base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG0_ADDR_HIGH 0x027f0 // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG0_DEPTH         128 // Register word depth
#define XSDFEC_LDPC_CODE_REG0_STEP           16
#define XSDFEC_LDPC_CODE_REG0_N_LSB           0 // LSB for LDPC_CODE_REG0_N field
#define XSDFEC_LDPC_CODE_REG0_N_MASK 0x0000ffff // Bit mask for LDPC_CODE_REG0_N field
#define XSDFEC_LDPC_CODE_REG0_K_LSB          16 // LSB for LDPC_CODE_REG0_K field
#define XSDFEC_LDPC_CODE_REG0_K_MASK 0x7fff0000 // Bit mask for LDPC_CODE_REG0_K field
/**
 * LDPC_CODE_REG1 register
 */
#define XSDFEC_LDPC_CODE_REG1_ADDR_BASE 0x02004 // Register base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG1_ADDR_HIGH 0x027f4 // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG1_DEPTH         128 // Register word depth
#define XSDFEC_LDPC_CODE_REG1_STEP           16
#define XSDFEC_LDPC_CODE_REG1_PSIZE_LSB           0 // LSB for LDPC_CODE_REG1_PSIZE field
#define XSDFEC_LDPC_CODE_REG1_PSIZE_MASK 0x000003ff // Bit mask for LDPC_CODE_REG1_PSIZE field
#define XSDFEC_LDPC_CODE_REG1_NO_PACKING_LSB          10 // LSB for LDPC_CODE_REG1_NO_PACKING field
#define XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK 0x00000400 // Bit mask for LDPC_CODE_REG1_NO_PACKING field
#define XSDFEC_LDPC_CODE_REG1_NM_LSB          11 // LSB for LDPC_CODE_REG1_NM field
#define XSDFEC_LDPC_CODE_REG1_NM_MASK 0x000ff800 // Bit mask for LDPC_CODE_REG1_NM field
/**
 * LDPC_CODE_REG2 register
 */
#define XSDFEC_LDPC_CODE_REG2_ADDR_BASE 0x02008 // Register base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG2_ADDR_HIGH 0x027f8 // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG2_DEPTH         128 // Register word depth
#define XSDFEC_LDPC_CODE_REG2_STEP           16
#define XSDFEC_LDPC_CODE_REG2_NLAYERS_LSB           0 // LSB for LDPC_CODE_REG2_NLAYERS field
#define XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK 0x000001ff // Bit mask for LDPC_CODE_REG2_NLAYERS field
#define XSDFEC_LDPC_CODE_REG2_NMQC_LSB           9 // LSB for LDPC_CODE_REG2_NMQC field
#define XSDFEC_LDPC_CODE_REG2_NMQC_MASK 0x000ffe00 // Bit mask for LDPC_CODE_REG2_NMQC field
#define XSDFEC_LDPC_CODE_REG2_NORM_TYPE_LSB          20 // LSB for LDPC_CODE_REG2_NORM_TYPE field
#define XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK 0x00100000 // Bit mask for LDPC_CODE_REG2_NORM_TYPE field
#define XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_LSB          21 // LSB for LDPC_CODE_REG2_SPECIAL_QC field
#define XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK 0x00200000 // Bit mask for LDPC_CODE_REG2_SPECIAL_QC field
#define XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_LSB          22 // LSB for LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK field
#define XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK 0x00400000 // Bit mask for LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK field
#define XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_LSB          23 // LSB for LDPC_CODE_REG2_MAX_SCHEDULE field
#define XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK 0x01800000 // Bit mask for LDPC_CODE_REG2_MAX_SCHEDULE field
/**
 * LDPC_CODE_REG3 register
 */
#define XSDFEC_LDPC_CODE_REG3_ADDR_BASE 0x0200c // Register base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG3_ADDR_HIGH 0x027fc // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG3_DEPTH         128 // Register word depth
#define XSDFEC_LDPC_CODE_REG3_STEP           16
#define XSDFEC_LDPC_CODE_REG3_SC_OFF_LSB           0 // LSB for LDPC_CODE_REG3_SC_OFF field
#define XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK 0x000000ff // Bit mask for LDPC_CODE_REG3_SC_OFF field
#define XSDFEC_LDPC_CODE_REG3_LA_OFF_LSB           8 // LSB for LDPC_CODE_REG3_LA_OFF field
#define XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK 0x0000ff00 // Bit mask for LDPC_CODE_REG3_LA_OFF field
#define XSDFEC_LDPC_CODE_REG3_QC_OFF_LSB          16 // LSB for LDPC_CODE_REG3_QC_OFF field
#define XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK 0x07ff0000 // Bit mask for LDPC_CODE_REG3_QC_OFF field
/**
 * LDPC_SC_TABLE register
 */
#define XSDFEC_LDPC_SC_TABLE_ADDR_BASE 0x10000 // Register base address relative to BaseAddress
#define XSDFEC_LDPC_SC_TABLE_ADDR_HIGH 0x103fc // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_SC_TABLE_DEPTH         256 // Register word depth
#define XSDFEC_LDPC_SC_TABLE_STEP            4
#define XSDFEC_LDPC_SC_TABLE_SCALE_0_LSB           0 // LSB for LDPC_SC_TABLE_SCALE_0 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_0_MASK 0x0000000f // Bit mask for LDPC_SC_TABLE_SCALE_0 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_1_LSB           4 // LSB for LDPC_SC_TABLE_SCALE_1 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_1_MASK 0x000000f0 // Bit mask for LDPC_SC_TABLE_SCALE_1 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_2_LSB           8 // LSB for LDPC_SC_TABLE_SCALE_2 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_2_MASK 0x00000f00 // Bit mask for LDPC_SC_TABLE_SCALE_2 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_3_LSB          12 // LSB for LDPC_SC_TABLE_SCALE_3 field
#define XSDFEC_LDPC_SC_TABLE_SCALE_3_MASK 0x0000f000 // Bit mask for LDPC_SC_TABLE_SCALE_3 field
/**
 * LDPC_LA_TABLE register
 */
#define XSDFEC_LDPC_LA_TABLE_ADDR_BASE 0x18000 // Register base address relative to BaseAddress
#define XSDFEC_LDPC_LA_TABLE_ADDR_HIGH 0x18ffc // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_LA_TABLE_DEPTH        1024 // Register word depth
#define XSDFEC_LDPC_LA_TABLE_STEP            4
#define XSDFEC_LDPC_LA_TABLE_CPLD_LSB           0 // LSB for LDPC_LA_TABLE_CPLD field
#define XSDFEC_LDPC_LA_TABLE_CPLD_MASK 0x0000007f // Bit mask for LDPC_LA_TABLE_CPLD field
#define XSDFEC_LDPC_LA_TABLE_ACCUM_LSB           7 // LSB for LDPC_LA_TABLE_ACCUM field
#define XSDFEC_LDPC_LA_TABLE_ACCUM_MASK 0x00000080 // Bit mask for LDPC_LA_TABLE_ACCUM field
#define XSDFEC_LDPC_LA_TABLE_STALL_LSB           8 // LSB for LDPC_LA_TABLE_STALL field
#define XSDFEC_LDPC_LA_TABLE_STALL_MASK 0x0000ff00 // Bit mask for LDPC_LA_TABLE_STALL field
/**
 * LDPC_QC_TABLE register
 */
#define XSDFEC_LDPC_QC_TABLE_ADDR_BASE 0x20000 // Register base address relative to BaseAddress
#define XSDFEC_LDPC_QC_TABLE_ADDR_HIGH 0x27ffc // Register upper address relative to BaseAddress
#define XSDFEC_LDPC_QC_TABLE_DEPTH        8192 // Register word depth
#define XSDFEC_LDPC_QC_TABLE_STEP            4
#define XSDFEC_LDPC_QC_TABLE_COLUMN_LSB           0 // LSB for LDPC_QC_TABLE_COLUMN field
#define XSDFEC_LDPC_QC_TABLE_COLUMN_MASK 0x000000ff // Bit mask for LDPC_QC_TABLE_COLUMN field
#define XSDFEC_LDPC_QC_TABLE_ROTATION_LSB           8 // LSB for LDPC_QC_TABLE_ROTATION field
#define XSDFEC_LDPC_QC_TABLE_ROTATION_MASK 0x0001ff00 // Bit mask for LDPC_QC_TABLE_ROTATION field
#define XSDFEC_LDPC_QC_TABLE_FIRST_USE_LSB          17 // LSB for LDPC_QC_TABLE_FIRST_USE field
#define XSDFEC_LDPC_QC_TABLE_FIRST_USE_MASK 0x00020000 // Bit mask for LDPC_QC_TABLE_FIRST_USE field
#define XSDFEC_LDPC_QC_TABLE_IS_PARITY_LSB          18 // LSB for LDPC_QC_TABLE_IS_PARITY field
#define XSDFEC_LDPC_QC_TABLE_IS_PARITY_MASK 0x00040000 // Bit mask for LDPC_QC_TABLE_IS_PARITY field
