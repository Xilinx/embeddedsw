/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xusb_l.h
* @addtogroup usb_v5_4
* @{
*
* This header file contains identifiers and low-level driver function prototypes
* (or macros) that can be used to access the USB device. High-level driver
* function prototypes are defined in xusb.h.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a hvm  2/22/07  First release
* 2.00a hvm  12/2/08  Updated the register offset values as per the new USB
*			device datasheet. Added new bitmasks related to DMA
*			operation as defined in the datasheet.
* 3.00a hvm 10/28/09 Updated the new USB error register related constants
* 3.00a hvm  11/18/09 Updated to use HAL processor APIs.
*			XUsb_mReadReg is renamed to XUsb_ReadReg and
*			XUsb_mWriteReg is renamed to XUsb_WriteReg.
*			Updated the new USB error register related constants.
* 4.00a hvm 10/21/10   Updated the ULPI PHY and interrupt register related
*			constants.
* 4.01a hvm 08/23/11   Updated with High bandwidth Isochronous bit definitions
*
* 4.03a bss  06/20/10 Added SIE Reset Mask (CR 660602)
* 4.04a bss  10/22/13 Added macros for HSIC PHY registers.
* </pre>
*
******************************************************************************/
#ifndef XUSB_L_H		/* prevent circular inclusions */
#define XUSB_L_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets for the USB device.
 * @{
 */
#define XUSB_EP0_CONFIG_OFFSET		0x0000 /**< EP0 Config Reg Offset */
#define XUSB_EP1_CONFIG_OFFSET		0x4000 /**< EP1 Config Reg Offset */
#define XUSB_SETUP_PKT_ADDR_OFFSET	0x0080 /**< Setup Packet Address */
#define XUSB_EP0_RAMBASE_OFFSET		0x0088 /**< EP0 RAM buffer Register */
#define XUSB_ADDRESS_OFFSET		0x0100 /**< Address Register */
#define XUSB_CONTROL_OFFSET		0x0104 /**< Control Register */
#define XUSB_STATUS_OFFSET		0x0108 /**< Status Register */
#define XUSB_FRAMENUM_OFFSET 		0x010C	/**< Frame Number Register */
#define XUSB_IER_OFFSET 		0x0110	/**< Interrupt Enable Reg */
#define XUSB_BUFFREADY_OFFSET		0x0114	/**< Buffer Ready Register */
#define XUSB_TESTMODE_OFFSET		0x0118	/**< Test Mode Register */
#define XUSB_ECR_OFFSET			0x011C  /**< USB Error Count Reg */
#define XUSB_UPAR_OFFSET		0x0120  /**< USB ULPI PHY Access Reg */
#define XUSB_DMA_RESET_OFFSET		0x0200  /**< DMA Soft Reset Register */
#define XUSB_DMA_CONTROL_OFFSET		0x0204	/**< DMA Control Register */
#define XUSB_DMA_DSAR_ADDR_OFFSET	0x0208	/**< DMA source Address Reg */
#define XUSB_DMA_DSAR_ADDR_OFFSET_LSB   0x0308  /**< DMA source Address Reg for LSB*/
#define XUSB_DMA_DSAR_ADDR_OFFSET_MSB   0x030C  /**< DMA source Address Reg for MSB*/
#define XUSB_DMA_DDAR_ADDR_OFFSET	0x020C	/**< DMA destination Addr Reg */
#define XUSB_DMA_DDAR_ADDR_OFFSET_LSB   0x0310  /**< DMA destination Addr Reg LSB*/
#define XUSB_DMA_DDAR_ADDR_OFFSET_MSB   0x0314  /**< DMA destination Addr Reg MSB*/
#define XUSB_DMA_LENGTH_OFFSET		0x0210	/**< DMA Length Register */
#define XUSB_DMA_STATUS_OFFSET		0x0214	/**< DMA Status Register */

/* @} */

/** @name Address Register
 *
 * @{
 */
#define XUSB_ADDRESS_ADDR_MASK		0x7F	/**< Address Mask  */

/* @} */

/** @name Control Register
 *
 * @{
 */
#define XUSB_CONTROL_USB_READY_MASK 	0x80000000 /**< USB ready Mask */
#define XUSB_REMOTE_WAKEUP_MASK		0x40000000 /**< Remote Wakeup Mask */
#define XUSB_CONTROL_SIE_RESET_MASK	0x20000000 /**< Soft Reset Mask */
/* @} */

/** @name Status Register and Interrupt Enable Register
 *
 * <b> Status Register  </b>
 *
 * This register holds the status flags for the USB device.
 *
 * <b> Interrupt Enable Register  </b>
 *
 * This register is used to enable interrupt sources for the USB device.
 * Writing a '1' to a bit in this register enables the corresponding Interrupt.
 * Writing a '0' to a bit in this register disables the corresponding Interrupt.
 *
 * Both these registers have the same bit definitions and are only defined
 * once.
 * @{
 */
#define XUSB_STATUS_GLOBAL_INTR_MASK	0x80000000 /**< Global Intr Enable */
#define XUSB_STATUS_PHY_ACCESS_MASK	0x40000000 /**< PHY access complete */
#define XUSB_STATUS_BITSTUFF_ERR_MASK   0x20000000 /**< USB Bit stuff error */
#define XUSB_STATUS_PID_ERR_MASK	0x10000000 /**< USB PID error */
#define XUSB_STATUS_CRC_ERR_MASK	0x08000000 /**< USB CRC error */
#define XUSB_STATUS_DMA_DONE_MASK	0x04000000 /**< DMA done  */
#define XUSB_STATUS_DMA_ERROR_MASK	0x02000000 /**< DMA error */
#define XUSB_STATUS_RESUME_MASK		0x01000000 /**< USB resume Mask */
#define XUSB_STATUS_RESET_MASK		0x00800000 /**< USB Reset Mask */
#define XUSB_STATUS_SUSPEND_MASK	0x00400000 /**< USB Suspend Mask */
#define XUSB_STATUS_DISCONNECT_MASK	0x00200000 /**< USB Disconnect Mask */
#define XUSB_STATUS_FIFO_BUFF_RDY_MASK	0x00100000 /**< FIFO Buff Ready Mask */
#define XUSB_STATUS_FIFO_BUFF_FREE_MASK	0x00080000 /**< FIFO Buff Free Mask */
#define XUSB_STATUS_SETUP_PACKET_MASK	0x00040000 /**< Setup packet received */
#define XUSB_STATUS_SOF_PACKET_MASK	0x00020000 /**< SOF packet received */
#define XUSB_STATUS_HIGH_SPEED_MASK	0x00010000 /**< USB Speed Mask */
#define XUSB_STATUS_EP7_BUFF2_COMP_MASK	0x00008000 /**< EP 7 Buff 2 Processed */
#define XUSB_STATUS_EP6_BUFF2_COMP_MASK	0x00004000 /**< EP 6 Buff 2 Processed */
#define XUSB_STATUS_EP5_BUFF2_COMP_MASK	0x00002000 /**< EP 5 Buff 2 Processed */
#define XUSB_STATUS_EP4_BUFF2_COMP_MASK	0x00001000 /**< EP 4 Buff 2 Processed */
#define XUSB_STATUS_EP3_BUFF2_COMP_MASK	0x00000800 /**< EP 3 Buff 2 Processed */
#define XUSB_STATUS_EP2_BUFF2_COMP_MASK	0x00000400 /**< EP 2 Buff 2 Processed */
#define XUSB_STATUS_EP1_BUFF2_COMP_MASK	0x00000200 /**< EP 1 Buff 2 Processed */
#define XUSB_STATUS_EP7_BUFF1_COMP_MASK	0x00000080 /**< EP 7 Buff 1 Processed */
#define XUSB_STATUS_EP6_BUFF1_COMP_MASK	0x00000040 /**< EP 6 Buff 1 Processed */
#define XUSB_STATUS_EP5_BUFF1_COMP_MASK	0x00000020 /**< EP 5 Buff 1 Processed */
#define XUSB_STATUS_EP4_BUFF1_COMP_MASK	0x00000010 /**< EP 4 Buff 1 Processed */
#define XUSB_STATUS_EP3_BUFF1_COMP_MASK	0x00000008 /**< EP 3 Buff 1 Processed */
#define XUSB_STATUS_EP2_BUFF1_COMP_MASK	0x00000004 /**< EP 2 Buff 1 Processed */
#define XUSB_STATUS_EP1_BUFF1_COMP_MASK	0x00000002 /**< EP 1 Buff 1 Processed */
#define XUSB_STATUS_EP0_BUFF1_COMP_MASK	0x00000001 /**< EP 0 Buff 1 Processed */

#define XUSB_STATUS_EP_BUFF2_SHIFT	8	   /**< EP buffer offset */
/* @} */

/** @name Frame Number Register
 *
 * @{
 */
#define XUSB_FRAMENUM_FRAME_MASK	0x00007FF8 /**< Frame Number Mask  */
#define XUSB_FRAMENUM_MICRO_FRAME_MASK	0x00000007 /**< Micro Frame Mask */
#define XUSB_FRAMENUM_FRAME_SHIFT	0x3	   /**< Frame Number Shift */

/* @} */

/** @name Buffer Ready Register
 *
 * @{
 */
#define XUSB_BUFFREADY_EP7_BUFF2_MASK	0x00008000  /**< EP 7 Buff 2 Ready */
#define XUSB_BUFFREADY_EP6_BUFF2_MASK	0x00004000  /**< EP 6 Buff 2 Ready */
#define XUSB_BUFFREADY_EP5_BUFF2_MASK	0x00002000  /**< EP 5 Buff 2 Ready */
#define XUSB_BUFFREADY_EP4_BUFF2_MASK	0x00001000  /**< EP 4 Buff 2 Ready */
#define XUSB_BUFFREADY_EP3_BUFF2_MASK	0x00000800  /**< EP 3 Buff 2 Ready */
#define XUSB_BUFFREADY_EP2_BUFF2_MASK	0x00000400  /**< EP 2 Buff 2 Ready */
#define XUSB_BUFFREADY_EP1_BUFF2_MASK	0x00000200  /**< EP 1 Buff 2 Ready */
#define XUSB_BUFFREADY_EP7_BUFF1_MASK	0x00000080  /**< EP 7 Buff 1 Ready */
#define XUSB_BUFFREADY_EP6_BUFF1_MASK	0x00000040  /**< EP 6 Buff 1 Ready */
#define XUSB_BUFFREADY_EP5_BUFF1_MASK	0x00000020  /**< EP 5 Buff 1 Ready */
#define XUSB_BUFFREADY_EP4_BUFF1_MASK	0x00000010  /**< EP 4 Buff 1 Ready */
#define XUSB_BUFFREADY_EP3_BUFF1_MASK	0x00000008  /**< EP 3 Buff 1 Ready */
#define XUSB_BUFFREADY_EP2_BUFF1_MASK	0x00000004  /**< EP 2 Buff 1 Ready */
#define XUSB_BUFFREADY_EP1_BUFF1_MASK	0x00000002  /**< EP 1 Buff 1 Ready */
#define XUSB_BUFFREADY_EP0_BUFF_MASK	0x00000001  /**< EP 0 Buffer Ready */

#define XUSB_STATUS_INTR_EVENT_MASK	(XUSB_STATUS_HIGH_SPEED_MASK | \
					XUSB_STATUS_RESET_MASK | \
					XUSB_STATUS_SUSPEND_MASK | \
					XUSB_STATUS_DISCONNECT_MASK | \
					XUSB_STATUS_SOF_PACKET_MASK|\
					XUSB_STATUS_RESUME_MASK)
#define XUSB_STATUS_INTR_BUFF_COMP_ALL_MASK  \
					(XUSB_STATUS_EP7_BUFF2_COMP_MASK | \
					XUSB_STATUS_EP6_BUFF2_COMP_MASK |  \
					XUSB_STATUS_EP5_BUFF2_COMP_MASK |  \
					XUSB_STATUS_EP4_BUFF2_COMP_MASK |  \
					XUSB_STATUS_EP3_BUFF2_COMP_MASK |  \
					XUSB_STATUS_EP2_BUFF2_COMP_MASK |  \
					XUSB_STATUS_EP1_BUFF2_COMP_MASK |  \
					XUSB_STATUS_EP7_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP6_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP5_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP4_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP3_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP2_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP1_BUFF1_COMP_MASK |  \
					XUSB_STATUS_EP0_BUFF1_COMP_MASK )
#define XUSB_STATUS_DMA_EVENT_MASK	(XUSB_STATUS_DMA_DONE_MASK | \
					XUSB_STATUS_DMA_ERROR_MASK)

