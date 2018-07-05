/******************************************************************************
*
* Copyright (C) 2008 - 2018 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xavb.h
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xavb.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a mbr  09/19/08 First release
* 1.01a mbr  06/24/09 PTP frame format updates for IEEE802.1 AS draft 5-0
* 2_02a mbr  09/16/09 Updates for programmable PTP timers
* 2_04a kag  07/23/10 PTP frame format updates for IEEE802.1 AS draft 6-7
* 3_01a kag  08/29/11 Added new APIs to update the RX Filter Control Reg.
*		      Fix for CR:572539. Updated bit map for Rx Filter
*		      control reg.
* 4_0	asa  03/06/14 Fix for CR 740863. The value of XAVB_CLOCK_LOCK_THRESHOLD
*					  is increased to 1000ns (1 us) to make it more
*					  meningful and reasonable.
*
* </pre>
*
******************************************************************************/

#ifndef XAVB_H    /* prevent circular inclusions */
#define XAVB_H    /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xavb_hw.h"
#include "xstatus.h"
#include "stdio.h"

/************************** Constant Definitions *****************************/

/** @name Define the Debug Level Verbosity for print messages for this driver
 *  @{
 */
#undef DEBUG_XAVB_LEVEL1  /* Basic messages and PTP frame reception messages */
#undef DEBUG_XAVB_LEVEL2  /* PTP Calculation messages */
#undef DEBUG_XAVB_LEVEL3  /* PTP frame transmission messages */

/** @name MAC Latency Definitions
 *  @{
 */
#define XAVB_TX_MAC_LATENCY_IN_NS                            80
#define XAVB_RX_MAC_LATENCY_IN_NS                            80
/* @} */


/** @name PTP Buffer Storage Definitions
 *  @{
 */
#define XAVB_PTP_TX_SYNC                                     0x0
#define XAVB_PTP_TX_FOLLOW_UP                                0x1
#define XAVB_PTP_TX_PDELAYREQ                                0x2
#define XAVB_PTP_TX_PDELAYRESP                               0x3
#define XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP                     0x4
#define XAVB_PTP_TX_ANNOUNCE                                 0x5
#define XAVB_PTP_TX_SIGNALING                                0x6
/* @} */


/** @name PTP Packet Message Type Definitions
 *  @{
 */
#define XAVB_PTP_TYPE_SYNC                                   0x0
#define XAVB_PTP_TYPE_FOLLOW_UP                              0x8
#define XAVB_PTP_TYPE_PDELAYREQ                              0x2
#define XAVB_PTP_TYPE_PDELAYRESP                             0x3
#define XAVB_PTP_TYPE_PDELAYRESP_FOLLOW_UP                   0xA
#define XAVB_PTP_TYPE_ANNOUNCE                               0xB
#define XAVB_PTP_TYPE_SIGNALING                              0xC
/* @} */


/** @name Common PTP Frame Structure Definitions.
 *  @{
 */
#define XAVB_PTP_PKT_CAPTURED_TIMESTAMP_OFFSET               0x000000FC
/* @} */


/** @name General Tx PTP Frame Structure Definitions.
 *  @{
 */
#define XAVB_PTP_TX_PKT_SA_UPPER_OFFSET                      0x0000000C
#define XAVB_PTP_TX_PKT_SA_LOWER_OFFSET                      0x00000010
#define XAVB_PTP_TX_PKT_TYPE_OFFSET                          0x00000014
#define XAVB_PTP_TX_PKT_CORRECTION_FIELD_OFFSET              0x00000020
#define XAVB_PTP_TX_PKT_PORTID_UPPER_OFFSET                  0x00000028
#define XAVB_PTP_TX_PKT_PORTID_MID_OFFSET                    0x0000002C
#define XAVB_PTP_TX_PKT_PORTID_LOWER_OFFSET                  0x00000030
#define XAVB_PTP_TX_PKT_SEQUENCEID_OFFSET                    0x00000034
#define XAVB_PTP_TX_PKT_TIMESTAMP_UPPER_OFFSET               0x00000038
#define XAVB_PTP_TX_PKT_TIMESTAMP_MID_OFFSET                 0x0000003C
#define XAVB_PTP_TX_PKT_TIMESTAMP_LOWER_OFFSET               0x00000040  /* upper 2 bytes */
#define XAVB_PTP_TX_PKT_REQ_PORTID_UPPER_OFFSET              0x00000040  /* lower 2 bytes */
#define XAVB_PTP_TX_PKT_REQ_PORTID_MID_OFFSET                0x00000044
#define XAVB_PTP_TX_PKT_REQ_PORTID_LOWER_OFFSET              0x00000048
/* @} */

