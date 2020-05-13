/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp.h
 * @addtogroup dp_v7_3
 * @{
 * @details
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
 *	   - Prefix: XDp_Tx* and XDP_TX_*
 *	2) Specific to the DisplayPort RX mode of operation.
 *	   - Prefix: XDp_Rx* and XDP_RX_*
 *	3) Common to both DisplayPort modes of operation.
 *	   - Prefix: XDp_* and XDP_*
 *
 * <b>Driver description: TX mode of operation</b>
 *
 * The device driver enables higher-level software (e.g., an application) to
 * configure and control a DisplayPort TX soft IP, communicate and control an
 * RX device/sink monitor over the AUX channel, and to initialize and transmit
 * data streams over the main link. This driver follows the DisplayPort 1.2a
 * specification.
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
 * <b>Driver description: RX mode of operation</b>
 *
 * The device driver enables higher-level software (e.g., an application) to
 * configure and control a DisplayPort RX soft IP.
 *
 * This driver gives applications the ability to configure the RX using various
 * settings, handle and issue interrupts, and modify a subset of its DisplayPort
 * Configuration Data (DPCD) fields.
 *
 * Link training is done automatically by the hardware.
 *
 * <b>Interrupt processing</b>
 *
 * For the driver to process interrupts, the application must set up the
 * system's interrupt controller and connect the XDp_InterruptHandler function
 * to service interrupts. When an interrupt occurs, XDp_InterruptHandler will
 * check which mode of operation the DisplayPort core is running in, and will
 * call the appropriate interrupt handler for that core
 * (XDp_TxInterruptHandler or XDp_RxInterruptHandler - local to xdp_intr.c).
 *
 * <b>Interrupt processing: TX mode of operation</b>
 *
 * DisplayPort interrupts occur on the HPD signal line when the DisplayPort
 * cable is connected/disconnected or when the RX device sends a pulse. The user
 * hardware design must contain an interrupt controller which the DisplayPort
 * TX instance's interrupt signal is connected to. The user application must
 * enable interrupts in the system and set up the interrupt controller such that
 * the XDp_TxHpdInterruptHandler handler will service DisplayPort interrupts.
 * When the XDp_TxHpdInterruptHandler function is invoked, the handler will
 * identify what type of DisplayPort interrupt has occurred, and will call
 * either the HPD event handler function or the HPD pulse handler function,
 * depending on whether a an HPD event on an HPD pulse event occurred.
 *
 * The DisplayPort TX's XDP_TX_INTERRUPT_STATUS register indicates the type of
 * interrupt that has occurred, and the XDp_TxInterruptHandler will use this
 * information to decide which handler to call. An HPD event is identified if
 * bit XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK is set, and an HPD pulse is
 * identified from the XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK bit.
 *
 * The HPD event and HPD pulse handler may be set up by using the
 * XDp_TxSetCallback function and XDP_TX_HANDLER_HPDEVENT and
 * XDP_TX_HANDLER_HPDPULSE enumerations of the XDp_Tx_HandlerType type.
 *
 * <b>Interrupt processing: RX mode of operation</b>
 *
 * The DisplayPort RX driver may generate a pulse on the hot-plug-detect (HPD)
 * signal line using the XDp_RxGenerateHpdInterrupt function. This allows the RX
 * to send an interrupt to the upstream TX device, useful for signaling the TX
 * that it needs to do some checks for changes in downstream devices or a loss
 * of link training.
 *
 * For RX interrupt handling of HPD events or events that happen internal to the
 * RX, the user hardware design must contain an interrupt controller which the
 * DisplayPort RX instance's interrupt signal is connected to. The user
 * application must enable interrupts in the system and set up the interrupt
 * controller such that the XDp_RxInterruptHandler handler will service
 * interrupts. When the XDp_RxInterruptHandler function is invoked, the handler
 * will identify what type of interrupt has occurred, and will call the
 * appropriate interrupt handler.
 *
 * The DisplayPort RX's XDP_RX_INTERRUPT_CAUSE register indicates the type of
 * interrupt that has occured, and the XDp_RxInterruptHandler will use this
 * information to decide which handler to call.
 *
 * The handlers are set up using the XDp_RxSetIntr* functions.
 *
 * Specific interrupts may be enabled or disabled using the
 * XDp_RxInterruptEnable and XDp_RxInterruptDisable functions.
 *
 * <b>Multi-stream transport (MST) mode: TX mode of operation</b>
 *
 * The driver handles MST mode functionality in TX mode of operation, including
 * sideband messaging, topology discovery, virtual channel payload ID table
 * management, and directing streams to different sinks.
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
 * <b>Multi-stream transport (MST) mode: RX mode of operation</b>
 *
 * The driver handles MST mode functionality in RX mode of operation. The API
 * function, XDp_RxHandleDownReq, will read a sideband message from the down
 * request registers in the DisplayPort RX core, parse it, arbitrate control for
 * the sideband message type that was requested, format a reply, and send the
 * reply.
 *
 * The current version of the driver only supports a software representation of
 * a shallow topology - meaning virtual sinks are defined in the user
 * application and assigned to port numbers. The RX acts as a branch connected
 * to multiple sinks, but is not connected to another branch. Sideband messages
 * will then be handled for the targeted downstream sink.
 *
 * The user application creates the topology using the driver's API functions:
 * - XDp_RxSetIicMapEntry : Used to specify the I2C contents of a virtual sink.
 * - XDp_RxSetDpcdMap : Used to specify the DPCD of a virtual sink.
 * - XDp_RxMstSetPbn : Used to specify the available PBN of a virtual sink.
 * - XDp_RxMstSetPort : Used to specify how the sink will appear when the an
 *	upstream device sends a LINK_ADDRESS sideband request to the RX branch.
 * - XDp_RxMstSetInputPort : Used to specify the input port.
 * - XDp_RxMstExposePort : Used to enable the port by exposing it to incoming
 *	LINK_ADDRESS sideband requests.
 *
 * The driver will keep track of the topology in the structures:
 * - XDp_RxTopology : Stores topology information as a reply to LINK_ADDRESS
 *	requests, the virtual channel payload table, and port representations
 *	(XDp_RxPort[] type).
 * - XDp_RxPort : Stores the I2C map (XDp_RxIicMapEntry[] type), DPCD address
 *	space (XDp_RxDpcdMap type), PBN information, and whether or not the
 *	represented sink is exposed to upstream devices.
 * - XDp_RxIicMapEntry : Represents data stored at an associated I2C address.
 * - XDp_RxDpcdMap : Represents the DPCD of an associated port's sink.
 *
 * Note that the driver uses the topology's XDp_RxPort[] array such that the
 * indices match the port number that attaches the virtual sink to the RX
 * branch.
 *
 * The following sideband messages are supported:
 * - CLEAR_PAYLOAD_ID_TABLE
 * - LINK_ADDRESS
 * - REMOTE_I2C_READ
 * - REMOTE_DPCD_READ
 * - ENUM_PATH_RESOURCES
 * - ALLOCATE_PAYLOAD
 * Other sideband messages are replied to with NACK.
 *
 * <b>Audio</b>
 *
 * The driver in RX mode of operation may received audio info and extension
 * packets. When this happens, if interrupts are enabled, the appropriate
 * handlers will be invoked.
 * Control functions are available for enabling, disabling, and resetting audio
 * in the DisplayPort RX core.
 *
 * The TX driver does not handle audio. For an example as to how to configure
 * and transmit audio, examples/xdptx_audio_example.c illustrates the required
 * sequence in the TX mode of operation. The user will need to configure the
 * audio source connected to the Displayport TX instance and set up the audio
 * info frame as per user requirements.
 *
 * <b>Asserts</b>
 *
 * Asserts are used within all Xilinx drivers to enforce constraints on argument
 * values. Asserts can be turned off on a system-wide basis by defining, at
 * compile time, the NDEBUG identifier.  By default, asserts are turned on and
 * it is recommended that application developers leave asserts on during
 * development.
 *
 * <b>Limitations: TX mode of operation</b>
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
 * <b>Limitations: RX mode of operation</b>
 *
 * - Sideband messages that aren't supported as specified above will be NACK'ed.
 *
 * @note	For a 5.4Gbps link rate, a high performance 7 series FPGA is
 *		required with a speed grade of -2 or -3.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 2.0   als  06/08/15 Added MST functionality to RX. New APIs:
 *                         XDp_RxHandleDownReq, XDp_RxGetIicMapEntry,
 *                         XDp_RxSetIicMapEntry, XDp_RxSetDpcdMap,
 *                         XDp_RxMstExposePort, XDp_RxMstSetPort,
 *                         XDp_RxMstSetInputPort, XDp_RxMstSetPbn,
 *                         XDp_RxSetIntrDownReqHandler,
 *                         XDp_RxSetIntrDownReplyHandler,
 *                         XDp_RxSetIntrAudioOverHandler,
 *                         XDp_RxSetIntrPayloadAllocHandler,
 *                         XDp_RxSetIntrActRxHandler,
 *                         XDp_RxSetIntrCrcTestHandler
 *                     Added Intr*Handler and Intr*CallbackRef interrupt-related
 *                     members to XDp_Rx structure for:
 *                         DownReq, DownReply, AudioOver, PayloadAlloc, ActRx,
 *                         CrcTest
 *                     Added new data structures related to RX MST topology:
 *                         XDp_RxIicMapEntry, XDp_RxDpcdMap, XDp_RxPort,
 *                         XDp_RxTopology
 *                     Renamed XDp_Tx* to XDp_* to reflect commonality with RX
 *                     for:
 *                         XDp_TxSbMsgLinkAddressReplyPortDetail
 *                         XDp_TxSbMsgLinkAddressReplyDeviceInfo
 *                     GUID type change for ease of use:
 *                         'u32 Guid[4]' changed to 'u8 Guid[16]'
 *                     Added handlers and setter functions for HDCP and unplug
 *                     events.
 *                     Added callbacks for lane count changes, link rate changes
 *                     and pre-emphasis + voltage swing adjust requests.
 * 3.0   als  10/07/15 Added MSA callback.
 * 4.0   als  12/08/15 Added link rate and lane count validity check APIs:
 *                         XDp_IsLinkRateValid
 *                         XDp_IsLaneCountValid
 *                     XDp_TxAllocatePayloadVcIdTable now takes an additional
 *                     argument (StartTs, the starting timeslot).
 *                     Added RX API to get color depth of a given stream.
 *                         XDp_RxGetBpc
 *                     Added RX API to get color component format of a stream.
 *                         XDp_RxGetColorComponent
 *                     Added RX API to set end of line reset as appropriate.
 *                         XDp_RxSetLineReset
 *                     Added RX MST API to allocate payload from ISR:
 *                         XDp_RxAllocatePayloadStream
 * 5.0   als  05/16/16 Added APIs to set color encoding scheme.
 * 5.1   als  08/12/16 Updates to support 64-bit base addresses.
 *       ms   01/23/17 Added xil_printf statement in main function for all
 *                     examples to ensure that "Successfully ran" and "Failed"
 *                     strings are available in all examples. This is a fix
 *                     for CR-965028.
 *       ms   03/17/17 Modified readme.txt file in examples folder for doxygen
 *                     generation.
 * 5.3   ms   04/18/17 Modified tcl file to add suffix U for all macros
 *                     definitions of dp in xparameters.h
 * 6.0   tu   09/08/17 Added two interrupt handler that addresses driver's
 *                     internal callback function of application
 *                     DrvHpdEventHandler and DrvHpdPulseHandler
 * 6.0   tu   09/08/17 Added three interrupt handler that addresses callback
 *                     function of driver
 * 6.0   jb   02/19/19 Added Intr*Handler and Intr*CallbackRef interrupt-related
 *                     members to XDp_Rx structure for:
 *                     		Hdcp22AkeInitWr, Hdcp22AkeNoStoredKmWr,
 *                     		Hdcp22AkeStoredWr, Hdcp22LcInitWr,
 *                     		Hdcp22SkeSendEksWr, Hdcp22LinkIntegrityFail,
 *                     		Hdcp22HprimeReadDone, Hdcp22PairingReadDone
 *                     	Added new HDCP22 functions:
 *                     		XDp_GenerateCpIrq,
 *                     		XDp_EnableDisableHdcp22AuxDeffers
 *
 * </pre>
 *
