/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_hw.h
* @addtogroup usbpsu_api USBPSU APIs
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   sg    06/06/16 First release
* 1.4   myk   12/01/18 Added support of hibernation
* 1.6	pm    08/08/19 Added AXI-Cache bits masking for CCI feature is enable
* 1.7	pm    02/20/20 Added Coherency Mode Register for CCI feature is enable
* 1.8	pm    07/01/20 Add versal hibernation support
*	pm    24/07/20 Fixed MISRA-C and Coverity warnings
* 1.9	pm    15/03/21 Fixed doxygen warnings
* 1.14	pm    21/06/23 Added support for system device-tree flow.
* 1.15  np    26/03/24 Add doxygen and editorial fixes
*
* </pre>
*
*****************************************************************************/

/** @cond INTERNAL */
#ifndef XUSBPSU_HW_H	/* Prevent circular inclusions */
#define XUSBPSU_HW_H	/**< by using protection macros  */

#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions ****************************/

/**@name Register offsets
 *
 * The following constants provide access to each of the registers of the
 * USBPSU device.
 * @{
 */

/* Port Status and Control Register */
#define XUSBPSU_PORTSC_30			0x430U /**< Port Status and Control Register */
#define XUSBPSU_PORTMSC_30			0x434U /**< USB3 Port Power Management Status and Control Register */

/* XUSBPSU registers memory space boundaries */
#define XUSBPSU_GLOBALS_REGS_START              0xC100U /**< Global register start address */
#define XUSBPSU_GLOBALS_REGS_END                0xC6FFU /**< Global register end address */
#define XUSBPSU_DEVICE_REGS_START               0xC700U /**< Device register start address */
#define XUSBPSU_DEVICE_REGS_END                 0xCBFFU /**< Device register end address */
#define XUSBPSU_OTG_REGS_START                  0xCC00U /**< OTG register start address */
#define XUSBPSU_OTG_REGS_END                    0xCCFFU /**< OTG register end address */

/* Global Registers */
#define XUSBPSU_GSBUSCFG0                       0xC100U /**< Global SoC Bus Configuration Register 0 */
#define XUSBPSU_GSBUSCFG1                       0xC104U /**< Global SoC Bus Configuration Register 1 */
#define XUSBPSU_GTXTHRCFG                       0xC108U /**< Global Tx Threshold Control Register */
#define XUSBPSU_GRXTHRCFG                       0xC10CU /**< Global Rx Threshold Control Register */
#define XUSBPSU_GCTL                            0xC110U /**< Global Core Control Register */
#define XUSBPSU_GEVTEN                          0xC114U /**< Global Event Enable Register */
#define XUSBPSU_GSTS                            0xC118U /**< Global Status Register */
#define XUSBPSU_GSNPSID                         0xC120U /**< Global ID Register */
#define XUSBPSU_GGPIO                           0xC124U /**< Global General Purpose Input/Output Register */
#define XUSBPSU_GUID                            0xC128U /**< Global User ID Register This is a read/write */
#define XUSBPSU_GUCTL                           0xC12CU /**< Global User Control Register */
#define XUSBPSU_GBUSERRADDR0                    0xC130U /**< Global SoC Bus Error Address Register - Low */
#define XUSBPSU_GBUSERRADDR1                    0xC134U /**< Global SoC Bus Error Address Register - high */
#define XUSBPSU_GPRTBIMAP0                      0xC138U /**< Global SS Port to Bus Instance Mapping Register - Low */
#define XUSBPSU_GPRTBIMAP1                      0xC13CU /**< Global SS Port to Bus Instance Mapping Register - High */
#define XUSBPSU_GHWPARAMS0_OFFSET               0xC140U /**< Global Hardware Parameter Register 0 */
#define XUSBPSU_GHWPARAMS1_OFFSET               0xC144U /**< Global Hardware Parameter Register 1 */
#define XUSBPSU_GHWPARAMS2_OFFSET               0xC148U /**< Global Hardware Parameter Register 2 */
#define XUSBPSU_GHWPARAMS3_OFFSET               0xC14CU /**< Global Hardware Parameter Register 3 */
#define XUSBPSU_GHWPARAMS4_OFFSET               0xC150U /**< Global Hardware Parameter Register 4 */
#define XUSBPSU_GHWPARAMS5_OFFSET               0xC154U /**< Global Hardware Parameter Register 5 */
#define XUSBPSU_GHWPARAMS6_OFFSET               0xC158U /**< Global Hardware Parameter Register 6 */
#define XUSBPSU_GHWPARAMS7_OFFSET               0xC15CU /**< Global Hardware Parameter Register 7 */
#define XUSBPSU_GDBGFIFOSPACE                   0xC160U /**< Global Debug Queue/FIFO Space Available Register */
#define XUSBPSU_GDBGLTSSM                       0xC164U /**< Global Debug LTSSM Register */
#define XUSBPSU_GPRTBIMAP_HS0                   0xC180U /**< Global High-Speed Port to Bus Instance Mapping Register - Low */
#define XUSBPSU_GPRTBIMAP_HS1                   0xC184U /**< Global High-Speed Port to Bus Instance Mapping Register - High */
#define XUSBPSU_GPRTBIMAP_FS0                   0xC188U /**< Global Full-Speed Port to Bus Instance Mapping Register - Low */
#define XUSBPSU_GPRTBIMAP_FS1                   0xC18CU /**< Global Full-Speed Port to Bus Instance Mapping Register - High */

