/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp1x_port_dp.h
* @addtogroup hdcp1x_v4_0
* @{
*
* This file contains the definitions for the hdcp port registers/offsets for
* display port interfaces
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 3.0   yas    02/13/16 Upgraded to support HDCP Repeater functionality.
*                       Added macro defintions for:
*                       XHDCP1X_PORT_HDCP_RESET_KSV,
*                       XHDCP1X_PORT_SIZE_HDCP_RESET_KSV and
*                       XHDCP1X_PORT_HDCP_RESET_KSV_RST.
* 3.1	yas    07/28/16 Added Bitmasks for BIFO register
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_PORT_DP_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_PORT_DP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#if defined(XHDCP1X_PORT_HDMI_H)
#error "cannot include both xhdcp1x_port_dp.h and xhdcp1x_port_hdmi.h"
#endif

/************************** Constant Definitions *****************************/

/**
 * These constants specify the offsets for the various fields and/or
 * attributes within the hdcp port
 */
#define XHDCP1X_PORT_OFFSET_BKSV 	(0x00u)   /**< Bksv Offset        */
#define XHDCP1X_PORT_OFFSET_RO		(0x05u)   /**< R0' Offset         */
#define XHDCP1X_PORT_OFFSET_AKSV	(0x07u)   /**< Aksv Offset        */
#define XHDCP1X_PORT_OFFSET_AN		(0x0Cu)   /**< An Offset          */
#define XHDCP1X_PORT_OFFSET_VH0		(0x14u)   /**< V'.H0 Offset       */
#define XHDCP1X_PORT_OFFSET_VH1		(0x18u)   /**< V'.H1 Offset       */
#define XHDCP1X_PORT_OFFSET_VH2		(0x1Cu)   /**< V'.H2 Offset       */
#define XHDCP1X_PORT_OFFSET_VH3		(0x20u)   /**< V'.H3 Offset       */
#define XHDCP1X_PORT_OFFSET_VH4		(0x24u)   /**< V'.H4 Offset       */
#define XHDCP1X_PORT_OFFSET_BCAPS	(0x28u)   /**< Bcaps Offset       */
#define XHDCP1X_PORT_OFFSET_BSTATUS	(0x29u)   /**< Bstatus Offset     */
#define XHDCP1X_PORT_OFFSET_BINFO	(0x2Au)   /**< Binfo Offset       */
#define XHDCP1X_PORT_OFFSET_KSVFIFO	(0x2Cu)   /**< KSV FIFO Offset    */
#define XHDCP1X_PORT_OFFSET_AINFO	(0x3Bu)   /**< Ainfo Offset       */
#define XHDCP1X_PORT_OFFSET_DBG		(0xC0u)   /**< Debug Space Offset */
#define XHDCP1X_PORT_HDCP_RESET_KSV (0xD0u)   /**< KSV FIFO Read pointer
						*  reset Offset*/

/**
 * These constants specify the sizes for the various fields and/or
 * attributes within the hdcp port
 */
#define XHDCP1X_PORT_SIZE_BKSV		(0x05u)   /**< Bksv Size          */
#define XHDCP1X_PORT_SIZE_RO		(0x02u)   /**< R0' Size           */
#define XHDCP1X_PORT_SIZE_AKSV		(0x05u)   /**< Aksv Size          */
#define XHDCP1X_PORT_SIZE_AN		(0x08u)   /**< An Size            */
#define XHDCP1X_PORT_SIZE_VH0		(0x04u)   /**< V'.H0 Size         */
#define XHDCP1X_PORT_SIZE_VH1		(0x04u)   /**< V'.H1 Size         */
#define XHDCP1X_PORT_SIZE_VH2		(0x04u)   /**< V'.H2 Size         */
#define XHDCP1X_PORT_SIZE_VH3		(0x04u)   /**< V'.H3 Size         */
#define XHDCP1X_PORT_SIZE_VH4		(0x04u)   /**< V'.H4 Size         */
#define XHDCP1X_PORT_SIZE_BCAPS		(0x01u)   /**< Bcaps Size         */
#define XHDCP1X_PORT_SIZE_BSTATUS	(0x01u)   /**< Bstatus Size       */
#define XHDCP1X_PORT_SIZE_BINFO		(0x02u)   /**< Binfo Size         */
#define XHDCP1X_PORT_SIZE_KSVFIFO	(0x0Fu)   /**< KSV FIFO Size      */
#define XHDCP1X_PORT_SIZE_AINFO		(0x01u)   /**< Ainfo Offset       */
#define XHDCP1X_PORT_SIZE_DBG		(0x40u)   /**< Debug Space Size   */
#define XHDCP1X_PORT_SIZE_HDCP_RESET_KSV (0x40u)   /**< KSV FIFO pointer
						     *  reset Size   */

/**
 * These constants specify the bit definitions within the various fields
 * and/or attributes within the hdcp port
 */
#define XHDCP1X_PORT_BIT_BSTATUS_READY		(1u << 0) /**< BStatus Ready
							    *  Mask          */
#define XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE	(1u << 1)/**< BStatus Ro
							   *  available Mask */
#define XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE	(1u << 2) /**< BStatus Link
							    *  Failure Mask  */
#define XHDCP1X_PORT_BIT_BSTATUS_REAUTH_REQUEST	(1u << 3) /**< BStatus Reauth
							    *  Request Mask  */
#define XHDCP1X_PORT_BIT_BCAPS_HDCP_CAPABLE	(1u << 0) /**< BCaps HDCP
							    *  Capable Mask  */
#define XHDCP1X_PORT_BIT_BCAPS_REPEATER		(1u << 1) /**< BCaps HDCP
							    *  Repeater Mask */
#define XHDCP1X_PORT_BIT_AINFO_REAUTH_ENABLE_IRQ	(1u << 0) /**< Ainfo
						      *  Reauth Enable Mask  */

#define XHDCP1X_PORT_HDCP_RESET_KSV_RST		(1u << 0) /**< KSV FIFO pointer
							    *  Reset Mask    */
#define XHDCP1X_PORT_BINFO_BIT_DEV_CNT_ERR	(1u << 7) /**< BInfo Device
							 *  Count Error Mask */
#define XHDCP1X_PORT_BINFO_BIT_DEV_CNT_NO_ERR	(0u << 7) /**< BInfo Device
					          *  Count for No Error Mask */
#define XHDCP1X_PORT_BINFO_DEV_CNT_MASK		(0x7F) /**< BInfo Device
							 *  Count Error Mask */
#define XHDCP1X_PORT_BINFO_BIT_DEPTH_ERR	(1u << 11) /**< BInfo Depth
							    *  Error Mask    */
#define XHDCP1X_PORT_BINFO_BIT_DEPTH_NO_ERR	(0u << 11) /**< BInfo Depth
						  *  Error for No Error Mask */
#define XHDCP1X_PORT_BINFO_DEV_CNT_ERR_SHIFT	(7) /**< BStatus Device
						    *  Count Error Shift Mask*/
#define XHDCP1X_PORT_BINFO_DEPTH_ERR_SHIFT	(11) /**< BStatus Depth
							  *  Error Shift Mask*/
#define XHDCP1X_PORT_BINFO_DEPTH_SHIFT		(8) /**< BInfo Device
							 *  Count Error Mask */

/**
 * This constant defines the base address of the hdcp port within the DPCD
 * address space
 */
#define XHDCP1X_PORT_DPCD_BASE		(0x68000u)   /**< Base Addr in DPCD */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_PORT_DP_H */
/** @} */
