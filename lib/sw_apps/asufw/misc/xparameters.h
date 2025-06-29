/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
#ifndef XPARAMETERS_H   /* prevent circular inclusions */
#define XPARAMETERS_H   /* by using protection macros */

#define XPAR_CPU_ID 4U

/* Definitions for PMC Microblaze */
#define XPAR_MICROBLAZE_ADDR_SIZE 32
#define XPAR_MICROBLAZE_ADDR_TAG_BITS 0
#define XPAR_MICROBLAZE_ALLOW_DCACHE_WR 1
#define XPAR_MICROBLAZE_ALLOW_ICACHE_WR 1
#define XPAR_MICROBLAZE_AREA_OPTIMIZED 0
#define XPAR_MICROBLAZE_ASYNC_INTERRUPT 1
#define XPAR_MICROBLAZE_ASYNC_WAKEUP 3
#define XPAR_MICROBLAZE_AVOID_PRIMITIVES 0
#define XPAR_MICROBLAZE_BASE_VECTORS 0x00000000EBE40000
#define XPAR_MICROBLAZE_BRANCH_TARGET_CACHE_SIZE 0
#define XPAR_MICROBLAZE_CACHE_BYTE_SIZE 8192
#define XPAR_MICROBLAZE_DADDR_SIZE 32
#define XPAR_MICROBLAZE_DATA_SIZE 32
#define XPAR_MICROBLAZE_DCACHE_ADDR_TAG 0
#define XPAR_MICROBLAZE_DCACHE_ALWAYS_USED 0
#define XPAR_MICROBLAZE_DCACHE_BASEADDR 0x00000000
#define XPAR_MICROBLAZE_DCACHE_BYTE_SIZE 8192
#define XPAR_MICROBLAZE_DCACHE_DATA_WIDTH 0
#define XPAR_MICROBLAZE_DCACHE_FORCE_TAG_LUTRAM 0
#define XPAR_MICROBLAZE_DCACHE_HIGHADDR 0x3FFFFFFF
#define XPAR_MICROBLAZE_DCACHE_LINE_LEN 4
#define XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK 0
#define XPAR_MICROBLAZE_DCACHE_VICTIMS 0
#define XPAR_MICROBLAZE_DC_AXI_MON 0
#define XPAR_MICROBLAZE_DEBUG_COUNTER_WIDTH 32
#define XPAR_MICROBLAZE_DEBUG_ENABLED 1
#define XPAR_MICROBLAZE_DEBUG_EVENT_COUNTERS 5
#define XPAR_MICROBLAZE_DEBUG_EXTERNAL_TRACE 0
#define XPAR_MICROBLAZE_DEBUG_INTERFACE 0
#define XPAR_MICROBLAZE_DEBUG_LATENCY_COUNTERS 1
#define XPAR_MICROBLAZE_DEBUG_PROFILE_SIZE 0
#define XPAR_MICROBLAZE_DEBUG_TRACE_SIZE 8192
#define XPAR_MICROBLAZE_DIV_ZERO_EXCEPTION 1
#define XPAR_MICROBLAZE_DP_AXI_MON 0
#define XPAR_MICROBLAZE_DYNAMIC_BUS_SIZING 0
#define XPAR_MICROBLAZE_D_AXI 1
#define XPAR_MICROBLAZE_D_LMB 1
#define XPAR_MICROBLAZE_D_LMB_MON 0
#define XPAR_MICROBLAZE_ECC_USE_CE_EXCEPTION 0
#define XPAR_MICROBLAZE_EDGE_IS_POSITIVE 1
#define XPAR_MICROBLAZE_ENABLE_DISCRETE_PORTS 0
#define XPAR_MICROBLAZE_ENDIANNESS 1
#define XPAR_MICROBLAZE_FAULT_TOLERANT 1
#define XPAR_MICROBLAZE_FPU_EXCEPTION 0
#define XPAR_MICROBLAZE_FREQ 320000000
#define XPAR_MICROBLAZE_FSL_EXCEPTION 0
#define XPAR_MICROBLAZE_FSL_LINKS 0
#define XPAR_MICROBLAZE_IADDR_SIZE 32
#define XPAR_MICROBLAZE_ICACHE_ALWAYS_USED 0
#define XPAR_MICROBLAZE_ICACHE_BASEADDR 0x00000000
#define XPAR_MICROBLAZE_ICACHE_DATA_WIDTH 0
#define XPAR_MICROBLAZE_ICACHE_FORCE_TAG_LUTRAM 0
#define XPAR_MICROBLAZE_ICACHE_HIGHADDR 0x3FFFFFFF
#define XPAR_MICROBLAZE_ICACHE_LINE_LEN 4
#define XPAR_MICROBLAZE_ICACHE_STREAMS 0
#define XPAR_MICROBLAZE_ICACHE_VICTIMS 0
#define XPAR_MICROBLAZE_IC_AXI_MON 0
#define XPAR_MICROBLAZE_ILL_OPCODE_EXCEPTION 1
#define XPAR_MICROBLAZE_IMPRECISE_EXCEPTIONS 0
#define XPAR_MICROBLAZE_INSTR_SIZE 32
#define XPAR_MICROBLAZE_INTERCONNECT 2
#define XPAR_MICROBLAZE_INTERRUPT_IS_EDGE 0
#define XPAR_MICROBLAZE_INTERRUPT_MON 0
#define XPAR_MICROBLAZE_IP_AXI_MON 0
#define XPAR_MICROBLAZE_I_AXI 0
#define XPAR_MICROBLAZE_I_LMB 1
#define XPAR_MICROBLAZE_I_LMB_MON 0
#define XPAR_MICROBLAZE_LOCKSTEP_MASTER 0
#define XPAR_MICROBLAZE_LOCKSTEP_SELECT 0
#define XPAR_MICROBLAZE_LOCKSTEP_SLAVE 0
#define XPAR_MICROBLAZE_M0_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M0_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M1_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M1_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M2_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M2_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M3_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M3_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M4_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M4_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M5_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M5_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M6_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M6_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M7_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M7_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M8_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M8_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M9_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M9_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M10_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M10_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M11_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M11_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M12_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M12_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M13_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M13_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M14_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M14_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_M15_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M15_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_MMU_DTLB_SIZE 4
#define XPAR_MICROBLAZE_MMU_ITLB_SIZE 2
#define XPAR_MICROBLAZE_MMU_PRIVILEGED_INSTR 0
#define XPAR_MICROBLAZE_MMU_TLB_ACCESS 3
#define XPAR_MICROBLAZE_MMU_ZONES 16
#define XPAR_MICROBLAZE_M_AXI_DC_ADDR_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_DC_ARUSER_WIDTH 5
#define XPAR_MICROBLAZE_M_AXI_DC_AWUSER_WIDTH 5
#define XPAR_MICROBLAZE_M_AXI_DC_BUSER_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_DC_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_DC_EXCLUSIVE_ACCESS 0
#define XPAR_MICROBLAZE_M_AXI_DC_RUSER_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_DC_THREAD_ID_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_DC_USER_SIGNALS 0
#define XPAR_MICROBLAZE_M_AXI_DC_USER_VALUE 31
#define XPAR_MICROBLAZE_M_AXI_DC_WUSER_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_DP_ADDR_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_DP_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_DP_EXCLUSIVE_ACCESS 0
#define XPAR_MICROBLAZE_M_AXI_DP_THREAD_ID_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_D_BUS_EXCEPTION 1
#define XPAR_MICROBLAZE_M_AXI_IC_ADDR_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_IC_ARUSER_WIDTH 5
#define XPAR_MICROBLAZE_M_AXI_IC_AWUSER_WIDTH 5
#define XPAR_MICROBLAZE_M_AXI_IC_BUSER_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_IC_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_IC_RUSER_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_IC_THREAD_ID_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_IC_USER_SIGNALS 0
#define XPAR_MICROBLAZE_M_AXI_IC_USER_VALUE 31
#define XPAR_MICROBLAZE_M_AXI_IC_WUSER_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_IP_ADDR_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_IP_DATA_WIDTH 32
#define XPAR_MICROBLAZE_M_AXI_IP_THREAD_ID_WIDTH 1
#define XPAR_MICROBLAZE_M_AXI_I_BUS_EXCEPTION 0
#define XPAR_MICROBLAZE_NUMBER_OF_PC_BRK 2
#define XPAR_MICROBLAZE_NUMBER_OF_RD_ADDR_BRK 1
#define XPAR_MICROBLAZE_NUMBER_OF_WR_ADDR_BRK 1
#define XPAR_MICROBLAZE_NUM_SYNC_FF_CLK 2
#define XPAR_MICROBLAZE_NUM_SYNC_FF_CLK_DEBUG 2
#define XPAR_MICROBLAZE_NUM_SYNC_FF_CLK_IRQ 1
#define XPAR_MICROBLAZE_NUM_SYNC_FF_DBG_CLK 1
#define XPAR_MICROBLAZE_OPCODE_0X0_ILLEGAL 1
#define XPAR_MICROBLAZE_OPTIMIZATION 0
#define XPAR_MICROBLAZE_PC_WIDTH 32
#define XPAR_MICROBLAZE_PVR 2
#define XPAR_MICROBLAZE_PVR_USER1 0x05
#define XPAR_MICROBLAZE_PVR_USER2 0x00000000
#define XPAR_MICROBLAZE_RESET_MSR 0x00000000
#define XPAR_MICROBLAZE_RESET_MSR_BIP 1
#define XPAR_MICROBLAZE_RESET_MSR_DCE 0
#define XPAR_MICROBLAZE_RESET_MSR_EE 0
#define XPAR_MICROBLAZE_RESET_MSR_EIP 0
#define XPAR_MICROBLAZE_RESET_MSR_ICE 0
#define XPAR_MICROBLAZE_RESET_MSR_IE 0
#define XPAR_MICROBLAZE_S0_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S0_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S1_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S1_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S2_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S2_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S3_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S3_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S4_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S4_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S5_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S5_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S6_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S6_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S7_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S7_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S8_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S8_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S9_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S9_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S10_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S10_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S11_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S11_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S12_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S12_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S13_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S13_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S14_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S14_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_S15_AXIS_DATA_WIDTH 32
#define XPAR_MICROBLAZE_S15_AXIS_PROTOCOL GENERIC
#define XPAR_MICROBLAZE_SCO 0
#define XPAR_MICROBLAZE_TRACE 1
#define XPAR_MICROBLAZE_UNALIGNED_EXCEPTIONS 1
#define XPAR_MICROBLAZE_USE_BARREL 1
#define XPAR_MICROBLAZE_USE_BRANCH_TARGET_CACHE 0
#define XPAR_MICROBLAZE_USE_CONFIG_RESET 0
#define XPAR_MICROBLAZE_USE_DCACHE 0
#define XPAR_MICROBLAZE_USE_DIV 1
#define XPAR_MICROBLAZE_USE_EXTENDED_FSL_INSTR 0
#define XPAR_MICROBLAZE_USE_EXT_BRK 0
#define XPAR_MICROBLAZE_USE_EXT_NM_BRK 0
#define XPAR_MICROBLAZE_USE_FPU 0
#define XPAR_MICROBLAZE_USE_HW_MUL 2
#define XPAR_MICROBLAZE_USE_ICACHE 0
#define XPAR_MICROBLAZE_USE_INTERRUPT 1
#define XPAR_MICROBLAZE_USE_MMU 0
#define XPAR_MICROBLAZE_USE_MSR_INSTR 1
#define XPAR_MICROBLAZE_USE_NON_SECURE 0
#define XPAR_MICROBLAZE_USE_PCMP_INSTR 1
#define XPAR_MICROBLAZE_USE_REORDER_INSTR 1
#define XPAR_MICROBLAZE_USE_STACK_PROTECTION 1
#define XPAR_MICROBLAZE_EDK_IPTYPE PROCESSOR
#define XPAR_MICROBLAZE_EDK_SPECIAL microblaze
#define XPAR_MICROBLAZE_G_TEMPLATE_LIST 0
#define XPAR_MICROBLAZE_G_USE_EXCEPTIONS 1

