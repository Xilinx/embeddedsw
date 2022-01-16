/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsem_client_api.h
*
* This file has definitions of commonly used macros and data types needed for
* XilSEM client interface.
* @cond xsem_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   gm   01/04/2021   Initial creation : Cram IPI commands
* 0.2   hb   01/06/2021   Added Npi IPI commands
* 0.3   rb   01/18/2021   Added Npi PMC RAM status read API
*       rb   01/25/2021   Updated IPI API command Index
* 0.4   rb   01/25/2021   Support of event registration API
* 0.5   rb   03/04/2021   Updated PMC RAM registers
* 0.6   gm   03/10/2021   Updated cram error event notification
* 0.7   hv   03/11/2021   Doxygen changes
* 0.8   hb   03/15/2021   MISRA fixes, added Event Notifiers macros and
*                         formatted code
* 0.9   rb   04/07/2021   Doxygen changes
* 1.0	hv   05/04/2021   Updated Doxygen comments
* 1.1	hv   08/18/2021   Fix Doxygen warnings
* 1.2	hv   10/08/2021   Added user interface to Get SEM configuration
* 1.3	hv   01/06/2022   Replaced library specific utility functions and
* 			  standard lib functions with Xilinx maintained
* 			  functions
* 1.4   hb   01/07/2022   Added defines and struct for NPI error events
*                         and get golden Sha command
* 1.5	hv   01/11/2022   Added interface for reading Frame ECC
* 1.6   rama 01/14/2022   Added user interface to get golden CRC & total frames
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSEM_CLIENT_API_H
#define XSEM_CLIENT_API_H
/***************************** Include Files *********************************/
#include "xsem_ipi_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CRAM Commands Acknowledgment IDs */
/** CRAM Initialization Acknowledgment ID */
#define CMD_ACK_CFR_INIT		(0x00010301U)
/** CRAM Start Scan Acknowledgment ID */
#define CMD_ACK_CFR_START_SCAN		(0x00010302U)
/** CRAM Stop Scan Acknowledgment ID */
#define CMD_ACK_CFR_STOP_SCAN		(0x00010303U)
/** CRAM Error Injection Acknowledgment ID */
#define CMD_ACK_CFR_NJCT_ERR		(0x00010304U)
/** SEM Read Frame ECC Acknowledgment ID */
#define CMD_ACK_SEM_READ_FRAME_ECC	(0x0003030AU)

/* NPI Commands Acknowledgment ID */
/** NPI Start Scan Acknowledgment ID */
#define CMD_ACK_NPI_STARTSCAN		(0x00010305U)
/** NPI Stop Scan Acknowledgment ID */
#define CMD_ACK_NPI_STOPSCAN		(0x00010306U)
/** NPI Error Injection Acknowledgment ID */
#define CMD_ACK_NPI_ERRINJECT		(0x00010307U)
/** NPI Get NPI Golden SHA Acknowledgment ID */
#define CMD_ACK_NPI_GET_GLDN_SHA	(0x00010310U)
/** SEM Get configuration Acknowledgment ID */
#define CMD_ACK_SEM_GET_CONFIG		(0x00030309U)


/** Maximum CRAM error register count */
#define MAX_CRAMERR_REGISTER_CNT	(7U)
/** Maximum NPI slave skip count */
#define MAX_NPI_SLV_SKIP_CNT		(8U)
/** Maximum NPI Error info count */
#define MAX_NPI_ERR_INFO_CNT		(2U)

/** PMC_RAM - Base Address */
#define PMC_RAM_BASEADDR		(0XF2014000U)

/** SEM NPI Status */
#define PMC_RAM_SEM_NPI_STATUS		(PMC_RAM_BASEADDR + 0x00000050U)

/** SEM NPI Slave Skip Count */
#define PMC_RAM_SEM_NPI_SLVSKIP_CNT0	(PMC_RAM_BASEADDR + 0x00000054U)

