/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1.h
*
* This is the main header file for Xilinx HDMI RX core. HDMI RX core is used
* for extracting the video and audio streams from HDMI stream. It consists of
* - Receiver core
* - AXI4-Stream to Video Bridge
* - Video Timing Controller and
* - High-bandwidth Digital Content Protection (HDCP) (Optional)
* - Data Recovery Unit (DRU) (Optional).
*
* Receiver core performs following operations:
* - Aligns incoming data stream to the word boundary and removes inter channel
* skew.
* - Unscrambles the data if data rates above the 3.4 Gps. Otherwise bypasses
* the Scrambler.
* - Splits the data stream into video and packet data streams.
* - Optional data streams decrypt by an external HDCP module.
* - Decodes TMDS data into video data.
* - Converts the pixel data from the link domain into the video domain.
*
* AXI Video Bridge converts the captured native video to AXI stream and outputs
* the video data through the AXI video interface.
*
* Video Timing Controller (VTC) measures the video timing.
*
* Data Recovery Unit (DRU) to recover the data from the HDMI stream if incoming
* HDMI stream is too slow for the transceiver.
*
* <b>Core Features </b>
*
* For a full description of HDMI RX features, please see the hardware
* specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* HDMI RX core to be ready.
*
* - Call XV_HdmiRx1_LookupConfig using a device ID to find the core configuration.
* - Call XV_HdmiRx1_CfgInitialize to initialize the device and the driver
* instance associated with it.
*
* <b>Interrupts </b>
*
* This driver provides interrupt handlers
* - XV_HdmiRx1_IntrHandler, for handling the interrupts from the HDMI RX core
* peripherals.
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripheral's Status register.

* This driver provides XV_HdmiRx1_SetCallback API to register functions with HDMI
* RX core instance.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The HDMI RX driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     02/05/19 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_HDMIRX1_H_
#define XV_HDMIRX1_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/*#define DEBUG_RX_FRL_VERBOSITY	1*/

/***************************** Include Files *********************************/

#include "sleep.h"
#include "xv_hdmirx1_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xvidc.h"
#include "xv_hdmic.h"
#include "xv_hdmirx1_frl.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
	XV_HDMIRX1_HANDLER_CONNECT = 1,		/**< A connect event interrupt
						  *  type */
	XV_HDMIRX1_HANDLER_BRDG_OVERFLOW,	/**< Interrupt type for bridge
						  *  verflow */
	XV_HDMIRX1_HANDLER_AUX,			/**< Interrupt type for AUX
						  *  peripheral */
	XV_HDMIRX1_HANDLER_AUD,			/**< Interrupt type for AUD
						  *  peripheral */
	XV_HDMIRX1_HANDLER_LNKSTA,		/**< Interrupt type for LNKSTA
						  *  peripheral */
	XV_HDMIRX1_HANDLER_DDC,			/**< Interrupt type for DDC
						  *  peripheral */
	XV_HDMIRX1_HANDLER_STREAM_DOWN,		/**< Interrupt type for
						  *  stream down */
	XV_HDMIRX1_HANDLER_STREAM_INIT,		/**< Interrupt type for
						  *  stream init */
	XV_HDMIRX1_HANDLER_STREAM_UP,		/**< Interrupt type for
						  *  stream up */
	XV_HDMIRX1_HANDLER_HDCP,		/**< Interrupt type for hdcp */
	XV_HDMIRX1_HANDLER_DDC_HDCP_14_PROT,	/**< Interrupt type for
						  *  HDCP14PROT event */
	XV_HDMIRX1_HANDLER_DDC_HDCP_22_PROT,	/**< Interrupt type for
						  *  HDCP22PROT event */
	XV_HDMIRX1_HANDLER_LINK_ERROR,		/**< Interrupt type for
						  *  link error */
	XV_HDMIRX1_HANDLER_SYNC_LOSS,		/**< Interrupt type for
						  *  sync loss */
	XV_HDMIRX1_HANDLER_MODE,		/**< Interrupt type for mode */
	XV_HDMIRX1_HANDLER_TMDS_CLK_RATIO,	/**< Interrupt type for
						  *  TMDS clock ratio */
	XV_HDMIRX1_HANDLER_VIC_ERROR,		/**< Interrupt type for
						  *  VIC error */
	XV_HDMIRX1_HANDLER_PHY_RESET,		/**< Handler for Configuration
						  *  Retry Request */
	XV_HDMIRX1_HANDLER_LNK_RDY_ERR,		/**< Interrupt type for
						  *  Link Ready error */
	XV_HDMIRX1_HANDLER_VID_RDY_ERR,		/**< Interrupt type for
						  *  Video Ready error */
	XV_HDMIRX1_HANDLER_SKEW_LOCK_ERR,	/**< Interrupt type for
						  *  Skew Lock error */
	XV_HDMIRX1_HANDLER_FRL_CONFIG,		/**< Handler for FRL Config*/
	XV_HDMIRX1_HANDLER_FRL_START,		/**< Handler for FRL Start*/
	XV_HDMIRX1_HANDLER_TMDS_CONFIG,		/**< Handler for TMDS*/
	XV_HDMIRX1_HANDLER_FRL_LTS1,
	XV_HDMIRX1_HANDLER_FRL_LTS2,
	XV_HDMIRX1_HANDLER_FRL_LTS3,
	XV_HDMIRX1_HANDLER_FRL_LTS4,
	XV_HDMIRX1_HANDLER_FRL_LTSP,
	XV_HDMIRX1_HANDLER_FRL_LTSL,
	XV_HDMIRX1_HANDLER_VFP_CHANGE,		/**< Handler for VFP change
						  * event */
	XV_HDMIRX1_HANDLER_VRR_RDY,		/**<Handler for VRR rdy event */
	XV_HDMIRX1_HANDLER_DYN_HDR,		/**< Handler for Dynamic HDR */
} XV_HdmiRx1_HandlerType;
/*@}*/

