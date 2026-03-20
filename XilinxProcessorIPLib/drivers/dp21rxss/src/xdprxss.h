/******************************************************************************
* Copyright (C) 2015 - 2023 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss.h
* @addtogroup dprxss Overview
* @{
* @details
*
* This is the main header file for Xilinx DisplayPort Receiver Subsystem core.
* It abstracts Subsystem cores and provides high level API's to application
* developer.
*
* <b>Core Features</b>
*
* For a full description of DisplayPort Receiver Subsystem core, please
* see the hardware specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* DisplayPort Receiver Subsystem core to be ready.
*
* - Call XDpRxSs_LookupConfig using a device ID to find the core
*   configuration.
* - Call XDpRxSs_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts</b>
*
* The DisplayPort RX Subsystem driver provides the interrupt handlers
* - XDpRxSs_DpIntrHandler
* - XDpRxSs_HdcpIntrHandler
* - XDpRxSs_TmrCtrIntrHandler, for handling the interrupt from the DisplayPort,
* optional HDCP and Timer Counter sub-cores respectively. The users of this
* driver have to register this handler with the interrupt system and provide
* the callback functions by using XDpRxSs_SetCallBack API.
*
* <b>Virtual Memory</b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b>Threads</b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b>Building the driver</b>
*
* The DisplayPort Receiver Subsystem driver is composed of several source
* files. This allows the user to build and link only those parts of the driver
* that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Removed HDCP interrupt handler types.
*                   Added HDCP and Timer Counter support.
* 3.0  sha 02/19/16 Removed indexing from enum XDpRxSs_HandlerType.
*                   Added handler type as enum for HDCP:
*                   XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT.
*                   Added function: XDpRxSs_DownstreamReady.
* 3.1  als 08/08/16 Added HDCP timeout functionality.
* 3.1  aad 09/07/16 Updates to support 64-bit base addresses.
* 4.0  aad 12/01/16 Added interrupt handler for HDCP authentication
*      ms  01/23/17 Modified xil_printf statement in main function for all
*                   examples to ensure that "Successfully ran" and "Failed"
*                   strings are available in all examples. This is a fix
*                   for CR-965028.
*      ms  03/17/17 Modified readme.txt file in examples folder for doxygen
*                   generation.
* 4.1  tu  09/08/17 Added three driver side interrupt handler for Video,
*                   NoVideo and PowerChange events
* 4.1  jb  02/19/19 Added support for HDCP22.
* 6.1  rg  09/23/20 Added below list of APIs related to color encoding parameters
*                   XDpRxss_GetBpc
*                   XDpRxss_GetColorComponent
*                   XDpRxss_GetColorimetry
*                   XDpRxss_GetDynamicRange
* 8.0  jb  01/12/22 Added clk_wizard configuration for rx_dec_clk which is
*                   required for 8b10b logic.
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_H_
#define XDPRXSS_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdprxss_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/* Subsystem sub-cores header files */
#include "xdprxss_dprx.h"
#include "xdprxss_iic.h"
#include "xdprxss_hdcp1x.h"
#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
#include "xdprxss_hdcp22.h"
#endif

/************************** Constant Definitions *****************************/

/* XDPRXSS wrapper macros for XDP_RX register offsets - only used ones */
/** DPCD link bandwidth set register */
#define XDPRXSS_DPCD_LINK_BW_SET	XDP_RX_DPCD_LINK_BW_SET
/** User FIFO overflow register */
#define XDPRXSS_USER_FIFO_OVERFLOW	XDP_RX_USER_FIFO_OVERFLOW
/** Line reset disable register */
#define XDPRXSS_LINE_RESET_DISABLE	XDP_RX_LINE_RESET_DISABLE
/** Minimum voltage swing register */
#define XDPRXSS_MIN_VOLTAGE_SWING	XDP_RX_MIN_VOLTAGE_SWING
/** CDR control configuration register */
#define XDPRXSS_CDR_CONTROL_CONFIG	XDP_RX_CDR_CONTROL_CONFIG
/** MSA MISC0 register */
#define XDPRXSS_MSA_MISC0		XDP_RX_MSA_MISC0

/* XDPRXSS wrapper macros for XDP_RX interrupt masks - only used ones */
/** Info packet interrupt mask */
#define XDPRXSS_INTERRUPT_MASK_INFO_PKT_MASK	XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK
/** Vertical blanking interrupt mask */
#define XDPRXSS_INTERRUPT_MASK_VBLANK_MASK	XDP_RX_INTERRUPT_MASK_VBLANK_MASK
/** ACT RX interrupt mask */
#define XDPRXSS_INTERRUPT_MASK_ACT_RX_MASK	XDP_RX_INTERRUPT_MASK_ACT_RX_MASK
/** All interrupts mask */
#define XDPRXSS_INTERRUPT_MASK_ALL_MASK		XDP_RX_INTERRUPT_MASK_ALL_MASK
#define XDPRXSS_INTERRUPT1_MASK_MST_ALL		0x3FFFF	/**< MST stream interrupts (all 18 bits) */

/* XDPRXSS wrapper macros for XDP_RX CDR control config - only used ones */
/** CDR control config TDLOCK DP159 */
#define XDPRXSS_CDR_CONTROL_CONFIG_TDLOCK_DP159	XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159


