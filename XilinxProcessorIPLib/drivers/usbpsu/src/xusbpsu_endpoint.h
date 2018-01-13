/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbpsu_endpoint.h
* @addtogroup usbpsu_v1_7
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
 * </pre>
 *
 ******************************************************************************/
#ifndef XUSBPSU_ENDPOINT_H
#define XUSBPSU_ENDPOINT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xil_types.h"

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/* Device Generic Command Register */
#define XUSBPSU_DGCMD_SET_LMP                   0x00000001U
#define XUSBPSU_DGCMD_SET_PERIODIC_PAR          0x00000002U
#define XUSBPSU_DGCMD_XMIT_FUNCTION             0x00000003U

/* These apply for core versions 1.94a and later */
#define XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_LO    0x00000004U
#define XUSBPSU_DGCMD_SET_SCRATCHPAD_ADDR_HI    0x00000005U

#define XUSBPSU_DGCMD_SELECTED_FIFO_FLUSH       0x00000009U
#define XUSBPSU_DGCMD_ALL_FIFO_FLUSH            0x0000000aU
#define XUSBPSU_DGCMD_SET_ENDPOINT_NRDY         0x0000000cU
#define XUSBPSU_DGCMD_RUN_SOC_BUS_LOOPBACK      0x00000010U

#define XUSBPSU_DGCMD_STATUS(n)                 (((u32)(n) >> 15U) & 1U)
#define XUSBPSU_DGCMD_CMDACT                    (0x00000001U << 10U)
#define XUSBPSU_DGCMD_CMDIOC                    (0x00000001U << 8U)

/* Device Generic Command Parameter Register */
#define XUSBPSU_DGCMDPAR_FORCE_LINKPM_ACCEPT    (0x00000001U << 0U)
#define XUSBPSU_DGCMDPAR_FIFO_NUM(n)            ((u32)(n) << 0U)
#define XUSBPSU_DGCMDPAR_RX_FIFO                (0x00000000U << 5U)
#define XUSBPSU_DGCMDPAR_TX_FIFO                (0x00000001U << 5U)
#define XUSBPSU_DGCMDPAR_LOOPBACK_DIS           (0x00000000U << 0U)
#define XUSBPSU_DGCMDPAR_LOOPBACK_ENA           (0x00000001U << 0U)

/* Device Endpoint Command Register */
#define XUSBPSU_DEPCMD_PARAM_SHIFT              16U
#define XUSBPSU_DEPCMD_PARAM(x)         ((u32)(x) << XUSBPSU_DEPCMD_PARAM_SHIFT)
#define XUSBPSU_DEPCMD_GET_RSC_IDX(x)  (((u32)(x) >> XUSBPSU_DEPCMD_PARAM_SHIFT) & \
                                        (u32)0x0000007fU)
#define XUSBPSU_DEPCMD_STATUS(x)                (((u32)(x) >> 12U) & (u32)0xFU)
#define XUSBPSU_DEPCMD_HIPRI_FORCERM            (0x00000001U << 11U)
#define XUSBPSU_DEPCMD_CMDACT                   (0x00000001U << 10U)
#define XUSBPSU_DEPCMD_CMDIOC                   (0x00000001U << 8U)

#define XUSBPSU_DEPCMD_DEPSTARTCFG              0x00000009U
#define XUSBPSU_DEPCMD_ENDTRANSFER              0x00000008U
#define XUSBPSU_DEPCMD_UPDATETRANSFER           0x00000007U
#define XUSBPSU_DEPCMD_STARTTRANSFER            0x00000006U
#define XUSBPSU_DEPCMD_CLEARSTALL               0x00000005U
#define XUSBPSU_DEPCMD_SETSTALL                 0x00000004U
#define XUSBPSU_DEPCMD_GETEPSTATE               0x00000003U
#define XUSBPSU_DEPCMD_SETTRANSFRESOURCE        0x00000002U
#define XUSBPSU_DEPCMD_SETEPCONFIG              0x00000001U

/* The EP number goes 0..31 so ep0 is always out and ep1 is always in */
#define XUSBPSU_DALEPENA_EP(n)                  (0x00000001U << (n))

#define XUSBPSU_DEPCFG_INT_NUM(n)               ((u32)(n) << 0U)
#define XUSBPSU_DEPCFG_XFER_COMPLETE_EN         (0x00000001U << 8U)
#define XUSBPSU_DEPCFG_XFER_IN_PROGRESS_EN      (0x00000001U << 9U)
#define XUSBPSU_DEPCFG_XFER_NOT_READY_EN        (0x00000001U << 10U)
#define XUSBPSU_DEPCFG_FIFO_ERROR_EN            (0x00000001U << 11U)
#define XUSBPSU_DEPCFG_STREAM_EVENT_EN          (0x00000001U << 13U)
#define XUSBPSU_DEPCFG_BINTERVAL_M1(n)          ((u32)(n) << 16U)
#define XUSBPSU_DEPCFG_STREAM_CAPABLE           (0x00000001U << 24U)
#define XUSBPSU_DEPCFG_EP_NUMBER(n)             ((u32)(n) << 25U)
#define XUSBPSU_DEPCFG_BULK_BASED               (0x00000001U << 30U)
#define XUSBPSU_DEPCFG_FIFO_BASED               (0x00000001U << 31U)