/******************************************************************/

#define XPAR_CPU_CORE_CLOCK_FREQ_HZ XPAR_MICROBLAZE_FREQ

/******************************************************************/

/* Definition for PSS REF CLK FREQUENCY */
#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ 33333000U

#define STDIN_BASEADDRESS 0xF1920000
#define STDOUT_BASEADDRESS 0xF1920000

/******************************************************************/

/* Definitions for sleep timer configuration */
#define XSLEEP_TIMER_IS_DEFAULT_TIMER

/* Definitions for driver IOMODULE */
#define XPAR_XIOMODULE_NUM_INSTANCES 1U

/******************************************************************/

#define XPAR_IOMODULE_SINGLE_BASEADDR 0xEBE80000
#define XPAR_IOMODULE_SINGLE_HIGHADDR 0xEBE80FFF
#define XPAR_IOMODULE_INTC_SINGLE_DEVICE_ID 0x0U
#define XPAR_IOMODULE_INTC_MAX_INTR_SIZE 32U

/******************************************************************/

/* Canonical definitions for peripheral PSV_PMC_IOMODULE_0 */
#define XPAR_IOMODULE_0_NUM_INSTANCES 1U
#define XPAR_IOMODULE_0_DEVICE_ID 0U
#define XPAR_IOMODULE_0_BASEADDR 0xEBE80000U
#define XPAR_XIOMODULE_0_BASEADDR 0xEBE80000U
#define XPAR_IOMODULE_0_HIGHADDR 0xEBE80FFFU
#define XPAR_IOMODULE_0_MASK 0x00000000FFFFF000U
#define XPAR_IOMODULE_0_FREQ 100000000U
#define XPAR_IOMODULE_0_USE_UART_RX 1U
#define XPAR_IOMODULE_0_USE_UART_TX 1U
#define XPAR_IOMODULE_0_UART_BAUDRATE 115200U
#define XPAR_IOMODULE_0_UART_PROG_BAUDRATE 1U
#define XPAR_IOMODULE_0_UART_DATA_BITS 8U
#define XPAR_IOMODULE_0_UART_USE_PARITY 0U
#define XPAR_IOMODULE_0_UART_ODD_PARITY 0U
#define XPAR_IOMODULE_0_UART_RX_INTERRUPT 1U
#define XPAR_IOMODULE_0_UART_TX_INTERRUPT 1U
#define XPAR_IOMODULE_0_UART_ERROR_INTERRUPT 1U
#define XPAR_IOMODULE_0_USE_FIT1 0U
#define XPAR_IOMODULE_0_FIT1_NO_CLOCKS 6216U
#define XPAR_IOMODULE_0_FIT1_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_FIT2 0U
#define XPAR_IOMODULE_0_FIT2_NO_CLOCKS 6216U
#define XPAR_IOMODULE_0_FIT2_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_FIT3 0U
#define XPAR_IOMODULE_0_FIT3_NO_CLOCKS 6216U
#define XPAR_IOMODULE_0_FIT3_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_FIT4 0U
#define XPAR_IOMODULE_0_FIT4_NO_CLOCKS 6216U
#define XPAR_IOMODULE_0_FIT4_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_PIT1 1U
#define XPAR_IOMODULE_0_PIT1_SIZE 32U
#define XPAR_IOMODULE_0_PIT1_EXPIRED_MASK 0xFFFFFFFFU
#define XPAR_IOMODULE_0_PIT1_READABLE 1U
#define XPAR_IOMODULE_0_PIT1_PRESCALER 9U
#define XPAR_IOMODULE_0_PIT1_INTERRUPT 1U
#define XPAR_IOMODULE_0_USE_PIT2 1U
#define XPAR_IOMODULE_0_PIT2_SIZE 32U
#define XPAR_IOMODULE_0_PIT2_EXPIRED_MASK 0xFFFFFFFFU
#define XPAR_IOMODULE_0_PIT2_READABLE 1U
#define XPAR_IOMODULE_0_PIT2_PRESCALER 0U
#define XPAR_IOMODULE_0_PIT2_INTERRUPT 1U
#define XPAR_IOMODULE_0_USE_PIT3 1U
#define XPAR_IOMODULE_0_PIT3_SIZE 32U
#define XPAR_IOMODULE_0_PIT3_EXPIRED_MASK 0xFFFFFFFFU
#define XPAR_IOMODULE_0_PIT3_READABLE 1U
#define XPAR_IOMODULE_0_PIT3_PRESCALER 9U
#define XPAR_IOMODULE_0_PIT3_INTERRUPT 1U
#define XPAR_IOMODULE_0_USE_PIT4 1U
#define XPAR_IOMODULE_0_PIT4_SIZE 32U
#define XPAR_IOMODULE_0_PIT4_EXPIRED_MASK 0xFFFFFFFFU
#define XPAR_IOMODULE_0_PIT4_READABLE 1U
#define XPAR_IOMODULE_0_PIT4_PRESCALER 0U
#define XPAR_IOMODULE_0_PIT4_INTERRUPT 1U
#define XPAR_IOMODULE_0_USE_GPO1 1U
#define XPAR_IOMODULE_0_GPO1_SIZE 3U
#define XPAR_IOMODULE_0_USE_GPO2 0U
#define XPAR_IOMODULE_0_GPO2_SIZE 32U
#define XPAR_IOMODULE_0_USE_GPO3 0U
#define XPAR_IOMODULE_0_GPO3_SIZE 32U
#define XPAR_IOMODULE_0_USE_GPO4 0U
#define XPAR_IOMODULE_0_GPO4_SIZE 32U
#define XPAR_IOMODULE_0_USE_GPI1 0U
#define XPAR_IOMODULE_0_GPI1_SIZE 32U
#define XPAR_IOMODULE_0_GPI1_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_GPI2 0U
#define XPAR_IOMODULE_0_GPI2_SIZE 32U
#define XPAR_IOMODULE_0_GPI2_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_GPI3 0U
#define XPAR_IOMODULE_0_GPI3_SIZE 32U
#define XPAR_IOMODULE_0_GPI3_INTERRUPT 0U
#define XPAR_IOMODULE_0_USE_GPI4 0U
#define XPAR_IOMODULE_0_GPI4_SIZE 32U
#define XPAR_IOMODULE_0_GPI4_INTERRUPT 0U
#define XPAR_IOMODULE_0_INTC_USE_EXT_INTR 1U
#define XPAR_IOMODULE_0_INTC_INTR_SIZE 16U
#define XPAR_IOMODULE_0_INTC_HAS_FAST 0U
#define XPAR_IOMODULE_0_INTC_BASE_VECTORS 0xEBE40000U
#define XPAR_IOMODULE_0_INTC_ADDR_WIDTH 32U
#define XPAR_IOMODULE_0_ 0U
#define XPAR_IOMODULE_0_USE_IO_BUS 0U
#define XPAR_IOMODULE_0_IO_BASEADDR 0xFFFFFFFFFFFFFFFFU
#define XPAR_IOMODULE_0_IO_HIGHADDR 0x00000000U
#define XPAR_IOMODULE_0_IO_MASK 0x00000000FFFE0000U
#define XPAR_IOMODULE_0_GPO1_INIT 0x00000000U
#define XPAR_IOMODULE_0_GPO2_INIT 0x00000000U
#define XPAR_IOMODULE_0_GPO3_INIT 0x00000000U
#define XPAR_IOMODULE_0_GPO4_INIT 0x00000000U
#define XPAR_IOMODULE_0_INTC_LEVEL_EDGE 0x0000U
#define XPAR_IOMODULE_0_INTC_POSITIVE 0xFFFFU

