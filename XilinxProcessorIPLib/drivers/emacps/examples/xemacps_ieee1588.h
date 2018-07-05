/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xemacps_ieee1588.h
*
* Defines hash defines, common data types and prototypes to be used with the
* PTP standalone example source code residing in this directory.
*
* The PTP standalone example files implement the basic PTPv2 protocol as an
* example application. However the accuracy of clock offset adjustment is
* not guaranteed as of now. Also the clock rate adjustment and signalling
* frames are not implemented. Also it may not be implementing all aspects of
* PTPv2 strictly as per specs. Since it is based on AVB driver (which is
* 802.1as based), some aspects of 802.1as which are not there in IEEE1588
* may be there in the protocol implementation inadvertently. The sync frame
* interval, announce frame interval and PDelayReq frame intervals are
* hard-coded.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a asa  09/16/11 First release based on AVB driver
* 1.01a asa  03/03/12 New hashdefines are added and new function prototypes
* 					  are added.
* 3.3   asa  05/19/16 Fix for CR#951152. Made following changes.
*                     - Removed code specific for PEEP.
*                     - Ensured that each buffer in RxBuf array is cache
*                       line aligned. The Rxbuf array itself is made cache
*                       line aligned.
*                     - The XEMACPS_PACKET_LEN is changed from 1538 to 1598.
*                       Though the packet length can never be 1598 for
*                       non-jumbo ethernet packets, it is changes to ensure
*                       that each Rx buffer becomes cache line aligned.
*                       If a Rx buffer is not cache line aligned, the
*                       A9 based invalidation logic flushes the first cache
*                       line to ensure that other data faling in this cache
*                       line are not lost when the buffer in invalidated
*                       upon reception of a packet. But that will also mean
*                       the data received in this cache line is lost.
*                       By making this change we always ensure that when an
*                       invalidation happens no flushing takes place and no
*                       incoming data is lost.
*                     - Changes made not to disable and enable back the MMU
*                       when we change the attribute of BD space to make it
*                       strongly ordered.
*                     - The number of Rx and Tx bufs are changed from 32 to 16
*                       as for this simple example so many BDs will never
*                       be needed.
*
* </pre>
*
******************************************************************************/

#ifndef XEMACPS_IEEE1588_H
#define XEMACPS_IEEE1588_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

#define DEBUG_XEMACPS_LEVEL1 /*
			      * Error messages which should be printed on
			      * console. It does not include error messages
			      * from interrupt service routines.
			      */
#undef DEBUG_LEVEL_TWO  /*
			      * Other debug messages, e.g. function names being
			      * executed, PTP protocol messages etc.
			      */

/* PTP Packet Message Type Definitions */
#define XEMACPS_PTP_TYPE_SYNC				0x0
#define XEMACPS_PTP_TYPE_FOLLOW_UP			0x8
#define XEMACPS_PTP_TYPE_PDELAYREQ			0x2
#define XEMACPS_PTP_TYPE_PDELAYRESP			0x3
#define XEMACPS_PTP_TYPE_PDELAYRESP_FOLLOW_UP		0xA
#define XEMACPS_PTP_TYPE_ANNOUNCE			0xB
#define XEMACPS_PTP_TYPE_SIGNALING			0xC


#define XEMACPS_PKT_TYPE_DISABLED			0xffff
#define XEMACPS_PKT_MAX_BUF_LEN				128


/*
 * XEMACPS_ANNOUNCE_RECEIPT_TIMEOUT is the number of announce
 * intervals without the receipt of an Announce message from
 * the GM that are allowed before the GM is assumed to be no
 * longer present and BMCA should be run again
 */
#define XEMACPS_ANNOUNCE_RECEIPT_TIMEOUT		5

/*
 * XEMACPS_SYNC_RECEIPT_TIMEOUT is the number of sync intervals
 * without the receipt of an Sync message from the GM that are
 * allowed before the GM is assumed to be no longer present and
 * BMCA should be run again
 */
#define XEMACPS_SYNC_RECEIPT_TIMEOUT			5

/*
 * XEMACPS_NEIGHBOR_PROP_DELAY_THRESH is the maximum allowed delay
 * (in nanosecs) across a full duplex link for which the PTP protocol is
 * allowed to function. Although this parameter is defined in the
 * IEEE spec, no default is defined.
 */
