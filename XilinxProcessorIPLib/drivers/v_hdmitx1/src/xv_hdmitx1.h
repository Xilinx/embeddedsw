/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1.h
*
* This is the main header file for Xilinx HDMI TX core. HDMI TX core is used
* for transmitting the incoming video and audio streams. It consists of
* - Transmitter core
* - AXI4-Stream to Video Bridge
* - Video Timing Controller and
* - High-bandwidth Digital Content Protection (HDCP) (Optional).
*
* The HDMI TX uses three AXI interfaces for Video, Audio and Processor:
* - AXI4-Stream interface for Video, can be single, dual or quad pixels per
* clock and supports 8 and 10 bits per component.
* - AXI4-Stream interface for Audio, accepts multiple channels uncompressed
* and compressed audio data.
* - AXI4-Lite interface for processor, controls the transmitter.
* Please do refer AXI Reference Guide (UG761) for more information on AXI
* interfaces.
*
* Transmitter core performs following operations:
* - Converts video data from the video clock domain into the link clock domain.
* - TMDS (Transition Minimized Differential Signaling) encoding.
* - Merges encoded video data and packet data into a single HDMI stream.
* - Optional HDMI stream is encrypted by an external HDCP module.
* - Over samples HDMI stream if stream bandwidth is too low for the transceiver
* to handle.
* - Scrambles encrypted/HDMI stream if data rate is above 3.4 Gbps otherwise
* bypasses the Scrambler.
*
* AXI Video Bridge converts the incoming video AXI-stream to native video.
*
* Video Timing Controller (VTC) generates the native video timing.
*
* <b>Core Features </b>
*
* For a full description of HDMI TX features, please see the hardware
* specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* HDMI TX core to be ready.
*
* - Call XV_HdmiTx1_LookupConfig using a device ID to find the core
*   configuration.
* - Call XV_HdmiTx1_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts </b>
*
* This driver provides interrupt handlers
* - XV_HdmiTx1_IntrHandler, for handling the interrupts from the HDMI TX core
* PIO and DDC peripheral respectively.
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripheral's Status register.

* This driver provides XV_HdmiTx1_SetCallback API to register functions with HDMI
* TX core instance.
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
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The HDMI TX driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*s
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     22/05/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_HDMITX1_H_
#define XV_HDMITX1_H_	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/*#define DEBUG_DDC_VERBOSITY			1*/
/*#define DEBUG_TX_FRL_VERBOSITY 		1*/

/***************************** Include Files *********************************/

#include "xv_hdmitx1_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xvidc.h"
#include "xv_hdmic_vsif.h"
#include "xv_hdmic.h"
#include "xv_hdmitx1_frl.h"

/************************** Constant Definitions *****************************/
#define XV_HDMITX1_DDC_ADDRESS 0x54

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/**************************** Type Definitions *******************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
	XV_HDMITX1_HANDLER_CONNECT = 1,  /* Handler for connect*/
	XV_HDMITX1_HANDLER_TOGGLE,       /* Handler for toggle*/
	XV_HDMITX1_HANDLER_BRDGLOCK,     /* Handler for bridge locked*/
	XV_HDMITX1_HANDLER_BRDGUNLOCK,   /* Handler for bridge unlocked*/
	XV_HDMITX1_HANDLER_BRDGOVERFLOW, /* Handler for bridge overflow*/
	XV_HDMITX1_HANDLER_BRDGUNDERFLOW,/* Handler for bridge underflow*/
	XV_HDMITX1_HANDLER_VS,           /* Handler for vsync*/
	XV_HDMITX1_HANDLER_STREAM_DOWN,  /* Handler for stream down*/
	XV_HDMITX1_HANDLER_STREAM_UP,    /* Handler for stream up*/
	XV_HDMITX1_HANDLER_FRL_CONFIG,   /* Handler for FRL Config*/
	XV_HDMITX1_HANDLER_FRL_FFE,      /* Handler for FRL FFE*/
	XV_HDMITX1_HANDLER_FRL_START,    /* Handler for FRL Start*/
	XV_HDMITX1_HANDLER_FRL_STOP,     /* Handler for FRL Stop*/
	XV_HDMITX1_HANDLER_TMDS_CONFIG,   /* Handler for TMDS*/
	XV_HDMITX1_HANDLER_FRL_LTS1,
	XV_HDMITX1_HANDLER_FRL_LTS2,
	XV_HDMITX1_HANDLER_FRL_LTS3,
	XV_HDMITX1_HANDLER_FRL_LTS4,
	XV_HDMITX1_HANDLER_FRL_LTSP,
	XV_HDMITX1_HANDLER_FRL_LTSL,
	XV_HDMITX1_HANDLER_CED_UPDATE,
	XV_HDMITX1_HANDLER_DYNHDR_MWT,
	XV_HDMITX1_HANDLER_DSCDECODE_FAIL,
} XV_HdmiTx1_HandlerType;
/*@}*/