/* DEPCFG parameter 0 */
#define XUSBPSU_DEPCFG_EP_TYPE(n)               ((u32)(n) << 1U)
#define XUSBPSU_DEPCFG_MAX_PACKET_SIZE(n)       ((u32)(n) << 3U)
#define XUSBPSU_DEPCFG_FIFO_NUMBER(n)           ((u32)(n) << 17U)
#define XUSBPSU_DEPCFG_BURST_SIZE(n)            ((u32)(n) << 22U)
#define XUSBPSU_DEPCFG_DATA_SEQ_NUM(n)          ((u32)(n) << 26U)
/* This applies for core versions earlier than 1.94a */
#define XUSBPSU_DEPCFG_IGN_SEQ_NUM              (0x00000001U << 31U)
/* These apply for core versions 1.94a and later */
#define XUSBPSU_DEPCFG_ACTION_INIT              (0x00000000U << 30U)
#define XUSBPSU_DEPCFG_ACTION_RESTORE           (0x00000001U << 30U)
#define XUSBPSU_DEPCFG_ACTION_MODIFY            (0x00000002U << 30U)

/* DEPXFERCFG parameter 0 */
#define XUSBPSU_DEPXFERCFG_NUM_XFER_RES(n) ((u32)(n) & (u32)0xFFFFU)

#define XUSBPSU_DEPCMD_TYPE_BULK                2U
#define XUSBPSU_DEPCMD_TYPE_INTR                3U

/* TRB Length, PCM and Status */
#define XUSBPSU_TRB_SIZE_MASK           (0x00ffffffU)
#define XUSBPSU_TRB_SIZE_LENGTH(n)      ((u32)(n) & XUSBPSU_TRB_SIZE_MASK)
#define XUSBPSU_TRB_SIZE_PCM1(n)        (((u32)(n) & (u32)0x03) << 24U)
#define XUSBPSU_TRB_SIZE_TRBSTS(n)      (((u32)(n) & ((u32)0x0f << 28U)) >> 28U)

#define XUSBPSU_TRBSTS_OK               0U
#define XUSBPSU_TRBSTS_MISSED_ISOC      1U
#define XUSBPSU_TRBSTS_SETUP_PENDING    2U
#define XUSBPSU_TRB_STS_XFER_IN_PROG    4U

/* TRB Control */
#define XUSBPSU_TRB_CTRL_HWO            ((u32)0x00000001U << 0U)
#define XUSBPSU_TRB_CTRL_LST            ((u32)0x00000001U << 1U)
#define XUSBPSU_TRB_CTRL_CHN            ((u32)0x00000001U << 2U)
#define XUSBPSU_TRB_CTRL_CSP            ((u32)0x00000001U << 3U)
#define XUSBPSU_TRB_CTRL_TRBCTL(n)      (((u32)(n) & (u32)0x3FU) << 4U)
#define XUSBPSU_TRB_CTRL_ISP_IMI        (0x00000001U << 10U)
#define XUSBPSU_TRB_CTRL_IOC            (0x00000001U << 11U)
#define XUSBPSU_TRB_CTRL_SID_SOFN(n)    (((u32)(n) & (u32)0xFFFFU) << 14U)

#define XUSBPSU_TRBCTL_NORMAL                   XUSBPSU_TRB_CTRL_TRBCTL(1U)
#define XUSBPSU_TRBCTL_CONTROL_SETUP            XUSBPSU_TRB_CTRL_TRBCTL(2U)
#define XUSBPSU_TRBCTL_CONTROL_STATUS2          XUSBPSU_TRB_CTRL_TRBCTL(3U)
#define XUSBPSU_TRBCTL_CONTROL_STATUS3          XUSBPSU_TRB_CTRL_TRBCTL(4U)
#define XUSBPSU_TRBCTL_CONTROL_DATA             XUSBPSU_TRB_CTRL_TRBCTL(5U)
#define XUSBPSU_TRBCTL_ISOCHRONOUS_FIRST        XUSBPSU_TRB_CTRL_TRBCTL(6U)
#define XUSBPSU_TRBCTL_ISOCHRONOUS              XUSBPSU_TRB_CTRL_TRBCTL(7U)
#define XUSBPSU_TRBCTL_LINK_TRB                 XUSBPSU_TRB_CTRL_TRBCTL(8U)

#ifdef __cplusplus
}
#endif

#endif /* XUSBPSU_ENDPOINT_H */
/** @} */
