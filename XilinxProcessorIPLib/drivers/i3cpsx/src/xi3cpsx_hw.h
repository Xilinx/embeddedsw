/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XI3CPSX_HW_H_		/* prevent circular inclusions */
#define XI3CPSX_HW_H_

/*****************************************************************************/
/**
*
* @file xi3cpsx_hw.h
* @brief Defines I3C hardware register offsets and access macros.
*
* This file contains low level access functions using the base address
* directly without an instance.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.00  sd  06/10/22 First release
* 1.4   gm  10/07/24 Added macros for reading write and read fifo levels,
* 		     enable and disable interrupts, and register offset.
* 1.7   vlt 01/27/26 Fixed codespell issues.
* 1.8   vn  07/30/26 Fix Doxygen warnings
* 1.8   tb  09/22/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3cpsx_api I3CPSX APIs
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#ifdef SDT
#define XI3CPSX_BASEADDR      0x0	/**< I3C controller register base. */
#else
#define XI3CPSX_BASEADDR      0x8000	/**< I3C controller register base. */
#endif
#define XI3cPsx_In32  Xil_In32	/**< 32-bit register read. */
#define XI3cPsx_Out32 Xil_Out32	/**< 32-bit register write. */

/****************************************************************************/
/**
* @brief Reads an I3C register.
*
* @param[in] BaseAddress The base address of the device.
* @param[in] RegOffset The offset from the 1st register of the device
*            to select the specific register.
*
* @return The value read from the register.
*
* @note
* C-style signature
*	u32 XI3cPsx_ReadReg(u32 BaseAddress, int RegOffset)
*
******************************************************************************/
#define XI3cPsx_ReadReg(BaseAddress, RegOffset) \
	XI3cPsx_In32((BaseAddress) + (u32)(RegOffset))

/***************************************************************************/
/**
* @brief Writes an I3C register.
*
* @param[in] BaseAddress The base address of the device.
* @param[in] RegOffset The offset from the 1st register of the device
*            to select the specific register.
* @param[in] RegisterValue The value to be written to the register.
*
* @note
* C-style signature
*	void XI3cPsx_WriteReg(u32 BaseAddress, int RegOffset,
*		u32 RegisterValue)
*
******************************************************************************/
#define XI3cPsx_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XI3cPsx_Out32((BaseAddress) + (u32)(RegOffset), (u32)(RegisterValue))

/****************************************************************************/
/**
*
* @brief Reads the transmit FIFO level.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
*
* @return Transmit FIFO empty-location count.
*
* @note
* C-style signature
*	u8 XI3cPsx_WrFifoLevel(XI3cPsx *InstancePtr)
*
*****************************************************************************/
#define XI3cPsx_WrFifoLevel(InstancePtr)				     \
	(u8)(XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,		     \
			     XI3CPSX_DATA_BUFFER_STATUS_LEVEL)		     \
	     & XI3CPSX_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC_MASK)

/****************************************************************************/
/**
*
* @brief Reads the receive FIFO level.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
*
* @return Receive FIFO fill level.
*
* @note
* C-style signature
*	u8 XI3cPsx_RdFifoLevel(XI3cPsx *InstancePtr)
*
*****************************************************************************/
#define XI3cPsx_RdFifoLevel(InstancePtr)				     \
	(u8)((XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,		     \
			      XI3CPSX_DATA_BUFFER_STATUS_LEVEL)	     	     \
	     & XI3CPSX_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_MASK)	     \
	     >> XI3CPSX_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_SHIFT)

/*****************************************************************************/
/**
*
* @brief Enables interrupts.
*
* @param[in] BaseAddress The base address of the XI3cPsx core instance.
* @param[in] IntrMask The interrupt mask value.
*
* @note
* C-style signature
*	void XI3cPsx_EnableInterrupts(u32 BaseAddress, u32 IntrMask)
*
******************************************************************************/
#define XI3cPsx_EnableInterrupts(BaseAddress, IntrMask) 		          \
	XI3cPsx_WriteReg((BaseAddress), XI3CPSX_INTR_STATUS_EN,		          \
			 ((XI3cPsx_ReadReg(BaseAddress, XI3CPSX_INTR_STATUS_EN))| \
			 (IntrMask)));					          \
	XI3cPsx_WriteReg((BaseAddress), XI3CPSX_INTR_SIGNAL_EN,		          \
			 ((XI3cPsx_ReadReg(BaseAddress, XI3CPSX_INTR_SIGNAL_EN))| \
			 (IntrMask)))

/*****************************************************************************/
/**
*
* @brief Disables interrupts.
*
* @param[in] BaseAddress The base address of the XI3cPsx core instance.
* @param[in] IntrMask The interrupt mask value.
*
* @note
* C-style signature
*	void XI3cPsx_DisableInterrupts(u32 BaseAddress, u32 IntrMask)
*
******************************************************************************/
#define XI3cPsx_DisableInterrupts(BaseAddress, IntrMask)			\
	XI3cPsx_WriteReg((BaseAddress), XI3CPSX_INTR_STATUS_EN,			\
			 ((XI3cPsx_ReadReg(BaseAddress, XI3CPSX_INTR_STATUS_EN))\
			 & ~(IntrMask)));					\
	XI3cPsx_WriteReg((BaseAddress), XI3CPSX_INTR_SIGNAL_EN,			\
			 ((XI3cPsx_ReadReg(BaseAddress, XI3CPSX_INTR_SIGNAL_EN))\
			 & ~(IntrMask)))

/** Number of nanoseconds in one second. */
#define XI3CPSX_NS_1SEC			1000000000

/**
 * @name Bit-width masks used by I3C helper logic.
 *
 * These masks carve out the low N bits of a value. Used by the folded-XOR
 * odd-parity helper (XI3cPsx_OddParity7) and the dynamic-address encoding
 * in XI3cPsx_SetDynamicAddr.
 *
 * @{
 */
#define	XI3CPSX_1BIT_MASK	0x00000001U	/**< Mask for the least-significant 1 bit (values 0..1) */
#define	XI3CPSX_2BITS_MASK	0x00000003U	/**< Mask for the least-significant 2 bits (values 0..3) */
#define	XI3CPSX_4BITS_MASK	0x0000000FU	/**< Mask for the least-significant 4 bits (values 0..15) */
#define	XI3CPSX_7BITS_MASK	0x0000007FU	/**< Mask for the 7-bit I3C dynamic address field (values 0x00..0x7F) */
/** @} */

/**
 * @brief Defines the odd-parity bit position in an encoded dynamic address.
 *
 * Bit position of the odd-parity bit within the 8-bit encoded I3C dynamic
 * address.
 *
 * The encoded dynamic address is an 8-bit field: bit[7] is the odd parity
 * of the 7-bit dynamic address (bits[6:0]), that is, parity = ~XOR(addr[6:0]).
 * This matches the MIPI I3C bus format and the
 * DEV_ADDR_TABLE_LOC*.DEV_DYNAMIC_ADDR[23:16] field layout, where bit[23]
 * (= bit[7] of the 8-bit field) is the parity bit.
 */
#define	XI3CPSX_DYNAMIC_ADDR_PARITY_SHIFT	7U

/**
 * @brief Defines the DEVICE_CTRL register offset.
 */
#define XI3CPSX_DEVICE_CTRL     XI3CPSX_BASEADDR
/** DEVICE CTRL full mask. */
#define XI3CPSX_DEVICE_CTRL_FULLMASK     0xeb000181
/** DEVICE CTRL full read/write mask. */
#define XI3CPSX_DEVICE_CTRL_FULLRWMASK  0xeb000181
/** DEVICE CTRL default value. */
#define XI3CPSX_DEVICE_CTRL_DEFVAL   0x0

/* access_type: rw */
/** DEVICE CTRL ENABLE bit shift. */
#define XI3CPSX_DEVICE_CTRL_ENABLE_SHIFT   31
/** DEVICE CTRL ENABLE bit width. */
#define XI3CPSX_DEVICE_CTRL_ENABLE_WIDTH   1
/** DEVICE CTRL ENABLE bit mask. */
#define XI3CPSX_DEVICE_CTRL_ENABLE_MASK    0x80000000
/** DEVICE CTRL ENABLE default value. */
#define XI3CPSX_DEVICE_CTRL_ENABLE_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL RESUME bit shift. */
#define XI3CPSX_DEVICE_CTRL_RESUME_SHIFT   30
/** DEVICE CTRL RESUME bit width. */
#define XI3CPSX_DEVICE_CTRL_RESUME_WIDTH   1
/** DEVICE CTRL RESUME bit mask. */
#define XI3CPSX_DEVICE_CTRL_RESUME_MASK    0x40000000
/** DEVICE CTRL RESUME default value. */
#define XI3CPSX_DEVICE_CTRL_RESUME_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL ABORT bit shift. */
#define XI3CPSX_DEVICE_CTRL_ABORT_SHIFT   29
/** DEVICE CTRL ABORT bit width. */
#define XI3CPSX_DEVICE_CTRL_ABORT_WIDTH   1
/** DEVICE CTRL ABORT bit mask. */
#define XI3CPSX_DEVICE_CTRL_ABORT_MASK    0x20000000
/** DEVICE CTRL ABORT default value. */
#define XI3CPSX_DEVICE_CTRL_ABORT_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL ADAPTIVE I2C I3C bit shift. */
#define XI3CPSX_DEVICE_CTRL_ADAPTIVE_I2C_I3C_SHIFT   27
/** DEVICE CTRL ADAPTIVE I2C I3C bit width. */
#define XI3CPSX_DEVICE_CTRL_ADAPTIVE_I2C_I3C_WIDTH   1
/** DEVICE CTRL ADAPTIVE I2C I3C bit mask. */
#define XI3CPSX_DEVICE_CTRL_ADAPTIVE_I2C_I3C_MASK    0x08000000
/** DEVICE CTRL ADAPTIVE I2C I3C default value. */
#define XI3CPSX_DEVICE_CTRL_ADAPTIVE_I2C_I3C_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL IDLE CNT MULTPLIER bit shift. */
#define XI3CPSX_DEVICE_CTRL_IDLE_CNT_MULTPLIER_SHIFT   24
/** DEVICE CTRL IDLE CNT MULTPLIER bit width. */
#define XI3CPSX_DEVICE_CTRL_IDLE_CNT_MULTPLIER_WIDTH   2
/** DEVICE CTRL IDLE CNT MULTPLIER bit mask. */
#define XI3CPSX_DEVICE_CTRL_IDLE_CNT_MULTPLIER_MASK    0x03000000
/** DEVICE CTRL IDLE CNT MULTPLIER default value. */
#define XI3CPSX_DEVICE_CTRL_IDLE_CNT_MULTPLIER_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL HOT JOIN CTRL bit shift. */
#define XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_SHIFT   8
/** DEVICE CTRL HOT JOIN CTRL bit width. */
#define XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_WIDTH   1
/** DEVICE CTRL HOT JOIN CTRL bit mask. */
#define XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_MASK    0x00000100
/** DEVICE CTRL HOT JOIN CTRL default value. */
#define XI3CPSX_DEVICE_CTRL_HOT_JOIN_CTRL_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL I2C SLAVE PRESENT bit shift. */
#define XI3CPSX_DEVICE_CTRL_I2C_SLAVE_PRESENT_SHIFT   7
/** DEVICE CTRL I2C SLAVE PRESENT bit width. */
#define XI3CPSX_DEVICE_CTRL_I2C_SLAVE_PRESENT_WIDTH   1
/** DEVICE CTRL I2C SLAVE PRESENT bit mask. */
#define XI3CPSX_DEVICE_CTRL_I2C_SLAVE_PRESENT_MASK    0x00000080
/** DEVICE CTRL I2C SLAVE PRESENT default value. */
#define XI3CPSX_DEVICE_CTRL_I2C_SLAVE_PRESENT_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL IBA INCLUDE bit shift. */
#define XI3CPSX_DEVICE_CTRL_IBA_INCLUDE_SHIFT   0
/** DEVICE CTRL IBA INCLUDE bit width. */
#define XI3CPSX_DEVICE_CTRL_IBA_INCLUDE_WIDTH   1
/** DEVICE CTRL IBA INCLUDE bit mask. */
#define XI3CPSX_DEVICE_CTRL_IBA_INCLUDE_MASK    0x00000001
/** DEVICE CTRL IBA INCLUDE default value. */
#define XI3CPSX_DEVICE_CTRL_IBA_INCLUDE_DEFVAL  0x0

/**
 * @brief Defines the DEVICE_ADDR register offset.
 */
#define XI3CPSX_DEVICE_ADDR    ( ( XI3CPSX_BASEADDR ) + 0x00000004 )
/** DEVICE ADDR full mask. */
#define XI3CPSX_DEVICE_ADDR_FULLMASK     0x807f807f
/** DEVICE ADDR full read/write mask. */
#define XI3CPSX_DEVICE_ADDR_FULLRWMASK  0x807f807f
/** DEVICE ADDR default value. */
#define XI3CPSX_DEVICE_ADDR_DEFVAL   0x0

/** DEVICE ADDR SELF DYNAMIC ADDR register field. */
#define XI3CPSX_DEVICE_ADDR_SELF_DYNAMIC_ADDR   0x45

/* access_type: rw */
/** DEVICE ADDR DYNAMIC ADDR VALID bit shift. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_SHIFT   31
/** DEVICE ADDR DYNAMIC ADDR VALID bit width. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_WIDTH   1
/** DEVICE ADDR DYNAMIC ADDR VALID bit mask. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_MASK    0x80000000
/** DEVICE ADDR DYNAMIC ADDR VALID default value. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_VALID_DEFVAL  0x0

/* access_type: rw */
/** DEVICE ADDR DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_SHIFT   16
/** DEVICE ADDR DYNAMIC ADDR bit width. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_WIDTH   7
/** DEVICE ADDR DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_MASK    0x007f0000
/** DEVICE ADDR DYNAMIC ADDR default value. */
#define XI3CPSX_DEVICE_ADDR_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEVICE ADDR STATIC ADDR VALID bit shift. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_VALID_SHIFT   15
/** DEVICE ADDR STATIC ADDR VALID bit width. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_VALID_WIDTH   1
/** DEVICE ADDR STATIC ADDR VALID bit mask. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_VALID_MASK    0x00008000
/** DEVICE ADDR STATIC ADDR VALID default value. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_VALID_DEFVAL  0x0

/* access_type: rw */
/** DEVICE ADDR STATIC ADDR bit shift. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_SHIFT   0
/** DEVICE ADDR STATIC ADDR bit width. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_WIDTH   7
/** DEVICE ADDR STATIC ADDR bit mask. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_MASK    0x0000007f
/** DEVICE ADDR STATIC ADDR default value. */
#define XI3CPSX_DEVICE_ADDR_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the HW_CAPABILITY register offset.
 */
#define XI3CPSX_HW_CAPABILITY    ( ( XI3CPSX_BASEADDR ) + 0x00000008 )
/** HW CAPABILITY full mask. */
#define XI3CPSX_HW_CAPABILITY_FULLMASK     0x000fffff
/** HW CAPABILITY full read/write mask. */
#define XI3CPSX_HW_CAPABILITY_FULLRWMASK  0x00000000
/** HW CAPABILITY default value. */
#define XI3CPSX_HW_CAPABILITY_DEFVAL   0xd40ab

/* access_type: ro */
/** HW CAPABILITY SLV IBI CAP bit shift. */
#define XI3CPSX_HW_CAPABILITY_SLV_IBI_CAP_SHIFT   19
/** HW CAPABILITY SLV IBI CAP bit width. */
#define XI3CPSX_HW_CAPABILITY_SLV_IBI_CAP_WIDTH   1
/** HW CAPABILITY SLV IBI CAP bit mask. */
#define XI3CPSX_HW_CAPABILITY_SLV_IBI_CAP_MASK    0x00080000
/** HW CAPABILITY SLV IBI CAP default value. */
#define XI3CPSX_HW_CAPABILITY_SLV_IBI_CAP_DEFVAL  0x1

/* access_type: ro */
/** HW CAPABILITY SLV HJ CAP bit shift. */
#define XI3CPSX_HW_CAPABILITY_SLV_HJ_CAP_SHIFT   18
/** HW CAPABILITY SLV HJ CAP bit width. */
#define XI3CPSX_HW_CAPABILITY_SLV_HJ_CAP_WIDTH   1
/** HW CAPABILITY SLV HJ CAP bit mask. */
#define XI3CPSX_HW_CAPABILITY_SLV_HJ_CAP_MASK    0x00040000
/** HW CAPABILITY SLV HJ CAP default value. */
#define XI3CPSX_HW_CAPABILITY_SLV_HJ_CAP_DEFVAL  0x1

/* access_type: ro */
/** HW CAPABILITY DMA EN bit shift. */
#define XI3CPSX_HW_CAPABILITY_DMA_EN_SHIFT   17
/** HW CAPABILITY DMA EN bit width. */
#define XI3CPSX_HW_CAPABILITY_DMA_EN_WIDTH   1
/** HW CAPABILITY DMA EN bit mask. */
#define XI3CPSX_HW_CAPABILITY_DMA_EN_MASK    0x00020000
/** HW CAPABILITY DMA EN default value. */
#define XI3CPSX_HW_CAPABILITY_DMA_EN_DEFVAL  0x0

/* access_type: ro */
/** HW CAPABILITY HDR TX CLOCK PERIOD bit shift. */
#define XI3CPSX_HW_CAPABILITY_HDR_TX_CLOCK_PERIOD_SHIFT   11
/** HW CAPABILITY HDR TX CLOCK PERIOD bit width. */
#define XI3CPSX_HW_CAPABILITY_HDR_TX_CLOCK_PERIOD_WIDTH   6
/** HW CAPABILITY HDR TX CLOCK PERIOD bit mask. */
#define XI3CPSX_HW_CAPABILITY_HDR_TX_CLOCK_PERIOD_MASK    0x0001f800
/** HW CAPABILITY HDR TX CLOCK PERIOD default value. */
#define XI3CPSX_HW_CAPABILITY_HDR_TX_CLOCK_PERIOD_DEFVAL  0x28

/* access_type: ro */
/** HW CAPABILITY CLOCK PERIOD bit shift. */
#define XI3CPSX_HW_CAPABILITY_CLOCK_PERIOD_SHIFT   5
/** HW CAPABILITY CLOCK PERIOD bit width. */
#define XI3CPSX_HW_CAPABILITY_CLOCK_PERIOD_WIDTH   6
/** HW CAPABILITY CLOCK PERIOD bit mask. */
#define XI3CPSX_HW_CAPABILITY_CLOCK_PERIOD_MASK    0x000007e0
/** HW CAPABILITY CLOCK PERIOD default value. */
#define XI3CPSX_HW_CAPABILITY_CLOCK_PERIOD_DEFVAL  0x5

/* access_type: ro */
/** HW CAPABILITY HDR TS EN bit shift. */
#define XI3CPSX_HW_CAPABILITY_HDR_TS_EN_SHIFT   4
/** HW CAPABILITY HDR TS EN bit width. */
#define XI3CPSX_HW_CAPABILITY_HDR_TS_EN_WIDTH   1
/** HW CAPABILITY HDR TS EN bit mask. */
#define XI3CPSX_HW_CAPABILITY_HDR_TS_EN_MASK    0x00000010
/** HW CAPABILITY HDR TS EN default value. */
#define XI3CPSX_HW_CAPABILITY_HDR_TS_EN_DEFVAL  0x0

/* access_type: ro */
/** HW CAPABILITY HDR DDR EN bit shift. */
#define XI3CPSX_HW_CAPABILITY_HDR_DDR_EN_SHIFT   3
/** HW CAPABILITY HDR DDR EN bit width. */
#define XI3CPSX_HW_CAPABILITY_HDR_DDR_EN_WIDTH   1
/** HW CAPABILITY HDR DDR EN bit mask. */
#define XI3CPSX_HW_CAPABILITY_HDR_DDR_EN_MASK    0x00000008
/** HW CAPABILITY HDR DDR EN default value. */
#define XI3CPSX_HW_CAPABILITY_HDR_DDR_EN_DEFVAL  0x1

/* access_type: ro */
/** HW CAPABILITY DEVICE ROLE CONFIG bit shift. */
#define XI3CPSX_HW_CAPABILITY_DEVICE_ROLE_CONFIG_SHIFT   0
/** HW CAPABILITY DEVICE ROLE CONFIG bit width. */
#define XI3CPSX_HW_CAPABILITY_DEVICE_ROLE_CONFIG_WIDTH   3
/** HW CAPABILITY DEVICE ROLE CONFIG bit mask. */
#define XI3CPSX_HW_CAPABILITY_DEVICE_ROLE_CONFIG_MASK    0x00000007
/** HW CAPABILITY DEVICE ROLE CONFIG default value. */
#define XI3CPSX_HW_CAPABILITY_DEVICE_ROLE_CONFIG_DEFVAL  0x3

/**
 * @brief Defines the COMMAND_QUEUE_PORT register offset.
 */
#define XI3CPSX_COMMAND_QUEUE_PORT    ( ( XI3CPSX_BASEADDR ) + 0x0000000C )
/** COMMAND QUEUE PORT full mask. */
#define XI3CPSX_COMMAND_QUEUE_PORT_FULLMASK     0xffffffff
/** COMMAND QUEUE PORT full read/write mask. */
#define XI3CPSX_COMMAND_QUEUE_PORT_FULLRWMASK  0x00000000
/** COMMAND QUEUE PORT default value. */
#define XI3CPSX_COMMAND_QUEUE_PORT_DEFVAL   0x0