*******************************************************************************/

#ifndef XDP_H_
/* Prevent circular inclusions by using protection macros. */
#define XDP_H_

/******************************* Include Files ********************************/

#include "xil_assert.h"
#include "xil_types.h"
#include "xdp_hw.h"
#include "xstatus.h"
#include "xvidc.h"

/****************************** Type Definitions ******************************/

/**
 * This typedef enumerates the RX and TX modes of operation for the DisplayPort
 * core.
 */
typedef enum {
	XDP_TX = 0,
	XDP_RX
} XDp_CoreType;

/**
 * This typedef enumerates the dynamic ranges available to the DisplayPort core.
 */
typedef enum {
	XDP_DR_VESA = 0,
	XDP_DR_CEA,
} XDp_DynamicRange;

/**
 * This typedef enumerates the handlers for the the DisplayPort Receiver.
 */
typedef enum {
	XDP_RX_HANDLER_VMCHANGE = 0,
	XDP_RX_HANDLER_PWRSTATECHANGE,
	XDP_RX_HANDLER_NOVIDEO,
	XDP_RX_HANDLER_VBLANK,
	XDP_RX_HANDLER_TRAININGLOST,
	XDP_RX_HANDLER_VIDEO,
	XDP_RX_HANDLER_AUD_INFOPKTRECV,
	XDP_RX_HANDLER_AUD_EXTPKTRECV,
	XDP_RX_HANDLER_TRAININGDONE,
	XDP_RX_HANDLER_BWCHANGE,
	XDP_RX_HANDLER_TP1,
	XDP_RX_HANDLER_TP2,
	XDP_RX_HANDLER_TP3,
	XDP_RX_HANDLER_TP4,
	XDP_RX_HANDLER_DOWNREQ,
	XDP_RX_HANDLER_DOWNREPLY,
	XDP_RX_HANDLER_AUD_PKTOVERFLOW,
	XDP_RX_HANDLER_PAYLOADALLOC,
	XDP_RX_HANDLER_ACT_SEQ,
	XDP_RX_HANDLER_CRC_TEST,
	XDP_RX_HANDLER_HDCP_DEBUG,
	XDP_RX_HANDLER_HDCP_AKSV,
	XDP_RX_HANDLER_HDCP_AN,
	XDP_RX_HANDLER_HDCP_AINFO,
	XDP_RX_HANDLER_HDCP_RO,
	XDP_RX_HANDLER_HDCP_BINFO,
	XDP_RX_HANDLER_UNPLUG,
	XDP_RX_HANDLER_ACCESS_LANE_SET,
	XDP_RX_HANDLER_ACCESS_LINK_QUAL,
	XDP_RX_HANDLER_ACCESS_ERR_COUNTER,
	XDP_RX_HANDLER_DRV_PWRSTATE,
	XDP_RX_HANDLER_DRV_NOVIDEO,
	XDP_RX_HANDLER_DRV_VIDEO,
#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
	XDP_RX_HANDLER_HDCP22_AKE_INIT,
	XDP_RX_HANDLER_HDCP22_AKE_NO_STORED_KM,
	XDP_RX_HANDLER_HDCP22_AKE_STORED_KM,
	XDP_RX_HANDLER_HDCP22_LC_INIT,
	XDP_RX_HANDLER_HDCP22_SKE_SEND_EKS,
	XDP_RX_HANDLER_HDCP22_HPRIME_READ_DONE,
	XDP_RX_HANDLER_HDCP22_PAIRING_READ_DONE,
	XDP_RX_HANDLER_HDCP22_STREAM_TYPE,
#endif
	XDP_RX_HANDLER_VBLANK_STREAM_2,
	XDP_RX_HANDLER_VBLANK_STREAM_3,
	XDP_RX_HANDLER_VBLANK_STREAM_4,
	XDP_RX_NUM_HANDLERS
} Dp_Rx_HandlerType;

