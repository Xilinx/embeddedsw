/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XI3C_HW_H_		/* prevent circular inclusions */
#define XI3C_HW_H_

/*****************************************************************************/
/**
*
* @file xi3c_hw.h
*
* @brief Defines I3C hardware register offsets and access macros.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.00  gm  01/25/24 First release
* 1.1   gm  10/07/24 Added macros for Version Register bit fields.
* 1.2   gm  02/18/24 Added slave mode support
* 1.4   vn  07/30/26 Fix Doxygen warnings
* 1.4   tb  09/21/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3c_api I3C APIs
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

/**
 * @brief Defines the AXI_I3C0 base address.
 *
 * \#define XI3C_BASEADDR      0x80000000
 */

#define XI3C_BASEADDR      0x0
#define XI3c_In32  Xil_In32	/**< 32-bit register read. */
#define XI3c_Out32 Xil_Out32	/**< 32-bit register write. */

/**
 * @brief Defines the register offsets for the XI3c device.
 * @{
 */
#define XI3C_VERSION_OFFSET			0x00	/**< Version Register */
#define XI3C_RESET_OFFSET			0x04	/**< Soft Reset Register */
#define XI3C_CR_OFFSET				0x08	/**< Control Register */
#define XI3C_ADDRESS_OFFSET			0x0C	/**< Target Address Register */
#define XI3C_SR_OFFSET				0x10	/**< Status Register */
#define XI3C_INTR_STATUS_OFFSET			0x14	/**< Status Event Register */
#define XI3C_INTR_RE_OFFSET			0x18	/**< Status Event Enable(Rising Edge) Register */
#define XI3C_INTR_FE_OFFSET			0x1C	/**< Status Event Enable(Falling Edge) Register  */
#define XI3C_CMD_FIFO_OFFSET			0x20	/**< I3C Command FIFO Register */
#define XI3C_WR_FIFO_OFFSET			0x24	/**< I3C Write Data FIFO Register */
#define XI3C_RD_FIFO_OFFSET			0x28	/**< I3C Read Data FIFO Register */
#define XI3C_RESP_STATUS_FIFO_OFFSET		0x2C	/**< I3C Response status FIFO Register */
#define XI3C_FIFO_LVL_STATUS_OFFSET		0x30	/**< I3C CMD & WR FIFO LVL Register */
#define XI3C_FIFO_LVL_STATUS_1_OFFSET		0x34	/**< I3C RESP & RD FIFO LVL  Register */
#define XI3C_SCL_HIGH_TIME_OFFSET		0x38	/**< I3C SCL HIGH Register */
#define XI3C_SCL_LOW_TIME_OFFSET		0x3C	/**< I3C SCL LOW  Register */
#define XI3C_SDA_HOLD_TIME_OFFSET		0x40	/**< I3C SDA HOLD Register */
#define XI3C_BUS_IDLE_OFFSET			0x44	/**< I3C CONTROLLER BUS IDLE Register */
#define XI3C_TSU_START_OFFSET			0x48	/**< I3C START SETUP Register  */
#define XI3C_THD_START_OFFSET			0x4C	/**< I3C START HOLD Register */
#define XI3C_TSU_STOP_OFFSET			0x50	/**< I3C STOP Setup Register  */
#define XI3C_OD_SCL_HIGH_TIME_OFFSET		0x54	/**< I3C OD SCL HIGH Register */
#define XI3C_OD_SCL_LOW_TIME_OFFSET		0x58	/**< I3C OD SCL LOW  Register */
#define XI3C_TARGET_ADDR_BCR			0x60	/**< I3C Target dynamic Address and BCR Register */
#define XI3C_MWL_MRL				0x74	/**< Maximum Write and Max Read length */
#define XI3C_EVENT				0x78	/**< Target events */
#define XI3C_GETMXDS				0x80	/**< Target Device Max Data Speed */
#define XI3C_GETSTATUS				0x84	/**< Target Device current Status */
#define XI3C_GETCAPS_REG0			0x88	/**< Target Device Format 1 Capabilities */
#define XI3C_GETCAPS_REG1			0x8c	/**< Target Device Format 2 Capabilities */
#define XI3C_CON_RD_BYTE_COUNT			0x90	/**< Read byte count register */
/** @} */

/**
 * @name Version Register mask(s)
 * @{
 */