#define XUSBPSU_GUSB2PHYCFG(n)                  ((u32)0xc200 + ((u32)(n) * (u32)0x04)) /**< Global USB2 PHY Configuration Register */
#define XUSBPSU_GUSB2I2CCTL(n)                  ((u32)0xc240 + ((u32)(n) * (u32)0x04)) /**< Reserved Register */

#define XUSBPSU_GUSB2PHYACC(n)                  ((u32)0xc280 + ((u32)(n) * (u32)0x04)) /**< Global USB 2.0 UTMI PHY Vendor Control Register */

#define XUSBPSU_GUSB3PIPECTL(n)                 ((u32)0xc2c0 + ((u32)(n) * (u32)0x04)) /**< Global USB 3.0 PIPE Control Register */

#define XUSBPSU_GTXFIFOSIZ(n)                   ((u32)0xc300 + ((u32)(n) * (u32)0x04)) /**< Global Transmit FIFO Size Register */
#define XUSBPSU_GRXFIFOSIZ(n)                   ((u32)0xc380 + ((u32)(n) * (u32)0x04)) /**< Global Receive FIFO Size Register */

#define XUSBPSU_GEVNTADRLO(n)                   ((u32)0xc400 + ((u32)(n) * (u32)0x10)) /**< Global Event Buffer Address (Low) Register */
#define XUSBPSU_GEVNTADRHI(n)                   ((u32)0xc404 + ((u32)(n) * (u32)0x10)) /**< Global Event Buffer Address (High) Register */
#define XUSBPSU_GEVNTSIZ(n)                     ((u32)0xc408 + ((u32)(n) * (u32)0x10)) /**< Global Event Buffer Size Register */
#define XUSBPSU_GEVNTCOUNT(n)                   ((u32)0xc40c + ((u32)(n) * (u32)0x10)) /**< Global Event Buffer Count Register */

#define XUSBPSU_GHWPARAMS8                      0x0000c600U /**< Global Hardware Parameters */