/* access_type: wo */
/** COMMAND QUEUE PORT COMMAND bit shift. */
#define XI3CPSX_COMMAND_QUEUE_PORT_COMMAND_SHIFT   0
/** COMMAND QUEUE PORT COMMAND bit width. */
#define XI3CPSX_COMMAND_QUEUE_PORT_COMMAND_WIDTH   32
/** COMMAND QUEUE PORT COMMAND bit mask. */
#define XI3CPSX_COMMAND_QUEUE_PORT_COMMAND_MASK    0xffffffff
/** COMMAND QUEUE PORT COMMAND default value. */
#define XI3CPSX_COMMAND_QUEUE_PORT_COMMAND_DEFVAL  0x0

/**
 * @brief Defines the RESPONSE_QUEUE_PORT register offset.
 */
#define XI3CPSX_RESPONSE_QUEUE_PORT    ( ( XI3CPSX_BASEADDR ) + 0x00000010 )
/** RESPONSE QUEUE PORT full mask. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_FULLMASK     0xffffffff
/** RESPONSE QUEUE PORT full read/write mask. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_FULLRWMASK  0x00000000
/** RESPONSE QUEUE PORT default value. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_DEFVAL   0x0

/* access_type: ro */
/** RESPONSE QUEUE PORT RESPONSE bit shift. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_RESPONSE_SHIFT   0
/** RESPONSE QUEUE PORT RESPONSE bit width. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_RESPONSE_WIDTH   32
/** RESPONSE QUEUE PORT RESPONSE bit mask. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_RESPONSE_MASK    0xffffffff
/** RESPONSE QUEUE PORT RESPONSE default value. */
#define XI3CPSX_RESPONSE_QUEUE_PORT_RESPONSE_DEFVAL  0x0

/** RESPONSE ERR STS bit shift. */
#define XI3CPSX_RESPONSE_ERR_STS_SHIFT			28
/** RESPONSE ERR STS bit mask. */
#define XI3CPSX_RESPONSE_ERR_STS_MASK			0xf0000000

/* RESPONSE_QUEUE_PORT error-status (ERR_STS) codes, DWC I3C databook. */
#define XI3CPSX_RESPONSE_ERR_NONE		0x0U	/**< No error */
#define XI3CPSX_RESPONSE_ERR_CRC		0x1U	/**< CRC error */
#define XI3CPSX_RESPONSE_ERR_PARITY		0x2U	/**< Parity error */
#define XI3CPSX_RESPONSE_ERR_FRAME		0x3U	/**< Frame error */
#define XI3CPSX_RESPONSE_ERR_IBA_NACK		0x4U	/**< Broadcast address NACK */
#define XI3CPSX_RESPONSE_ERR_ADDR_NACK		0x5U	/**< Address NACK (no target) */
#define XI3CPSX_RESPONSE_ERR_OVERFLOW		0x6U	/**< RX/TX buffer over/underflow */
#define XI3CPSX_RESPONSE_ERR_ABORTED		0x8U	/**< Transfer aborted */
#define XI3CPSX_RESPONSE_ERR_I2C_W_NACK		0x9U	/**< I2C write data NACK */

/**
 * @brief Defines the TX_RX_DATA_PORT register offset.
 */
#define XI3CPSX_TX_RX_DATA_PORT    ( ( XI3CPSX_BASEADDR ) + 0x00000014 )
/** TX RX DATA PORT full mask. */
#define XI3CPSX_TX_RX_DATA_PORT_FULLMASK     0xffffffff
/** TX RX DATA PORT full read/write mask. */
#define XI3CPSX_TX_RX_DATA_PORT_FULLRWMASK  0xffffffff
/** TX RX DATA PORT default value. */
#define XI3CPSX_TX_RX_DATA_PORT_DEFVAL   0x0

/* access_type: rw */
/** TX RX DATA PORT TX RX DATA PORT bit shift. */
#define XI3CPSX_TX_RX_DATA_PORT_TX_RX_DATA_PORT_SHIFT   0
/** TX RX DATA PORT TX RX DATA PORT bit width. */
#define XI3CPSX_TX_RX_DATA_PORT_TX_RX_DATA_PORT_WIDTH   32
/** TX RX DATA PORT TX RX DATA PORT bit mask. */
#define XI3CPSX_TX_RX_DATA_PORT_TX_RX_DATA_PORT_MASK    0xffffffff
/** TX RX DATA PORT TX RX DATA PORT default value. */
#define XI3CPSX_TX_RX_DATA_PORT_TX_RX_DATA_PORT_DEFVAL  0x0

/**
 * @brief Defines the IBI_QUEUE_STATUS register offset.
 */
#define XI3CPSX_IBI_QUEUE_STATUS    ( ( XI3CPSX_BASEADDR ) + 0x00000018 )
/** IBI QUEUE STATUS full mask. */
#define XI3CPSX_IBI_QUEUE_STATUS_FULLMASK     0xf000ffff
/** IBI QUEUE STATUS full read/write mask. */
#define XI3CPSX_IBI_QUEUE_STATUS_FULLRWMASK  0x00000000
/** IBI QUEUE STATUS default value. */
#define XI3CPSX_IBI_QUEUE_STATUS_DEFVAL   0x0

/* access_type: ro */
/** IBI QUEUE STATUS IBI STS bit shift. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_STS_SHIFT   28
/** IBI QUEUE STATUS IBI STS bit width. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_STS_WIDTH   4
/** IBI QUEUE STATUS IBI STS bit mask. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_STS_MASK    0xf0000000
/** IBI QUEUE STATUS IBI STS default value. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_STS_DEFVAL  0x0

/* access_type: ro */
/** IBI QUEUE STATUS IBI ID bit shift. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_ID_SHIFT   8
/** IBI QUEUE STATUS IBI ID bit width. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_ID_WIDTH   8
/** IBI QUEUE STATUS IBI ID bit mask. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_ID_MASK    0x0000ff00
/** IBI QUEUE STATUS IBI ID default value. */
#define XI3CPSX_IBI_QUEUE_STATUS_IBI_ID_DEFVAL  0x0

/* access_type: ro */
/** IBI QUEUE STATUS DATA LENGTH bit shift. */
#define XI3CPSX_IBI_QUEUE_STATUS_DATA_LENGTH_SHIFT   0
/** IBI QUEUE STATUS DATA LENGTH bit width. */
#define XI3CPSX_IBI_QUEUE_STATUS_DATA_LENGTH_WIDTH   8
/** IBI QUEUE STATUS DATA LENGTH bit mask. */
#define XI3CPSX_IBI_QUEUE_STATUS_DATA_LENGTH_MASK    0x000000ff
/** IBI QUEUE STATUS DATA LENGTH default value. */
#define XI3CPSX_IBI_QUEUE_STATUS_DATA_LENGTH_DEFVAL  0x0

/**
 * @brief Defines the QUEUE_THLD_CTRL register offset.
 */
#define XI3CPSX_QUEUE_THLD_CTRL    ( ( XI3CPSX_BASEADDR ) + 0x0000001C )
#define XI3CPSX_QUEUE_THLD_CTRL_FULLMASK     0xffffffff	/**< Mask of all valid bits in QUEUE_THLD_CTRL */
#define XI3CPSX_QUEUE_THLD_CTRL_FULLRWMASK  0xffffffff	/**< Mask of all read/write bits in QUEUE_THLD_CTRL */
#define XI3CPSX_QUEUE_THLD_CTRL_DEFVAL   0x1000101	/**< Reset value of QUEUE_THLD_CTRL */

/* access_type: rw */
/** QUEUE THLD CTRL IBI STATUS THLD bit shift. */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_STATUS_THLD_SHIFT   24
/** QUEUE THLD CTRL IBI STATUS THLD bit width. */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_STATUS_THLD_WIDTH   8
/** QUEUE THLD CTRL IBI STATUS THLD bit mask. */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_STATUS_THLD_MASK    0xff000000
/** QUEUE THLD CTRL IBI STATUS THLD default value. */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_STATUS_THLD_DEFVAL  0x1

/* access_type: rw */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_SHIFT   16	/**< Bit shift for the IBI data segment threshold field */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_WIDTH   8	/**< Bit width of the IBI data segment threshold field */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_MASK    0x00ff0000	/**< Mask for the IBI data segment threshold field */
#define XI3CPSX_QUEUE_THLD_CTRL_IBI_DATA_THLD_DEFVAL  0x0	/**< Reset value of the IBI data segment threshold field */

/* access_type: rw */
/** QUEUE THLD CTRL RESP BUF THLD bit shift. */
#define XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_SHIFT   8
/** QUEUE THLD CTRL RESP BUF THLD bit width. */
#define XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_WIDTH   8
/** QUEUE THLD CTRL RESP BUF THLD bit mask. */
#define XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_MASK    0x0000ff00
/** QUEUE THLD CTRL RESP BUF THLD default value. */
#define XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_DEFVAL  0x1

/* access_type: rw */
/** QUEUE THLD CTRL CMD EMPTY BUF THLD bit shift. */
#define XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_SHIFT   0
/** QUEUE THLD CTRL CMD EMPTY BUF THLD bit width. */
#define XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_WIDTH   8
/** QUEUE THLD CTRL CMD EMPTY BUF THLD bit mask. */
#define XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_MASK    0x000000ff
/** QUEUE THLD CTRL CMD EMPTY BUF THLD default value. */
#define XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_DEFVAL  0x1

/**
 * @brief Defines the DATA_BUFFER_THLD_CTRL register offset.
 */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL    ( ( XI3CPSX_BASEADDR ) + 0x00000020 )
/** DATA BUFFER THLD CTRL full mask. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_FULLMASK     0x07070707
/** DATA BUFFER THLD CTRL full read/write mask. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_FULLRWMASK  0x07070707
/** DATA BUFFER THLD CTRL default value. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_DEFVAL   0x1010101

/* access_type: rw */
/** DATA BUFFER THLD CTRL RX START THLD bit shift. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_SHIFT   24
/** DATA BUFFER THLD CTRL RX START THLD bit width. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_WIDTH   3
/** DATA BUFFER THLD CTRL RX START THLD bit mask. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_MASK    0x07000000
/** DATA BUFFER THLD CTRL RX START THLD default value. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_DEFVAL  0x1

/* access_type: rw */
/** DATA BUFFER THLD CTRL TX START THLD bit shift. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_START_THLD_SHIFT   16
/** DATA BUFFER THLD CTRL TX START THLD bit width. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_START_THLD_WIDTH   3
/** DATA BUFFER THLD CTRL TX START THLD bit mask. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_START_THLD_MASK    0x00070000
/** DATA BUFFER THLD CTRL TX START THLD default value. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_START_THLD_DEFVAL  0x1

/* access_type: rw */
/** DATA BUFFER THLD CTRL RX BUF THLD bit shift. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_SHIFT   8
/** DATA BUFFER THLD CTRL RX BUF THLD bit width. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_WIDTH   3
/** DATA BUFFER THLD CTRL RX BUF THLD bit mask. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_MASK    0x00000700
/** DATA BUFFER THLD CTRL RX BUF THLD default value. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_DEFVAL  0x1

/* access_type: rw */
/** DATA BUFFER THLD CTRL TX EMPTY BUF THLD bit shift. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_SHIFT   0
/** DATA BUFFER THLD CTRL TX EMPTY BUF THLD bit width. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_WIDTH   3
/** DATA BUFFER THLD CTRL TX EMPTY BUF THLD bit mask. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_MASK    0x00000007
/** DATA BUFFER THLD CTRL TX EMPTY BUF THLD default value. */
#define XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_DEFVAL  0x1

/**
 * @brief Defines the IBI_QUEUE_CTRL register offset.
 */
#define XI3CPSX_IBI_QUEUE_CTRL    ( ( XI3CPSX_BASEADDR ) + 0x00000024 )
/** IBI QUEUE CTRL full mask. */
#define XI3CPSX_IBI_QUEUE_CTRL_FULLMASK     0x0000000b
/** IBI QUEUE CTRL full read/write mask. */
#define XI3CPSX_IBI_QUEUE_CTRL_FULLRWMASK  0x0000000b
/** IBI QUEUE CTRL default value. */
#define XI3CPSX_IBI_QUEUE_CTRL_DEFVAL   0x0

/* access_type: rw */
/** IBI QUEUE CTRL NOTIFY SIR REJECTED bit shift. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_SIR_REJECTED_SHIFT   3
/** IBI QUEUE CTRL NOTIFY SIR REJECTED bit width. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_SIR_REJECTED_WIDTH   1
/** IBI QUEUE CTRL NOTIFY SIR REJECTED bit mask. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_SIR_REJECTED_MASK    0x00000008
/** IBI QUEUE CTRL NOTIFY SIR REJECTED default value. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_SIR_REJECTED_DEFVAL  0x0

/* access_type: rw */
/** IBI QUEUE CTRL NOTIFY MR REJECTED bit shift. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_MR_REJECTED_SHIFT   1
/** IBI QUEUE CTRL NOTIFY MR REJECTED bit width. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_MR_REJECTED_WIDTH   1
/** IBI QUEUE CTRL NOTIFY MR REJECTED bit mask. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_MR_REJECTED_MASK    0x00000002
/** IBI QUEUE CTRL NOTIFY MR REJECTED default value. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_MR_REJECTED_DEFVAL  0x0

/* access_type: rw */
/** IBI QUEUE CTRL NOTIFY HJ REJECTED bit shift. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_HJ_REJECTED_SHIFT   0
/** IBI QUEUE CTRL NOTIFY HJ REJECTED bit width. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_HJ_REJECTED_WIDTH   1
/** IBI QUEUE CTRL NOTIFY HJ REJECTED bit mask. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_HJ_REJECTED_MASK    0x00000001
/** IBI QUEUE CTRL NOTIFY HJ REJECTED default value. */
#define XI3CPSX_IBI_QUEUE_CTRL_NOTIFY_HJ_REJECTED_DEFVAL  0x0

/**
 * @brief Defines the IBI_MR_REQ_REJECT register offset.
 */
#define XI3CPSX_IBI_MR_REQ_REJECT    ( ( XI3CPSX_BASEADDR ) + 0x0000002C )
/** IBI MR REQ REJECT full mask. */
#define XI3CPSX_IBI_MR_REQ_REJECT_FULLMASK     0xffffffff
/** IBI MR REQ REJECT full read/write mask. */
#define XI3CPSX_IBI_MR_REQ_REJECT_FULLRWMASK  0xffffffff
/** IBI MR REQ REJECT default value. */
#define XI3CPSX_IBI_MR_REQ_REJECT_DEFVAL   0x0

/* access_type: rw */
/** IBI MR REQ REJECT MR REQ REJECT bit shift. */
#define XI3CPSX_IBI_MR_REQ_REJECT_MR_REQ_REJECT_SHIFT   0
/** IBI MR REQ REJECT MR REQ REJECT bit width. */
#define XI3CPSX_IBI_MR_REQ_REJECT_MR_REQ_REJECT_WIDTH   32
/** IBI MR REQ REJECT MR REQ REJECT bit mask. */
#define XI3CPSX_IBI_MR_REQ_REJECT_MR_REQ_REJECT_MASK    0xffffffff
/** IBI MR REQ REJECT MR REQ REJECT default value. */
#define XI3CPSX_IBI_MR_REQ_REJECT_MR_REQ_REJECT_DEFVAL  0x0

/**
 * @brief Defines the IBI_SIR_REQ_REJECT register offset.
 */
#define XI3CPSX_IBI_SIR_REQ_REJECT    ( ( XI3CPSX_BASEADDR ) + 0x00000030 )
/** IBI SIR REQ REJECT full mask. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_FULLMASK     0xffffffff
/** IBI SIR REQ REJECT full read/write mask. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_FULLRWMASK  0xffffffff
/** IBI SIR REQ REJECT default value. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_DEFVAL   0x0

/* access_type: rw */
/** IBI SIR REQ REJECT SIR REQ REJECT bit shift. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT_SHIFT   0
/** IBI SIR REQ REJECT SIR REQ REJECT bit width. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT_WIDTH   32
/** IBI SIR REQ REJECT SIR REQ REJECT bit mask. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT_MASK    0xffffffff
/** IBI SIR REQ REJECT SIR REQ REJECT default value. */
#define XI3CPSX_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT_DEFVAL  0x0

/**
 * @brief Defines the RESET_CTRL register offset.
 */
#define XI3CPSX_RESET_CTRL    ( ( XI3CPSX_BASEADDR ) + 0x00000034 )
/** RESET CTRL full mask. */
#define XI3CPSX_RESET_CTRL_FULLMASK     0xe000003f
/** RESET CTRL full read/write mask. */
#define XI3CPSX_RESET_CTRL_FULLRWMASK  0xe000003f
#define XI3CPSX_RESET_CTRL_DEFVAL   0x0	/**< RESET CTRL default value. */

/* access_type: rw */
/** RESET CTRL BUS RESET bit shift. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_SHIFT   31
/** RESET CTRL BUS RESET bit width. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_WIDTH   1
/** RESET CTRL BUS RESET bit mask. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_MASK    0x80000000
/** RESET CTRL BUS RESET default value. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_DEFVAL  0x0

/* access_type: rw */
/** RESET CTRL BUS RESET TYPE bit shift. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_TYPE_SHIFT   29
/** RESET CTRL BUS RESET TYPE bit width. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_TYPE_WIDTH   2
/** RESET CTRL BUS RESET TYPE bit mask. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_TYPE_MASK    0x60000000
/** RESET CTRL BUS RESET TYPE default value. */
#define XI3CPSX_RESET_CTRL_BUS_RESET_TYPE_DEFVAL  0x0

/* access_type: rw */
/** RESET CTRL BI QUEUE RST bit shift. */
#define XI3CPSX_RESET_CTRL_BI_QUEUE_RST_SHIFT   5
/** RESET CTRL BI QUEUE RST bit width. */
#define XI3CPSX_RESET_CTRL_BI_QUEUE_RST_WIDTH   1
/** RESET CTRL BI QUEUE RST bit mask. */
#define XI3CPSX_RESET_CTRL_BI_QUEUE_RST_MASK    0x00000020
/** RESET CTRL BI QUEUE RST default value. */
#define XI3CPSX_RESET_CTRL_BI_QUEUE_RST_DEFVAL  0x0

/* access_type: rw */
/** RESET CTRL RX FIFO RST bit shift. */
#define XI3CPSX_RESET_CTRL_RX_FIFO_RST_SHIFT   4
/** RESET CTRL RX FIFO RST bit width. */
#define XI3CPSX_RESET_CTRL_RX_FIFO_RST_WIDTH   1
/** RESET CTRL RX FIFO RST bit mask. */
#define XI3CPSX_RESET_CTRL_RX_FIFO_RST_MASK    0x00000010
/** RESET CTRL RX FIFO RST default value. */
#define XI3CPSX_RESET_CTRL_RX_FIFO_RST_DEFVAL  0x0

/* access_type: rw */
/** RESET CTRL TX FIFO RST bit shift. */
#define XI3CPSX_RESET_CTRL_TX_FIFO_RST_SHIFT   3
/** RESET CTRL TX FIFO RST bit width. */
#define XI3CPSX_RESET_CTRL_TX_FIFO_RST_WIDTH   1
/** RESET CTRL TX FIFO RST bit mask. */
#define XI3CPSX_RESET_CTRL_TX_FIFO_RST_MASK    0x00000008
/** RESET CTRL TX FIFO RST default value. */
#define XI3CPSX_RESET_CTRL_TX_FIFO_RST_DEFVAL  0x0

/** RESET CTRL ALL FIFOS RST VAL register field. */
#define XI3CPSX_RESET_CTRL_ALL_FIFOS_RST_VAL   0x1E

/* access_type: rw */
/** RESET CTRL RESP QUEUE RST bit shift. */
#define XI3CPSX_RESET_CTRL_RESP_QUEUE_RST_SHIFT   2
/** RESET CTRL RESP QUEUE RST bit width. */
#define XI3CPSX_RESET_CTRL_RESP_QUEUE_RST_WIDTH   1
/** RESET CTRL RESP QUEUE RST bit mask. */
#define XI3CPSX_RESET_CTRL_RESP_QUEUE_RST_MASK    0x00000004
/** RESET CTRL RESP QUEUE RST default value. */
#define XI3CPSX_RESET_CTRL_RESP_QUEUE_RST_DEFVAL  0x0

/* access_type: rw */
/** RESET CTRL CMD QUEUE RST bit shift. */
#define XI3CPSX_RESET_CTRL_CMD_QUEUE_RST_SHIFT   1
/** RESET CTRL CMD QUEUE RST bit width. */
#define XI3CPSX_RESET_CTRL_CMD_QUEUE_RST_WIDTH   1
/** RESET CTRL CMD QUEUE RST bit mask. */
#define XI3CPSX_RESET_CTRL_CMD_QUEUE_RST_MASK    0x00000002
/** RESET CTRL CMD QUEUE RST default value. */
#define XI3CPSX_RESET_CTRL_CMD_QUEUE_RST_DEFVAL  0x0

/* access_type: rw */
/** RESET CTRL SOFT RST bit shift. */
#define XI3CPSX_RESET_CTRL_SOFT_RST_SHIFT   0
/** RESET CTRL SOFT RST bit width. */
#define XI3CPSX_RESET_CTRL_SOFT_RST_WIDTH   1
/** RESET CTRL SOFT RST bit mask. */
#define XI3CPSX_RESET_CTRL_SOFT_RST_MASK    0x00000001
/** RESET CTRL SOFT RST default value. */
#define XI3CPSX_RESET_CTRL_SOFT_RST_DEFVAL  0x0
/** RESET CTRL SOFT RST VAL register field. */
#define XI3CPSX_RESET_CTRL_SOFT_RST_VAL     0x1

/**
 * @brief Defines the SLV_EVENT_STATUS register offset.
 */