/** @name Tx Announce PTP Frame Structure Definitions.
 *  @{
 */
#define XAVB_PTP_TX_PKT_ANNOUNCE_CURR_UTCOFF_OFFSET               0x00000040
#define XAVB_PTP_TX_PKT_ANNOUNCE_QUAL_LOW_PRI2_GMID_HI_OFFSET     0x00000048
#define XAVB_PTP_TX_PKT_ANNOUNCE_GMID_MID_OFFSET                  0x0000004C
#define XAVB_PTP_TX_PKT_ANNOUNCE_GMID_LOW_STEPSREMOVED_HI_OFFSET  0x00000050
#define XAVB_PTP_TX_PKT_ANNOUNCE_STEPSREMOVED_LOW_TIMESRC_OFFSET  0x00000054
#define XAVB_PTP_TX_PKT_ANNOUNCE_TLVLEN_PATHSEQ_START_OFFSET      0x00000058
/* @} */

/** @name PTP frame constant definitions
 *  @{
 *
 *  Constant value for the control field and LogMeanMessageInterval field for
 *  pDelayResp and pDelayRespFollowUp PTP frames (defined in IEEE802.1 AS)
 */
#define XAVB_PDELAY_LOG_MEAN_MESSAGE_INT                     0x7F050000
/* @} */


/** @name Default LogMeanInterval values
*   @{
*/
#define XAVB_DEFAULT_LOG_MEAN_SYNC_INTERVAL                  (-3)      /** 2^(-3) = 125ms */
#define XAVB_DEFAULT_LOG_MEAN_PDELAY_REQ_INTERVAL            0         /** 128/128 = 2^(0) = 1 s */
#define XAVB_DEFAULT_LOG_MEAN_ANNOUNCE_INTERVAL              0         /** 128/128 = 2^(0) = 1 s */
#define XAVB_PKT_TYPE_DISABLED                               0xffff

#define XAVB_MAX_SUPPORTED_LOG_MEAN_INTERVAL                 8         /** 32768/128 = 256 s */
#define XAVB_MIN_SUPPORTED_LOG_MEAN_INTERVAL                 (-7)      /** 1/128 s */
/* @} */

/** @name Announce / Sync Timeout values
*   @{
*/
/** XAVB_ANNOUNCE_RECEIPT_TIMEOUT is the number of announce
 *  intervals without the receipt of an Announce message from
 *  the GM that are allowed before the GM is assumed to be no
 *  longer present and BMCA should be run again
 */
#define XAVB_ANNOUNCE_RECEIPT_TIMEOUT                        2

/** XAVB_SYNC_RECEIPT_TIMEOUT is the number of sync intervals
 *  without the receipt of an Sync message from the GM that are
 *  allowed before the GM is assumed to be no longer present and
 *  BMCA should be run again
 */
#define XAVB_SYNC_RECEIPT_TIMEOUT                            3
/* @} */


/** @name AS Network Requirements
*   @{
*/
/** XAVB_NEIGHBOR_PROP_DELAY_THRESH is the maximum allowed delay (in nanosecs)
 *  across a full duplex link for which the AVB protocol is allowed to function.
 *  Although this parameter is defined in the IEEE spec, no default is defined.
 */