/**
 * This typedef enumerates the handlers for the the DisplayPort Transmitter.
 */
typedef enum {
	XDP_TX_HANDLER_SETMSA = 0,
	XDP_TX_HANDLER_HPDEVENT,
	XDP_TX_HANDLER_DRV_HPDEVENT,
	XDP_TX_HANDLER_HPDPULSE,
	XDP_TX_HANDLER_DRV_HPDPULSE,
	XDP_TX_HANDLER_LANECNTCHANGE,
	XDP_TX_HANDLER_LINKRATECHANGE,
	XDP_TX_HANDLER_PEVSADJUST,
	XDP_TX_NUM_HANDLERS
} XDp_Tx_HandlerType;

/**
 * This typedef contains configuration information for the DisplayPort core.
 */
typedef struct {
	u16 DeviceId;		/**< Device instance ID. */
	UINTPTR BaseAddr;	/**< The base address of the core instance. */
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
					0 = v1.1a, 1 = v1.2, 2 = v1.4. */
	u8 IsRx;		/**< The type of DisplayPort core.
					0 = TX, 1 = RX. */
} XDp_Config;

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
} XDp_TxSinkConfig;

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
	u8 cr_done_cnt;			/**< The number of lanes done with
						clock recovery */
	u8 cr_done_oldstate;		/**< Restores the number of lanes done
						with clock recovery. */
} XDp_TxLinkConfig;

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
	u32 MVid;			/**< M value used to recover the video
						clock from the link clock. */
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
} XDp_TxMainStreamAttributes;

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
} XDp_TxMstStream;

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
} XDp_TxBoardChar;

/**
 * This typedef describes a downstream DisplayPort device when the driver is
 * running in multi-stream transport (MST) mode.
 */
typedef struct {
	u8 Guid[XDP_GUID_NBYTES];	/**< The global unique identifier (GUID)
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
} XDp_TxTopologyNode;

/**
 * This typedef describes a the entire topology of connected downstream
 * DisplayPort devices (from the DisplayPort TX) when the driver is operating
 * in multi-stream transport (MST) mode.
 */
typedef struct {
	u8 NodeTotal;			/**< The total number of nodes that were
						found in the MST topology. */
	XDp_TxTopologyNode NodeTable[63]; /**< A table listing all the nodes in
						the MST topology. */
	u8 SinkTotal;			/**< The total number of sinks in the
						MST topology. */
	XDp_TxTopologyNode *SinkList[63]; /**< A pointer list of sinks in the
						MST topology. The entries will
						point to the sinks in the
						NodeTable. */
} XDp_TxTopology;

/**
 * This typedef describes Audio InfoFrame packet.
 */
typedef struct
{
	u16 info_length;
	u8 type;
	u8 version;
	u8 length;
	u8 audio_coding_type;
	u8 audio_channel_count;
	u8 sampling_frequency;
	u8 sample_size;
	u8 level_shift;
	u8 downmix_inhibit;
	u8 channel_allocation;
} XDp_TxAudioInfoFrame;

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
	u8 Guid[XDP_GUID_NBYTES];	/**< The global unique identifier (GUID)
						of the device connected to this
						port. */
	u8 NumSdpStreams;		/**< The total number of Secondary-Data
						Packet (SDP) streams that this
						port can handle. */
	u8 NumSdpStreamSinks;		/**< The number of SDP streams
						associated with this port. */
} XDp_SbMsgLinkAddressReplyPortDetail;

/**
 * This typedef describes a DisplayPort branch device. This structure is used
 * when the driver is operating in multi-stream transport (MST) mode.
 */
typedef struct {
	u8 Guid[XDP_GUID_NBYTES];	/**< The global unique identifier (GUID)
						of the branch device. */
	u8 NumPorts;			/**< The number of ports associated with
						this branch device. */
	XDp_SbMsgLinkAddressReplyPortDetail PortDetails[XDP_MAX_NPORTS]; /**< An
						array describing all ports
						attached to this branch
						device. */
} XDp_SbMsgLinkAddressReplyDeviceInfo;

/**
 * This typedef contains configuration information about the main link settings.
 */
typedef struct {
	u8 LaneCount;			/**< The current lane count of the main
						link. */
	u8 LinkRate;			/**< The current link rate of the main
						link. */
} XDp_RxLinkConfig;

/**
 * This typedef represents one I2C map entry for a device. This is used to allow
 * the user to define an I2C map for any port.
 */
typedef struct {
	u8 IicAddress;			/**< The I2C address for the which to
						link the data to. */
	u8 WriteVal;			/**< The last value written to this I2C
						address. This may be used when
						the TX writes to this I2C
						address in order to set a read
						offset or set the segment
						pointer. */
	u16 ReadNumBytes;		/**< The total number of available data
						bytes at this I2C address. */
	u8 *ReadData;			/**< The data available at the specified
						I2C address. User-defined by
						setting the pointer to a
						structure residing in the
						application. */
} XDp_RxIicMapEntry;

/**
 * This typedef represents the DPCD address map for a device. This is used to
 * allow the user to define a DPCD map for any sink connected to one of the RX's
 * ports.
 */
typedef struct {
	u8 *DataPtr;
	u32 NumBytes;
	u32 StartAddr;
} XDp_RxDpcdMap;

/**
 * This typedef contains information on the directly connected ports to the RX
 * branch. Information contained in XDp_SbMsgLinkAddressReplyDeviceInfo is not
 * duplicated here.
 */