#define XI3CPSX_SLV_EVENT_STATUS    ( ( XI3CPSX_BASEADDR ) + 0x00000038 )
/** SLV EVENT STATUS full mask. */
#define XI3CPSX_SLV_EVENT_STATUS_FULLMASK     0x000000fb
/** SLV EVENT STATUS full read/write mask. */
#define XI3CPSX_SLV_EVENT_STATUS_FULLRWMASK  0x00000008
/** SLV EVENT STATUS default value. */
#define XI3CPSX_SLV_EVENT_STATUS_DEFVAL   0xb

/* access_type: wtc */
/** SLV EVENT STATUS MWL UPDATED bit shift. */
#define XI3CPSX_SLV_EVENT_STATUS_MWL_UPDATED_SHIFT   7
/** SLV EVENT STATUS MWL UPDATED bit width. */
#define XI3CPSX_SLV_EVENT_STATUS_MWL_UPDATED_WIDTH   1
/** SLV EVENT STATUS MWL UPDATED bit mask. */
#define XI3CPSX_SLV_EVENT_STATUS_MWL_UPDATED_MASK    0x00000080
/** SLV EVENT STATUS MWL UPDATED default value. */
#define XI3CPSX_SLV_EVENT_STATUS_MWL_UPDATED_DEFVAL  0x0

/* access_type: wtc */
/** SLV EVENT STATUS MRL UPDATED bit shift. */
#define XI3CPSX_SLV_EVENT_STATUS_MRL_UPDATED_SHIFT   6
/** SLV EVENT STATUS MRL UPDATED bit width. */
#define XI3CPSX_SLV_EVENT_STATUS_MRL_UPDATED_WIDTH   1
/** SLV EVENT STATUS MRL UPDATED bit mask. */
#define XI3CPSX_SLV_EVENT_STATUS_MRL_UPDATED_MASK    0x00000040
/** SLV EVENT STATUS MRL UPDATED default value. */
#define XI3CPSX_SLV_EVENT_STATUS_MRL_UPDATED_DEFVAL  0x0

/* access_type: ro */
/** SLV EVENT STATUS ACTIVITY STATE bit shift. */
#define XI3CPSX_SLV_EVENT_STATUS_ACTIVITY_STATE_SHIFT   4
/** SLV EVENT STATUS ACTIVITY STATE bit width. */
#define XI3CPSX_SLV_EVENT_STATUS_ACTIVITY_STATE_WIDTH   2
/** SLV EVENT STATUS ACTIVITY STATE bit mask. */
#define XI3CPSX_SLV_EVENT_STATUS_ACTIVITY_STATE_MASK    0x00000030
/** SLV EVENT STATUS ACTIVITY STATE default value. */
#define XI3CPSX_SLV_EVENT_STATUS_ACTIVITY_STATE_DEFVAL  0x0

/* access_type: rw */
/** SLV EVENT STATUS HJ EN bit shift. */
#define XI3CPSX_SLV_EVENT_STATUS_HJ_EN_SHIFT   3
/** SLV EVENT STATUS HJ EN bit width. */
#define XI3CPSX_SLV_EVENT_STATUS_HJ_EN_WIDTH   1
/** SLV EVENT STATUS HJ EN bit mask. */
#define XI3CPSX_SLV_EVENT_STATUS_HJ_EN_MASK    0x00000008
/** SLV EVENT STATUS HJ EN default value. */
#define XI3CPSX_SLV_EVENT_STATUS_HJ_EN_DEFVAL  0x1

/* access_type: ro */
/** SLV EVENT STATUS MR EN bit shift. */
#define XI3CPSX_SLV_EVENT_STATUS_MR_EN_SHIFT   1
/** SLV EVENT STATUS MR EN bit width. */
#define XI3CPSX_SLV_EVENT_STATUS_MR_EN_WIDTH   1
/** SLV EVENT STATUS MR EN bit mask. */
#define XI3CPSX_SLV_EVENT_STATUS_MR_EN_MASK    0x00000002
/** SLV EVENT STATUS MR EN default value. */
#define XI3CPSX_SLV_EVENT_STATUS_MR_EN_DEFVAL  0x1

/* access_type: ro */
/** SLV EVENT STATUS SIR EN bit shift. */
#define XI3CPSX_SLV_EVENT_STATUS_SIR_EN_SHIFT   0
/** SLV EVENT STATUS SIR EN bit width. */
#define XI3CPSX_SLV_EVENT_STATUS_SIR_EN_WIDTH   1
/** SLV EVENT STATUS SIR EN bit mask. */
#define XI3CPSX_SLV_EVENT_STATUS_SIR_EN_MASK    0x00000001
/** SLV EVENT STATUS SIR EN default value. */
#define XI3CPSX_SLV_EVENT_STATUS_SIR_EN_DEFVAL  0x1

/**
 * @brief Defines the INTR_STATUS register offset.
 */
#define XI3CPSX_INTR_STATUS    ( ( XI3CPSX_BASEADDR ) + 0x0000003C )
/** INTR STATUS full mask. */
#define XI3CPSX_INTR_STATUS_FULLMASK     0x0000bf7f
/** INTR STATUS full read/write mask. */
#define XI3CPSX_INTR_STATUS_FULLRWMASK  0x00000000
/** INTR STATUS default value. */
#define XI3CPSX_INTR_STATUS_DEFVAL   0x0

/* access_type: wtc */
/** INTR STATUS BUS RESET DONE STS bit shift. */
#define XI3CPSX_INTR_STATUS_BUS_RESET_DONE_STS_SHIFT   15
/** INTR STATUS BUS RESET DONE STS bit width. */
#define XI3CPSX_INTR_STATUS_BUS_RESET_DONE_STS_WIDTH   1
/** INTR STATUS BUS RESET DONE STS bit mask. */
#define XI3CPSX_INTR_STATUS_BUS_RESET_DONE_STS_MASK    0x00008000
/** INTR STATUS BUS RESET DONE STS default value. */
#define XI3CPSX_INTR_STATUS_BUS_RESET_DONE_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS BUSOWNER UPDATED STS bit shift. */
#define XI3CPSX_INTR_STATUS_BUSOWNER_UPDATED_STS_SHIFT   13
/** INTR STATUS BUSOWNER UPDATED STS bit width. */
#define XI3CPSX_INTR_STATUS_BUSOWNER_UPDATED_STS_WIDTH   1
/** INTR STATUS BUSOWNER UPDATED STS bit mask. */
#define XI3CPSX_INTR_STATUS_BUSOWNER_UPDATED_STS_MASK    0x00002000
/** INTR STATUS BUSOWNER UPDATED STS default value. */
#define XI3CPSX_INTR_STATUS_BUSOWNER_UPDATED_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS IBI UPDATED STS bit shift. */
#define XI3CPSX_INTR_STATUS_IBI_UPDATED_STS_SHIFT   12
/** INTR STATUS IBI UPDATED STS bit width. */
#define XI3CPSX_INTR_STATUS_IBI_UPDATED_STS_WIDTH   1
/** INTR STATUS IBI UPDATED STS bit mask. */
#define XI3CPSX_INTR_STATUS_IBI_UPDATED_STS_MASK    0x00001000
/** INTR STATUS IBI UPDATED STS default value. */
#define XI3CPSX_INTR_STATUS_IBI_UPDATED_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS READ REQ RECV STS bit shift. */
#define XI3CPSX_INTR_STATUS_READ_REQ_RECV_STS_SHIFT   11
/** INTR STATUS READ REQ RECV STS bit width. */
#define XI3CPSX_INTR_STATUS_READ_REQ_RECV_STS_WIDTH   1
/** INTR STATUS READ REQ RECV STS bit mask. */
#define XI3CPSX_INTR_STATUS_READ_REQ_RECV_STS_MASK    0x00000800
/** INTR STATUS READ REQ RECV STS default value. */
#define XI3CPSX_INTR_STATUS_READ_REQ_RECV_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS DEFSLV STS bit shift. */
#define XI3CPSX_INTR_STATUS_DEFSLV_STS_SHIFT   10
/** INTR STATUS DEFSLV STS bit width. */
#define XI3CPSX_INTR_STATUS_DEFSLV_STS_WIDTH   1
/** INTR STATUS DEFSLV STS bit mask. */
#define XI3CPSX_INTR_STATUS_DEFSLV_STS_MASK    0x00000400
/** INTR STATUS DEFSLV STS default value. */
#define XI3CPSX_INTR_STATUS_DEFSLV_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS TRANSFER ERR STS bit shift. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ERR_STS_SHIFT   9
/** INTR STATUS TRANSFER ERR STS bit width. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ERR_STS_WIDTH   1
/** INTR STATUS TRANSFER ERR STS bit mask. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ERR_STS_MASK    0x00000200
/** INTR STATUS TRANSFER ERR STS default value. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ERR_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS DYN ADDR ASSGN STS bit shift. */
#define XI3CPSX_INTR_STATUS_DYN_ADDR_ASSGN_STS_SHIFT   8
/** INTR STATUS DYN ADDR ASSGN STS bit width. */
#define XI3CPSX_INTR_STATUS_DYN_ADDR_ASSGN_STS_WIDTH   1
/** INTR STATUS DYN ADDR ASSGN STS bit mask. */
#define XI3CPSX_INTR_STATUS_DYN_ADDR_ASSGN_STS_MASK    0x00000100
/** INTR STATUS DYN ADDR ASSGN STS default value. */
#define XI3CPSX_INTR_STATUS_DYN_ADDR_ASSGN_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS CCC UPDATED STS bit shift. */
#define XI3CPSX_INTR_STATUS_CCC_UPDATED_STS_SHIFT   6
/** INTR STATUS CCC UPDATED STS bit width. */
#define XI3CPSX_INTR_STATUS_CCC_UPDATED_STS_WIDTH   1
/** INTR STATUS CCC UPDATED STS bit mask. */
#define XI3CPSX_INTR_STATUS_CCC_UPDATED_STS_MASK    0x00000040
/** INTR STATUS CCC UPDATED STS default value. */
#define XI3CPSX_INTR_STATUS_CCC_UPDATED_STS_DEFVAL  0x0

/* access_type: wtc */
/** INTR STATUS TRANSFER ABORT STS bit shift. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ABORT_STS_SHIFT   5
/** INTR STATUS TRANSFER ABORT STS bit width. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ABORT_STS_WIDTH   1
/** INTR STATUS TRANSFER ABORT STS bit mask. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ABORT_STS_MASK    0x00000020
/** INTR STATUS TRANSFER ABORT STS default value. */
#define XI3CPSX_INTR_STATUS_TRANSFER_ABORT_STS_DEFVAL  0x0

/* access_type: ro */
/** INTR STATUS RESP READY STS bit shift. */
#define XI3CPSX_INTR_STATUS_RESP_READY_STS_SHIFT   4
/** INTR STATUS RESP READY STS bit width. */
#define XI3CPSX_INTR_STATUS_RESP_READY_STS_WIDTH   1
/** INTR STATUS RESP READY STS bit mask. */
#define XI3CPSX_INTR_STATUS_RESP_READY_STS_MASK    0x00000010
/** INTR STATUS RESP READY STS default value. */
#define XI3CPSX_INTR_STATUS_RESP_READY_STS_DEFVAL  0x0

/* access_type: ro */
/** INTR STATUS CMD QUEUE READY STS bit shift. */
#define XI3CPSX_INTR_STATUS_CMD_QUEUE_READY_STS_SHIFT   3
/** INTR STATUS CMD QUEUE READY STS bit width. */
#define XI3CPSX_INTR_STATUS_CMD_QUEUE_READY_STS_WIDTH   1
/** INTR STATUS CMD QUEUE READY STS bit mask. */
#define XI3CPSX_INTR_STATUS_CMD_QUEUE_READY_STS_MASK    0x00000008
/** INTR STATUS CMD QUEUE READY STS default value. */
#define XI3CPSX_INTR_STATUS_CMD_QUEUE_READY_STS_DEFVAL  0x0

/* access_type: ro */
/** INTR STATUS IBI THLD STS bit shift. */
#define XI3CPSX_INTR_STATUS_IBI_THLD_STS_SHIFT   2
/** INTR STATUS IBI THLD STS bit width. */
#define XI3CPSX_INTR_STATUS_IBI_THLD_STS_WIDTH   1
/** INTR STATUS IBI THLD STS bit mask. */
#define XI3CPSX_INTR_STATUS_IBI_THLD_STS_MASK    0x00000004
/** INTR STATUS IBI THLD STS default value. */
#define XI3CPSX_INTR_STATUS_IBI_THLD_STS_DEFVAL  0x0

/* access_type: ro */
/** INTR STATUS RX THLD STS bit shift. */
#define XI3CPSX_INTR_STATUS_RX_THLD_STS_SHIFT   1
/** INTR STATUS RX THLD STS bit width. */
#define XI3CPSX_INTR_STATUS_RX_THLD_STS_WIDTH   1
/** INTR STATUS RX THLD STS bit mask. */
#define XI3CPSX_INTR_STATUS_RX_THLD_STS_MASK    0x00000002
/** INTR STATUS RX THLD STS default value. */
#define XI3CPSX_INTR_STATUS_RX_THLD_STS_DEFVAL  0x0

/* access_type: ro */
/** INTR STATUS TX THLD STS bit shift. */
#define XI3CPSX_INTR_STATUS_TX_THLD_STS_SHIFT   0
/** INTR STATUS TX THLD STS bit width. */
#define XI3CPSX_INTR_STATUS_TX_THLD_STS_WIDTH   1
/** INTR STATUS TX THLD STS bit mask. */
#define XI3CPSX_INTR_STATUS_TX_THLD_STS_MASK    0x00000001
/** INTR STATUS TX THLD STS default value. */
#define XI3CPSX_INTR_STATUS_TX_THLD_STS_DEFVAL  0x0

/**
 * @brief Defines the INTR_STATUS_EN register offset.
 */
#define XI3CPSX_INTR_STATUS_EN    ( ( XI3CPSX_BASEADDR ) + 0x00000040 )
/** INTR STATUS EN full mask. */
#define XI3CPSX_INTR_STATUS_EN_FULLMASK     0x0000bf7f
/** INTR STATUS EN full read/write mask. */
#define XI3CPSX_INTR_STATUS_EN_FULLRWMASK  0x0000bf7f
/** INTR STATUS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_DEFVAL   0x0

/* access_type: rw */
/** INTR STATUS EN BUS RESET DONE STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_BUS_RESET_DONE_STS_EN_SHIFT   15
/** INTR STATUS EN BUS RESET DONE STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_BUS_RESET_DONE_STS_EN_WIDTH   1
/** INTR STATUS EN BUS RESET DONE STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_BUS_RESET_DONE_STS_EN_MASK    0x00008000
/** INTR STATUS EN BUS RESET DONE STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_BUS_RESET_DONE_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN BUSOWNER UPDATED STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN_SHIFT   13
/** INTR STATUS EN BUSOWNER UPDATED STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN_WIDTH   1
/** INTR STATUS EN BUSOWNER UPDATED STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN_MASK    0x00002000
/** INTR STATUS EN BUSOWNER UPDATED STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN IBI UPDATED STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_IBI_UPDATED_STS_EN_SHIFT   12
/** INTR STATUS EN IBI UPDATED STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_IBI_UPDATED_STS_EN_WIDTH   1
/** INTR STATUS EN IBI UPDATED STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_IBI_UPDATED_STS_EN_MASK    0x00001000
/** INTR STATUS EN IBI UPDATED STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_IBI_UPDATED_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN READ REQ RECV STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_READ_REQ_RECV_STS_EN_SHIFT   11
/** INTR STATUS EN READ REQ RECV STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_READ_REQ_RECV_STS_EN_WIDTH   1
/** INTR STATUS EN READ REQ RECV STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_READ_REQ_RECV_STS_EN_MASK    0x00000800
/** INTR STATUS EN READ REQ RECV STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_READ_REQ_RECV_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN DEFSLV STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_DEFSLV_STS_EN_SHIFT   10
/** INTR STATUS EN DEFSLV STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_DEFSLV_STS_EN_WIDTH   1
/** INTR STATUS EN DEFSLV STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_DEFSLV_STS_EN_MASK    0x00000400
/** INTR STATUS EN DEFSLV STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_DEFSLV_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN TRANSFER ERR STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ERR_STS_EN_SHIFT   9
/** INTR STATUS EN TRANSFER ERR STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ERR_STS_EN_WIDTH   1
/** INTR STATUS EN TRANSFER ERR STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ERR_STS_EN_MASK    0x00000200
/** INTR STATUS EN TRANSFER ERR STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ERR_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN DYN ADDR ASSGN STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_DYN_ADDR_ASSGN_STS_EN_SHIFT   8
/** INTR STATUS EN DYN ADDR ASSGN STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_DYN_ADDR_ASSGN_STS_EN_WIDTH   1
/** INTR STATUS EN DYN ADDR ASSGN STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_DYN_ADDR_ASSGN_STS_EN_MASK    0x00000100
/** INTR STATUS EN DYN ADDR ASSGN STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_DYN_ADDR_ASSGN_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN CCC UPDATED STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_CCC_UPDATED_STS_EN_SHIFT   6
/** INTR STATUS EN CCC UPDATED STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_CCC_UPDATED_STS_EN_WIDTH   1
/** INTR STATUS EN CCC UPDATED STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_CCC_UPDATED_STS_EN_MASK    0x00000040
/** INTR STATUS EN CCC UPDATED STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_CCC_UPDATED_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN TRANSFER ABORT STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ABORT_STS_EN_SHIFT   5
/** INTR STATUS EN TRANSFER ABORT STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ABORT_STS_EN_WIDTH   1
/** INTR STATUS EN TRANSFER ABORT STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ABORT_STS_EN_MASK    0x00000020
/** INTR STATUS EN TRANSFER ABORT STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_TRANSFER_ABORT_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN RESP READY STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_RESP_READY_STS_EN_SHIFT   4
/** INTR STATUS EN RESP READY STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_RESP_READY_STS_EN_WIDTH   1
/** INTR STATUS EN RESP READY STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_RESP_READY_STS_EN_MASK    0x00000010
/** INTR STATUS EN RESP READY STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_RESP_READY_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN CMD QUEUE READY STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_CMD_QUEUE_READY_STS_EN_SHIFT   3
/** INTR STATUS EN CMD QUEUE READY STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_CMD_QUEUE_READY_STS_EN_WIDTH   1
/** INTR STATUS EN CMD QUEUE READY STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_CMD_QUEUE_READY_STS_EN_MASK    0x00000008
/** INTR STATUS EN CMD QUEUE READY STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_CMD_QUEUE_READY_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN IBI THLD STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_IBI_THLD_STS_EN_SHIFT   2
/** INTR STATUS EN IBI THLD STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_IBI_THLD_STS_EN_WIDTH   1
/** INTR STATUS EN IBI THLD STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_IBI_THLD_STS_EN_MASK    0x00000004
/** INTR STATUS EN IBI THLD STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_IBI_THLD_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN RX THLD STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_RX_THLD_STS_EN_SHIFT   1
/** INTR STATUS EN RX THLD STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_RX_THLD_STS_EN_WIDTH   1
/** INTR STATUS EN RX THLD STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_RX_THLD_STS_EN_MASK    0x00000002
/** INTR STATUS EN RX THLD STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_RX_THLD_STS_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR STATUS EN TX THLD STS EN bit shift. */
#define XI3CPSX_INTR_STATUS_EN_TX_THLD_STS_EN_SHIFT   0
/** INTR STATUS EN TX THLD STS EN bit width. */
#define XI3CPSX_INTR_STATUS_EN_TX_THLD_STS_EN_WIDTH   1
/** INTR STATUS EN TX THLD STS EN bit mask. */
#define XI3CPSX_INTR_STATUS_EN_TX_THLD_STS_EN_MASK    0x00000001
/** INTR STATUS EN TX THLD STS EN default value. */
#define XI3CPSX_INTR_STATUS_EN_TX_THLD_STS_EN_DEFVAL  0x0

/**
 * @brief Defines the INTR_SIGNAL_EN register offset.
 */