/******************************************************************/

/* Definitions for driver UARTPSV */
#define XPAR_XUARTPSV_NUM_INSTANCES 1

/* Definitions for peripheral PSV_SBSAUART_0 */
#define XPAR_PSV_SBSAUART_0_DEVICE_ID 0
#define XPAR_PSV_SBSAUART_0_BASEADDR 0xF1920000
#define XPAR_PSV_SBSAUART_0_HIGHADDR 0xF192FFFF
#define XPAR_PSV_SBSAUART_0_UART_CLK_FREQ_HZ 99999992
#define XPAR_PSV_SBSAUART_0_HAS_MODEM 0
#define XPAR_PSV_SBSAUART_0_BAUDRATE 115200

/******************************************************************/

/* Definitions for driver TMR_INJECT */
#define XPAR_XTMR_INJECT_NUM_INSTANCES 1

/* Definitions for peripheral PSV_PMC_TMR_INJECT_0 */
#define XPAR_PSV_PMC_TMR_INJECT_0_DEVICE_ID 0
#define XPAR_PSV_PMC_TMR_INJECT_0_BASEADDR 0xEBED0000
#define XPAR_PSV_PMC_TMR_INJECT_0_HIGHADDR 0xEBED0FFF
#define XPAR_PSV_PMC_TMR_INJECT_0_MASK 0x0000000000084000
#define XPAR_PSV_PMC_TMR_INJECT_0_MAGIC 0x27
#define XPAR_PSV_PMC_TMR_INJECT_0_CPU_ID 1
#define XPAR_PSV_PMC_TMR_INJECT_0_INJECT_LMB_AWIDTH 0