#define XAVB_NEIGHBOR_PROP_DELAY_THRESH                      5000

/** XAVB_ALLOWED_LOST_RESPONSES is the number of Pdelay_Req
 *  messages for which a valid response is not received, above
 *  which the Peer should no longer be considered ASCapable
 */
#define XAVB_ALLOWED_LOST_RESPONSES                          3

/* @} */


/** @name General Rx PTP Frame Structure Definitions.
 *  @{
 */
#define XAVB_PTP_RX_PKT_SA_UPPER_OFFSET                      0x00000004
#define XAVB_PTP_RX_PKT_SA_LOWER_OFFSET                      0x00000008
#define XAVB_PTP_RX_PKT_TYPE_OFFSET                          0x0000000C
#define XAVB_PTP_RX_PKT_CORRECTION_FIELD_OFFSET              0x00000018
#define XAVB_PTP_RX_PKT_PORTID_UPPER_OFFSET                  0x00000020
#define XAVB_PTP_RX_PKT_PORTID_MID_OFFSET                    0x00000024
#define XAVB_PTP_RX_PKT_PORTID_LOWER_OFFSET                  0x00000028
#define XAVB_PTP_RX_PKT_SEQUENCEID_OFFSET                    0x0000002C
#define XAVB_PTP_RX_PKT_TIMESTAMP_UPPER_OFFSET               0x00000030
#define XAVB_PTP_RX_PKT_TIMESTAMP_MID_OFFSET                 0x00000034
#define XAVB_PTP_RX_PKT_TIMESTAMP_LOWER_OFFSET               0x00000038  /* upper 2 bytes */
#define XAVB_PTP_RX_PKT_REQ_PORTID_UPPER_OFFSET              0x00000038  /* lower 2 bytes */
#define XAVB_PTP_RX_PKT_REQ_PORTID_MID_OFFSET                0x0000003C
#define XAVB_PTP_RX_PKT_REQ_PORTID_LOWER_OFFSET              0x00000040
/* @} */


/** @name Rx Announce PTP Frame Structure Definitions.
 *  @{
 */
#define XAVB_PTP_RX_PKT_ANNOUNCE_PRI1_QUAL_HI_OFFSET              0x0000003C
#define XAVB_PTP_RX_PKT_ANNOUNCE_QUAL_LOW_PRI2_GMID_HI_OFFSET     0x00000040
#define XAVB_PTP_RX_PKT_ANNOUNCE_GMID_MID_OFFSET                  0x00000044
#define XAVB_PTP_RX_PKT_ANNOUNCE_GMID_LOW_STEPSREMOVED_HI_OFFSET  0x00000048
#define XAVB_PTP_RX_PKT_ANNOUNCE_STEPSREMOVED_LOW_TIMESRC_OFFSET  0x0000004C
/* @} */


/** @name Rx Signalling PTP Frame Structure Definitions.
 *  @{
 */
#define XAVB_PTP_RX_PKT_SIGNALING_DELAY_INTERVAL_OFFSET      0x00000044
/* @} */

/** @name Standard PTP Frame Field Definitions (from IEEE802.1AS specification).
 *  @{
 */
#define XAVB_PTP_ETHERTYPE                                   0x88F7
#define XAVB_PTP_VERSION_PTP                                 2
/* @} */


/** @name Real Time Clock Definitions.
 *  @{
 */
#define XAVB_ONE_SECOND            1000000000    /**< Value in ns */
#define XAVB_PERIOD_8KHZ           125000        /**< Value in ns */
/* @} */

/** @name Real Time Clock Locked Definitions.
 *  @{
 *  If the Slave error is > this threshold then define PTP to be
 *  unlocked and link not asCapable.
 *  @note: This threshold is not specified in IEEE 802.1as.
 */
#define XAVB_CLOCK_LOCK_THRESHOLD  1000           /**< Value in ns */
/* @} */