#define XI3CPSX_INTR_SIGNAL_EN    ( ( XI3CPSX_BASEADDR ) + 0x00000044 )
/** INTR SIGNAL EN full mask. */
#define XI3CPSX_INTR_SIGNAL_EN_FULLMASK     0x0000bf7f
/** INTR SIGNAL EN full read/write mask. */
#define XI3CPSX_INTR_SIGNAL_EN_FULLRWMASK  0x0000bf7f
/** INTR SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_DEFVAL   0x0

/* access_type: rw */
/** INTR SIGNAL EN BUS RESET DONE SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_BUS_RESET_DONE_SIGNAL_EN_SHIFT   15
/** INTR SIGNAL EN BUS RESET DONE SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_BUS_RESET_DONE_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN BUS RESET DONE SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_BUS_RESET_DONE_SIGNAL_EN_MASK    0x00008000
/** INTR SIGNAL EN BUS RESET DONE SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_BUS_RESET_DONE_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN BUSOWNER UPDATED SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_BUSOWNER_UPDATED_SIGNAL_EN_SHIFT   13
/** INTR SIGNAL EN BUSOWNER UPDATED SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_BUSOWNER_UPDATED_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN BUSOWNER UPDATED SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_BUSOWNER_UPDATED_SIGNAL_EN_MASK    0x00002000
/** INTR SIGNAL EN BUSOWNER UPDATED SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_BUSOWNER_UPDATED_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN IBI UPDATED SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_UPDATED_SIGNAL_EN_SHIFT   12
/** INTR SIGNAL EN IBI UPDATED SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_UPDATED_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN IBI UPDATED SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_UPDATED_SIGNAL_EN_MASK    0x00001000
/** INTR SIGNAL EN IBI UPDATED SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_UPDATED_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN READ REQ RECV SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_READ_REQ_RECV_SIGNAL_EN_SHIFT   11
/** INTR SIGNAL EN READ REQ RECV SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_READ_REQ_RECV_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN READ REQ RECV SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_READ_REQ_RECV_SIGNAL_EN_MASK    0x00000800
/** INTR SIGNAL EN READ REQ RECV SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_READ_REQ_RECV_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN DEFSLV SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_DEFSLV_SIGNAL_EN_SHIFT   10
/** INTR SIGNAL EN DEFSLV SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_DEFSLV_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN DEFSLV SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_DEFSLV_SIGNAL_EN_MASK    0x00000400
/** INTR SIGNAL EN DEFSLV SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_DEFSLV_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN TRANSFER ERR SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ERR_SIGNAL_EN_SHIFT   9
/** INTR SIGNAL EN TRANSFER ERR SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ERR_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN TRANSFER ERR SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ERR_SIGNAL_EN_MASK    0x00000200
/** INTR SIGNAL EN TRANSFER ERR SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ERR_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN DYN ADDR ASSGN SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_DYN_ADDR_ASSGN_SIGNAL_EN_SHIFT   8
/** INTR SIGNAL EN DYN ADDR ASSGN SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_DYN_ADDR_ASSGN_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN DYN ADDR ASSGN SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_DYN_ADDR_ASSGN_SIGNAL_EN_MASK    0x00000100
/** INTR SIGNAL EN DYN ADDR ASSGN SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_DYN_ADDR_ASSGN_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN CCC UPDATED SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_CCC_UPDATED_SIGNAL_EN_SHIFT   6
/** INTR SIGNAL EN CCC UPDATED SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_CCC_UPDATED_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN CCC UPDATED SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_CCC_UPDATED_SIGNAL_EN_MASK    0x00000040
/** INTR SIGNAL EN CCC UPDATED SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_CCC_UPDATED_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN TRANSFER ABORT SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ABORT_SIGNAL_EN_SHIFT   5
/** INTR SIGNAL EN TRANSFER ABORT SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ABORT_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN TRANSFER ABORT SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ABORT_SIGNAL_EN_MASK    0x00000020
/** INTR SIGNAL EN TRANSFER ABORT SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_TRANSFER_ABORT_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN RESP READY SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_SHIFT   4
/** INTR SIGNAL EN RESP READY SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN RESP READY SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_MASK    0x00000010
/** INTR SIGNAL EN RESP READY SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_RESP_READY_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN CMD QUEUE READY SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_CMD_QUEUE_READY_SIGNAL_EN_SHIFT   3
/** INTR SIGNAL EN CMD QUEUE READY SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_CMD_QUEUE_READY_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN CMD QUEUE READY SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_CMD_QUEUE_READY_SIGNAL_EN_MASK    0x00000008
/** INTR SIGNAL EN CMD QUEUE READY SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_CMD_QUEUE_READY_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN IBI THLD SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_THLD_SIGNAL_EN_SHIFT   2
/** INTR SIGNAL EN IBI THLD SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_THLD_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN IBI THLD SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_THLD_SIGNAL_EN_MASK    0x00000004
/** INTR SIGNAL EN IBI THLD SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_IBI_THLD_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN RX THLD SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_RX_THLD_SIGNAL_EN_SHIFT   1
/** INTR SIGNAL EN RX THLD SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_RX_THLD_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN RX THLD SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_RX_THLD_SIGNAL_EN_MASK    0x00000002
/** INTR SIGNAL EN RX THLD SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_RX_THLD_SIGNAL_EN_DEFVAL  0x0

/* access_type: rw */
/** INTR SIGNAL EN TX THLD SIGNAL EN bit shift. */
#define XI3CPSX_INTR_SIGNAL_EN_TX_THLD_SIGNAL_EN_SHIFT   0
/** INTR SIGNAL EN TX THLD SIGNAL EN bit width. */
#define XI3CPSX_INTR_SIGNAL_EN_TX_THLD_SIGNAL_EN_WIDTH   1
/** INTR SIGNAL EN TX THLD SIGNAL EN bit mask. */
#define XI3CPSX_INTR_SIGNAL_EN_TX_THLD_SIGNAL_EN_MASK    0x00000001
/** INTR SIGNAL EN TX THLD SIGNAL EN default value. */
#define XI3CPSX_INTR_SIGNAL_EN_TX_THLD_SIGNAL_EN_DEFVAL  0x0

/**
 * @brief Defines the INTR_FORCE register offset.
 */
#define XI3CPSX_INTR_FORCE    ( ( XI3CPSX_BASEADDR ) + 0x00000048 )
/** INTR FORCE full mask. */
#define XI3CPSX_INTR_FORCE_FULLMASK     0x0000bf7f
/** INTR FORCE full read/write mask. */
#define XI3CPSX_INTR_FORCE_FULLRWMASK  0x00000000
#define XI3CPSX_INTR_FORCE_DEFVAL   0x0	/**< INTR FORCE default value. */

/* access_type: wo */
/** INTR FORCE BUS RESET DONE FORCE E bit shift. */
#define XI3CPSX_INTR_FORCE_BUS_RESET_DONE_FORCE_E_SHIFT   15
/** INTR FORCE BUS RESET DONE FORCE E bit width. */
#define XI3CPSX_INTR_FORCE_BUS_RESET_DONE_FORCE_E_WIDTH   1
/** INTR FORCE BUS RESET DONE FORCE E bit mask. */
#define XI3CPSX_INTR_FORCE_BUS_RESET_DONE_FORCE_E_MASK    0x00008000
/** INTR FORCE BUS RESET DONE FORCE E default value. */
#define XI3CPSX_INTR_FORCE_BUS_RESET_DONE_FORCE_E_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE BUSOWNER UPDATED FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_BUSOWNER_UPDATED_FORCE_EN_SHIFT   13
/** INTR FORCE BUSOWNER UPDATED FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_BUSOWNER_UPDATED_FORCE_EN_WIDTH   1
/** INTR FORCE BUSOWNER UPDATED FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_BUSOWNER_UPDATED_FORCE_EN_MASK    0x00002000
/** INTR FORCE BUSOWNER UPDATED FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_BUSOWNER_UPDATED_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE IBI UPDATED FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_IBI_UPDATED_FORCE_EN_SHIFT   12
/** INTR FORCE IBI UPDATED FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_IBI_UPDATED_FORCE_EN_WIDTH   1
/** INTR FORCE IBI UPDATED FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_IBI_UPDATED_FORCE_EN_MASK    0x00001000
/** INTR FORCE IBI UPDATED FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_IBI_UPDATED_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE READ REQ FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_READ_REQ_FORCE_EN_SHIFT   11
/** INTR FORCE READ REQ FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_READ_REQ_FORCE_EN_WIDTH   1
/** INTR FORCE READ REQ FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_READ_REQ_FORCE_EN_MASK    0x00000800
/** INTR FORCE READ REQ FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_READ_REQ_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE DEFSLV FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_DEFSLV_FORCE_EN_SHIFT   10
/** INTR FORCE DEFSLV FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_DEFSLV_FORCE_EN_WIDTH   1
/** INTR FORCE DEFSLV FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_DEFSLV_FORCE_EN_MASK    0x00000400
/** INTR FORCE DEFSLV FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_DEFSLV_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE TRANSFER ERR FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ERR_FORCE_EN_SHIFT   9
/** INTR FORCE TRANSFER ERR FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ERR_FORCE_EN_WIDTH   1
/** INTR FORCE TRANSFER ERR FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ERR_FORCE_EN_MASK    0x00000200
/** INTR FORCE TRANSFER ERR FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ERR_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE DYN ADDR ASSGN FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_DYN_ADDR_ASSGN_FORCE_EN_SHIFT   8
/** INTR FORCE DYN ADDR ASSGN FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_DYN_ADDR_ASSGN_FORCE_EN_WIDTH   1
/** INTR FORCE DYN ADDR ASSGN FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_DYN_ADDR_ASSGN_FORCE_EN_MASK    0x00000100
/** INTR FORCE DYN ADDR ASSGN FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_DYN_ADDR_ASSGN_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE CCC UPDATED FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_CCC_UPDATED_FORCE_EN_SHIFT   6
/** INTR FORCE CCC UPDATED FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_CCC_UPDATED_FORCE_EN_WIDTH   1
/** INTR FORCE CCC UPDATED FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_CCC_UPDATED_FORCE_EN_MASK    0x00000040
/** INTR FORCE CCC UPDATED FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_CCC_UPDATED_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE TRANSFER ABORT FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ABORT_FORCE_EN_SHIFT   5
/** INTR FORCE TRANSFER ABORT FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ABORT_FORCE_EN_WIDTH   1
/** INTR FORCE TRANSFER ABORT FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ABORT_FORCE_EN_MASK    0x00000020
/** INTR FORCE TRANSFER ABORT FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_TRANSFER_ABORT_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE RESP READY FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_RESP_READY_FORCE_EN_SHIFT   4
/** INTR FORCE RESP READY FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_RESP_READY_FORCE_EN_WIDTH   1
/** INTR FORCE RESP READY FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_RESP_READY_FORCE_EN_MASK    0x00000010
/** INTR FORCE RESP READY FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_RESP_READY_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE CMD QUEUE READY FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_CMD_QUEUE_READY_FORCE_EN_SHIFT   3
/** INTR FORCE CMD QUEUE READY FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_CMD_QUEUE_READY_FORCE_EN_WIDTH   1
/** INTR FORCE CMD QUEUE READY FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_CMD_QUEUE_READY_FORCE_EN_MASK    0x00000008
/** INTR FORCE CMD QUEUE READY FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_CMD_QUEUE_READY_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE IBI THLD FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_IBI_THLD_FORCE_EN_SHIFT   2
/** INTR FORCE IBI THLD FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_IBI_THLD_FORCE_EN_WIDTH   1
/** INTR FORCE IBI THLD FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_IBI_THLD_FORCE_EN_MASK    0x00000004
/** INTR FORCE IBI THLD FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_IBI_THLD_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE RX THLD FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_RX_THLD_FORCE_EN_SHIFT   1
/** INTR FORCE RX THLD FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_RX_THLD_FORCE_EN_WIDTH   1
/** INTR FORCE RX THLD FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_RX_THLD_FORCE_EN_MASK    0x00000002
/** INTR FORCE RX THLD FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_RX_THLD_FORCE_EN_DEFVAL  0x0

/* access_type: wo */
/** INTR FORCE TX THLD FORCE EN bit shift. */
#define XI3CPSX_INTR_FORCE_TX_THLD_FORCE_EN_SHIFT   0
/** INTR FORCE TX THLD FORCE EN bit width. */
#define XI3CPSX_INTR_FORCE_TX_THLD_FORCE_EN_WIDTH   1
/** INTR FORCE TX THLD FORCE EN bit mask. */
#define XI3CPSX_INTR_FORCE_TX_THLD_FORCE_EN_MASK    0x00000001
/** INTR FORCE TX THLD FORCE EN default value. */
#define XI3CPSX_INTR_FORCE_TX_THLD_FORCE_EN_DEFVAL  0x0

/**
 * @name interrupt Register  (INTR) mask(s)
 * @{
 */
#define XI3CPSX_INTR_TX_THLD			0x00000001	/**< BIT 0 - Transmit Buffer Threshold */
#define XI3CPSX_INTR_RX_THLD			0x00000002	/**< BIT 1 - Receive Buffer Threshold */
#define XI3CPSX_INTR_IBI_THLD			0x00000004	/**< BIT 2 - IBI Buffer Threshold */
#define XI3CPSX_INTR_CMD_QUEUE_READY		0x00000008	/**< BIT 3 - Command Queue Ready */
#define XI3CPSX_INTR_RESP_READY			0x00000010	/**< BIT 4 - Response Queue Ready */
#define XI3CPSX_INTR_TRANSFER_ABORT		0x00000020	/**< BIT 5 - Transfer Abort */
#define XI3CPSX_INTR_CCC_UPDATED		0x00000040	/**< BIT 6 - CCC Table Updated */
#define XI3CPSX_INTR_DYN_ADDR_ASSGN		0x00000100	/**< BIT 8 - Dynamic Address Assigned - only in slave mode */
#define XI3CPSX_INTR_TRANSFER_ERR		0x00000200	/**< BIT 9 - Transfer Error */
#define XI3CPSX_INTR_DEFSLV			0x00000400	/**< BIT 10 - Define Slave CCC Received */
#define XI3CPSX_INTR_READ_REQ_RECV		0x00000800	/**< BIT 11 - Read Request Received */
#define XI3CPSX_INTR_IBI_UPDATED		0x00001000	/**< BIT 12 - IBI status is updated */
/** BIT 13 - Controller role changes between master and slave. */
#define XI3CPSX_INTR_BUSOWNER_UPDATED		0x00002000
#define XI3CPSX_INTR_BUS_RESET_DONE		0x00008000	/**< BIT 15 - Bus Reset Pattern Generation Done */
/** @} */

/**
 * @brief Defines the QUEUE_STATUS_LEVEL register offset.
 */
#define XI3CPSX_QUEUE_STATUS_LEVEL    ( ( XI3CPSX_BASEADDR ) + 0x0000004C )
/** QUEUE STATUS LEVEL full mask. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_FULLMASK     0x1fffffff
/** QUEUE STATUS LEVEL full read/write mask. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_FULLRWMASK  0x00000000
/** QUEUE STATUS LEVEL default value. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_DEFVAL   0x8

/* access_type: ro */
/** QUEUE STATUS LEVEL IBI STS CNT bit shift. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_STS_CNT_SHIFT   24
/** QUEUE STATUS LEVEL IBI STS CNT bit width. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_STS_CNT_WIDTH   5
/** QUEUE STATUS LEVEL IBI STS CNT bit mask. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_STS_CNT_MASK    0x1f000000
/** QUEUE STATUS LEVEL IBI STS CNT default value. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_STS_CNT_DEFVAL  0x0

/* access_type: ro */
/** QUEUE STATUS LEVEL IBI BUF BLR bit shift. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_BUF_BLR_SHIFT   16
/** QUEUE STATUS LEVEL IBI BUF BLR bit width. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_BUF_BLR_WIDTH   8
/** QUEUE STATUS LEVEL IBI BUF BLR bit mask. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_BUF_BLR_MASK    0x00ff0000
/** QUEUE STATUS LEVEL IBI BUF BLR default value. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_IBI_BUF_BLR_DEFVAL  0x0

/* access_type: ro */
/** QUEUE STATUS LEVEL RESP BUF BLR bit shift. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_SHIFT   8
/** QUEUE STATUS LEVEL RESP BUF BLR bit width. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_WIDTH   8
/** QUEUE STATUS LEVEL RESP BUF BLR bit mask. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_MASK    0x0000ff00
/** QUEUE STATUS LEVEL RESP BUF BLR default value. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_DEFVAL  0x0

/* access_type: ro */
/** QUEUE STATUS LEVEL CMD QUEUE EMPTY LOC bit shift. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_CMD_QUEUE_EMPTY_LOC_SHIFT   0
/** QUEUE STATUS LEVEL CMD QUEUE EMPTY LOC bit width. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_CMD_QUEUE_EMPTY_LOC_WIDTH   8
/** QUEUE STATUS LEVEL CMD QUEUE EMPTY LOC bit mask. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_CMD_QUEUE_EMPTY_LOC_MASK    0x000000ff
/** QUEUE STATUS LEVEL CMD QUEUE EMPTY LOC default value. */
#define XI3CPSX_QUEUE_STATUS_LEVEL_CMD_QUEUE_EMPTY_LOC_DEFVAL  0x8

/**
 * @brief Defines the DATA_BUFFER_STATUS_LEVEL register offset.
 */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL    ( ( XI3CPSX_BASEADDR ) + 0x00000050 )
/** DATA BUFFER STATUS LEVEL full mask. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_FULLMASK     0x00ff00ff
/** DATA BUFFER STATUS LEVEL full read/write mask. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_FULLRWMASK  0x00000000
/** DATA BUFFER STATUS LEVEL default value. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_DEFVAL   0x20

/* access_type: ro */
/** DATA BUFFER STATUS LEVEL RX BUF BLR bit shift. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_SHIFT   16
/** DATA BUFFER STATUS LEVEL RX BUF BLR bit width. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_WIDTH   8
/** DATA BUFFER STATUS LEVEL RX BUF BLR bit mask. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_MASK    0x00ff0000
/** DATA BUFFER STATUS LEVEL RX BUF BLR default value. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_DEFVAL  0x0

/* access_type: ro */
/** DATA BUFFER STATUS LEVEL TX BUF EMPTY LOC bit shift. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC_SHIFT   0
/** DATA BUFFER STATUS LEVEL TX BUF EMPTY LOC bit width. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC_WIDTH   8
/** DATA BUFFER STATUS LEVEL TX BUF EMPTY LOC bit mask. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC_MASK    0x000000ff
/** DATA BUFFER STATUS LEVEL TX BUF EMPTY LOC default value. */
#define XI3CPSX_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC_DEFVAL  0x20

/**
 * @brief Defines the PRESENT_STATE register offset.
 */
#define XI3CPSX_PRESENT_STATE    ( ( XI3CPSX_BASEADDR ) + 0x00000054 )
/** PRESENT STATE full mask. */
#define XI3CPSX_PRESENT_STATE_FULLMASK     0x1f3f3f07
/** PRESENT STATE full read/write mask. */
#define XI3CPSX_PRESENT_STATE_FULLRWMASK  0x00000000
/** PRESENT STATE default value. */
#define XI3CPSX_PRESENT_STATE_DEFVAL   0x10000007

/* access_type: ro */
/** PRESENT STATE MASTER IDLE bit shift. */
#define XI3CPSX_PRESENT_STATE_MASTER_IDLE_SHIFT   28
/** PRESENT STATE MASTER IDLE bit width. */
#define XI3CPSX_PRESENT_STATE_MASTER_IDLE_WIDTH   1
/** PRESENT STATE MASTER IDLE bit mask. */
#define XI3CPSX_PRESENT_STATE_MASTER_IDLE_MASK    0x10000000
/** PRESENT STATE MASTER IDLE default value. */
#define XI3CPSX_PRESENT_STATE_MASTER_IDLE_DEFVAL  0x1

/* access_type: ro */
/** PRESENT STATE CMD TID bit shift. */
#define XI3CPSX_PRESENT_STATE_CMD_TID_SHIFT   24
/** PRESENT STATE CMD TID bit width. */
#define XI3CPSX_PRESENT_STATE_CMD_TID_WIDTH   4
/** PRESENT STATE CMD TID bit mask. */
#define XI3CPSX_PRESENT_STATE_CMD_TID_MASK    0x0f000000
/** PRESENT STATE CMD TID default value. */
#define XI3CPSX_PRESENT_STATE_CMD_TID_DEFVAL  0x0

/* access_type: ro */
/** PRESENT STATE CM TFR ST STS bit shift. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_ST_STS_SHIFT   16
/** PRESENT STATE CM TFR ST STS bit width. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_ST_STS_WIDTH   6
/** PRESENT STATE CM TFR ST STS bit mask. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_ST_STS_MASK    0x003f0000
/** PRESENT STATE CM TFR ST STS default value. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_ST_STS_DEFVAL  0x0

/* access_type: ro */
/** PRESENT STATE CM TFR STS bit shift. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_STS_SHIFT   8
/** PRESENT STATE CM TFR STS bit width. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_STS_WIDTH   6
/** PRESENT STATE CM TFR STS bit mask. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_STS_MASK    0x00003f00
/** PRESENT STATE CM TFR STS default value. */
#define XI3CPSX_PRESENT_STATE_CM_TFR_STS_DEFVAL  0x0

/* access_type: ro */
/** PRESENT STATE CURRENT MASTER bit shift. */
#define XI3CPSX_PRESENT_STATE_CURRENT_MASTER_SHIFT   2
/** PRESENT STATE CURRENT MASTER bit width. */
#define XI3CPSX_PRESENT_STATE_CURRENT_MASTER_WIDTH   1
/** PRESENT STATE CURRENT MASTER bit mask. */
#define XI3CPSX_PRESENT_STATE_CURRENT_MASTER_MASK    0x00000004
/** PRESENT STATE CURRENT MASTER default value. */
#define XI3CPSX_PRESENT_STATE_CURRENT_MASTER_DEFVAL  0x1

/* access_type: ro */
/** PRESENT STATE SDA LINE SIGNAL LEVEL bit shift. */
#define XI3CPSX_PRESENT_STATE_SDA_LINE_SIGNAL_LEVEL_SHIFT   1
/** PRESENT STATE SDA LINE SIGNAL LEVEL bit width. */
#define XI3CPSX_PRESENT_STATE_SDA_LINE_SIGNAL_LEVEL_WIDTH   1
/** PRESENT STATE SDA LINE SIGNAL LEVEL bit mask. */
#define XI3CPSX_PRESENT_STATE_SDA_LINE_SIGNAL_LEVEL_MASK    0x00000002
/** PRESENT STATE SDA LINE SIGNAL LEVEL default value. */
#define XI3CPSX_PRESENT_STATE_SDA_LINE_SIGNAL_LEVEL_DEFVAL  0x1

/* access_type: ro */
/** PRESENT STATE SCL LINE SIGNAL LEVEL bit shift. */
#define XI3CPSX_PRESENT_STATE_SCL_LINE_SIGNAL_LEVEL_SHIFT   0
/** PRESENT STATE SCL LINE SIGNAL LEVEL bit width. */
#define XI3CPSX_PRESENT_STATE_SCL_LINE_SIGNAL_LEVEL_WIDTH   1
/** PRESENT STATE SCL LINE SIGNAL LEVEL bit mask. */
#define XI3CPSX_PRESENT_STATE_SCL_LINE_SIGNAL_LEVEL_MASK    0x00000001
/** PRESENT STATE SCL LINE SIGNAL LEVEL default value. */
#define XI3CPSX_PRESENT_STATE_SCL_LINE_SIGNAL_LEVEL_DEFVAL  0x1