typedef struct {
	XDp_RxIicMapEntry IicMap[XDP_RX_NUM_I2C_ENTRIES_PER_PORT]; /**< When the
						RX replies to a REMOTE_I2C_READ
						sideband message, it responds
						with the associated I2C map for
						the requested port. The driver
						allows the user to define up to
						XDP_RX_NUM_I2C_ENTRIES_PER_PORT
						I2C addresses per port. */
	XDp_RxDpcdMap DpcdMap;		/**< When the RX replies to a
						REMOTE_DPCD_READ sideband
						message, it responds with the
						associated DPCD map for the
						requested port. */
	u16 FullPbn;			/**< The payload bandwidth number (PBN)
						associated with the sink
						connected to this port. */
	u16 AvailPbn;			/**< The available PBN of the sink
						connected to this port. */
	u8 Exposed;			/**< When set to 1, the RX branch device
						will expose the port in the
						LINK_ADDRESS reply. */
} XDp_RxPort;

/**
 * This typedef contains topology information on directly connected sinks and of
 * the RX branch itself.
 */
typedef struct {
	XDp_SbMsgLinkAddressReplyDeviceInfo LinkAddressInfo; /**< Topology
						information used by the RX to
						form the LINK_ADDRESS reply. */
	u8 PayloadTable[64];		/**< The payload table of the RX
						representing allocated bandwidth
						per stream. */
	XDp_RxPort Ports[XDP_MAX_NPORTS]; /**< Port information additional to
						that contained in
						LinkAddressInfo. */
} XDp_RxTopology;

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

/******************************************************************************/
/**
 * Callback type which represents the handler for interrupts.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDp_IntrHandler)(void *InstancePtr);

/**
 * The XDp driver instance data representing the TX mode of operation.
 */
typedef struct {
	u32 MstEnable;				/**< Multi-stream transport
							(MST) mode. Enables
							functionality, allowing
							multiple streams to be
							sent over the main
							link. */
	u8 TrainAdaptive;			/**< Downshift lane count and
							link rate if necessary
							during training. */
	u8 IsTps4Supported;		/**< Is TPS4 supported by the
							downstream sink */
	XDp_TxSinkConfig RxConfig;		/**< Configuration structure for
							the RX device. */
	XDp_TxLinkConfig LinkConfig;		/**< Configuration structure for
							the main link. */
	XDp_TxBoardChar BoardChar;		/**< Some board characteristics
							information that affects
							link training. */
	XDp_TxMainStreamAttributes MsaConfig[4]; /**< Configuration structure
							for the main stream
							attributes (MSA). Each
							stream has its own set
							of attributes. When MST
							mode is disabled, only
							MsaConfig[0] is used. */
	XDp_TxMstStream MstStreamConfig[4];	/**< Configuration structure
							for a multi-stream
							transport (MST)
							stream. */
	XDp_TxTopology Topology;		/**< The topology of connected
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
	XDp_IntrHandler TxSetMsaCallback;	/**< Callback function for
							setting the TX MSA. */
	void *TxMsaCallbackRef;			/**< A pointer to the user data
							passed to the TX MSA
							callback function. */
	XDp_IntrHandler HpdEventHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) event
							interrupts. */
	void *HpdEventCallbackRef;		/**< A pointer to the user data
							passed to the HPD event
							callback function. */
	XDp_IntrHandler DrvHpdEventHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) event
							interrupts. */
	void *DrvHpdEventCallbackRef;		/**< A pointer to the user data
							passed to the HPD event
							callback function. */
	XDp_IntrHandler HpdPulseHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) pulse
							interrupts. */
	void *HpdPulseCallbackRef;		/**< A pointer to the user data
							passed to the HPD pulse
							callback function. */
	XDp_IntrHandler DrvHpdPulseHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) pulse
							interrupts. */
	void *DrvHpdPulseCallbackRef;		/**< A pointer to the user data
							passed to the HPD pulse
							callback function. */
	XDp_IntrHandler LaneCountChangeCallback; /** Callback function to be
							invoked once a lane
							count change has
							occurred within the
							driver. */
	void *LaneCountChangeCallbackRef;	/** A pointer to the user data
							passed to the lane count
							change callback
							function. */
	XDp_IntrHandler LinkRateChangeCallback;	/**< Callback function to be
							invoked once a link
							rate change has
							occurred within the
							driver. */
	void *LinkRateChangeCallbackRef;	/** A pointer to the user data
							passed to the link rate
							change callback
							function. */
	XDp_IntrHandler PeVsAdjustCallback;	/** Callback function to be
							invoked once a voltage
							swing and pre-emphasis
							adjust request has been
							handled within the
							driver. */
	void *PeVsAdjustCallbackRef;		/** A pointer to the user data
							passed to the voltage
							swing and pre-emphasis
							adjust request callback
							function. */
} XDp_Tx;

/**
 * The XDp driver instance data representing the RX mode of operation.
 */