/** @name RTC Increment Value Update Definitions
 *  @{
 *  Define how often to re-calculate the RTC Increment This value
 *  indicates how many good Sync/FollowUp message pairs
 *  are received before the re-calculation is performed.
 */
#define XAVB_NUM_SYNC_FU_PAIR_CALC_RTC_INCREMENT     2
/* @} */

/**************************** Type Definitions *******************************/

/**
 * Callback function.  The first argument is a callback reference passed in by
 * the upper layer when setting the callback functions, and passed back to the
 * upper layer when the callback is invoked.
 * The second argument is a '1' if a Grandmaster Discontinuity event has occurred
 * otherwise it is '0'.
 */
typedef void (*XAvb_Handler)(void *CallBackRef, u32 TimestampsUncertain);

/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
  u16 DeviceId;       /**< Unique ID  of device */
  u32 BaseAddress;    /**< Register base address */
} XAvb_Config;


/**
 * This typedef defines the format for a data structure which stores the Port
 * Identity information from received Announce packets
 */
typedef struct
{
  u32 ClockIdentityUpper;     /**< Upper 4 bytes of ClockIdentity */
  u32 ClockIdentityLower;     /**< Lower 4 bytes of ClockIdentity*/
  u16 PortNumber;             /**< PortNumber associated with ClockIdentity*/
} XAvb_PortIdentity;

/**
 * This typedef defines the format for a data structure which stores the Clock
 * Identity information from received Announce packets
 */
typedef struct
{
  u32 ClockIdentityUpper;     /**< Upper 4 bytes of ClockIdentity */
  u32 ClockIdentityLower;     /**< Lower 4 bytes of ClockIdentity*/
} XAvb_ClockIdentity;

/**
 * This typedef defines the quality of a clock
 */
typedef struct
{
  u8  clockClass;                /**< Announce message: clockClass */
  u8  clockAccuracy;             /**< Announce message: clockAccuracy */
  u16 offsetScaledLogVariance;   /**< Announce message: offsetScaledLogVariance*/
} XAvb_ClockQuality;

/**
 * This typedef defines the format for a data structure which stores the relevant
 * fields which are captured from Announce Packets.
 */
typedef struct
{
  XAvb_PortIdentity SourcePortIdentity;        /**< Announce message: sourcePortIdentity */
  XAvb_ClockIdentity GrandmasterIdentity;      /**< Announce message: grandmasterIdentity */
  u16               stepsRemoved;              /**< Announce message: stepsRemoved */
  XAvb_ClockQuality ClockQuality;              /**< Announce message: grandmasterClockQuality */
  u8                GrandmasterPriority1;      /**< Announce message: grandmasterPriority1*/
  u8                GrandmasterPriority2;      /**< Announce message: grandmasterPriority2*/
  u8                IAmTheRtcMaster;           /**< Boolean: 1 = grandmaster, 0 = slave*/
  u16               tlvLengthField;            /**< Announce message: lengthField (for TLV)*/
  char              logMessageInterval;        /**< Announce message: logMessageInterval.
                                                *   NOTE: 8-bit signed integer */
  u16               AnnounceIntervalDuration;  /**< Announce Interval in units of 1/128 secs */
} XAvb_BmcData;


/**
 * This typedef defines the format for a data structure which stores information
 * relating to the 1588 based PTP timing calculations
 */
typedef struct
{
  u32 Nanosec;                 /**< ns value for sync frame tx request   */
  u32 SlaveSyncTimestamp;      /**< The timestamp taken of a rx'd sync   */
  u32 MasterCorrectionField;   /**< Correction Field from rx'd follow-up */
  u32 PDelayTimestampT1;       /**< T1 :PDelayReq Frame transmission     */
  u32 PDelayTimestampT2;       /**< T2 :PDelayReq rx'd at link partner   */
  u32 PDelayTimestampT3;       /**< T3 :PDelayResp Frame reception       */
  u32 PDelayTimestampT4;       /**< T4 :PDelayResp tx'd by link partner  */
  u32 LinkDelay;               /**< Last calculated value of Link Delay  */
  u32 NewSlaveTime;            /**< RTC ns at slave for last rx'd sync   */
  u32 NewMasterTime;           /**< RTC ns at master for last tx'd sync  */
  u32 OldSlaveTime;            /**< Stored RTC slave ns for past sync rx */
  u32 OldMasterTime;           /**< Stored RTC master ns for past sync tx*/
  u32 NsOffsetForPDelayResp;   /**< RTC ns offset at PDelayResp tx time  */
} XAvb_PtpStruct;