#define XUSB_STATUS_ERROR_EVENT_MASK	(XUSB_STATUS_BITSTUFF_ERR_MASK |   \
					XUSB_STATUS_PID_ERR_MASK |        \
					XUSB_STATUS_CRC_ERR_MASK)

#define XUSB_STATUS_INTR_ALL_MASK	0xFFFFFEFF /**< All Interrupts Mask */


/* @} */

/** @name Test Mode Register
 *
 * @{
 */
#define XUSB_TESTMODE_J_MASK		0x00000001 /**< Test Mode J */
#define XUSB_TESTMODE_K_MASK            0x00000002 /**< Test Mode K */
#define XUSB_TESTMODE_SE0_NAK_MASK	0x00000003 /**< Test Mode SE0 NAK */
#define XUSB_TESTMODE_TEST_PKT_MASK	0x00000004 /**< Test Packet Mode */

/* @} */

/** @name Error Counter Register
 *
 * @{
 */
#define XUSB_ECR_BITSTUFF_ERRCNT_MASK	0xFF000000 /**< Bit Stuff error counter */
#define XUSB_ECR_PID_ERRCNT_MASK	0x00FF0000 /**< PID error counter */
#define XUSB_ECR_CRC_ERRCNT_MASK	0x0000FF00 /**< CRC error counter */
#define XUSB_ECR_BITSTUFF_ERRCNT_SHIFT	24
#define XUSB_ECR_PID_ERRCNT_SHIFT	16
#define XUSB_ECR_CRC_ERRCNT_SHIFT	8