typedef struct {
	XDp_RxTopology Topology;		/**< Topology of connected sinks
							to the RX. */
	XDp_RxLinkConfig LinkConfig;		/**< Configuration structure for
							the main link. */
	XDp_IntrHandler IntrVmChangeHandler;	/**< Callback function for video
							mode change
							interrupts. */
	void *IntrVmChangeCallbackRef;		/**< A pointer to the user data
							passed to the video mode
							change callback
							function. */
	XDp_IntrHandler IntrPowerStateHandler;	/**< Callback function for
							power state change
							interrupts. */
	void *IntrPowerStateCallbackRef;	/**< A pointer to the user data
							passed to the power
							state change callback
							function. */
	XDp_IntrHandler IntrNoVideoHandler;	/**< Callback function for
							no video interrupts. */
	void *IntrNoVideoCallbackRef;		/**< A pointer to the user data
							passed to the no video
							callback function. */
	XDp_IntrHandler IntrVBlankHandler[XDP_RX_STREAM_ID4];	/**< Array of
							callback functions
							for vertical blanking
							interrupts for all
							streams*/
	void *IntrVBlankCallbackRef[XDP_RX_STREAM_ID4];	/**< An array of pointer
							  to the user data
							  passed to the vertical
							  blanking callback
							  functions. */
	XDp_IntrHandler IntrTrainingLostHandler;/**< Callback function for
							training lost
							interrupts. */
	void *IntrTrainingLostCallbackRef;	/**< A pointer to the user data
							passed to the training
							lost callback
							function. */
	XDp_IntrHandler IntrVideoHandler;	/**< Callback function for valid
							video interrupts. */
	void *IntrVideoCallbackRef;		/**< A pointer to the user data
							passed to the valid
							video callback
							function. */
	XDp_IntrHandler IntrInfoPktHandler;	/**< Callback function for audio
							info packet received
							interrupts. */
	void *IntrInfoPktCallbackRef;		/**< A pointer to the user data
							passed to the audio info
							packet callback
							function. */
	XDp_IntrHandler IntrExtPktHandler;	/**< Callback function for audio
							extension packet
							received interrupts. */
	void *IntrExtPktCallbackRef;		/**< A pointer to the user data
							passed to the audio
							extension packet
							callback function. */
	XDp_IntrHandler IntrTrainingDoneHandler; /**< Callback function for
							training done
							interrupts. */
	void *IntrTrainingDoneCallbackRef;	/**< A pointer to the user data
							passed to the training
							done callback
							function. */
	XDp_IntrHandler IntrBwChangeHandler;	/**< Callback function for
							bandwidth change
							interrupts. */
	void *IntrBwChangeCallbackRef;		/**< A pointer to the user data
							passed to the bandwidth
							change callback
							function. */
	XDp_IntrHandler IntrTp1Handler;		/**< Callback function for
							training pattern 1
							interrupts. */
	void *IntrTp1CallbackRef;		/**< A pointer to the user data
							passed to the training
							pattern 1 callback
							function. */
	XDp_IntrHandler IntrTp2Handler;		/**< Callback function for
							training pattern 2
							interrupts. */
	void *IntrTp2CallbackRef;		/**< A pointer to the user data
							passed to the training
							pattern 2 callback
							function. */
	XDp_IntrHandler IntrTp3Handler;		/**< Callback function for
							training pattern 3
							interrupts. */
	void *IntrTp3CallbackRef;		/**< A pointer to the user data
							passed to the training
							pattern 3 callback
							function. */
	/* Interrupt callback(s) defined for DP 1.4 */
	XDp_IntrHandler IntrTp4Handler;		/**< Callback function for
							training pattern 4
							interrupts. */
	void *IntrTp4CallbackRef;		/**< A pointer to the user data
							passed to the training
							pattern 4 callback
							function. */
	/* End of definitions for DP 1.4 interrupt callback(s) */
	XDp_IntrHandler IntrDownReqHandler;	/**< Callback function for down
							request interrupts. */
	void *IntrDownReqCallbackRef;		/**< A pointer to the user data
							passed to the down
							request callback
							function. */
	XDp_IntrHandler IntrDownReplyHandler;	/**< Callback function for down
							reply interrupts. */
	void *IntrDownReplyCallbackRef;		/**< A pointer to the user data
							passed to the down
							reply callback
							function. */
	XDp_IntrHandler IntrAudioOverHandler;	/**< Callback function for audio
							packet overflow
							interrupts. */
	void *IntrAudioOverCallbackRef;		/**< A pointer to the user data
							passed to the audio
							packet overflow callback
							function. */
	XDp_IntrHandler IntrPayloadAllocHandler; /**< Callback function for
							payload allocation
							interrupts. */
	void *IntrPayloadAllocCallbackRef;	/**< A pointer to the user data
							passed to the payload
							allocation callback
							function. */
	XDp_IntrHandler IntrActRxHandler;	/**< Callback function for ACT
							sequence received
							interrupts. */
	void *IntrActRxCallbackRef;		/**< A pointer to the user data
							passed to the ACT
							sequence received
							callback function. */
	XDp_IntrHandler IntrCrcTestHandler;	/**< Callback function for CRC
							test start
							interrupts. */
	void *IntrCrcTestCallbackRef;		/**< A pointer to the user data
							passed to the CRC test
							start callback
							function. */
	XDp_IntrHandler IntrHdcpDbgWrHandler;	/**< Callback function for
							HDCP debug register
							write interrupts. */
	void *IntrHdcpDbgWrCallbackRef;		/**< A pointer to the user data
							passed to the hdcp
							debug register write
							callback function. */
	XDp_IntrHandler IntrHdcpAksvWrHandler;	/**< Callback function for
							HDCP Aksv MSB register
							write interrupts. */
	void *IntrHdcpAksvWrCallbackRef;	/**< A pointer to the user data
							passed to the HDCP
							Aksv MSB register write
							callback function. */
	XDp_IntrHandler IntrHdcpAnWrHandler;	/**< Callback function for
							HDCP An MSB register
							write interrupts. */
	void *IntrHdcpAnWrCallbackRef;		/**< A pointer to the user data
							passed to the HDCP
							An MSB register write
							callback function. */
	XDp_IntrHandler IntrHdcpAinfoWrHandler;	/**< Callback function for
							HDCP Ainfo register
							write interrupts. */
	void *IntrHdcpAinfoWrCallbackRef;	/**< A pointer to the user data
							passed to the HDCP
							Ainfo register write
							callback function. */
	XDp_IntrHandler IntrHdcpRoRdHandler;	/**< Callback function for
							HDCP Ro register
							read interrupts. */
	void *IntrHdcpRoRdCallbackRef;		/**< A pointer to the user data
							passed to the HDCP
							Ro register read
							callback function. */
	XDp_IntrHandler IntrHdcpBinfoRdHandler;	/**< Callback function for
							HDCP Binfo register
							read interrupts. */
	void *IntrHdcpBinfoRdCallbackRef;	/**< A pointer to the user data
							passed to the HDCP
							Binfo register read
							callback function. */
#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
	XDp_IntrHandler IntrHdcp22AkeInitWrHandler;	/**< Callback function
							  for HDCP22 Ake_Init
							  register write
							  interrupts. */
	void *IntrHdcp22AkeInitWrCallbackRef;	/**< A pointer to the user data
						  passed to the HDCP22
						  Ake_Init register write
						  callback function. */
	XDp_IntrHandler IntrHdcp22AkeNoStoredKmWrHandler;/**< Callback function
							   for HDCP22
							   Ake_No_Stored_Km
							   register write
							   interrupts. */
	void *IntrHdcp22AkeNoStoredKmWrCallbackRef;	/**< A pointer to the
							  user data passed to
							  the HDCP22
							  Ake_No_Stored_Km
							  register write
							  callback function. */
	XDp_IntrHandler IntrHdcp22AkeStoredWrHandler;	/**< Callback function
							  for HDCP22
							  Ake_Stored_Km
							  register write
							  interrupts. */
	void *IntrHdcp22AkeStoredWrCallbackRef;	/**< A pointer to the user data
						  passed to the HDCP22
						  Ake_Stored register write
						  callback function. */
	XDp_IntrHandler IntrHdcp22LcInitWrHandler;	/**< Callback function
							  for HDCP22 Lc_Init
							  register write
							  interrupts. */
	void *IntrHdcp22LcInitWrCallbackRef;	/**< A pointer to the user data
						  passed to the HDCP22
						  Lc_Init register write
						  callback function. */
	XDp_IntrHandler IntrHdcp22SkeSendEksWrHandler;	/**< Callback function
							  for HDCP22
							  Ske_Send_Eks register
							  write interrupts. */
	void *IntrHdcp22SkeSendEksWrCallbackRef;	/**< A pointer to the
							  user data passed to
							  the HDCP22
							  Ske_Send_Eks
							  register write
							  callback function. */
	XDp_IntrHandler IntrHdcp22HprimeReadDoneHandler;	/**< Callback
								  function for
								  HDCP22 H'
								  Read Done
								  interrupts. */
	void *IntrHdcp22HprimeReadDoneCallbackRef;	/**< A pointer to the
							  user data passed to
							  the HDCP22
							  HprimeReadDone
							  callback function. */
	XDp_IntrHandler IntrHdcp22PairingReadDoneHandler;	/**< Callback
								  function for
								  HDCP22 Pairing
								  Info read Done
								  interrupts. */
	void *IntrHdcp22PairingReadDoneCallbackRef;	/**< A pointer to the
							  user data passed to
							  the HDCP22
							  PairingReadDone
							  callback function. */
	XDp_IntrHandler IntrHdcp22StreamTypeWrHandler;	/**< Callback function
							  for HDCP22 stream
							  Type write
							  interrupts. */
	void *IntrHdcp22StreamTypeWrCallbackRef;	/**< A pointer to the
							  user data passed to
							  the HDCP22
							  stream Type write
							  callback function. */
#endif

	XDp_IntrHandler IntrUnplugHandler;	/**< Callback function for
							unplug interrupts. */
	void *IntrUnplugCallbackRef;		/**< A pointer to the user data
							passed to the unplug
							callback function. */
	XDp_IntrHandler IntrDrvPowerStateHandler; /**< Callback function for
						    driver power state
						    interrupt */
	void *IntrDrvPowerStateCallbackRef;	/**< A pointer to the user data
						  passed to the power
						  state drv function */
	XDp_IntrHandler IntrDrvNoVideoHandler;	/**< Callback function for
						  driver no video
						  interrupts. */
	void *IntrDrvNoVideoCallbackRef;	/**< A pointer to the user data
						  passed to the no video
						  drv function */
	XDp_IntrHandler IntrDrvVideoHandler;	/**< Callback function for
						  driver video
						  interrupts. */
	void *IntrDrvVideoCallbackRef;		/**< A pointer to the user data
						  passed to the video
						  drv function */
	/* Interrupt callback(s) defined for DP 1.4 */
	XDp_IntrHandler IntrAccessLaneSetHandler; /**< Callback function for
						  access lane set
						  interrupts. */
	void *IntrAccessLaneSetCallbackRef; 	  /**< A pointer to the user
						  data passed to the access
						  lane set callback */
	XDp_IntrHandler IntrAccessLinkQualHandler; /**< Callback function for
						   access link qual
						   interrupts. */
	void *IntrAccessLinkQualCallbackRef; 	   /**< A pointer to the user
						   data passed to the access
						   lane set callback
						   function. */
	XDp_IntrHandler IntrAccessErrorCounterHandler;  /**< Callback function
							for access error counter
							interrupts. */
	void *IntrAccessErrorCounterCallbackRef;    /**< A pointer to the user
						    data passed to the access
						    lane set callback
						    function. */
	/* End of definitions for DP 1.4 interrupt callback(s) */
} XDp_Rx;