#define XEMACPS_NEIGHBOR_PROP_DELAY_THRESH		5000

/*
 * XEMACPS_ALLOWED_LOST_RESPONSES is the number of Pdelay_Req messages for
 * which a valid response is not received, above which the Peer should no
 * longer be considered PTPv2 capable.
 */
#define XEMACPS_ALLOWED_LOST_RESPONSES			3


/* Standard PTP Frame Field Definitions (from IEEE1588 specification) */
#define XEMACPS_PTP_ETHERTYPE				0x88F7
#define XEMACPS_PTP_VERSION_PTP				2


/* Real Time Clock Definitions.*/
#define XEMACPS_ONE_SECOND				1000000000 /* In ns */

/*
 * Define how often to re-calculate the RTC Increment This value indicates how
 * many good Sync/FollowUp message pairs are received before the
 * re-calculation is performed.
 */
#define XEMACPS_NUM_SYNC_FU_PAIR_CALC_RTC_INCREMENT	2

/* PHY register number and register content mask used for PHY detection.*/
#define PHY_DETECT_REG 					1
#define PHY_DETECT_MASK 				0x1808

/* PHY register 0 and Register 16 masks*/
#define PHY_R0_RESET					0x8000
#define PHY_R0_LOOPBACK					0x4000
#define PHY_R0_AUTONEG_EN				0x1000
#define PHY_R0_AUTONEG_START				0x0200
#define PHY_R0_10					0x0100
#define PHY_R0_100					0x2100
#define PHY_R0_1000					0x0140
#define PHY_R16_FIFO_DEPTH				0xF078

/*
 * Maximum buffer length used to store the PTP pakcets. Max buffer length
 * for ethernet packet can never be so high as 1598. But this is done
 * to ensure that start address of each Rx buffer is cache line aligned.
 */
#define XEMACPS_PACKET_LEN				1598

/* BD alignment used to allocate the BDs */
#define XEMACPS_IEEE1588_BD_ALIGNMENT			4

/* Number of BDs used in the Tx and Rx paths */
#define XEMACPS_IEEE1588_NO_OF_RX_DESCS			16
#define XEMACPS_IEEE1588_NO_OF_TX_DESCS			16

/* Various offsets in the PTP Ethernet packet and masks to extract contents */
#define XEMACPS_MSGTYP_OFFSET				14
#define XEMACPS_MSGTYP_MASK				0x0F
#define XEMACPS_VERSPTP_OFFSET				15
#define XEMACPS_MSGLENGTH_OFFSET			16
#define XEMACPS_FLAGS_OFFSET				20
#define XEMACPS_CORRFIELD_OFFSET			22
#define XEMACPS_PORTIDENTITY_OFFSET			34
#define XEMACPS_SEQID_OFFSET				44
#define XEMACPS_CONTROL_OFFSET				46
#define XEMACPS_LOGMSG_INTERVAL_OFFSET			47
#define XEMACPS_PRECISE_TS_OFFSET			48
#define XEMACPS_CURRUTCOFFSET_OFFSET			58
#define XEMACPS_GMPRI_ONE_OFFSET			61
#define XEMACPS_GM_CLK_QUALITY_OFFSET			62
#define XEMACPS_GMPRI_TWO_OFFSET			66
#define XEMACPS_GM_IDENTITY_OFFSET			67
#define XEMACPS_STEPS_REMOVED_OFFSET			75
#define XEMACPS_TIMESOURCE_OFFSET			77
#define XEMACPS_TLVTYPE_OFFSET				78
#define XEMACPS_LENGTHFIELD_OFFSET			80
#define XEMACPS_PATHSEQ_OFFSET				82
#define XEMACPS_REQPORTID_OFFSET			58

/*
 * The PTP message type, length value, flags value that are
 * populated in different PTP frames
 */