/* @} */

/** @name ULPI PHY Access Register
 *
 * @{
 */
#define XUSB_UPAR_REG_DATA_MASK		0x0000FF00 /**< PHY Data register */
#define XUSB_UPAR_REG_ADDR_MASK		0x0000003F /**< PHY Address register */
#define XUSB_UPAR_READ_WRITE_MASK	0x00000040 /**< PHY Read/Write
							Transaction selection
							bit */
#define XUSB_UPAR_BUSY_MASK	0x80000000 /**< PHY access enable */

#define XUSB_UPAR_REG_DATA_SHIFT	8
/* @} */

/** @name Endpoint Configuration Space offsets
 *
 * @{
 */
#define XUSB_EP_CFGSTATUS_OFFSET	0x00	/**< Endpoint Config Status  */
#define XUSB_EP_BUF0COUNT_OFFSET	0x08	/**< Buffer 0 Count */
#define XUSB_EP_BUF1COUNT_OFFSET	0x0C	/**< Buffer 1 Count */

/* @} */

/** @name Endpoint Configuration Status Register
 *
 * @{
 */
#define XUSB_EP_CFG_VALID_MASK		0x80000000 /**< Endpoint Valid bit */
#define XUSB_EP_CFG_STALL_MASK		0x40000000 /**< Endpoint Stall bit */
#define XUSB_EP_CFG_OUT_IN_MASK		0x20000000 /**< Endpoint OUT/IN cfg */
#define XUSB_EP_CFG_ISO_MASK		0x10000000 /**< Endpoint ISO config */
#define XUSB_EP_CFG_DATA_TOGGLE_MASK	0x08000000 /**< Endpoint Data toggle */
#define XUSB_EP_CFG_BUF_SELECT_MASK	0x04000000 /**< Endpoint Buff Select */
#define XUSB_EP_CFG_PACKET_SIZE_MASK	0x03ff8000 /**< Endpoint Packet Size */
#define XUBS_EP_CFG_ISOTRANS_MASK	0x00006000 /**< Isochronous Transfer */
#define XUSB_EP_CFG_BASE_MASK		0x00001fff /**< Endpoint Base Mask */