/**
 * The XDp instance data. The user is required to allocate a variable of this
 * type for every XDp device in the system. A pointer to a variable of this type
 * is then passed to the driver API functions.
 */
typedef struct {
	XDp_Config Config;			/**< Configuration structure for
							the DisplayPort TX
							core. */
	u32 IsReady;				/**< Device is initialized and
							ready. */
	XDp_TimerHandler UserTimerWaitUs;	/**< Custom user function for
							delay/sleep. */
	void *UserTimerPtr;			/**< Pointer to a timer instance
							used by the custom user
							delay/sleep function. */
	union {
		XDp_Tx TxInstance;
		XDp_Rx RxInstance;
	};
} XDp;

/**************************** Function Prototypes *****************************/

/* xdp_sinit.c: Configuration extraction function. */
XDp_Config *XDp_LookupConfig(u16 DeviceId);

/* xdp.c: Setup and initialization functions. */
void XDp_CfgInitialize(XDp *InstancePtr, XDp_Config *ConfigPtr,
							UINTPTR EffectiveAddr);
u32 XDp_Initialize(XDp *InstancePtr);
u32 XDp_TxGetRxCapabilities(XDp *InstancePtr);
/* Defined for DP 1.4 */
u32 XDp_TxTp4Capable(XDp *InstancePtr);

#if XPAR_XDPTXSS_NUM_INSTANCES
/* xdp.c: TX link policy maker functions. */
u32 XDp_TxCfgMainLinkMax(XDp *InstancePtr);
u32 XDp_TxEstablishLink(XDp *InstancePtr);
u32 XDp_TxCheckLinkStatus(XDp *InstancePtr, u8 LaneCount);
void XDp_TxEnableTrainAdaptive(XDp *InstancePtr, u8 Enable);
void XDp_TxSetHasRedriverInPath(XDp *InstancePtr, u8 Set);
void XDp_TxCfgTxVsOffset(XDp *InstancePtr, u8 Offset);
void XDp_TxCfgTxVsLevel(XDp *InstancePtr, u8 Level, u8 TxLevel);
void XDp_TxCfgTxPeLevel(XDp *InstancePtr, u8 Level, u8 TxLevel);

/* xdp.c: TX AUX transaction functions. */
u32 XDp_TxAuxRead(XDp *InstancePtr, u32 DpcdAddress, u32 BytesToRead,
								void *ReadData);
u32 XDp_TxAuxWrite(XDp *InstancePtr, u32 DpcdAddress, u32 BytesToWrite,
							void *WriteData);
u32 XDp_TxIicRead(XDp *InstancePtr, u8 IicAddress, u16 Offset,
					u16 BytesToRead, void *ReadData);
u32 XDp_TxIicWrite(XDp *InstancePtr, u8 IicAddress, u8 BytesToWrite,
							void *WriteData);

/* xdp.c: TX functions for controlling the link configuration. */
u32 XDp_TxSetDownspread(XDp *InstancePtr, u8 Enable);
u32 XDp_TxSetEnhancedFrameMode(XDp *InstancePtr, u8 Enable);
u32 XDp_TxSetLaneCount(XDp *InstancePtr, u8 LaneCount);
u32 XDp_TxSetLinkRate(XDp *InstancePtr, u8 LinkRate);
u32 XDp_TxSetScrambler(XDp *InstancePtr, u8 Enable);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/* xdp.c: General usage functions. */
void XDp_SetUserTimerHandler(XDp *InstancePtr,
			XDp_TimerHandler CallbackFunc, void *CallbackRef);
void XDp_WaitUs(XDp *InstancePtr, u32 MicroSeconds);

#if XPAR_XDPTXSS_NUM_INSTANCES
u32 XDp_TxIsConnected(XDp *InstancePtr);
void XDp_TxEnableMainLink(XDp *InstancePtr);
void XDp_TxDisableMainLink(XDp *InstancePtr);
void XDp_TxResetPhy(XDp *InstancePtr, u32 Reset);
void XDp_TxSetPhyPolarityAll(XDp *InstancePtr, u8 Polarity);
void XDp_TxSetPhyPolarityLane(XDp *InstancePtr, u8 Lane, u8 Polarity);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
u32 XDp_RxCheckLinkStatus(XDp *InstancePtr);
void XDp_RxDtgEn(XDp *InstancePtr);
void XDp_RxDtgDis(XDp *InstancePtr);
void XDp_RxSetLinkRate(XDp *InstancePtr, u8 LinkRate);
void XDp_RxSetLaneCount(XDp *InstancePtr, u8 LaneCount);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

u8 XDp_IsLaneCountValid(XDp *InstancePtr, u8 LaneCount);
u8 XDp_IsLinkRateValid(XDp *InstancePtr, u8 LinkRate);

#if XPAR_XDPRXSS_NUM_INSTANCES
/* xdp.c: Audio functions. */
void XDp_RxAudioEn(XDp *InstancePtr);
void XDp_RxAudioDis(XDp *InstancePtr);
void XDp_Rx_Mst_AudioEn(XDp *InstancePtr, u8 StreamId);
void XDp_RxAudioReset(XDp *InstancePtr);
void XDp_RxVSCEn(XDp *InstancePtr);
void XDp_RxVSCDis(XDp *InstancePtr);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/* xdp_edid.c: EDID utility functions. */
u32 XDp_TxGetEdid(XDp *InstancePtr, u8 *Edid);
u32 XDp_TxGetRemoteEdid(XDp *InstancePtr, u8 LinkCountTotal,
						u8 *RelativeAddress, u8 *Edid);