/** @name HDMI RX stream status
* @{
*/
typedef enum {
	XV_HDMIRX1_STATE_FRL_LINK_TRAINING,
	XV_HDMIRX1_STATE_STREAM_DOWN,		/**< Stream down */
	XV_HDMIRX1_STATE_STREAM_IDLE,		/**< Stream idle */
	XV_HDMIRX1_STATE_STREAM_INIT,		/**< Stream init */
	XV_HDMIRX1_STATE_STREAM_ARM,		/**< Stream arm */
	XV_HDMIRX1_STATE_STREAM_LOCK,		/**< Stream lock */
	XV_HDMIRX1_STATE_STREAM_RDY,		/**< Stream ready */
	XV_HDMIRX1_STATE_STREAM_UP		/**< Stream up */
} XV_HdmiRx1_State;

/** @name HDMI RX sync status
* @{
*/
typedef enum {
	XV_HDMIRX1_SYNCSTAT_SYNC_LOSS,		/**< Sync Loss */
	XV_HDMIRX1_SYNCSTAT_SYNC_EST,		/**< Sync Lock */
} XV_HdmiRx1_SyncStatus;

/** @name HDMI RX audio format
* @{
*/
typedef enum {
	XV_HDMIRX1_AUDFMT_UNKNOWN = 0,
	XV_HDMIRX1_AUDFMT_LPCM,     /* L-PCM*/
	XV_HDMIRX1_AUDFMT_HBR,      /* HBR*/
	XV_HDMIRX1_AUDFMT_3D,	    /* 3D Audio */
} XV_HdmiRx1_AudioFormatType;

/** @name HDMI RX EDID RAM Size
* @{
*/
typedef enum {
	XV_HDMIRX1_EDID_SIZE_256B = 256,
	XV_HDMIRX1_EDID_SIZE_512B = 512,
	XV_HDMIRX1_EDID_SIZE_1024B = 1024,
	XV_HDMIRX1_EDID_SIZE_4096B = 4096
} XV_HdmiRx1_EdidSize;


/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for the HDMI RX core.
* Each HDMI RX device should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of
				  *  the HDMI RX core */
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */

	u32 AxiLiteClkFreq;
	u32 FRLClkFreqkHz;
	u32 VideoClkFreqkHz;
	u32 MaxFrlRate; /** < Maximum FRL Rate Supporte */
	u32 DynamicHDR; /**< Dynamic HDR supported */
	XV_HdmiRx1_EdidSize EdidRamSize;
} XV_HdmiRx1_Config;

/**
* This typedef contains HDMI RX audio stream specific data structure.
*/
typedef struct {
	u8	Active;		/**< Active flag. This flag is set when
				  *  an acitve audio stream was detected */
	u8	Channels;	/**< Channels */
} XV_HdmiRx1_AudioStream;

/**
* This typedef contains HDMI RX stream specific data structure.
*/
typedef struct {
	XVidC_VideoStream 	Video;		/* Video stream for HDMI RX */
	XVidC_PixelsPerClock	CorePixPerClk;	/* Core operates at a fixed 4PPC */
	XV_HdmiRx1_AudioStream	Audio;		/* Audio stream */
	XV_HdmiRx1_Frl		Frl;		/* FRL for HDMI TR */
	u8 	Vic;				/* Video Identification code flag */
	u8 	IsHdmi;	  /* HDMI TMDS flag. 1 - HDMI Stream, 0 - DVI Stream */
	u8 	IsFrl;	  /* FRL flag. 1 - FRL Mode, 0 - TMDS Mode */
	u64 PixelClk;				/* Pixel Clock */
	u64 RefClk;				/* Reference Clock */
	u8  IsScrambled; 			/* Scrambler flag
						 * 1 - scrambled data ,
						 * 0 - non scrambled data */
	XV_HdmiRx1_State	State;		/* State */
	XV_HdmiRx1_SyncStatus SyncStatus;	/* Stream Sync Status */
	u8 IsConnected;				/* Connected flag. This flag
						 * is set when the cable
						 * is connected */
	u8 GetVideoPropertiesTries;		/* This value is used in
						 * the GetVideoProperties API */

	u16 CedCounter[4];
	u16 RsCounter;
	u8 CedTimer;
} XV_HdmiRx1_Stream;

