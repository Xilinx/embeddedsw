/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c_hw.h
* @addtogroup Overview
* @{
*
* This file contains low level access functions using the base address
* directly without an instance.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.00  gm  01/25/24 First release
* 1.1   gm  10/07/24 Added macros for Version Register bit fields.
* 1.2   gm  02/18/24 Added slave mode support
*
* </pre>
*
******************************************************************************/
#ifndef XI3C_HW_H_		/**< prevent circular inclusions */
#define XI3C_HW_H_		/**< by using protection macros */

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
 * AXI_I3C0 Base Address
#define XI3C_BASEADDR      0x80000000
 */

#define XI3C_BASEADDR      0x0
#define XI3c_In32  Xil_In32
#define XI3c_Out32 Xil_Out32

/**
 * Register offsets for the XI3c device.
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
/* @} */


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
#define  XI3C_SR_ERROR_TYPE_CE3_MASK		0x00000400	/**< BIT 10 - This field will be set if there
								is no START coming from the new Controller
								that has took over the Role.   */
#define  XI3C_SR_RETURN_ROLE_REQ_ACK_MASK	0x00000800	/**< BIT 11 - Received ACK on controller role request back */
#define  XI3C_SR_RD_FIFO_ALMOST_FULL_MASK	0x00001000	/**< BIT 12 - Read Fifo almost Full */
#define  XI3C_SR_CMD_FIFO_NOT_EMPTY_MASK	0x00002000	/**< BIT 13 - CMD FIFO empty */
#define  XI3C_SR_WR_FIFO_NOT_EMPTY_MASK		0x00004000	/**< BIT 14 - Write FIFO empty */
#define  XI3C_SR_RD_FIFO_NOT_EMPTY_MASK		0x00008000	/**< BIT 15 - Read FIFO empty */
#define  XI3C_SR_SLV_DYNC_ADDR_DONE_MASK	0x00080000	/**< BIT 19 - Dynamic address assigned to slave */

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

/**
 * @name response and other mask(s)
 * @{
 */
#define  XI3C_RESP_ID_MASK		0x0000000F
#define  XI3C_RESP_RW_MASK		0x00000010
#define  XI3C_RESP_CODE_MASK		0x000001E0
#define  XI3C_RESP_BYTES_MASK		0x001FFE00

#define  XI3C_SLV_RESP_CCC_MASK		0x1FE00000
#define  XI3C_SLV_RESP_7E_FRAME_MASK	0x20000000
#define  XI3C_MRL_MASK			0x0FFF0000
#define  XI3C_GRP_ADDR_MASK		0x0000FF00

/**
 * @name response and other shift(s)
 * @{
 */
#define  XI3C_RESP_TID_SHIFT		0
#define  XI3C_RESP_RW_SHIFT		4
#define  XI3C_RESP_CODE_SHIFT		5
#define  XI3C_RESP_BYTES_SHIFT		9
#define  XI3C_RESP_LVL_SHIFT		16

#define  XI3C_SLV_RESP_CCC_SHIFT	21
#define  XI3C_SLV_RESP_7E_FRAME_SHIFT	29

#define  XI3C_CMD_LVL_SHIFT			16
#define  XI3C_MRL_SHIFT				16
#define  XI3C_MWL_MRL_MSB_SHIFT			8
#define  XI3C_GRP_ADDR_SHIFT			8
#define  XI3C_GETSTATUS_FORMAT2_SHIFT		16
#define  XI3C_GETMXDS_FORMAT3_DATA_SHIFT	16
#define  XI3C_CAPS4_SHIFT			24
#define  XI3C_CAPS3_SHIFT			16
#define  XI3C_CAPS2_SHIFT			8

/**
 * @name bit masks
 * @{
 */

#define	XI3C_1BIT_MASK		0x00000001
#define	XI3C_2BITS_MASK		0x00000003
#define	XI3C_3BITS_MASK		0x00000007
#define	XI3C_4BITS_MASK		0x0000000F
#define	XI3C_5BITS_MASK		0x0000001F
#define	XI3C_6BITS_MASK		0x0000003F
#define	XI3C_7BITS_MASK		0x0000007F
#define	XI3C_8BITS_MASK		0x000000FF
#define	XI3C_9BITS_MASK		0x000001FF
#define	XI3C_10BITS_MASK	0x000003FF
#define	XI3C_11BITS_MASK	0x000007FF
#define	XI3C_12BITS_MASK	0x00000FFF
#define	XI3C_13BITS_MASK	0x00001FFF
#define	XI3C_14BITS_MASK	0x00003FFF
#define	XI3C_15BITS_MASK	0x00007FFF
#define	XI3C_16BITS_MASK	0x0000FFFF
#define	XI3C_17BITS_MASK	0x0001FFFF
#define	XI3C_18BITS_MASK	0x0003FFFF
#define	XI3C_19BITS_MASK	0x0007FFFF
#define	XI3C_20BITS_MASK	0x000FFFFF

#define	XI3C_MSB_8BITS_MASK	0x0000FF00
#define	XI3C_MSB_16BITS_MASK	0xFFFF0000
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
#define  XI3C_INTR_ERROR_TYPE_CE3_MASK		0x00000400	/**< BIT 10 - This field will be set if there
								is no START coming from the new Controller
								that has took over the Role.   */
#define  XI3C_INTR_RETURN_ROLE_REQ_ACK_MASK	0x00000800	/**< BIT 11 - Received ACK on controller role request back */
#define  XI3C_INTR_RD_FIFO_ALMOST_FULL_MASK	0x00001000	/**< BIT 12 - Read Fifo almost Full */
#define  XI3C_INTR_CMD_FIFO_NOT_EMPTY_MASK	0x00002000	/**< BIT 13 - CMD FIFO empty */
#define  XI3C_INTR_WR_FIFO_NOT_EMPTY_MASK	0x00004000	/**< BIT 14 - Write FIFO empty */
#define  XI3C_INTR_RD_FIFO_NOT_EMPTY_MASK	0x00008000	/**< BIT 15 - Read FIFO empty */

/****************************************************************************/
/**
* Read an I3C register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XI3c_ReadReg(u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XI3c_ReadReg(BaseAddress, RegOffset) \
	XI3c_In32((BaseAddress) + (u32)(RegOffset))

/***************************************************************************/
/**
* Write an I3C register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note	C-Style signature:
*	void XI3c_WriteReg(u32 BaseAddress, int RegOffset, u32 RegisterValue)
*
******************************************************************************/
#define XI3c_WriteReg(BaseAddress, RegOffset, RegisterValue) 		\
	XI3c_Out32((BaseAddress) + (u32)(RegOffset), (u32)(RegisterValue))

/****************************************************************************/
/**
*
* Read WR FIFO LEVEL.
*
* @param        InstancePtr is the instance of I3C
*
* @return       None.
*
* @note         C-Style signature:
*               void XI3c_WrFifoLevel(XI3cPsx *InstancePtr)
*
*****************************************************************************/
#define XI3c_WrFifoLevel(InstancePtr)					  \
	(u16)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		  \
			   XI3C_FIFO_LVL_STATUS_OFFSET) & XI3C_16BITS_MASK)