u32 XDp_TxGetEdidBlock(XDp *InstancePtr, u8 *Data, u8 BlockNum);
u32 XDp_TxGetRemoteEdidBlock(XDp *InstancePtr, u8 *Data, u8 BlockNum,
					u8 LinkCountTotal, u8 *RelativeAddress);
u32 XDp_TxGetRemoteEdidDispIdExt(XDp *InstancePtr, u8 *Data,
					u8 LinkCountTotal, u8 *RelativeAddress);
u32 XDp_TxGetDispIdDataBlock(u8 *DisplayIdRaw, u8 SectionTag,
							u8 **DataBlockPtr);
u32 XDp_TxGetRemoteTiledDisplayDb(XDp *InstancePtr, u8 *EdidExt,
		u8 LinkCountTotal, u8 *RelativeAddress, u8 **DataBlockPtr);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/* xdp_intr.c: Interrupt handling functions. */
void XDp_InterruptHandler(XDp *InstancePtr);
#if XPAR_XDPTXSS_NUM_INSTANCES
int XDp_TxSetCallback(XDp *InstancePtr,	XDp_Tx_HandlerType HandlerType,
			XDp_IntrHandler CallbackFunc, void *CallbackRef);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
void XDp_RxGenerateHpdInterrupt(XDp *InstancePtr, u16 DurationUs);
void XDp_RxInterruptEnable(XDp *InstancePtr, u32 Mask);
void XDp_RxInterruptDisable(XDp *InstancePtr, u32 Mask);
int XDp_RxSetCallback(XDp *InstancePtr,	Dp_Rx_HandlerType HandlerType,
			XDp_IntrHandler CallbackFunc, void *CallbackRef);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/* xdp_mst.c: Multi-stream transport (MST) functions for enabling or disabling
 * MST mode. */
void XDp_TxMstCfgModeEnable(XDp *InstancePtr);
void XDp_TxMstCfgModeDisable(XDp *InstancePtr);
u32 XDp_TxMstCapable(XDp *InstancePtr);
u32 XDp_TxMstEnable(XDp *InstancePtr);
u32 XDp_TxMstDisable(XDp *InstancePtr);
void XDp_TxAudioDis(XDp *InstancePtr);
void XDp_Tx_Mst_AudioEn(XDp *InstancePtr, u8 StreamId);
void XDp_TxSendAudioInfoFrame(XDp *InstancePtr,
		XDp_TxAudioInfoFrame *xilInfoFrame);

/* xdp_mst.c: Multi-stream transport (MST) functions for enabling or disabling
 * MST streams and selecting their associated target sinks. */
void XDp_TxMstCfgStreamEnable(XDp *InstancePtr, u8 Stream);
void XDp_TxMstCfgStreamDisable(XDp *InstancePtr, u8 Stream);
u8 XDp_TxMstStreamIsEnabled(XDp *InstancePtr, u8 Stream);
void XDp_TxSetStreamSelectFromSinkList(XDp *InstancePtr, u8 Stream, u8 SinkNum);
void XDp_TxSetStreamSinkRad(XDp *InstancePtr, u8 Stream, u8 LinkCountTotal,
							u8 *RelativeAddress);

/* xdp_mst.c: Multi-stream transport (MST) functions related to MST topology
 * discovery and management. */
u32 XDp_TxDiscoverTopology(XDp *InstancePtr);
u32 XDp_TxFindAccessibleDpDevices(XDp *InstancePtr, u8 LinkCountTotal,
							u8 *RelativeAddress);
void XDp_TxTopologySwapSinks(XDp *InstancePtr, u8 Index0, u8 Index1);
void XDp_TxTopologySortSinksByTiling(XDp *InstancePtr);

/* xdp_mst.c: Multi-stream transport (MST) functions for communicating
 * with downstream DisplayPort devices. */
u32 XDp_TxRemoteDpcdRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData);
u32 XDp_TxRemoteDpcdWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData);
u32 XDp_TxRemoteIicRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicAddress, u16 Offset, u16 BytesToRead,
	u8 *ReadData);
u32 XDp_TxRemoteIicWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicAddress, u8 BytesToWrite, u8 *WriteData);

/* xdp_mst.c: Multi-stream transport (MST) functions related to MST stream
 * allocation. */
u32 XDp_TxAllocatePayloadStreams(XDp *InstancePtr);
u32 XDp_TxAllocatePayloadVcIdTable(XDp *InstancePtr, u8 VcId, u8 Ts,
		u8 StartTs);
u32 XDp_TxClearPayloadVcIdTable(XDp *InstancePtr);

/* xdp_mst.c: Multi-stream transport (MST) functions for issuing sideband
 * messages. */
u32 XDp_TxSendSbMsgRemoteDpcdWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData);
u32 XDp_TxSendSbMsgRemoteDpcdRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData);
u32 XDp_TxSendSbMsgRemoteIicWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 BytesToWrite, u8 *WriteData);
u32 XDp_TxSendSbMsgRemoteIicRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 Offset, u8 BytesToRead,
	u8 *ReadData);
u32 XDp_TxSendSbMsgLinkAddress(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, XDp_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo);
u32 XDp_TxSendSbMsgEnumPathResources(XDp *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, u16 *AvailPbn, u16 *FullPbn);
u32 XDp_TxSendSbMsgAllocatePayload(XDp *InstancePtr, u8 LinkCountTotal,
					u8 *RelativeAddress, u8 VcId, u16 Pbn);
u32 XDp_TxSendSbMsgClearPayloadIdTable(XDp *InstancePtr);

/* xdp_mst.c: Multi-stream transport (MST) utility functions. */
void XDp_TxWriteGuid(XDp *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u8 *Guid);
void XDp_TxGetGuid(XDp *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u8 *Guid);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
u32 XDp_RxHandleDownReq(XDp *InstancePtr);
XDp_RxIicMapEntry *XDp_RxGetIicMapEntry(XDp *InstancePtr, u8 PortNum,
								u8 IicAddress);
u32 XDp_RxSetIicMapEntry(XDp *InstancePtr, u8 PortNum, u8 IicAddress,
						u16 ReadNumBytes, u8 *ReadData);
void XDp_RxSetDpcdMap(XDp *InstancePtr, u8 PortNum, u32 StartAddr, u32 NumBytes,
								u8 *DpcdMap);
void XDp_RxMstExposePort(XDp *InstancePtr, u8 PortNum, u8 Expose);
void XDp_RxMstSetPort(XDp *InstancePtr, u8 PortNum,
			XDp_SbMsgLinkAddressReplyPortDetail *PortDetails);
void XDp_RxMstSetInputPort(XDp *InstancePtr, u8 PortNum,
			XDp_SbMsgLinkAddressReplyPortDetail *PortOverride);
void XDp_RxMstSetPbn(XDp *InstancePtr, u8 PortNum, u16 PbnVal);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

/* xdp_selftest.c: Self test function. */
u32 XDp_SelfTest(XDp *InstancePtr);