/** @name HDMI RX Dynamic HDR Error type
* @{
*/
typedef enum {
	XV_HDMIRX1_DYNHDR_ERR_SEQID = 1, /* Sequence id or ECC error */
	XV_HDMIRX1_DYNHDR_ERR_MEMWR = 2, /* Memory write error */
} XV_HdmiRx1_DynHdrErrType;

/**
* This typedef contains HDMI RX stream specific Dynamic HDR info.
*/
typedef struct {
	XV_HdmiRx1_DynHdrErrType err; /* Error type */
	u16 pkt_type;	/* Packet Type */
	u16 pkt_length; /* Packet length */
	u8 gof;		/* Graphics Overlay Flag */
} XV_HdmiRx1_DynHDR_Info;

/**
* Callback type for interrupt.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
*/
typedef void (*XV_HdmiRx1_Callback)(void *CallbackRef);
typedef void (*XV_HdmiRx1_HdcpCallback)(void *CallbackRef, int Data);

/**
* The XHdmiRx1 driver instance data. An instance must be allocated for each
* HDMI RX core in use.
*/
typedef struct {
	XV_HdmiRx1_Config Config;			/**< Hardware Configuration */
	u32 IsReady;					/**< Core and the driver instance are initialized */

	/*Callbacks */
	XV_HdmiRx1_Callback ConnectCallback;		/**< Callback for connect event interrupt */
	void *ConnectRef;				/**< To be passed to the connect interrupt callback */

	XV_HdmiRx1_Callback AuxCallback;		/**< Callback for AUX event interrupt */
	void *AuxRef;					/**< To be passed to the AUX interrupt callback */

	XV_HdmiRx1_Callback AudCallback;		/**< Callback for AUD event interrupt */
	void *AudRef;					/**< To be passed to the Audio interrupt callback */

	XV_HdmiRx1_Callback LnkStaCallback;		/**< Callback for LNKSTA event interrupt */
	void *LnkStaRef;				/**< To be passed to the LNKSTA interrupt callback */

	XV_HdmiRx1_Callback DdcCallback;		/**< Callback for PDDC interrupt */
	void *DdcRef;					/**< To be passed to the DDC interrupt callback */

	XV_HdmiRx1_Callback StreamDownCallback;		/**< Callback for stream down callback */
	void *StreamDownRef;				/**< To be passed to the stream down callback */

	XV_HdmiRx1_Callback StreamInitCallback;		/**< Callback for stream init callback */
	void *StreamInitRef;				/**< To be passed to the stream start callback */

	XV_HdmiRx1_Callback StreamUpCallback;		/**< Callback for stream up callback */
	void *StreamUpRef;				/**< To be passed to the stream up callback */

	XV_HdmiRx1_HdcpCallback HdcpCallback;		/**< Callback for hdcp callback */
	void *HdcpRef;					/**< To be passed to the hdcp callback */

	XV_HdmiRx1_Callback Hdcp14ProtEvtCallback;	/**< Callback for hdcp 1.4 protocol event written on the ddc. */
	void *Hdcp14ProtEvtRef;				/**< To be passed to the hdcp 1.4 protocol event callback */

	XV_HdmiRx1_Callback Hdcp22ProtEvtCallback;	/**< Callback for hdcp 2.2 protocol event written on the ddc. */
	void *Hdcp22ProtEvtRef;				/**< To be passed to the hdcp 2.2 protocol event callback */
	XV_HdmiRx1_Callback LinkErrorCallback;		/**< Callback for link error callback */
	void *LinkErrorRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback BrdgOverflowCallback;	/**< Callback for bridge overflow callback */
	void *BrdgOverflowRef;				/**< To be passed to the bridge overflow callback */

	XV_HdmiRx1_Callback SyncLossCallback;		/**< Callback for sync loss callback */
	void *SyncLossRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback ModeCallback;		/**< Callback for sync loss callback */
	void *ModeRef;					/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback TmdsClkRatioCallback;	/**< Callback for TMDS clock ratio change */
	void *TmdsClkRatioRef;				/**< To be passed to the TMDS callback */

	XV_HdmiRx1_Callback VicErrorCallback;		/**< Callback for Vic error detection */
	void *VicErrorRef;				/**< To be passed to the vic error callback */

	XV_HdmiRx1_Callback PhyResetCallback;		/**< Callback for Phy Reset */
	void *PhyResetRef;				/**< To be passed to the phy reset callback */

	XV_HdmiRx1_Callback LnkRdyErrorCallback;	/**< Callback for Link Ready Error detection */
	void *LnkRdyErrorRef;				/**< To be passed to the link ready error callback */

	XV_HdmiRx1_Callback VidRdyErrorCallback;	/**< Callback for Video Ready Error detection */
	void *VidRdyErrorRef;				/**< To be passed to the video ready error callback */

	XV_HdmiRx1_Callback SkewLockErrorCallback;	/**< Callback for Skew Lock Error detection */
	void *SkewLockErrorRef;				/**< To be passed to the skew lock error callback */

	XV_HdmiRx1_Callback FrlConfigCallback;		/**< Callback for sync loss callback */
	void *FrlConfigRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlStartCallback;		/**< Callback for sync loss callback */
	void *FrlStartRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback TmdsConfigCallback;		/**< Callback for sync loss callback */
	void *TmdsConfigRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlLtsLCallback;		/**< Callback for sync loss callback */
	void *FrlLtsLRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlLts1Callback;		/**< Callback for sync loss callback */
	void *FrlLts1Ref;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlLts2Callback;		/**< Callback for sync loss callback */
	void *FrlLts2Ref;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlLts3Callback;		/**< Callback for sync loss callback */
	void *FrlLts3Ref;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlLts4Callback;		/**< Callback for sync loss callback */
	void *FrlLts4Ref;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback FrlLtsPCallback;		/**< Callback for sync loss callback */
	void *FrlLtsPRef;				/**< To be passed to the link error callback */

	XV_HdmiRx1_Callback VfpChangeCallback;		/**< Callback for VFP change callback */
	void *VfpChangeRef;				/**< To be passed to the vfp change callback */

	XV_HdmiRx1_Callback VrrRdyCallback;		/**< Callback for VRR ready callback */
	void *VrrRdyRef;				/**< To be passed to the VRR ready callback */

	XV_HdmiRx1_Callback DynHdrCallback;		/**< Callback for Dynamic HDR event */
	void *DynHdrRef;				/**< To be passed to Dynamic HDR event callback */

	/* HDMI RX stream */
	XV_HdmiRx1_Stream Stream;			/**< HDMI RX stream information */

	/* Aux peripheral specific */
	XHdmiC_Aux Aux;					/**< AUX peripheral information */

	/* Audio peripheral specific */
	u32 AudCts;					/**< Audio CTS */
	u32 AudN;					/**< Audio N element */
	XV_HdmiRx1_AudioFormatType AudFormat;		/**< Audio Format */

	XV_HdmiC_VrrInfoFrame VrrIF;			/**< VRR infoframe SPDIF or VTEM */

	u8 IsFirstVtemReceived;
	u8  DBMessage;					/**< Debug Message for Logs */
	u16  IsErrorPrintCount;      /**< Error Print is completed */
	u8 SubsysVidIntfc;			/**< Video Interface 0 - AXI4S, 1 - Native, 2 - Native DE */
	XVidC_PixelsPerClock SubsysPpc;		/**< Subsystem Pixels per clock */
} XV_HdmiRx1;