/**************************** Type Definitions *******************************/
/**
* These constants specify the HDCP protection schemes
*/
typedef enum
{
  XDPRXSS_HDCP_NONE,       /**< No content protection */
  XDPRXSS_HDCP_14,         /**< HDCP 1.4 */
  XDPRXSS_HDCP_22,         /**< HDCP 2.2 */
  XDPRXSS_HDCP_BOTH        /**< Both HDCP 1.4 and 2.2 */
} XDpRxSs_HdcpProtocol;

/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from sub-cores.
*/
typedef enum {
	XDPRXSS_HANDLER_DP_VM_CHG_EVENT = 1,	/**< Video mode change event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,	/**< Power state change
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_NO_VID_EVENT,	/**< No video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VBLANK_EVENT,	/**< Vertical blanking event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_TLOST_EVENT,		/**< Training lost event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VID_EVENT,		/**< Valid video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,		/**< Info packet event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,	/**< Extension packet event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_TDONE_EVENT,		/**< Training done event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_BW_CHG_EVENT,	/**< Bandwidth change event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,	/**< Down request event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_DWN_REP_EVENT,	/**< Down reply event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT,	 /**< Audio packet overflow
						    *  event interrupt type for
						    *  DisplayPort core */
	XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,		/**< Payload allocation
							  *  event interrupt
							  *  type for
							  *  DisplayPort
							  *  core */
	XDPRXSS_HANDLER_DP_ACT_RX_EVENT,	/**< ACT sequence received
						  *  event interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,	/**< CRC test start event
						  *  interrupt type for
						  *  DisplayPort core */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT,	/**< Repeater Trigger
						  *  Downstream AUTH event
						  *  interrupt type for
						  *  HDCP core */
	XDPRXSS_HANDLER_HDCP_AUTHENTICATED,	/**< HDCP Authentication
						  *  completion interrupt type for  HDCP core */
#endif
#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
	XDPRXSS_HANDLER_HDCP22_AUTHENTICATED,	/**< Handler for HDCP22
						  authenticated event */
	XDPRXSS_HANDLER_HDCP22_UNAUTHENTICATED,	/**< Handler for HDCP22
						  authenticated event */
	XDPRXSS_HANDLER_HDCP22_AUTHENTICATION_REQUEST, /**< Handler for HDCP
							 authentication request
							 event */
	XDPRXSS_HANDLER_HDCP22_ENCRYPTION_UPDATE, /**< Handler for HDCP
						    encryption status update
						    event */
	XDPRXSS_HANDLER_HDCP22_SKE_SEND_EKS,	/**< Handler for HDCP Repeater
                                                     first stage of authentication
						     event */
#endif
	XDPRXSS_HANDLER_UNPLUG_EVENT,		/**< Unplug event type for
						  *  DisplayPort RX
						  *  Subsystem */
	XDPRXSS_HANDLER_LINKBW_EVENT,		/**< Link BW event type for
						  *  DisplayPort RX Subsystem
						  */
	XDPRXSS_HANDLER_PLL_RESET_EVENT,	/**< PLL reset event type for
						  *  DisplayPort RX Subsystem
						  */
	XDPRXSS_DRV_HANDLER_DP_PWR_CHG_EVENT,   /**< Drv power state change
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_DRV_HANDLER_DP_VID_EVENT,       /**< Drv Valid video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_DRV_HANDLER_DP_NO_VID_EVENT,     /**< Drv No video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_ACCESS_LANE_SET_EVENT,    /**< Drv Access lane set
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,    /**< Drv Access link qual
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,  /**< Drv Access error
						      *	counter interrupt type
						      *	for DisplayPort core */
	XDPRXSS_HANDLER_DP_VBLANK_STREAM_2_EVENT,    /**< Vertical blanking
						      *	event for stream 2
						      *	interrupt type for
						      *	DisplayPort core */
	XDPRXSS_HANDLER_DP_VBLANK_STREAM_3_EVENT,    /**< Vertical blanking
						      *	event for stream 3
						      *	interrupt type for
						      *	DisplayPort core */
	XDPRXSS_HANDLER_DP_VBLANK_STREAM_4_EVENT,    /**< Vertical blanking
						      *	event for stream 4
						      *	interrupt type for
						      *	DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_1_EVENT,	/**< Adaptive sync SDP
							     * packet event for stream 1
							     * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_2_EVENT,	/**< Adaptive sync SDP
								 * packet event for stream 2
								 * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_3_EVENT,	/**< Adaptive sync SDP
								 * packet event for stream 3
								 * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_4_EVENT,	/**< Adaptive sync SDP
								 * packet event for stream 4
								 * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_1_EVENT,	/**< Adaptive sync
							     * vblank event for stream 1
							     * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_2_EVENT,	/**< Adaptive sync
								 * vblank event for stream 2
								 * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_3_EVENT,	/**< Adaptive sync
								 * vblank event for stream 3
								 * DisplayPort core */
	XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_4_EVENT,	/**< Adaptive sync
								 * vblank event for stream 4
								 * DisplayPort core */
	XDPRXSS_HANDLER_DP_VM_CHG_STREAM_2_EVENT,		/**< Video mode change event
								  *  interrupt type for stream 2
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VM_CHG_STREAM_3_EVENT,		/**< Video mode change event
								  *  interrupt type for stream 3
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VM_CHG_STREAM_4_EVENT,		/**< Video mode change event
								  *  interrupt type for stream 4
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_NO_VID_STREAM_2_EVENT,		/**< No video event
								  *  interrupt type for stream 2
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_NO_VID_STREAM_3_EVENT,		/**< No video event
								  *  interrupt type for stream 3
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_NO_VID_STREAM_4_EVENT,		/**< No video event
								  *  interrupt type for stream 4
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VID_STREAM_2_EVENT,			/**< Valid video event
								  *  interrupt type for stream 2
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VID_STREAM_3_EVENT,			/**< Valid video event
								  *  interrupt type for stream 3
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VID_STREAM_4_EVENT,			/**< Valid video event
								  *  interrupt type for stream 4
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_2_EVENT,		/**< Info packet event
								  *  interrupt type for stream 2
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_3_EVENT,		/**< Info packet event
								  *  interrupt type for stream 3
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_4_EVENT,		/**< Info packet event
								  *  interrupt type for stream 4
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_2_EVENT,		/**< Extension packet event
								  *  interrupt type for stream 2
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_3_EVENT,		/**< Extension packet event
								  *  interrupt type for stream 3
								  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_4_EVENT,		/**< Extension packet event
								  *  interrupt type for stream 4
								  *  DisplayPort core */
} XDpRxSs_HandlerType;

