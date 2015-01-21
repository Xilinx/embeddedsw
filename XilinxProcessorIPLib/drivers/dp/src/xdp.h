/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdp.h
 *
 * The Xilinx DisplayPort transmitter (DP) driver. This driver supports the
 * Xilinx DisplayPort soft IP core in both transmit/source (TX) and receive/sink
 * (RX) modes of operation.
 *
 * The Xilinx DisplayPort soft IP supports the following features:
 *	- 1, 2, or 4 lanes.
 *	- A link rate of 1.62, 2.70, or 5.40Gbps per lane.
 *	- 1, 2, or 4 pixel-wide video interfaces.
 *	- RGB and YCbCr color space.
 *	- Up to 16 bits per component.
 *	- Up to 4Kx2K monitor resolution.
 *	- Auto lane rate and width negotiation.
 *	- I2C over a 1Mb/s AUX channel.
 *	- Secondary channel audio support (2 channels).
 *	- 4 independent video multi-streams.
 *
 * The Xilinx DisplayPort soft IP does not support the following features:
 *	- The automated test feature.
 *	- Audio (3-8 channel).
 *	- FAUX.
 *	- Bridging function.
 *	- MST audio.
 *	- eDP optional features.
 *	- iDP.
 *	- GTC.
 *
 * <b>DisplayPort overview</b>
 *
 * A DisplayPort link consists of:
 *	- A unidirectional main link which is used to transport isochronous data
 *	  streams such as video and audio. The main link may use 1, 2, or 4
 *	  lanes at a link rate of 1.62, 2.70, or 5.40Gbps per lane. The link
 *	  needs to be trained prior to sending streams.
 *	- An auxiliary (AUX) channel is a 1MBps bidirectional channel used for
 *	  link training, link management, and device control.
 *	- A hot-plug-detect (HPD) signal line is used to determine whether a
 *	  DisplayPort connection exists between the DisplayPort TX connector and
 *	  an RX device. It is serves as an interrupt request by the RX device.
 *
 * <b>Device configuration</b>
 *
 * The device can be configured in various ways during the FPGA implementation
 * process. Configuration parameters are stored in the xdp_g.c file which is
 * generated when compiling the board support package (BSP). A table is defined
 * where each entry contains configuration information for the DisplayPort
 * instances present in the system. This information includes parameters that
 * are defined in the driver's data/dp.tcl file such as the base address of the
 * memory-mapped device and the maximum number of lanes, maximum link rate, and
 * video interface that the DisplayPort instance supports, among others.
 *
 * The DisplayPort core may be configured in both transmitter (TX) or receiver
 * (RX) modes of operation. Depending on which mode of operation the hardware is
 * configured for, the set of registers associated with the core will be
 * different.
 *
 * <b>Driver description</b>
 *
 * The DisplayPort (DP) driver consists of functions, structures, and
 * definitions:
 *	1) Specific to the DisplayPort TX mode of operation.
 *	   - Prefix: XDptx_* and XDPTX_*
 *	2) Specific to the DisplayPort RX mode of operation.
 *	   - Prefix: XDprx_* and XDPRX_*
 *	3) Common to both DisplayPort modes of operation.
 *	   - Prefix: XDp_* and XDP_*
 *
 * Depending on whether the DisplayPort core is configured for TX or RX mode of
 * operation, the set of registers and required functionality will be entirely
 * different.
 *	- A detailed description of the DisplayPort TX functionality and
 *	  associated functions may be found in xdptx.h. xdptx_hw.h contains
 *	  definitions of the TX register space.
 *	- A detailed description of the DisplayPort RX functionality and
 *	  associated functions may be found in xdprx.h. xdprx_hw.h contains
 *	  definitions of the RX register space.
 *
 * <b>Asserts</b>
 *
 * Asserts are used within all Xilinx drivers to enforce constraints on argument
 * values. Asserts can be turned off on a system-wide basis by defining, at
 * compile time, the NDEBUG identifier.  By default, asserts are turned on and
 * it is recommended that application developers leave asserts on during
 * development.
 *
 * @note	For a 5.4Gbps link rate, a high performance 7 series FPGA is
 *		required with a speed grade of -2 or -3.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/14 Initial release.
 * </pre>
 *