#define XUSB_EP_CFG_VALID_SHIFT		31	/**< Endpoint Valid bit Shift */
#define XUSB_EP_CFG_STALL_SHIFT		30	/**< Endpoint Stall bit Shift */
#define XUSB_EP_CFG_OUT_IN_SHIFT	29	/**< Endpoint OUT/IN config */
#define XUSB_EP_CFG_ISO_SHIFT		28	/**< Endpoint ISO config Shft */
#define XUSB_EP_CFG_DATA_TOGGLE_SHIFT	27	/**< Endpoint Data toggle  */
#define XUSB_EP_CFG_BUF_SELECT_SHIFT	26	/**< Endpoint Buf Select Shft */
#define XUSB_EP_CFG_PACKET_SIZE_SHIFT	15	/**< Endpoint Pkt Size Shift */
#define XUSB_EP_CFG_ISOTRANS_SHIFT	13	/**< Endpoint Isotrans Shift*/

/* @} */

/** @name DMA Reset/Control and Status register bits
 *
 * @{
 */
#define XUSB_DMA_RESET_VALUE		0x0000000A /**< DMA Reset value */
#define XUSB_DMA_WRITE_TO_DPRAM		0	/**< DPRAM is the destination
							address for DMA transfer
							*/
#define XUSB_DMA_READ_FROM_DPRAM	0x80000000/**< DPRAM is the source
							address for DMA transfer
							*/