/**
* User input structure
*/
typedef struct {
	u8 Bpc;			/**< Bits per color */
	u8 LaneCount;		/**< Lane count */
	u8 LinkRate;		/**< Link rate */
	u8 MstSupport;		/**< Multi-stream transport (MST) support */
	u8 NumOfStreams;	/**< The total number of MST streams */
} XDpRxSs_UsrOpt;

/*
 * User input structure for DpRxSs setup
 */
typedef struct {
    u32 BsIdleTimeout;      /**< Blank screen idle timeout value */
    u8  AuxClkDeferCount;   /**< AUX clock defer count for link status reads */
    u8  CrcEnable;          /**< CRC enable flag (1=enabled, 0=disabled) */
    u8  ChannelCoding;      /**< Channel coding: 8b/10b or 128b/132b */
} XDpRxSs_UserConfig;

/**
* DisplayPort Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of DisplayPort
				  *  Receiver core. */
	XDp_Config DpConfig;	/**< DisplayPort core configuration
				  *  information */
} XDpRxSs_DpSubCore;

/**
* This typedef contains configuration information for the
* DpRxSs I2C dummy subcore instance.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;	  /**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;  /**< Device base address */
	int Has10BitAddr; /**< Does device have 10 bit address decoding */
	u8 GpOutWidth;	  /**< Number of bits in general purpose output */
#ifdef SDT
	u16 IntrId;             /** Bits[11:0] Interrupt-id Bits[15:12]
				 * trigger type and level flags */
	UINTPTR IntrParent;     /** Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
} XDpRxSs_IicDummySubCoreConfig;

/**
* IIC Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of DisplayPort
				  *  Receiver core. */
#ifdef XPAR_XIIC_NUM_INSTANCES
	XIic_Config IicConfig;	/**< IIC core configuration
				  *  information */
#else
	XDpRxSs_IicDummySubCoreConfig	XIicConfig;	/**< Dummy IIC configuration */
#endif
} XDpRxSs_IicSubCore;

/**
* This typedef contains configuration information for the
* DpRxSs subcore instances.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;	/**< Device ID of the sub-core */
	UINTPTR AbsAddr;/**< Absolute Base Address of the Sub-cores*/
#else
	char *Name;
    UINTPTR BaseAddress;
#endif
} XDpRxSs_SubCoreConfig;

typedef struct {
	u16 IsPresent;	/**< Flag to hold the presence of clocking wizard */
	XDpRxSs_SubCoreConfig ClkWizConfig;	/**< Clocking Wizard core configuration
						 *  information */
} XDpRxSs_ClkWizSubCore;

/**
* High-Bandwidth Content Protection (HDCP) Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of HDCP core. */
	XDpRxSs_SubCoreConfig Hdcp1xConfig;	/**< HDCP core configuration
						information */
} XDpRxSs_Hdcp1xSubCore;

/**
 * Timer Counter Sub-core structure.
 */
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of Timer
				 *  Counter core */
	XDpRxSs_SubCoreConfig TmrCtrConfig;	/**< Timer Counter core
					 * configuration information */
} XDpRxSs_TmrCtrSubCore;

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
	u16 IsPresent;  /**< Flag to indicate if sub-core is present in
			  the design*/
	XDpRxSs_SubCoreConfig Hdcp22Config; /**< HDCP22 core configuration */
} XDpRxSs_Hdcp22SubCore;

/**
* This typedef contains configuration information for the DisplayPort
* Receiver Subsystem core. Each DisplayPort RX Subsystem core should have
* a configuration structure associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  DisplayPort RX Subsystem core */
#else
    char *Name;