#define XEMACPS_SYNCFRM_MSG_TYPE			0x10
#define XEMACPS_SYNCFRM_LENGTH				0x002C
#define XEMACPS_SYNCFRM_FLAGS_VAL			0x0200
#define XEMACPS_FOLLOWUPFRM_MSG_TYPE			0x18
#define XEMACPS_FOLLOWUPFRM_LENGTH			0x004C
#define XEMACPS_PDELAYREQFRM_LENGTH			54
#define XEMACPS_PDELAYREQFRM_FLAGS_VAL			0x0200
#define XEMACPS_PDELAYREQFRM_MSG_TYPE			0x02
#define XEMACPS_PDELAYRESPFRM_MSG_TYPE			0x03
#define XEMACPS_PDELAYRESPFRM_LENGTH			54
#define XEMACPS_PDELAYRESPFOLLOWUPFRM_MSG_TYPE		0x0A
#define XEMACPS_PDELAYRESPFOLLOWUP_LENGTH		54
#define XEMACPS_ANNOUNCEFRM_MSG_TYPE			0x1B
#define XEMACPS_ANNOUNCEFRM_LENGTH			0x0040
#define XEMACPS_ANNOUNCEFRM_FLAGS_VAL			0x0008

/* The total length of various PTP packets */
#define XEMACPS_ANNOUNCEMSG_TOT_LEN			90
#define XEMACPS_SYNCMSG_TOT_LEN				58
#define XEMACPS_FOLLOWUPMSG_TOT_LEN			90
#define XEMACPS_PDELAYREQMSG_TOT_LEN			68
#define XEMACPS_PDELAYRESPMSG_TOT_LEN			68
#define XEMACPS_PDELAYRESPFOLLOWUP_TOT_LEN		68

/*
 * The bit field information for different PTP packets in the variable
 * PTPSendPacket. This variable controls the sending of PTP packets.
 */
#define SEND_PDELAY_RESP				0x00000001
#define SEND_PDELAY_RESP_FOLLOWUP			0x00000002
#define SEND_PDELAY_REQ					0x00000004
#define SEND_SYNC					0x00000008
#define SEND_FOLLOW_UP					0x00000010

#define NS_PER_SEC 1000000000ULL      /* Nanoseconds per second */
#define FP_MULT    1000ULL

/* Advertisement control register. */
#define ADVERTISE_10HALF	0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL	0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL	0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF	0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF	0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE	0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL	0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM	0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4	0x0200  /* Try for 100mbps 4k packets  */


#define ADVERTISE_100_AND_10	(ADVERTISE_10FULL | ADVERTISE_100FULL | \
				ADVERTISE_10HALF | ADVERTISE_100HALF)
#define ADVERTISE_100		(ADVERTISE_100FULL | ADVERTISE_100HALF)
#define ADVERTISE_10		(ADVERTISE_10FULL | ADVERTISE_10HALF)

#define ADVERTISE_1000		0x0300


#define IEEE_CONTROL_REG_OFFSET			0
#define IEEE_STATUS_REG_OFFSET			1
#define IEEE_AUTONEGO_ADVERTISE_REG		4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET	5
#define IEEE_PARTNER_ABILITIES_2_REG_OFFSET	8
#define IEEE_PARTNER_ABILITIES_3_REG_OFFSET	10
#define IEEE_1000_ADVERTISE_REG_OFFSET		9
#define IEEE_SPECIFIC_STATUS_REG		17
#define IEEE_CTRL_1GBPS_LINKSPEED_MASK		0x2040
#define IEEE_CTRL_LINKSPEED_MASK		0x0040
#define IEEE_CTRL_LINKSPEED_1000M		0x0040
#define IEEE_CTRL_LINKSPEED_100M		0x2000
#define IEEE_CTRL_LINKSPEED_10M			0x0000
#define IEEE_CTRL_RESET_MASK			0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE		0x1000
#define IEEE_STAT_AUTONEGOTIATE_CAPABLE		0x0008
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE	0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART		0x0200
#define IEEE_STAT_1GBPS_EXTENSIONS		0x0100
#define IEEE_AN1_ABILITY_MASK			0x1FE0
#define IEEE_AN3_ABILITY_MASK_1GBPS		0x0C00
#define IEEE_AN1_ABILITY_MASK_100MBPS		0x0380
#define IEEE_AN1_ABILITY_MASK_10MBPS		0x0060