/******************************************************************/

/* Canonical definitions for peripheral PSV_PMC_TMR_INJECT_0 */
#define XPAR_TMR_INJECT_0_DEVICE_ID XPAR_PSV_PMC_TMR_INJECT_0_DEVICE_ID
#define XPAR_TMR_INJECT_0_BASEADDR 0xEBED0000
#define XPAR_TMR_INJECT_0_HIGHADDR 0xEBED0FFF
#define XPAR_TMR_INJECT_0_MASK 0x0000000000084000
#define XPAR_TMR_INJECT_0_MAGIC 0x27
#define XPAR_TMR_INJECT_0_CPU_ID 1
#define XPAR_TMR_INJECT_0_INJECT_LMB_AWIDTH 0

/******************************************************************/

/* Definitions for driver TMR_MANAGER */
#define XPAR_XTMR_MANAGER_NUM_INSTANCES 1

/* Definitions for peripheral PSV_PMC_TMR_MANAGER_0 */
#define XPAR_PSV_PMC_TMR_MANAGER_0_DEVICE_ID 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_BASEADDR 0xEBEC0000
#define XPAR_PSV_PMC_TMR_MANAGER_0_HIGHADDR 0xEBEC0FFF
#define XPAR_PSV_PMC_TMR_MANAGER_0_MASK 0x0000000000083000
#define XPAR_PSV_PMC_TMR_MANAGER_0_BRK_DELAY_RST_VALUE 0x00000000
#define XPAR_PSV_PMC_TMR_MANAGER_0_MASK_RST_VALUE 0xFFFFFFFFFFFFFFFF
#define XPAR_PSV_PMC_TMR_MANAGER_0_MAGIC1 0x00
#define XPAR_PSV_PMC_TMR_MANAGER_0_MAGIC2 0x00
#define XPAR_PSV_PMC_TMR_MANAGER_0_UE_IS_FATAL 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_UE_WIDTH 3
#define XPAR_PSV_PMC_TMR_MANAGER_0_NO_OF_COMPARATORS 1
#define XPAR_PSV_PMC_TMR_MANAGER_0_COMPARATORS_MASK 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_WATCHDOG 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_WATCHDOG_WIDTH 30
#define XPAR_PSV_PMC_TMR_MANAGER_0_SEM_INTERFACE 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_SEM_HEARTBEAT_WATCHDOG 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_SEM_HEARTBEAT_WATCHDOG_WIDTH 10
#define XPAR_PSV_PMC_TMR_MANAGER_0_BRK_DELAY_WIDTH 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_TMR 1
#define XPAR_PSV_PMC_TMR_MANAGER_0_TEST_COMPARATOR 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_STRICT_MISCOMPARE 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_USE_DEBUG_DISABLE 0
#define XPAR_PSV_PMC_TMR_MANAGER_0_USE_TMR_DISABLE 0