/* Device Registers */
#define XUSBPSU_DCFG                            0x0000c700U /**< Device Configuration Register */
#define XUSBPSU_DCTL                            0x0000c704U /**< Device Control Register */
#define XUSBPSU_DEVTEN                          0x0000c708U /**< Device Event Enable Register */
#define XUSBPSU_DSTS                            0x0000c70cU /**< Device Status Register */
#define XUSBPSU_DGCMDPAR                        0x0000c710U /**< Device Generic Command Parameter Register */
#define XUSBPSU_DGCMD                           0x0000c714U /**< Device Generic Command Register */
#define XUSBPSU_DALEPENA                        0x0000c720U /**< Device Active USB Endpoint Enable Register */
#define XUSBPSU_DEPCMDPAR2(n)                   ((u32)0xc800 + ((u32)(n) * (u32)0x10U)) /**< Device Physical Endpoint-n Command Parameter 2 Register */
#define XUSBPSU_DEPCMDPAR1(n)                   ((u32)0xc804 + ((u32)(n) * (u32)0x10U)) /**< Device Physical Endpoint-n Command Parameter 1 Register */
#define XUSBPSU_DEPCMDPAR0(n)                   ((u32)0xc808 + ((u32)(n) * (u32)0x10U)) /**< Device Physical Endpoint-n Command Parameter 0 Register */
#define XUSBPSU_DEPCMD(n)                       ((u32)0xc80c + ((u32)(n) * (u32)0x10U)) /**< Device Physical Endpoint-n Command Register */

/* OTG Registers */
#define XUSBPSU_OCFG                            0x0000cc00U /**< OTG Configuration Register */
#define XUSBPSU_OCTL                            0x0000cc04U /**< OTG Control Register */
#define XUSBPSU_OEVT                            0xcc08U /**< OTG Events Register */
#define XUSBPSU_OEVTEN                          0xcc0CU /**< OTG Events Enable Register */
#define XUSBPSU_OSTS                            0xcc10U /**< OTG Status Register */

/* Bit fields */

/* Global Configuration Register */
#define XUSBPSU_GCTL_PWRDNSCALE(n)              ((n) << 19U) /**< Power down scale */
#define XUSBPSU_GCTL_U2RSTECN                   (1U << 16U) /**< U2RSTECN */
#define XUSBPSU_GCTL_RAMCLKSEL(x)       	(((x) & XUSBPSU_GCTL_CLK_MASK) << 6U) /**< RAM Clock Select */
#define XUSBPSU_GCTL_CLK_BUS                    (0U) /**< bus clock */
#define XUSBPSU_GCTL_CLK_PIPE                   (1U) /**< pipe clock */
#define XUSBPSU_GCTL_CLK_PIPEHALF               (2U) /**< pipe half clock */
#define XUSBPSU_GCTL_CLK_MASK                   (3U) /**< clock mask */

#define XUSBPSU_GCTL_PRTCAP(n)                  (((n) & (3U << 12U)) >> 12U) /**< Port Capability */
#define XUSBPSU_GCTL_PRTCAPDIR(n)               ((u32)(n) << 12U) /**< Port Capability Direction */
#define XUSBPSU_GCTL_PRTCAP_HOST                1U /**< Port Capability Host */
#define XUSBPSU_GCTL_PRTCAP_DEVICE              2U /**< Port Capability Device */
#define XUSBPSU_GCTL_PRTCAP_OTG                 3U /**< Port Capability OTG */

#define XUSBPSU_GCTL_CORESOFTRESET              0x00000800U /**< Core soft reset bit 11 */
#define XUSBPSU_GCTL_SOFITPSYNC                 0x00000400U /**< SOFITPSYNC bit 10 */
#define XUSBPSU_GCTL_SCALEDOWN(n)               ((u32)(n) << 4U) /**< Scale Down */
#define XUSBPSU_GCTL_SCALEDOWN_MASK             XUSBPSU_GCTL_SCALEDOWN(3U) /**< Scale Down mask */
#define XUSBPSU_GCTL_DISSCRAMBLE                (0x00000001U << 3U) /**< Disable Scrambling */
#define XUSBPSU_GCTL_U2EXIT_LFPS                (0x00000001U << 2U) /**< U2EXIT_LFPS */
#define XUSBPSU_GCTL_GBLHIBERNATIONEN           (0x00000001U << 1U) /**< Global Hibernation Enable */
#define XUSBPSU_GCTL_DSBLCLKGTNG                (0x00000001U << 0U) /**< Disable Clock Gating */

/* Global Status Register Device Interrupt Mask */
#define XUSBPSU_GSTS_DEVICE_IP_MASK 		0x00000040U /**< Device Interrupt */
#define XUSBPSU_GSTS_CUR_MODE			(0x00000001U << 0U) /**< Current Mode of Operation */