/*****************************************************************************/
/**
*
* Read CMD FIFO LEVEL
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_CmdFifoLevel(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_CmdFifoLevel(InstancePtr)					\
        (u16)((XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_FIFO_LVL_STATUS_OFFSET) &		\
	      XI3C_MSB_16BITS_MASK) >> XI3C_CMD_LVL_SHIFT)

/*****************************************************************************/
/**
*
* Read RD FIFO LEVEL
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_RdFifoLevel(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RdFifoLevel(InstancePtr)					\
        (u16)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			   XI3C_FIFO_LVL_STATUS_1_OFFSET) &		\
	      XI3C_16BITS_MASK)

/*****************************************************************************/
/**
*
* Read RESP FIFO LEVEL
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_RespFifoLevel(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RespFifoLevel(InstancePtr)					\
        (u16)((XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_FIFO_LVL_STATUS_1_OFFSET) &		\
                            XI3C_MSB_16BITS_MASK) >> XI3C_RESP_LVL_SHIFT)

/*****************************************************************************/
/**
*
* Check Read FIFO empty status
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       None.
*
* @note         C-style signature:
*               u32 XI3c_RxFifoNotEmpty(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RxFifoNotEmpty(InstancePtr)				\
        (u32)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_SR_OFFSET) &				\
                            XI3C_SR_RD_FIFO_NOT_EMPTY_MASK)


/*****************************************************************************/
/**
*
* Check Response FIFO empty status
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       None.
*
* @note         C-style signature:
*               u32 XI3c_RespFifoNotEmpty(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_RespFifoNotEmpty(InstancePtr)				\
        (u32)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			    XI3C_SR_OFFSET) &				\
                            XI3C_SR_RESP_NOT_EMPTY_MASK)

/*****************************************************************************/
/**
*
* Enable Raising edge interrupts
*
* @param        Base address of the XI3c core instance.
* @param        interrupt mask value.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_EnableREInterrupts(XI3c *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XI3c_EnableREInterrupts(BaseAddress, IntrMask) 			    \
	XI3c_WriteReg((BaseAddress), XI3C_INTR_RE_OFFSET,		    \
			((XI3c_ReadReg(BaseAddress, XI3C_INTR_RE_OFFSET)) | \
			(IntrMask)))

/*****************************************************************************/
/**
*
* Enable Faling edge interrupts
*
* @param        Base address of the XI3c core instance.
* @param        interrupt mask value.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_EnableFEInterrupts(XI3c *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XI3c_EnableFEInterrupts(BaseAddress, IntrMask) 			    \
	XI3c_WriteReg((BaseAddress), XI3C_INTR_FE_OFFSET,		    \
			((XI3c_ReadReg(BaseAddress, XI3C_INTR_FE_OFFSET)) | \
			(IntrMask)))

/*****************************************************************************/
/**
*
* Disable raising edge interrupts
*
* @param        Base address of the XI3c core instance.
* @param        interrupt mask value.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_DisableREInterrupts(XI3c *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XI3c_DisableREInterrupts(BaseAddress, IntrMask)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_RE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_RE_OFFSET))	\
		      & ~(IntrMask)))					\

/*****************************************************************************/
/**
*
* Disable faling edge interrupts
*
* @param        Base address of the XI3c core instance.
* @param        interrupt mask value.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_DisableFEInterrupts(XI3c *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XI3c_DisableFEInterrupts(BaseAddress, IntrMask)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_FE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_FE_OFFSET))	\
		      & ~(IntrMask)))					\

/*****************************************************************************/
/**
*
* Disable all raising edge interrupts
*
* @param        Base address of the XI3c core instance.
* @param        interrupt mask value.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_DisableAllREInterrupts(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_DisableAllREInterrupts(BaseAddress)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_RE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_RE_OFFSET))	\
		      & ~(XI3C_ALL_INTR_MASK)))				\

/*****************************************************************************/
/**
*
* Disable all faling edge interrupts
*
* @param        Base address of the XI3c core instance.
* @param        interrupt mask value.
*
* @return       None.
*
* @note         C-style signature:
*               u16 XI3c_DisableAllFEInterrupts(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_DisableAllFEInterrupts(BaseAddress)			\
	XI3c_WriteReg((BaseAddress), XI3C_INTR_FE_OFFSET,		\
		      ((XI3c_ReadReg(BaseAddress,XI3C_INTR_FE_OFFSET))	\
		      & ~(XI3C_ALL_INTR_MASK)))				\

/*****************************************************************************/
/**
*
* Fill Slave send byte count value.
*
* @param        InstancePtr is a pointer to the XI3c core instance.
* @param        Byte count value.
*
* @return       None.
*
* @note         C-style signature:
*               void XI3c_FillSlaveSendCount(XI3c *InstancePtr, u16 ByteCount)
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
* Clear the group address of target.
*
* @param        InstancePtr is a pointer to the XI3c core instance.
* @param        Byte count value.
*
* @return       None.
*
* @note         C-style signature:
*               void XI3c_FillSlaveSendCount(XI3c *InstancePtr, u16 ByteCount)
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
* Read Maximum write length
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       Maximum write length.
*
* @note         C-style signature:
*               u16 XI3c_GetMWL(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetMWL(InstancePtr)					\
        (u16)(XI3c_ReadReg(InstancePtr->Config.BaseAddress,		\
			   XI3C_MWL_MRL) &				\
			   XI3C_12BITS_MASK)

/*****************************************************************************/
/**
*
* Read Maximum read length
*
* @param        InstancePtr is a pointer to the XI3c core instance.
*
* @return       Maximum read length.
*
* @note         C-style signature:
*               u16 XI3c_GetMRL(XI3c *InstancePtr)
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