/**
 * This typedef defines the format for a data structure which stores the last
 * used sequence ID for all of the PTP timing frames.
 */
typedef struct
{
  u16 SyncSequenceId;            /**< SequenceId of the latest RX'd Sync message  */
  u16 FollowUpSequenceId;        /**< SequenceId of the latest RX'd FollowUp message  */
  u16 PDelayReqSequenceId;       /**< SequenceId of the latest TX'd PDelayReq message  */
  u16 PDelayRespSequenceId;      /**< SequenceId of the latest RX'd PDelayResp message  */
  u16 PDelayFollowUpSequenceId;  /**< SequenceId of the latest RX'd PDelayRespFollowUp message  */
  u16 OldSyncSequenceId;         /**< SequenceId of the previous RX'd Sync message used
                                  *   for XAvb_UpdateRtcIncrement() calculations */
  u16 NewSyncSequenceId;         /**< SequenceId of the current RX'd Sync message used
                                  * for XAvb_UpdateRtcIncrement() calculations */
} XAvb_SequenceIdStruct;


/**
 * The Signalling frame defines the delays to be used between Sync Frames, Link
 * Delay measurements and Announce Frame events
 */
typedef struct
{
  u16  SyncIntervalDuration;      /**< Sync Interval in units of 1/128 seconds */
  u16  LinkDelayIntervalDuration; /**< Link Delay Interval in units of 1/128 seconds  */
  u16  AnnounceIntervalDuration;  /**< Announce Interval in units of 1/128 seconds  */
} XAvb_SignallingFrameData;


/**
 * This typedef defines the various counters which have to maintained for the
 * PTP operation.
 */
typedef struct
{
  u8  RxPtpHardPointer;          /**< The Rx PTP Buffer last written to */
  u8  RxPtpSoftPointer;          /**< The current software pointer to Rx Buffer */
  u16 CounterSyncInterval;       /**< To count units of 1/128 seconds */
  u16 CounterLinkDelayInterval;  /**< To count units of 1/128 seconds */
  u16 CounterAnnounceInterval;   /**< To count units of 1/128 seconds */
  u8  CounterSyncEvents;         /**< To count the number of Sync Events */
} XAvb_Counters;

/** Keep track of state machine data to make sure we're fully
 *  compliant with the spec
 */
typedef struct
{
  u8                lostResponses;            /**< Keep track of the state machine errors */
  u8                rcvdPDelayResp;           /**< Received a valid PDelayResp packet since PDelayReq was sent */
  u8                rcvdPDelayRespFollowUp;   /**< Received a valid PDelayFollowUp packet since PDelayResp was received */
  XAvb_PortIdentity respPortIdentity;         /**< sourcePortIdentity of the last PDelayResp packet received */
  XAvb_PortIdentity respReqPortIdentity;      /**< requestingPortIdentity of the last PDelayResp packet received */
} XAvb_StateMachineData;


/** This struct captures information from RX'd Sync/FollowUp message pairs in a format similar
 * to the MDSyncReceive structure described in the IEEE P802.1AS specification.
 */
typedef struct
{
  /** The logMessageInterval is the value of the logMessageInterval of the time-synchronization
   *  event message received by this port. It is the logSyncInterval of the upstream MasterPort
   *  that sent the event message.*/
  u8 logMessageInterval;

  /** Convert logMessageInterval into a value we can relate to our 1/128 ns clk pulse */
  u16 SyncIntervalDuration;

} XAvb_MDSyncReceive;