/** SEM NPI Scan Count */
#define PMC_RAM_SEM_NPI_SCAN_CNT	(PMC_RAM_BASEADDR + 0x00000074U)

/** SEM NPI Heartbeat Count */
#define PMC_RAM_SEM_NPI_HEARTBEAT_CNT	(PMC_RAM_BASEADDR + 0x00000078U)

/** SEM NPI Error Info 0 */
#define PMC_RAM_SEM_NPIERR_INFO0	(PMC_RAM_BASEADDR + 0x0000007CU)

/** SEM NPI Error Info 1 */
#define PMC_RAM_SEM_NPIERR_INFO1	(PMC_RAM_BASEADDR + 0x00000080U)

/** PMC_RAM_SEM_CRAM_STATUS */
#define PMC_RAM_SEM_CRAM_STATUS		(PMC_RAM_BASEADDR + 0X00000084U)

/** PMC_RAM_SEM_CRAMERR_ADDRL0 */
#define PMC_RAM_SEM_CRAMERR_ADDRL0	(PMC_RAM_BASEADDR + 0X00000088U)

/** PMC_RAM_SEM_CRAMERR_ADDRH0 */
#define PMC_RAM_SEM_CRAMERR_ADDRH0	(PMC_RAM_BASEADDR + 0X0000008CU)

/** PMC_RAM_SEM_CRAM_COR_BITCNT */
#define PMC_RAM_SEM_CRAM_COR_BITCNT	(PMC_RAM_BASEADDR + 0X000000C0U)

/* CRAM Commands ID */
/** Command ID for CRAM initialization */
#define CMD_ID_CFR_INIT			(0x01U)
/** Command ID for CRAM Start scan */
#define CMD_ID_CFR_START_SCAN		(0x02U)
/** Command ID for CRAM Stop scan */
#define CMD_ID_CFR_STOP_SCAN		(0x03U)
/** Command ID for CRAM error injection */
#define CMD_ID_CFR_NJCT_ERR		(0x04U)
/** Command ID for CRAM Read Frame ECC */
#define CMD_ID_CFR_RDFRAME_ECC		(0x0BU)

/* NPI Commands ID */
/** Command ID for NPI Start scan */
#define CMD_NPI_STARTSCAN		(0x05U)
/** Command ID for NPI Stop scan */
#define CMD_NPI_STOPSCAN		(0x06U)
/** Command ID for NPI error injection */
#define CMD_NPI_ERRINJECT		(0x07U)
/** Command ID for NPI Get Golden SHA */
#define CMD_NPI_GET_GLDN_SHA	(0x0AU)

/** Event Notification Register Command ID */
#define CMD_EM_EVENT_REGISTER		(0x08U)

/** Command ID for SEM Get Configuration */
#define CMD_ID_SEM_GET_CONFIG		(0x09U)

/** Total number of possible descriptors in NPI scan */
#define NPI_MAX_DESCRIPTORS	(50U)

/** Base address for CFRAME Registers  */
#define CFRAME_BASE_ADDRESS				(0xF12D0000U)

/** Offset address to select CFRAME Row instance */
#define CFRAME_ROW_OFFSET				(0x2000U)

/** Offset address for Golden CRC */
#define CFRAME_SEU_CRC_ADDR				(0x1F0U)

/** Offset address for reading Total frames in type 0, 1, 2, 3 */
#define CFRAME_LAST_BOT_ADDR			(0x220U)

/** Maximum number of frame types  */
#define CFRAME_MAX_TYPE					(0x07U)

/** MASK for 0 to 19 bits for getting Type_0, Type_4 frames */
#define CFRAME_BIT_0_19_MASK			(0x000FFFFFU)

/** MASK for 20 to 31 bits for getting Type_1, Type_5 frames */
#define CFRAME_BIT_20_39_MASK_LOW		(0xFFF00000U)

/** MASK for 32 to 39 bits for getting Type_1, Type_5 frames */
#define CFRAME_BIT_20_39_MASK_HIGH		(0x000000FFU)

