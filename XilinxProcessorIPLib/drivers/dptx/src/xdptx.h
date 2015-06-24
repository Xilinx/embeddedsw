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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx.h
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
 * 1.0   als  05/17/14 Initial release.
 *       als  08/03/14 Initial MST addition.
 * 2.0   als  09/21/14 Added XDptx_DiscoverTopology function and changed
 *                     XDptx_IsConnected from macro to function.
 * 3.0   als  12/16/14 Updated to use common video library.
 *                     Added topology reordering functions:
 *                         XDptx_TopologySwapSinks,
 *                         XDptx_TopologySortSinksByTiling
 *                     Added wrapper functions for remote DPCD/I2C read/writes:
 *                         XDptx_RemoteDpcdRead, XDptx_RemoteDpcdWrite,
 *                         XDptx_RemoteIicRead, XDptx_RemoteIicWrite
 *                     Added EDID utility functions:
 *                         XDptx_GetRemoteEdid, XDptx_GetEdidBlock,
 *                         XDptx_GetRemoteEdidBlock,
 *                         XDptx_GetRemoteEdidDispIdExt,
 *                         XDptx_GetDispIdDataBlock,
 *                         XDptx_GetRemoteTiledDisplayDb
 *                     Remove unused arguments from functions:
 *                         LinkCountTotal, RelativeAddress from
 *                             XDptx_AllocatePayloadVcIdTable
 *                         RegStartAddress from XDptx_IicWrite
 * </pre>
 *
*******************************************************************************/

#ifndef XDPTX_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPTX_H_

/******************************* Include Files ********************************/

#include "xdptx_hw.h"
#include "xil_assert.h"
#include "xil_types.h"
#include "xvidc.h"

/****************************** Type Definitions ******************************/

/**
 * This typedef contains configuration information for the DisplayPort TX core.
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
} XDptx_Config;

/**
 * This typedef contains configuration information about the RX device.
 */
typedef struct {
	u8 DpcdRxCapsField[16];	/**< The first 16 bytes of the raw capabilities
					field of the RX device's DisplayPort
					Configuration Data (DPCD). */
	u8 LaneStatusAdjReqs[6];/**< This is a raw read of the RX device's
					status registers. The first 4 bytes
					correspond to the lane status associated
					with clock recovery, channel
					equalization, symbol lock, and interlane
					alignment. The remaining 2 bytes
					represent the pre-emphasis and voltage
					swing level adjustments requested by the
					RX device. */
} XDptx_SinkConfig;

/**
 * This typedef contains configuration information about the main link settings.
 */
typedef struct {
	u8 LaneCount;			/**< The current lane count of the main
						link. */
	u8 LinkRate;			/**< The current link rate of the main
						link. */
	u8 ScramblerEn;			/**< Symbol scrambling is currently in
						use over the main link. */
	u8 EnhancedFramingMode;		/**< Enhanced frame mode is currently in
						use over the main link. */
	u8 DownspreadControl;		/**< Downspread control is currently in
						use over the main link. */
	u8 MaxLaneCount;		/**< The maximum lane count of the main
						link. */
	u8 MaxLinkRate;			/**< The maximum link rate of the main
						link. */
	u8 SupportEnhancedFramingMode;	/**< Enhanced frame mode is supported by
						the RX device. */
	u8 SupportDownspreadControl;	/**< Downspread control is supported by
						the RX device. */
	u8 VsLevel;			/**< The current voltage swing level for
						each lane. */
	u8 PeLevel;			/**< The current pre-emphasis/cursor
						level for each lane. */
	u8 Pattern;			/**< The current pattern currently in
						use over the main link. */
} XDptx_LinkConfig;

/**
 * This typedef contains the main stream attributes which determine how the
 * video will be displayed.
 */