/**
 * The XAvb driver instance data. The user is required to allocate a
 * variable of this type for every AVB device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
  /** hardware configuration */
  XAvb_Config Config;

  /** Device is initialized and ready */
  u32 IsReady;

  /** Is AVB enabled */
  u32 AVBIsEnabled;

  /** The PTP algorithm can be started and stopped as requested */
  u32 PtpIsRunning;

  /** The peer must be AS capable before we start full blown PTP */
  u32 PeerASCapable;

  /** Current PTP Slave Error is less than XAVB_CLOCK_LOCK_THRESHOLD nsec */
  u32 PTPLocked;

  /** Store the info from the latest RX'd Sync/Follow message pair */
  XAvb_MDSyncReceive latestMDSyncReceive;

  /** Contains the local port Identity information */
  XAvb_PortIdentity portIdLocal;

  /** Create a data structure for the Best Master Clock Algorithm (BMCA) */
  XAvb_BmcData CurrentBmc;

  /** Create a data structure for the Precise Timing Protocol (PTP) */
  XAvb_PtpStruct PtpRecords;

  /** Create a data structure to record the PTP frames Sequence ID values */
  XAvb_SequenceIdStruct SequenceIdRecords;

  /** Create a data structure to store the Signalling frame information */
  XAvb_SignallingFrameData SignallingFrameData;

  /** Create a data structure to store various PTP counters/timers */
  XAvb_Counters PtpCounters;

  /** Create a data structure to store state machine flags */
  XAvb_StateMachineData StateMachineData;

  /** Callback Handler for a GrandMaster discontinuity event */
  XAvb_Handler GMDiscHandler;
  /** Callback ref for a GrandMaster discontinuity event */
  void *GMDiscCallBackRef;

} XAvb;


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*
 * Functions in xavb.c
 */
XStatus XAvb_CfgInitialize(XAvb *InstancePtr,      \
                           XAvb_Config *ConfigPtr, \
			   u32 EffectiveAddress);

XAvb_Config *XAvb_LookupConfig(u16 DeviceId);

void XAvb_Reset(XAvb * InstancePtr);

void XAvb_Start(XAvb * InstancePtr);

void XAvb_Stop(XAvb * InstancePtr);

void XAvb_PtpTimerInterruptHandler(XAvb * InstancePtr);

void XAvb_PtpRxInterruptHandler(XAvb * InstancePtr);

void XAvb_SetupRxFilterControlPcp(XAvb * InstancePtr, \
                                  u32    VlanPriority, \
                                  u8     SrClass);

void XAvb_SetupRxFilterControlVid(XAvb * InstancePtr, \
                                  u32    VlanVid, \
                                  u8     SrClass);

void XAvb_SetupRxFilterControlMatchMode(XAvb * InstancePtr, \
                                        u32    VlanMatchMode);

/*
 * Functions in xavb_ptp_packets.c
 */
u32  XAvb_ReorderWord(u32 Data);
u32 XAvb_CompareClockIdentity(u32 BaseAddress, \
                              XAvb_ClockIdentity Identity1, \
                              XAvb_ClockIdentity Identity2);

u32 XAvb_ComparePortIdentity(u32 BaseAddress, \
                             XAvb_PortIdentity Identity1, \
                             XAvb_PortIdentity Identity2);

void XAvb_WriteToMultipleTxPtpFrames(u32 BaseAddress, \
                                     u32 Address, \
                                     u32 Data, \
                                     u32 DataBitEnable, \
                                     u8  BufferEnable);

u32  XAvb_IncSequenceId(u32 BaseAddress, \
                        u32 PTPFrameBaseAddress);

void XAvb_GetPortIdentity(u32 BaseAddress, \
                          u32 PtpFrameBaseAddr, \
                          u32 PortIdOffset, \
                          XAvb_PortIdentity *portID);

void XAvb_WaitOnTxPtpQueue(XAvb * InstancePtr);