/** @name HDMI TX stream status
* @{
*/
typedef enum {
	XV_HDMITX1_STATE_STREAM_DOWN,    /* Stream down*/
	XV_HDMITX1_STATE_STREAM_UP       /* Stream up*/
} XV_HdmiTx1_State;

/** @name HDMI TX audio format
* @{
*/
typedef enum {
	XV_HDMITX1_AUDFMT_LPCM = 0,    /* L-PCM*/
	XV_HDMITX1_AUDFMT_HBR,         /* HBR*/
	XV_HDMITX1_AUDFMT_3D,           /* 3D Audio */
} XV_HdmiTx1_AudioFormatType;

/** @name HDMI TX SCDC Fields
* @{
*/
typedef enum {
	XV_HDMITX1_SCDCFIELD_SOURCE_VER = 0,
	XV_HDMITX1_SCDCFIELD_SNK_CFG0,		/* Bit0 = RR_Enable
						 * Bit1 = FLT_no_retrain */
	XV_HDMITX1_SCDCFIELD_SNK_CFG1,		/* Bits[3:0] = FRL_Rate
	 	 	 	 	 	 * Bits[7:4] = FFE_Levels */
	XV_HDMITX1_SCDCFIELD_SNK_STU,		/* Source_Test_Update */
	XV_HDMITX1_SCDCFIELD_CED_UPDATE,	/* CED_Update */
	XV_HDMITX1_SCDCFIELD_FRL_START,		/* FLT_start */
	XV_HDMITX1_SCDCFIELD_FLT_UPDATE,	/* FLT_update */
	XV_HDMITX1_SCDCFIELD_FLT_NO_RETRAIN,	/* FLT_no_retrain */
	XV_HDMITX1_SCDCFIELD_SIZE
} XV_HdmiTx1_ScdcFieldType;

/** @name HDMI TX CTS and N Source
* @{
*/
typedef enum {
	XV_HDMITX1_EXTERNAL_CTS_N = 0,
	XV_HDMITX1_INTERNAL_CTS_N,
} XV_HdmiTx1_CTSNSource;

/**
* This typedef contains DDC registers offset, mask, shift.
*/
typedef struct {
/*	XV_HdmiTx1_ScdcFieldType Field;	/\**< SCDC Field *\/ */
	u8 Offset;				/**< Register offset */
	u8 Mask;				/**< Bits mask */
	u8 Shift;				/**< Bits shift */
} XV_HdmiTx1_ScdcField;

/**
* This typedef contains configuration information for the HDMI TX core.
* Each HDMI TX device should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< DeviceId is the unique ID
				  *  of the HDMI TX core */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;    /**< BaseAddress is the physical
				  * base address of the core's registers */
	u32 MaxFrlRate ;	/**< Maximum FRL Rate Supported */
	u32 DynHdr;		/**< Dynamic HDR supported */
	u32 AxiLiteClkFreq;
} XV_HdmiTx1_Config;

/**
* This typedef contains audio stream specific data structure
*/
typedef struct {
	u8 Channels;
	XHdmiC_SamplingFrequencyVal SampleFrequency;    /* Audio Sample
							 * Freq. in Hz */
	XHdmiC_SamplingFrequency    SFreq;		/* Audio Sample Freq.
							 * Enumeration */
} XV_HdmiTx1_AudioStream;

/**
* This typedef contains HDMI TX stream specific data structure.
*/
typedef struct {
	XVidC_VideoStream       Video;		/**< Video stream for HDMI TX */
	XVidC_PixelsPerClock	CorePixPerClk;	/**< Core operates at a fixed 4PPC */
	XV_HdmiTx1_AudioStream  Audio;		/**< Audio stream for HDMI TX */
	XV_HdmiTx1_Frl		Frl;		/**< FRL for HDMI TX */
	u8                      Vic;		/**< Video Identification code
	                                          *  flag  */
	u8                      IsHdmi;		/* HDMI TMDS flag. 1 - HDMI Stream, 0 - DVI Stream */
	u8                      IsFrl;	  	/* FRL flag. 1 - FRL Mode, 0 - TMDS Mode */
	u8                      ScdcSupport;	/**< SCDC Support flag  */
	u8                      ScdcEd[11];	/**< SCDC - Character Error
	                                          *  Detection Readings */
	u8                      IsScrambled;    /**< Scrambler flag
						  *  1 - scrambled data ,
						  *  0 - non scrambled data */
	u8			OverrideScrambler;	/**< Override scramble
							  *  flag */
	u64                     TMDSClock;      /**< TMDS clock */
	u8                      TMDSClockRatio; /**< TMDS clock ration
						  *  0 - 1/10, 1 - 1/40 */
	u64                     PixelClk;       /**< Pixel Clock  */
	XV_HdmiTx1_State        State;          /**< State */
	u8                      IsConnected;    /**< Connected flag.
						  *  This flag is set when the
						  *  cable is connected  */
	u8                      SampleRate;	/**< Sample rate */
} XV_HdmiTx1_Stream;