typedef struct {
	XVidC_VideoTimingMode Vtm;	/**< The video timing. */
	u32 PixelClockHz;		/**< The pixel clock of the stream (in
						Hz). */
	u32 HStart;			/**< Horizontal blank start (in
						pixels). */
	u32 VStart;			/**< Vertical blank start (in lines). */
	u32 Misc0;			/**< Miscellaneous stream attributes 0
						as specified by the DisplayPort
						1.2 specification. */
	u32 Misc1;			/**< Miscellaneous stream attributes 1
						as specified by the DisplayPort
						1.2 specification. */
	u32 NVid;			/**< N value for the video stream. */
	u32 UserPixelWidth;		/**< The width of the user data input
						port. */
	u32 DataPerLane;		/**< Used to translate the number of
						pixels per line to the native
						internal 16-bit datapath. */
	u32 AvgBytesPerTU;		/**< Average number of bytes per
						transfer unit, scaled up by a
						factor of 1000. */
	u32 TransferUnitSize;		/**< Size of the transfer unit in the
						framing logic. In MST mode, this
						is also the number of time slots
						that are alloted in the payload
						ID table. */
	u32 InitWait;			/**< Number of initial wait cycles at
						the start of a new line by
						the framing logic. */
	u32 BitsPerColor;		/**< Number of bits per color
						component. */
	u8 ComponentFormat;		/**< The component format currently in
						use by the video stream. */
	u8 DynamicRange;		/**< The dynamic range currently in use
						by the video stream. */
	u8 YCbCrColorimetry;		/**< The YCbCr colorimetry currently in
						use by the video stream. */
	u8 SynchronousClockMode;	/**< Synchronous clock mode is currently
						in use by the video stream. */
	u8 OverrideUserPixelWidth;	/**< If set to 1, the value stored for
						UserPixelWidth will be used as
						the pixel width. */
} XDptx_MainStreamAttributes;

/**
 * This typedef describes a stream when the driver is running in multi-stream
 * transport (MST) mode.
 */
typedef struct {
	u8 LinkCountTotal;		/** The total number of DisplayPort
						links from the DisplayPort TX to
						the sink device that this MST
						stream is targeting.*/
	u8 RelativeAddress[15];		/** The relative address from the
						DisplayPort TX to the sink
						device that this MST stream is
						targeting.*/
	u16 MstPbn;			/**< Payload bandwidth number used to
						allocate bandwidth for the MST
						stream. */
	u8 MstStreamEnable;		/**< In MST mode, enables the
						corresponding stream for this
						MSA configuration. */
} XDptx_MstStream;

/**
 * This typedef describes some board characteristics information that affects
 * link training.
 */
typedef struct {
	u8 HasRedriverInPath;	/**< Redriver in path requires different voltage
					swing and pre-emphasis. */
	u8 TxVsLevels[4];	/**< The voltage swing levels to be used by the
					DisplayPort TX. */
	u8 TxPeLevels[4];	/**< The pre-emphasis/cursor level to be used by
					the DisplayPort TX. */
	u8 TxVsOffset;		/**< Voltage swing compensation offset used when
					pre-emphasis is used. */
} XDptx_BoardChar;

/**
 * This typedef describes a downstream DisplayPort device when the driver is
 * running in multi-stream transport (MST) mode.
 */
typedef struct {
	u32 Guid[4];			/**< The global unique identifier (GUID)
						of the device. */
	u8 RelativeAddress[15];		/**< The relative address from the
						DisplayPort TX to this
						device. */
	u8 DeviceType;			/**< The type of DisplayPort device.
						Either a branch or sink. */
	u8 LinkCountTotal;		/**< The total number of DisplayPort
						links connecting this device to
						the DisplayPort TX. */
	u8 DpcdRev;			/**< The revision of the device's
						DisplayPort Configuration Data
						(DPCD). For this device to
						support MST features, this value
						must represent a protocl version
						greater or equal to 1.2. */
	u8 MsgCapStatus;		/**< This device is capable of sending
						and receiving sideband
						messages. */
} XDptx_TopologyNode;