*******************************************************************************/

#ifndef XDP_H_
/* Prevent circular inclusions by using protection macros. */
#define XDP_H_

/******************************* Include Files ********************************/

#include "xil_assert.h"
#include "xil_types.h"
#include "xvidc.h"
/* xdprx.h and xdptx.h are included. They require some type definitions. */

/****************************** Type Definitions ******************************/

/**
 * This typedef contains configuration information for the DisplayPort core.
 */
typedef struct {
	u16 DeviceId;		/**< Device instance ID. */
	u32 BaseAddr;		/**< The base address of the core instance. */
	u32 SAxiClkHz;		/**< The clock frequency of the core instance's
					S_AXI_ACLK port. */
	u8 MaxLaneCount;	/**< The maximum lane count supported by this
					core instance. */
	u8 MaxLinkRate;		/**< The maximum link rate supported by this
					core instance. */
	u8 MaxBitsPerColor;	/**< The maximum bits/color supported by this
					core instance*/
	u8 QuadPixelEn;		/**< Quad pixel support by this core
					instance. */
	u8 DualPixelEn;		/**< Dual pixel support by this core
					instance. */
	u8 YCrCbEn;		/**< YCrCb format support by this core
					instance. */
	u8 YOnlyEn;		/**< YOnly format support by this core
					instance. */
	u8 PayloadDataWidth;	/**< The payload data width used by this core
					instance. */
	u8 SecondaryChEn;	/**< This core instance supports audio packets
					being sent by the secondary channel. */
	u8 NumAudioChs;		/**< The number of audio channels supported by
					this core instance. */
	u8 MstSupport;		/**< Multi-stream transport (MST) mode is
					enabled by this core instance. */
	u8 NumMstStreams;	/**< The total number of MST streams supported
					by this core instance. */
	u8 DpProtocol;		/**< The DisplayPort protocol version that this
					core instance is configured for.
					0 = v1.1a, 1 = v1.2. */
	u8 IsRx;		/**< The type of DisplayPort core.
					0 = TX, 1 = RX. */
} XDp_Config;

/******************************************************************************/
/**
 * Callback type which represents a custom timer wait handler. This is only
 * used for Microblaze since it doesn't have a native sleep function. To avoid
 * dependency on a hardware timer, the default wait functionality is implemented
 * using loop iterations; this isn't too accurate. If a custom timer handler is
 * used, the user may implement their own wait implementation using a hardware
 * timer (see example/) for better accuracy.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	MicroSeconds is the number of microseconds to be passed to the
 *		timer function.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDp_TimerHandler)(void *InstancePtr, u32 MicroSeconds);

/**************************** Function Prototypes *****************************/

/* xdp_sinit.c: Configuration extraction function. */
XDp_Config *XDp_LookupConfig(u16 DeviceId);

/******************* Macros (Inline Functions) Definitions ********************/

#define XDP_TX 0
#define XDP_RX 1

/******************************************************************************/
/**
 * This is function determines whether the DisplayPort core that the
 * configuration structure represents is a transmitter (TX) or a receiver (RX).
 *
 * @param	ConfigPtr is a pointer to the DisplayPort core's configuration
 *		structure.
 *
 * @return	XDP_RX if the configuration structure is for a core of type RX.
 *		XDP_TX if the configuration structure is for a core of type TX.
 *
 * @note	C-style signature:
 *		u32 XDp_CfgGetCoreType(XDp_Config *ConfigPtr)
 *
*******************************************************************************/
#define XDp_CfgGetCoreType(ConfigPtr)	((ConfigPtr)->IsRx ? XDP_RX : XDP_TX)

/******************************* Compatibility ********************************/

#define XDptx_LookupConfig	XDp_LookupConfig
#define XDptx_Config		XDp_Config
#define XDptx_TimerHandler	XDp_TimerHandler

#include "xdprx.h"
#include "xdptx.h"

#endif /* XDP_H_ */