/**
* Callback type for Vsync event interrupt.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
*/
typedef void (*XV_HdmiTx1_Callback)(void *CallbackRef);

/**
* The XV_HdmiTx1 driver instance data. An instance must be allocated for each
* HDMI TX core in use.
*/
typedef struct {
	XV_HdmiTx1_Config Config;    /**< Hardware Configuration */
	u32 IsReady;        /**< Core and the driver instance are initialized */

	/* Callbacks */
	XV_HdmiTx1_Callback ConnectCallback;     /**< Callback for connect event
						   *  interrupt */
	void *ConnectRef;                       /**< To be passed to the connect
						  *  interrupt callback */

	XV_HdmiTx1_Callback ToggleCallback;     /**< Callback for toggle event
						  *  interrupt */
	void *ToggleRef;			/**< To be passed to the toggle
						  *  interrupt callback */

	XV_HdmiTx1_Callback VsCallback;		/**< Callback for Vsync event
						  *  interrupt */
	void *VsRef;                            /**< To be passed to the Vsync
						  *  interrupt callback */

	XV_HdmiTx1_Callback BrdgLockedCallback;	/**< Callback for Bridge
						  *  Locked event
						  *  interrupt */
	void *BrdgLockedRef;			/**< To be passed to the Bridge
						  *  Unlocked interrupt callback */

	XV_HdmiTx1_Callback BrdgUnlockedCallback;	/**< Callback for Bridge
							  *  UnLocked event
							  *  interrupt */
	void *BrdgUnlockedRef;			/**< To be passed to the Bridge
						  *  Unlocked interrupt callback */

	XV_HdmiTx1_Callback BrdgOverflowCallback;   /**< Callback for Bridge
	                                              * Overflow event interrupt */
	void *BrdgOverflowRef;                  /**< To be passed to the Bridge
	                                          *  Overflow interrupt callback */

	XV_HdmiTx1_Callback BrdgUnderflowCallback;   /**< Callback for Bridge
	                                              * Underflow event
	                                              * interrupt */
	void *BrdgUnderflowRef;                 /**< To be passed to the Bridge
	                                          *  Underflow interrupt callback */
	XV_HdmiTx1_Callback StreamDownCallback; /**< Callback for stream down
	                                          *  callback */
	void *StreamDownRef;                    /**< To be passed to the stream
	                                          *  down callback */

	XV_HdmiTx1_Callback StreamUpCallback;   /**< Callback for stream up
						  *  callback */
	void *StreamUpRef;                      /**< To be passed to the stream up
	                                          *  callback */

	XV_HdmiTx1_Callback FrlConfigCallback;  /**< Callback for FRL Config event
	                                          *  interrupt */
	void *FrlConfigRef;                     /**< To be passed to the FRL
	                                          *  Config interrupt callback */

	XV_HdmiTx1_Callback FrlFfeCallback;     /**< Callback for FRL FFE event
	                                          * interrupt */
	void *FrlFfeRef;			/**< To be passed to the FRL
	                                          *  FFE interrupt callback */

	XV_HdmiTx1_Callback FrlStartCallback;   /**< Callback for FRL Start event
	                                          *  interrupt */
	void *FrlStartRef;                      /**< To be passed to the FRL
	                                          *  Start interrupt callback */

	XV_HdmiTx1_Callback FrlStopCallback;    /**< Callback for FRL Start event
	                                          *  interrupt */
	void *FrlStopRef;                       /**< To be passed to the FRL
	                                          *  Start interrupt callback */

	XV_HdmiTx1_Callback TmdsConfigCallback; /**< Callback for TMDS Config event
	                                          *  interrupt */
	void *TmdsConfigRef;                    /**< To be passed to the TMDS
	                                          *  Config interrupt callback */

	XV_HdmiTx1_Callback FrlLtsLCallback;	/**< Callback for FRL LTS:L
						  *  callback */
	void *FrlLtsLRef;			/**< To be passed to the link
						  *  error callback */

	XV_HdmiTx1_Callback FrlLts1Callback;	/**< Callback for FRL LTS:1
						  *  callback */
	void *FrlLts1Ref;			/**< To be passed to the link
						  *  error callback */

	XV_HdmiTx1_Callback FrlLts2Callback;	/**< Callback for FRL LTS:2
						  *  callback */
	void *FrlLts2Ref;			/**< To be passed to the link
						  *  error callback */

	XV_HdmiTx1_Callback FrlLts3Callback;	/**< Callback for FRL LTS:3
						  *  callback */
	void *FrlLts3Ref;			/**< To be passed to the
						  *  link error callback */

	XV_HdmiTx1_Callback FrlLts4Callback;	/**< Callback for FRL LTS:4
						  *  callback */
	void *FrlLts4Ref;			/**< To be passed to the
						  *  link error callback */

	XV_HdmiTx1_Callback FrlLtsPCallback;	/**< Callback for FRL LTS:P
						  *  callback */
	void *FrlLtsPRef;			/**< To be passed to the
						  *  link error callback */

	XV_HdmiTx1_Callback CedUpdateCallback;	/**< Callback for CED Update
						  *  callback */
	void *CedUpdateRef;			/**< To be passed to the
						  *  link error callback */

	XV_HdmiTx1_Callback DynHdrMtwCallback;	/**< Callback for Dynamic HDR
						  *  MTW Start */
	void *DynHdrMtwRef;			/**< To be passed to the
						  *  Dynamic HDR callback */
	XV_HdmiTx1_Callback DscDecodeFailCallback; /**< Callback for DSC decode fail */
	void *DscDecodeFailRef;  /**< To be passed to DSC decode fail callback */
	/* Aux peripheral specific */
	XHdmiC_Aux Aux;                         /**< AUX peripheral information */
	XV_HdmiC_VrrInfoFrame VrrIF;		/**< VRR infoframe SPDIF or VTEM */

	/* ACR CTS and N Source*/
	XV_HdmiTx1_CTSNSource CTS_N_Source;

	/* HDMI TX stream */
	XV_HdmiTx1_Stream Stream;               /**< HDMI TX stream information */
	u32 CpuClkFreq;                         /**< CPU Clock frequency */

	u8  DBMessage;
} XV_HdmiTx1;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro returns the clock cycles required to count up to 1MS with respect
* to AXI Lite Frequency
*
* @param  InstancePtr is a pointer to the XV_HdmiTX1 core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiTx1_GetTime1Ms(InstancePtr) \
	(InstancePtr)->Config.AxiLiteClkFreq/1000