#endif
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */
	u8 SecondaryChEn;	/**< This Subsystem core supports audio packets
				  *  being sent by the secondary channel. */
	u8 MaxNumAudioCh;	/**< The total number of Audio channels
				  *  supported by this core instance. */
	u8 MaxBpc;		/**< The maximum bits/color supported by this
				  *  Subsystem core */
	u8 HdcpEnable;		/**< This Subsystem core supports digital
				  *  content protection. */
	u8 Hdcp22Enable;	/**< This Subsystem core supports digital
				  content protection(HDCP22). */
	u8 MaxLaneCount;	/**< The maximum lane count supported by this
				  *  core instance. */
	u8 MstSupport;		/**< Multi-stream transport (MST) mode is
				  *  enabled by this core instance. */
	u8 NumMstStreams;	/**< The total number of MST streams supported
				  *  by this core instance. */
	u8 ColorFormat;		/**< Type of color format supported by this
				  *  core instance. */
	u8 IncludeAxiIic;  	/**< AXI IIC support flag */
	u8 IncludeClkWiz;	/**< Clocking wizard support for dec_clk */

	XDpRxSs_IicSubCore IicSubCore;	/**< IIC Configuration */
	XDpRxSs_TmrCtrSubCore TmrCtrSubCore;	/**< Timer Counter
						  *  Configuration */
	XDpRxSs_ClkWizSubCore ClkWizSubCore; /**< Clocking Wizard Configuration*/
	XDpRxSs_DpSubCore DpSubCore;	/**< DisplayPort Configuration */
	XDpRxSs_Hdcp1xSubCore Hdcp1xSubCore;	/**< HDCP Configuration */
	XDpRxSs_Hdcp22SubCore Hdcp22SubCore;	/**< HDCP 2.2 Configuration */
#ifdef SDT
	u32 IntrId[5];
	UINTPTR IntrParent;
#endif
} XDpRxSs_Config;

/*****************************************************************************/
/**
*
* Callback type which represents the handler for events.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
typedef void (*XDpRxSs_Callback)(void *InstancePtr);

/**
* The DisplayPort RX Subsystem driver instance data. An instance must be
* allocated for each core in use.
*/
typedef struct {
	XDpRxSs_Config Config;		/**< Hardware Configuration */
	u32 IsReady;			/**< Core and the driver instance are
					  *  initialized */
	/* Sub-core instances */
	XDp *DpPtr;			/**< DisplayPort sub-core instance */
#ifdef XPAR_XIIC_NUM_INSTANCES
	XIic *IicPtr;			/**< IIC sub-core instance */
#endif
#ifdef XPAR_XIICPS_NUM_INSTANCES
	XIicPs *IicPsPtr;		/**< PS i2c core instance */
#endif
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	XHdcp1x *Hdcp1xPtr;		/**< HDCP sub-core instance */
#endif
#if (((XPAR_XHDCP_NUM_INSTANCES > 0) || \
	(XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)) \
		&& (XPAR_XTMRCTR_NUM_INSTANCES > 0))
	XTmrCtr *TmrCtrPtr;		/**< Timer Counter sub-core instance */
	u8 TmrCtrResetDone;		/**< Timer reset done. This is used for
					  *  MacBook which authenticates just
					  *  after training is done. This
					  *  ensures that system does not do
					  *  anything until this variable set
					  *  to one.*/
#endif
#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
	XHdcp22_Rx_Dp  *Hdcp22Ptr;           /**< handle to HDCP22 sub-core driver
					    instance */
#endif

	/* Callback */
	XDpRxSs_Callback PllResetCallback;	/**< Callback function for PLL
						  *  reset */
	void *PllResetRef;		/**< A pointer to the user data passed
					  *  to the PLL reset callback
					  *  function */
	XDpRxSs_Callback LinkBwCallback;	/**< Callback function for
						  *  link bandwidth */
	void *LinkBwRef;		/**< A pointer to the user data passed
					  *  to the link bandwidth callback
					  *  function */
	XDpRxSs_Callback UnplugCallback;	/**< Callback function for
						  *  unplug event */
	void *UnplugRef;		/**< A pointer to the user data passed
					  *  to the unplug event callback
					  *  function */

	XDpRxSs_Callback AccessLaneSetCallback;	/**< Callback function for
						  *  AccessLaneSet */
	void *AccessLaneSetRef;		/**< A pointer to the user data passed
					  *  to the AccessLaneSet callback
					  *  function */
	/* Vertical blank */
	u8 VBlankEnable;		/**< Vertical Blank Enable */
	u8 VBlankCount;			/**< Vertical Blank Count */

	/* User options */
	XDpRxSs_UsrOpt UsrOpt;		/**< User Options structure */

	u8 ceItrCounter; 		/**< Equalization counter to
					  *  keep track of iterations */
	u32 ceRequestValue; 	/**< To keep track of previous value and
				  *  used to compare with current value*/
	u8 ltState; 		/**< To check if current LT is in CR or CE */
	u8 prevLinkRate;	/**< Previous link rate value */
	u8 prevLaneCounts;	/**< Previous lane count value */
	u8 link_up_trigger;	/**< Link up trigger flag */
	u8 no_video_trigger;	/**< No video trigger flag */
	XDpRxSs_HdcpProtocol HdcpProtocol; /**< HDCP protocol selected */