/**
 * @brief Defines the CCC_DEVICE_STATUS register offset.
 */
#define XI3CPSX_CCC_DEVICE_STATUS    ( ( XI3CPSX_BASEADDR ) + 0x00000058 )
/** CCC DEVICE STATUS full mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_FULLMASK     0x00003fef
/** CCC DEVICE STATUS full read/write mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_FULLRWMASK  0x00000000
/** CCC DEVICE STATUS default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_DEFVAL   0x0

/* access_type: ro */
/** CCC DEVICE STATUS FRAME ERROR bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_FRAME_ERROR_SHIFT   13
/** CCC DEVICE STATUS FRAME ERROR bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_FRAME_ERROR_WIDTH   1
/** CCC DEVICE STATUS FRAME ERROR bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_FRAME_ERROR_MASK    0x00002000
/** CCC DEVICE STATUS FRAME ERROR default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_FRAME_ERROR_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS BUFFER NOT AVAIL bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_BUFFER_NOT_AVAIL_SHIFT   12
/** CCC DEVICE STATUS BUFFER NOT AVAIL bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_BUFFER_NOT_AVAIL_WIDTH   1
/** CCC DEVICE STATUS BUFFER NOT AVAIL bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_BUFFER_NOT_AVAIL_MASK    0x00001000
/** CCC DEVICE STATUS BUFFER NOT AVAIL default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_BUFFER_NOT_AVAIL_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS DATA NOT READY bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_DATA_NOT_READY_SHIFT   11
/** CCC DEVICE STATUS DATA NOT READY bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_DATA_NOT_READY_WIDTH   1
/** CCC DEVICE STATUS DATA NOT READY bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_DATA_NOT_READY_MASK    0x00000800
/** CCC DEVICE STATUS DATA NOT READY default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_DATA_NOT_READY_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS OVERFLOW ERR bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_OVERFLOW_ERR_SHIFT   10
/** CCC DEVICE STATUS OVERFLOW ERR bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_OVERFLOW_ERR_WIDTH   1
/** CCC DEVICE STATUS OVERFLOW ERR bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_OVERFLOW_ERR_MASK    0x00000400
/** CCC DEVICE STATUS OVERFLOW ERR default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_OVERFLOW_ERR_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS SLAVE BUSY bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_SLAVE_BUSY_SHIFT   9
/** CCC DEVICE STATUS SLAVE BUSY bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_SLAVE_BUSY_WIDTH   1
/** CCC DEVICE STATUS SLAVE BUSY bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_SLAVE_BUSY_MASK    0x00000200
/** CCC DEVICE STATUS SLAVE BUSY default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_SLAVE_BUSY_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS UNDERFLOW ERR bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_UNDERFLOW_ERR_SHIFT   8
/** CCC DEVICE STATUS UNDERFLOW ERR bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_UNDERFLOW_ERR_WIDTH   1
/** CCC DEVICE STATUS UNDERFLOW ERR bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_UNDERFLOW_ERR_MASK    0x00000100
/** CCC DEVICE STATUS UNDERFLOW ERR default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_UNDERFLOW_ERR_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS ACTIVITY MODE bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_ACTIVITY_MODE_SHIFT   6
/** CCC DEVICE STATUS ACTIVITY MODE bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_ACTIVITY_MODE_WIDTH   2
/** CCC DEVICE STATUS ACTIVITY MODE bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_ACTIVITY_MODE_MASK    0x000000c0
/** CCC DEVICE STATUS ACTIVITY MODE default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_ACTIVITY_MODE_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS PROTOCOL ERR bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_PROTOCOL_ERR_SHIFT   5
/** CCC DEVICE STATUS PROTOCOL ERR bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_PROTOCOL_ERR_WIDTH   1
/** CCC DEVICE STATUS PROTOCOL ERR bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_PROTOCOL_ERR_MASK    0x00000020
/** CCC DEVICE STATUS PROTOCOL ERR default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_PROTOCOL_ERR_DEFVAL  0x0

/* access_type: ro */
/** CCC DEVICE STATUS PENDING INTR bit shift. */
#define XI3CPSX_CCC_DEVICE_STATUS_PENDING_INTR_SHIFT   0
/** CCC DEVICE STATUS PENDING INTR bit width. */
#define XI3CPSX_CCC_DEVICE_STATUS_PENDING_INTR_WIDTH   4
/** CCC DEVICE STATUS PENDING INTR bit mask. */
#define XI3CPSX_CCC_DEVICE_STATUS_PENDING_INTR_MASK    0x0000000f
/** CCC DEVICE STATUS PENDING INTR default value. */
#define XI3CPSX_CCC_DEVICE_STATUS_PENDING_INTR_DEFVAL  0x0

/**
 * @brief Defines the DEVICE_ADDR_TABLE_POINTER register offset.
 */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER    ( ( XI3CPSX_BASEADDR ) + 0x0000005C )
/** DEVICE ADDR TABLE POINTER full mask. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_FULLMASK     0xffffffff
/** DEVICE ADDR TABLE POINTER full read/write mask. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_FULLRWMASK  0x00000000
/** DEVICE ADDR TABLE POINTER default value. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEFVAL   0xb02c0

/* access_type: ro */
/** DEVICE ADDR TABLE POINTER DEV ADDR TABLE DEPTH bit shift. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_SHIFT   16
/** DEVICE ADDR TABLE POINTER DEV ADDR TABLE DEPTH bit width. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_WIDTH   16
/** DEVICE ADDR TABLE POINTER DEV ADDR TABLE DEPTH bit mask. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_MASK    0xffff0000
/** DEVICE ADDR TABLE POINTER DEV ADDR TABLE DEPTH default value. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_DEFVAL  0xb

/* access_type: ro */
/** DEVICE ADDR TABLE POINTER P DEV ADDR TABLE START ADDR bit shift. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_SHIFT   0
/** DEVICE ADDR TABLE POINTER P DEV ADDR TABLE START ADDR bit width. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_WIDTH   16
/** DEVICE ADDR TABLE POINTER P DEV ADDR TABLE START ADDR bit mask. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_MASK    0x0000ffff
/** DEVICE ADDR TABLE POINTER P DEV ADDR TABLE START ADDR default. */
#define XI3CPSX_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_DEFVAL  0x2c0

/**
 * @brief Defines the DEV_CHAR_TABLE_POINTER register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER    ( ( XI3CPSX_BASEADDR ) + 0x00000060 )
/** DEV CHAR TABLE POINTER full mask. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_FULLMASK     0x007fffff
/** DEV CHAR TABLE POINTER full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_FULLRWMASK  0x00780000
/** DEV CHAR TABLE POINTER default value. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_DEFVAL   0x2c200

/* access_type: rw */
/** DEV CHAR TABLE POINTER PRESENT DEV CHAR TABLE INDX bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_SHIFT   19
/** DEV CHAR TABLE POINTER PRESENT DEV CHAR TABLE INDX bit width. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_WIDTH   4
/** DEV CHAR TABLE POINTER PRESENT DEV CHAR TABLE INDX bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_MASK    0x00780000
/** DEV CHAR TABLE POINTER PRESENT DEV CHAR TABLE INDX default. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE POINTER DEV CHAR TABLE DEPTH bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_DEV_CHAR_TABLE_DEPTH_SHIFT   12
/** DEV CHAR TABLE POINTER DEV CHAR TABLE DEPTH bit width. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_DEV_CHAR_TABLE_DEPTH_WIDTH   7
/** DEV CHAR TABLE POINTER DEV CHAR TABLE DEPTH bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_DEV_CHAR_TABLE_DEPTH_MASK    0x0007f000
/** DEV CHAR TABLE POINTER DEV CHAR TABLE DEPTH default value. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_DEV_CHAR_TABLE_DEPTH_DEFVAL  0x2c

/* access_type: ro */
/** DEV CHAR TABLE POINTER P DEV CHAR TABLE START ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_P_DEV_CHAR_TABLE_START_ADDR_SHIFT   0
/** DEV CHAR TABLE POINTER P DEV CHAR TABLE START ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_P_DEV_CHAR_TABLE_START_ADDR_WIDTH   12
/** DEV CHAR TABLE POINTER P DEV CHAR TABLE START ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_P_DEV_CHAR_TABLE_START_ADDR_MASK    0x00000fff
/** DEV CHAR TABLE POINTER P DEV CHAR TABLE START ADDR default. */
#define XI3CPSX_DEV_CHAR_TABLE_POINTER_P_DEV_CHAR_TABLE_START_ADDR_DEFVAL  0x200

/**
 * @brief Defines the VENDOR_SPECIFIC_REG_POINTER register offset.
 */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER    ( ( XI3CPSX_BASEADDR ) + 0x0000006C )
/** VENDOR SPECIFIC REG POINTER full mask. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_FULLMASK     0x0000ffff
/** VENDOR SPECIFIC REG POINTER full read/write mask. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_FULLRWMASK  0x00000000
/** VENDOR SPECIFIC REG POINTER default value. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_DEFVAL   0xb0

/* access_type: ro */
/** VENDOR SPECIFIC REG POINTER P VENDOR REG START ADDR bit shift. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_P_VENDOR_REG_START_ADDR_SHIFT   0
/** VENDOR SPECIFIC REG POINTER P VENDOR REG START ADDR bit width. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_P_VENDOR_REG_START_ADDR_WIDTH   16
/** VENDOR SPECIFIC REG POINTER P VENDOR REG START ADDR bit mask. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_P_VENDOR_REG_START_ADDR_MASK    0x0000ffff
/** VENDOR SPECIFIC REG POINTER P VENDOR REG START ADDR default. */
#define XI3CPSX_VENDOR_SPECIFIC_REG_POINTER_P_VENDOR_REG_START_ADDR_DEFVAL  0xb0

/**
 * @brief Defines the SLV_MIPI_ID_VALUE register offset.
 */
#define XI3CPSX_SLV_MIPI_ID_VALUE    ( ( XI3CPSX_BASEADDR ) + 0x00000070 )
/** SLV MIPI ID VALUE full mask. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_FULLMASK     0x0000ffff
/** SLV MIPI ID VALUE full read/write mask. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_FULLRWMASK  0x0000ffff
/** SLV MIPI ID VALUE default value. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_DEFVAL   0x0

/* access_type: rw */
/** SLV MIPI ID VALUE SLV MIPI MFG ID bit shift. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_SHIFT   1
/** SLV MIPI ID VALUE SLV MIPI MFG ID bit width. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_WIDTH   15
/** SLV MIPI ID VALUE SLV MIPI MFG ID bit mask. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_MASK    0x0000fffe
/** SLV MIPI ID VALUE SLV MIPI MFG ID default value. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_DEFVAL  0x0

/* access_type: rw */
/** SLV MIPI ID VALUE SLV PROV ID SEL bit shift. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL_SHIFT     0
/** SLV MIPI ID VALUE SLV PROV ID SEL bit width. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL_WIDTH     1
/** SLV MIPI ID VALUE SLV PROV ID SEL bit mask. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL_MASK      0x00000001
/** SLV MIPI ID VALUE SLV PROV ID SEL default value. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL_DEFVAL    0x0
/** SLV MIPI ID VALUE SLV PROV ID SEL VAL register field. */
#define XI3CPSX_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL_VAL       0x2

/**
 * @brief Defines the SLV_PID_VALUE register offset.
 */
#define XI3CPSX_SLV_PID_VALUE    ( ( XI3CPSX_BASEADDR ) + 0x00000074 )
/** SLV PID VALUE full mask. */
#define XI3CPSX_SLV_PID_VALUE_FULLMASK     0xffffffff
/** SLV PID VALUE full read/write mask. */
#define XI3CPSX_SLV_PID_VALUE_FULLRWMASK  0xffffffff
/** SLV PID VALUE default value. */
#define XI3CPSX_SLV_PID_VALUE_DEFVAL   0x0

/* access_type: rw */
/** SLV PID VALUE SLV PART ID bit shift. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PART_ID_SHIFT   16
/** SLV PID VALUE SLV PART ID bit width. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PART_ID_WIDTH   16
/** SLV PID VALUE SLV PART ID bit mask. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PART_ID_MASK    0xffff0000
/** SLV PID VALUE SLV PART ID default value. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PART_ID_DEFVAL  0x0

/* access_type: rw */
/** SLV PID VALUE SLV INST ID bit shift. */
#define XI3CPSX_SLV_PID_VALUE_SLV_INST_ID_SHIFT   12
/** SLV PID VALUE SLV INST ID bit width. */
#define XI3CPSX_SLV_PID_VALUE_SLV_INST_ID_WIDTH   4
/** SLV PID VALUE SLV INST ID bit mask. */
#define XI3CPSX_SLV_PID_VALUE_SLV_INST_ID_MASK    0x0000f000
/** SLV PID VALUE SLV INST ID default value. */
#define XI3CPSX_SLV_PID_VALUE_SLV_INST_ID_DEFVAL  0x0

/* access_type: rw */
/** SLV PID VALUE SLV PID DCR bit shift. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PID_DCR_SHIFT   0
/** SLV PID VALUE SLV PID DCR bit width. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PID_DCR_WIDTH   12
/** SLV PID VALUE SLV PID DCR bit mask. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PID_DCR_MASK    0x00000fff
/** SLV PID VALUE SLV PID DCR default value. */
#define XI3CPSX_SLV_PID_VALUE_SLV_PID_DCR_DEFVAL  0x0

/**
 * @brief Defines the SLV_CHAR_CTRL register offset.
 */
#define XI3CPSX_SLV_CHAR_CTRL    ( ( XI3CPSX_BASEADDR ) + 0x00000078 )
/** SLV CHAR CTRL full mask. */
#define XI3CPSX_SLV_CHAR_CTRL_FULLMASK     0x00ffffff
/** SLV CHAR CTRL full read/write mask. */
#define XI3CPSX_SLV_CHAR_CTRL_FULLRWMASK  0x0000ffe1
/** SLV CHAR CTRL default value. */
#define XI3CPSX_SLV_CHAR_CTRL_DEFVAL   0x10062

/* access_type: ro */
/** SLV CHAR CTRL HDR CAP bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAP_SHIFT   16
/** SLV CHAR CTRL HDR CAP bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAP_WIDTH   8
/** SLV CHAR CTRL HDR CAP bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAP_MASK    0x00ff0000
/** SLV CHAR CTRL HDR CAP default value. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAP_DEFVAL  0x1

/* access_type: rw */
/** SLV CHAR CTRL DCR bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_DCR_SHIFT        8
/** SLV CHAR CTRL DCR bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_DCR_WIDTH        8
/** SLV CHAR CTRL DCR bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_DCR_MASK         0x0000ff00
/** SLV CHAR CTRL DCR default value. */
#define XI3CPSX_SLV_CHAR_CTRL_DCR_DEFVAL       0x0
/** SLV CHAR CTRL DCR VAL register field. */
#define XI3CPSX_SLV_CHAR_CTRL_DCR_VAL          0xC4

/* access_type: rw */
/** SLV CHAR CTRL DEVICE ROLE bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_DEVICE_ROLE_SHIFT   6
/** SLV CHAR CTRL DEVICE ROLE bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_DEVICE_ROLE_WIDTH   2
/** SLV CHAR CTRL DEVICE ROLE bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_DEVICE_ROLE_MASK    0x000000c0
/** SLV CHAR CTRL DEVICE ROLE default value. */
#define XI3CPSX_SLV_CHAR_CTRL_DEVICE_ROLE_DEFVAL  0x1
/** SLV CHAR CTRL DEVICE ROLE SLAVE register field. */
#define XI3CPSX_SLV_CHAR_CTRL_DEVICE_ROLE_SLAVE  0x2

/* access_type: rw */
/** SLV CHAR CTRL HDR CAPABLE bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAPABLE_SHIFT   5
/** SLV CHAR CTRL HDR CAPABLE bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAPABLE_WIDTH   1
/** SLV CHAR CTRL HDR CAPABLE bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAPABLE_MASK    0x00000020
/** SLV CHAR CTRL HDR CAPABLE default value. */
#define XI3CPSX_SLV_CHAR_CTRL_HDR_CAPABLE_DEFVAL  0x1

/* access_type: ro */
/** SLV CHAR CTRL BRIDGE IDENTIFIER bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_BRIDGE_IDENTIFIER_SHIFT   4
/** SLV CHAR CTRL BRIDGE IDENTIFIER bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_BRIDGE_IDENTIFIER_WIDTH   1
/** SLV CHAR CTRL BRIDGE IDENTIFIER bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_BRIDGE_IDENTIFIER_MASK    0x00000010
/** SLV CHAR CTRL BRIDGE IDENTIFIER default value. */
#define XI3CPSX_SLV_CHAR_CTRL_BRIDGE_IDENTIFIER_DEFVAL  0x0

/* access_type: ro */
/** SLV CHAR CTRL OFFLINE CAPABLE bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_OFFLINE_CAPABLE_SHIFT   3
/** SLV CHAR CTRL OFFLINE CAPABLE bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_OFFLINE_CAPABLE_WIDTH   1
/** SLV CHAR CTRL OFFLINE CAPABLE bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_OFFLINE_CAPABLE_MASK    0x00000008
/** SLV CHAR CTRL OFFLINE CAPABLE default value. */
#define XI3CPSX_SLV_CHAR_CTRL_OFFLINE_CAPABLE_DEFVAL  0x0

/* access_type: ro */
/** SLV CHAR CTRL IBI PAYLOAD bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_PAYLOAD_SHIFT   2
/** SLV CHAR CTRL IBI PAYLOAD bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_PAYLOAD_WIDTH   1
/** SLV CHAR CTRL IBI PAYLOAD bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_PAYLOAD_MASK    0x00000004
/** SLV CHAR CTRL IBI PAYLOAD default value. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_PAYLOAD_DEFVAL  0x0

/* access_type: ro */
/** SLV CHAR CTRL IBI REQUEST CAPABLE bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_REQUEST_CAPABLE_SHIFT   1
/** SLV CHAR CTRL IBI REQUEST CAPABLE bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_REQUEST_CAPABLE_WIDTH   1
/** SLV CHAR CTRL IBI REQUEST CAPABLE bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_REQUEST_CAPABLE_MASK    0x00000002
/** SLV CHAR CTRL IBI REQUEST CAPABLE default value. */
#define XI3CPSX_SLV_CHAR_CTRL_IBI_REQUEST_CAPABLE_DEFVAL  0x1

/* access_type: rw */
/** SLV CHAR CTRL MAX DATA SPEED LIMIT bit shift. */
#define XI3CPSX_SLV_CHAR_CTRL_MAX_DATA_SPEED_LIMIT_SHIFT   0
/** SLV CHAR CTRL MAX DATA SPEED LIMIT bit width. */
#define XI3CPSX_SLV_CHAR_CTRL_MAX_DATA_SPEED_LIMIT_WIDTH   1
/** SLV CHAR CTRL MAX DATA SPEED LIMIT bit mask. */
#define XI3CPSX_SLV_CHAR_CTRL_MAX_DATA_SPEED_LIMIT_MASK    0x00000001
/** SLV CHAR CTRL MAX DATA SPEED LIMIT default value. */
#define XI3CPSX_SLV_CHAR_CTRL_MAX_DATA_SPEED_LIMIT_DEFVAL  0x0

/**
 * @brief Defines the SLV_MAX_LEN register offset.
 */
#define XI3CPSX_SLV_MAX_LEN    ( ( XI3CPSX_BASEADDR ) + 0x0000007C )
/** SLV MAX LEN full mask. */
#define XI3CPSX_SLV_MAX_LEN_FULLMASK     0xffffffff
/** SLV MAX LEN full read/write mask. */
#define XI3CPSX_SLV_MAX_LEN_FULLRWMASK  0x00000000
/** SLV MAX LEN default value. */
#define XI3CPSX_SLV_MAX_LEN_DEFVAL   0xff00ff

/* access_type: ro */
/** SLV MAX LEN MRL bit shift. */
#define XI3CPSX_SLV_MAX_LEN_MRL_SHIFT   16
/** SLV MAX LEN MRL bit width. */
#define XI3CPSX_SLV_MAX_LEN_MRL_WIDTH   16
/** SLV MAX LEN MRL bit mask. */
#define XI3CPSX_SLV_MAX_LEN_MRL_MASK    0xffff0000
/** SLV MAX LEN MRL default value. */
#define XI3CPSX_SLV_MAX_LEN_MRL_DEFVAL  0xff

/* access_type: ro */
/** SLV MAX LEN MWL bit shift. */
#define XI3CPSX_SLV_MAX_LEN_MWL_SHIFT   0
/** SLV MAX LEN MWL bit width. */
#define XI3CPSX_SLV_MAX_LEN_MWL_WIDTH   16
/** SLV MAX LEN MWL bit mask. */
#define XI3CPSX_SLV_MAX_LEN_MWL_MASK    0x0000ffff
/** SLV MAX LEN MWL default value. */
#define XI3CPSX_SLV_MAX_LEN_MWL_DEFVAL  0xff

/**
 * @brief Defines the MAX_READ_TURNAROUND register offset.
 */
#define XI3CPSX_MAX_READ_TURNAROUND    ( ( XI3CPSX_BASEADDR ) + 0x00000080 )
/** MAX READ TURNAROUND full mask. */
#define XI3CPSX_MAX_READ_TURNAROUND_FULLMASK     0x00ffffff
/** MAX READ TURNAROUND full read/write mask. */
#define XI3CPSX_MAX_READ_TURNAROUND_FULLRWMASK  0x00000000
/** MAX READ TURNAROUND default value. */
#define XI3CPSX_MAX_READ_TURNAROUND_DEFVAL   0x0