#define  XI3C_INTERNAL_REVISION_MASK		0x0000000F	/**< BITS 3:0   - Internal revision */
#define  XI3C_CORE_PATCH_REVISION_MASK		0x000000F0	/**< BITS 7:4   - Patch revision */
#define  XI3C_CORE_REVISION_NUM_MASK		0x0000FF00	/**< BITS 15:8  - Revision number */
#define  XI3C_CORE_VERSION_MINOR_MASK		0x00FF0000	/**< BITS 23:16 - Minor version */
#define  XI3C_CORE_VERSION_MAJOR_MASK		0xFF000000	/**< BITS 31:24 - Major version */

#define  XI3C_CORE_REVISION_NUM_SHIFT		8		/**< Revision number shift */

/** @} */

/**
 * @name Reset Register mask(s)
 * @{
 */

#define  XI3C_SOFT_RESET_MASK		0x00000001	/**< BIT 0 - Reset */
#define  XI3C_CMD_FIFO_RESET_MASK	0x00000002	/**< BIT 1 - Cmd fifo reset */
#define  XI3C_WR_FIFO_RESET_MASK	0x00000004	/**< BIT 2 - Write fifo reset  */
#define  XI3C_RD_FIFO_RESET_MASK	0x00000008	/**< BIT 3 - Read fifo reset */
#define  XI3C_RESP_FIFO_RESET_MASK	0x00000010	/**< BIT 4 - Response fifo reset */
#define  XI3C_ALL_FIFOS_RESET_MASK	0x0000001E	/**< BIT 1 to 4 - All fifos reset */

/** @} */

/**
 * @name Control Register  (CR) mask(s)
 * @{
 */

#define  XI3C_CR_EN_MASK		0x00000001   /**< BIT 0 - Core Enable */
#define  XI3C_CR_ABORT_MASK		0x00000002   /**< BIT 1 - Abort Transaction */
#define  XI3C_CR_RESUME_MASK		0x00000004   /**< BIT 2 - Resume Operation  */
#define  XI3C_CR_IBI_MASK		0x00000008   /**< BIT 3 - IBI Enable */
#define  XI3C_CR_HJ_MASK		0x00000010   /**< BIT 4 - Hot Join Enable */
#define  XI3C_CR_ACCEPT_CTRL_ROLE_REQ	0x00000020   /**< BIT 5 - Generate ACK for secondary controller role request IBI */

/** @} */

/**
 * @name Status Register  (SR) mask(s)
 * @{
 */

#define  XI3C_SR_BUS_BUSY_MASK			0x00000001	/**< BIT 0 - Bus Busy */
#define  XI3C_SR_CLK_STALL_MASK			0x00000002	/**< BIT 1 - Clock Stall */
#define  XI3C_SR_CMD_FULL_MASK			0x00000004	/**< BIT 2 - Cmd Fifo Full  */
#define  XI3C_SR_RESP_FULL_MASK			0x00000008	/**< BIT 3 - Resp Fifo Full */
#define  XI3C_SR_RESP_NOT_EMPTY_MASK		0x00000010	/**< BIT 4 - Resp Fifo not empty */
#define  XI3C_SR_WR_FULL_MASK			0x00000020	/**< BIT 5 - Write Fifo Full */
#define  XI3C_SR_RD_FULL_MASK			0x00000040	/**< BIT 6 - Read Fifo Full */
#define  XI3C_SR_IBI_MASK			0x00000080	/**< BIT 7 - IBI */
#define  XI3C_SR_HJ_MASK			0x00000100	/**< BIT 8 - Hot join */
#define  XI3C_SR_CTRL_ROLE_REQUEST_MASK		0x00000200	/**< BIT 9 - Received control role request */
/** BIT 10 - No START from the new controller after role takeover. */
#define  XI3C_SR_ERROR_TYPE_CE3_MASK		0x00000400
#define  XI3C_SR_RETURN_ROLE_REQ_ACK_MASK	0x00000800	/**< BIT 11 - Received ACK on controller role request back */
#define  XI3C_SR_RD_FIFO_ALMOST_FULL_MASK	0x00001000	/**< BIT 12 - Read Fifo almost Full */
#define  XI3C_SR_CMD_FIFO_NOT_EMPTY_MASK	0x00002000	/**< BIT 13 - CMD FIFO empty */
#define  XI3C_SR_WR_FIFO_NOT_EMPTY_MASK		0x00004000	/**< BIT 14 - Write FIFO empty */
#define  XI3C_SR_RD_FIFO_NOT_EMPTY_MASK		0x00008000	/**< BIT 15 - Read FIFO empty */
#define  XI3C_SR_SLV_DYNC_ADDR_DONE_MASK	0x00080000	/**< BIT 19 - Dynamic address assigned to slave */