/* Global USB2 PHY Configuration Register */
#define XUSBPSU_GUSB2PHYCFG_PHYSOFTRST          0x80000000U /**< PHY Soft Reset */
#define XUSBPSU_GUSB2PHYCFG_SUSPHY              (0x00000001U << 6U) /**< Suspend USB2.0 HS/FS/LS PHY */

/* Global USB3 PIPE Control Register */
#define XUSBPSU_GUSB3PIPECTL_PHYSOFTRST         0x80000000U /**< USB3 PHY Soft Reset */
#define XUSBPSU_GUSB3PIPECTL_SUSPHY             0x00020000U /**< Suspend USB3.0 SS PHY (Suspend_en) */

/* Global TX Fifo Size Register */
#define XUSBPSU_GTXFIFOSIZ_TXFDEF(n)            ((u32)(n) & (u32)0xffffU) /**< TxFIFO Depth */
#define XUSBPSU_GTXFIFOSIZ_TXFSTADDR(n)         ((u32)(n) & 0xffff0000U) /**< Transmit FIFOn RAM Start Address */

/* Global Event Size Registers */
#define XUSBPSU_GEVNTSIZ_INTMASK                ((u32)0x00000001U << 31U) /**< Event Interrupt Mask */
#define XUSBPSU_GEVNTSIZ_SIZE(n)                ((u32)(n) & (u32)0xffffU) /**< Event Buffer Size in bytes */

/* Global HWPARAMS1 Register */
#define XUSBPSU_GHWPARAMS1_EN_PWROPT(n)         (((u32)(n) & ((u32)3 << 24U)) >> 24U) /**< USB3_EN_PWROPT */
#define XUSBPSU_GHWPARAMS1_EN_PWROPT_NO         0U /**< No USB3_EN_PWROPT */
#define XUSBPSU_GHWPARAMS1_EN_PWROPT_CLK        1U /**< Clock gating ctrl */
#define XUSBPSU_GHWPARAMS1_EN_PWROPT_HIB        2U /**< Hibernation */
#define XUSBPSU_GHWPARAMS1_PWROPT(n)            ((u32)(n) << 24U) /**< Power Post */
#define XUSBPSU_GHWPARAMS1_PWROPT_MASK          XUSBPSU_GHWPARAMS1_PWROPT(3U) /**< USB3_EN_PWROPT mask */

/* Global HWPARAMS4 Register */
#define XUSBPSU_GHWPARAMS4_HIBER_SCRATCHBUFS(n) (((u32)(n) & ((u32)0x0F << 13U)) >> 13U) /**< Number of external scratchpad buffers */
#define XUSBPSU_MAX_HIBER_SCRATCHBUFS           15U /**< Number of maximum scratchpad buffers */

/* Device Configuration Register */
#define XUSBPSU_DCFG_DEVADDR(addr)              ((u32)(addr) << 3U) /**< Device Address */
#define XUSBPSU_DCFG_DEVADDR_MASK               XUSBPSU_DCFG_DEVADDR(0x7FU) /**< Device Address Mask */

#define XUSBPSU_DCFG_SPEED_MASK			7U /**< Speed mask */
#define XUSBPSU_DCFG_SUPERSPEED			4U /**< Super Speed */
#define XUSBPSU_DCFG_HIGHSPEED			0U /**< High Speed */
#define XUSBPSU_DCFG_FULLSPEED2			1U /**< Full Speed */
#define XUSBPSU_DCFG_LOWSPEED			2U /**< Low Speed */
#define XUSBPSU_DCFG_FULLSPEED1			3U /**< Full1 Speed */

#define XUSBPSU_DCFG_LPM_CAP                    (0x00000001U << 22U) /**< LPM Capable */

/* Device Control Register */
#define XUSBPSU_DCTL_RUN_STOP                   0x80000000U /**< Run/Stop bit 31 */
#define XUSBPSU_DCTL_CSFTRST                    0x40000000U /**< Core Soft Reset bit 30 */
#define XUSBPSU_DCTL_LSFTRST                    0x20000000U /**< Reserved for DB-2.90a */

