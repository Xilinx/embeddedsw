/******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbpsu_endpoint.h
* @addtogroup usbpsu_v1_9
* @{
 *
 * This is an internal file containing the definitions for endpoints. It is
 * included by the xusbps_endpoint.c which is implementing the endpoint
 * functions and by xusbps_intr.c.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- --------------------------------------------------------
 * 1.0   sg  06/06/16  First release
 * 1.4 	 bk  12/01/18  Modify USBPSU driver code to fit USB common example code
 *		       for all USB IPs.
 * 1.6   pm  22/07/18  Removed coverity warning- INCLUDE_RECURSION
 * 1.8	 pm  24/07/20  Fixed MISRA-C and Coverity warnings
 * 1.9	 pm  15/03/21  Fixed doxygen warnings
 * </pre>
 *
 ******************************************************************************/
#ifndef XUSBPSU_ENDPOINT_H	/* Prevent circular inclusions */
#define XUSBPSU_ENDPOINT_H	/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xil_types.h"

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/* Device Generic Command Register */
#define XUSBPSU_DGCMD_SET_LMP                   0x00000001U /**< Set LMP */
#define XUSBPSU_DGCMD_SET_PERIODIC_PAR          0x00000002U /**< Set Periodic Parameters */
#define XUSBPSU_DGCMD_XMIT_FUNCTION             0x00000003U /**< Transmit Function */

/* These apply for core versions 1.94a and later */
#define XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_LO    0x00000004U /**< Set Scratchpad Buffer Array Address Lo */
#define XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_HI    0x00000005U /**< Set Scratchpad Buffer Array Address Hi */

#define XUSBPSU_DGCMD_SELECTED_FIFO_FLUSH       0x00000009U /**< Selected FIFO Flush */
#define XUSBPSU_DGCMD_ALL_FIFO_FLUSH            0x0000000aU /**< All FIFO Flush */
#define XUSBPSU_DGCMD_SET_ENDPOINT_NRDY         0x0000000cU /**< Set Endpoint NRDY */
#define XUSBPSU_DGCMD_RUN_SOC_BUS_LOOPBACK      0x00000010U /**< Run SoC Bus LoopBack Test */

#define XUSBPSU_DGCMD_STATUS(n)                 (((u32)(n) >> 15U) & 1U) /**< Command Status */
#define XUSBPSU_DGCMD_CMDACT                    0x00000400U /**< Command Active - bit 10 */
#define XUSBPSU_DGCMD_CMDIOC                    0x00000100U /**< Command Interrupt on Complete - bit 8 */

/* Device Generic Command Parameter Register */
#define XUSBPSU_DGCMDPAR_FORCE_LINKPM_ACCEPT    (0x00000001U << 0U) /**< Link PM acceept */
#define XUSBPSU_DGCMDPAR_FIFO_NUM(n)            ((u32)(n) << 0U) /**< FIFO NUM */
#define XUSBPSU_DGCMDPAR_RX_FIFO                (0x00000000U << 5U) /**< RX FIFO */
#define XUSBPSU_DGCMDPAR_TX_FIFO                (0x00000001U << 5U) /**< TX FIFO */
#define XUSBPSU_DGCMDPAR_LOOPBACK_DIS           (0x00000000U << 0U) /**< Loopback disable */
#define XUSBPSU_DGCMDPAR_LOOPBACK_ENA           (0x00000001U << 0U) /**< Loopback enable */

/* Device Endpoint Command Register */
#define XUSBPSU_DEPCMD_PARAM_SHIFT              16U /**< Command parameter shift by 16 bit */
#define XUSBPSU_DEPCMD_PARAM(x)         ((u32)(x) << XUSBPSU_DEPCMD_PARAM_SHIFT)
							/**< Command Parameters
							 *   Or Event Parameters
							 */
#define XUSBPSU_DEPCMD_GET_RSC_IDX(x)  (((u32)(x) >> XUSBPSU_DEPCMD_PARAM_SHIFT) & \
                                        (u32)0x0000007fU) /**< Transfer Resource Index */
#define XUSBPSU_DEPCMD_STATUS(x)                (((u32)(x) >> 12U) & (u32)0xFU) /**< Command Status */
#define XUSBPSU_DEPCMD_HIPRI_FORCERM            0x00000800U /**< HighPriority/ForceRM bit 11 */
#define XUSBPSU_DEPCMD_CMDACT                   0x00000400U /**< Command Active bit 10U */
#define XUSBPSU_DEPCMD_CMDIOC                   0x00000100U /**< Command Interrupt on Complete bit 8U */

