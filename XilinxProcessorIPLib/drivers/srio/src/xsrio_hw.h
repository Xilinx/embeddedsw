/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
* @file xsrio_hw.h
* @addtogroup srio_v1_1
* @{
*
* This header file contains identifiers and macros that can be used to access
* the Axi srio gen2 device. The driver APIs/functions are defined in
* xsrio.h.
*
* @note
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------- 
* 1.0   adk  16/04/14 Initial release.
* 
******************************************************************************/

#ifndef XSRIO_HW_H /* prevent circular inclusions */
#define XSRIO_HW_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_io.h"

/*
 * Register offset definitions. Unless otherwise noted, register access is
 * 32 bit.
 */
/** @name Device registers
 *  @{
 */
 
/** 
 *  Capability Address Register Space 0x00-0x3C Registers
 */
#define XSRIO_DEV_ID_CAR_OFFSET     0x00    /**< Device Identity CAR */
#define XSRIO_DEV_INFO_CAR_OFFSET   0x04    /**< Device Information CAR */
#define XSRIO_ASM_ID_CAR_OFFSET     0x08    /**< Assembly Identity CAR */
#define XSRIO_ASM_INFO_CAR_OFFSET   0x0C    /**< Assembly Information CAR */
#define XSRIO_PEF_CAR_OFFSET	    0x10    /**< Processing Element 
					     * Features CAR 
					     */
#define XSRIO_SWP_INFO_CAR_OFFSET   0x14    /**< Switch Port Information CAR */
#define XSRIO_SRC_OPS_CAR_OFFSET    0x18    /**< Source operations CAR */
#define XSRIO_DST_OPS_CAR_OFFSET    0x1c    /**< Destination operations CAR */

/**
 *  Command and Status Register Space 0x040-0xFC Registers
 */
#define XSRIO_PELL_CTRL_CSR_OFFSET	  0x4c    /**< PE Logical layer 
						   * Control CSR 
                                                   */
#define XSRIO_LCS0_BASEADDR_CSR_OFFSET	  0x58    /**< Local Configuration
					   	   * Space 0 Base Address CSR 
					    	   */
#define XSRIO_LCS1_BASEADDR_CSR_OFFSET	  0x5c    /**< Local Configuration 
						   * Space 1 Base Address CSR 
						   */
#define XSRIO_BASE_DID_CSR_OFFSET	  0x60    /**< Base Device ID CSR */
#define XSRIO_HOST_DID_LOCK_CSR_OFFSET    0x68    /**< Host Base Device ID
						   * Lock CSR 
						   */
#define XSRIO_COMPONENT_TAG_CSR_OFFSET   0x6c    /**< Component Tag CSR */

/**
 *  Extended Feature Register Space 0x0100-0xFFFC Registers
 */
#define XSRIO_EFB_HEADER_OFFSET           0x100   /**< Extended features LP
						   * Serial Register Block Header 
						   */
#define XSRIO_PORT_LINK_TOUT_CSR_OFFSET   0x120   /**< Port Link Timeout CSR */
#define XSRIO_PORT_RESP_TOUT_CSR_OFFSET   0x124   /**< Port Response Timeout
						   * CSR
						   */
#define XSRIO_PORT_GEN_CTL_CSR_OFFSET     0x13c   /**< General Control CSR */
#define XSRIO_PORT_N_MNT_REQ_CSR_OFFSET   0x140   /**< Port n Link Maintenance 
						   * Request CSR 
						   */
#define XSRIO_PORT_N_MNT_RES_CSR_OFFSET   0x144   /**< Port n Maintenance 
						   * Response CSR 
						   */
#define XSRIO_PORT_N_ACKID_CSR_OFFSET     0x148   /**< Port n Local Ack ID CSR */
#define XSRIO_PORT_N_ERR_STS_CSR_OFFSET   0x158   /**< Port n Error and 
						   * Status CSR 
						   */
#define XSRIO_PORT_N_CTL_CSR_OFFSET       0x15c   /**< Port n Control CSR */
#define XSRIO_EFB_LPSL_OFFSET		  0x0400  /**< LP-Serial Lane Extended 
						   * Features offset 
						   */
#define XSRIO_SL_HEADER_OFFSET            0x00    /**< Serial Lane Block Header */
#define XSRIO_SLS0_CSR_OFFSET(n)	  (0x10 + n*0x20)   
                                         	  /**< Serial Lane N 
						   * Status 0 CSR 
						   */   
#define XSRIO_SLS1_CSR_OFFSET(n)	  (0x14 + n*0x20) 
						  /**< Serial Lane N 
						   * Status 1 CSR 
						   */
/**
 * Implementation Defined Space 0x010000 - 0xFFFFFC Registers
 */
#define XSRIO_IMP_WCSR_OFFSET           0x10000  /**< Water Mark CSR */
#define XSRIO_IMP_BCSR_OFFSET		0x10004  /**< Buffer Control CSR */
#define XSRIO_IMP_MRIR_OFFSET 		0x10100  /**< Maintenance Request
						  * Information Register 
						  */

/*@}*/

/** @name Device Identity CAR bit definitions.
 *  These bits are associated with the XSRIO_DEV_ID_CAR_OFFSET register.
 * @{
 */
#define XSRIO_DEV_ID_DEVID_CAR_MASK  0xFFFF0000  /**< Device ID Mask */
#define XSRIO_DEV_ID_VDRID_CAR_MASK  0x0000FFFF  /**< Device Vendor ID Mask */

#define XSRIO_DEV_ID_DEVID_CAR_SHIFT 16	     /**< Device ID shift */
/*@}*/

/** @name Device Information CAR bit definitions.
 *  These bits are associated with the XSRIO_DEV_INFO_CAR_OFFSET register.
 * @{
 */
#define XSRIO_DEV_INFO_CAR_PATCH_MASK     0x0000000F  /**< Patch Mask */
#define XSRIO_DEV_INFO_CAR_MINREV_MASK    0x000000F0  /**< Minor Revision Mask */
#define XSRIO_DEV_INFO_CAR_MAJREV_MASK    0x00000F00  /**< Major Revision Mask */
#define XSRIO_DEV_INFO_CAR_DEVREV_MASK    0x000F0000  /**< Device Revision 
						       * Lable Mask
						       */
/*@}*/

/** @name Assembly Identity CAR  bit definitions.
 *  These bits are associated with the XSRIO_ASM_ID_CAR_OFFSET register.
 * @{
 */
#define XSRIO_ASM_ID_CAR_ASMID_MASK  0xFFFF0000 /**< Assembly ID Mask */
#define XSRIO_ASM_ID_CAR_ASMVID_MASK 0x0000FFFF /**< Assembly Vendor ID Mask */

#define XSRIO_ASM_ID_CAR_ASMID_SHIFT  16        /**< Assembly ID Shift */
/*@}*/

/** @name Assembly Device Information CAR  bit definitions.
 *  These bits are associated with the XSRIO_ASM_INFO_CAR_OFFSET register.
 * @{
 */
#define XSRIO_ASM_INFO_CAR_ASMREV_MASK  0xFFFF0000  /**< Assembly Revision 
						     * Mask
						     */
#define XSRIO_ASM_INFO_CAR_EFP_MASK     0x0000FFFF  /**< Extended Features
					             * Pointer Mask
					             */

#define XSRIO_ASM_INFO_CAR_ASMREV_SHIFT 16  	 /**< Assembly Revision Shift */
/*@}*/

/** @name Processing Element Features CAR  bit definitions.
 *  These bits are associated with the XSRIO_PEF_CAR_OFFSET register.
 * @{
 */
#define XSRIO_PEF_CAR_EAS_MASK        0x00000007 /**< Extended Addressing
					         * Support Mask 
					         */
#define XSRIO_PEF_CAR_EF_MASK        0x00000008 /**< Extended Features Mask */
#define XSRIO_PEF_CAR_CTS_MASK       0x00000010 /**< Common Transport Large
					         * System support Mask
					         */
#define XSRIO_PEF_CAR_CRF_MASK       0x00000020 /**< CRF Support Mask */
#define XSRIO_PEF_CAR_MPORT_MASK     0x08000000 /**< Multi Port Mask */
#define XSRIO_PEF_CAR_SWITCH_MASK    0x10000000 /**< Switch Mask */
#define XSRIO_PEF_CAR_PROCESSOR_MASK 0x20000000 /**< Processor Mask */
#define XSRIO_PEF_CAR_MEMORY_MASK    0x40000000 /**< Memory Mask */
#define XSRIO_PEF_CAR_BRIDGE_MASK    0x80000000 /**< Bridge Mask */
/*@}*/

/** @name Source Operations CAR  bit definitions.
 *  These bits are associated with the XSRIO_SRC_OPS_CAR_OFFSET 
 *  register and XSRIO_DST_OPS_CAR register.
 * @{
 */
#define XSRIO_SRCDST_OPS_CAR_PORT_WRITE_MASK      0x00000004 /**< Port write 
						              * operation Mask
						              */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_SWP_MASK      0x00000008 /**< Atomic Swap 
							      * Mask 
							      */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_CLR_MASK      0x00000010 /**< Atomic Clear
							      * Mask
							      */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_SET_MASK      0x00000020 /**< Atomic Set
							      * Mask
							      */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_DECR_MASK      0x00000040 /**< Atomic 
						              * Decrement Mask
						              */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_INCR_MASK     0x00000080 /**< Atomic
						              * Increment Mask
						              */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_TSWP_MASK     0x00000100 /**< Atomic test
						              * and swap Mask 
						              */
#define XSRIO_SRCDST_OPS_CAR_ATOMIC_CSWP_MASK    0x00000200 /**< Atomic compare
						              * and Swap Mask
						              */
#define XSRIO_SRCDST_OPS_CAR_DOORBELL_MASK        0x00000400 /**< Doorbell Mask */
#define XSRIO_SRCDST_OPS_CAR_DATA_MSG_MASK        0x00000800 /**< Data Message
							      * Mask
							      */
#define XSRIO_SRCDST_OPS_CAR_WRITE_RESPONSE_MASK  0x00001000 /**< Write with
						              * Response Mask
						              */
#define XSRIO_SRCDST_OPS_CAR_SWRITE_MASK	  0x00002000 /**< Streaming
						              * Write Mask 
						              */
#define XSRIO_SRCDST_OPS_CAR_WRITE_MASK 	  0x00004000 /**< Write Mask */
#define XSRIO_SRCDST_OPS_CAR_READ_MASK	          0x00008000 /**< Read Mask */
/*@}*/

/** @name PE Logical layer Control CSR bit definitions.
 *  These bits are associated with the XSRIO_PELL_CTRL_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PELL_CTRL_CSR_EAC_MASK   0x00000007 /**< Extended Addressing
					           * Control Mask 
					           */
/*@}*/

/** @name Local Configuration Space Base Address 1 CSR bit definitions.
 *  These bits are associated with the XSRIO_LCS1_BASEADDR_CSR_OFFSET register.
 * @{
 */
#define XSRIO_LCS1_BASEADDR_LCSBA_CSR_MASK   0x7FE00000 /**< LCSBA Mask */
#define XSRIO_LCS1_BASEADDR_LCSBA_CSR_SHIFT  21         /**< LCSBA Shift */
/*@}*/

/** @name Base Device ID CSR bit definitions.
 *  These bits are associated with the XSRIO_BASE_DID_CSR_OFFSET register.
 * @{
 */
#define XSRIO_BASE_DID_CSR_LBDID_MASK	0x0000FFFF /**< Large Base Device ID
						    * Mask(16-bit device ID)
						    */
#define XSRIO_BASE_DID_CSR_BDID_MASK	0x00FF0000 /**< Base Device ID
						    * Mask(8-bit device ID)
						    */
#define XSRIO_BASE_DID_CSR_BDID_SHIFT	16 	   /**< Base Device ID Shift */
/*@}*/

/** @name Host Base Device ID CSR bit definitions.
 *  These bits are associated with the XSRIO_HOST_DID_LOCK_CSR_OFFSET register.
 * @{
 */
#define XSRIO_HOST_DID_LOCK_CSR_HBDID_MASK  0x0000FFFF /**< Host Base
						        * Device ID Mask
						        */
/*@}*/

/** @name LP - Serial Register Block header bit definitions.
 *  These bits are associated with the XSRIO_EFB_HEADER_OFFSET register.
 * @{
 */
#define XSRIO_EFB_HEADER_EFID_MASK     0x0000FFFF /**< Extended Features ID
						   * Mask
						   */
#define XSRIO_EFB_HEADER_EFP_MASK      0xFFFF0000 /**< Extended Features
						   * Pointer Mask
					           */
#define XSRIO_EFB_HEADER_EFP_SHIFT     16         /**< Extended Features
						   * Pointer Shift
						   */
/*@}*/

/** @name Port Link timeout value CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_LINK_TOUT_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_LINK_TOUT_CSR_TOUTVAL_MASK  0xFFFFFF00 /**< Timeout Value 
							   * Mask
							   */
#define XSRIO_PORT_LINK_TOUT_CSR_TOUTVAL_SHIFT 8	  /**< Timeout Value
							   * Shift
							   */
/*@}*/
 
/** @name Port response timeout  value CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_RESP_TOUT_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_RESP_TOUT_CSR_TOUTVAL_MASK  0xFFFFFF00 /**< Response Timeout
					                   * Value Mask
						           */
#define XSRIO_PORT_RESP_TOUT_CSR_TOUTVAL_SHIFT 8	  /**< Response Timeout
							   * Shift
							   */
/*@}*/
 
/** @name Port General Control CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_GEN_CTL_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_GEN_CTL_CSR_DISCOVERED_MASK  0x20000000 /**< Discovered Mask */
#define XSRIO_PORT_GEN_CTL_CSR_MENABLE_MASK     0x40000000 /**< Master Enable Mask */
#define XSRIO_PORT_GEN_CTL_CSR_HOST_MASK	0x80000000 /**< Host Mask */

/*@}*/

/** @name Port n maintenance request CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_N_MNT_REQ_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_N_MNT_REQ_CSR_CMD_MASK  0x00000007 /**< Command Mask */
/*@}*/

/** @name Port n maintenance response CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_N_MNT_RES_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_N_MNT_RES_CSR_LS_MASK     0x0000001F /**< link status Mask */
#define XSRIO_PORT_N_MNT_RES_CSR_ACKS_MASK   0x000007E0 /**< Ack ID status
							 * Mask
							 */
#define XSRIO_PORT_N_MNT_RES_CSR_RVALID_MASK 0x80000000 /**< Response Valid
							 * Mask
							 */
/*@}*/

/** @name Port n local ack ID CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_N_ACKID_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_N_ACKID_CSR_OBACKID_MASK       0x0000003F /**< Out bound
					                      * ACK ID Mask
						              */
#define XSRIO_PORT_N_ACKID_CSR_OSACKID_MASK       0x00003F00 /**< Out Standing
						              * ACK ID Mask
						              */
#define XSRIO_PORT_N_ACKID_CSR_IBACKID_MASK       0x3F000000 /**< In bound 
							      * ACK ID Mask
							      */
#define XSRIO_PORT_N_ACKID_CSR_CLSACKID_MASK      0x80000000 /**< Clear
							      * Outstanding
						              * ACK ID Mask
						              */
#define XSRIO_PORT_N_ACKID_CSR_RESET_OBACKID_MASK 0xFFFFFFC0 /**< Out bound ACK
							      * ID Reset Mask 
							      */
#define XSRIO_PORT_N_ACKID_CSR_RESET_IBACKID_MASK 0xC0FFFFFF /**< In bound ACK
							      * ID Reset Mask
							      */
#define XSRIO_PORT_N_ACKID_CSR_IBACKID_SHIFT      24         /**< In bound 
					   		      * ACK ID shift
						              */
/*@}*/

/** @name Port n Error and Status CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_N_ERR_STS_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_N_ERR_STS_CSR_PUINT_MASK    0x00000001 /**< Port
						           * un-initialized Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_POK_MASK      0x00000002 /**< Port Ok Mask */
#define XSRIO_PORT_N_ERR_STS_CSR_PERR_MASK     0x00000004 /**< Port Error Mask */
#define XSRIO_PORT_N_ERR_STS_CSR_IERRS_MASK    0x00000100 /**< Input Error
						           * stopped Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_IERRE_MASK    0x00000200 /**< Input Error
						           * encountered Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_IRTS_MASK     0x00000400 /**< Input Retry
						           * Stopped Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_OERRS_MASK    0x00010000 /**< Output error
						           * Stopped Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_OERRE_MASK    0x00020000 /**< Output error
						           * encountered Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_ORTS_MASK     0x00040000 /**< Output Retry
						           * Stopped Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_OR_MASK       0x00080000 /**< Output
						           * Retried Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_ORE_MASK      0x00100000 /**< Output Retry
						           * Encountered Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_FLOWCNTL_MASK 0x08000000 /**< Flow Control
						           * Mode Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_IDL_SEQ_MASK  0x20000000 /**< Idle sequence
							   * Mask
							   */
#define XSRIO_PORT_N_ERR_STS_CSR_IDL_SEQE_MASK 0x40000000 /**< Idle sequence 2
						           * Enable Mask 
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_IDL_SEQS_MASK 0x80000000 /**< Idle sequence 2
						           * support Mask
						           */
#define XSRIO_PORT_N_ERR_STS_CSR_ERR_ALL_MASK  0x001FFF07 /**< Port Errors Mask */
/*@}*/

/** @name Port n Control CSR bit definitions.
 *  These bits are associated with the XSRIO_PORT_N_CTL_CSR_OFFSET register.
 * @{
 */
#define XSRIO_PORT_N_CTL_CSR_PTYPE_MASK       0x00000001 /**< Port Type Mask */
#define XSRIO_PORT_N_CTL_CSR_EPWDS_MASK       0x00003000 /**< Extended Port
					                  * Width Support Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_EPWOR_MASK       0x0000C000 /**< Extended Port
						          * Width Override Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_ENUMB_MASK       0x00020000 /**< Enumeration
						          * Boundary Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_MCENT_MASK       0x00080000 /**< Multi-cast Event
						          * Participant Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_ERRD_MASK 	      0x00100000 /**< Error Checking
						          * Disable Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_IPE_MASK	      0x00200000 /**< Input port
						          * enable Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_OPE_MASK	      0x00400000 /**< Output port
						          * enable Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_PD_MASK	      0x00800000 /**< Output port
						          * disable Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_PWO_MASK	      0x07000000 /**< Port width
						          * Override Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_RESET_PWO_MASK   0xF8FFFFFF /**< Port width 
						          * Override Reset Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_IPW_MASK         0x38000000 /**< Initialized
						          * Port width Mask
						          */
#define XSRIO_PORT_N_CTL_CSR_PW_MASK	      0xc0000000 /**< Port width Mask */
#define XSRIO_PORT_N_CTL_CSR_STATUS_ALL_MASK 0x00F00000 /**< Port Status All 
						 	  * Mask
							  */

#define XSRIO_PORT_N_CTL_CSR_PWO_SHIFT	  24         /**< Port width
						      * Override Shift
						      */
#define XSRIO_PORT_N_CTL_CSR_PW_SHIFT	 30          /**< Port width
						      * Shift
						      */
/*@}*/

/** @name LP -Serial Lane Register Block Header bit definitions.
 *  These bits are associated with the XSRIO_SL_HEADER_OFFSET register.
 * @{
 */
#define XSRIO_SL_HEADER_EFID_MASK     0x0000FFFF /**< Extended
						  * Features ID Mask
						  */
#define XSRIO_SL_HEADER_EFP_MASK      0xFFFF0000 /**< Extended Features
						  * Pointer Mask 
						  */
#define XSRIO_SL_HEADER_EFP_SHIFT     16         /**< Extended Features
						  * Pointer Shift
						  */
/*@}*/

/** @name LP -Seral Lane n Status 0 CSRS bit definitions.
 *  These bits are associated with the XSRIO_SLS0_CSR(x) register.
 * @{
 */
#define XSRIO_SLS0_CSR_PORT_NUM_MASK         0xFF000000 /**< Port Number Mask */
#define XSRIO_SLS0_CSR_LANE_NUM_MASK	     0x00F00000 /**< Lane Number Mask */
#define XSRIO_SLS0_CSR_TRANSMIT_TYPE_MASK    0x00080000 /**< Transmitter
							 * Type Mask
							 */
#define XSRIO_SLS0_CSR_TRANSMIT_MODE_MASK    0x00040000 /**< Transmitter 
						         * Mode Mask
							 */
#define XSRIO_SLS0_CSR_RCV_INPUT_INV_MASK    0x00008000 /**< Receiver Input
							 * Inverted Mask
							 */
#define XSRIO_SLS0_CSR_RCV_TRAINED_MASK      0x00004000 /**< Receiver
							 * Trained Mask
							 */
#define XSRIO_SLS0_CSR_RCVLANE_SYNC_MASK     0x00002000 /**< Receive Lane
							 * Sync Mask
							 */ 
#define XSRIO_SLS0_CSR_RCVLANE_RDY_MASK	     0x00001000 /**< Receive Lane
							 * Ready Mask
							 */
#define XSRIO_SLS0_CSR_DECODING_ERRORS_MASK  0x00000F00 /**< 8B/10B Decoding
							 * errors Mask
							 */
#define XSRIO_SLS0_CSR_LANESYNC_CHAN_MASK    0x00000080 /**< lane_sync state
							 * change Mask
							 */
#define XSRIO_SLS0_CSR_RCVTRAINED_CHAN_MASK  0x00000040 /**< rcvr_train state
							 * changed Mask
							 */
#define XSRIO_SLS0_CSR_STAT1_IMP_MASK        0x00000008 /**< Status 1 CSR
							 * Implemented Mask 
							 */
#define XSRIO_SLS0_CSR_DECODING_ERRORS_SHIFT 8
/*@}*/

/** @name LP -Seral Lane n Status 1 CSRS bit definitions.
 *  These bits are associated with the XSRIO_SLS1_CSR(x) register.
 * @{
 */
 
#define XSRIO_SLS1_CSR_SCRDSCR_EN_MASK   0x00008000 /**< Connected port
						     * Scrambling/Descrambling 
						     * Enabled Mask
						     */
#define XSRIO_SLS1_CSR_CPTEIS_MASK      0x00030000  /**< Connected port transmit
						     * Emphasis Tap(+1) Status 
						     * Mask
						     */
#define XSRIO_SLS1_CSR_CPTEDS_MASK       0x000C0000 /**< Connected port transmit
						     * Emphasis Tap(-1) Status
						     * Mask
						     */
#define XSRIO_SLS1_CSR_LANENUM_MASK      0x00F00000 /**< Lane number within 
						     * connected port
						     */
#define XSRIO_SLS1_CSR_RXPORT_WIDTH_MASK 0x07000000 /**< Receive port width 
						     * Mask
						     */
#define XSRIO_SLS1_CSR_CPLR_TRAINED_MASK 0x08000000 /**< Connected port lane
						     * Receiver trained Mask 
						     */
#define XSRIO_SLS1_CSR_IMPDEFINED_MASK   0x10000000 /**< Implementation defined
						     * Mask
						     */
#define XSRIO_SLS1_CSR_VALCHANGED_MASK   0x20000000 /**< Values Changed Mask */
#define XSRIO_SLS1_CSR_IDLE2_INFO_MASK   0x40000000 /**< IDLE2 Information
						     * Current Mask
						     */
#define XSRIO_SLS1_CSR_IDLE2_REC_MASK    0x80000000 /**< IDLE2 Received Mask */
/*@}*/

/** @name Water Mark CSRS  bit definitions.
 *  These bits are associated with the XSRIO_IMP_WCSR_OFFSET register.
 * @{
 */

#define XSRIO_IMP_WCSR_WM2_MASK        0x003F0000 /**< Water Mark 2 Mask */
#define XSRIO_IMP_WCSR_WM1_MASK        0x00003F00 /**< Water Mark 1 Mask */
#define XSRIO_IMP_WCSR_WM0_MASK        0x0000003F /**< Water Mark 0 Mask */
#define XSRIO_IMP_WCSR_WM1_SHIFT       8 	  /**< Water Mark 1 Shift */
#define XSRIO_IMP_WCSR_WM2_SHIFT       16	  /**< Water Mark 2 Shift */
/*@}*/

/** @name Buffer Control CSRS  bit definitions.
 *  These bits are associated with the XSRIO_IMP_BCSR_OFFSET register.
 * @{
 */
#define XSRIO_IMP_BCSR_RXFLOW_CNTLONLY_MASK  0x80000000 /**< Rx Flow Control
							 * Only Mask
							 */
#define XSRIO_IMP_BCSR_UNIFIED_CLK_MASK      0x40000000 /**< Buffer Control
							 * Mask 
							 */
#define XSRIO_IMP_BCSR_TX_FLOW_CNTL_MASK     0x20000000 /**< Tx Flow 
							 * Control Mask
							 */
#define XSRIO_IMP_BCSR_TXREQ_REORDER_MASK    0x10000000 /**< Tx Request
							 * Reorder Mask */
#define XSRIO_IMP_BCSR_TXSIZE_MASK 	     0x07FF0000 /**< Tx size Mask */
#define XSRIO_IMP_BCSR_FRX_FLOW_CNTL_MASK    0x00008000 /**< Force Rx flow
							 * Control Mask 
							 */ 
#define XSRIO_IMP_BCSR_RXSIZE_MASK           0x000000FF /**< Rx size Mask */
#define XSRIO_IMP_BCSR_TXSIZE_SHIFT	     16		/**< Tx size shift */
/*@}*/
 
/** @name Maintenance Request Information Register bit definitions.
 *  These bits are associated with the XSRIO_IMP_MRIR_OFFSET register.
 * @{
 */
#define XSRIO_IMP_MRIR_REQ_TID_MASK     0xFF000000 /**< Request TID Mask */
#define XSRIO_IMP_MRIR_REQ_PRIO_MASK    0x00060000 /**< Request Priority Mask */
#define XSRIO_IMP_MRIR_REQ_CRF_MASK     0x00010000 /**< Request CRF Mask */
#define XSRIO_IMP_MRIR_REQ_DESTID_MASK  0x0000FFFF /**< Request Destination
					            * ID Mask
						    */
#define XSRIO_IMP_MRIR_REQ_PRIO_SHIFT	   17 
#define XSRIO_IMP_MRIR_REQ_CRF_SHIFT	   16  
#define XSRIO_IMP_MRIR_REQ_TID_SHIFT	   24 

/*@}*/

/****************** Macros (Inline Functions) Definitions ********************/
/*****************************************************************************/
/**
* Macro to read register.
*
* @param        BaseAddress is the base address of the SRIO
* @param        RegOffset is the register offset.
*
* @return       Value of the register.
*
* @note         C-style signature:
*               u32 XSrio_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XSrio_ReadReg(BaseAddress, RegOffset) \
        Xil_In32((BaseAddress) + (RegOffset))


/*****************************************************************************/
/**
* Macro to write register.
*
* @param        BaseAddress is the base address of the SRIO.
* @param        RegOffset is the register offset.
* @param        Data is the data to write.
*
* @return       None
*
* @note         C-style signature:
*               void XSRIO_WriteReg(u32 BaseAddress, u32 RegOffset,
*                                                               u32 Data)
*
******************************************************************************/
#define XSrio_WriteReg(BaseAddress, RegOffset, Data) \
        Xil_Out32((BaseAddress) + (RegOffset), (Data))
/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/
#ifdef __cplusplus
}
#endif

#endif		/* end of protection macro */

/** @} */