#define XUSBPSU_DCTL_HIRD_THRES_MASK            0x1F000000U /**< HIRD Threshold mask */
#define XUSBPSU_DCTL_HIRD_THRES(n)              ((u32)(n) << 24U) /**< HIRD Threshold */

#define XUSBPSU_DCTL_APPL1RES                   (0x00000001U << 23U) /**< LPM Response */

/* These apply for core versions 1.87a and earlier */
#define XUSBPSU_DCTL_TRGTULST_MASK              (0x0000000FU << 17U) /**< TRGTULST mask */
#define XUSBPSU_DCTL_TRGTULST(n)                ((u32)(n) << 17U) /**< TRGTULST */
#define XUSBPSU_DCTL_TRGTULST_U2                (XUSBPSU_DCTL_TRGTULST(2U)) /**< U2 */
#define XUSBPSU_DCTL_TRGTULST_U3                (XUSBPSU_DCTL_TRGTULST(3U)) /**< U3 */
#define XUSBPSU_DCTL_TRGTULST_SS_DIS            (XUSBPSU_DCTL_TRGTULST(4U)) /**< SS_DIS */
#define XUSBPSU_DCTL_TRGTULST_RX_DET            (XUSBPSU_DCTL_TRGTULST(5U)) /**< RX_DET */
#define XUSBPSU_DCTL_TRGTULST_SS_INACT          (XUSBPSU_DCTL_TRGTULST(6U)) /**< SS_INACT */

/* These apply for core versions 1.94a and later */
#define XUSBPSU_DCTL_KEEP_CONNECT               0x00080000U /**< Keep Connect */
#define XUSBPSU_DCTL_L1_HIBER_EN                0x00040000U /**< L1 Hibernation Enable */
#define XUSBPSU_DCTL_CRS                        0x00020000U /**< Controller Restore State */
#define XUSBPSU_DCTL_CSS                        0x00010000U /**< Controller Save State */

#define XUSBPSU_DCTL_INITU2ENA                  0x00001000U /**< Initiate U2 Enable */
#define XUSBPSU_DCTL_ACCEPTU2ENA                0x00000800U /**< Accept U2 Enable */
#define XUSBPSU_DCTL_INITU1ENA                  0x00000400U /**< Initiate U1 Enable */
#define XUSBPSU_DCTL_ACCEPTU1ENA                0x00000200U /**< Accept U1 Enable */
#define XUSBPSU_DCTL_TSTCTRL_MASK               (0x0000000fU << 1U) /**< Test Control */

#define XUSBPSU_DCTL_ULSTCHNGREQ_MASK           (0x0000000FU << 5U) /**< USB/Link state change request mask value */
#define XUSBPSU_DCTL_ULSTCHNGREQ(n) (((u32)(n) << 5U) & XUSBPSU_DCTL_ULSTCHNGREQ_MASK) /**< USB/Link state change request */

#define XUSBPSU_DCTL_ULSTCHNG_NO_ACTION         (XUSBPSU_DCTL_ULSTCHNGREQ(0U)) /**< NO_ACTION */
#define XUSBPSU_DCTL_ULSTCHNG_SS_DISABLED       (XUSBPSU_DCTL_ULSTCHNGREQ(4U)) /**< SS_DISABLED */
#define XUSBPSU_DCTL_ULSTCHNG_RX_DETECT         (XUSBPSU_DCTL_ULSTCHNGREQ(5U)) /**< RX_DETECT */
#define XUSBPSU_DCTL_ULSTCHNG_SS_INACTIVE       (XUSBPSU_DCTL_ULSTCHNGREQ(6U)) /**< SS_INACTIVE */
#define XUSBPSU_DCTL_ULSTCHNG_RECOVERY          (XUSBPSU_DCTL_ULSTCHNGREQ(8U)) /**< RECOVERY */
#define XUSBPSU_DCTL_ULSTCHNG_COMPLIANCE        (XUSBPSU_DCTL_ULSTCHNGREQ(10U)) /**< COMPLIANCE */
#define XUSBPSU_DCTL_ULSTCHNG_LOOPBACK          (XUSBPSU_DCTL_ULSTCHNGREQ(11U)) /**< LOOPBACK */
/** @endcond */

