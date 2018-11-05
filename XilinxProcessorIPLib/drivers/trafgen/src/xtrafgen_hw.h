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
* @file xtrafgen_hw.h
* @addtogroup trafgen_v4_2
* @{
*
* This header file contains identifiers and macros that can be used to access
* the Axi Traffic Generator device. The driver APIs/functions are defined in
* xtrafgen.h.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a srt  1/12/13  First release
* 1.01a adk  03/09/13 Updated Driver to Support Static and Streaming Mode
* 2.00a adk  16/09/13 Fixed CR:737291
* 2.01a adk  21/10/13 Fixed CR:740522 Updated the MasterRam offset as per latest 
*		      IP.This driver is valid only for IP(v2.0) onwards. The 
*		      XTG_MASTER_RAM_OFFSET has been changed from 
*		      0x10000 to 0xc000.
* 2.01a adk  15/11/13 Fixed CR:760808 Added Mask for the New bit field added
*		      (XTG_MCNTL_LOOPEN_MASK).
* 3.1   adk  28/04/14 Fixed CR:782131 Incorrect Mask value for the loopenable
*		      bit.
*
* </pre>

******************************************************************************/
#ifndef XTRAFGEN_HW_H		/* prevent circular inclusions */
#define XTRAFGEN_HW_H		/* by using protection macros */

/***************************** Include Files *********************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/*
 * Register offset definitions. Unless otherwise noted, register access is
 * 32 bit.
 */
/** @name Device registers
 *  @{
 */
#define XTG_MCNTL_OFFSET	0x00	/**< Master Control */
#define XTG_SCNTL_OFFSET	0x04	/**< Slave Control */
#define XTG_ERR_STS_OFFSET	0x08	/**< Error Status  */
#define XTG_ERR_EN_OFFSET	0x0C	/**< Error Enable */
#define XTG_MSTERR_INTR_OFFSET	0x10	/**< Master Err Interrupt Enable */
#define XTG_CFG_STS_OFFSET	0x14	/**< Config Status */
#define XTG_STREAM_CNTL_OFFSET	0x30	/**< Streaming Control */
#define XTG_STREAM_CFG_OFFSET	0x34    /**< Streaming Config */
#define XTG_STREAM_TL_OFFSET	0x38    /**< Streaming Transfer Length */
 
 /** 
  *  Static Mode Register Descrptions 
  */
 #define XTG_STATIC_CNTL_OFFSET	0x60	/**< Static Control */
 #define XTG_STATIC_LEN_OFFSET	0x64	/**< Static Length */

/*@}*/

/** @name Internal RAM Offsets
 * @{
 */
#define XTG_PARAM_RAM_OFFSET	0x1000	/**< Parameter RAM Offset */
#define XTG_COMMAND_RAM_OFFSET	0x8000	/**< Command RAM Offset */
#define XTG_MASTER_RAM_OFFSET	0xC000	/**< Master RAM Offset */
#define XTG_COMMAND_RAM_MSB_OFFSET	0xa000	/**< Command RAM MSB Offset */

/*@}*/

/** @name Master Control Register bit definitions.
 *  These bits are associated with the XTG_MCNTL_OFFSET register.
 * @{
 */
#define XTG_MCNTL_REV_MASK	0xFF000000	/**< Core Revision Mask */
#define XTG_MCNTL_MSTID_MASK	0x00E00000	/**< M_ID_WIDTH Mask */
#define XTG_MCNTL_MSTEN_MASK	0x00100000	/**< Master Logic Enable
						Mask */
#define XTG_MCNTL_LOOPEN_MASK   0x00080000	/**< Loop enable Mask */

#define XTG_MCNTL_REV_SHIFT	24		/**< Core Rev shift */
#define XTG_MCNTL_MSTID_SHIFT	21		/**< M_ID_WIDTH shift */

/*@}*/

/** @name Slave Control Register bit definitions.
 *  These bits are associated with the XTG_SCNTL_OFFSET register.
 * @{
 */
#define XTG_SCNTL_BLKRD_MASK	0x00080000	/**< Enable  
						Block Read */	
#define XTG_SCNTL_DISEXCL_MASK	0x00040000	/**< Disable  
						Exclusive Access */	
#define XTG_SCNTL_WORDR_MASK	0x00020000	/**< Write Response  
						Order Enable */	
#define XTG_SCNTL_RORDR_MASK	0x00010000	/**< Read Response  
						Order Enable */	
#define XTG_SCNTL_ERREN_MASK	0x00008000	/**< Slv Error 
						Interrupt Enable */	

/*@}*/

/** @name Error bitmasks
 *  These bits are shared with the XTG_ERR_STS_OFFSET and 
 *  XTG_ERR_EN_OFFSET register.
 * @{
 */
#define XTG_ERR_ALL_MSTERR_MASK	0x001F0000	/**< Master
						Errors Mask */
#define XTG_ERR_ALL_SLVERR_MASK	0x00000003	/**< Slave
						Errors Mask */
#define XTG_ERR_ALL_ERR_MASK	0x001F0003	/**< All
						Errors Mask */

#define XTG_ERR_MSTCMP_MASK	0x80000000	/**< Master
						Complete Mask */
#define XTG_ERR_RIDER_MASK	0x00100000 	/**< Master 
						Invalid 
						RVALID Mask */
#define XTG_ERR_WIDER_MASK	0x00080000	/**< Master
						Invalid
						BVALID Mask */
#define XTG_ERR_WRSPER_MASK	0x00040000	/**< MW
						Invalid
						RESP Mask */
#define XTG_ERR_RERRSP_MASK	0x00020000	/**< MR
						Invalid
						RESP Mask */
#define XTG_ERR_RLENER_MASK	0x00010000	/**< Master
						Read Length 
						Mask */
#define XTG_ERR_SWSTRB_MASK	0x00000002	/**< Slave
						WSTRB Illegal
						Mask */
#define XTG_ERR_SWLENER_MASK	0x00000001	/**< Slave
						Read Length
						Mask */

/*@}*/

/*@}*/

/** @name Master Error Interrupt Enable Register bit definitions.
 *  These bits are associated with the XTG_MSTERR_INTR_OFFSET register.
 * @{
 */
#define XTG_MSTERR_INTR_MINTREN_MASK	0x00008000	/**< Master Err 
							 Interrupt Enable */

/*@}*/

/** @name Config Status Register bit definitions.
 *  These bits are associated with the XTG_CFG_STS_OFFSET register.
 * @{
 */
#define XTG_CFG_STS_MWIDTH_SHIFT	28		/**< Master Width Shift */
#define XTG_CFG_STS_MWIDTH_MASK		0x70000000	/**< Master Width Mask */

#define XTG_CFG_STS_SWIDTH_SHIFT	25		/**< Slave Width Shift */
#define XTG_CFG_STS_SWIDTH_MASK		0x0E000000	/**< Slave Width Mask */

#define XTG_CFG_STS_MFULL_MASK		0x01000000	/**< Full Mode */
#define XTG_CFG_STS_MBASIC_MASK		0x00800000	/**< Basic Mode */

/*@}*/

/*@}*/
/** @name Streaming Control Register bit definitions.
 *  These bits are associated with the XTG_STR_CFG_OFFSET register.
 * @{
 */
#define XTG_STREAM_CNTL_VER_SHIFT		24		/**< Version Shift */
#define XTG_STREAM_CNTL_VER_MASK		0xFF000000	/**< Version Mask */

#define XTG_STREAM_CNTL_TD_SHIFT		1		/**< Transfer Done Shift */
#define XTG_STREAM_CNTL_TD_MASK			0x00000002	/**< Transfer Done Mask */
 
#define XTG_STREAM_CNTL_STEN_MASK		0x00000001	/**< Streaming Enable Mask */
#define XTG_STREAM_CNTL_RESET_MASK		0x00000000	/**< Streaming Disable Mask */
/*@}*/

/*@}*/
/** @name Streaming Config Register bit definitions.
 *  These bits are associated with the XTG_STR_CFG_OFFSET register.
 * @{
 */
 #define XTG_STREAM_CFG_PDLY_SHIFT		16	 	/**< Programmable Delay Shift */
 #define XTG_STREAM_CFG_PDLY_MASK		0xFFFF0000	/**< Programmable Delay Mask */
 
 #define XTG_STREAM_CFG_TDEST_SHIFT		8		/**< TDEST PORT Shift */
 #define XTG_STREAM_CFG_TDEST_MASK		0x0000FF00	/**< TDEST PORT Mask */
 
 #define XTG_STREAM_CFG_RANDLY_SHIFT		1		/**< Random Delay Shift */
 #define XTG_STREAM_CFG_RANDLY_MASK		0x00000002	/**< Random Delay Mask */
 
 #define XTG_STREAM_CFG_RANDL_MASK		0x00000001	/**< Random Length Mask */
 /*@}*/
 
 
 /*@}*/
/** @name Streaming Transfer Length Register bit definitions.
 *  These bits are associated with the XTG_STR_TL_OFFSET register.
 * @{
 */
 #define XTG_STREAM_TL_TCNT_SHIFT		16	 	/**< Transfer Count Shift */
 #define XTG_STREAM_TL_TCNT_MASK		0xFFFF0000	/**< Transfer Count Mask */
 
 #define XTG_STREAM_TL_TLEN_MASK		0x0000FFFF	/**< Transfer Length Mask */
 /*@}*/
 
  /*@}*/
/** @name Static Control Register bit definitions.
 *  These bits are associated with the XTG_STATIC_CNTL_OFFSET register.
 * @{
 */
#define XTG_STATIC_CNTL_VER_SHIFT		24		/**< Version Shift */
#define XTG_STATIC_CNTL_VER_MASK		0xFF000000	/**< Version Mask */

#define XTG_STATIC_CNTL_TD_SHIFT		1		/**< Transfer Done Shift */
#define XTG_STATIC_CNTL_TD_MASK			0x00000002	/**< Transfer Done Mask */
 
#define XTG_STATIC_CNTL_STEN_MASK		0x00000001	/**< Static enable Mask */
#define XTG_STATIC_CNTL_RESET_MASK		0x00000000	/**< Static Disable Mask */
 /*@}*/
 
  /*@}*/
/** @name Static Length Register bit definitions.
 *  These bits are associated with the XTG_STATIC_LEN_OFFSET register.
 * @{
 */
 #define XTG_STATIC_LEN_BLEN_MASK		0x000000FF	 /**< Burst length Mask */
 /*@}*/

/** @name Axi Traffic Generator Command Entry field mask/shifts
 *  @{
 */
#define XTG_ADDR_MASK		0xFFFFFFFF	/**< Driven to a*_addr line */
#define XTG_LEN_MASK		0xFF		/**< Driven to a*_len line  */
#define XTG_LOCK_MASK		0x1		/**< Driven to a*_lock line */
#define XTG_BURST_MASK		0x3		/**< Driven to a*_burst line */
#define XTG_SIZE_MASK		0x7		/**< Driven to a*_size line */
#define XTG_ID_MASK		0x3F		/**< Driven to a*_id line */
#define XTG_PROT_MASK		0x7		/**< Driven to a*_prot line */
#define XTG_LAST_ADDR_MASK	0x7		/**< Last address */
#define XTG_VALID_CMD_MASK	0x1		/**< Valid Command */	
#define XTG_MSTRAM_INDEX_MASK	0x1FFF		/**< Master RAM Index */
#define XTG_OTHER_DEPEND_MASK	0x1FF		/**< Other depend Command no */
#define XTG_MY_DEPEND_MASK	0x1FF		/**< My depend command no */
#define XTG_QOS_MASK		0xF		/**< Driven to a*_qos line */
#define XTG_USER_MASK		0xFF		/**< Driven to a*_user line */
#define XTG_CACHE_MASK		0xF		/**< Driven to a*_cache line */
#define XTG_EXPECTED_RESP_MASK	0x7		/**< Expected response */

#define XTG_ADDR_SHIFT		0		/**< Driven to a*_addr line */
#define XTG_LEN_SHIFT		0		/**< Driven to a*_len line  */
#define XTG_LOCK_SHIFT		8		/**< Driven to a*_lock line */
#define XTG_BURST_SHIFT		10		/**< Driven to a*_burst line */
#define XTG_SIZE_SHIFT		12		/**< Driven to a*_size line */
#define XTG_ID_SHIFT		15		/**< Driven to a*_id line */
#define XTG_PROT_SHIFT		21		/**< Driven to a*_prot line */
#define XTG_LAST_ADDR_SHIFT	28		/**< Last address */
#define XTG_VALID_CMD_SHIFT	31		/**< Valid Command */
#define XTG_MSTRAM_INDEX_SHIFT	0		/**< Master RAM Index */
#define XTG_OTHER_DEPEND_SHIFT	13		/**< Other depend cmd num */
#define XTG_MY_DEPEND_SHIFT	22		/**< My depend cmd num */
#define XTG_QOS_SHIFT		16		/**< Driven to a*_qos line */
#define XTG_USER_SHIFT		8		/**< Driven to a*_user line */
#define XTG_CACHE_SHIFT		4		/**< Driven to a*_cache line */
#define XTG_EXPECTED_RESP_SHIFT	0		/**< Expected response */

/*@}*/


/** @name Axi Traffic Generator Parameter Entry field mask/shifts
 *  @{
 */
/* Parameter Entry field shift values */
#define XTG_PARAM_ADDRMODE_SHIFT	24	/**< Address mode */
#define XTG_PARAM_INTERVALMODE_SHIFT	26	/**< Interval mode */
#define XTG_PARAM_IDMODE_SHIFT		28	/**< Id mode */
#define XTG_PARAM_OP_SHIFT		29	/**< Opcode */

/* PARAM RAM Opcode shift values */
#define XTG_PARAM_COUNT_SHIFT		0	/**< Repeat/Delay count */
#define XTG_PARAM_DELAYRANGE_SHIFT	0	/**< Delay Range */
#define XTG_PARAM_DELAY_SHIFT		8	/**< FIXED RPT Delay count */
#define XTG_PARAM_ADDRRANGE_SHIFT	20	/**< Address Range */

/* Parameter Entry field mask values */
#define XTG_PARAM_ADDRMODE_MASK		0x3	/**< Address mode */	
#define XTG_PARAM_INTERVALMODE_MASK	0x3	/**< Interval mode */
#define XTG_PARAM_IDMODE_MASK		0x1	/**< Id mode */
#define XTG_PARAM_OP_MASK		0x7	/**< Opcode */

/* PARAM RAM Opcode mask values */
#define XTG_PARAM_COUNT_MASK		0xFFFFFF/**< Repeat/Delay count */
#define XTG_PARAM_DELAYRANGE_MASK	0xFF	/**< Delay Range */ 
#define XTG_PARAM_DELAY_MASK		0xFFF	/**< FIXED RPT Delay count */
#define XTG_PARAM_ADDRRANGE_MASK	0xF	/**< Address Range */

/* PARAM RAM Opcode values */
#define XTG_PARAM_OP_NOP		0	/**< NOP mode */
#define XTG_PARAM_OP_RPT		1	/**< Repeat mode */
#define XTG_PARAM_OP_DELAY		2 	/**< Delay mode */
#define XTG_PARAM_OP_FIXEDRPT		3	/**< Fixed Repeat Delay */ 

/* PARAM RAM Address mode values */
#define XTG_PARAM_OP_ADDRMODE_CONST	0	/**< Constant Addr mode */
#define XTG_PARAM_OP_ADDRMODE_INCR	1	/**< Increment Addr mode */
#define XTG_PARAM_OP_ADDRMODE_RAND	2 	/**< Random Addr mode */

/* PARAM RAM Interval mode values */
#define XTG_PARAMOP_INTERVALMODE_CONST	0	/**< Constant Interval mode */	
#define XTG_PARAMOP_INTERVALMODE_RAND	1	/**< Random Interval mode */

/*@}*/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* XTrafGen_ReadReg returns the value read from the register specified by
* <i>RegOffset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	RegOffset is the offset of the register to be read.
*
* @return	Returns the 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XTrafGen_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XTrafGen_ReadReg(BaseAddress, RegOffset) \
	(Xil_In32(((BaseAddress) + (RegOffset))))

/****************************************************************************/
/**
*
* XTrafGen_WriteReg, writes <i>Data</i> to the register specified by
* <i>RegOffset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	RegOffset is the offset of the register to be written.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note
* 	C-style signature:
*	void XTrafGen_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XTrafGen_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32(((BaseAddress) + (RegOffset)), (Data))

/****************************************************************************/
/**
*
* XTrafGen_ReadParamRam returns the value read from the Parameter RAM
* specified by <i>Offset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	Offset is the offset of the Parameter RAM to be read.
*
* @return	Returns the 32-bit value of the memory location.
*
* @note		C-style signature:
*		u32 XTrafGen_ReadParamRam(u32 BaseAddress, u32 Offset)
*
*****************************************************************************/
#define XTrafGen_ReadParamRam(BaseAddress, Offset)	\
	(Xil_In32(((BaseAddress) + XTG_PARAM_RAM_OFFSET + (Offset))))

/****************************************************************************/
/**
*
* XTrafGen_WriteParamRam, writes <i>Data</i> to the Parameter RAM 
* specified by <i>Offset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	Offset is the offset of the location in Parameter RAM
*		to be written.
* @param	Data is the 32-bit value to write to the Parameter RAM.
*
* @return	None.
*
* @note
* 	C-style signature:
*	void XTrafGen_WriteParamRam(u32 BaseAddress, u32 Offset, u32 Data)
*
*****************************************************************************/
#define XTrafGen_WriteParamRam(BaseAddress, Offset, Data) \
	Xil_Out32(((BaseAddress) + XTG_PARAM_RAM_OFFSET + (Offset)), (Data))

/****************************************************************************/
/**
*
* XTrafGen_ReadCmdRam returns the value read from the Command RAM
* specified by <i>Offset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	Offset is the offset of the Command RAM to be read.
*
* @return	Returns the 32-bit value of the memory location.
*
* @note		C-style signature:
*		u32 XTrafGen_ReadCmdRam(u32 BaseAddress, u32 Offset)
*
*****************************************************************************/
#define XTrafGen_ReadCmdRam(BaseAddress, Offset) \
	(Xil_In32(((BaseAddress) + XTG_COMMAND_RAM_OFFSET + (Offset))))

/****************************************************************************/
/**
*
* XTrafGen_ReadCmdRam_Msb returns the value read from the Command RAM
* specified by <i>Offset</i>.
*
* @param	BaseAddress is the MSB of base address of the Axi TrafGen device.
* @param	Offset is the offset of the Command RAM to be read.
*
* @return	Returns the 32-bit value of the memory location.
*
* @note		C-style signature:
*		u32 XTrafGen_ReadCmdRam_Msb(u32 BaseAddress, u32 Offset)
*
*****************************************************************************/
#define XTrafGen_ReadCmdRam_Msb(BaseAddress, Offset) \
	(Xil_In32(((BaseAddress) + XTG_COMMAND_RAM_MSB_OFFSET + (Offset))))
/****************************************************************************/
/**
*
* XTrafGen_WriteCmdRam, writes <i>Data</i> to the Command RAM specified by
* <i>Offset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	Offset is the offset of the location in Command RAM
*		to be written.
* @param	Data is the 32-bit value to write to the Command RAM.
*
* @return	None.
*
* @note
* 	C-style signature:
*	void XTrafGen_WriteCmdRam(u32 BaseAddress, u32 Offset, u32 Data)
*
*****************************************************************************/
#define XTrafGen_WriteCmdRam(BaseAddress, Offset, Data) \
	Xil_Out32(((BaseAddress) + XTG_COMMAND_RAM_OFFSET + (Offset)), (Data))
/****************************************************************************/
/**
*
* XTrafGen_WriteCmdRam_Msb, writes <i>Data</i> to the Command RAM specified by
* <i>Offset</i>.
*
* @param	BaseAddress is the MSB of base address of the Axi TrafGen device.
* @param	Offset is the offset of the location in Command RAM
*		to be written.
* @param	Data is the 32-bit value to write to the Command RAM.
*
* @return	None.
*
* @note
* 	C-style signature:
*	void XTrafGen_WriteCmdRam_Msb(u32 BaseAddress, u32 Offset, u32 Data)
*
*****************************************************************************/
#define XTrafGen_WriteCmdRam_Msb(BaseAddress, Offset, Data) \
	Xil_Out32(((BaseAddress) + XTG_COMMAND_RAM_MSB_OFFSET + (Offset)), (Data))

/****************************************************************************/
/**
*
* XTrafGen_ReadMasterRam returns the value read from the Master RAM
* specified by <i>Offset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	Offset is the offset of the Master RAM to be read.
*
* @return	Returns the 32-bit value of the memory location.
*
* @note		C-style signature:
*		u32 XTrafGen_ReadMasterRam(u32 BaseAddress, u32 Offset)
*
*****************************************************************************/
#define XTrafGen_ReadMasterRam(BaseAddress, Offset)	\
	(Xil_In32(((BaseAddress) + XTG_MASTER_RAM_OFFSET + (Offset))))

/****************************************************************************/
/**
*
* XTrafGen_WriteMasterRam, writes <i>Data</i> to the Master RAM specified
* by <i>Offset</i>.
*
* @param	BaseAddress is the base address of the Axi TrafGen device.
* @param	Offset is the offset of the location in Master RAM
*		to be written.
* @param	Data is the 32-bit value to write to the Master RAM.
*
* @return	None.
*
* @note
* 	C-style signature:
*	void XTrafGen_WriteMasterRam(u32 BaseAddress, u32 Offset, u32 Data)
*
*****************************************************************************/
#define XTrafGen_WriteMasterRam(BaseAddress, Offset, Data) \
	Xil_Out32(((BaseAddress) + XTG_MASTER_RAM_OFFSET + (Offset)), (Data))

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