void XAvb_MasterSendAnnounce(XAvb * InstancePtr);

void XAvb_MasterSendSync(XAvb * InstancePtr);

void XAvb_MasterSendFollowUp(XAvb * InstancePtr);

void XAvb_SendPDelayReq(XAvb * InstancePtr);

void XAvb_SendPDelayResp(XAvb * InstancePtr, \
                         u32    PtpFrameBaseAddr);

void XAvb_SendPDelayRespFollowUp(XAvb * InstancePtr);

u32  XAvb_IsRxFramePTP(XAvb * InstancePtr, \
                       u32    PtpFrameBaseAddr);

void XAvb_DecodeRxSync(XAvb * InstancePtr, \
                       u32    PtpFrameBaseAddr);

void XAvb_DecodeRxFollowUp(XAvb * InstancePtr, \
                           u32    PtpFrameBaseAddr);

void XAvb_DecodeRxPDelayResp(XAvb * InstancePtr, \
                             u32    PtpFrameBaseAddr);

void XAvb_DecodeRxPDelayRespFollowUp(XAvb * InstancePtr, \
                                     u32    PtpFrameBaseAddr);

void XAvb_DecodeRxSignaling(XAvb * InstancePtr, \
                            u32    PtpFrameBaseAddr);

u16 XAvb_UpdateIntervalDuration(u16   currentIntervalDuration, \
                                char logMeanVal);

u16 XAvb_ConvertLogMeanToDuration(char logMeanVal);

char XAvb_ConvertDurationToLogMean(u16 fractionalVal);

void XAvb_UpdateLogMeanMessageInterval(u32 BaseAddress, \
                                       u32 PtpFrameBaseAddr, \
                                       u16 intervalDuration);

void XAvb_SetupSourcePortIdentity(XAvb * InstancePtr, \
                                  XAvb_PortIdentity systemIdentity);

/*
 * Functions in xavb_ptp_bmca.c
 */
void XAvb_DecodeTxAnnounceFrame(XAvb * InstancePtr);

void XAvb_DecodeRxAnnounceFrame(XAvb * InstancePtr, \
                                u32    PtpFrameBaseAddr);

void XAvb_ReadAnnounceFrame(u32 BaseAddress, \
                            u32 PtpFrameBaseAddr, \
                            XAvb_BmcData* AnnounceFrame);

void XAvb_ReadAnnounceReceiptTimeout(u32 BaseAddress, \
                                     u32 PtpFrameBaseAddr, \
                                     XAvb_BmcData * AnnounceFrame);

void XAvb_UpdateBmcRecords(XAvb_BmcData* NewMaster, \
                           XAvb_BmcData* CurrentBmc);

u32  XAvb_BestMasterClockAlgorithm(XAvb_BmcData* AnnounceFrame, \
                                   XAvb_BmcData* CurrentBmc);

void XAvb_BecomeRtcMaster(XAvb * InstancePtr, u8 txAnnounceHasWon);

void XAvb_BecomeRtcSlave(XAvb * InstancePtr);

void XAvb_ChangePTPLockStatus(XAvb * InstancePtr, u8 locked);

void XAvb_ChangePeerASCapability(XAvb *InstancePtr, u8 capable);

void XAvb_SetGMDiscontinuityHandler(XAvb *InstancePtr, \
                                    XAvb_Handler FuncPtr, \
                                    void *CallBackRef);

/*
 * Functions in xavb_rtc_sync.c
 */
u32  XAvb_CaptureNanoSec(u32 BaseAddress, \
                         u32 PtpFrameBaseAddr);

void XAvb_CalcDelay(XAvb * InstancePtr);

void XAvb_CalcRtcOffset(XAvb * InstancePtr, \
                        u32    PtpFrameBaseAddr);

void XAvb_UpdateRtcIncrement(XAvb * InstancePtr);

void XAvb_Adjust8kClock(u32 BaseAddress, \
                        u32 NewOffset);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