/* Device Event Enable Register */
#define XUSBPSU_DEVTEN_VNDRDEVTSTRCVEDEN        ((u32)0x00000001U << 12U) /**< Vendor Device Test LMP Received Event */
#define XUSBPSU_DEVTEN_EVNTOVERFLOWEN           ((u32)0x00000001U << 11U) /**< Reserved */
#define XUSBPSU_DEVTEN_CMDCMPLTEN               ((u32)0x00000001U << 10U) /**< Reserved */
#define XUSBPSU_DEVTEN_ERRTICERREN              ((u32)0x00000001U << 9U) /**< Erratic Error Event Enable */
#define XUSBPSU_DEVTEN_SOFEN                    ((u32)0x00000001U << 7U) /**< Start of (u)frame */
#define XUSBPSU_DEVTEN_EOPFEN                   ((u32)0x00000001U << 6U) /**< U3/L2-L1 Suspend Event Enable */
#define XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN      ((u32)0x00000001U << 5U) /**< Hibernation Request Event */
#define XUSBPSU_DEVTEN_WKUPEVTEN                ((u32)0x00000001U << 4U) /**< Resume/Remote Wakeup Detected Event Enable */
#define XUSBPSU_DEVTEN_ULSTCNGEN                ((u32)0x00000001U << 3U) /**< Link State Change Event Enable */
#define XUSBPSU_DEVTEN_CONNECTDONEEN            ((u32)0x00000001U << 2U) /**< Connection Done Enable */
#define XUSBPSU_DEVTEN_USBRSTEN                 ((u32)0x00000001U << 1U) /**< USB Reset Enable */
#define XUSBPSU_DEVTEN_DISCONNEVTEN             ((u32)0x00000001U << 0U) /**< Disconnect Detected Event Enable */

/** @cond INTERNAL */
/* Device Status Register */
#define XUSBPSU_DSTS_DCNRD                      0x40000000U /**< Device Controller Not Ready */

/* This applies for core versions 1.87a and earlier */
#define XUSBPSU_DSTS_PWRUPREQ                   0x01000000U /**< Reserved */

/* These apply for core versions 1.94a and later */
#define XUSBPSU_DSTS_RSS                        0x02000000U /**< RSS Restore State Status */
#define XUSBPSU_DSTS_SSS                        0x01000000U /**< SSS Save State Status */

#define XUSBPSU_DSTS_COREIDLE                   0x00800000U /**< Core Idle */
#define XUSBPSU_DSTS_DEVCTRLHLT                 0x00400000U /**< Device Controller Halted */

#define XUSBPSU_DSTS_USBLNKST_MASK              0x003C0000U /**< USB/Link State mask */
#define XUSBPSU_DSTS_USBLNKST(n) (((u32)(n) & XUSBPSU_DSTS_USBLNKST_MASK) >> 18U) /**< USB/Link State */

#define XUSBPSU_DSTS_RXFIFOEMPTY                (0x00000001U << 17U) /**< RxFIFO Empty */

#define XUSBPSU_DSTS_SOFFN_MASK                 (0x00003FFFU << 3U) /**< Frame/Microframe Number of the Received SOF mask */
#define XUSBPSU_DSTS_SOFFN(n)           (((u32)(n) & XUSBPSU_DSTS_SOFFN_MASK) >> 3U) /**< Frame/Microframe Number of the Received SOF */

#define XUSBPSU_DSTS_CONNECTSPD                 (0x00000007U << 0U) /**< Connected Speed mask */

#define XUSBPSU_DSTS_SUPERSPEED                 (4U << 0U) /**< Connected Super Speed */
#define XUSBPSU_DSTS_HIGHSPEED                  (0U << 0U) /**< Connected High Speed */
#define XUSBPSU_DSTS_FULLSPEED2                 (1U << 0U) /**< Connected Full Speed */
#define XUSBPSU_DSTS_LOWSPEED                   (2U << 0U) /**< Connected Low Speed */
#define XUSBPSU_DSTS_FULLSPEED1                 (3U << 0U) /**< Connected Full_Speed */