/*****************************************************************************/
/**
*
* This macro returns the clock cycles required to count up to 10MS with respect
* to AXI Lite Frequency
*
* @param  InstancePtr is a pointer to the XV_HdmiTX core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiTx1_GetTime10Ms(InstancePtr) \
	(InstancePtr)->Config.AxiLiteClkFreq/100

/*****************************************************************************/
/**
*
* This macro reads the TX version
*
* @param  InstancePtr is a pointer to the XV_HdmiTX1 core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiTx1_GetVersion(InstancePtr) \
	XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMITX1_VER_VERSION_OFFSET))

/*****************************************************************************/
/**
*
* This macro asserts or releases the HDMI TX reset.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX reset.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI . Therefore, clearing the PIO reset output will assert
*       the HDMI link and video reset.
*       C-style signature:
*       void XV_HdmiTx1_Reset(XV_HdmiTx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
#define XV_HdmiTx1_Reset(InstancePtr, Reset) \
{ \
	if (Reset) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_RST_MASK)); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_RST_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro controls the HDMI TX Scrambler.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    SetClr specifies TRUE/FALSE value to either set ON or clear
*       Scrambler.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_SetScrambler(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_SetScrambler(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_SCRM_MASK)); \
		(InstancePtr)->Stream.IsScrambled = (TRUE); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_SCRM_MASK)); \
		(InstancePtr)->Stream.IsScrambled = (FALSE); \
	} \
}

/*****************************************************************************/
/**
*
* This macro controls the YUV420 mode for video bridge.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		YUV 420 Support.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_Bridge_yuv420(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_Bridge_yuv420(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_BRIDGE_YUV420_MASK)); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_BRIDGE_YUV420_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro controls the Pixel Repeat mode for video bridge.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		Pixel Repitition Support.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_Bridge_pixel(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_Bridge_pixel(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_BRIDGE_PIXEL_MASK)); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET), \
				    (XV_HDMITX1_PIO_OUT_BRIDGE_PIXEL_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro enables the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_PioEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_PioEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_PIO_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_PioDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_PioDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_PIO_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_PioIntrEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_PioIntrEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_PIO_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_PioIntrDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_PioIntrDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_PIO_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro clears HDMI TX PIO interrupt.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_PioIntrClear(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_PioIntrClear(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_STA_OFFSET), \
			    (XV_HDMITX1_PIO_STA_IRQ_MASK))

/*****************************************************************************/
/**
*
* This macro enables the HDMI TX Display Data Channel (DDC) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_DdcEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DdcEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_DDC_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI TX Display Data Channel (DDC) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_DdcDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DdcDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_DDC_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX DDC peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_DdcIntrEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DdcIntrEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_DDC_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_DDC_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX DDC peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_DdcIntrDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DdcIntrDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_DDC_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_DDC_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro clears HDMI TX DDC interrupt.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_DdcIntrClear(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DdcIntrClear(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_DDC_STA_OFFSET), \
			    (XV_HDMITX1_DDC_STA_IRQ_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI TX Auxiliary (AUX) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_AuxDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_AuxDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_AUX_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX AUX peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_AuxIntrEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_AuxIntrEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_AUX_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro allows enabling/disabling of VRR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable
* 		the VFP Event
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_VrrControl(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_VrrControl(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
				    (XV_HDMITX1_AUX_CTRL_VRR_EN_MASK)); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
				    (XV_HDMITX1_AUX_CTRL_VRR_EN_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro allows enabling/disabling of FSync in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable
* 		the VFP Event
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_FSyncControl(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_FSyncControl(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
				    (XV_HDMITX1_AUX_CTRL_FYSYNC_EN_MASK)); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
				    (XV_HDMITX1_AUX_CTRL_FYSYNC_EN_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro enables the data mover for Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_DM_Enable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_DM_Enable(InstancePtr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    XV_HDMITX1_PIO_OUT_SET_OFFSET, \
				    XV_HDMITX1_PIO_OUT_DYN_HDR_DM_EN_MASK); \
	} \
}

/*****************************************************************************/
/**
*
* This macro disables the data mover for Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_DM_Disable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_DM_Disable(InstancePtr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    XV_HDMITX1_PIO_OUT_CLR_OFFSET, \
				    XV_HDMITX1_PIO_OUT_DYN_HDR_DM_EN_MASK); \
	} \
}

/*****************************************************************************/
/**
*
* This macro allows enabling/disabling of Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable
* 		the Dynamic HDR
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdrControl(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_Control(InstancePtr, SetClr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		if (SetClr) { \
			XV_HdmiTx1_DynHdr_DM_Enable(InstancePtr); \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_EN_MASK)); \
		} \
		else { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_EN_MASK)); \
			XV_HdmiTx1_DynHdr_DM_Disable(InstancePtr); \
		} \
	} \
}

/*****************************************************************************/
/**
*
* This macro allows enabling/disabling of FAPA Location Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	SetClr specifies TRUE/FALSE value for FAPA location.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_FAPA_Control(XV_HdmiTx1 *InstancePtr,
*		u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_FAPA_Control(InstancePtr, SetClr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		if (SetClr) { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_FAPA_LOC_MASK)); \
		} \
		else { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_FAPA_LOC_MASK)); \
		} \
	} \
}

/*****************************************************************************/
/**
*
* This macro allows enabling/disabling of GOF Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable
* 		the GOF (Graphics Overlay Flag)
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_GOF_Control(XV_HdmiTx1 *InstancePtr,
*		u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_GOF_Control(InstancePtr, SetClr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		if (SetClr) { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_GOF_EN_MASK)); \
		} \
		else { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_GOF_EN_MASK)); \
		} \
	} \
}

/*****************************************************************************/
/**
*
* This macro allows set/clear of GOF Value Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	SetClr specifies TRUE/FALSE value to either set or clear
* 		the GOF (Graphics Overlay Flag) Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_GOFVal_Control(XV_HdmiTx1 *InstancePtr,
*		u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_GOFVal_Control(InstancePtr, SetClr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		if (SetClr) { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_GOF_VAL_MASK)); \
		} \
		else { \
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
					    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
					    (XV_HDMITX1_AUX_CTRL_DYNHDR_GOF_VAL_MASK)); \
		} \
	} \
}

/*****************************************************************************/
/**
*
* This macro allows to set the MTW bit to clear it for Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_MTW_Clear(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_MTW_Clear(InstancePtr) \
{ \
	u32 val; \
	if (InstancePtr->Config.DynHdr) { \
		val = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress, \
					 XV_HDMITX1_AUX_STA_OFFSET); \
		val |= XV_HDMITX1_AUX_STA_DYNHDR_MTW_MASK; \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_AUX_STA_OFFSET), \
				    val); \
	} \
}

/*****************************************************************************/
/**
*
* This macro gets the read status of Data Mover for Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	u32 val - 0 means ok else memory read error
*
* @note		C-style signature:
*		u32 XV_HdmiTx1_DynHdr_GetReadStatus(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_GetReadStatus(InstancePtr) \
	(XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   XV_HDMITX1_AUX_STA_OFFSET) & \
			XV_HDMITX1_AUX_DYNHDR_RD_STS_MASK);

/*****************************************************************************/
/**
*
* This macro sets the Header packet type and length for Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	PktLength is a u16 length of Dynamic HDR packet.
* @param	PktType is a u16 Type of Dynamic HDR packet.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_SetPacket(XV_HdmiTx1 *InstancePtr,
*						u16 PacketLength, u16 PacketType)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_SetPacket(InstancePtr, PktLen, PktType) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		u32 val = (u16)PktType | \
		((u16)PktLen << XV_HDMITX1_AUX_DYNHDR_PKT_LENGTH_SHIFT); \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    XV_HDMITX1_AUX_DYNHDR_PKT_OFFSET, \
				    val); \
	} \
}

/*****************************************************************************/
/**
*
* This macro sets the buffer address for Dynamic HDR in HDMI Tx
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param	Addr is a u64 which contains the buffer address
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_DynHdr_SetAddr(XV_HdmiTx1 *InstancePtr, u64 Addr)
*
******************************************************************************/
#define XV_HdmiTx1_DynHdr_SetAddr(InstancePtr, Addr) \
{ \
	if (InstancePtr->Config.DynHdr) { \
		u32 val; \
		val = (u32)Addr; \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    XV_HDMITX1_AUX_DYNHDR_ADDR_LSB_OFFSET, \
				    val); \
		val = (u32)(((u64)Addr & 0xFFFFFFFF00000000) >> 32); \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    XV_HDMITX1_AUX_DYNHDR_ADDR_MSB_OFFSET, \
				    val); \
	} \
}

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX AUX peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_AuxIntrDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_AuxIntrDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_AUX_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables audio in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_AudioDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_AudioDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_AUD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro sets the mode bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_SetMode(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_SetMode(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_OUT_SET_OFFSET), \
			    (XV_HDMITX1_PIO_OUT_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro clears the mode bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_ClearMode(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_ClearMode(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_OUT_CLR_OFFSET), \
			    (XV_HDMITX1_PIO_OUT_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro provides the current mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Current mode.
*       0 = DVI
*       1 = HDMI
*
* @note     C-style signature:
*       u8 XV_HdmiTx1_GetMode(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_GetMode(InstancePtr) \
	(XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_PIO_OUT_OFFSET)) & \
	(XV_HDMITX1_PIO_OUT_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro provides the current sample rate.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Sample rate
*
* @note     C-style signature:
*       u8 XV_HdmiTx1_GetSampleRate(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_GetSampleRate(InstancePtr) \
	(InstancePtr)->Stream.SampleRate

/*****************************************************************************/
/**
*
* This macro provides the active audio channels.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Audio channels
*
*
******************************************************************************/
#define XV_HdmiTx1_GetAudioChannels(InstancePtr) \
	(InstancePtr)->Stream.Audio.Channels

/*****************************************************************************/
/**
*
* This macro provides the current pixel packing phase.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Pixel packing phase.
*
*
******************************************************************************/
#define XV_HdmiTx1_GetPixelPackingPhase(InstancePtr) \
	(((XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			      (XV_HDMITX1_PIO_IN_OFFSET))) >> \
	  (XV_HDMITX1_PIO_IN_PPP_SHIFT)) & \
	 (XV_HDMITX1_PIO_IN_PPP_MASK))

/*****************************************************************************/
/**
*
* This macro disables video mask in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_MaskDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_MaskDisable(InstancePtr) \
{ \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_MASK_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_MASK_CTRL_RUN_MASK)); \
}

/*****************************************************************************/
/**
*
* This macro enables video mask in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_MaskEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_MaskEnable(InstancePtr) \
{ \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_MASK_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_MASK_CTRL_RUN_MASK)); \
}

/*****************************************************************************/
/**
*
* This macro enables or disables the noise in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		Noise.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_MaskNoise(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_MaskNoise(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_MASK_CTRL_SET_OFFSET), \
				    (XV_HDMITX1_MASK_CTRL_NOISE_MASK)); \
	} \
	else { \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_MASK_CTRL_CLR_OFFSET), \
				    (XV_HDMITX1_MASK_CTRL_NOISE_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro sets the red component value in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_MaskSetRed(XV_HdmiTx1 *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx1_MaskSetRed(InstancePtr, Value) \
{ \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_MASK_RED_OFFSET), \
				    (Value)); \
}

/*****************************************************************************/
/**
*
* This macro sets the green component value in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_MaskSetGreen(XV_HdmiTx1 *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx1_MaskSetGreen(InstancePtr, Value) \
{ \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_MASK_GREEN_OFFSET), \
				    (Value)); \
}

/*****************************************************************************/
/**
*
* This macro sets the blue component value in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_MaskSetBlue(XV_HdmiTx1 *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx1_MaskSetBlue(InstancePtr, Value) \
{ \
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
				    (XV_HDMITX1_MASK_BLUE_OFFSET), \
				    (Value)); \
}

/*****************************************************************************/
/**
*
* This macro provides the current video mask mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Current mode.
*       0 = Video masking is disabled
*       1 = Video masking is enabled
*
* @note     C-style signature:
*       u8 XV_HdmiTx1_IsMasked(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_IsMasked(InstancePtr) \
	XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress, \
			   (XV_HDMITX1_MASK_CTRL_OFFSET)) & \
	(XV_HDMITX1_MASK_CTRL_RUN_MASK)

/*****************************************************************************/
/**
*
* This macro allows enabling/disabling of DSC in HDMI-TX
*
* @param        InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param        SetClr specifies TRUE/FALSE value to either enable or disable
*               the DSC
*
* @return       None.
*
* @note         C-style signature:
* 			void XV_HdmiTx1_DscControl(XV_HdmiTx1 *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx1_DscControl(InstancePtr, SetClr) \
{ \
        if (SetClr) { \
                XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
                                    (XV_HDMITX1_AUX_CTRL_SET_OFFSET), \
                                    (XV_HDMITX1_AUX_CTRL_DSC_EN_MASK)); \
        } \
        else { \
                XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
                                    (XV_HDMITX1_AUX_CTRL_CLR_OFFSET), \
                                    (XV_HDMITX1_AUX_CTRL_DSC_EN_MASK)); \
        } \
}

/************************** Function Prototypes ******************************/

/* Initialization function in xv_hdmitx1_sinit.c */
#ifndef SDT
XV_HdmiTx1_Config *XV_HdmiTx1_LookupConfig(u16 DeviceId);
#else
XV_HdmiTx1_Config *XV_HdmiTx1_LookupConfig(UINTPTR BaseAddress);
#endif

/* Initialization and control functions in xv_hdmitx1.c */
int XV_HdmiTx1_CfgInitialize(XV_HdmiTx1 *InstancePtr,
			     XV_HdmiTx1_Config *CfgPtr,
			     UINTPTR EffectiveAddr);
void XV_HdmiTx1_SetHdmiFrlMode(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetHdmiTmdsMode(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetDviMode(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_AuxEnable(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_AudioEnable(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_Clear(XV_HdmiTx1 *InstancePtr);
u8 XV_HdmiTx1_LookupVic(XVidC_VideoMode VideoMode);
XVidC_VideoMode XV_HdmiTx1_GetVideoModeFromVic(u8 Vic);
u32 XV_HdmiTx1_SetStream(XV_HdmiTx1 *InstancePtr,
		XVidC_VideoTiming VideoTiming,
		XVidC_FrameRate FrameRate,
		XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc,
		XVidC_PixelsPerClock Ppc,
		XVidC_3DInfo *Info3D,
		u8 FVaFactor,
		u8 VrrEnabled,
		u8 CnmvrrEnabled,
		u64 *TmdsClk);
u64 XV_HdmiTx1_GetTmdsClk(XV_HdmiTx1 *InstancePtr);

void XV_HdmiTx1_INT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset);
void XV_HdmiTx1_INT_LRST(XV_HdmiTx1 *InstancePtr, u8 Reset);
void XV_HdmiTx1_EXT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset);
void XV_HdmiTx1_EXT_SYSRST(XV_HdmiTx1 *InstancePtr, u8 Reset);
void XV_HdmiTx1_SetGcpAvmuteBit(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_ClearGcpAvmuteBit(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetGcpClearAvmuteBit(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_ClearGcpClearAvmuteBit(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetPixelRate(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetSampleRate(XV_HdmiTx1 *InstancePtr, u8 SampleRate);
void XV_HdmiTx1_SetColorFormat(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetColorDepth(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_IsStreamScrambled(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_IsStreamConnected(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetAxiClkFreq(XV_HdmiTx1 *InstancePtr, u32 ClkFreq);
void XV_HdmiTx1_DdcInit(XV_HdmiTx1 *InstancePtr, u32 Frequency);
int XV_HdmiTx1_DdcWrite(XV_HdmiTx1 *InstancePtr, u8 Slave, u16 Length,
			u8 *Buffer, u8 Stop);
int XV_HdmiTx1_DdcRead(XV_HdmiTx1 *InstancePtr, u8 Slave, u16 Length,
		       u8 *Buffer, u8 Stop);
int XV_HdmiTx1_DdcReadReg(XV_HdmiTx1 *InstancePtr, u8 Slave, u16 Length,
			  u8 RegAddr, u8 *Buffer);
int XV_HdmiTx1_DdcWriteField(XV_HdmiTx1 *InstancePtr,
			     XV_HdmiTx1_ScdcFieldType Field,
			     u8 Value);
void XV_HdmiTx1_Aux_Dsc_Send_Header(XV_HdmiTx1 *InstancePtr, u32 Data);
void XV_HdmiTx1_Aux_Dsc_Send_Data(XV_HdmiTx1 *InstancePtr, u32 Data);
u32 XV_HdmiTx1_AuxSend(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_Scrambler(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_ClockRatio(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_DetectHdmi20(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_ShowSCDC(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_Info(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_DebugInfo(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_RegisterDebug(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_SetAudioChannels(XV_HdmiTx1 *InstancePtr, u8 Value);
int XV_HdmiTx1_SetAudioFormat(XV_HdmiTx1 *InstancePtr,
			      XV_HdmiTx1_AudioFormatType Value);
XV_HdmiTx1_AudioFormatType
XV_HdmiTx1_GetAudioFormat(XV_HdmiTx1 *InstancePtr);
u32 XV_HdmiTxSs1_GetAudioCtsVal(XV_HdmiTx1 *InstancePtr);
u32 XV_HdmiTxSs1_GetAudioNVal(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_FRLACRStart(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_TMDSACRStart(XV_HdmiTx1 *InstancePtr);

/* Fixed Rate Link */
int XV_HdmiTx1_StartTmdsMode(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_StartFrlTraining(XV_HdmiTx1 *InstancePtr,
		XHdmiC_MaxFrlRate FrlRate);
void XV_HdmiTx1_SetFrlMaxFrlRate(XV_HdmiTx1 *InstancePtr,
		XHdmiC_MaxFrlRate MaxFrlRate);
int XV_HdmiTx1_ExecFrlState(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_FrlStreamStart(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_FrlStreamStop(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetFrlLtp(XV_HdmiTx1 *InstancePtr, u8 Lane,
			  XV_HdmiTx1_FrlLtpType Ltp);
void XV_HdmiTx1_SetFrlActive(XV_HdmiTx1 *InstancePtr,
			     XV_HdmiTx1_FrlActiveMode Mode);
void XV_HdmiTx1_SetFrlLanes(XV_HdmiTx1 *InstancePtr, u8 Lanes);
void XV_HdmiTx1_FrlModeEn(XV_HdmiTx1 *InstancePtr, u8 Mode);
void XV_HdmiTx1_FrlReset(XV_HdmiTx1 *InstancePtr, u8 Reset);
int XV_HdmiTx1_FrlRate(XV_HdmiTx1 *InstancePtr, u8 FrlRate);
void XV_HdmiTx1_FrlExtVidCkeSource(XV_HdmiTx1 *InstancePtr, u8 Value);
void XV_HdmiTx1_FrlExecute(XV_HdmiTx1 *InstancePtr);
int XV_HdmiTx1_FrlTrainingInit(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetFrlTimer(XV_HdmiTx1 *InstancePtr, u32 Milliseconds);
void XV_HdmiTx1_SetFrlTimerClockCycles(XV_HdmiTx1 *InstancePtr,
		u32 ClockCycles);
u32 XV_HdmiTx1_GetFrlTimer(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetFrl10MicroSecondsTimer(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_SetFrlWrongLtp(XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_ClearFrlWrongLtp(XV_HdmiTx1 *InstancePtr);
u8 *XV_HdmiTx1_GetScdcEdRegisters(XV_HdmiTx1 *InstancePtr);
void  XV_HdmiTx1_Start(XV_HdmiTx1 *InstancePtr);
void  XV_HdmiTx1_Stop(XV_HdmiTx1 *InstancePtr);

/* Self test function in xv_hdmitx1_selftest.c */
int XV_HdmiTx1_SelfTest(XV_HdmiTx1 *InstancePtr);

/* Interrupt related functions in xv_hdmitx1_intr.c */
void XV_HdmiTx1_IntrHandler(void *InstancePtr);
int XV_HdmiTx1_SetCallback(XV_HdmiTx1 *InstancePtr,
		XV_HdmiTx1_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef);

XV_HdmiC_VideoTimingExtMeta *XV_HdmiTx1_GetVidTimingExtMeta(
		XV_HdmiTx1 *InstancePtr);
XV_HdmiC_SrcProdDescIF *XV_HdmiTx1_GetSrcProdDescIF(
		XV_HdmiTx1 *InstancePtr);
void XV_HdmiTx1_GenerateVideoTimingExtMetaIF(XV_HdmiTx1 *InstancePtr,
			XV_HdmiC_VideoTimingExtMeta *ExtMeta);
void XV_HdmiTx1_GenerateCustomVideoTimingExtMetaIF(XV_HdmiTx1 *InstancePtr,
				XV_HdmiC_VideoTimingExtMeta *ExtMeta, u16 Sync,
				u16 DataSetLen);
void XV_HdmiTx1_GenerateSrcProdDescInfoframe(XV_HdmiTx1 *InstancePtr,
			XV_HdmiC_SrcProdDescIF *SpdIfPtr);

/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