#if ((XPAR_XHDCP_NUM_INSTANCES > 0) || (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0))
	u8 HdcpIsReady;			/**< HDCP ready flag */
#endif
#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
	XDpRxSs_Hdcp22EventQueue Hdcp22EventQueue; /**< HDCP22 event queue */
	u8 *Hdcp22Lc128Ptr;		/**< Pointer to HDCP 2.2 LC128 */
	u8 *Hdcp22PrivateKeyPtr;	/**< Pointer to HDCP 2.2 Private key */
#endif
	u8 *EdidDataPtr[XDP_MAX_NPORTS];/**< Pointer to EDID Data */
	u16 EdidSize[XDP_MAX_NPORTS];	/**< Size of EDID Data */
	UINTPTR EdidBaseAddr;		/**< BaseAddress of Edid */
	UINTPTR clk_wiz_abs_addr;	/**< Clock Wizard absolute base address */
} XDpRxSs;

/***************** Macros (Inline Functions) Definitions *********************/

/**
* Callback type which represents a custom timer wait handler.
*/
#define XDpRxSs_TimerHandler		XDp_TimerHandler

/*****************************************************************************/
/**
*
* This function macro enables the display timing generator (DTG).
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @note		C-style signature:
*		void XDpRxSs_DtgEnable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_DtgEnable(InstancePtr)	XDp_RxDtgEn((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro disables the display timing generator (DTG).
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
*
* @note		C-style signature:
*		void XDpRxSs_DtgDisable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_DtgDisable(InstancePtr)	XDp_RxDtgDis((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro enables audio stream packets on the main link.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
*
* @note		C-style signature:
*		void XDpRxSs_AudioEnable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_AudioEnable(InstancePtr) \
	XDp_RxAudioEn((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro disables audio stream packets on the main link.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @note		C-style signature:
*		void XDpRxSs_AudioDisable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_AudioDisable(InstancePtr) \
	XDp_RxAudioDis((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro resets the reception of audio stream packets on the
* main link.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @note		C-style signature:
*		void XDpRxSs_AudioReset(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_AudioReset(InstancePtr) \
	XDp_RxAudioReset((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
 *
 * This function macro enable the stream address
 *
 * @param	StreamId is zero for SST, veries for MST.
 *
 * @note		C-style signature:
 *		XDP_RX_STREAM_MSA_OFFSET(StreamNumber)
 *
 *******************************************************i***********************/
#define XDP_RX_STREAM_MSA_OFFSET(StreamId) \
		(XDP_RX_STREAM1_MSA_START + ((StreamId) * 0x40))

/*****************************************************************************/
/**
 * This function macro enables MST audio on a given stream on the main link.
 *
 * @param      InstancePtr is a pointer to the XDpRxSs core instance.
 * @param      StreamId to be enabled audio
 *
 * @note       C-style signature:
 *             void XDpRxSs_Mst_AudioEnable(XDpRxSs *InstancePtr, u8 StreamId)
 *
 *******************************************************************************/
#define XDpRxSs_Mst_AudioEnable(InstancePtr, StreamId) \
        XDp_Rx_Mst_AudioEn((InstancePtr)->DpPtr, StreamId)

/*****************************************************************************/
/**
 *
 * This function macro disables audio on a given stream on the main link.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_Mst_AudioDisable(XDpRxSs *InstancePtr)
 *
 ******************************************************************************/
#define XDpRxSs_Mst_AudioDisable(InstancePtr) \
	XDp_RxAudioDis((InstancePtr)->DpPtr)

/******************************************************************************/
/**
*
* This function macro is the delay/sleep function for the XDpRxSs driver.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
* @param	MicroSeconds is the number of microseconds to delay/sleep for.
*
* @note		C-style signature:
*		void XDpRxSs_WaitUs(XDpRxSs *InstancePtr, u32 MicroSeconds)
*
*******************************************************************************/
#define XDpRxSs_WaitUs(InstancePtr, MicroSeconds) \
	XDp_WaitUs((InstancePtr)->DpPtr, MicroSeconds)

/*****************************************************************************/
/**
 *
 * This function macro Enables/Disables the Dp Rx link.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Enable is to enable/disable the Dp Rx link.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void DpRxSs_RxEnable(XDpRxSs *InstancePtr, u32 Enable)
 *
 ******************************************************************************/
#define DpRxSs_RxEnable(InstancePtr, Enable)				       \
	XDp_WriteReg((InstancePtr)->DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE,\
		     Enable)

/*****************************************************************************/
/**
 *
 * This function macro provides link Quality of Dp Rx lane.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 *
 * @return       Link quality of lane.
 *
 * @note         C-style signature:
 *               u8 DpRxSs_GetLinkQualLane(XDpRxSs *InstancePtr)
 *
 ******************************************************************************/
#define DpRxSs_GetLinkQualLane(InstancePtr)				\
	(u8)(XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,		\
			 XDPRXSS_DPC_LINK_QUAL_CONFIG) &		\
	     XDPRXSS_DPC_LINK_QUAL_PRBS_MASK)