/**
 * This typedef describes a the entire topology of connected downstream
 * DisplayPort devices (from the DisplayPort TX) when the driver is operating
 * in multi-stream transport (MST) mode.
 */
typedef struct {
	u8 NodeTotal;			/**< The total number of nodes that were
						found in the MST topology. */
	XDptx_TopologyNode NodeTable[63]; /**< A table listing all the nodes in
						the MST topology. */
	u8 SinkTotal;			/**< The total number of sinks in the
						MST topology. */
	XDptx_TopologyNode *SinkList[63]; /**< A pointer list of sinks in the
						MST topology. The entries will
						point to the sinks in the
						NodeTable. */
} XDptx_Topology;

/**
 * This typedef describes a port that is connected to a DisplayPort branch
 * device. This structure is used when the driver is operating in multi-stream
 * transport (MST) mode.
 */
typedef struct {
	u8 InputPort;			/**< Specifies that this port is an
						input port. */
	u8 PeerDeviceType;		/**< Specifies the device type connected
						to this port. */
	u8 PortNum;			/**< The port number of this port. */
	u8 MsgCapStatus;		/**< This port or the device at this
						port can send and receive MST
						messages. */
	u8 DpDevPlugStatus;		/**< There is a device connected to this
						port. */

	u8 LegacyDevPlugStatus;		/**< This port is connected to a legacy
						device. */
	u8 DpcdRev;			/**< The DisplayPort Configuration Data
						(DPCD) revision of the device
						connected to this port. */
	u32 Guid[4];			/**< The global unique identifier (GUID)
						of the device connected to this
						port. */
	u8 NumSdpStreams;		/**< The total number of Secondary-Data
						Packet (SDP) streams that this
						port can handle. */
	u8 NumSdpStreamSinks;		/**< The number of SDP streams
						associated with this port. */
} XDptx_SbMsgLinkAddressReplyPortDetail;

/**
 * This typedef describes a DisplayPort branch device. This structure is used
 * when the driver is operating in multi-stream transport (MST) mode.
 */
typedef struct {
	u8 ReplyType;			/**< The reply type of the sideband
						message. A value of 1 indicates
						that the request wasn't
						successful and the return data
						will give the reason for a
						negative-acknowledge (NACK). */
	u8 RequestId;			/**< The request identifier of the
						reply. This should correspond to
						the request identifier for the
						LINK_ADDRESS sideband message
						request. */
	u32 Guid[4];			/**< The global unique identifier (GUID)
						of the branch device. */
	u8 NumPorts;			/**< The number of ports associated with
						this branch device. */
	XDptx_SbMsgLinkAddressReplyPortDetail PortDetails[16]; /**< An array
						describing all ports attached to
						this branch device. */
} XDptx_SbMsgLinkAddressReplyDeviceInfo;

/******************************************************************************/
/**
 * Callback type which represents a custom timer wait handler. This is only
 * used for Microblaze since it doesn't have a native sleep function. To avoid
 * dependency on a hardware timer, the default wait functionality is implemented
 * using loop iterations; this isn't too accurate. If a custom timer handler is
 * used, the user may implement their own wait implementation using a hardware
 * timer (see example/) for better accuracy.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	MicroSeconds is the number of microseconds to be passed to the
 *		timer function.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDptx_TimerHandler)(void *InstancePtr, u32 MicroSeconds);

/******************************************************************************/
/**
 * Callback type which represents the handler for a Hot-Plug-Detect (HPD) event
 * interrupt.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDptx_HpdEventHandler)(void *InstancePtr);

/******************************************************************************/
/**
 * Callback type which represents the handler for a Hot-Plug-Detect (HPD) pulse
 * interrupt.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDptx_HpdPulseHandler)(void *InstancePtr);

/**
 * The XDptx driver instance data. The user is required to allocate a variable
 * of this type for every XDptx device in the system. A pointer to a variable of
 * this type is then passed to the driver API functions.
 */