/* AXI-cache bits offset DATRDREQINFO */
#define XUSBPSU_GSBUSCFG0_BITMASK		0xFFFF0000U /**< Global SoC Bus Configuration mask */

/* Coherency Mode Register Offset */
#define XUSBPSU_COHERENCY			0x005CU /**< Coherency Mode Register offset */
#define XUSBPSU_COHERENCY_MODE_ENABLE		0x01U /**< Coherency Mode Enable */

/*Portpmsc 3.0 bit field*/
#define XUSBPSU_PORTMSC_30_FLA_MASK		(1U << 16U) /**< Force Link PM Accept */
#define XUSBPSU_PORTMSC_30_U2_TIMEOUT_MASK	0xFF00U /**< U2 TIMEOUT mask */
#define XUSBPSU_PORTMSC_30_U2_TIMEOUT_SHIFT	(8U) /**< U2 TIMEOUT  */
#define XUSBPSU_PORTMSC_30_U1_TIMEOUT_MASK	0xFFU /**< U1 TIMEOUT mask */
#define XUSBPSU_PORTMSC_30_U1_TIMEOUT_SHIFT	(0U) /**< U2 TIMEOUT */

/* Register for LPD block */
#if defined (PLATFORM_ZYNQMP)
#define RST_LPD_TOP				0x23CU /**< Software Reset Control for LPD System */
#define USB0_CORE_RST				0x40U /**< USB0 core reset */
#define USB1_CORE_RST				0x80U /**< USB1 core reset */
#else
#define RST_LPD_TOP				0x0314U /**< Software Reset Control for LPD System */
#define USB0_CORE_RST				0x01U /**< USB0 core reset */
#endif


/* Vendor registers for Xilinx */
#if defined (PLATFORM_ZYNQMP)
#define XIL_CUR_PWR_STATE			0x00U /**< Current power state offset */
#define XIL_PME_ENABLE				0x34U /**< PME Enable offset*/
#define XIL_REQ_PWR_STATE			0x3CU /**< Required power state offset */
#define XIL_PWR_CONFIG_USB3			0x48U /**< Power Configuration offset */
#else
#define XIL_CUR_PWR_STATE			0x00U /**< Current power state */
#define XIL_PME_ENABLE				0x1CU /**< PME Enable offset */
#define XIL_REQ_PWR_STATE			0X08U /**< Required power state offset */
#define XIL_PWR_CONFIG_USB3			0X20U /**< Power Configuration offset */
#define XIL_VSL_USB2_PHYRST_MASK		0X1CU /**< Versal PHY reset mask */
#endif

#define XIL_REQ_PWR_STATE_D0			0U /**< Required power state D0 */
#define XIL_REQ_PWR_STATE_D3			3U /**< Required power state D3 */
#define XIL_PME_ENABLE_SIG_GEN			1U /**< PME Enable */
#define XIL_CUR_PWR_STATE_D0			0U /**< Current power state D0 */
#define XIL_CUR_PWR_STATE_D3			3U /**< Current power state D3 */
#define XIL_CUR_PWR_STATE_BITMASK		0x03U /**< Current power state bit mask */

#define VENDOR_BASE_ADDRESS			0xFF9D0000U /**< Vendor base address */

#if defined (versal)
#define VSL_CUR_PWR_ST_REG			0xF1060600U /**< versal current power state */
#endif

#define LPD_BASE_ADDRESS			0xFF5E0000U /**< LPD system base address */
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* Reads a register of the USBPS8 device. This macro provides register
* access to all registers using the register offsets.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	Offset Offset of the register to read.
*
* @return	The contents of the register.
*
* @note		C-style Signature:
*		    u32 XUsbPsu_ReadReg(struct XUsbPsu *InstancePtr, u32 Offset);
*
******************************************************************************/
#define XUsbPsu_ReadReg(InstancePtr, Offset) \
	Xil_In32((InstancePtr)->ConfigPtr->BaseAddress + (u32)(Offset))