/*****************************************************************************/
/**
 *
 * This function macro provides user fifo overflow status.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 *
 * @return       user fifo overflow status.
 *
 * @note         C-style signature:
 *               u8 XDpRxSs_UserFifoOverflowStatus(XDpRxSs *InstancePtr)
 *
 ******************************************************************************/
#define XDpRxSs_UserFifoOverflowStatus(InstancePtr)			\
	(XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,		\
		     XDP_RX_USER_FIFO_OVERFLOW))

/*****************************************************************************/
/**
 *
 * This function macro provides RX line reset disable status.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 *
 * @return       RX line reset disable status.
 *
 * @note         C-style signature:
 *               u32 XDpRxSs_GetRxResetDisable(XDpRxSs *InstancePtr)
 *
 ******************************************************************************/
#define XDpRxSs_GetRxResetDisable(InstancePtr)			\
	(XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,		\
		     XDP_RX_LINE_RESET_DISABLE))

/*****************************************************************************/
/**
 *
 * This function macro provides the contents of DPCD registers
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 *
 * @return       DPCD registers content.
 *
 * @note         C-style signature:
 *               u8 XDpRxSs_GetRxMstAlloc(XDpRxSs *InstancePtr)
 *
 ******************************************************************************/
#define XDpRxSs_GetRxMstAlloc(InstancePtr)				\
	(XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,		\
		     XDP_RX_MST_ALLOC))

/*****************************************************************************/
/**
 *
 * This function macro provides Rx Audio info data.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Index of the registers.
 *
 * @return       Rx audio info.
 *
 * @note         C-style signature:
 *               u32 DpRxSs_GetRxAudioInfo(XDpRxSs *InstancePtr, u8 Index)
 *
 ******************************************************************************/
#define DpRxSs_GetRxAudioInfo(InstancePtr, Index)			\
	(XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,		\
		     XDP_RX_AUDIO_INFO_DATA(Index)))

/*****************************************************************************/
/**
 *
 * This function macro provides Rx Audio Extension packet data.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Index of the registers.
 *
 * @return       Rx Audio Extension packet data.
 *
 * @note         C-style signature:
 *               u32 DpRxSs_GetRxAudioExtInfo(InstancePtr, Index)
 *
 ******************************************************************************/
#define DpRxSs_GetRxAudioExtInfo(InstancePtr, Index)			\
	(XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,		\
		     XDP_RX_AUDIO_EXT_DATA(Index)))

/*****************************************************************************/
/**
 *
 * This function macro Update the CDR config value of Dp Rx.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        CdrConfig is combined value of CDR timeout and timeout disable.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void DpRxSs_UpdateCdrConfig(XDpRxSs *InstancePtr, u32 CdrConfig)
 *
 ******************************************************************************/
#define DpRxSs_UpdateCdrConfig(InstancePtr, CdrConfig)				\
	XDp_WriteReg((InstancePtr)->DpPtr->Config.BaseAddr,			\
		     XDP_RX_CDR_CONTROL_CONFIG, CdrConfig)

/*****************************************************************************/
/**
 *
 * This function macro updates the CDR timeout value (bits [19:0]) without
 * disturbing the Disable Training Timeout bit (bit 30).
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        TimeOutVal is the timeout value to be set (20-bit value).
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_UpdateCdrTimeOutVal(XDpRxSs *InstancePtr, u32 TimeOutVal)
 *
 ******************************************************************************/
#define XDpRxSs_UpdateCdrTimeOutVal(InstancePtr, TimeOutVal)			\
	XDp_WriteReg((InstancePtr)->DpPtr->Config.BaseAddr,			\
		     XDP_RX_CDR_CONTROL_CONFIG,					\
		     (XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,	\
				  XDP_RX_CDR_CONTROL_CONFIG) & 			\
		      ~XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_TO_MASK) |		\
		     ((TimeOutVal) & XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_TO_MASK))

/*****************************************************************************/
/**
 *
 * This function macro controls the CDR Disable Training Timeout bit (bit 30).
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Disable is 1 to disable training timeout, 0 to enable it.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_CdrDisableTimeout(XDpRxSs *InstancePtr, u8 Disable)
 *
 ******************************************************************************/
#define XDpRxSs_CdrDisableTimeout(InstancePtr, Disable)                             \
    XDp_WriteReg((InstancePtr)->DpPtr->Config.BaseAddr, XDP_RX_CDR_CONTROL_CONFIG, \
    (XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr, XDP_RX_CDR_CONTROL_CONFIG) \
    & ~XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT)                                  \
    | ((Disable) ? XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT : 0u))

/*****************************************************************************/
/**
 *
 * This function macro Reset the AUX logic from Dp RX.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        AuxReset is AUX logic reset value.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_AuxReset(XDpRxSs *InstancePtr, u8 AuxReset)
 *
 ******************************************************************************/
#define XDpRxSs_AuxReset(InstancePtr, AuxReset)					\
	XDp_WriteReg((InstancePtr)->DpPtr->Config.BaseAddr,			\
		     XDP_RX_SOFT_RESET,						\
		     ((XDp_ReadReg((InstancePtr)->DpPtr->Config.BaseAddr,	\
		       XDP_RX_SOFT_RESET) & (~XDP_RX_SOFT_RESET_AUX_MASK))	\
		      | (AuxReset << XDPRXSS_SOFT_RESET_AUX_SHIFT)))