/***************** Macros (Inline Functions) Definitions *********************/
#define TIME_10MS	(XPAR_XV_HDMIRX1_0_AXI_LITE_FREQ_HZ/100)
#define TIME_200MS	(XPAR_XV_HDMIRX1_0_AXI_LITE_FREQ_HZ/5)
#define TIME_16MS	((XPAR_XV_HDMIRX1_0_AXI_LITE_FREQ_HZ*10)/625)

/*****************************************************************************/
/**
*
* This macro returns the clock cycles required to count up to 10Ms with respect
* to AXI Lite Frequency
*
* @param  InstancePtr is a pointer to the XV_HdmiRX1 core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiRx1_GetTime10Ms(InstancePtr) \
  (InstancePtr)->Config.AxiLiteClkFreq/100

/*****************************************************************************/
/**
*
* This macro returns the clock cycles required to count up to 16Ms with
* respect to AXI Lite Frequency
*
* @param  InstancePtr is a pointer to the XV_HdmiRX1 core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiRx1_GetTime16Ms(InstancePtr) \
	((InstancePtr)->Config.AxiLiteClkFreq * 10) / 625

/*****************************************************************************/
/**
*
* This macro returns the clock cycles required to count up to 200Ms with
* respect to AXI Lite Frequency
*
* @param  InstancePtr is a pointer to the XV_HdmiRX1 core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiRx1_GetTime200Ms(InstancePtr) \
	(InstancePtr)->Config.AxiLiteClkFreq/5

/*****************************************************************************/
/**
*
* This macro returns the clock cycles required to count up to 1s with
* respect to AXI Lite Frequency
*
* @param  InstancePtr is a pointer to the XV_HdmiRX1 core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiRx1_GetTime1S(InstancePtr) \
	(InstancePtr)->Config.AxiLiteClkFreq

/*****************************************************************************/
/**
*
* This macro reads the RX version
*
* @param  InstancePtr is a pointer to the XHdmi_RX core instance.
*
* @return RX version.
*
* *note	C-style signature:
*		u32 XV_HdmiRx1_GetVersion(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_GetVersion(InstancePtr) \
	XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMIRX1_VER_VERSION_OFFSET))

/*****************************************************************************/
/**
*
* This macro asserts or clears the HDMI RX reset.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param	Reset specifies TRUE/FALSE value to either assert or
*		release HDMI RX reset.
*
* @return	None.
*
* @note		The reset output of the PIO is inverted. When the system is
*		in reset, the PIO output is cleared and this will reset the
*		HDMI RX. Therefore, clearing the PIO reset output will assert
*		the HDMI link and video reset.
*		C-style signature:
*		void XV_HdmiRx1_Reset(XV_HdmiRx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
#define XV_HdmiRx1_Reset(InstancePtr, Reset) \
{ \
	if (Reset) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_RESET_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_RESET_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro asserts or clears the HDMI RX link enable.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either assert or
*		release HDMI RX link enable.
*
* @return	None.
*
* @note
*		C-style signature:
*		void XV_HdmiRx1_Reset(XV_HdmiRx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiRx1_LinkEnable(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_LNK_EN_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_LNK_EN_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro asserts or clears the HDMI RX video enable.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either assert or
*		release HDMI RX video enable.
*
* @return	None.
*
* @note
*		C-style signature:
*		void XV_HdmiRx1_Reset(XV_HdmiRx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiRx1_VideoEnable(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_VID_EN_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_VID_EN_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro controls the HDMI RX Scrambler.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		scrambler.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_SetScrambler(XV_HdmiRx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiRx1_SetScrambler(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_SCRM_MASK)); \
		(InstancePtr)->Stream.IsScrambled = (TRUE); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_SCRM_MASK)); \
		(InstancePtr)->Stream.IsScrambled = (FALSE); \
	} \
	XV_HdmiRx1_FrlDdcWriteField((InstancePtr), \
				    XV_HDMIRX1_SCDCFIELD_SCRAMBLER_STAT, \
				    SetClr); \
}

/*****************************************************************************/
/**
*
* This macro controls the YUV420 mode for video bridge.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		YUV 420 Support.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Bridge_yuv420(XV_HdmiRx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiRx1_Bridge_yuv420(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_BRIDGE_YUV420_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_BRIDGE_YUV420_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro controls the Pixel Drop mode for video bridge.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		Pixel Repitition.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Bridge_pixel(XV_HdmiRx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiRx1_Bridge_pixel(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_BRIDGE_PIXEL_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_BRIDGE_PIXEL_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro asserts or clears the AXIS enable output port.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param	Reset specifies TRUE/FALSE value to either assert or
*		release HDMI RX reset.
*
* @return	None.
*
* @note		The reset output of the PIO is inverted. When the system is
*		in reset, the PIO output is cleared and this will reset the
*		HDMI RX. Therefore, clearing the PIO reset output will assert
*		the HDMI link and video reset.
*		C-style signature:
*		void XV_HdmiRx1_AxisEnable(InstancePtr, Enable)
*
******************************************************************************/
#define XV_HdmiRx1_AxisEnable(InstancePtr, Enable) \
{ \
	if (Enable) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_AXIS_EN_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMIRX1_PIO_OUT_AXIS_EN_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX PIO peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_PioEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_PioEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_PIO_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_PIO_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX PIO peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_PioDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_PioDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_PIO_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_PIO_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX PIO peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_PioIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_PioIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_PIO_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_PIO_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupts in the HDMI RX PIO peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_PioIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_PioIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_PIO_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_PIO_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr1Enable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr1Enable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR1_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr1Disable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr1Disable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET),\
		            (XV_HDMIRX1_TMR1_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_TmrIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr1IntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR1_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr1IntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr1IntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_TMR1_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro starts the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr1Start(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr1Start(InstancePtr, Value) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR1_CNT_OFFSET), (u32)(Value))