#define PHY_REG0_RESET    0x8000
#define PHY_REG0_LOOPBACK 0x4000
#define PHY_REG0_10       0x0100
#define PHY_REG0_100      0x2100
#define PHY_REG0_1000     0x0140
#define PHY_REG21_10      0x0030
#define PHY_REG21_100     0x2030
#define PHY_REG21_1000    0x0070

/* Frequency setting */
#define SLCR_LOCK_ADDR                  (XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR                (XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR         (XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR         (XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_LOCK_KEY_VALUE             0x767B
#define SLCR_UNLOCK_KEY_VALUE           0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL          (XPS_SYS_CTRL_BASEADDR + 0x214)
#define EMACPS_SLCR_DIV_MASK		0xFC0FC0FF

/**************************** Type Definitions *******************************/
/*
 * This typedef defines the format for a data structure which stores the Port
 * Identity information specified in IEEE1588.
 */
typedef struct
{
	u8 ClockIdentity[8];
	u16 PortNumber;
} XEmacPs_PortIdentity;

/*
 * This typedef defines the format for a data structure which stores the Clock
 * Identity information specified in IEEE1588.
 */
typedef struct
{
	u8 ClockIdentity[8];
} XEmacPs_ClockIdentity;

/*
 * This typedef defines the quality of a clock
 */
typedef struct
{
	u8 clockClass;
	u8 clockAccuracy;
	u16 offsetScaledLogVariance;
} XEmacPs_ClockQuality;

/*
 * This typedef defines the format for a data structure which stores the
 * relevant fields which are captured from Announce Packets.
 */
typedef struct
{
	XEmacPs_PortIdentity SourcePortIdentity;
	XEmacPs_ClockIdentity GrandmasterIdentity;
	u16 StepsRemoved;
	XEmacPs_ClockQuality ClockQuality;
	u8 GrandmasterPriority1;
	u8 GrandmasterPriority2;
	u8 IAmTheRtcMaster;
	u16 TlvLengthField;
	char LogMessageInterval;
	u16 AnnounceIntervalDuration;
} XEmacPs_BmcData;


/*
 * This typedef defines the format for a data structure which stores
 * information relating to the 1588 based PTP timing calculations.
 */
typedef struct
{
	u32 Nanosec;		/* ns value for sync frame tx request   */
	u32 SlaveSyncTimestampSec;
	u32 SlaveSyncTimestampNSec;
	u32 MasterCorrectionField;/* Correction Field from rx'd follow-up */
	u32 PDelayTimestampT1;	/* T1 :PDelayReq Frame transmission     */
	u32 PDelayTimestampT2;	/* T2 :PDelayReq rx'd at link partner   */
	u32 PDelayTimestampT3;	/* T3 :PDelayResp Frame reception       */
	u32 PDelayTimestampT4;	/* T4 :PDelayResp tx'd by link partner  */
	u32 LinkDelay;		/* Last calculated value of Link Delay  */
	u32 NewSlaveTime;	/* RTC ns at slave for last rx'd sync   */
	u32 NewMasterTime;	/* RTC ns at master for last tx'd sync  */
	u32 OldSlaveTime;	/* Stored RTC slave ns for past sync rx */
	u32 OldMasterTime;	/* Stored RTC master ns for past sync tx*/
	u32 PDelayReqRecdTSNs;
	u32 PDelayReqRecdTSSec;
	u32 PDelayRespTxedTSNs;
	u32 PDelayRespTxedTSSec;
} XEmacPs_PtpStruct;


/*
 * This typedef defines the format for a data structure which stores the last
 * used sequence ID for all of the PTP timing frames.
 */
typedef struct
{
	u16 SyncSequenceId;
	u16 FollowUpSequenceId;
	u16 PDelayReqSequenceId;
	u16 PDelayRespSequenceId;
	u16 PDelayFollowUpSequenceId;
	u16 OldSyncSequenceId;
	u16 NewSyncSequenceId;
} XEmacPs_SequenceIdStruct;


/*
 * The Signalling frame defines the delays to be used between Sync Frames, Link
 * Delay measurements and Announce Frame events
 */
typedef struct
{
	u16 SyncIntervalDuration;
	u16 LinkDelayIntervalDuration;
	u16 AnnounceIntervalDuration;
} XEmacPs_SignallingFrameData;