/** @} */

/**
 * @name Status Register  (SR) Shifts(s)
 * @{
 */
#define  XI3C_SR_BUS_BUSY_SHIFT			0		/**< BIT 0 - Bus Busy */
#define  XI3C_SR_CLK_STALL_SHIFT		1		/**< BIT 1 - Clock Stall */
#define  XI3C_SR_CMD_FULL_SHIFT			2		/**< BIT 2 - Cmd Fifo Full  */
#define  XI3C_SR_RESP_FULL_SHIFT		3		/**< BIT 3 - Resp Fifo Full */
#define  XI3C_SR_RESP_NOT_EMPTY_SHIFT		4		/**< BIT 4 - Resp Fifo not empty */
#define  XI3C_SR_WR_FULL_SHIFT			5		/**< BIT 5 - Write Fifo Full */
#define  XI3C_SR_RD_FULL_SHIFT			6		/**< BIT 6 - Read Fifo Full */
#define  XI3C_SR_SLV_DYNC_ADDR_DONE_SHIFT	19		/**< BIT 19 - Dynamic address assigned to slave */

/** @} */

/**
 * @name response and other mask(s)
 * @{
 */
/** Response transaction ID mask. */
#define  XI3C_RESP_ID_MASK		0x0000000F
/** Response read/write bit mask. */
#define  XI3C_RESP_RW_MASK		0x00000010
/** Response error code mask. */
#define  XI3C_RESP_CODE_MASK		0x000001E0
/** Response byte count mask. */
#define  XI3C_RESP_BYTES_MASK		0x001FFE00

/** Slave CCC value mask. */
#define  XI3C_SLV_RESP_CCC_MASK		0x1FE00000
/** Broadcast 7E frame present mask. */
#define  XI3C_SLV_RESP_7E_FRAME_MASK	0x20000000
/** Maximum read length field mask. */
#define  XI3C_MRL_MASK			0x0FFF0000
/** Group address field mask. */
#define  XI3C_GRP_ADDR_MASK		0x0000FF00

/** @} */

/**
 * @name response and other shift(s)
 * @{
 */
/** Response transaction ID shift. */
#define  XI3C_RESP_TID_SHIFT		0
/** Response read/write shift. */
#define  XI3C_RESP_RW_SHIFT		4
/** Response error code shift. */
#define  XI3C_RESP_CODE_SHIFT		5
/** Response byte count shift. */
#define  XI3C_RESP_BYTES_SHIFT		9
/** Response FIFO level shift. */
#define  XI3C_RESP_LVL_SHIFT		16

#define  XI3C_SLV_RESP_CCC_SHIFT	21	/**< Slave CCC value shift. */
/** Broadcast 7E frame shift. */
#define  XI3C_SLV_RESP_7E_FRAME_SHIFT	29

/** Command FIFO level shift. */
#define  XI3C_CMD_LVL_SHIFT			16
/** Maximum read length shift. */
#define  XI3C_MRL_SHIFT				16
/** MWL/MRL high-byte shift. */
#define  XI3C_MWL_MRL_MSB_SHIFT			8
/** Group address shift. */
#define  XI3C_GRP_ADDR_SHIFT			8
/** GETSTATUS format-2 shift. */
#define  XI3C_GETSTATUS_FORMAT2_SHIFT		16
/** GETMXDS format-3 data shift. */
#define  XI3C_GETMXDS_FORMAT3_DATA_SHIFT	16
/** Capabilities byte 4 shift. */
#define  XI3C_CAPS4_SHIFT			24
/** Capabilities byte 3 shift. */
#define  XI3C_CAPS3_SHIFT			16
/** Capabilities byte 2 shift. */
#define  XI3C_CAPS2_SHIFT			8

/** @} */

/**
 * @name bit masks
 * @{
 */