/*****************************************************************************/
/**
*
* This macro reads the HDMI RX timer peripheral's remaining timer counter value
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_GetTmr1Value(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_GetTmr1Value(InstancePtr) \
	XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMIRX1_TMR1_CNT_OFFSET))

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr2Enable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr2Enable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR2_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr2Disable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr2Disable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET),\
		            (XV_HDMIRX1_TMR2_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr2IntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr2IntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR2_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr2IntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr2IntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_TMR2_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro starts the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr2Start(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr2Start(InstancePtr, Value) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR2_CNT_OFFSET), (u32)(Value))

/*****************************************************************************/
/**
*
* This macro reads the HDMI RX timer peripheral's remaining timer counter value
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_GetTmr2Value(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_GetTmr2Value(InstancePtr) \
	XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMIRX1_TMR2_CNT_OFFSET))

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr3Enable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr3Enable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR3_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr3Disable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr3Disable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET),\
		            (XV_HDMIRX1_TMR3_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr3IntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr3IntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR3_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr3IntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr3IntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_TMR3_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro starts the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr3Start(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr3Start(InstancePtr, Value) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR3_CNT_OFFSET), (u32)(Value))

/*****************************************************************************/
/**
*
* This macro reads the HDMI RX timer peripheral's remaining timer counter value
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_GetTmr3Value(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_GetTmr3Value(InstancePtr) \
	XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMIRX1_TMR3_CNT_OFFSET))

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr4Enable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr4Enable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR4_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr4Disable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr4Disable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET),\
		            (XV_HDMIRX1_TMR4_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr4IntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr4IntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_TMR4_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr4IntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr4IntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_TMR4_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro starts the HDMI RX timer peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_Tmr4Start(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_Tmr4Start(InstancePtr, Value) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_TMR4_CNT_OFFSET), (u32)(Value))

/*****************************************************************************/
/**
*
* This macro reads the HDMI RX timer peripheral's remaining timer counter value
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_GetTmr4Value(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_GetTmr4Value(InstancePtr) \
	XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMIRX1_TMR4_CNT_OFFSET))

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX Timing Detector peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_VtdEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_VtdEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_VTD_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_VTD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX Timing Detector peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_VtdDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_VtdDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_VTD_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_VTD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI RX Timing Detector peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_VtdIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_VtdIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_VTD_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_VTD_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI RX Timing Detector peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_VtdIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_VtdIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_VTD_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_VTD_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro allow control to enable/disable the HDMI RX VFP event.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable
* 		the VFP Event
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_VtdVfpEvent(XV_HdmiRx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiRx1_VtdVfpEvent(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_VTD_CTRL_SET_OFFSET), \
				    (XV_HDMIRX1_VTD_CTRL_VFP_ENABLE_MASK)); \
	} \
	else { \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMIRX1_VTD_CTRL_CLR_OFFSET), \
				    (XV_HDMIRX1_VTD_CTRL_VFP_ENABLE_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro sets the timebase in the HDMI RX Timing Detector peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_VtdSetTimebase(XV_HdmiRx1 *InstancePtr, Value)
*
******************************************************************************/
#define XV_HdmiRx1_VtdSetTimebase(InstancePtr, Value) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_VTD_CTRL_OFFSET), \
			    (u32)(Value << XV_HDMIRX1_VTD_CTRL_TIMEBASE_SHIFT))