typedef struct {
	u32 MstEnable;				/**< Multi-stream transport
							(MST) mode. Enables
							functionality, allowing
							multiple streams to be
							sent over the main
							link. */
	u32 IsReady;				/**< Device is initialized and
							ready. */
	u8 TrainAdaptive;			/**< Downshift lane count and
							link rate if necessary
							during training. */
	XDptx_Config Config;			/**< Configuration structure for
							the DisplayPort TX
							core. */
	XDptx_SinkConfig RxConfig;		/**< Configuration structure for
							the RX device. */
	XDptx_LinkConfig LinkConfig;		/**< Configuration structure for
							the main link. */
	XDptx_BoardChar BoardChar;		/**< Some board characteristics
							information that affects
							link training. */
	XDptx_MainStreamAttributes MsaConfig[4]; /**< Configuration structure
							for the main stream
							attributes (MSA). Each
							stream has its own set
							of attributes. When MST
							mode is disabled, only
							MsaConfig[0] is used. */
	XDptx_MstStream MstStreamConfig[4];	/**< Configuration structure
							for a multi-stream
							transport (MST)
							stream. */
	XDptx_Topology Topology;		/**< The topology of connected
							downstream DisplayPort
							devices when the driver
							is running in MST
							mode. */
	u32 AuxDelayUs;				/**< Amount of latency in micro-
							seconds to use between
							AUX transactions. */
	u32 SbMsgDelayUs;			/**< Amount of latency in micro-
							seconds to use between
							sideband messages for
							multi-stream transport
							(MST) mode. */
	XDptx_TimerHandler UserTimerWaitUs;	/**< Custom user function for
							delay/sleep. */
	void *UserTimerPtr;			/**< Pointer to a timer instance
							used by the custom user
							delay/sleep function. */
	XDptx_HpdEventHandler HpdEventHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) event
							interrupts. */
	void *HpdEventCallbackRef;		/**< A pointer to the user data
							passed to the HPD event
							callback function. */
	XDptx_HpdPulseHandler HpdPulseHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) pulse
							interrupts. */
	void *HpdPulseCallbackRef;		/**< A pointer to the user data
							passed to the HPD pulse
							callback function. */
} XDptx;

/**************************** Function Prototypes *****************************/

/* xdptx.c: Setup and initialization functions. */
u32 XDptx_InitializeTx(XDptx *InstancePtr);
void XDptx_CfgInitialize(XDptx *InstancePtr, XDptx_Config *ConfigPtr,
							u32 EffectiveAddr);
u32 XDptx_GetRxCapabilities(XDptx *InstancePtr);

/* xdptx.c: Link policy maker functions. */
u32 XDptx_CfgMainLinkMax(XDptx *InstancePtr);
u32 XDptx_EstablishLink(XDptx *InstancePtr);
u32 XDptx_CheckLinkStatus(XDptx *InstancePtr, u8 LaneCount);
void XDptx_EnableTrainAdaptive(XDptx *InstancePtr, u8 Enable);
void XDptx_SetHasRedriverInPath(XDptx *InstancePtr, u8 Set);
void XDptx_CfgTxVsOffset(XDptx *InstancePtr, u8 Offset);
void XDptx_CfgTxVsLevel(XDptx *InstancePtr, u8 Level, u8 TxLevel);
void XDptx_CfgTxPeLevel(XDptx *InstancePtr, u8 Level, u8 TxLevel);

/* xdptx.c: AUX transaction functions. */
u32 XDptx_AuxRead(XDptx *InstancePtr, u32 DpcdAddress, u32 BytesToRead,
								void *ReadData);
u32 XDptx_AuxWrite(XDptx *InstancePtr, u32 DpcdAddress, u32 BytesToWrite,
							void *WriteData);
u32 XDptx_IicRead(XDptx *InstancePtr, u8 IicAddress, u16 Offset,
					u16 BytesToRead, void *ReadData);
u32 XDptx_IicWrite(XDptx *InstancePtr, u8 IicAddress, u8 BytesToWrite,
							void *WriteData);