/** Shift for 20 to 31 bits for getting Type_1, Type_5 frames */
#define CFRAME_BIT_20_39_SHIFT_R		(20U)

/** Shift for 32 to 39 bits for getting Type_1, Type_5 frames */
#define CFRAME_BIT_20_39_SHIFT_L		(12U)

/** MASK for 40 to 59 bits for getting Type_2, Type_6 frames */
#define CFRAME_BIT_40_59_MASK			(0x0FFFFF00U)

/** Shift for 40 to 59 bits for getting Type_2, Type_6 frames */
#define CFRAME_BIT_40_59_SHIFT_R		(8U)

/** MASK for 60 to 63 bits for getting Type_3 */
#define CFRAME_BIT_60_79_MASK_LOW		(0xF0000000U)

/** MASK for 64 to 79 bits for getting Type_3 */
#define CFRAME_BIT_60_79_MASK_HIGH		(0x0000FFFFU)

/** Shift for 60 to 63 bits for getting Type_3 */
#define CFRAME_BIT_60_79_SHIFT_R		(28U)

/** Shift for 64 to 79 bits for getting Type_3 */
#define CFRAME_BIT_60_79_SHIFT_L		(4U)

/**
 * XSemIpiResp - IPI Response Data structure
 */
typedef struct {
	u32 RespMsg1; /**< Response word 1 (Ack Header)*/
	u32 RespMsg2; /**< Response word 2 */
	u32 RespMsg3; /**< Response word 3 */
	u32 RespMsg4; /**< Response word 4 */
	u32 RespMsg5; /**< Response word 5 */
	u32 RespMsg6; /**< Response word 6 */
	u32 RespMsg7; /**< Response word 7 (Reserved for CRC)*/
} XSemIpiResp;


/**
 * XSemCfrErrInjData - CRAM Error Injection structure to hold the
 * Error Injection Location details for CRAM:
 * - Frame address
 * - Quad Word in a Frame, Range from 0 to 24
 * - Bit Position in a Quad Word, Range from 0 to 127
 * - Row Number, Range can be found from CFU_APB_CFU_ROW_RANGE
 *   register
 */
typedef struct {
	u32 Efar;  /**< Frame Address */
	u32 Qword; /**< Quad Word 0...24 */
	u32 Bit;   /**< Bit Position 0...127 */
	u32 Row;   /**< Row Number */
} XSemCfrErrInjData;

/**
 * XSemCfrStatus - CRAM Status structure to store the data read from
 * PMC RAM registers
 * This structure provides:
 * - CRAM scan state information
 * - The low address of last 7 corrected error details if correction
 *   is enabled in design
 * - The high address of last 7 corrected error details if correction
 *   is enabled in design
 * - CRAM corrected error bits count value
 */
typedef struct {
	u32 Status; /**< CRAM Status */
	u32 ErrAddrL[MAX_CRAMERR_REGISTER_CNT]; /**< Error Low register L0...L6 */
	u32 ErrAddrH[MAX_CRAMERR_REGISTER_CNT]; /**< Error High register H0...H6 */
	u32 ErrCorCnt; /**< Count of correctable errors */
} XSemCfrStatus;

/**
 * XSemNpiStatus - NPI Status structure to store the data read from
 * PMC RAM registers.
 * This structure provides:
 * - NPI scan status information
 * - NPI descriptor slave skip counter value if arbitration fails
 * - NPI scan counter value
 * - NPI heartbeat counter value
 * - NPI scan error information if SHA mismatch is detected
 */
typedef struct {
	u32 Status; /**< NPI Status */
	u32 SlvSkipCnt[MAX_NPI_SLV_SKIP_CNT]; /**< Slave Skip Count */
	u32 ScanCnt; /**< Scan Count */
	u32 HbCnt; /**< Heart Beat Count */
	u32 ErrInfo[MAX_NPI_ERR_INFO_CNT]; /**< Error Information when SHA
						mismatch occurs */
} XSemNpiStatus;