/*****************************************************************************/
/**
*
* This macro enables the HDMI RX Display Data Channel (DDC) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables the SCDC in the DDC peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcScdcEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcScdcEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_SCDC_EN_MASK));

/*****************************************************************************/
/**
*
* This macro enables the HDCP in the DDC peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcHdcpEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcHdcpEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_HDCP_EN_MASK));

#define XV_HdmiRx1_DdcHdcpDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_HDCP_EN_MASK));


/*****************************************************************************/
/**
*
* This macro sets the DDC peripheral into HDCP 1.4 mode.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcHdcp14Mode(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcHdcp14Mode(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_HDCP_MODE_MASK));

/*****************************************************************************/
/**
*
* This macro sets the DDC peripheral into HDCP 2.2 mode.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcHdcp22Mode(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcHdcp22Mode(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_HDCP_MODE_MASK));

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX Display Data Channel (DDC) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX Display Data Channel (DDC)
* peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupts in the HDMI RX Display Data Channel (DDC)
* peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro clears the SCDC registers in the DDC peripheral
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DdcScdcClear(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcScdcClear(InstancePtr) \
{ \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_SCDC_CLR_MASK)); \
	usleep(50);\
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_SCDC_CLR_MASK)); \
	XV_HdmiRx1_FrlDdcWriteField(InstancePtr, XV_HDMIRX1_SCDCFIELD_FLT_READY,\
				    1);\
	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,XV_HDMIRX1_SCDCFIELD_SINK_VER, \
				    1);\
}

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX Auxiliary (AUX) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AuxEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AuxEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUX_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_AUX_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX Auxiliary (AUX) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AuxDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AuxDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUX_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_AUX_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX Auxiliary (AUX) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AuxIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AuxIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUX_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_AUX_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupts in the HDMI RX Auxiliary (AUX) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AuxIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AuxIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUX_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_AUX_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro enables FSync/VRR event interrupt in the HDMI RX Auxiliary
* (AUX) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AuxFSyncVrrChEvtEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AuxFSyncVrrChEvtEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUX_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_AUX_CTRL_FSYNC_VRR_CH_EVT_MASK))

/*****************************************************************************/
/**
*
* This macro disables FSync/VRR event interrupt in the HDMI RX Auxiliary
* (AUX) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AuxFSyncVrrChEvtDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AuxFSyncVrrChEvtDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUX_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_AUX_CTRL_FSYNC_VRR_CH_EVT_MASK))

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX Audio (AUD) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AudioEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AudioEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUD_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_AUD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX Audio (AUD) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AudioDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AudioDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUD_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_AUD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupts in the HDMI RX Audio (AUD) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AudioIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AudioIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUD_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_AUD_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupts in the HDMI RX Audio (AUD) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_AudioIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_AudioIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_AUD_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_AUD_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro enables ACR Update Event in the HDMI RX Audio (AUD) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
#define XV_HdmiRx1_SetAudioAcrUpdateEventEn(InstancePtr) \
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress, \
			XV_HDMIRX1_AUD_CTRL_SET_OFFSET, \
			XV_HDMIRX1_AUD_CTRL_ACR_UPD_EVT_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables ACR Update Event in the HDMI RX Audio (AUD) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
#define XV_HdmiRx1_ClearAudioAcrUpdateEventEn(InstancePtr) \
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress, \
			XV_HDMIRX1_AUD_CTRL_CLR_OFFSET, \
			XV_HDMIRX1_AUD_CTRL_ACR_UPD_EVT_EN_MASK)

/*****************************************************************************/
/**
*
* This macro enables the HDMI RX Link Status (LNKSTA) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_LinkstaEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_LnkstaEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_LNKSTA_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_LNKSTA_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI RX Link Status (LNKSTA) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_LinkstaDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_LnkstaDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_LNKSTA_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_LNKSTA_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI RX Link Status (LNKSTA) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_LinkIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_LinkIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_LNKSTA_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_LNKSTA_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disable interrupt in the HDMI RX Link Status (LNKSTA) peripheral.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_LinkIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_LinkIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_LNKSTA_CTRL_CLR_OFFSET), \
			    (XV_HDMIRX1_LNKSTA_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro returns true is the audio stream is active else false
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	TRUE if the audio stream is active, FALSE if it is not.
*
* @note		C-style signature:
*		u32 XV_HdmiRx1_IsAudioActive(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_IsAudioActive(InstancePtr) \
	(InstancePtr)->Stream.Audio.Active

/*****************************************************************************/
/**
*
* This macro returns the number of active audio channels.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	Number of active audio channels.
*
* @note		C-style signature:
*		u32 XV_HdmiRx1_GetAudioChannels(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_GetAudioChannels(InstancePtr) \
	(InstancePtr)->Stream.Audio.Channels

/*****************************************************************************/
/**
*
* This macro clears the HDCP write message buffer in the DDC peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XHdmiRx1_DdcHdcpClearWriteMessageBuffer(XHdmi_Rx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcHdcpClearWriteMessageBuffer(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_WMSG_CLR_MASK))

/*****************************************************************************/
/**
*
* This macro clears the HDCP read message buffer in the DDC peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XHdmiRx1_DdcHdcpClearReadMessageBuffer(XHdmi_Rx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DdcHdcpClearReadMessageBuffer(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMIRX1_DDC_CTRL_RMSG_CLR_MASK))

/*****************************************************************************/
/**
*
* This macro enables the data mover for Dynamic HDR.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DynHDR_DM_Enable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DynHDR_DM_Enable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    XV_HDMIRX1_PIO_OUT_SET_OFFSET, \
			    XV_HDMIRX1_PIO_OUT_DYN_HDR_DM_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables the data mover for Dynamic HDR.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_DynHDR_DM_Disable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    XV_HDMIRX1_PIO_OUT_CLR_OFFSET, \
			    XV_HDMIRX1_PIO_OUT_DYN_HDR_DM_EN_MASK)

/************************** Function Prototypes ******************************/

