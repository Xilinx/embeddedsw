// d52cbaca0ef8cf4fd3d6354deb5066970fb6511d02d18d15835e6014ed847fb0
// (c) Copyright 2016-2018 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.

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
#define XSDFEC_LDPC_CODE_REG0_ADDR_BASE 0x02000 // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG0_ADDR_HIGH 0x021fc // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG0_DEPTH         508 // Register depth
#define XSDFEC_LDPC_CODE_REG0_STEP           16
#define XSDFEC_LDPC_CODE_REG0_N_LSB           0 // LSB for LDPC_CODE_REG0_N field
#define XSDFEC_LDPC_CODE_REG0_N_MASK 0x0000ffff // Bit mask for LDPC_CODE_REG0_N field
#define XSDFEC_LDPC_CODE_REG0_K_LSB          16 // LSB for LDPC_CODE_REG0_K field
#define XSDFEC_LDPC_CODE_REG0_K_MASK 0x7fff0000 // Bit mask for LDPC_CODE_REG0_K field
/**
 * LDPC_CODE_REG1 register
 */
#define XSDFEC_LDPC_CODE_REG1_ADDR_BASE 0x02004 // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG1_ADDR_HIGH 0x02200 // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG1_DEPTH         508 // Register depth
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
#define XSDFEC_LDPC_CODE_REG2_ADDR_BASE 0x02008 // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG2_ADDR_HIGH 0x02204 // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG2_DEPTH         508 // Register depth
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
#define XSDFEC_LDPC_CODE_REG3_ADDR_BASE 0x0200c // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG3_ADDR_HIGH 0x02208 // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_CODE_REG3_DEPTH         508 // Register depth
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
#define XSDFEC_LDPC_SC_TABLE_ADDR_BASE 0x10000 // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_SC_TABLE_ADDR_HIGH 0x10100 // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_SC_TABLE_DEPTH         256 // Register depth
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
#define XSDFEC_LDPC_LA_TABLE_ADDR_BASE 0x18000 // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_LA_TABLE_ADDR_HIGH 0x18400 // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_LA_TABLE_DEPTH        1024 // Register depth
#define XSDFEC_LDPC_LA_TABLE_CPLD_LSB           0 // LSB for LDPC_LA_TABLE_CPLD field
#define XSDFEC_LDPC_LA_TABLE_CPLD_MASK 0x0000007f // Bit mask for LDPC_LA_TABLE_CPLD field
#define XSDFEC_LDPC_LA_TABLE_ACCUM_LSB           7 // LSB for LDPC_LA_TABLE_ACCUM field
#define XSDFEC_LDPC_LA_TABLE_ACCUM_MASK 0x00000080 // Bit mask for LDPC_LA_TABLE_ACCUM field
#define XSDFEC_LDPC_LA_TABLE_STALL_LSB           8 // LSB for LDPC_LA_TABLE_STALL field
#define XSDFEC_LDPC_LA_TABLE_STALL_MASK 0x0000ff00 // Bit mask for LDPC_LA_TABLE_STALL field
/**
 * LDPC_QC_TABLE register
 */
#define XSDFEC_LDPC_QC_TABLE_ADDR_BASE 0x20000 // Resgiter base address relative to BaseAddress
#define XSDFEC_LDPC_QC_TABLE_ADDR_HIGH 0x22000 // Resgiter upper address relative to BaseAddress
#define XSDFEC_LDPC_QC_TABLE_DEPTH        8192 // Register depth
#define XSDFEC_LDPC_QC_TABLE_COLUMN_LSB           0 // LSB for LDPC_QC_TABLE_COLUMN field
#define XSDFEC_LDPC_QC_TABLE_COLUMN_MASK 0x000000ff // Bit mask for LDPC_QC_TABLE_COLUMN field
#define XSDFEC_LDPC_QC_TABLE_ROTATION_LSB           8 // LSB for LDPC_QC_TABLE_ROTATION field
#define XSDFEC_LDPC_QC_TABLE_ROTATION_MASK 0x0001ff00 // Bit mask for LDPC_QC_TABLE_ROTATION field
#define XSDFEC_LDPC_QC_TABLE_FIRST_USE_LSB          17 // LSB for LDPC_QC_TABLE_FIRST_USE field
#define XSDFEC_LDPC_QC_TABLE_FIRST_USE_MASK 0x00020000 // Bit mask for LDPC_QC_TABLE_FIRST_USE field
#define XSDFEC_LDPC_QC_TABLE_IS_PARITY_LSB          18 // LSB for LDPC_QC_TABLE_IS_PARITY field
#define XSDFEC_LDPC_QC_TABLE_IS_PARITY_MASK 0x00040000 // Bit mask for LDPC_QC_TABLE_IS_PARITY field