#define	XI3C_1BIT_MASK		0x00000001	/**< 1-bit field mask. */
#define	XI3C_2BITS_MASK		0x00000003	/**< 2-bit field mask. */
#define	XI3C_3BITS_MASK		0x00000007	/**< 3-bit field mask. */
#define	XI3C_4BITS_MASK		0x0000000F	/**< 4-bit field mask. */
#define	XI3C_5BITS_MASK		0x0000001F	/**< 5-bit field mask. */
#define	XI3C_6BITS_MASK		0x0000003F	/**< 6-bit field mask. */
#define	XI3C_7BITS_MASK		0x0000007F	/**< 7-bit field mask. */
#define	XI3C_8BITS_MASK		0x000000FF	/**< 8-bit field mask. */
#define	XI3C_9BITS_MASK		0x000001FF	/**< 9-bit field mask. */
#define	XI3C_10BITS_MASK	0x000003FF	/**< 10-bit field mask. */
#define	XI3C_11BITS_MASK	0x000007FF	/**< 11-bit field mask. */
#define	XI3C_12BITS_MASK	0x00000FFF	/**< 12-bit field mask. */
#define	XI3C_13BITS_MASK	0x00001FFF	/**< 13-bit field mask. */
#define	XI3C_14BITS_MASK	0x00003FFF	/**< 14-bit field mask. */
#define	XI3C_15BITS_MASK	0x00007FFF	/**< 15-bit field mask. */
#define	XI3C_16BITS_MASK	0x0000FFFF	/**< 16-bit field mask. */
#define	XI3C_17BITS_MASK	0x0001FFFF	/**< 17-bit field mask. */
#define	XI3C_18BITS_MASK	0x0003FFFF	/**< 18-bit field mask. */
#define	XI3C_19BITS_MASK	0x0007FFFF	/**< 19-bit field mask. */
#define	XI3C_20BITS_MASK	0x000FFFFF	/**< 20-bit field mask. */

/** Most significant 8-bit field mask. */
#define	XI3C_MSB_8BITS_MASK	0x0000FF00
/** Most significant 16-bit field mask. */
#define	XI3C_MSB_16BITS_MASK	0xFFFF0000

/** @} */

/**
 * @name interrupt Register  (INTR) mask(s)
 * @{
 */

#define  XI3C_INTR_BUS_BUSY_MASK		0x00000001	/**< BIT 0 - Bus Busy */
#define  XI3C_INTR_CLK_STALL_MASK		0x00000002	/**< BIT 1 - Clock Stall */
#define  XI3C_INTR_CMD_FULL_MASK		0x00000004	/**< BIT 2 - Cmd Fifo Full  */
#define  XI3C_INTR_RESP_FULL_MASK		0x00000008	/**< BIT 3 - Resp Fifo Full */
#define  XI3C_INTR_RESP_NOT_EMPTY_MASK		0x00000010	/**< BIT 4 - Resp Fifo not empty */
#define  XI3C_INTR_WR_FIFO_ALMOST_FULL_MASK	0x00000020	/**< BIT 5 - Write Fifo Full */
#define  XI3C_INTR_RD_FULL_MASK			0x00000040	/**< BIT 6 - Read Fifo Full */
#define  XI3C_ALL_INTR_MASK			0x0000007F	/**< 6:0 BITS */
#define  XI3C_INTR_IBI_MASK			0x00000080	/**< BIT 7 - IBI */
#define  XI3C_INTR_HJ_MASK			0x00000100	/**< BIT 8 - Hot join */
#define  XI3C_INTR_CTRL_ROLE_REQUEST_MASK	0x00000200	/**< BIT 9 - Received control role request */
/** BIT 10 - No START from the new controller after role takeover. */
#define  XI3C_INTR_ERROR_TYPE_CE3_MASK		0x00000400
#define  XI3C_INTR_RETURN_ROLE_REQ_ACK_MASK	0x00000800	/**< BIT 11 - Received ACK on controller role request back */
#define  XI3C_INTR_RD_FIFO_ALMOST_FULL_MASK	0x00001000	/**< BIT 12 - Read Fifo almost Full */
#define  XI3C_INTR_CMD_FIFO_NOT_EMPTY_MASK	0x00002000	/**< BIT 13 - CMD FIFO empty */
#define  XI3C_INTR_WR_FIFO_NOT_EMPTY_MASK	0x00004000	/**< BIT 14 - Write FIFO empty */
#define  XI3C_INTR_RD_FIFO_NOT_EMPTY_MASK	0x00008000	/**< BIT 15 - Read FIFO empty */

/** @} */

/****************************************************************************/
/**
* @brief Reads an I3C register.
*
* @param[in]	BaseAddress The base address of the device.
* @param[in]	RegOffset The offset from the 1st register of the
*		device to select the specific register.
*
* @return
* 		The value read from the register.
*
* @note
* C-style signature
*	u32 XI3c_ReadReg(u32 BaseAddress, int RegOffset)
*
******************************************************************************/
#define XI3c_ReadReg(BaseAddress, RegOffset) \
	XI3c_In32((BaseAddress) + (u32)(RegOffset))