/* Initialization function in xv_hdmirx1_sinit.c */
XV_HdmiRx1_Config *XV_HdmiRx1_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xv_hdmirx1.c */
int XV_HdmiRx1_CfgInitialize(XV_HdmiRx1 *InstancePtr, XV_HdmiRx1_Config *CfgPtr, \
			     UINTPTR EffectiveAddr);
void XV_HdmiRx1_SetAxiClkFreq(XV_HdmiRx1 *InstancePtr, u32 ClkFreq);
void XV_HdmiRx1_Clear(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_SetStream(XV_HdmiRx1 *InstancePtr, XVidC_PixelsPerClock Ppc,
			 u32 Clock);
int XV_HdmiRx1_IsStreamUp(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_IsStreamScrambled(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_IsStreamConnected(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_SetHpd(XV_HdmiRx1 *InstancePtr, u8 SetClr);
void XV_HdmiRx1_INT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset);
void XV_HdmiRx1_INT_LRST(XV_HdmiRx1 *InstancePtr, u8 Reset);
void XV_HdmiRx1_EXT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset);
void XV_HdmiRx1_EXT_SYSRST(XV_HdmiRx1 *InstancePtr, u8 Reset);
int XV_HdmiRx1_SetPixelRate(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_SetColorFormat(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_IsLinkStatusErrMax(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_ClearLinkStatus(XV_HdmiRx1 *InstancePtr);
u32 XV_HdmiRx1_GetLinkStatus(XV_HdmiRx1 *InstancePtr, u8 Type);
u32 XV_HdmiRx1_GetAcrCts(XV_HdmiRx1 *InstancePtr);
u32 XV_HdmiRx1_GetAcrN(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_DdcLoadEdid(XV_HdmiRx1 *InstancePtr, u8 *Data, u16 Length);
void XV_HdmiRx1_DdcHdcpSetAddress(XV_HdmiRx1 *InstancePtr, u32 Addr);
void XV_HdmiRx1_DdcHdcpWriteData(XV_HdmiRx1 *InstancePtr, u32 Data);
u32 XV_HdmiRx1_DdcHdcpReadData(XV_HdmiRx1 *InstancePtr);
u16 XV_HdmiRx1_DdcGetHdcpWriteMessageBufferWords(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_DdcIsHdcpWriteMessageBufferEmpty(XV_HdmiRx1 *InstancePtr);
u16 XV_HdmiRx1_DdcGetHdcpReadMessageBufferWords(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_DdcIsHdcpReadMessageBufferEmpty(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_GetTmdsClockRatio(XV_HdmiRx1 *InstancePtr);
u8 XV_HdmiRx1_GetAviVic(XV_HdmiRx1 *InstancePtr);
XVidC_ColorFormat XV_HdmiRx1_GetAviColorSpace(XV_HdmiRx1 *InstancePtr);
XVidC_ColorDepth XV_HdmiRx1_GetGcpColorDepth(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_GetVideoProperties(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_GetVideoTiming(XV_HdmiRx1 *InstancePtr);
u32 XV_HdmiRx1_Divide(u32 Dividend, u32 Divisor);
void XV_HdmiRx1_SetPixelClk(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_Start(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_Stop(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_UpdateEdFlags(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_TmrStartMs(XV_HdmiRx1 *InstancePtr, u32 Milliseconds,
		u8 TimerSelect);
XVidC_VideoMode XV_HdmiRx1_LookupVmId(u8 Vic);
void XV_HdmiRx1_ParseSrcProdDescInfoframe(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_ParseVideoTimingExtMetaIF(XV_HdmiRx1 *InstancePtr);

/* Fixed Rate Link */
void XV_HdmiRx1_FrlModeEnable(XV_HdmiRx1 *InstancePtr, u8 LtpThreshold,
				XV_HdmiRx1_FrlLtp DefaultLtp, u8 FfeSuppFlag);
int XV_HdmiRx1_ExecFrlState(XV_HdmiRx1 *InstancePtr);
u32 XV_HdmiRx1_GetPatternsMatchStatus(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_PhyResetPoll(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_FrlLinkRetrain(XV_HdmiRx1 *InstancePtr, u8 LtpThreshold,
		XV_HdmiRx1_FrlLtp DefaultLtp);
void XV_HdmiRx1_FrlReset(XV_HdmiRx1 *InstancePtr, u8 Reset);
int XV_HdmiRx1_ConfigFrlLtpDetection(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_SetFrlLtpDetection(XV_HdmiRx1 *InstancePtr, u8 Lane,
				XV_HdmiRx1_FrlLtpType Ltp);
u32 XV_HdmiRx1_GetFrlLtpDetection(XV_HdmiRx1 *InstancePtr, u8 Lane);
void XV_HdmiRx1_ResetFrlLtpDetection(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_FrlLtpDetectionEnable(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_FrlLtpDetectionDisable(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_SetFrlLtpThreshold(XV_HdmiRx1 *InstancePtr, u8 Threshold);
int XV_HdmiRx1_RetrieveFrlRateLanes(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_SetFrlRateWrEvent_En(XV_HdmiRx1 *InstancePtr);
int XV_HdmiRx1_FrlDdcWriteField(XV_HdmiRx1 *InstancePtr,
				XV_HdmiRx1_FrlScdcFieldType Field,
				u8 Value);
u32 XV_HdmiRx1_FrlDdcReadField(XV_HdmiRx1 *InstancePtr,
				XV_HdmiRx1_FrlScdcFieldType Field);
void XV_HdmiRx1_SetFrlFltNoTimeout(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_ClearFrlFltNoTimeout(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_SetFrl10MicroSecondsTimer(XV_HdmiRx1 *InstancePtr);
u32 XV_HdmiRx1_GetFrlTotalPixRatio(XV_HdmiRx1 *InstancePtr);
u32 XV_HdmiRx1_GetFrlActivePixRatio(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_RestartFrlLt(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_FrlFltUpdate(XV_HdmiRx1 *InstancePtr, u8 Flag);

/* Log specific functions */
void XV_HdmiRx1_Info(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_DebugInfo(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_RegisterDebug(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_DdcRegDump(XV_HdmiRx1 *InstancePtr);

/* Self test function in xv_hdmirx1_selftest.c */
int XV_HdmiRx1_SelfTest(XV_HdmiRx1 *InstancePtr);

/* Interrupt related function in xv_hdmirx1_intr.c */
void XV_HdmiRx1_IntrHandler(void *InstancePtr);
int XV_HdmiRx1_SetCallback(XV_HdmiRx1 *InstancePtr,
		XV_HdmiRx1_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef);

XV_HdmiC_VideoTimingExtMeta *XV_HdmiRx1_GetVidTimingExtMeta(
				XV_HdmiRx1 *InstancePtr);
XV_HdmiC_SrcProdDescIF *XV_HdmiRx1_GetSrcProdDescIF(
			XV_HdmiRx1 *InstancePtr);
XV_HdmiC_VrrInfoframeType XV_HdmiRx1_GetVrrIfType(XV_HdmiRx1 *InstancePtr);
void XV_HdmiRx1_SetVrrIfType(XV_HdmiRx1 *InstancePtr,
		XV_HdmiC_VrrInfoframeType Type);
void XV_HdmiRx1_DynHDR_SetAddr(XV_HdmiRx1 *InstancePtr, u64 Addr);
void XV_HdmiRx1_DynHDR_GetInfo(XV_HdmiRx1 *InstancePtr,
			       XV_HdmiRx1_DynHDR_Info *RxDynInfoPtr);

/************************** Variable Declarations ****************************/
/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