#define XUSBPSU_DEPCMD_DEPSTARTCFG              0x00000009U /**< Start New Configuration */
#define XUSBPSU_DEPCMD_ENDTRANSFER              0x00000008U /**< End Transfer */
#define XUSBPSU_DEPCMD_UPDATETRANSFER           0x00000007U /**< Update Transfer */
#define XUSBPSU_DEPCMD_STARTTRANSFER            0x00000006U /**< Start Transfer */
#define XUSBPSU_DEPCMD_CLEARSTALL               0x00000005U /**< Clear Stall */
#define XUSBPSU_DEPCMD_SETSTALL                 0x00000004U /**< Set Stall */
#define XUSBPSU_DEPCMD_GETEPSTATE               0x00000003U /**< Get Endpoint State */
#define XUSBPSU_DEPCMD_SETTRANSFRESOURCE        0x00000002U /**< Set Endpoint Transfer Resource Configuration */
#define XUSBPSU_DEPCMD_SETEPCONFIG              0x00000001U /**< Set Endpoint Configuration */

/* The EP number goes 0..31 so ep0 is always out and ep1 is always in */
#define XUSBPSU_DALEPENA_EP(n)                  ((u32)0x00000001U << (n))
						/**< Device Active USB Endpoint
						 *   Enable Register
						 */

#define XUSBPSU_DEPCFG_INT_NUM(n)               ((u32)(n) << 0U) /**< Interrupt number */
#define XUSBPSU_DEPCFG_XFER_COMPLETE_EN         0x00000100U /**< XferComplete Enable bit 8 */
#define XUSBPSU_DEPCFG_XFER_IN_PROGRESS_EN      0x00000200U /**< XferInProgress Enable bit 9 */
#define XUSBPSU_DEPCFG_XFER_NOT_READY_EN        0x00000400U /**< XferNotReady Enable bit 10 */
#define XUSBPSU_DEPCFG_FIFO_ERROR_EN            0x00000800U /**< FIFO error Enable bit 11 */
#define XUSBPSU_DEPCFG_STREAM_EVENT_EN          0x00002000U /**< stream event Enable  bit 13 */
#define XUSBPSU_DEPCFG_BINTERVAL_M1(n)          ((u32)(n) << 16U) /**< BInterval */
#define XUSBPSU_DEPCFG_STREAM_CAPABLE           0x01000000U /**< Indicates this endpoint is stream-capable bit 24 */
#define XUSBPSU_DEPCFG_EP_NUMBER(n)             ((u32)(n) << 25U) /**< USB Endpoint Number */
#define XUSBPSU_DEPCFG_BULK_BASED               0x40000000U /**< Bulk based bit 30 */
#define XUSBPSU_DEPCFG_FIFO_BASED               0x80000000U /**< FIFO based bit 31 */

/* DEPCFG parameter 0 */
#define XUSBPSU_DEPCFG_EP_TYPE(n)               ((u32)(n) << 1U) /**< Endpoint Type */
#define XUSBPSU_DEPCFG_MAX_PACKET_SIZE(n)       ((u32)(n) << 3U) /**< Maximum Packet Size */
#define XUSBPSU_DEPCFG_FIFO_NUMBER(n)           ((u32)(n) << 17U) /**< FIFO Number */
#define XUSBPSU_DEPCFG_BURST_SIZE(n)            ((u32)(n) << 22U) /**< Burst Size 0: Burst length = 1, and so on, up to 16. */

#define XUSBPSU_DEPCFG_DATA_SEQ_NUM(n)          ((u32)(n) << 26U) /**< Data sequence number */
/* This applies for core versions earlier than 1.94a */
#define XUSBPSU_DEPCFG_IGN_SEQ_NUM              (0x00000001U << 31U) /**< IGN sequence number*/
/* These apply for core versions 1.94a and later */
#define XUSBPSU_DEPCFG_ACTION_INIT              0x00000000U /**< Initialize endpoint state */
#define XUSBPSU_DEPCFG_ACTION_RESTORE           0x40000000U /**< Restore endpoint state bit 30 */
#define XUSBPSU_DEPCFG_ACTION_MODIFY            0x80000000U /**< Modify endpoint state bit 30 */

