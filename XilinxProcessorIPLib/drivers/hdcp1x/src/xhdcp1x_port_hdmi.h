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
* @file xhdcp1x_port_hdmi.h
* @addtogroup hdcp1x_v4_2
* @{
*
* This file contains the definitions for the hdcp port registers/offsets for
* HDMI interfaces
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 3.1   yas    07/28/16 Added Bitmasks for BSTATUS register.
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_PORT_HDMI_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_PORT_HDMI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#if defined(XHDCP1X_PORT_DP_H)
#error "cannot include both xhdcp1x_port_dp.h and xhdcp1x_port_hdmi.h"
#endif

/************************** Constant Definitions *****************************/

/**
 * These constants specify the offsets for the various fields and/or
 * attributes within the hdcp port
 */
#define XHDCP1X_PORT_OFFSET_BKSV	(0x00u)   /**< Bksv Offset        */
#define XHDCP1X_PORT_OFFSET_RO		(0x08u)   /**< Ri'/Ro' Offset     */
#define XHDCP1X_PORT_OFFSET_PJ		(0x0Au)   /**< Pj' Offset         */
#define XHDCP1X_PORT_OFFSET_AKSV	(0x10u)   /**< Aksv Offset        */
#define XHDCP1X_PORT_OFFSET_AINFO	(0x15u)   /**< Ainfo Offset       */
#define XHDCP1X_PORT_OFFSET_AN		(0x18u)   /**< An Offset          */
#define XHDCP1X_PORT_OFFSET_VH0		(0x20u)   /**< V'.H0 Offset       */
#define XHDCP1X_PORT_OFFSET_VH1		(0x24u)   /**< V'.H1 Offset       */
#define XHDCP1X_PORT_OFFSET_VH2		(0x28u)   /**< V'.H2 Offset       */
#define XHDCP1X_PORT_OFFSET_VH3		(0x2Cu)   /**< V'.H3 Offset       */
#define XHDCP1X_PORT_OFFSET_VH4		(0x30u)   /**< V'.H4 Offset       */
#define XHDCP1X_PORT_OFFSET_BCAPS	(0x40u)   /**< Bcaps Offset       */
#define XHDCP1X_PORT_OFFSET_BSTATUS	(0x41u)   /**< Bstatus Offset     */
#define XHDCP1X_PORT_OFFSET_KSVFIFO	(0x43u)   /**< KSV FIFO Offset    */
#define XHDCP1X_PORT_OFFSET_DBG		(0xC0u)   /**< Debug Space Offset */

/**
 * These constants specify the sizes for the various fields and/or
 * attributes within the hdcp port
 */
#define XHDCP1X_PORT_SIZE_BKSV		(0x05u)   /**< Bksv Size          */
#define XHDCP1X_PORT_SIZE_RO		(0x02u)   /**< Ri' Size           */
#define XHDCP1X_PORT_SIZE_PJ		(0x01u)   /**< Pj' Size           */
#define XHDCP1X_PORT_SIZE_AKSV		(0x05u)   /**< Aksv Size          */
#define XHDCP1X_PORT_SIZE_AINFO		(0x01u)   /**< Ainfo Size         */
#define XHDCP1X_PORT_SIZE_AN		(0x08u)   /**< An Size            */
#define XHDCP1X_PORT_SIZE_VH0		(0x04u)   /**< V'.H0 Size         */
#define XHDCP1X_PORT_SIZE_VH1		(0x04u)   /**< V'.H1 Size         */
#define XHDCP1X_PORT_SIZE_VH2		(0x04u)   /**< V'.H2 Size         */
#define XHDCP1X_PORT_SIZE_VH3		(0x04u)   /**< V'.H3 Size         */
#define XHDCP1X_PORT_SIZE_VH4		(0x04u)   /**< V'.H4 Size         */
#define XHDCP1X_PORT_SIZE_BCAPS		(0x01u)   /**< Bcaps Size         */
#define XHDCP1X_PORT_SIZE_BSTATUS	(0x02u)   /**< Bstatus Size       */
#define XHDCP1X_PORT_SIZE_KSVFIFO	(0x01u)   /**< KSV FIFO Size      */
#define XHDCP1X_PORT_SIZE_DBG		(0xC0u)   /**< Debug Space Size   */

/**
 * These constants specify the bit definitions within the various fields
 * and/or attributes within the hdcp port
 */
#define XHDCP1X_PORT_BIT_BSTATUS_HDMI_MODE  (1u << 12) /**< BStatus HDMI Mode
							 *  Mask */
#define XHDCP1X_PORT_BIT_BCAPS_FAST_REAUTH  (1u <<  0) /**< BCaps Fast Reauth
							 *  Mask */
#define XHDCP1X_PORT_BIT_BCAPS_1d1_FEATURES (1u <<  1) /**< BCaps HDCP 1.1
							 *  Features Support
							 *  Mask */
#define XHDCP1X_PORT_BIT_BCAPS_FAST	    (1u <<  4) /**< BCaps Fast
							 *  Transfers Mask */
#define XHDCP1X_PORT_BIT_BCAPS_READY	    (1u <<  5) /**< BCaps KSV FIFO
							 *  Ready bit Mask */
#define XHDCP1X_PORT_BIT_BCAPS_REPEATER	    (1u <<  6) /**< BCaps Repeater
							 *  Capable Mask */
#define XHDCP1X_PORT_BIT_BCAPS_HDMI	    (1u <<  7) /**< BCaps HDMI
							 *  Supported Mask */
#define XHDCP1X_PORT_BIT_AINFO_ENABLE_1d1_FEATURES (1u <<  1) /**< AInfo Enable
								*  1.1
								*  Features */

#define XHDCP1X_PORT_BSTATUS_BIT_DEV_CNT_ERR	(1u << 7) /**< BStatus Device
							 *  Count Error Mask */
#define XHDCP1X_PORT_BSTATUS_BIT_DEV_CNT_NO_ERR	(0u << 7) /**< BStatus Device
						  *  Count for No Error Mask */
#define XHDCP1X_PORT_BSTATUS_DEV_CNT_MASK		(0x7F) /**< BStatus
						  *  Device Count Error Mask */
#define XHDCP1X_PORT_BSTATUS_BIT_DEPTH_ERR		(1u << 11) /**< BStatus
							  * Depth Error Mask */
#define XHDCP1X_PORT_BSTATUS_BIT_DEPTH_NO_ERR	(0u << 11) /**< BStatus Depth
						  *  Error for No Error Mask */
#define XHDCP1X_PORT_BSTATUS_DEV_CNT_ERR_SHIFT	(7) /**< BStatus Device
						    *  Count Error Shift Mask*/
#define XHDCP1X_PORT_BSTATUS_DEPTH_ERR_SHIFT	(11) /**< BStatus Depth
							  *  Error Shift Mask*/
#define XHDCP1X_PORT_BSTATUS_DEPTH_SHIFT		(8) /**< BStatus Device
							 *  Count Error Mask */

/**
 * This constant defines the i2c address of the hdcp port
 */
#define XHDCP1X_PORT_PRIMARY_I2C_ADDR	(0x74u)  /**< I2C Addr Primary Link  */
#define XHDCP1X_PORT_SECONDARY_I2C_ADDR	(0x76u)  /**< I2C Addr Secondary Link*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_PORT_HDMI_H */
/** @} */