/******************************************************************/

/* Canonical definitions for peripheral PSV_PMC_TMR_MANAGER_0 */
#define XPAR_TMR_MANAGER_0_DEVICE_ID XPAR_PSV_PMC_TMR_MANAGER_0_DEVICE_ID
#define XPAR_TMR_MANAGER_0_BASEADDR 0xEBEC0000
#define XPAR_TMR_MANAGER_0_HIGHADDR 0xEBEC0FFF
#define XPAR_TMR_MANAGER_0_BRK_DELAY_RST_VALUE 0x00000000
#define XPAR_TMR_MANAGER_0_MASK_RST_VALUE 0xFFFFFFFFFFFFFFFF
#define XPAR_TMR_MANAGER_0_MASK 0x0000000000083000
#define XPAR_TMR_MANAGER_0_MAGIC1 0x00
#define XPAR_TMR_MANAGER_0_MAGIC2 0x00
#define XPAR_TMR_MANAGER_0_UE_IS_FATAL 0
#define XPAR_TMR_MANAGER_0_UE_WIDTH 3
#define XPAR_TMR_MANAGER_0_NO_OF_COMPARATORS 1
#define XPAR_TMR_MANAGER_0_COMPARATORS_MASK 0
#define XPAR_TMR_MANAGER_0_WATCHDOG 0
#define XPAR_TMR_MANAGER_0_WATCHDOG_WIDTH 30
#define XPAR_TMR_MANAGER_0_SEM_INTERFACE 0
#define XPAR_TMR_MANAGER_0_SEM_HEARTBEAT_WATCHDOG 0
#define XPAR_TMR_MANAGER_0_SEM_HEARTBEAT_WATCHDOG_WIDTH 10
#define XPAR_TMR_MANAGER_0_BRK_DELAY_WIDTH 0
#define XPAR_TMR_MANAGER_0_TMR 1
#define XPAR_TMR_MANAGER_0_TEST_COMPARATOR 0
#define XPAR_TMR_MANAGER_0_STRICT_MISCOMPARE 0
#define XPAR_TMR_MANAGER_0_USE_DEBUG_DISABLE 0
#define XPAR_TMR_MANAGER_0_USE_TMR_DISABLE 0