/*****************************************************************************/
/**
 *
 * This function macro generates HPD (Hot-Plug-Detect) pulse interrupt.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        DurationUs is the duration of the HPD pulse in microseconds.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_GenerateHpdInterrupt(XDpRxSs *InstancePtr, u32 DurationUs)
 *
 ******************************************************************************/
#define XDpRxSs_GenerateHpdInterrupt(InstancePtr, DurationUs)	\
	XDp_RxGenerateHpdInterrupt((InstancePtr)->DpPtr, DurationUs)

/*****************************************************************************/
/**
 *
 * This function macro enables RX interrupt.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Mask is the interrupt mask to enable.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_RxInterruptEnable(XDpRxSs *InstancePtr, u32 Mask)
 *
 ******************************************************************************/
#define XDpRxSs_RxInterruptEnable(InstancePtr, Mask)	\
	XDp_RxInterruptEnable((InstancePtr)->DpPtr, Mask)

/*****************************************************************************/
/**
 *
 * This function macro disables RX interrupt.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Mask is the interrupt mask to disable.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_RxInterruptDisable(XDpRxSs *InstancePtr, u32 Mask)
 *
 ******************************************************************************/
#define XDpRxSs_RxInterruptDisable(InstancePtr, Mask)	\
	XDp_RxInterruptDisable((InstancePtr)->DpPtr, Mask)

/*****************************************************************************/
/**
 *
 * This function macro enables RX interrupt (Interrupt Enable 1 register).
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Mask is the interrupt mask to enable.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_RxInterruptEnable1(XDpRxSs *InstancePtr, u32 Mask)
 *
 ******************************************************************************/
#define XDpRxSs_RxInterruptEnable1(InstancePtr, Mask)	\
	XDp_RxInterruptEnable1((InstancePtr)->DpPtr, Mask)

/*****************************************************************************/
/**
 *
 * This function macro disables RX interrupt (Interrupt Enable 1 register).
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 * @param        Mask is the interrupt mask to disable.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_RxInterruptDisable1(XDpRxSs *InstancePtr, u32 Mask)
 *
 ******************************************************************************/
#define XDpRxSs_RxInterruptDisable1(InstancePtr, Mask)	\
	XDp_RxInterruptDisable1((InstancePtr)->DpPtr, Mask)

/*****************************************************************************/
/**
 *
 * This function macro allocates payload stream for MST mode.
 *
 * @param        InstancePtr is a pointer to the XDpRxSs core instance.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpRxSs_RxAllocatePayloadStream(XDpRxSs *InstancePtr)
 *
 ******************************************************************************/
#define XDpRxSs_RxAllocatePayloadStream(InstancePtr)	\
	XDp_RxAllocatePayloadStream((InstancePtr)->DpPtr)

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XDpRxSs_Printf		XHdcp1x_Printf	/**< Debug printf */
#define XDpRxSs_LogMsg		XHdcp1x_LogMsg	/**< Debug log message */
#endif

/************************** Function Prototypes ******************************/
#ifndef SDT
/* Initialization function in xdprxss_sinit.c */
/** Looks up the device configuration based on the unique device ID. */
XDpRxSs_Config* XDpRxSs_LookupConfig(u16 DeviceId);
#else
XDpRxSs_Config *XDpRxSs_LookupConfig(UINTPTR BaseAddress);
u32 XDpRxSs_GetDrvIndex(UINTPTR BaseAddress);
#endif
/* Initialization and control functions in xdprxss.c */
u32 XDpRxSs_CfgInitialize(XDpRxSs *InstancePtr, XDpRxSs_Config *CfgPtr,
				UINTPTR EffectiveAddr);
u32 XDpRxSs_Start(XDpRxSs *InstancePtr);
void XDpRxSs_Setup(XDpRxSs *InstancePtr, XDpRxSs_UserConfig *UserConfigPtr);
void DpRxSs_UpdatePrbsErrCouters(XDpRxSs *InstancePtr, u16 Lane0, u16 Lane1,
				 u16 Lane2, u16 Lane3);
void XDpRxSs_Reset(XDpRxSs *InstancePtr);
u32 XDpRxSs_SetLinkRate(XDpRxSs *InstancePtr, u8 LinkRate);
u32 XDpRxSs_SetLaneCount(XDpRxSs *InstancePtr, u8 LaneCount);
u32 XDpRxSs_ExposePort(XDpRxSs *InstancePtr, u8 Port);
u32 XDpRxSs_CheckLinkStatus(XDpRxSs *InstancePtr);
u32 XDpRxSs_HandleDownReq(XDpRxSs *InstancePtr);
void XDpRxSs_HandleActRxInterrupts(XDpRxSs *InstancePtr);
void XDpRxSs_SetUserPixelWidth(XDpRxSs *InstancePtr, u8 UserPixelWidth);
u8 XDpRxss_GetBpc(XDpRxSs *InstancePtr, u8 Stream);
u8 XDpRxss_GetColorComponent(XDpRxSs *InstancePtr, u8 Stream);
u8 XDpRxss_GetColorimetry(XDpRxSs *InstancePtr, u8 Stream);
u8 XDpRxss_GetDynamicRange(XDpRxSs *InstancePtr, u8 Stream);