/* access_type: ro */
/** MAX READ TURNAROUND MXDS MAX RD TURN bit shift. */
#define XI3CPSX_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN_SHIFT   0
/** MAX READ TURNAROUND MXDS MAX RD TURN bit width. */
#define XI3CPSX_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN_WIDTH   24
/** MAX READ TURNAROUND MXDS MAX RD TURN bit mask. */
#define XI3CPSX_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN_MASK    0x00ffffff
/** MAX READ TURNAROUND MXDS MAX RD TURN default value. */
#define XI3CPSX_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN_DEFVAL  0x0

/**
 * @brief Defines the MAX_DATA_SPEED register offset.
 */
#define XI3CPSX_MAX_DATA_SPEED    ( ( XI3CPSX_BASEADDR ) + 0x00000084 )
/** MAX DATA SPEED full mask. */
#define XI3CPSX_MAX_DATA_SPEED_FULLMASK     0x00070707
/** MAX DATA SPEED full read/write mask. */
#define XI3CPSX_MAX_DATA_SPEED_FULLRWMASK  0x00070707
/** MAX DATA SPEED default value. */
#define XI3CPSX_MAX_DATA_SPEED_DEFVAL   0x0

/* access_type: rw */
/** MAX DATA SPEED MXDS CLK DATA TURN bit shift. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_CLK_DATA_TURN_SHIFT   16
/** MAX DATA SPEED MXDS CLK DATA TURN bit width. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_CLK_DATA_TURN_WIDTH   3
/** MAX DATA SPEED MXDS CLK DATA TURN bit mask. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_CLK_DATA_TURN_MASK    0x00070000
/** MAX DATA SPEED MXDS CLK DATA TURN default value. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_CLK_DATA_TURN_DEFVAL  0x0

/* access_type: rw */
/** MAX DATA SPEED MXDS MAX RD SPEED bit shift. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_SHIFT   8
/** MAX DATA SPEED MXDS MAX RD SPEED bit width. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_WIDTH   3
/** MAX DATA SPEED MXDS MAX RD SPEED bit mask. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_MASK    0x00000700
/** MAX DATA SPEED MXDS MAX RD SPEED default value. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_DEFVAL  0x0

/* access_type: rw */
/** MAX DATA SPEED MXDS MAX WR SPEED bit shift. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED_SHIFT   0
/** MAX DATA SPEED MXDS MAX WR SPEED bit width. */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED_WIDTH   3
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED_MASK    0x00000007	/**< Mask for the max write speed field */
#define XI3CPSX_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED_DEFVAL  0x0	/**< Reset value of the max write speed field */

/**
 * @brief Defines the SLV_INTR_REQ register offset.
 */
#define XI3CPSX_SLV_INTR_REQ    ( ( XI3CPSX_BASEADDR ) + 0x0000008C )
#define XI3CPSX_SLV_INTR_REQ_FULLMASK     0x00ffff0f	/**< Mask of all valid bits in SLV_INTR_REQ */
#define XI3CPSX_SLV_INTR_REQ_FULLRWMASK  0x00ffff0f	/**< Mask of all read/write bits in SLV_INTR_REQ */
#define XI3CPSX_SLV_INTR_REQ_DEFVAL   0x0	/**< Reset value of SLV_INTR_REQ */

/* access_type: rw */
#define XI3CPSX_SLV_INTR_REQ_SIR_DATA_LENGTH_SHIFT   16	/**< Bit shift for the SIR data length field */
#define XI3CPSX_SLV_INTR_REQ_SIR_DATA_LENGTH_WIDTH   8	/**< Bit width of the SIR data length field */
#define XI3CPSX_SLV_INTR_REQ_SIR_DATA_LENGTH_MASK    0x00ff0000	/**< Mask for the SIR data length field */
#define XI3CPSX_SLV_INTR_REQ_SIR_DATA_LENGTH_DEFVAL  0x0	/**< Reset value of the SIR data length field */

/* access_type: rw */
#define XI3CPSX_SLV_INTR_REQ_MDB_SHIFT   8	/**< Bit shift for the Mandatory Data Byte (MDB) field */
#define XI3CPSX_SLV_INTR_REQ_MDB_WIDTH   8	/**< Bit width of the Mandatory Data Byte (MDB) field */
#define XI3CPSX_SLV_INTR_REQ_MDB_MASK    0x0000ff00	/**< Mask for the Mandatory Data Byte (MDB) field */
#define XI3CPSX_SLV_INTR_REQ_MDB_DEFVAL  0x0	/**< Reset value of the Mandatory Data Byte (MDB) field */

/*
 * IBI response status is read from XI3CPSX_SLV_IBI_RESP, not from this
 * register; SLV_INTR_REQ[9:8] is not used for that purpose by this driver.
 */

/* access_type: rw */
/** SLV INTR REQ MR bit shift. */
#define XI3CPSX_SLV_INTR_REQ_MR_SHIFT   3
/** SLV INTR REQ MR bit width. */
#define XI3CPSX_SLV_INTR_REQ_MR_WIDTH   1
/** SLV INTR REQ MR bit mask. */
#define XI3CPSX_SLV_INTR_REQ_MR_MASK    0x00000008
/** SLV INTR REQ MR default value. */
#define XI3CPSX_SLV_INTR_REQ_MR_DEFVAL  0x0

/* access_type: rw */
#define XI3CPSX_SLV_INTR_REQ_SIR_CTRL_SHIFT   1	/**< Bit shift for the SIR control field */
#define XI3CPSX_SLV_INTR_REQ_SIR_CTRL_WIDTH   2	/**< Bit width of the SIR control field */
#define XI3CPSX_SLV_INTR_REQ_SIR_CTRL_MASK    0x00000006	/**< Mask for the SIR control field */
#define XI3CPSX_SLV_INTR_REQ_SIR_CTRL_DEFVAL  0x0	/**< Reset value of the SIR control field */

/* access_type: rw */
#define XI3CPSX_SLV_INTR_REQ_SIR_SHIFT   0	/**< Bit shift for the SIR request field */
#define XI3CPSX_SLV_INTR_REQ_SIR_WIDTH   1	/**< Bit width of the SIR request field */
#define XI3CPSX_SLV_INTR_REQ_SIR_MASK    0x00000001	/**< Mask for the SIR request field */
#define XI3CPSX_SLV_INTR_REQ_SIR_DEFVAL  0x0	/**< Reset value of the SIR request field */

/**
 * @brief Defines the SLV_SIR_DATA register offset.
 */
#define XI3CPSX_SLV_SIR_DATA    ( ( XI3CPSX_BASEADDR ) + 0x00000094 )
#define XI3CPSX_SLV_SIR_DATA_FULLMASK     0xffffffff	/**< Mask of all valid bits in SLV_SIR_DATA */
#define XI3CPSX_SLV_SIR_DATA_FULLRWMASK  0xffffffff	/**< Mask of all read/write bits in SLV_SIR_DATA */
#define XI3CPSX_SLV_SIR_DATA_DEFVAL   0x0	/**< Reset value of SLV_SIR_DATA */

#define XI3CPSX_SLV_SIR_DATA_BYTE3_SHIFT   24	/**< Bit shift for IBI payload data byte 3 */
#define XI3CPSX_SLV_SIR_DATA_BYTE3_MASK    0xff000000	/**< Mask for IBI payload data byte 3 */
#define XI3CPSX_SLV_SIR_DATA_BYTE2_SHIFT   16	/**< Bit shift for IBI payload data byte 2 */
#define XI3CPSX_SLV_SIR_DATA_BYTE2_MASK    0x00ff0000	/**< Mask for IBI payload data byte 2 */
#define XI3CPSX_SLV_SIR_DATA_BYTE1_SHIFT   8	/**< Bit shift for IBI payload data byte 1 */
#define XI3CPSX_SLV_SIR_DATA_BYTE1_MASK    0x0000ff00	/**< Mask for IBI payload data byte 1 */
#define XI3CPSX_SLV_SIR_DATA_BYTE0_SHIFT   0	/**< Bit shift for IBI payload data byte 0 */
#define XI3CPSX_SLV_SIR_DATA_BYTE0_MASK    0x000000ff	/**< Mask for IBI payload data byte 0 */

/**
 * @brief Defines the SLV_IBI_RESP register offset.
 */
#define XI3CPSX_SLV_IBI_RESP    ( ( XI3CPSX_BASEADDR ) + 0x00000098 )
#define XI3CPSX_SLV_IBI_RESP_FULLMASK     0x00ffffff	/**< Mask of all valid bits in SLV_IBI_RESP */
#define XI3CPSX_SLV_IBI_RESP_FULLRWMASK  0x00000000	/**< Mask of all read/write bits in SLV_IBI_RESP (read-only register) */
#define XI3CPSX_SLV_IBI_RESP_DEFVAL   0x0	/**< Reset value of SLV_IBI_RESP */

#define XI3CPSX_SLV_IBI_RESP_DATA_LENGTH_SHIFT   8	/**< Bit shift for the accepted IBI data length field */
#define XI3CPSX_SLV_IBI_RESP_DATA_LENGTH_MASK    0x00ffff00	/**< Mask for the accepted IBI data length field */

#define XI3CPSX_SLV_IBI_RESP_IBI_STS_SHIFT   0	/**< Bit shift for the IBI response status field */
#define XI3CPSX_SLV_IBI_RESP_IBI_STS_MASK    0x00000003	/**< Mask for the IBI response status field */
#define XI3CPSX_SLV_IBI_RESP_IBI_STS_ACK             0x1	/**< IBI_STS: controller ACK'd the IBI */
#define XI3CPSX_SLV_IBI_RESP_IBI_STS_EARLY_TERMINATE 0x2	/**< IBI_STS: controller accepted only part of the IBI payload */
#define XI3CPSX_SLV_IBI_RESP_IBI_STS_NACK            0x3	/**< IBI_STS: controller NACK'd the IBI */

/**
 * @brief Defines the DEVICE_CTRL_EXTENDED register offset.
 */
#define XI3CPSX_DEVICE_CTRL_EXTENDED    ( ( XI3CPSX_BASEADDR ) + 0x000000B0 )
/** DEVICE CTRL EXTENDED full mask. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_FULLMASK     0x0000000b
/** DEVICE CTRL EXTENDED full read/write mask. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_FULLRWMASK  0x0000000b
/** DEVICE CTRL EXTENDED default value. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEFVAL   0x0

/* access_type: rw */
/** DEVICE CTRL EXTENDED REQMST ACK CTRL bit shift. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_REQMST_ACK_CTRL_SHIFT   3
/** DEVICE CTRL EXTENDED REQMST ACK CTRL bit width. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_REQMST_ACK_CTRL_WIDTH   1
/** DEVICE CTRL EXTENDED REQMST ACK CTRL bit mask. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_REQMST_ACK_CTRL_MASK    0x00000008
/** DEVICE CTRL EXTENDED REQMST ACK CTRL default value. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_REQMST_ACK_CTRL_DEFVAL  0x0

/* access_type: rw */
/** DEVICE CTRL EXTENDED DEV OPERATION MODE bit shift. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_SHIFT    0
/** DEVICE CTRL EXTENDED DEV OPERATION MODE MASTER register field. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_MASTER   0x0
/** DEVICE CTRL EXTENDED DEV OPERATION MODE SLAVE register field. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_SLAVE    0x1
/** DEVICE CTRL EXTENDED DEV OPERATION MODE bit width. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_WIDTH    2
/** DEVICE CTRL EXTENDED DEV OPERATION MODE bit mask. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_MASK     0x00000003
/** DEVICE CTRL EXTENDED DEV OPERATION MODE default value. */
#define XI3CPSX_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_DEFVAL   0x0

/**
 * @brief Defines the SCL_I3C_OD_TIMING register offset.
 */
#define XI3CPSX_SCL_I3C_OD_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000B4 )
/** SCL I3C OD TIMING full mask. */
#define XI3CPSX_SCL_I3C_OD_TIMING_FULLMASK     0x00ff00ff
/** SCL I3C OD TIMING full read/write mask. */
#define XI3CPSX_SCL_I3C_OD_TIMING_FULLRWMASK  0x00ff00ff
/** SCL I3C OD TIMING default value. */
#define XI3CPSX_SCL_I3C_OD_TIMING_DEFVAL   0xa0010

/* access_type: rw */
/** SCL I3C OD TIMING I3C OD HCNT bit shift. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_SHIFT   16
/** SCL I3C OD TIMING I3C OD HCNT bit width. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_WIDTH   8
/** SCL I3C OD TIMING I3C OD HCNT bit mask. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_MASK    0x00ff0000
/** SCL I3C OD TIMING I3C OD HCNT default value. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_DEFVAL  0xa
/** SCL I3C OD TIMING I3C OD HCNT VAL register field. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_HCNT_VAL     8

/* access_type: rw */
/** SCL I3C OD TIMING I3C OD LCNT bit shift. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_SHIFT   0
/** SCL I3C OD TIMING I3C OD LCNT bit width. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_WIDTH   8
/** SCL I3C OD TIMING I3C OD LCNT bit mask. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_MASK    0x000000ff
/** SCL I3C OD TIMING I3C OD LCNT default value. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_DEFVAL  0x10
/** SCL I3C OD TIMING I3C OD LCNT VAL register field. */
#define XI3CPSX_SCL_I3C_OD_TIMING_I3C_OD_LCNT_VAL     50

/**
 * @brief Defines the SCL_I3C_PP_TIMING register offset.
 */
#define XI3CPSX_SCL_I3C_PP_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000B8 )
/** SCL I3C PP TIMING full mask. */
#define XI3CPSX_SCL_I3C_PP_TIMING_FULLMASK     0x00ff00ff
/** SCL I3C PP TIMING full read/write mask. */
#define XI3CPSX_SCL_I3C_PP_TIMING_FULLRWMASK  0x00ff00ff
/** SCL I3C PP TIMING default value. */
#define XI3CPSX_SCL_I3C_PP_TIMING_DEFVAL   0xa000a

/* access_type: rw */
/** SCL I3C PP TIMING I3C PP HCNT bit shift. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_HCNT_SHIFT   16
/** SCL I3C PP TIMING I3C PP HCNT bit width. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_HCNT_WIDTH   8
/** SCL I3C PP TIMING I3C PP HCNT bit mask. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_HCNT_MASK    0x00ff0000
/** SCL I3C PP TIMING I3C PP HCNT default value. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_HCNT_DEFVAL  0xa
/** SCL I3C PP TIMING I3C PP HCNT VAL register field. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_HCNT_VAL     8

/* access_type: rw */
/** SCL I3C PP TIMING I3C PP LCNT bit shift. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_SHIFT   0
/** SCL I3C PP TIMING I3C PP LCNT bit width. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_WIDTH   8
/** SCL I3C PP TIMING I3C PP LCNT bit mask. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_MASK    0x000000ff
/** SCL I3C PP TIMING I3C PP LCNT default value. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_DEFVAL  0xa
/** SCL I3C PP TIMING I3C PP LCNT VAL register field. */
#define XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_VAL     8

/**
 * @brief Defines the SCL_I2C_FM_TIMING register offset.
 */
#define XI3CPSX_SCL_I2C_FM_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000BC )
/** SCL I2C FM TIMING full mask. */
#define XI3CPSX_SCL_I2C_FM_TIMING_FULLMASK     0xffffffff
/** SCL I2C FM TIMING full read/write mask. */
#define XI3CPSX_SCL_I2C_FM_TIMING_FULLRWMASK  0xffffffff
/** SCL I2C FM TIMING default value. */
#define XI3CPSX_SCL_I2C_FM_TIMING_DEFVAL   0x100010

/* access_type: rw */
/** SCL I2C FM TIMING I2C FM HCNT bit shift. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_HCNT_SHIFT   16
/** SCL I2C FM TIMING I2C FM HCNT bit width. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_HCNT_WIDTH   16
/** SCL I2C FM TIMING I2C FM HCNT bit mask. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_HCNT_MASK    0xffff0000
/** SCL I2C FM TIMING I2C FM HCNT default value. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_HCNT_DEFVAL  0x10

/* access_type: rw */
/** SCL I2C FM TIMING I2C FM LCNT bit shift. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_LCNT_SHIFT   0
/** SCL I2C FM TIMING I2C FM LCNT bit width. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_LCNT_WIDTH   16
/** SCL I2C FM TIMING I2C FM LCNT bit mask. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_LCNT_MASK    0x0000ffff
/** SCL I2C FM TIMING I2C FM LCNT default value. */
#define XI3CPSX_SCL_I2C_FM_TIMING_I2C_FM_LCNT_DEFVAL  0x10

/**
 * @brief Defines the SCL_I2C_FMP_TIMING register offset.
 */
#define XI3CPSX_SCL_I2C_FMP_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000C0 )
/** SCL I2C FMP TIMING full mask. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_FULLMASK     0x00ffffff
/** SCL I2C FMP TIMING full read/write mask. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_FULLRWMASK  0x00ffffff
/** SCL I2C FMP TIMING default value. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_DEFVAL   0x100010

/* access_type: rw */
/** SCL I2C FMP TIMING I2C FMP HCNT bit shift. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT_SHIFT   16
/** SCL I2C FMP TIMING I2C FMP HCNT bit width. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT_WIDTH   8
/** SCL I2C FMP TIMING I2C FMP HCNT bit mask. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT_MASK    0x00ff0000
/** SCL I2C FMP TIMING I2C FMP HCNT default value. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT_DEFVAL  0x10

/* access_type: rw */
/** SCL I2C FMP TIMING 2C FMP LCNT bit shift. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_2C_FMP_LCNT_SHIFT   0
/** SCL I2C FMP TIMING 2C FMP LCNT bit width. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_2C_FMP_LCNT_WIDTH   16
/** SCL I2C FMP TIMING 2C FMP LCNT bit mask. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_2C_FMP_LCNT_MASK    0x0000ffff
/** SCL I2C FMP TIMING 2C FMP LCNT default value. */
#define XI3CPSX_SCL_I2C_FMP_TIMING_2C_FMP_LCNT_DEFVAL  0x10

/**
 * @brief Defines the SCL_EXT_LCNT_TIMING register offset.
 */
#define XI3CPSX_SCL_EXT_LCNT_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000C8 )
/** SCL EXT LCNT TIMING full mask. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_FULLMASK     0xffffffff
/** SCL EXT LCNT TIMING full read/write mask. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_FULLRWMASK  0xffffffff
/** SCL EXT LCNT TIMING default value. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_DEFVAL   0x20202020

/* access_type: rw */
/** SCL EXT LCNT TIMING I3C EXT LCNT 4 bit shift. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4_SHIFT   24
/** SCL EXT LCNT TIMING I3C EXT LCNT 4 bit width. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4_WIDTH   8
/** SCL EXT LCNT TIMING I3C EXT LCNT 4 bit mask. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4_MASK    0xff000000
/** SCL EXT LCNT TIMING I3C EXT LCNT 4 default value. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4_DEFVAL  0x20

/* access_type: rw */
/** SCL EXT LCNT TIMING I3C EXT LCNT 3 bit shift. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3_SHIFT   16
/** SCL EXT LCNT TIMING I3C EXT LCNT 3 bit width. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3_WIDTH   8
/** SCL EXT LCNT TIMING I3C EXT LCNT 3 bit mask. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3_MASK    0x00ff0000
/** SCL EXT LCNT TIMING I3C EXT LCNT 3 default value. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3_DEFVAL  0x20

/* access_type: rw */
/** SCL EXT LCNT TIMING I3C EXT LCNT 2 bit shift. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2_SHIFT   8
/** SCL EXT LCNT TIMING I3C EXT LCNT 2 bit width. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2_WIDTH   8
/** SCL EXT LCNT TIMING I3C EXT LCNT 2 bit mask. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2_MASK    0x0000ff00
/** SCL EXT LCNT TIMING I3C EXT LCNT 2 default value. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2_DEFVAL  0x20

/* access_type: rw */
/** SCL EXT LCNT TIMING I3C EXT LCNT 1 bit shift. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1_SHIFT   0
/** SCL EXT LCNT TIMING I3C EXT LCNT 1 bit width. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1_WIDTH   8
/** SCL EXT LCNT TIMING I3C EXT LCNT 1 bit mask. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1_MASK    0x000000ff
/** SCL EXT LCNT TIMING I3C EXT LCNT 1 default value. */
#define XI3CPSX_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1_DEFVAL  0x20

/**
 * @brief Defines the SCL_EXT_TERMN_LCNT_TIMING register offset.
 */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000CC )
/** SCL EXT TERMN LCNT TIMING full mask. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_FULLMASK     0x0000000f
/** SCL EXT TERMN LCNT TIMING full read/write mask. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_FULLRWMASK  0x0000000f
/** SCL EXT TERMN LCNT TIMING default value. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_DEFVAL   0x0

/* access_type: rw */
/** SCL EXT TERMN LCNT TIMING I3C EXT TERMN LCNT bit shift. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_I3C_EXT_TERMN_LCNT_SHIFT   0
/** SCL EXT TERMN LCNT TIMING I3C EXT TERMN LCNT bit width. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_I3C_EXT_TERMN_LCNT_WIDTH   4
/** SCL EXT TERMN LCNT TIMING I3C EXT TERMN LCNT bit mask. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_I3C_EXT_TERMN_LCNT_MASK    0x0000000f
/** SCL EXT TERMN LCNT TIMING I3C EXT TERMN LCNT default value. */
#define XI3CPSX_SCL_EXT_TERMN_LCNT_TIMING_I3C_EXT_TERMN_LCNT_DEFVAL  0x0

/**
 * @brief Defines the SDA_HOLD_SWITCH_DLY_TIMING register offset.
 */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000D0 )