/* Structure to hold each descriptor information
 * Descriptor Attribute:
 * Bit[31]: STNN - The descriptor contains Split Tiles/NPS/NCRB registers
 * Bit[30]: XMPU - The descriptor contains XMPU registers
 * Bit[29:16]: Reserved
 * Bit[15:8]: Exclusive Lock Type - Bit[8] signifies that descriptor
 * contains GT slaves. Bit[9] signifies that descriptor contains DDRMC slaves
 * bits 10 to 15 are reserved for future use
 * Bit[7:0]: Slave skip count index - Points the corresponding byte in Sem
 * NPI slave skip count registers that need to be updated on arbitration
 * failure
 * Descriptor Golden SHA:
 * This word contains the Golden SHA value that is used to compare with
 * hardware calculated SHA value
 *  */
typedef struct {
	u32 DescriptorAttrib; /**< Descriptor attributes */
	u32 DescriptorGldnSha; /**< Descriptor Golden SHA value */
}XSem_DescriptorInfo;

/* Structure contains descriptor information used during SEM NPI scan*/
typedef struct {
	u32 DescriptorCount; /**< Current Total Descriptor Count */
	XSem_DescriptorInfo DescriptorInfo[NPI_MAX_DESCRIPTORS]; /**< Descriptor information */
}XSem_DescriptorData;

/** SEM CRAM Module Notification ID */
#define XSEM_NOTIFY_CRAM	(0x0U)
/** SEM NPI Module Notification ID */
#define XSEM_NOTIFY_NPI		(0x1U)

/** SEM Generic Event ID */
#define XSEM_EVENT_ERROR	(0x1U)

/* CRAM Errors Event Type */
/** SEM CRAM Uncorrectable ECC Error Event  */
#define XSEM_EVENT_CRAM_UNCOR_ECC_ERR	(0x1U)
/** SEM CRAM Uncorrectable CRC Error Event  */
#define XSEM_EVENT_CRAM_CRC_ERR		(0x2U)
/** SEM CRAM Internal or Fatal Errors */
#define XSEM_EVENT_CRAM_INT_ERR		(0x3U)
/** SEM Correctable ECC Error Event, detected when correction is disabled */
#define XSEM_EVENT_CRAM_COR_ECC_ERR	(0x4U)

/* NPI Errors Event Type */
/** SEM NPI Uncorrectable CRC Error Event  */
#define XSEM_EVENT_NPI_CRC_ERR		(0x1U)
/** NPI Unsupported Descriptor Format */
#define XSEM_EVENT_NPI_DESC_FMT_ERR	(0x2U)
/** NPI Descriptors absent for Scan */
#define XSEM_EVENT_NPI_DESC_ABSNT_ERR	(0x3U)
/** NPI SHA Indicator mismatch error */
#define XSEM_EVENT_NPI_SHA_IND_ERR	(0x4U)
/** NPI SHA engine error */
#define XSEM_EVENT_NPI_SHA_ENGINE_ERR	(0x5U)
/** NPI Periodic Scan Missed Error */
#define XSEM_EVENT_NPI_PSCAN_MISSED_ERR	(0x6U)
/** NPI Cryptographic Accelerator Disabled error */
#define XSEM_EVENT_NPI_CRYPTO_EXPORT_SET_ERR	(0x7U)
/** NPI Safety Write Failure */
#define XSEM_EVENT_NPI_SFTY_WR_ERR	(0x8U)
/** NPI GPIO Error event */
#define XSEM_EVENT_NPI_GPIO_ERR	(0x9U)

/* SEM Event Flags */
/** SEM Event Notification Enable */
#define XSEM_EVENT_ENABLE	(0x1U)
/** SEM Event Notification Disable */
#define XSEM_EVENT_DISABLE	(0x0U)

/**
 * XSem_Notifier : This structure contains details of event notifications
 * 	               to be registered with the XilSEM server
 */