#if XPAR_XDPTXSS_NUM_INSTANCES
/* xdp_spm.c: Stream policy maker functions. */
void XDp_TxCfgMsaRecalculate(XDp *InstancePtr, u8 Stream);
void XDp_TxCfgMsaUseStandardVideoMode(XDp *InstancePtr, u8 Stream,
						XVidC_VideoMode VideoMode);
void XDp_TxCfgMsaUseEdidPreferredTiming(XDp *InstancePtr, u8 Stream,
								u8 *Edid);
void XDp_TxCfgMsaUseCustom(XDp *InstancePtr, u8 Stream,
		XDp_TxMainStreamAttributes *MsaConfigCustom, u8 Recalculate);
u32 XDp_TxCfgSetColorEncode(XDp *InstancePtr, u8 Stream,
		XVidC_ColorFormat Format, XVidC_ColorStd ColorCoeffs,
		XDp_DynamicRange Range);
void XDp_TxCfgMsaSetBpc(XDp *InstancePtr, u8 Stream, u8 BitsPerColor);
void XDp_TxCfgMsaEnSynchClkMode(XDp *InstancePtr, u8 Stream, u8 Enable);
void XDp_TxSetVideoMode(XDp *InstancePtr, u8 Stream);
void XDp_TxClearMsaValues(XDp *InstancePtr, u8 Stream);
void XDp_TxSetMsaValues(XDp *InstancePtr, u8 Stream);
void XDp_TxSetUserPixelWidth(XDp *InstancePtr, u8 UserPixelWidth);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
void XDp_RxSetUserPixelWidth(XDp *InstancePtr, u8 UserPixelWidth);
XVidC_ColorDepth XDp_RxGetBpc(XDp *InstancePtr, u8 Stream);
XVidC_ColorFormat XDp_RxGetColorComponent(XDp *InstancePtr, u8 Stream);
void XDp_RxSetLineReset(XDp *InstancePtr, u8 Stream);
void XDp_RxAllocatePayloadStream(XDp *InstancePtr);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
void XDp_GenerateCpIrq(XDp *InstancePtr);
void XDp_EnableDisableHdcp22AuxDeffers(XDp *InstancePtr, u8 EnableDisable);
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
void XDp_TxHdcp22Enable(XDp *InstancePtr);
void XDp_TxHdcp22Disable(XDp *InstancePtr);
#endif

/******************* Macros (Inline Functions) Definitions ********************/

/******************************************************************************/
/**
 * This is function determines whether the DisplayPort core, represented by the
 * XDp structure pointed to, is a transmitter (TX) or a receiver (RX).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	XDP_RX if the core is of type RX.
 *		XDP_TX if the core is of type TX.
 *
 * @note	C-style signature:
 *		XDp_CoreType XDp_GetCoreType(XDp *InstancePtr)
 *
*******************************************************************************/
#define XDp_GetCoreType(InstancePtr)	((InstancePtr)->Config.IsRx \
							? XDP_RX : XDP_TX)

/******************************************************************************/
/**
 * The following functions set the color encoding scheme for a given stream.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream number for which to configure the color
 *		encoding scheme for.
 *
 * @return	XST_SUCCESS.
 *
 * @note	C-style signatures:
 *		u32 XDp_TxCfgSetRGB(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetSRGB(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetYonly(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetYCbCr422Bt601(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetYCbCr422Bt709(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetYCbCr444Bt601(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetYCbCr444Bt709(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetXvYcc422Bt601(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetXvYcc422Bt709(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetXvYcc444Bt601(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetXvYcc444Bt709(XDp *InstancePtr, u8 Stream)
 *		u32 XDp_TxCfgSetAdbRGB(XDp *InstancePtr, u8 Stream)
 *
*******************************************************************************/
#define XDp_TxCfgSetRGB(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_RGB, XVIDC_BT_601, XDP_DR_VESA)
#define XDp_TxCfgSetSRGB(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_RGB, XVIDC_BT_601, XDP_DR_CEA)
#define XDp_TxCfgSetYonly(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YONLY, XVIDC_BT_601, XDP_DR_VESA)
#define XDp_TxCfgSetYCbCr422Bt601(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_422, XVIDC_BT_601, XDP_DR_CEA)
#define XDp_TxCfgSetYCbCr422Bt709(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_422, XVIDC_BT_709, XDP_DR_CEA)
#define XDp_TxCfgSetYCbCr444Bt601(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_444, XVIDC_BT_601, XDP_DR_CEA)
#define XDp_TxCfgSetYCbCr444Bt709(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_444, XVIDC_BT_709, XDP_DR_CEA)
#define XDp_TxCfgSetXvYcc422Bt601(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_422, XVIDC_BT_601, XDP_DR_VESA)
#define XDp_TxCfgSetXvYcc422Bt709(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_422, XVIDC_BT_709, XDP_DR_VESA)
#define XDp_TxCfgSetXvYcc444Bt601(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_444, XVIDC_BT_601, XDP_DR_VESA)
#define XDp_TxCfgSetXvYcc444Bt709(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_YCRCB_444, XVIDC_BT_709, XDP_DR_VESA)
#define XDp_TxCfgSetAdbRGB(InstancePtr, Stream) \
		XDp_TxCfgSetColorEncode((InstancePtr), (Stream), \
			XVIDC_CSF_RGB, XVIDC_BT_709, XDP_DR_CEA)

/******************************* Compatibility ********************************/

#define XDptx_ReadReg			XDp_ReadReg
#define XDprx_ReadReg			XDp_ReadReg
#define XDptx_WriteReg			XDp_WriteReg
#define XDprx_WriteReg			XDp_WriteReg
#define XDptx_Config			XDp_Config
#define XDprx_Config			XDp_Config
#define XDptx_TimerHandler		XDp_TimerHandler
#define XDprx_TimerHandler		XDp_TimerHandler
#define XDptx_HpdEventHandler		XDp_IntrHandler
#define XDptx_HpdPulseHandler		XDp_IntrHandler
#define XDprx_IntrHandler		XDp_IntrHandler

#define XDptx_LookupConfig		XDp_LookupConfig
#define XDprx_LookupConfig		XDp_LookupConfig
#define XDptx_CfgInitialize		XDp_CfgInitialize
#define XDprx_CfgInitialize		XDp_CfgInitialize
#define XDptx_InitializeTx		XDp_Initialize
#define XDprx_InitializeRx		XDp_Initialize
#define XDptx_WaitUs			XDp_WaitUs
#define XDprx_WaitUs			XDp_WaitUs
#define XDptx_SetUserTimerHandler	XDp_SetUserTimerHandler
#define XDprx_SetUserTimerHandler	XDp_SetUserTimerHandler
#define XDptx_SelfTest			XDp_SelfTest
#define XDprx_SelfTest			XDp_SelfTest
#define XDptx_HpdInterruptHandler	XDp_InterruptHandler
#define XDprx_InterruptHandler		XDp_InterruptHandler

#define XDptx				XDp
#define XDprx				XDp

#define XDPTX				XDP_TX
#define XDPRX				XDP_RX

#define XDp_TxSbMsgLinkAddressReplyDeviceInfo \
					XDp_SbMsgLinkAddressReplyDeviceInfo
#define XDp_TxSbMsgLinkAddressReplyPortDetail \
					XDp_SbMsgLinkAddressReplyPortDetail

#endif /* XDP_H_ */
/** @} */