/* DEPXFERCFG parameter 0 */
#define XUSBPSU_DEPXFERCFG_NUM_XFER_RES(n) ((u32)(n) & (u32)0xFFFFU) /**< parameter0 transfer number */

#define XUSBPSU_DEPCMD_TYPE_BULK                2U /**< Bulk Type */
#define XUSBPSU_DEPCMD_TYPE_INTR                3U /**< Interrupt Type */

/* TRB Length, PCM and Status */
#define XUSBPSU_TRB_SIZE_MASK           (0x00ffffffU) /**< TRB size mask */
#define XUSBPSU_TRB_SIZE_LENGTH(n)      ((u32)(n) & XUSBPSU_TRB_SIZE_MASK) /**< TRB length */
#define XUSBPSU_TRB_SIZE_PCM1(n)        (((u32)(n) & (u32)0x03) << 24U) /**< TRB PCM1 */
#define XUSBPSU_TRB_SIZE_TRBSTS(n)      (((u32)(n) & ((u32)0x0f << 28U)) >> 28U) /**< TRB status */

#define XUSBPSU_TRBSTS_OK               0U /**< TRB Status OK */
#define XUSBPSU_TRBSTS_MISSED_ISOC      1U /**< TRB missed ISOC */
#define XUSBPSU_TRBSTS_SETUP_PENDING    2U /**< TRB Setup Pending */
#define XUSBPSU_TRB_STS_XFER_IN_PROG    4U /**< TRB Transfer In-Progress */

/* TRB Control */
#define XUSBPSU_TRB_CTRL_HWO            ((u32)0x00000001U << 0U) /**< Indicates that hardware owns the TRB */
#define XUSBPSU_TRB_CTRL_LST            ((u32)0x00000001U << 1U) /**< XferComplete event */
#define XUSBPSU_TRB_CTRL_CHN            ((u32)0x00000001U << 2U) /**< Chain Buffers */
#define XUSBPSU_TRB_CTRL_CSP            ((u32)0x00000001U << 3U) /**< Continue on Short Packet */
#define XUSBPSU_TRB_CTRL_TRBCTL(n)      (((u32)(n) & (u32)0x3FU) << 4U) /**< TRB Control Stage */
#define XUSBPSU_TRB_CTRL_ISP_IMI        0x00000400U /**< Interrupt on Missed ISOC - bit 10 */
#define XUSBPSU_TRB_CTRL_IOC            0x00000800U /**< Interrupt on Complete - bit 11 */
#define XUSBPSU_TRB_CTRL_SID_SOFN(n)    (((u32)(n) & (u32)0xFFFFU) << 14U) /**< Stream ID / SOF Number */

#define XUSBPSU_TRBCTL_NORMAL                   XUSBPSU_TRB_CTRL_TRBCTL(1U) /**< Normal (Control-Data-2+ / Bulk / Interrupt) */
#define XUSBPSU_TRBCTL_CONTROL_SETUP            XUSBPSU_TRB_CTRL_TRBCTL(2U) /**< Control-Setup */
#define XUSBPSU_TRBCTL_CONTROL_STATUS2          XUSBPSU_TRB_CTRL_TRBCTL(3U) /**< Control-Status-2 */
#define XUSBPSU_TRBCTL_CONTROL_STATUS3          XUSBPSU_TRB_CTRL_TRBCTL(4U) /**< Control-Status-3 */
#define XUSBPSU_TRBCTL_CONTROL_DATA             XUSBPSU_TRB_CTRL_TRBCTL(5U) /**< Control-Data */
#define XUSBPSU_TRBCTL_ISOCHRONOUS_FIRST        XUSBPSU_TRB_CTRL_TRBCTL(6U) /**< Isochronous-First */
#define XUSBPSU_TRBCTL_ISOCHRONOUS              XUSBPSU_TRB_CTRL_TRBCTL(7U) /**< Isochronous */
#define XUSBPSU_TRBCTL_LINK_TRB                 XUSBPSU_TRB_CTRL_TRBCTL(8U) /**< Normal-ZLP */

#ifdef __cplusplus
}
#endif

#endif /* XUSBPSU_ENDPOINT_H */
/** @} */