/** SDA HOLD SWITCH DLY TIMING full mask. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_FULLMASK     0x00070000
/** SDA HOLD SWITCH DLY TIMING full read/write mask. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_FULLRWMASK  0x00070000
/** SDA HOLD SWITCH DLY TIMING default value. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_DEFVAL   0x10000

/* access_type: rw */
/** SDA HOLD SWITCH DLY TIMING SDA TX HOLD bit shift. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_SHIFT   16
/** SDA HOLD SWITCH DLY TIMING SDA TX HOLD bit width. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_WIDTH   3
/** SDA HOLD SWITCH DLY TIMING SDA TX HOLD bit mask. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_MASK    0x00070000
/** SDA HOLD SWITCH DLY TIMING SDA TX HOLD default value. */
#define XI3CPSX_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_DEFVAL  0x1

/**
 * @brief Defines the BUS_FREE_AVAIL_TIMING register offset.
 */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000D4 )
/** BUS FREE AVAIL TIMING full mask. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_FULLMASK     0xffffffff
/** BUS FREE AVAIL TIMING full read/write mask. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_FULLRWMASK  0xffffffff
/** BUS FREE AVAIL TIMING default value. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_DEFVAL   0x200020

/* access_type: rw */
/** BUS FREE AVAIL TIMING BUS AVAILABLE TIME bit shift. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_SHIFT       16
/** BUS FREE AVAIL TIMING BUS AVAILABLE TIME bit width. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_WIDTH       16
/** BUS FREE AVAIL TIMING BUS AVAILABLE TIME bit mask. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_MASK        0xffff0000
/** BUS FREE AVAIL TIMING BUS AVAILABLE TIME default value. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_DEFVAL      0x20
/** BUS FREE AVAIL TIMING BUS AVAILABLE TIME VAL register field. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME_VAL         0xa0

/* access_type: rw */
/** BUS FREE AVAIL TIMING BUS FREE TIME bit shift. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME_SHIFT   0
/** BUS FREE AVAIL TIMING BUS FREE TIME bit width. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME_WIDTH   16
/** BUS FREE AVAIL TIMING BUS FREE TIME bit mask. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME_MASK    0x0000ffff
/** BUS FREE AVAIL TIMING BUS FREE TIME default value. */
#define XI3CPSX_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME_DEFVAL  0x20

/**
 * @brief Defines the BUS_IDLE_TIMING register offset.
 */
#define XI3CPSX_BUS_IDLE_TIMING    ( ( XI3CPSX_BASEADDR ) + 0x000000D8 )
/** BUS IDLE TIMING full mask. */
#define XI3CPSX_BUS_IDLE_TIMING_FULLMASK     0x000fffff
/** BUS IDLE TIMING full read/write mask. */
#define XI3CPSX_BUS_IDLE_TIMING_FULLRWMASK  0x000fffff
/** BUS IDLE TIMING default value. */
#define XI3CPSX_BUS_IDLE_TIMING_DEFVAL   0x20

/* access_type: rw */
/** BUS IDLE TIMING BUS IDLE TIME bit shift. */
#define XI3CPSX_BUS_IDLE_TIMING_BUS_IDLE_TIME_SHIFT   0
/** BUS IDLE TIMING BUS IDLE TIME bit width. */
#define XI3CPSX_BUS_IDLE_TIMING_BUS_IDLE_TIME_WIDTH   20
/** BUS IDLE TIMING BUS IDLE TIME bit mask. */
#define XI3CPSX_BUS_IDLE_TIMING_BUS_IDLE_TIME_MASK    0x000fffff
/** BUS IDLE TIMING BUS IDLE TIME default value. */
#define XI3CPSX_BUS_IDLE_TIMING_BUS_IDLE_TIME_DEFVAL  0x20

/**
 * @brief Defines the SCL_LOW_MST_EXT_TIMEOUT register offset.
 */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT    ( ( XI3CPSX_BASEADDR ) + 0x000000DC )
/** SCL LOW MST EXT TIMEOUT full mask. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_FULLMASK     0x03ffffff
/** SCL LOW MST EXT TIMEOUT full read/write mask. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_FULLRWMASK  0x03ffffff
/** SCL LOW MST EXT TIMEOUT default value. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_DEFVAL   0x3567e0

/* access_type: rw */
/** SCL LOW MST EXT TIMEOUT SCL LOW MST TIMEOUT COUNT bit shift. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_SCL_LOW_MST_TIMEOUT_COUNT_SHIFT   0
/** SCL LOW MST EXT TIMEOUT SCL LOW MST TIMEOUT COUNT bit width. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_SCL_LOW_MST_TIMEOUT_COUNT_WIDTH   26
/** SCL LOW MST EXT TIMEOUT SCL LOW MST TIMEOUT COUNT bit mask. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_SCL_LOW_MST_TIMEOUT_COUNT_MASK    0x03ffffff
/** SCL LOW MST EXT TIMEOUT SCL LOW MST TIMEOUT COUNT default value. */
#define XI3CPSX_SCL_LOW_MST_EXT_TIMEOUT_SCL_LOW_MST_TIMEOUT_COUNT_DEFVAL  0x3567e0

/**
 * @brief Defines the I3C_VER_ID register offset.
 */
#define XI3CPSX_I3C_VER_ID    ( ( XI3CPSX_BASEADDR ) + 0x000000E0 )
/** I3C VER ID full mask. */
#define XI3CPSX_I3C_VER_ID_FULLMASK     0xffffffff
/** I3C VER ID full read/write mask. */
#define XI3CPSX_I3C_VER_ID_FULLRWMASK  0x00000000
/** I3C VER ID default value. */
#define XI3CPSX_I3C_VER_ID_DEFVAL   0x3130302a

/* access_type: ro */
/** I3C VER ID I3C VER ID bit shift. */
#define XI3CPSX_I3C_VER_ID_I3C_VER_ID_SHIFT   0
/** I3C VER ID I3C VER ID bit width. */
#define XI3CPSX_I3C_VER_ID_I3C_VER_ID_WIDTH   32
/** I3C VER ID I3C VER ID bit mask. */
#define XI3CPSX_I3C_VER_ID_I3C_VER_ID_MASK    0xffffffff
/** I3C VER ID I3C VER ID default value. */
#define XI3CPSX_I3C_VER_ID_I3C_VER_ID_DEFVAL  0x3130302a

/**
 * @brief Defines the I3C_VER_TYPE register offset.
 */
#define XI3CPSX_I3C_VER_TYPE    ( ( XI3CPSX_BASEADDR ) + 0x000000E4 )
/** I3C VER TYPE full mask. */
#define XI3CPSX_I3C_VER_TYPE_FULLMASK     0xffffffff
/** I3C VER TYPE full read/write mask. */
#define XI3CPSX_I3C_VER_TYPE_FULLRWMASK  0x00000000
/** I3C VER TYPE default value. */
#define XI3CPSX_I3C_VER_TYPE_DEFVAL   0x6c633033

/* access_type: ro */
/** I3C VER TYPE I3C VER TYPE bit shift. */
#define XI3CPSX_I3C_VER_TYPE_I3C_VER_TYPE_SHIFT   0
/** I3C VER TYPE I3C VER TYPE bit width. */
#define XI3CPSX_I3C_VER_TYPE_I3C_VER_TYPE_WIDTH   32
/** I3C VER TYPE I3C VER TYPE bit mask. */
#define XI3CPSX_I3C_VER_TYPE_I3C_VER_TYPE_MASK    0xffffffff
/** I3C VER TYPE I3C VER TYPE default value. */
#define XI3CPSX_I3C_VER_TYPE_I3C_VER_TYPE_DEFVAL  0x6c633033

/**
 * @brief Defines the QUEUE_SIZE_CAPABILITY register offset.
 */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY    ( ( XI3CPSX_BASEADDR ) + 0x000000E8 )
/** QUEUE SIZE CAPABILITY full mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_FULLMASK     0x000fffff
/** QUEUE SIZE CAPABILITY full read/write mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_FULLRWMASK  0x00000000
/** QUEUE SIZE CAPABILITY default value. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_DEFVAL   0x21244

/* access_type: ro */
/** QUEUE SIZE CAPABILITY IBI BUF SIZE bit shift. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_IBI_BUF_SIZE_SHIFT   16
/** QUEUE SIZE CAPABILITY IBI BUF SIZE bit width. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_IBI_BUF_SIZE_WIDTH   4
/** QUEUE SIZE CAPABILITY IBI BUF SIZE bit mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_IBI_BUF_SIZE_MASK    0x000f0000
/** QUEUE SIZE CAPABILITY IBI BUF SIZE default value. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_IBI_BUF_SIZE_DEFVAL  0x2

/* access_type: ro */
/** QUEUE SIZE CAPABILITY RESP BUF SIZE bit shift. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RESP_BUF_SIZE_SHIFT   12
/** QUEUE SIZE CAPABILITY RESP BUF SIZE bit width. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RESP_BUF_SIZE_WIDTH   4
/** QUEUE SIZE CAPABILITY RESP BUF SIZE bit mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RESP_BUF_SIZE_MASK    0x0000f000
/** QUEUE SIZE CAPABILITY RESP BUF SIZE default value. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RESP_BUF_SIZE_DEFVAL  0x1

/* access_type: ro */
/** QUEUE SIZE CAPABILITY CMD BUF SIZE bit shift. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_CMD_BUF_SIZE_SHIFT   8
/** QUEUE SIZE CAPABILITY CMD BUF SIZE bit width. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_CMD_BUF_SIZE_WIDTH   4
/** QUEUE SIZE CAPABILITY CMD BUF SIZE bit mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_CMD_BUF_SIZE_MASK    0x00000f00
/** QUEUE SIZE CAPABILITY CMD BUF SIZE default value. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_CMD_BUF_SIZE_DEFVAL  0x2

/* access_type: ro */
/** QUEUE SIZE CAPABILITY RX BUF SIZE bit shift. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RX_BUF_SIZE_SHIFT   4
/** QUEUE SIZE CAPABILITY RX BUF SIZE bit width. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RX_BUF_SIZE_WIDTH   4
/** QUEUE SIZE CAPABILITY RX BUF SIZE bit mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RX_BUF_SIZE_MASK    0x000000f0
/** QUEUE SIZE CAPABILITY RX BUF SIZE default value. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_RX_BUF_SIZE_DEFVAL  0x4

/* access_type: ro */
/** QUEUE SIZE CAPABILITY TX BUF SIZE bit shift. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_TX_BUF_SIZE_SHIFT   0
/** QUEUE SIZE CAPABILITY TX BUF SIZE bit width. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_TX_BUF_SIZE_WIDTH   4
/** QUEUE SIZE CAPABILITY TX BUF SIZE bit mask. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_TX_BUF_SIZE_MASK    0x0000000f
/** QUEUE SIZE CAPABILITY TX BUF SIZE default value. */
#define XI3CPSX_QUEUE_SIZE_CAPABILITY_TX_BUF_SIZE_DEFVAL  0x4

/**
 * @brief Defines the DEV_CHAR_TABLE1_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000200 )
/** DEV CHAR TABLE1 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE1 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE1 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE1 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE1 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE1 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE1 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE1 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE1 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE1 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE1 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE1 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE1 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE1 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE1 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE1_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000204 )
/** DEV CHAR TABLE1 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE1 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE1 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE1 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE1 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE1 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE1_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000208 )
/** DEV CHAR TABLE1 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE1 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE1 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE1 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE1 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE1 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE1 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE1 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE1 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE1_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000020C )
/** DEV CHAR TABLE1 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE1 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE1 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE1 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE1 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE1 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE1 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE1_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE2_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000210 )
/** DEV CHAR TABLE2 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE2 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE2 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE2 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE2 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE2 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE2 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE2 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE2 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE2 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE2 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE2 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE2 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE2 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE2 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE2_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000214 )
/** DEV CHAR TABLE2 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE2 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE2 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE2 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE2 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE2 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE2_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000218 )
/** DEV CHAR TABLE2 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE2 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE2 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE2 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE2 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE2 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE2 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE2 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE2 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE2_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000021C )
/** DEV CHAR TABLE2 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE2 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE2 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE2 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE2 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE2 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE2 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE2_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE3_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000220 )
/** DEV CHAR TABLE3 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE3 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE3 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE3 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE3 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE3 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE3 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE3 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE3 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE3 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE3 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE3 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE3 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE3 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE3 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE3_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000224 )
/** DEV CHAR TABLE3 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE3 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE3 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE3 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE3 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE3 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE3_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000228 )
/** DEV CHAR TABLE3 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE3 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE3 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE3 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE3 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE3 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE3 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE3 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE3 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE3_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000022C )
/** DEV CHAR TABLE3 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE3 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE3 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE3 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE3 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE3 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE3 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE3_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE4_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000230 )
/** DEV CHAR TABLE4 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE4 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE4 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE4 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE4 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE4 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE4 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE4 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE4 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE4 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE4 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE4 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE4 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE4 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE4 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE4_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000234 )
/** DEV CHAR TABLE4 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE4 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE4 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE4 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE4 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE4 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE4_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000238 )
/** DEV CHAR TABLE4 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE4 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE4 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE4 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE4 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE4 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE4 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE4 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE4 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE4_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000023C )
/** DEV CHAR TABLE4 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE4 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE4 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE4 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE4 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE4 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE4 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE4_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE5_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000240 )
/** DEV CHAR TABLE5 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE5 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE5 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE5 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE5 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE5 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE5 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE5 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE5 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE5 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE5 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE5 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE5 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE5 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE5 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE5_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000244 )
/** DEV CHAR TABLE5 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE5 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE5 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE5 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE5 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE5 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE5_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000248 )
/** DEV CHAR TABLE5 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE5 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE5 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE5 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE5 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE5 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE5 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE5 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE5 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE5_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000024C )
/** DEV CHAR TABLE5 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE5 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE5 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE5 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE5 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE5 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE5 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE5_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE6_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000250 )
/** DEV CHAR TABLE6 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE6 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE6 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE6 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE6 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE6 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE6 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE6 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE6 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE6 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE6 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE6 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE6 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE6 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE6 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE6_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000254 )
/** DEV CHAR TABLE6 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE6 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE6 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE6 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE6 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE6 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE6_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000258 )
/** DEV CHAR TABLE6 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE6 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE6 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE6 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE6 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE6 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE6 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE6 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE6 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE6_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000025C )
/** DEV CHAR TABLE6 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE6 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE6 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE6 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE6 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE6 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE6 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE6_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE7_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000260 )
/** DEV CHAR TABLE7 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE7 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE7 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE7 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE7 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE7 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE7 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE7 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE7 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE7 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE7 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE7 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE7 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE7 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE7 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE7_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000264 )
/** DEV CHAR TABLE7 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE7 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE7 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE7 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE7 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE7 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE7_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000268 )
/** DEV CHAR TABLE7 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE7 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE7 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE7 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE7 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE7 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE7 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE7 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE7 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE7_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000026C )
/** DEV CHAR TABLE7 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE7 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE7 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE7 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE7 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE7 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE7 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE7_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE8_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000270 )
/** DEV CHAR TABLE8 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE8 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE8 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE8 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE8 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE8 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE8 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE8 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE8 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE8 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE8 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE8 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE8 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE8 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE8 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE8_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000274 )
/** DEV CHAR TABLE8 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE8 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE8 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE8 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE8 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE8 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE8_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000278 )
/** DEV CHAR TABLE8 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE8 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE8 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE8 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE8 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE8 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE8 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE8 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE8 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE8_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000027C )
/** DEV CHAR TABLE8 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE8 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE8 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE8 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE8 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE8 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE8 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE8_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE9_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000280 )
/** DEV CHAR TABLE9 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE9 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE9 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE9 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE9 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE9 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE9 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE9 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE9 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE9 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE9 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE9 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE9 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE9 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE9 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE9_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000284 )
/** DEV CHAR TABLE9 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE9 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE9 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE9 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE9 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE9 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE9_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000288 )
/** DEV CHAR TABLE9 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE9 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE9 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE9 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE9 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE9 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE9 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE9 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE9 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE9_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000028C )
/** DEV CHAR TABLE9 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE9 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE9 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE9 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE9 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE9 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE9 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE9_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE10_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x00000290 )
/** DEV CHAR TABLE10 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE10 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE10 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE10 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE10 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE10 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE10 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE10 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE10 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE10 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE10 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE10 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE10 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE10 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE10 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE10_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x00000294 )
/** DEV CHAR TABLE10 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE10 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE10 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE10 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE10 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE10 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE10_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x00000298 )
/** DEV CHAR TABLE10 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE10 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE10 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE10 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE10 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE10 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE10 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE10 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE10 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE10_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x0000029C )
/** DEV CHAR TABLE10 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE10 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE10 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE10 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE10 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE10 LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE10 LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE10_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE11_LOC1 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x000002A0 )
/** DEV CHAR TABLE11 LOC1 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_FULLMASK     0xffffffff
/** DEV CHAR TABLE11 LOC1 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_FULLRWMASK  0x00000000
/** DEV CHAR TABLE11 LOC1 default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC1 STATIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_STATIC_ADDR_SHIFT   24
/** DEV CHAR TABLE11 LOC1 STATIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_STATIC_ADDR_WIDTH   8
/** DEV CHAR TABLE11 LOC1 STATIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_STATIC_ADDR_MASK    0xff000000
/** DEV CHAR TABLE11 LOC1 STATIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_STATIC_ADDR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC1 BCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_BCR_TYPE_SHIFT   16
/** DEV CHAR TABLE11 LOC1 BCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_BCR_TYPE_WIDTH   8
/** DEV CHAR TABLE11 LOC1 BCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_BCR_TYPE_MASK    0x00ff0000
/** DEV CHAR TABLE11 LOC1 BCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_BCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC1 DCR TYPE bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DCR_TYPE_SHIFT   8
/** DEV CHAR TABLE11 LOC1 DCR TYPE bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DCR_TYPE_WIDTH   8
/** DEV CHAR TABLE11 LOC1 DCR TYPE bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DCR_TYPE_MASK    0x0000ff00
/** DEV CHAR TABLE11 LOC1 DCR TYPE default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DCR_TYPE_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC1 DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE11 LOC1 DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DYNAMIC_ADDR_WIDTH   8
/** DEV CHAR TABLE11 LOC1 DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DYNAMIC_ADDR_MASK    0x000000ff
/** DEV CHAR TABLE11 LOC1 DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC1_DYNAMIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE11_LOC2 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x000002A4 )
/** DEV CHAR TABLE11 LOC2 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_FULLMASK     0x0000ffff
/** DEV CHAR TABLE11 LOC2 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_FULLRWMASK  0x00000000
/** DEV CHAR TABLE11 LOC2 default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC2 MSB PROVISIONAL ID bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_MSB_PROVISIONAL_ID_SHIFT   0
/** DEV CHAR TABLE11 LOC2 MSB PROVISIONAL ID bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_MSB_PROVISIONAL_ID_WIDTH   16
/** DEV CHAR TABLE11 LOC2 MSB PROVISIONAL ID bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_MSB_PROVISIONAL_ID_MASK    0x0000ffff
/** DEV CHAR TABLE11 LOC2 MSB PROVISIONAL ID default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC2_MSB_PROVISIONAL_ID_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE11_LOC3 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x000002A8 )
/** DEV CHAR TABLE11 LOC3 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_FULLMASK     0x0000ffff
/** DEV CHAR TABLE11 LOC3 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_FULLRWMASK  0x00000000
/** DEV CHAR TABLE11 LOC3 default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC3 BCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_BCR_SHIFT   8
/** DEV CHAR TABLE11 LOC3 BCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_BCR_WIDTH   8
/** DEV CHAR TABLE11 LOC3 BCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_BCR_MASK    0x0000ff00
/** DEV CHAR TABLE11 LOC3 BCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_BCR_DEFVAL  0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC3 DCR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_DCR_SHIFT   0
/** DEV CHAR TABLE11 LOC3 DCR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_DCR_WIDTH   8
/** DEV CHAR TABLE11 LOC3 DCR bit mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_DCR_MASK    0x000000ff
/** DEV CHAR TABLE11 LOC3 DCR default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC3_DCR_DEFVAL  0x0

/**
 * @brief Defines the DEV_CHAR_TABLE11_LOC4 register offset.
 */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x000002AC )
/** DEV CHAR TABLE11 LOC4 full mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_FULLMASK     0x000000ff
/** DEV CHAR TABLE11 LOC4 full read/write mask. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_FULLRWMASK  0x00000000
/** DEV CHAR TABLE11 LOC4 default value. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_DEFVAL   0x0

/* access_type: ro */
/** DEV CHAR TABLE11 LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_DEV_DYNAMIC_ADDR_SHIFT   0
/** DEV CHAR TABLE11 LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_DEV_DYNAMIC_ADDR_MASK    0x000000ff	/**< Mask for the device dynamic address field */
#define XI3CPSX_DEV_CHAR_TABLE11_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0	/**< Reset value of the device dynamic address field */

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC1 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1    ( ( XI3CPSX_BASEADDR ) + 0x000002C0 )
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_FULLMASK     0xe0ff787f	/**< Mask of all valid bits in DEV_ADDR_TABLE_LOC1 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_FULLRWMASK  0xe0ff787f	/**< Mask of all read/write bits in DEV_ADDR_TABLE_LOC1 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEFVAL   0x0	/**< Reset value of DEV_ADDR_TABLE_LOC1 */

/* access_type: rw */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_MR_REJECT_SHIFT   14	/**< Bit shift for the Master-Request reject field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_MR_REJECT_WIDTH   1	/**< Bit width of the Master-Request reject field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_MR_REJECT_MASK    0x00004000	/**< Mask for the Master-Request reject field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_MR_REJECT_DEFVAL  0x0	/**< Reset value of the Master-Request reject field */