/******************************************************************/
#define  XPAR_XIPIPSU_NUM_INSTANCES  1U

/* Parameter definitions for peripheral ASU IPI */
#define  XPAR_XIPIPSU_0_DEVICE_ID  0U
#define  XPAR_XIPIPSU_0_BASEADDR  0xEB310000U
#define  XPAR_XIPIPSU_0_BIT_MASK  0x00000001U
#define  XPAR_XIPIPSU_0_BUFFER_INDEX  0x0U
#define  XPAR_XIPIPSU_0_INT_ID  59U

#define  XPAR_XIPIPSU_NUM_TARGETS  16U

#define  XPAR_PSV_IPI_PMC_BIT_MASK  0x00000002U
#define  XPAR_PSV_IPI_PMC_BUFFER_INDEX  0x1U
#define  XPAR_PSV_IPI_0_BIT_MASK  0x00000004U
#define  XPAR_PSV_IPI_0_BUFFER_INDEX  0x2U
#define  XPAR_PSV_IPI_1_BIT_MASK  0x00000008U
#define  XPAR_PSV_IPI_1_BUFFER_INDEX  0x3U
#define  XPAR_PSV_IPI_2_BIT_MASK  0x00000010U
#define  XPAR_PSV_IPI_2_BUFFER_INDEX  0x4U
#define  XPAR_PSV_IPI_3_BIT_MASK  0x00000020U
#define  XPAR_PSV_IPI_3_BUFFER_INDEX  0x5U
#define  XPAR_PSV_IPI_4_BIT_MASK  0x00000040U
#define  XPAR_PSV_IPI_4_BUFFER_INDEX  0x6U
#define  XPAR_PSV_IPI_5_BIT_MASK  0x00000080U
#define  XPAR_PSV_IPI_5_BUFFER_INDEX  0x7U
#define  XPAR_PSV_IPI_PMC_NOBUF_BIT_MASK  0x00000100U
#define  XPAR_PSV_IPI_PMC_NOBUF_BUFFER_INDEX  0xFFFFU
#define  XPAR_PSV_IPI_6_BIT_MASK  0x00000200U
#define  XPAR_PSV_IPI_6_BUFFER_INDEX  0xFFFFU