/*
 * This typedef defines the various counters which have to maintained for the
 * PTP operation.
 */
typedef struct
{
	u16 CounterSyncInterval;
	u16 CounterLinkDelayInterval;
	u16 CounterAnnounceInterval;
	u8  CounterSyncEvents;
} XEmacPs_Counters;

/*
 * Keep track of state machine data to make sure we're fully compliant
 * with the spec
 */
typedef struct
{
	u8 LostResponses;	/* Keep track of the state machine errors */
	u8 RcvdPDelayResp;	/*
				 * Received a valid PDelayResp packet since
				 * PDelayReq was sent
				 */
	u8 RcvdPDelayRespFollowUp;/*
				   * Received a valid PDelayFollowUp packet
				   * since PDelayResp was received
				   */
	XEmacPs_PortIdentity RespPortIdentity;/*
					       * SourcePortIdentity of the last
					       * PDelayResp packet received
					       */
	XEmacPs_PortIdentity RespReqPortIdentity;/*
						  * RequestingPortIdentity of
						  * the last PDelayResp packet
						  received */
} XEmacPs_StateMachineData;


/*
 * This struct captures information from RX'd Sync/FollowUp message pairs in a
 * format similar to the MDSyncReceive structure described in the IEEE1588
 * specification.
 */
typedef struct
{
	u8 LogMessageInterval;
	u16 SyncIntervalDuration;

} XEmacPs_MDSyncReceive;

typedef struct
{
	u32 Seconds;
	u32 NanoSeconds;
} XEmacPs_RtcFormat;