/* access_type: rw */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_SIR_REJECT_SHIFT   13	/**< Bit shift for the Slave-Interrupt-Request reject field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_SIR_REJECT_WIDTH   1	/**< Bit width of the Slave-Interrupt-Request reject field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_SIR_REJECT_MASK    0x00002000	/**< Mask for the Slave-Interrupt-Request reject field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_SIR_REJECT_DEFVAL  0x0	/**< Reset value of the Slave-Interrupt-Request reject field */

/* access_type: rw */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_WITH_DATA_SHIFT   12	/**< Bit shift for the IBI-with-data field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_WITH_DATA_WIDTH   1	/**< Bit width of the IBI-with-data field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_WITH_DATA_MASK    0x00001000	/**< Mask for the IBI-with-data field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_WITH_DATA_DEFVAL  0x0	/**< Reset value of the IBI-with-data field */

/* access_type: rw */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_PEC_EN_SHIFT   11	/**< Bit shift for the IBI PEC-enable field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_PEC_EN_WIDTH   1	/**< Bit width of the IBI PEC-enable field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_PEC_EN_MASK    0x00000800	/**< Mask for the IBI PEC-enable field */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_IBI_PEC_EN_DEFVAL  0x0	/**< Reset value of the IBI PEC-enable field */

/* access_type: rw */
/** DEV ADDR TABLE LOC1 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC1 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC1 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC1 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC1 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC1 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC1 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC1 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC1 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC1 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC1 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC1 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC1 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC1 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC1 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC1 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC2 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2    ( ( XI3CPSX_BASEADDR ) + 0x000002C4 )
/** DEV ADDR TABLE LOC2 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC2 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC2 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC2 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC2 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC2 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC2 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC2 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC2 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC2 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC2 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC2 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC2 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC2 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC2 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC2 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC2 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC2 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC2 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC2_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC3 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3    ( ( XI3CPSX_BASEADDR ) + 0x000002C8 )
/** DEV ADDR TABLE LOC3 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC3 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC3 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC3 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC3 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC3 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC3 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC3 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC3 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC3 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC3 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC3 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC3 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC3 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC3 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC3 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC3 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC3 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC3 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC3_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC4 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4    ( ( XI3CPSX_BASEADDR ) + 0x000002CC )
/** DEV ADDR TABLE LOC4 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC4 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC4 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC4 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC4 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC4 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC4 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC4 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC4 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC4 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC4 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC4 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC4 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC4 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC4 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC4 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC4 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC4 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC4 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC4_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC5 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5    ( ( XI3CPSX_BASEADDR ) + 0x000002D0 )
/** DEV ADDR TABLE LOC5 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC5 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC5 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC5 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC5 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC5 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC5 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC5 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC5 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC5 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC5 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC5 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC5 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC5 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC5 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC5 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC5 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC5 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC5 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC5_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC6 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6    ( ( XI3CPSX_BASEADDR ) + 0x000002D4 )
/** DEV ADDR TABLE LOC6 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC6 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC6 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC6 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC6 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC6 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC6 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC6 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC6 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC6 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC6 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC6 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC6 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC6 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC6 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC6 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC6 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC6 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC6 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC6_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC7 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7    ( ( XI3CPSX_BASEADDR ) + 0x000002D8 )
/** DEV ADDR TABLE LOC7 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC7 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC7 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC7 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC7 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC7 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC7 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC7 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC7 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC7 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC7 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC7 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC7 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC7 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC7 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC7 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC7 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC7 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC7 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC7_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC8 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8    ( ( XI3CPSX_BASEADDR ) + 0x000002DC )
/** DEV ADDR TABLE LOC8 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC8 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC8 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC8 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC8 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC8 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC8 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC8 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC8 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC8 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC8 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC8 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC8 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC8 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC8 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC8 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC8 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC8 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC8 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC8_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC9 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9    ( ( XI3CPSX_BASEADDR ) + 0x000002E0 )
/** DEV ADDR TABLE LOC9 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC9 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC9 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC9 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC9 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC9 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC9 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC9 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC9 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC9 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC9 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC9 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC9 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC9 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC9 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC9 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC9 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC9 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC9 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC9_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC10 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10    ( ( XI3CPSX_BASEADDR ) + 0x000002E4 )
/** DEV ADDR TABLE LOC10 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC10 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC10 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC10 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC10 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC10 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC10 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC10 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC10 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC10 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC10 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC10 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC10 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC10 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC10 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC10 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC10 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC10 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC10 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC10_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the DEV_ADDR_TABLE_LOC11 register offset.
 */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11    ( ( XI3CPSX_BASEADDR ) + 0x000002E8 )
/** DEV ADDR TABLE LOC11 full mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_FULLMASK     0xe0ff007f
/** DEV ADDR TABLE LOC11 full read/write mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_FULLRWMASK  0xe0ff007f
/** DEV ADDR TABLE LOC11 default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEFVAL   0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC11 LEGACY I2C DEVICE bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_LEGACY_I2C_DEVICE_SHIFT   31
/** DEV ADDR TABLE LOC11 LEGACY I2C DEVICE bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_LEGACY_I2C_DEVICE_WIDTH   1
/** DEV ADDR TABLE LOC11 LEGACY I2C DEVICE bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_LEGACY_I2C_DEVICE_MASK    0x80000000
/** DEV ADDR TABLE LOC11 LEGACY I2C DEVICE default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_LEGACY_I2C_DEVICE_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC11 DEV NACK RETRY CNT bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_NACK_RETRY_CNT_SHIFT   29
/** DEV ADDR TABLE LOC11 DEV NACK RETRY CNT bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_NACK_RETRY_CNT_WIDTH   2
/** DEV ADDR TABLE LOC11 DEV NACK RETRY CNT bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_NACK_RETRY_CNT_MASK    0x60000000
/** DEV ADDR TABLE LOC11 DEV NACK RETRY CNT default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_NACK_RETRY_CNT_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC11 DEV DYNAMIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_DYNAMIC_ADDR_SHIFT   16
/** DEV ADDR TABLE LOC11 DEV DYNAMIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_DYNAMIC_ADDR_WIDTH   8
/** DEV ADDR TABLE LOC11 DEV DYNAMIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_DYNAMIC_ADDR_MASK    0x00ff0000
/** DEV ADDR TABLE LOC11 DEV DYNAMIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_DYNAMIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** DEV ADDR TABLE LOC11 DEV STATIC ADDR bit shift. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_STATIC_ADDR_SHIFT   0
/** DEV ADDR TABLE LOC11 DEV STATIC ADDR bit width. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_STATIC_ADDR_WIDTH   7
/** DEV ADDR TABLE LOC11 DEV STATIC ADDR bit mask. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_STATIC_ADDR_MASK    0x0000007f
/** DEV ADDR TABLE LOC11 DEV STATIC ADDR default value. */
#define XI3CPSX_DEV_ADDR_TABLE_LOC11_DEV_STATIC_ADDR_DEFVAL  0x0

/**
 * @brief Defines the EXT_REG_1 register offset.
 */
#define XI3CPSX_EXT_REG_1    ( ( XI3CPSX_BASEADDR ) + 0x00004000 )
/** EXT REG 1 full mask. */
#define XI3CPSX_EXT_REG_1_FULLMASK     0x3fffffff
/** EXT REG 1 full read/write mask. */
#define XI3CPSX_EXT_REG_1_FULLRWMASK  0x3feffffd
#define XI3CPSX_EXT_REG_1_DEFVAL   0x2	/**< EXT REG 1 default value. */

/* access_type: rw */
/** EXT REG 1 SLV CLK DATA TURN TIME bit shift. */
#define XI3CPSX_EXT_REG_1_SLV_CLK_DATA_TURN_TIME_SHIFT   27
/** EXT REG 1 SLV CLK DATA TURN TIME bit width. */
#define XI3CPSX_EXT_REG_1_SLV_CLK_DATA_TURN_TIME_WIDTH   3
/** EXT REG 1 SLV CLK DATA TURN TIME bit mask. */
#define XI3CPSX_EXT_REG_1_SLV_CLK_DATA_TURN_TIME_MASK    0x38000000
/** EXT REG 1 SLV CLK DATA TURN TIME default value. */
#define XI3CPSX_EXT_REG_1_SLV_CLK_DATA_TURN_TIME_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 SLV MAX WR SPEED bit shift. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_WR_SPEED_SHIFT   24
/** EXT REG 1 SLV MAX WR SPEED bit width. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_WR_SPEED_WIDTH   3
/** EXT REG 1 SLV MAX WR SPEED bit mask. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_WR_SPEED_MASK    0x07000000
/** EXT REG 1 SLV MAX WR SPEED default value. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_WR_SPEED_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 SLV MAX RD SPEED bit shift. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_RD_SPEED_SHIFT   21
/** EXT REG 1 SLV MAX RD SPEED bit width. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_RD_SPEED_WIDTH   3
/** EXT REG 1 SLV MAX RD SPEED bit mask. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_RD_SPEED_MASK    0x00e00000
/** EXT REG 1 SLV MAX RD SPEED default value. */
#define XI3CPSX_EXT_REG_1_SLV_MAX_RD_SPEED_DEFVAL  0x0

/* access_type: ro */
/** EXT REG 1 WAKEUP bit shift. */
#define XI3CPSX_EXT_REG_1_WAKEUP_SHIFT   20
/** EXT REG 1 WAKEUP bit width. */
#define XI3CPSX_EXT_REG_1_WAKEUP_WIDTH   1
/** EXT REG 1 WAKEUP bit mask. */
#define XI3CPSX_EXT_REG_1_WAKEUP_MASK    0x00100000
/** EXT REG 1 WAKEUP default value. */
#define XI3CPSX_EXT_REG_1_WAKEUP_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 INST ID bit shift. */
#define XI3CPSX_EXT_REG_1_INST_ID_SHIFT   16
/** EXT REG 1 INST ID bit width. */
#define XI3CPSX_EXT_REG_1_INST_ID_WIDTH   4
/** EXT REG 1 INST ID bit mask. */
#define XI3CPSX_EXT_REG_1_INST_ID_MASK    0x000f0000
/** EXT REG 1 INST ID default value. */
#define XI3CPSX_EXT_REG_1_INST_ID_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 STATIC ADDR bit shift. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_SHIFT   9
/** EXT REG 1 STATIC ADDR bit width. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_WIDTH   7
/** EXT REG 1 STATIC ADDR bit mask. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_MASK    0x0000fe00
/** EXT REG 1 STATIC ADDR default value. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 STATIC ADDR EN bit shift. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_EN_SHIFT   8
/** EXT REG 1 STATIC ADDR EN bit width. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_EN_WIDTH   1
/** EXT REG 1 STATIC ADDR EN bit mask. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_EN_MASK    0x00000100
/** EXT REG 1 STATIC ADDR EN default value. */
#define XI3CPSX_EXT_REG_1_STATIC_ADDR_EN_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 PENDING INT bit shift. */
#define XI3CPSX_EXT_REG_1_PENDING_INT_SHIFT   4
/** EXT REG 1 PENDING INT bit width. */
#define XI3CPSX_EXT_REG_1_PENDING_INT_WIDTH   4
/** EXT REG 1 PENDING INT bit mask. */
#define XI3CPSX_EXT_REG_1_PENDING_INT_MASK    0x000000f0
/** EXT REG 1 PENDING INT default value. */
#define XI3CPSX_EXT_REG_1_PENDING_INT_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 1 ACT MODE bit shift. */
#define XI3CPSX_EXT_REG_1_ACT_MODE_SHIFT   2
/** EXT REG 1 ACT MODE bit width. */
#define XI3CPSX_EXT_REG_1_ACT_MODE_WIDTH   2
/** EXT REG 1 ACT MODE bit mask. */
#define XI3CPSX_EXT_REG_1_ACT_MODE_MASK    0x0000000c
/** EXT REG 1 ACT MODE default value. */
#define XI3CPSX_EXT_REG_1_ACT_MODE_DEFVAL  0x0

/* access_type: ro */
/** EXT REG 1 I2C GLITCH FILTER EN bit shift. */
#define XI3CPSX_EXT_REG_1_I2C_GLITCH_FILTER_EN_SHIFT   1
/** EXT REG 1 I2C GLITCH FILTER EN bit width. */
#define XI3CPSX_EXT_REG_1_I2C_GLITCH_FILTER_EN_WIDTH   1
/** EXT REG 1 I2C GLITCH FILTER EN bit mask. */
#define XI3CPSX_EXT_REG_1_I2C_GLITCH_FILTER_EN_MASK    0x00000002
/** EXT REG 1 I2C GLITCH FILTER EN default value. */
#define XI3CPSX_EXT_REG_1_I2C_GLITCH_FILTER_EN_DEFVAL  0x1

/* access_type: rw */
/** EXT REG 1 MODE I2C bit shift. */
#define XI3CPSX_EXT_REG_1_MODE_I2C_SHIFT   0
/** EXT REG 1 MODE I2C bit width. */
#define XI3CPSX_EXT_REG_1_MODE_I2C_WIDTH   1
/** EXT REG 1 MODE I2C bit mask. */
#define XI3CPSX_EXT_REG_1_MODE_I2C_MASK    0x00000001
/** EXT REG 1 MODE I2C default value. */
#define XI3CPSX_EXT_REG_1_MODE_I2C_DEFVAL  0x0

/**
 * @brief Defines the EXT_REG_2 register offset.
 */
#define XI3CPSX_EXT_REG_2    ( ( XI3CPSX_BASEADDR ) + 0x00004004 )
/** EXT REG 2 full mask. */
#define XI3CPSX_EXT_REG_2_FULLMASK     0xffffffff
/** EXT REG 2 full read/write mask. */
#define XI3CPSX_EXT_REG_2_FULLRWMASK  0xffffffff
#define XI3CPSX_EXT_REG_2_DEFVAL   0x0	/**< EXT REG 2 default value. */

/* access_type: rw */
/** EXT REG 2 SLV PID L bit shift. */
#define XI3CPSX_EXT_REG_2_SLV_PID_L_SHIFT   0
/** EXT REG 2 SLV PID L bit width. */
#define XI3CPSX_EXT_REG_2_SLV_PID_L_WIDTH   32
/** EXT REG 2 SLV PID L bit mask. */
#define XI3CPSX_EXT_REG_2_SLV_PID_L_MASK    0xffffffff
/** EXT REG 2 SLV PID L default value. */
#define XI3CPSX_EXT_REG_2_SLV_PID_L_DEFVAL  0x0

/**
 * @brief Defines the EXT_REG_3 register offset.
 */
#define XI3CPSX_EXT_REG_3    ( ( XI3CPSX_BASEADDR ) + 0x00004008 )
/** EXT REG 3 full mask. */
#define XI3CPSX_EXT_REG_3_FULLMASK     0x00ffffff
/** EXT REG 3 full read/write mask. */
#define XI3CPSX_EXT_REG_3_FULLRWMASK  0x00ffffff
#define XI3CPSX_EXT_REG_3_DEFVAL   0x0	/**< EXT REG 3 default value. */

/* access_type: rw */
/** EXT REG 3 SLV DCR bit shift. */
#define XI3CPSX_EXT_REG_3_SLV_DCR_SHIFT   16
/** EXT REG 3 SLV DCR bit width. */
#define XI3CPSX_EXT_REG_3_SLV_DCR_WIDTH   8
/** EXT REG 3 SLV DCR bit mask. */
#define XI3CPSX_EXT_REG_3_SLV_DCR_MASK    0x00ff0000
/** EXT REG 3 SLV DCR default value. */
#define XI3CPSX_EXT_REG_3_SLV_DCR_DEFVAL  0x0

/* access_type: rw */
/** EXT REG 3 SLV PID U bit shift. */
#define XI3CPSX_EXT_REG_3_SLV_PID_U_SHIFT   0
/** EXT REG 3 SLV PID U bit width. */
#define XI3CPSX_EXT_REG_3_SLV_PID_U_WIDTH   16
/** EXT REG 3 SLV PID U bit mask. */
#define XI3CPSX_EXT_REG_3_SLV_PID_U_MASK    0x0000ffff
/** EXT REG 3 SLV PID U default value. */
#define XI3CPSX_EXT_REG_3_SLV_PID_U_DEFVAL  0x0

/**
 * @brief Defines the I3C_GLITCHFILTER_CTRL register offset.
 */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL    ( ( XI3CPSX_BASEADDR ) + 0x0000400C )
/** I3C GLITCHFILTER CTRL full mask. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_FULLMASK     0x0000003f
/** I3C GLITCHFILTER CTRL full read/write mask. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_FULLRWMASK  0x0000003f
/** I3C GLITCHFILTER CTRL default value. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_DEFVAL   0x10

/* access_type: rw */
/** I3C GLITCHFILTER CTRL GF COUNTER bit shift. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_GF_COUNTER_SHIFT   1
/** I3C GLITCHFILTER CTRL GF COUNTER bit width. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_GF_COUNTER_WIDTH   5
/** I3C GLITCHFILTER CTRL GF COUNTER bit mask. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_GF_COUNTER_MASK    0x0000003e
/** I3C GLITCHFILTER CTRL GF COUNTER default value. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_GF_COUNTER_DEFVAL  0x8

/* access_type: rw */
/** I3C GLITCHFILTER CTRL USE GF bit shift. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_USE_GF_SHIFT   0
/** I3C GLITCHFILTER CTRL USE GF bit width. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_USE_GF_WIDTH   1
/** I3C GLITCHFILTER CTRL USE GF bit mask. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_USE_GF_MASK    0x00000001
/** I3C GLITCHFILTER CTRL USE GF default value. */
#define XI3CPSX_I3C_GLITCHFILTER_CTRL_USE_GF_DEFVAL  0x0

/**
 * @brief Defines the ECO register offset.
 */
#define XI3CPSX_ECO    ( ( XI3CPSX_BASEADDR ) + 0x00004010 )
#define XI3CPSX_ECO_FULLMASK     0xffffffff	/**< ECO full mask. */
/** ECO full read/write mask. */
#define XI3CPSX_ECO_FULLRWMASK  0xffffffff
#define XI3CPSX_ECO_DEFVAL   0x0	/**< ECO default value. */

/* access_type: rw */
/** ECO FIELD NAME bit shift. */
#define XI3CPSX_ECO_FIELD_NAME_SHIFT   0
/** ECO FIELD NAME bit width. */
#define XI3CPSX_ECO_FIELD_NAME_WIDTH   32
/** ECO FIELD NAME bit mask. */
#define XI3CPSX_ECO_FIELD_NAME_MASK    0xffffffff
/** ECO FIELD NAME default value. */
#define XI3CPSX_ECO_FIELD_NAME_DEFVAL  0x0

/**
 * @brief Defines the EXT_DEBUG_REG_1 register offset.
 */
#define XI3CPSX_EXT_DEBUG_REG_1    ( ( XI3CPSX_BASEADDR ) + 0x00004014 )
/** EXT DEBUG REG 1 full mask. */
#define XI3CPSX_EXT_DEBUG_REG_1_FULLMASK     0xffffffff
/** EXT DEBUG REG 1 full read/write mask. */
#define XI3CPSX_EXT_DEBUG_REG_1_FULLRWMASK  0x00000000
/** EXT DEBUG REG 1 default value. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEFVAL   0x80005500

/* access_type: ro */
/** EXT DEBUG REG 1 DEBUG PORT 31 8 bit shift. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_31_8_SHIFT   8
/** EXT DEBUG REG 1 DEBUG PORT 31 8 bit width. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_31_8_WIDTH   24
/** EXT DEBUG REG 1 DEBUG PORT 31 8 bit mask. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_31_8_MASK    0xffffff00
/** EXT DEBUG REG 1 DEBUG PORT 31 8 default value. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_31_8_DEFVAL  0x800055

/* access_type: ro */
/** EXT DEBUG REG 1 DEBUG PORT 7 0 bit shift. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_7_0_SHIFT   0
/** EXT DEBUG REG 1 DEBUG PORT 7 0 bit width. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_7_0_WIDTH   8
/** EXT DEBUG REG 1 DEBUG PORT 7 0 bit mask. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_7_0_MASK    0x000000ff
/** EXT DEBUG REG 1 DEBUG PORT 7 0 default value. */
#define XI3CPSX_EXT_DEBUG_REG_1_DEBUG_PORT_7_0_DEFVAL  0x0

/**
 * @brief Defines the EXT_DEBUG_REG_2 register offset.
 */
#define XI3CPSX_EXT_DEBUG_REG_2    ( ( XI3CPSX_BASEADDR ) + 0x00004018 )
/** EXT DEBUG REG 2 full mask. */
#define XI3CPSX_EXT_DEBUG_REG_2_FULLMASK     0x0000ffff
/** EXT DEBUG REG 2 full read/write mask. */
#define XI3CPSX_EXT_DEBUG_REG_2_FULLRWMASK  0x00000000
/** EXT DEBUG REG 2 default value. */
#define XI3CPSX_EXT_DEBUG_REG_2_DEFVAL   0x0

/* access_type: ro */
/** EXT DEBUG REG 2 DEBUG PORT 47 32 bit shift. */
#define XI3CPSX_EXT_DEBUG_REG_2_DEBUG_PORT_47_32_SHIFT   0
/** EXT DEBUG REG 2 DEBUG PORT 47 32 bit width. */
#define XI3CPSX_EXT_DEBUG_REG_2_DEBUG_PORT_47_32_WIDTH   16
/** EXT DEBUG REG 2 DEBUG PORT 47 32 bit mask. */
#define XI3CPSX_EXT_DEBUG_REG_2_DEBUG_PORT_47_32_MASK    0x0000ffff
/** EXT DEBUG REG 2 DEBUG PORT 47 32 default value. */
#define XI3CPSX_EXT_DEBUG_REG_2_DEBUG_PORT_47_32_DEFVAL  0x0

#ifdef __cplusplus
}
#endif

#endif /* XI3CPSX_HW_H_ */
/** @} */