/******************************************************************/

/* Definitions for driver CSUDMA */
#define XPAR_XCSUDMA_NUM_INSTANCES 2

/* Definitions for peripheral ASU_DMA_0 */
#define XPAR_ASU_DMA_0_DEVICE_ID 0
#define XPAR_ASU_DMA_0_BASEADDR 0xEBE8C000
#define XPAR_ASU_DMA_0_HIGHADDR 0xEBE8CFFF
#define XPAR_ASU_DMA_0_CSUDMA_CLK_FREQ_HZ 0


/* Definitions for peripheral ASU_DMA_1 */
#define XPAR_ASU_DMA_1_DEVICE_ID 1
#define XPAR_ASU_DMA_1_BASEADDR 0xEBE8D000
#define XPAR_ASU_DMA_1_HIGHADDR 0xEBE8DFFF
#define XPAR_ASU_DMA_1_CSUDMA_CLK_FREQ_HZ 0


/******************************************************************/

#define XPAR_ASU_DMA0_DMA_TYPE 3
#define XPAR_ASU_DMA1_DMA_TYPE 4
/* Canonical definitions for peripheral ASU_DMA_0 */
#define XPAR_XCSUDMA_0_DEVICE_ID XPAR_ASU_DMA_0_DEVICE_ID
#define XPAR_XCSUDMA_0_BASEADDR 0xEBE8C000
#define XPAR_XCSUDMA_0_HIGHADDR 0xEBE8CFFF
#define XPAR_XCSUDMA_0_CSUDMA_CLK_FREQ_HZ 0

/* Canonical definitions for peripheral ASU_DMA_1 */
#define XPAR_XCSUDMA_1_DEVICE_ID XPAR_ASU_DMA_1_DEVICE_ID
#define XPAR_XCSUDMA_1_BASEADDR 0xEBE8D000
#define XPAR_XCSUDMA_1_HIGHADDR 0xEBE8DFFF
#define XPAR_XCSUDMA_1_CSUDMA_CLK_FREQ_HZ 0

/* Counter measure enable/disable for AES and ECC */
#define XASU_AES_CM_ENABLE
#define XASU_ECC_CM_ENABLE

/* Platform specific definitions */
#ifndef VERSAL_2VE_2VM
#define VERSAL_2VE_2VM
#endif

#define VERSAL_NET

#endif /* end of protection macro */