/*****************************************************************************/
/**
*
* Writes a register of the USBPS8 device. This macro provides
* register access to all registers using the register offsets.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	Offset Offset of the register to write.
* @param	Data Value to write to the register.
*
* @return	None.
*
* @note 	C-style Signature:
*		    void XUsbPsu_WriteReg(struct XUsbPsu *InstancePtr,
*								u32 Offset,u32 Data)
*
******************************************************************************/
#define XUsbPsu_WriteReg(InstancePtr, Offset, Data) \
	Xil_Out32((InstancePtr)->ConfigPtr->BaseAddress + (u32)(Offset), (u32)(Data))

/*****************************************************************************/
/**
*
* Reads a vendor register of the USBPS8 device.
*
* @param       Offset Offset of the register to read.
*
* @return      The contents of the register.
*
* @note        C-style Signature:
*              u32 XUsbPsu_ReadVendorReg(struct XUsbPsu *InstancePtr, u32 Offset);
*
******************************************************************************/
#define XUsbPsu_ReadVendorReg(Offset) \
	Xil_In32(VENDOR_BASE_ADDRESS + (u32)(Offset))

/*****************************************************************************/
/**
*
* Writes a Vendor register of the USBPS8 device.
*
* @param       Offset Offset of the register to write.
* @param       Data Value to write to the register.
*
* @return      None.
*
* @note        C-style Signature:
*              void XUsbPsu_WriteVendorReg(struct XUsbPsu *InstancePtr,
*                                                         u32 Offset,u32 Data);
*
******************************************************************************/
#define XUsbPsu_WriteVendorReg(Offset, Data) \
	Xil_Out32(VENDOR_BASE_ADDRESS + (u32)(Offset), (u32)(Data))

#if defined (versal)
/*****************************************************************************/
/**
*
* Reads a power state register of the USBPSU device.
*
* @param       Offset Offset of the register to read.
*
* @return      The contents of the register.
*
* @note        C-style Signature:
*              u32 XUsbPsu_ReadVslPwrStateReg(struct XUsbPsu *InstancePtr,
*								u32 Offset);
*
******************************************************************************/
#define XUsbPsu_ReadVslPwrStateReg(Offset) \
	Xil_In32(VSL_CUR_PWR_ST_REG + (u32)(Offset))

/*****************************************************************************/
/**
*
* Writes a power state register of the USBPSU device.
*
* @param       Offset Offset of the register to write.
* @param       Data Value to write to the register.
*
* @return      None.
*
* @note        C-style Signature:
*              void XUsbPsu_WriteVslPwrStateReg(struct XUsbPsu *InstancePtr,
*                                                         u32 Offset,u32 Data);
*
******************************************************************************/
#define XUsbPsu_WriteVslPwrStateReg(Offset, Data) \
	Xil_Out32(VSL_CUR_PWR_ST_REG + (u32)(Offset), (u32)(Data))
#endif

/*****************************************************************************/
/**
*
* Reads a LPD register of the USBPS8 device.
*
*
* @param       Offset Offset of the register to read.
*
* @return      The contents of the register.
*
* @note        C-style Signature:
*              u32 XUsbPsu_ReadLpdReg(struct XUsbPsu *InstancePtr, u32 Offset);
*
******************************************************************************/
#define XUsbPsu_ReadLpdReg(Offset) \
	Xil_In32(LPD_BASE_ADDRESS + (u32)(Offset))

/*****************************************************************************/
/**
*
* Writes a LPD register of the USBPS8 device.
*
* @param       Offset Offset of the register to write.
* @param       Data Value to write to the register.
*
* @return      None.
*
* @note        C-style Signature:
*              void XUsbPsu_WriteLpdReg(struct XUsbPsu *InstancePtr,
*                                                         u32 Offset,u32 Data);
*
******************************************************************************/
#define XUsbPsu_WriteLpdReg(Offset, Data) \
	Xil_Out32(LPD_BASE_ADDRESS + (u32)(Offset), (u32)(Data))

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
/** @endcond */
/** @} */