/***************************************************************************/
/**
* @brief Writes an I3C register.
*
* @param[in]	BaseAddress The base address of the device.
* @param[in]	RegOffset The offset from the 1st register of the
*		device to select the specific register.
* @param[in]	RegisterValue The value to be written to the register.
*
* @note
* C-style signature
*	void XI3c_WriteReg(u32 BaseAddress, int RegOffset, u32 RegisterValue)
*
******************************************************************************/
#define XI3c_WriteReg(BaseAddress, RegOffset, RegisterValue) 		\
	XI3c_Out32((BaseAddress) + (u32)(RegOffset), (u32)(RegisterValue))

/****************************************************************************/
/**
*
* @brief Reads the write FIFO level.
*
* @param[in]	InstancePtr Pointer to the XI3c instance.
*
* @return
*		The write FIFO level.
*
* @note
* C-style signature
*	u16 XI3c_WrFifoLevel(XI3c *InstancePtr)
*
*****************************************************************************/
#define XI3c_WrFifoLevel(InstancePtr)					  \
	(u16)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		  \
			   XI3C_FIFO_LVL_STATUS_OFFSET) & XI3C_16BITS_MASK)

/*****************************************************************************/
/**
*
* @brief Reads the command FIFO level.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
*		The command FIFO level.
*
* @note
* C-style signature
*	u16 XI3c_CmdFifoLevel(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_CmdFifoLevel(InstancePtr)					\
        (u16)((XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_FIFO_LVL_STATUS_OFFSET) &		\
	      XI3C_MSB_16BITS_MASK) >> XI3C_CMD_LVL_SHIFT)

/*****************************************************************************/
/**
*
* @brief Reads the read FIFO level.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
*		The read FIFO level.
*
* @note
* C-style signature
*	u16 XI3c_RdFifoLevel(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RdFifoLevel(InstancePtr)					\
        (u16)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			   XI3C_FIFO_LVL_STATUS_1_OFFSET) &		\
	      XI3C_16BITS_MASK)

/*****************************************************************************/
/**
*
* @brief Reads the response FIFO level.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
*		The response FIFO level.
*
* @note
* C-style signature
*	u16 XI3c_RespFifoLevel(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RespFifoLevel(InstancePtr)					\
        (u16)((XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_FIFO_LVL_STATUS_1_OFFSET) &		\
                            XI3C_MSB_16BITS_MASK) >> XI3C_RESP_LVL_SHIFT)

/*****************************************************************************/
/**
*
* @brief Checks whether the read FIFO is not empty.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
*		- Nonzero if the read FIFO is not empty.
*		- 0 otherwise.
*
* @note
* C-style signature
*	u32 XI3c_RxFifoNotEmpty(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RxFifoNotEmpty(InstancePtr)				\
        (u32)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_SR_OFFSET) &				\
                            XI3C_SR_RD_FIFO_NOT_EMPTY_MASK)

/*****************************************************************************/
/**
*
* @brief Checks whether the response FIFO is not empty.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
*		- Nonzero if the response FIFO is not empty.
*		- 0 otherwise.
*
* @note
* C-style signature
*	u32 XI3c_RespFifoNotEmpty(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RespFifoNotEmpty(InstancePtr)				\
        (u32)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_SR_OFFSET) &				\
                            XI3C_SR_RESP_NOT_EMPTY_MASK)

/*****************************************************************************/
/**
*
* @brief Enables rising-edge interrupts.
*
* @param[in]	BaseAddress The base address of the XI3c core instance.
* @param[in]	IntrMask The interrupt mask value.
*
* @note
* C-style signature
*	void XI3c_EnableREInterrupts(u32 BaseAddress, u32 IntrMask)
*
******************************************************************************/
#define XI3c_EnableREInterrupts(BaseAddress, IntrMask) 			    \
	XI3c_WriteReg((BaseAddress), XI3C_INTR_RE_OFFSET,		    \
			((XI3c_ReadReg(BaseAddress, XI3C_INTR_RE_OFFSET)) | \
			(IntrMask)))

/*****************************************************************************/
/**
*
* @brief Enables falling-edge interrupts.
*
* @param[in]	BaseAddress The base address of the XI3c core instance.
* @param[in]	IntrMask The interrupt mask value.
*
* @note
* C-style signature
*	void XI3c_EnableFEInterrupts(u32 BaseAddress, u32 IntrMask)
*
******************************************************************************/
#define XI3c_EnableFEInterrupts(BaseAddress, IntrMask) 			    \
	XI3c_WriteReg((BaseAddress), XI3C_INTR_FE_OFFSET,		    \
			((XI3c_ReadReg(BaseAddress, XI3C_INTR_FE_OFFSET)) | \
			(IntrMask)))