/*
 * The XEmacPs_Ieee1588 driver instance data. The user is required to allocate
 * a variable of this type for every PTP device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	/* The Emac Pss instance to be used for accessing the hardware */
	XEmacPs *EmacPsInstance;

	/* The current port protocol state */
	u32 PtpProtocolState;

	u32 PtpNewPktRecd;

	/* The PTP algorithm can be started and stopped as requested */
	u32 PtpIsRunning;

	/* The peer must be AS capable before we start full blown PTP */
	u32 PeerIeee1588v2Capable;

	u32 PTPLocked;

	/* Store the info from the latest RX'd Sync/Follow message pair */
	XEmacPs_MDSyncReceive LatestMDSyncReceive;

	/* Contains the local port Identity information */
	XEmacPs_PortIdentity PortIdLocal;

	/* Create a data structure for the Best Master Clock Algorithm (BMCA)*/
	XEmacPs_BmcData CurrentBmc;

	/* Create a data structure for the Precise Timing Protocol (PTP) */
	XEmacPs_PtpStruct PtpRecords;

	/* Create data structure to record the PTP frames Sequence ID values*/
	XEmacPs_SequenceIdStruct SequenceIdRecords;

	/* Create a data structure to store the Signalling frame information*/
	XEmacPs_SignallingFrameData SignallingFrameData;

	/* Create a data structure to store various PTP counters/timers */
	XEmacPs_Counters PtpCounters;

	/* Create a data structure to store state machine flags */
	XEmacPs_StateMachineData StateMachineData;

	/* Buffers to store the last recd PTP packet */
	u8 LastRecdSyncFrm[XEMACPS_PKT_MAX_BUF_LEN];
	u8 LastRecdFollowUpFrm[XEMACPS_PKT_MAX_BUF_LEN];
	u8 LastRecdPDelayReqFrm[XEMACPS_PKT_MAX_BUF_LEN];
	u8 LastRecdPDelayRespFrm[XEMACPS_PKT_MAX_BUF_LEN];
	u8 LastRecdPDelayRespFollowUpFrm[XEMACPS_PKT_MAX_BUF_LEN];
	u8 LastRecdSignallingFrm[XEMACPS_PKT_MAX_BUF_LEN];
	u8 LastRecdAnnounceFrm[XEMACPS_PKT_MAX_BUF_LEN];

	/* Buffers to store the PTP packet to be Txed */
	u8 SyncFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];
	u8 FollowUpFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];
	u8 PDelayReqFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];
	u8 PDelayRespFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];
	u8 PDelayRespFollowUpFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];
	u8 SignallingFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];
	u8 AnnounceFrmToTx[XEMACPS_PKT_MAX_BUF_LEN];

} XEmacPs_Ieee1588;


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void XEmacPs_PtpTimerInterruptHandler(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_PtpRxInterruptHandler(XEmacPs_Ieee1588 *InstancePtr);
int XEmacPs_PtpTxPacket(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf,
							int PacketLen);
u32 XEmacPs_ComparePortIdentity(
				XEmacPs_PortIdentity Identity1,
				XEmacPs_PortIdentity Identity2);
u32 XEmacPs_CompareClockIdentity(
				XEmacPs_ClockIdentity Identity1,
				XEmacPs_ClockIdentity Identity2);

void XEmacPs_MasterSendAnnounce(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_MasterSendSync(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_MasterSendFollowUp(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_SendPDelayReq(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_SendPDelayResp(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_SendPDelayRespFollowUp(XEmacPs_Ieee1588 *InstancePtr);
u32  XEmacPs_IsRxFramePTP(u8 *PacketBuf);
void XEmacPs_DecodeRxSync(XEmacPs_Ieee1588 *InstancePtr,u8 *PacketBuf);
void XEmacPs_DecodeRxFollowUp(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf);
void XEmacPs_DecodeRxPDelayResp(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf);
void XEmacPs_DecodeRxPDelayRespFollowUp(XEmacPs_Ieee1588 *InstancePtr,
							u8 *PacketBuf);
void XEmacPs_DecodeRxSignaling(XEmacPs_Ieee1588 *InstancePtr, u8 *PacketBuf);
u16 XEmacPs_UpdateIntervalDuration(u16 currentIntervalDuration,
						signed char logMeanVal);

u16 XEmacPs_ConvertLogMeanToDuration(signed char logMeanVal);

signed char XEmacPs_ConvertDurationToLogMean(u16 fractionalVal);
void XEmacPs_UpdateLogMeanMessageInterval(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_SetupSourcePortIdentity(XEmacPs_Ieee1588 *InstancePtr,
                                  XEmacPs_PortIdentity systemIdentity);

void XEmacPs_DecodeTxAnnounceFrame(XEmacPs_Ieee1588 *InstancePtr,
							u8 *PacketBuf);
void XEmacPs_DecodeRxAnnounceFrame(XEmacPs_Ieee1588 *InstancePtr,
								u8 *PacketBuf);
void XEmacPs_ReadAnnounceFrame(u8 *PacketBuf, XEmacPs_BmcData *AnnounceFrame);
void XEmacPs_UpdateBmcRecords(XEmacPs_BmcData *NewMaster,
						XEmacPs_BmcData *CurrentBmc);
u32  XEmacPs_BestMasterClockAlgorithm(XEmacPs_BmcData *AnnounceFrame,
						XEmacPs_BmcData *CurrentBmc);
void XEmacPs_BecomeRtcMaster(XEmacPs_Ieee1588 *InstancePtr,
							u8 txAnnounceHasWon);
void XEmacPs_BecomeRtcSlave(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_ChangePTPLockStatus(XEmacPs_Ieee1588 *InstancePtr, u8 locked);
void XEmacPs_ChangePeerIeee1588v2Capability(XEmacPs_Ieee1588 *InstancePtr,
							u8 capable);
void XEmacPs_CalcDelay(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_CalcRtcOffset(XEmacPs_Ieee1588 *InstancePtr);
void XEmacPs_UpdateRtcIncrement(XEmacPs_Ieee1588 *InstancePtr);
u32 XEmacPs_htonll (unsigned long long int n);
u32 XEmacPs_ntohll (long long int n);
u8 XEmacPs_GetMsgType (u8 *PacketBuf);
void XEmacPs_GetPortIdentity(u8 *PacketBuf, XEmacPs_PortIdentity *portID);
u16 XEmacPs_GetSequenceId(u8 *PacketBuf);
u16 XEmacPs_IncSequenceId(u8 *PacketBuf);
void XEmacPs_SetMdioDivisor(XEmacPs *InstancePtr, XEmacPs_MdcDiv Divisor);
unsigned int XEmacPs_TsuCalcClk(u32 Freq);
extern volatile u8 PDelayRespSent;
extern volatile u32 PTPSendPacket;
extern volatile u8 SyncSent;

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