#if (XPAR_DPRXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
int XDpRxSs_HdcpSetProtocol(XDpRxSs *InstancePtr,
		XDpRxSs_HdcpProtocol Protocol);
/* Optional HDCP related functions */
u32 XDpRxSs_HdcpEnable(XDpRxSs *InstancePtr);
u32 XDpRxSs_HdcpDisable(XDpRxSs *InstancePtr);
u32 XDpRxSs_SetLane(XDpRxSs *InstancePtr, u32 Lane);
void XDpRxSs_StartTimer(XDpRxSs *InstancePtr);
void XDpRxSs_StopTimer(XDpRxSs *InstancePtr);
#endif
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
/* Optional HDCP related functions */
u32 XDpRxSs_Poll(XDpRxSs *InstancePtr);
u32 XDpRxSs_SetPhysicalState(XDpRxSs *InstancePtr, u32 PhyState);
u32 XDpRxSs_Authenticate(XDpRxSs *InstancePtr);
u32 XDpRxSs_IsAuthenticated(XDpRxSs *InstancePtr);
u64 XDpRxSs_GetEncryption(XDpRxSs *InstancePtr);
void XDpRxSs_SetDebugPrintf(XDpRxSs *InstancePtr, XDpRxSs_Printf PrintfFunc);
void XDpRxSs_SetDebugLogMsg(XDpRxSs *InstancePtr, XDpRxSs_LogMsg LogFunc);
u32 XDpRxSs_DownstreamReady(XDpRxSs *InstancePtr);
void XDpRxSs_HandleTimeout(XDpRxSs *InstancePtr);
#endif

/** Reports information about the DisplayPort RX Subsystem cores. */
void XDpRxSs_ReportCoreInfo(XDpRxSs *InstancePtr);
/** Reports information about the current link status. */
void XDpRxSs_ReportLinkInfo(XDpRxSs *InstancePtr);
/** Reports Main Stream Attribute (MSA) information. */
void XDpRxSs_ReportMsaInfo(XDpRxSs *InstancePtr);
/**
 * Reports DP159 bit error count.
 * @param InstancePtr is a pointer to the XDpRxSs core instance.
 */
void XDpRxSs_ReportDp159BitErrCount(XDpRxSs *InstancePtr);
/** Reports HDCP information and status. */
void XDpRxSs_ReportHdcpInfo(XDpRxSs *InstancePtr);
u32 XDpRxSs_GetLinkRate(XDpRxSs *InstancePtr);

/* Self test function in xdprxss_selftest.c */
/** Runs a self-test on the DisplayPort RX Subsystem. */
u32 XDpRxSs_SelfTest(XDpRxSs *InstancePtr);

/* Interrupt functions in xdprxss_intr.c */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
void XDpRxSs_HdcpIntrHandler(void *InstancePtr);
#endif
#if (XPAR_XHDCP_NUM_INSTANCES > 0) || (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
void XDpRxSs_TmrCtrIntrHandler(void *InstancePtr);
#endif
void XDpRxSs_DpIntrHandler(void *InstancePtr);
u32 XDpRxSs_SetCallBack(XDpRxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);
void XDpRxSs_SetUserTimerHandler(XDpRxSs *InstancePtr,
		XDpRxSs_TimerHandler CallbackFunc, void *CallbackRef);
/* DpRxSs Interrupt Related Function */
void XDpRxSs_DrvNoVideoHandler(void *InstancePtr);
void XDpRxSs_DrvVideoHandler(void *InstancePtr);
void XDpRxSs_DrvPowerChangeHandler(void *InstancePtr);

/* Adaptive-Sync Related Functions */
void XDpRxSs_SetAdaptiveSyncCaps(XDpRxSs *InstancePtr, u32 Enable);
void XDpRxSs_MaskAdaptiveIntr(XDpRxSs *InstancePtr, u32 Mask);
void XDpRxSs_UnMaskAdaptiveIntr(XDpRxSs *InstancePtr, u32 Mask);
int XDpRxSs_GetVblank(XDpRxSs *InstancePtr, u8 Stream);
int XDpRxSs_GetVtotal(XDpRxSs *InstancePtr, u8 Stream);
XDp_MainStreamAttributes *XDpRxss_GetMsa(XDpRxSs *DpRxSsInst);
int XDpRxSs_GetVideoStream(XDpRxSs *InstancePtr, XVidC_VideoStream *VideoStream,
			   u8 Stream);
u8 XDpRxss_GetInterlace(XDpRxSs *InstancePtr, u8 Stream);
void XDpRxSs_MstEnable(XDpRxSs *InstancePtr, u8 Enable);

#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
void XDpRxSs_Hdcp22LicFailHandler(void *InstancePtr);
void XDpRxSs_Hdcp22SetKey(XDpRxSs *InstancePtr,
		XDpRxSs_Hdcp22KeyType KeyType, u8 *KeyPtr);
#endif
int XDpRxSs_Get_Dec_Clk_Lock(XDpRxSs *InstancePtr);
/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