/*****************************************************************************/
/**
*
* @brief Disables rising-edge interrupts.
*
* @param[in]	BaseAddress The base address of the XI3c core instance.
* @param[in]	IntrMask The interrupt mask value.
*
* @note
* C-style signature
*	void XI3c_DisableREInterrupts(u32 BaseAddress, u32 IntrMask)
*
******************************************************************************/
#define XI3c_DisableREInterrupts(BaseAddress, IntrMask)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_RE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_RE_OFFSET))	\
		      & ~(IntrMask)))					\

/*****************************************************************************/
/**
*
* @brief Disables falling-edge interrupts.
*
* @param[in]	BaseAddress The base address of the XI3c core instance.
* @param[in]	IntrMask The interrupt mask value.
*
* @note
* C-style signature
*	void XI3c_DisableFEInterrupts(u32 BaseAddress, u32 IntrMask)
*
******************************************************************************/
#define XI3c_DisableFEInterrupts(BaseAddress, IntrMask)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_FE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_FE_OFFSET))	\
		      & ~(IntrMask)))					\

/*****************************************************************************/
/**
*
* @brief Disables all rising-edge interrupts.
*
* @param[in]	BaseAddress The base address of the XI3c core instance.
*
* @note
* C-style signature
*	void XI3c_DisableAllREInterrupts(u32 BaseAddress)
*
******************************************************************************/
#define XI3c_DisableAllREInterrupts(BaseAddress)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_RE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_RE_OFFSET))	\
		      & ~(XI3C_ALL_INTR_MASK)))				\

/*****************************************************************************/
/**
*
* @brief Disables all falling-edge interrupts.
*
* @param[in]	BaseAddress The base address of the XI3c core instance.
*
* @note
* C-style signature
*	void XI3c_DisableAllFEInterrupts(u32 BaseAddress)
*
******************************************************************************/
#define XI3c_DisableAllFEInterrupts(BaseAddress)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_FE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_FE_OFFSET))	\
		      & ~(XI3C_ALL_INTR_MASK)))				\

/*****************************************************************************/
/**
*
* @brief Fills Slave send byte count value.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
* @param[in]	ByteCount The byte count value.
*
* @note
* C-style signature
*	void XI3c_FillSlaveSendCount(XI3c *InstancePtr, u16 ByteCount)
*
******************************************************************************/
#define XI3c_FillSlaveSendCount(InstancePtr, ByteCount) 		      \
	XI3c_WriteReg((InstancePtr->Config.BaseAddress),		      \
		      XI3C_CON_RD_BYTE_COUNT,				      \
		      (((XI3c_ReadReg(InstancePtr->Config.BaseAddress,	      \
				      XI3C_CON_RD_BYTE_COUNT))    	      \
                      & (~XI3C_16BITS_MASK)) | (ByteCount & XI3C_16BITS_MASK)))

/*****************************************************************************/
/**
*
* @brief Clears the group address of target.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @note
* C-style signature
*	void XI3c_ClearGrpAddr(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_ClearGrpAddr(InstancePtr) 		      \
	XI3c_WriteReg((InstancePtr->Config.BaseAddress),		      \
		      XI3C_ADDRESS_OFFSET,				      \
		      ((XI3c_ReadReg(InstancePtr->Config.BaseAddress,	      \
				     XI3C_ADDRESS_OFFSET))        	      \
                      & (~XI3C_GRP_ADDR_MASK)))

/*****************************************************************************/
/**
*
* @brief Reads Maximum write length.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
* 		Maximum write length.
*
* @note
* C-style signature
*	u16 XI3c_GetMWL(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetMWL(InstancePtr)					\
        (u16)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			   XI3C_MWL_MRL) &				\
			   XI3C_12BITS_MASK)

/*****************************************************************************/
/**
*
* @brief Reads Maximum read length.
*
* @param[in]	InstancePtr Pointer to the XI3c core instance.
*
* @return
* 		Maximum read length.
*
* @note
* C-style signature
*	u16 XI3c_GetMRL(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetMRL(InstancePtr)					\
        (u16)((XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			   XI3C_MWL_MRL) >> XI3C_MRL_SHIFT) &		\
			   XI3C_12BITS_MASK)
#ifdef __cplusplus
}
#endif

#endif /* XI3C_HW_H_ */
/** @} */