#define XUSB_DMA_DMASR_BUSY		0x80000000 /**< DMA busy*/
#define XUSB_DMA_DMASR_ERROR		0x40000000 /**< DMA Error */
#define XUSB_DMA_DMACR_DIR_MASK		0x80000000 /**< DMA direction */

/* @} */

/*
 * When this bit is set, the DMA buffer ready bit is set by hardware upon
 * DMA transfer completion.
 */
#define XUSB_DMA_BRR_CTRL		0x40000000 /**< DMA bufready ctrl bit */

/** @name HSIC PHY Registers
 *
 *  User need not program HSIC PHY registers for normal operation. These
 *  are provided to test the PHY access only.
 *
 * @{
 */
#define XUSB_HSIC_CONTROL_REGISTER	0x28	/**< HSIC Control Register */
#define XUSB_HSIC_STATUS_REGISTER	0x29	/**< HSIC Status Register */
#define XUSB_HSIC_STROBE_REGISTER	0x2A	/**< HSIC Strobe Register */
#define XUSB_HSIC_DATA_REGISTER		0x2B	/**< HSIC Data Register */

#define XUSB_HSIC_CR_SOFTRESET_MASK		0x00000001 /**< PHY Soft reset
							      mask */
#define XUSB_HSIC_CR_CONNECT_MASK		0x00000002 /**< PHY issues
							   connect mask */
#define XUSB_HSIC_CR_REMOTEWKP_ENABLE_MASK	0x00000004 /**< Enable Remote
					   		  Wake-up signal
					   		  forwarding to HSIC */

#define XUSB_HSIC_SR_LINKSTATUS_MASK		0x00000003 /**< Link Status */
#define XUSB_HSIC_SR_VBUSSTATE_MASK		0x00000004 /**< VBUS State */
#define XUSB_HSIC_SR_CONNECT_MASK		0x00000008 /**< Connect
							  signaling issued from
							  device to host */
#define XUSB_HSIC_SR_CDERR_MASK			0x00000010 /**< Carrier Detect
								Error */
#define XUSB_HSIC_SR_FRAMEERR_MASK		0x00000020 /**< Frame decoding
								Error */
#define XUSB_HSIC_SR_TXFIFO_UNDERRUN_MASK	0x00000040 /**< PHY Transmit
							   	FIFO underrun
							   	mask */
#define XUSB_HSIC_SR_ELFIFO_OVERRUN_MASK	0x00000080 /**< Elasticity FIFO
								overrun mask */

#define XUSB_HSIC_STROBE_IDELAY_MASK		0x0000001F /**< IDELAY value
								for HSIC strobe
								input for
								receive */
#define XUSB_HSIC_STROBE_READY_MASK		0x00000020 /**< IDELAY block
								ready mask */

#define XUSB_HSIC_DATA_IDELAY_MASK		0x0000001F /**< IDELAY value
								for HSIC data
								input for
								receive */
#define XUSB_HSIC_DATA_READY_MASK		0x00000020 /**< IDELAY block
								ready mask */

/* @} */

/*
 * Define the appropriate I/O access method to memory mapped I/O or
 * other interface if necessary
 */

#define XUsb_In32  Xil_In32
#define XUsb_Out32 Xil_Out32

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
* Read from the specified USB device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XUsb_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XUsb_ReadReg(BaseAddress, RegOffset) \
	XUsb_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write to the specified USB device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XUsb_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 RegisterValue);
******************************************************************************/
#define XUsb_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XUsb_Out32((BaseAddress) + (RegOffset), (RegisterValue))

#ifdef __cplusplus
}
#endif

#endif /* XUSB_L_H */
/** @} */
