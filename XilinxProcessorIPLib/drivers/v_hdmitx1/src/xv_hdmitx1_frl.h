/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1_frl.h
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
#ifndef XV_HDMITX1_FRL_H_
#define XV_HDMITX1_FRL_H_        /**< Prevent circular inclusions
				   *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#define XV_HDMITX1_DDC_ADDRESS 0x54

/**************************** Type Definitions *******************************/
/**
 * @enum XV_HdmiTx1_FrlTrainingState
 *
 * This enumeration defines the various training states for the HDMI TX FRL link.
 */
typedef enum {
	XV_HDMITX1_FRLSTATE_LTS_L,        /**< LTS:L */
	XV_HDMITX1_FRLSTATE_LTS_1,        /**< LTS:1 */
	XV_HDMITX1_FRLSTATE_LTS_2,        /**< LTS:2 */
	XV_HDMITX1_FRLSTATE_LTS_3_ARM,    /**< LTS:3 (ARM) */
	XV_HDMITX1_FRLSTATE_LTS_3,        /**< LTS:3 */
	XV_HDMITX1_FRLSTATE_LTS_4,        /**< LTS:4 */
	XV_HDMITX1_FRLSTATE_LTS_P_ARM,    /**< LTS:P (Step 1) */
	XV_HDMITX1_FRLSTATE_LTS_P,        /**< LTS:P */
	XV_HDMITX1_FRLSTATE_LTS_P_FRL_RDY /**< LTS:P (FRL_START = 1) */
} XV_HdmiTx1_FrlTrainingState;

/**
 * @enum XV_HdmiTx1_FrlLtpType
 *
 * This enumeration defines the possible Link Training Patterns (LTP) used by the HDMI TX FRL link.
 */
typedef enum {
	XV_HDMITX1_LTP_NO_LTP = 0,           /**< No LTP pattern */
	XV_HDMITX1_LTP_ALL_ONES,             /**< All ones pattern */
	XV_HDMITX1_LTP_ALL_ZEROES,           /**< All zeroes pattern */
	XV_HDMITX1_LTP_NYQUIST_CLOCK,        /**< Nyquist clock pattern */
	XV_HDMITX1_LTP_TXDDE_COMPLIANCE,     /**< TXDDE compliance pattern */
	XV_HDMITX1_LTP_LFSR0,                /**< LFSR0 pattern */
	XV_HDMITX1_LTP_LFSR1,                /**< LFSR1 pattern */
	XV_HDMITX1_LTP_LFSR2,                /**< LFSR2 pattern */
	XV_HDMITX1_LTP_LFSR3                 /**< LFSR3 pattern */
} XV_HdmiTx1_FrlLtpType;

/**
 * @enum XV_HdmiTx1_FrlActiveMode
 *
 * This enumeration defines the operational modes for the HDMI transmitter
 * when using FRL. It indicates whether the transmitter is sending only
 * gap sequences or the full HDMI stream including video, audio, and auxiliary data.
 */
typedef enum {
       /**
        * @brief Gap-only mode.
        * Only FRL gap sequences are transmitted. No active video or audio data is sent.
        */
	XV_HDMITX1_FRL_ACTIVE_MODE_GAP_ONLY = 0,
       /**
        * @brief Full stream mode.
        * The full HDMI stream is transmitted over FRL, including video, audio,
	* and auxiliary data.
        */
	XV_HDMITX1_FRL_ACTIVE_MODE_FULL_STREAM
} XV_HdmiTx1_FrlActiveMode;

typedef union {
	u32 Data;    /**< AUX data field */
	u8 Byte[4];    /**< AUX data byte field */
} XV_HdmiTx1_FrlFfeAdjType;

/**
* This typedef contains audio stream specific data structure
*/
typedef struct {
	XV_HdmiTx1_FrlTrainingState TrainingState;	/**< Fixed Rate Link
	 	 	 	  	 	 	  *  State*/
	u16			    TimerCnt;	/**< FRL Timer */
	u8			    MaxFrlRate; /**< Maximum Supported FRL Rate
	 	 	 	 	 	  */
	u8			    FrlRate;	/**< Current FRL Rate */
	u8			    MaxLineRate;/**< Maximum Supported FRL Line
	 	 	 	  	 	  *  Rate */
	u8			    LineRate;	/**< Current FRL Line Rate */
	u8			    MaxLanes;	/**< Maximum Supported number
	 	 	 	  	 	 of FRL lanes */
	u8			    Lanes;	/**< Current number of lanes
						  *  used */
	u8			    FfeLevels;	/**< Number of Supported FFE
						  *  Levels for the current FRL
						  *  Rate */
	XV_HdmiTx1_FrlFfeAdjType    LaneFfeAdjReq;    /**< The TxFFE for each
							*  of the lanes */
	u8 			    TimerEvent; /** This flag is set when the
						  * FRL timer expires */
	u8			    RateLock;	/**< This flag locks the TX
    						  *  line rate to the starting
    						  *  line rate */
	u8			    FltNoTimeout;
	u8			    DBSendWrongLTP;
	u32			    DBMessage;
} XV_HdmiTx1_Frl;

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX FRL peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_FrlIntrEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_FrlIntrEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_FRL_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_FRL_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX FRL peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_FrlIntrDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_FrlIntrDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_FRL_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_FRL_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro enables repeat count in the packetizer (with RC compress) in the
* HDMI TX FRL peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_FrlRcEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_FrlRcEnable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_FRL_CTRL_CLR_OFFSET), \
			    (XV_HDMITX1_FRL_CTRL_TST_RC_MASK))

/*****************************************************************************/
/**
*
* This macro disables repeat count in the packetizer (with RC compress) in the
* HDMI TX FRL peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_FrlRcDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_FrlRcDisable(InstancePtr) \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_FRL_CTRL_SET_OFFSET), \
			    (XV_HDMITX1_FRL_CTRL_TST_RC_MASK))

/*****************************************************************************/
/**
*
* This macro sets the link clock of TX Core's FRL peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @param	Value specifies the Link Clock
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_SetFrlLinkClock(XV_HdmiTx1 *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx1_SetFrlLinkClock(InstancePtr, Value) \
{ \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_FRL_LNK_CLK_OFFSET), \
			    (Value)); \
}

/*****************************************************************************/
/**
*
* This macro sets the video clock of TX Core's FRL peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @param	Value specifies the Video Clock
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx1_SetFrlVidClock(XV_HdmiTx1 *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx1_SetFrlVidClock(InstancePtr, Value) \
{ \
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			    (XV_HDMITX1_FRL_VID_CLK_OFFSET), \
			    (Value)); \
}

/*****************************************************************************/
/**
*
* This macro enables FRL Rate Lock. With FRL Rate Lock enabled, TX core will
* not change to any other FRL Rate even when it is requested by the sink to
* drop FRL rate.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_FrlRateLockEnable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_FrlRateLockEnable(InstancePtr) \
	(InstancePtr)->Stream.Frl.RateLock = TRUE

/*****************************************************************************/
/**
*
* This macro disables FRL Rate Lock. With FRL Rate Lock disabled, TX core will
* behave according to the HDMI spec and will drop FRL Rate when the sink
* requests to drop FRL rate.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx1_FrlRateLockDisable(XV_HdmiTx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx1_FrlRateLockDisable(InstancePtr) \
	(InstancePtr)->Stream.Frl.RateLock = FALSE

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