typedef struct {
	/**
	 * The module information to receive notifications.
	 * Module can be either CRAM or NPI.
	 * - For CRAM: use XSEM_NOTIFY_CRAM
	 * - For NPI: use XSEM_NOTIFY_NPI
	 */
	u32 Module;
	/**
	 * The event types specify the specific event registration.
	 * For CRAM module, events are:
	 * - Uncorrectable ECC error: use XSEM_EVENT_CRAM_UNCOR_ECC_ERR
	 * - Uncorrectable CRC error: use XSEM_EVENT_CRAM_CRC_ERR
	 * - Internal error: use XSEM_EVENT_CRAM_INT_ERR
	 * - Correctable ECC error: use XSEM_EVENT_CRAM_COR_ECC_ERR
	 *
	 * For NPI module, events are:
	 * - Uncorrectable CRC error: use XSEM_EVENT_NPI_CRC_ERR
	 * - Unsupported Descriptor Format: use XSEM_EVENT_NPI_DESC_FMT_ERR
	 * - Descriptors absent for Scan: use XSEM_EVENT_NPI_DESC_ABSNT_ERR
	 * - SHA Indicator mismatch: use XSEM_EVENT_NPI_SHA_IND_ERR
	 * - SHA engine error: use XSEM_EVENT_NPI_SHA_ENGINE_ERR
	 * - Periodic Scan Missed: use XSEM_EVENT_NPI_PSCAN_MISSED_ERR
	 * - Cryptographic Accelerator Disabled: use XSEM_EVENT_NPI_CRYPTO_EXPORT_SET_ERR
	 * - Safety Write Failure: useXSEM_EVENT_NPI_SFTY_WR_ERR
	 * - GPIO Error event: use XSEM_EVENT_NPI_GPIO_ERR
	 */
	u32 Event;
	/**
	 * Event flags to enable or disable notification.
	 * - To enable event notification: use XSEM_EVENT_ENABLE
	 * - To disable event notification: use XSEM_EVENT_DISABLE
	 */
	u32 Flag;
} XSem_Notifier;

/* CRAM functions */
XStatus XSem_CmdCfrInit(XIpiPsu *IpiInst, XSemIpiResp *Resp);
XStatus XSem_CmdCfrStartScan(XIpiPsu *IpiInst, XSemIpiResp *Resp);
XStatus XSem_CmdCfrStopScan(XIpiPsu *IpiInst, XSemIpiResp *Resp);
XStatus XSem_CmdCfrNjctErr(XIpiPsu *IpiInst, \
		XSemCfrErrInjData *ErrDetail, XSemIpiResp *Resp);
XStatus XSem_CmdCfrGetStatus(XSemCfrStatus *CfrStatusInfo);
XStatus XSem_CmdCfrReadFrameEcc(XIpiPsu *IpiInst, \
		u32 CframeAddr, u32 RowLoc, XSemIpiResp *Resp);
u32 XSem_CmdCfrGetCrc(u32 RowIndex);
void XSem_CmdCfrGetTotalFrames(u32 RowIndex, u32 *FrameCntPtr);

/* NPI functions */
XStatus XSem_CmdNpiStartScan(XIpiPsu *IpiInst, XSemIpiResp * Resp);
XStatus XSem_CmdNpiStopScan(XIpiPsu *IpiInst, XSemIpiResp * Resp);
XStatus XSem_CmdNpiInjectError(XIpiPsu *IpiInst, XSemIpiResp * Resp);
XStatus XSem_CmdNpiGetGldnSha(XIpiPsu *IpiInst, XSemIpiResp * Resp,
		XSem_DescriptorData * DescData);
XStatus XSem_CmdNpiGetStatus(XSemNpiStatus *NpiStatusInfo);

/* Event Notification Management */
XStatus XSem_RegisterEvent(XIpiPsu *IpiInst, XSem_Notifier* Notifier);

/* Get XilSEM configuration */
XStatus XSem_CmdGetConfig(XIpiPsu *IpiInst, XSemIpiResp *Resp);

#ifdef __cplusplus
}
#endif

#endif /* XSEM_CLIENT_API_H */
