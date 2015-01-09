/*******************************************************************************
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
 * The Xilinx DisplayPort transmitter (DPTX) driver. This driver supports the
 * Xilinx DisplayPort soft IP core in source (TX) mode. This driver follows the
 * DisplayPort 1.2a specification.
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
 * <b>Driver description</b>
 *
 * The device driver enables higher-level software (e.g., an application) to
 * configure and control a DisplayPort TX soft IP, communicate and control an
 * RX device/sink monitor over the AUX channel, and to initialize and transmit
 * data streams over the main link.
 *
 * This driver implements link layer functionality: a Link Policy Maker (LPM)
 * and a Stream Policy Maker (SPM) as per the DisplayPort 1.2a specification.
 * - The LPM manages the main link and is responsible for keeping the link
 *   synchronized. It will establish a link with a downstream RX device by
 *   undergoing a link training sequence which consists of:
 *	- Clock recovery: The clock needs to be recovered and PLLs need to be
 *	  locked for all lanes.
 *	- Channel equalization: All lanes need to achieve channel equalization
 *	  and and symbol lock, as well as for interlane alignment to take place.
 * - The SPM manages transportation of an isochronous stream. That is, it will
 *   initialize and maintain a video stream, establish a virtual channel to a
 *   sink monitor, and transmit the stream.
 *
 * Using AUX transactions to read/write from/to the sink's DisplayPort
 * Configuration Data (DPCD) address space, the LPM obtains the link
 * capabilities, obtains link configuration and link and sink status, and
 * configures and controls the link and sink. The main link is trained this way.
 *
 * I2C-over-AUX transactions are used to obtain the sink's Extended Display
 * Identification Data (EDID) which give information on the display capabilities
 * of the monitor. The SPM may use this information to determine what available
 * screen resolutions and video timing are possible.
 *
 * <b>Device configuration</b>
 *
 * The device can be configured in various ways during the FPGA implementation
 * process.  Configuration parameters are stored in the xdptx_g.c file which is
 * generated when compiling the board support package (BSP). A table is defined
 * where each entry contains configuration information for the DisplayPort
 * instances present in the system. This information includes parameters that
 * are defined in the driver's data/dptx.tcl file such as the base address of
 * the memory-mapped device and the maximum number of lanes, maximum link rate,
 * and video interface that the DisplayPort instance supports, among others.
 *
 * <b>Interrupt processing</b>
 *
 * DisplayPort interrupts occur on the HPD signal line when the DisplayPort
 * cable is connected/disconnected or when the RX device sends a pulse. The user
 * hardware design must contain an interrupt controller which the DisplayPort
 * TX instance's interrupt signal is connected to. The user application must
 * enable interrupts in the system and set up the interrupt controller such that
 * the XDptx_HpdInterruptHandler handler will service DisplayPort interrupts.
 * When the XDptx_HpdInterruptHandler function is invoked, the handler will
 * identify what type of DisplayPort interrupt has occurred, and will call
 * either the HPD event handler function or the HPD pulse handler function,
 * depending on whether a an HPD event on an HPD pulse event occurred.
 *
 * The DisplayPort TX's XDPTX_INTERRUPT_STATUS register indicates the type of
 * interrupt that has occured, and the XDptx_HpdInterruptHandler will use this
 * information to decide which handler to call. An HPD event is identified if
 * bit XDPTX_INTERRUPT_STATUS_HPD_EVENT_MASK is set, and an HPD pulse is
 * identified from the XDPTX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK bit.
 *
 * The HPD event handler may be set up by using the XDptx_SetHpdEventHandler
 * function and, for the HPD pulse handler, the XDptx_SetHpdPulseHandler
 * function.
 *
 * <b>Multi-stream transport (MST) mode</b>
 *
 * The driver handles MST mode functionality, including sideband messaging,
 * topology discovery, virtual channel payload ID table management, and
 * directing streams to different sinks.
 *
 * MST testing has been done at all possible link rate/lane count/topology/
 * resolution/color depth combinations with each setting using following values:
 * - Link rate: 1.62, 2.70, and 5.40Gbps per lane.
 * - Lane count: 1, 2, and 4 lanes.
 * - Number of sink displays: 1, 2, 3, and 4 sink displays in both a daisy-chain
 *   configuration and in a configuration using a combination of a 1-to-3 hub
 *   and daisy-chain. Each stream was using the same resolution.
 * - Resolutions (60Hz): 640x480, 800x600, 1024x768, 1280x800, 1280x1024,
 *   1360x768, 1400x1050, 1680x1050, 1920x1080, 1920x2160, and 3840x2160.
 * - Color depths: 18, 24, 30, 36, and 48 bits per pixel.
 *
 * <b>Audio</b>
 *
 * The driver does not handle audio. For an example as to how to configure and
 * transmit audio, examples/xdptx_audio_example.c illustrates the required
 * sequence. The user will need to configure the audio source connected to the
 * Displayport TX instance and set up the audio info frame as per user
 * requirements.
 *
 * <b>Asserts</b>
 *
 * Asserts are used within all Xilinx drivers to enforce constraints on argument
 * values. Asserts can be turned off on a system-wide basis by defining, at
 * compile time, the NDEBUG identifier.  By default, asserts are turned on and
 * it is recommended that application developers leave asserts on during
 * development.
 *
 * <b>Limitations</b>
 *
 * - For MST mode to correctly display, the current version of the driver
 *   requires that each of the DisplayPort TX streams be allocated without
 *   skipping streams (i.e. assign stream 1, stream 2, and stream 3 - problems
 *   were experienced if skipping stream 2 and assigning stream 4 instead).
 *   skipping monitors in a daisy chain is OK as long as they are assigned to
 *   streams in order.
 * - In MST mode, the current version of the driver does not support removal of
 *   an allocated stream from the virtual channel payload ID table without
 *   clearing the entire table.
 * - Some sideband messages have not been implemented in the current version of
 *   the driver for MST mode. Notably, reception of a CONNECTION_STATUS_NOTIFY
 *   sideband message.
 * - The driver does not handle audio. See the audio example in the driver
 *   examples directory for the required sequence for enabling audio.
 *
 * @note	For a 5.4Gbps link rate, a high performance 7 series FPGA is
 *		required with a speed grade of -2 or -3.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * </pre>
 *
*******************************************************************************/

#ifndef XDP_H_
/* Prevent circular inclusions by using protection macros. */
#define XDP_H_

/******************************* Include Files ********************************/

#include "xdptx.h"

/****************************** Type Definitions ******************************/

/**************************** Function Prototypes *****************************/

/* xdp_sinit.c: Configuration extraction function.*/
XDptx_Config *XDp_LookupConfig(u16 DeviceId);

/* Backwards compatible with dptx. */
#define XDptx_LookupConfig XDp_LookupConfig

#endif /* XDP_H_ */