/* xdptx.c: Functions for controlling the link configuration. */
u32 XDptx_SetDownspread(XDptx *InstancePtr, u8 Enable);
u32 XDptx_SetEnhancedFrameMode(XDptx *InstancePtr, u8 Enable);
u32 XDptx_SetLaneCount(XDptx *InstancePtr, u8 LaneCount);
u32 XDptx_SetLinkRate(XDptx *InstancePtr, u8 LinkRate);
u32 XDptx_SetScrambler(XDptx *InstancePtr, u8 Enable);

/* xdptx.c: General usage functions. */
u32 XDptx_IsConnected(XDptx *InstancePtr);
void XDptx_EnableMainLink(XDptx *InstancePtr);
void XDptx_DisableMainLink(XDptx *InstancePtr);
void XDptx_ResetPhy(XDptx *InstancePtr, u32 Reset);
void XDptx_WaitUs(XDptx *InstancePtr, u32 MicroSeconds);
void XDptx_SetUserTimerHandler(XDptx *InstancePtr,
			XDptx_TimerHandler CallbackFunc, void *CallbackRef);

/* xdptx_spm.c: Stream policy maker functions. */
void XDptx_CfgMsaRecalculate(XDptx *InstancePtr, u8 Stream);
void XDptx_CfgMsaUseStandardVideoMode(XDptx *InstancePtr, u8 Stream,
						XVidC_VideoMode VideoMode);
void XDptx_CfgMsaUseEdidPreferredTiming(XDptx *InstancePtr, u8 Stream,
								u8 *Edid);
void XDptx_CfgMsaUseCustom(XDptx *InstancePtr, u8 Stream,
		XDptx_MainStreamAttributes *MsaConfigCustom, u8 Recalculate);
void XDptx_CfgMsaSetBpc(XDptx *InstancePtr, u8 Stream, u8 BitsPerColor);
void XDptx_CfgMsaEnSynchClkMode(XDptx *InstancePtr, u8 Stream, u8 Enable);
void XDptx_SetVideoMode(XDptx *InstancePtr, u8 Stream);
void XDptx_ClearMsaValues(XDptx *InstancePtr, u8 Stream);
void XDptx_SetMsaValues(XDptx *InstancePtr, u8 Stream);

/* xdptx_intr.c: Interrupt handling functions. */
void XDptx_SetHpdEventHandler(XDptx *InstancePtr,
			XDptx_HpdEventHandler CallbackFunc, void *CallbackRef);
void XDptx_SetHpdPulseHandler(XDptx *InstancePtr,
			XDptx_HpdPulseHandler CallbackFunc, void *CallbackRef);
void XDptx_HpdInterruptHandler(XDptx *InstancePtr);

/* xdptx_selftest.c: Self test function. */
u32 XDptx_SelfTest(XDptx *InstancePtr);

/* xdptx_sinit.c: Configuration extraction function.*/
XDptx_Config *XDptx_LookupConfig(u16 DeviceId);

/* xdptx_mst.c: Multi-stream transport (MST) functions for enabling or disabling
 * MST mode. */
void XDptx_MstCfgModeEnable(XDptx *InstancePtr);
void XDptx_MstCfgModeDisable(XDptx *InstancePtr);
u32 XDptx_MstCapable(XDptx *InstancePtr);
u32 XDptx_MstEnable(XDptx *InstancePtr);
u32 XDptx_MstDisable(XDptx *InstancePtr);

/* xdptx_mst.c: Multi-stream transport (MST) functions for enabling or disabling
 * MST streams and selecting their associated target sinks. */
void XDptx_MstCfgStreamEnable(XDptx *InstancePtr, u8 Stream);
void XDptx_MstCfgStreamDisable(XDptx *InstancePtr, u8 Stream);
u8 XDptx_MstStreamIsEnabled(XDptx *InstancePtr, u8 Stream);
void XDptx_SetStreamSelectFromSinkList(XDptx *InstancePtr, u8 Stream, u8
								SinkNum);
void XDptx_SetStreamSinkRad(XDptx *InstancePtr, u8 Stream, u8 LinkCountTotal,
							u8 *RelativeAddress);

/* xdptx_mst.c: Multi-stream transport (MST) functions related to MST topology
 * discovery and management. */
u32 XDptx_DiscoverTopology(XDptx *InstancePtr);
u32 XDptx_FindAccessibleDpDevices(XDptx *InstancePtr, u8 LinkCountTotal,
							u8 *RelativeAddress);
void XDptx_TopologySwapSinks(XDptx *InstancePtr, u8 Index0, u8 Index1);
void XDptx_TopologySortSinksByTiling(XDptx *InstancePtr);

/* xdptx_mst.c: Multi-stream transport (MST) functions for communicating
 * with downstream DisplayPort devices. */
u32 XDptx_RemoteDpcdRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData);
u32 XDptx_RemoteDpcdWrite(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData);
u32 XDptx_RemoteIicRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicAddress, u16 Offset, u16 BytesToRead,
	u8 *ReadData);
u32 XDptx_RemoteIicWrite(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicAddress, u8 BytesToWrite, u8 *WriteData);

/* xdptx_mst.c: Multi-stream transport (MST) functions related to MST stream
 * allocation. */
u32 XDptx_AllocatePayloadStreams(XDptx *InstancePtr);
u32 XDptx_AllocatePayloadVcIdTable(XDptx *InstancePtr, u8 VcId, u8 Ts);
u32 XDptx_ClearPayloadVcIdTable(XDptx *InstancePtr);

/* xdptx_mst.c: Multi-stream transport (MST) functions for issuing sideband
 * messages. */
u32 XDptx_SendSbMsgRemoteDpcdWrite(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData);
u32 XDptx_SendSbMsgRemoteDpcdRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData);
u32 XDptx_SendSbMsgRemoteIicWrite(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 BytesToWrite, u8 *WriteData);
u32 XDptx_SendSbMsgRemoteIicRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 Offset, u8 BytesToRead,
	u8 *ReadData);
u32 XDptx_SendSbMsgLinkAddress(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo);
u32 XDptx_SendSbMsgEnumPathResources(XDptx *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, u16 *AvailPbn, u16 *FullPbn);
u32 XDptx_SendSbMsgAllocatePayload(XDptx *InstancePtr, u8 LinkCountTotal,
					u8 *RelativeAddress, u8 VcId, u16 Pbn);
u32 XDptx_SendSbMsgClearPayloadIdTable(XDptx *InstancePtr);

/* xdptx_mst.c: Multi-stream transport (MST) utility functions. */
void XDptx_WriteGuid(XDptx *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u32 Guid[4]);
void XDptx_GetGuid(XDptx *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u32 *Guid);

/* xdptx_edid.c: EDID utility functions. */
u32 XDptx_GetEdid(XDptx *InstancePtr, u8 *Edid);
u32 XDptx_GetRemoteEdid(XDptx *InstancePtr, u8 LinkCountTotal,
						u8 *RelativeAddress, u8 *Edid);
u32 XDptx_GetEdidBlock(XDptx *InstancePtr, u8 *Data, u8 BlockNum);
u32 XDptx_GetRemoteEdidBlock(XDptx *InstancePtr, u8 *Data, u8 BlockNum,
					u8 LinkCountTotal, u8 *RelativeAddress);
u32 XDptx_GetRemoteEdidDispIdExt(XDptx *InstancePtr, u8 *Data,
					u8 LinkCountTotal, u8 *RelativeAddress);
u32 XDptx_GetDispIdDataBlock(u8 *DisplayIdRaw, u8 SectionTag,
							u8 **DataBlockPtr);
u32 XDptx_GetRemoteTiledDisplayDb(XDptx *InstancePtr, u8 *EdidExt,
		u8 LinkCountTotal, u8 *RelativeAddress, u8 **DataBlockPtr);

#endif /* XDPTX_H_ */
