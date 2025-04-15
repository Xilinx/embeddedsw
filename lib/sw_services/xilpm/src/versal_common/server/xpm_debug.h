/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_DEBUG_H_
#define XPM_DEBUG_H_

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************** Power domain specific Errors ***********************/
/************************** (0x3000) - (0x30FF) ******************************/
/* Invalid power domain ID error */
#define XPM_INT_ERR_INVALID_PWR_DOMAIN		0x3000U
/* Error during power up */
#define XPM_INT_ERR_PWR_STATE_ON_EVENT		0x3001U
/* Error during power down */
#define XPM_INT_ERR_PWR_STATE_OFF_EVENT		0x3002U
/* Invalid state */
#define XPM_INT_ERR_INVALID_STATE		0x3003U
/* Error during domain isolation enable/disable */
#define XPM_INT_ERR_DOMAIN_ISO			0x3004U
/* Invalid power state */
#define XPM_INT_ERR_INVALID_PWR_STATE		0x3005U
/* Invalid function ID */
#define XPM_INT_ERR_INVALID_FUNC		0x3006U
/* Power domain state is OFF */
#define XPM_INT_ERR_PWR_DOMAIN_OFF		0x3007U
/* Error due to rail control during init */
#define XPM_INT_ERR_PWR_DOMAIN_RAIL_CONTROL	0X3008U
/* Invalid arguments for NOC clock gating */
#define XPM_INT_ERR_NOC_CLOCK_GATING		0x3009U

/*********************** Generic House Cleaning Errors ***********************/
/*************************** (0x3100) - (0x31FF) *****************************/
/* Error during power domain init */
#define XPM_INT_ERR_POWER_DOMAIN_INIT		0x3100U
/* Invalid base address */
#define XPM_INT_ERR_INVALID_BASEADDR		0x3101U
/* Power supply state does not match */
#define XPM_INT_ERR_POWER_SUPPLY		0x3102U
/* Error during apply Ams trim */
#define XPM_INT_ERR_AMS_TRIM			0x3103U
/* Error during pre BISR requirement */
#define XPM_INT_ERR_PRE_BISR_REQ		0x3104U
/* Scan clear done timeout */
#define XPM_INT_ERR_SCAN_CLEAR_TIMEOUT		0x3105U
/* Scan clear pass error */
#define XPM_INT_ERR_SCAN_PASS			0x3106U
/* LBIST done timeout */
#define XPM_INT_ERR_LBIST_DONE_TIMEOUT		0x3107U
/* Error during BISR repair */
#define XPM_INT_ERR_BISR_REPAIR			0x3108U
/* Mem clear done timeout */
#define XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT	0x3109U
/* Mem clear pass timeout */
#define XPM_INT_ERR_MEM_CLEAR_PASS_TIMEOUT	0x310AU
/* MBIST done timeout */
#define XPM_INT_ERR_MBIST_DONE_TIMEOUT		0x310BU
/* MBIST not done proper */
#define XPM_INT_ERR_MBIST_GOOD			0x310CU
/* MBIST GO bit not set */
#define XPM_INT_ERR_MBIST_GO			0x310DU
/* Invalid trim type */
#define XPM_INT_ERR_INVALID_TRIM_TYPE		0x310EU
/* Error during self test */
#define XPM_INT_ERR_SELF_TEST			0x310FU
/* MEM clear not passed */
#define XPM_INT_ERR_MEM_CLEAR_PASS		0x3110U
/* MBIST pass timeout error */
#define XPM_INT_ERR_MBIST_PASS			0x3111U
/* Error during MBIST */
#define XPM_INT_ERR_MBIST			0x3112U
/* Error during scan clear trigger */
#define XPM_INT_ERR_SCAN_CLEAR_TRIGGER		0x3113U
/* Scan clear pass timeout */
#define XPM_INT_ERR_SCAN_CLEAR_PASS		0x3114U
/* Scan clear trigger is unset */
#define XPM_INT_ERR_SCAN_CLEAR_TRIGGER_UNSET	0x3115U
/* Error during MEM clear enable */
#define XPM_INT_ERR_MEM_CLEAR_EN		0x3116U
/* Error during MBIST reset */
#define XPM_INT_ERR_MBIST_RESET			0x3117U
/* Error during BIST reset */
#define XPM_INT_ERR_BIST_RESET			0x3118U
/* Error during MEM clear trigger */
#define XPM_INT_ERR_MEM_CLEAR_TRIGGER		0x3119U
/* Error during MBIST reset release */
#define XPM_INT_ERR_MBIST_RESET_RELEASE		0x311AU
/* Error during BIST reset release */
#define XPM_INT_ERR_BIST_RESET_RELEASE		0x311BU
/* MEM clear trigger is unset*/
#define XPM_INT_ERR_MEM_CLEAR_TRIGGER_UNSET	0x311CU
/* Error during init start function */
#define XPM_INT_ERR_FUNC_INIT_START		0x311DU
/* Error during init finish function */
#define XPM_INT_ERR_FUNC_INIT_FINISH		0x311EU
/* Error during init scan clear function */
#define XPM_INT_ERR_FUNC_SCAN_CLEAR		0x311FU
/* Error during init BISR function */
#define XPM_INT_ERR_FUNC_BISR			0x3120U
/* Error during init LBIST function */
#define XPM_INT_ERR_FUNC_LBIST			0x3121U
/* Error during init MEM init function */
#define XPM_INT_ERR_FUNC_MEM_INIT		0x3122U
/* Error during init MBIST clear function */
#define XPM_INT_ERR_FUNC_MBIST_CLEAR		0x3123U
/* Error during init house clean function */
#define XPM_INT_ERR_FUNC_HOUSECLEAN_PL		0x3124U
/* Error during init house clean complete function */
#define XPM_INT_ERR_FUNC_HOUSECLEAN_COMPLETE	0x3125U
/* Required clocks are not found in LPD MBIST */
#define XPM_INT_ERR_LPD_MBIST_CLK_NOT_FOUND	0x3126U
/* CFU clock divider is in unexpected state */
#define XPM_INT_ERR_CFU_CLK_DIVIDER		0x3127U
/* Error during ams trim routine */
#define XPM_INT_ERR_FUNC_AMS_TRIM		0x3128U
/* Error during power domain init and house clean*/
#define XPM_INT_ERR_DOMAIN_INIT_AND_HC		0x3129U
/************************** LPD specific Errors ******************************/
/************************** (0x3200) - (0x32FF) ******************************/
/* Error during PMC-LPD DFX Isolation enable/disable */
#define XPM_INT_ERR_PMC_LPD_DFX_ISO		0x3200U
/* Error during PMC-LPD Isolation enable/disable */
#define XPM_INT_ERR_PMC_LPD_ISO			0x3201U
/* Error during LPD-SOC Isolation enable/disable */
#define XPM_INT_ERR_LPD_SOC_ISO			0x3202U
/* Error during PS POR reset assert/release */
#define XPM_INT_ERR_PS_POR			0x3203U
/* Error during PS SRST reset assert/release */
#define XPM_INT_ERR_PS_SRST			0x3204U
/* Error during XRAM BISR repair */
#define XPM_INT_ERR_XRAM_BISR_REPAIR		0x3205U
/* Error during XRAM MBIST */
#define XPM_INT_ERR_XRAM_MBIST			0x3206U
/* Error during LPD-PL test Isolation enable/disable */
#define XPM_INT_ERR_LPD_PL_TEST_ISO		0x3207U
/* Error during LPD-PL Isolation enable/disable */
#define XPM_INT_ERR_LPD_PL_ISO			0x3208U
/* Error during LPD-CPM Isolation enable/disable */
#define XPM_INT_ERR_LPD_CPM_ISO			0x3209U
/* Error during LPD rail control */
#define XPM_INT_ERR_LPD_RAIL_CONTROL		0X320AU

/************************* Device specific Errors ****************************/
/************************** (0x3300) - (0x33FF) ******************************/
/* Invalid device ID */
#define XPM_INT_ERR_INVALID_DEVICE		0x3300U
/* Error during device lookup configuration */
#define XPM_INT_ERR_DEVICE_LOOKUP		0x3301U
/* Error during configuration initialization */
#define XPM_INT_ERR_CFG_INIT			0x3302U
/* Error during request PL device */
#define XPM_INT_ERR_REQ_PL_DEVICE		0x3303U
/* Error during request ME device */
#define XPM_INT_ERR_REQ_ME_DEVICE		0x3304U
/* Error during device init */
#define XPM_INT_ERR_DEVICE_INIT			0x3305U
/* Error during Add requirement of device*/
#define XPM_INT_ERR_ADD_REQUIREMENT		0x3306U
/* Error during add clock */
#define XPM_INT_ERR_ADD_CLK			0x3307U
/* Error during add reset node */
#define XPM_INT_ERR_ADD_RST			0x3308U
/* Error during set device node */
#define XPM_INT_ERR_SET_DEV_NODE		0x3309U
/* Error during set PL device */
#define XPM_INT_ERR_SET_PL_DEV			0x330AU
/* Error during set mem region */
#define XPM_INT_ERR_SET_MEM_REG_DEV		0x330BU
/* Error during get device permission */
#define XPM_INT_ERR_GET_DEVICE_PERMISSION	0x330CU
/* Subsystem has no permission to that device */
#define XPM_INT_ERR_DEVICE_PERMISSION		0x330DU
/* Shared resource has no permission to access */
#define XPM_INT_ERR_SHARED_RESOURCE		0x330EU
/* Device is busy */
#define XPM_INT_ERR_DEVICE_BUSY			0x330FU
/* Error during power up device parent power domain */
#define XPM_INT_ERR_DEVICE_PWR_PARENT_UP	0x3310U
/* Error during device request */
#define XPM_INT_ERR_DEVICE_REQUEST		0x3311U
/* Error during device release */
#define XPM_INT_ERR_DEVICE_RELEASE		0x3312U
/* Error during device change state */
#define XPM_INT_ERR_DEVICE_CHANGE_STATE	0x3313U
/* Error during set virtual device node */
#define XPM_INT_ERR_SET_VIRT_DEV		0x3314U
/* Error during set healthy boot monitor device node */
#define XPM_INT_ERR_SET_HB_MON_DEV		0x3315U
/* Error during set AIE device node */
#define XPM_INT_ERR_SET_AIE_DEV		0x3316U
/* Error during request GPIO device node */
#define XPM_INT_ERR_REQ_GPIO		0x3317U
/* Error during request memory region device node */
#define XPM_INT_ERR_SET_MEM_REGN_DEV	0x3318U

/************************* Reset specific Errors *****************************/
/************************** (0x3400) - (0x34FF) ******************************/
/* Invalid Reset ID */
#define XPM_INT_ERR_INVALID_RST			0x3400U
/* Invalid reset state */
#define XPM_INT_ERR_RST_STATE			0x3401U
/* Error during reset release */
#define XPM_INT_ERR_RST_RELEASE			0x3402U
/* Error during reset assert */
#define XPM_INT_ERR_RST_ASSERT			0x3403U
/* Error during set reset node */
#define XPM_INT_ERR_SET_RESET_NODE		0x3404U
/* Error during custom reset release */
#define XPM_INT_ERR_CUSTOM_RESET_RELEASE	0x3405U
/* Error during custom reset assert */
#define XPM_INT_ERR_CUSTOM_RESET_ASSERT		0x3406U
/* Error during custom reset pulse */
#define XPM_INT_ERR_CUSTOM_RESET_PULSE		0x3407U
/* Error during USB reset release */
#define XPM_INT_ERR_USB_RST_RELEASE		0x3408U

/************************** FPD specific Errors ******************************/
/************************** (0x3500) - (0x35FF) ******************************/
/* Error during FPD power on reset release/assert */
#define XPM_INT_ERR_FPD_POR			0x3500U
/* Error during FPD-SOC Isolation disable/enable */
#define XPM_INT_ERR_FPD_SOC_ISO			0x3501U
/* Error during SRST FPD reset release/assert */
#define XPM_INT_ERR_SRST_FPD			0x3502U
/* Error during FPD-PL Isolation disable/enable */
#define XPM_INT_ERR_FPD_PL_ISO			0x3503U
/* Error during FPD-PL test Isolation disable/enable */
#define XPM_INT_ERR_FPD_PL_TEST_ISO		0x3504U
/* Error during fpd rail control */
#define XPM_INT_ERR_FPD_RAIL_CONTROL		0X3505U

/************************* Miscellaneous Errors ******************************/
/************************** (0x3600) - (0x36FF) ******************************/
/* Error during IPI send */
#define XPM_INT_ERR_IPI_SEND			0x3600U
/* Error during get IPI status */
#define XPM_INT_ERR_IPI_STATUS			0x3601U
/* PSMFW is not present */
#define XPM_INT_ERR_PSMFW_NOT_PRESENT		0x3602U
/* Invalid parameter passed to function */
#define XPM_INT_ERR_INVALID_PARAM		0x3603U
/* Invalid address */
#define XPM_INT_ERR_INVALID_ADDR		0x3604U
/* Error during reload image */
#define XPM_INT_ERR_RELOAD_IMAGE		0x3605U
/* Invalid processor */
#define XPM_INT_ERR_INVALID_PROC		0x3606U
/* Invalid NODE ID */
#define XPM_INT_ERR_INVALID_NODE		0x3607U
#if defined (VERSAL_ENABLE_DOMAIN_CONTROL_GPIO) || defined (RAIL_CONTROL)
/* Invalid NODE index */
#define XPM_INT_ERR_INVALID_NODE_IDX		0x3608U
#endif /* VERSAL_ENABLE_DOMAIN_CONTROL_GPIO || RAIL_CONTROL */
/* Invalid arguments */
#define XPM_INT_ERR_INVALID_ARGS		0x3609U
/* No feature supported */
#define XPM_INT_ERR_NO_FEATURE			0x360AU
/* Invalid subclass */
#define XPM_INT_ERR_INVALID_SUBCLASS		0x360BU
/* Buffer too small */
#define XPM_INT_ERR_BUFFER_TOO_SMALL		0x360CU
/* Invalid event */
#define XPM_INT_ERR_INVALID_EVENT		0x360DU
/* Error during reset SD DLL registers */
#define XPM_INT_ERR_RST_SD_DLL_REGS		0x360EU
/* Error during RPU core halt */
#define XPM_INT_ERR_RPU_CORE_HALT		0x360FU
/* Error during PSM RAM ECC init */
#define XPM_INT_ERR_ECC_INIT_PSM_RAM		0x3610U
/* Invalid state transition */
#define XPM_INT_ERR_INVALID_STATE_TRANS	0x3611U
/* Error during execution of InitNode */
#define XPM_INT_ERR_INITNODE			0x3612U
/* Error during parsing isolation node. getting invalid format */
#define XPM_INT_ERR_ISO_INVALID_FORMAT		0x3613U
/* A topology iso node has exceed number of dependencies */
#define XPM_INT_ERR_ISO_MAX_DEPENDENCIES	0X3614U
/* Out of bound */
#define XPM_INT_ERR_OUT_OF_RANGE		0x3615U

/************************** PLD specific Errors ******************************/
/************************** (0x3700) - (0x37FF) ******************************/
/* Error during GTY BISR repair */
#define XPM_INT_ERR_GTY_BISR_REPAIR		0x3700U
/* Error during GTM BISR repair */
#define XPM_INT_ERR_GTM_BISR_REPAIR		0x3701U
/* Error during GTYP BISR repair */
#define XPM_INT_ERR_GTYP_BISR_REPAIR		0x3702U
/* Error during DMAC BISR repair */
#define XPM_INT_ERR_DCMAC_BISR_REPAIR		0x3703U
/* Error during ILKN BISR repair */
#define XPM_INT_ERR_ILKN_BISR_REPAIR		0x3704U
/* Error during MRMAC BISR repair */
#define XPM_INT_ERR_MRMAC_BISR_REPAIR		0x3705U
/* Error during BRAM BISR repair */
#define XPM_INT_ERR_BRAM_BISR_REPAIR		0x3706U
/* Error during URAM BISR repair */
#define XPM_INT_ERR_URAM_BISR_REPAIR		0x3707U
/* Error during PL-SOC isolation enable/disable */
#define XPM_INT_ERR_PL_SOC_ISO			0x3708U
/* Error during PL-SOC-NPI isolation enable/disable */
#define XPM_INT_ERR_PMC_SOC_NPI_ISO		0x3709U
/* Error during PL house clean */
#define XPM_INT_ERR_PL_HC			0x370AU
/* Error during CFU initialization */
#define XPM_INT_ERR_CFU_INIT			0x370BU
/* Error during GTY house clean */
#define XPM_INT_ERR_GTY_HC			0x370CU
/* Error during VCCAUX-VCCRAM isolation enable/disable */
#define XPM_INT_ERR_VCCAUX_VCCRAM_ISO		0x370DU
/* Error during PL-POR reset assert/release */
#define XPM_INT_ERR_PL_POR			0x370EU
/* PL power status timeout */
#define XPM_INT_ERR_PL_STATUS_TIMEOUT		0x370FU
/* Error during PL SRST reset assert/release */
#define XPM_INT_ERR_PL_SRST			0x3710U
/* Error during CFRAME initialization */
#define XPM_INT_ERR_CFRAME_INIT			0x3711U
/* Error during PMC-PL-CFRAME isolation enable/disable */
#define XPM_INT_ERR_PMC_PL_CFRAME_ISO		0x3712U
/* Error during SDFEC BISR repair */
#define XPM_INT_ERR_SDFEC_BISR_REPAIR		0x3713U
/* Error during VCCRAM-SOC isolation enable/disable */
#define XPM_INT_ERR_VCCRAM_SOC_ISO		0x3714U
/* CFU not ready error */
#define XPM_INT_ERR_CFU_NOT_READY		0x3715U
/* Error during check for PL POR status */
#define XPM_INT_ERR_PL_POR_STATUS		0x3716U
/* Error during initialization of GTY Address Array */
#define XPM_INT_ERR_GTY_INIT			0x3717U
/* Error during PLD rail control */
#define XPM_INT_ERR_PLD_RAIL_CONTROL		0X3718U
/* Error during redundancy check for GTY Memory clear loop */
#define XPM_INT_ERR_GTY_MEM_CLEAR_LOOP		0x3719U
/* Error during Laguna repair */
#define XPM_INT_ERR_LAGUNA_REPAIR		0x371AU
/* Error during disabling NPI Ref Clk */
#define XPM_INT_ERR_DIS_NPI_REF_CLK		0x371BU
/* Error during enabling NPI Ref Clk */
#define XPM_INT_ERR_EN_NPI_REF_CLK		0x371CU
/* Error while asserting PL_POR, NPI Reset or disabling NPI Clk */
#define XPM_INT_ASSERT_RST_DIS_CLK		0x371DU
/* Error while removing PL_POR, NPI Reset or enabling NPI Clk */
#define XPM_INT_REMOVE_RST_EN_CLK		0x371EU
/* Error while applying GTY workaround */
#define XPM_INT_ERR_GT_WORKAROUND		0x371FU
/* Error during HSC BISR repair */
#define XPM_INT_ERR_HSC_BISR_REPAIR		0x3720U
/* Timeout for PL house cleaning completion*/
#define XPM_INT_ERR_PL_HC_COMPLETE_TIMEOUT	0x3721U
/* Error during Laguna housecleaning sequence */
#define XPM_INT_ERR_LAGUNA_HOUSECLEAN		0x3722U

/************************** NPD specific Errors ******************************/
/************************** (0x3800) - (0x38FF) ******************************/
/* Error during NOC POR assert/release*/
#define XPM_INT_ERR_NOC_POR			0x3800U
/* DDRMC power status error */
#define XPM_INT_ERR_NOC_DDRMC_STATUS		0x3801U
/* DDR MEM clear done timeout */
#define XPM_INT_ERR_DDR_MEM_CLEAR_DONE		0x3802U
/* DDR MEM clear pass timeout */
#define XPM_INT_ERR_DDR_MEM_CLEAR_PASS		0x3803U
/* Error during NIDB BISR repair */
#define XPM_INT_ERR_NIDB_BISR_REPAIR		0x3804U
/* Error during PMC-SOC isolation enable/disable */
#define XPM_INT_ERR_PMC_SOC_ISO			0x3805U
/* Error during NPI reset assert/release */
#define XPM_INT_ERR_RST_NPI			0x3806U
/* Error idling bus during NPD ScanClear workaround for xcvm2152 */
#define XPM_INT_ERR_NPD_SCANCLEAR_BUS_IDLE	0x3807U
/* Error activating bus during NPD ScanClear workaround for xcvm2152 */
#define XPM_INT_ERR_NPD_SCANCLEAR_BUS_ACT	0x3808U

/************************* Clock specific errors *****************************/
/************************** (0x3900) - (0x39FF) ******************************/
/* Error during clock enable */
#define XPM_INT_ERR_CLK_ENABLE			0x3900U
/* Max clock parents error */
#define XPM_INT_ERR_MAX_CLK_PARENTS		0x3901U
/* Error during clock init */
#define XPM_INT_ERR_CLK_INIT			0x3902U
/* Topology nodes greater than max number of nodes */
#define XPM_INT_ERR_CLK_TOPOLOGY_MAX_NUM_NODES	0x3903U
/* Invalid clock type */
#define XPM_INT_ERR_INVALID_CLK_TYPE		0x3904U
/* Invalid clock parent */
#define XPM_INT_ERR_INVALID_CLK_PARENT		0x3905U
/* Read only clock */
#define XPM_INT_ERR_READ_ONLY_CLK		0x3906U
/* PLL permission error as PLLs are shared */
#define XPM_INT_ERR_PLL_PERMISSION		0x3907U
/* Error during clock disable */
#define XPM_INT_ERR_CLK_DISABLE			0x3908U
/* Error enabling CPM_TOPSW_REF clk */
#define XPM_INT_ERR_CPM_TOPSW_REF_CLK_ENABLE	0x3909U
/* Error enabling USB clk */
#define XPM_INT_ERR_USB_CLK_ENABLE		0x390AU
/* Error enabling CAN_0 clk */
#define XPM_INT_ERR_CAN0_CLK_ENABLE		0x390BU
/* Error enabling CAN_1 clk */
#define XPM_INT_ERR_CAN1_CLK_ENABLE		0x390CU

/************************** CPM specific Errors ******************************/
/************************** (0x4000) - (0x40FF) ******************************/
/* Error during LPD-CPM DFX Isolation enable/disable */
#define XPM_INT_ERR_LPD_CPM_DFX_ISO		0x4000U
/* Error during LPD-CPM5 DFX Isolation enable/disable */
#define XPM_INT_ERR_LPD_CPM5_DFX_ISO		0x4001U
/* Error during CPM5 BISR repair*/
#define XPM_INT_ERR_CPM5_BISR_REPAIR		0x4002U
/* Error during CPM5 GTYP BISR repair */
#define XPM_INT_ERR_CPM5_GTYP_BISR_REPAIR	0x4003U
/* Error during PL-CPM-PCIEA0 Isolation enable/disable */
#define XPM_INT_ERR_PL_CPM_PCIEA0_ISO		0x4004U
/* Error during PL-CPM-PCIEA1 Isolation enable/disable */
#define XPM_INT_ERR_PL_CPM_PCIEA1_ISO		0x4005U
/* Error during PL_CPM_RST_CPI0 Isolation enable/disable */
#define XPM_INT_ERR_PL_CPM_RST_CPI0_ISO		0x4006U
/* Error during PL_CPM_RST_CPI1 Isolation enable/disable */
#define XPM_INT_ERR_PL_CPM_RST_CPI1_ISO		0x4007U
/* Error in CPM5 MBIST loop redundancy check */
#define XPM_INT_ERR_CPM5_MBIST_LOOP		0x4008U
/* Error during LPD-CPM5 Isolation enable/disable */
#define XPM_INT_ERR_LPD_CPM5_ISO		0x4009U
/* Error during PL-CPM5 Isolation enable/disable */
#define XPM_INT_ERR_PL_CPM5_ISO			0x400AU
/* Error during PL-CPM5 DFX Isolation enable/disable */
#define XPM_INT_ERR_PL_CPM5_DFX_ISO		0x400BU
/* Error during CPM5 init in reset */
#define XPM_INT_ERR_CPM5_INIT_RST			0x400CU
/* Error during CPM5 scan clear in reset */
#define XPM_INT_ERR_CPM5_SCAN_CLEAR_RST		0x400DU
/* Error during CPM BISR in reset */
#define XPM_INT_ERR_CPM_BISR_RST			0x400EU
/* Error during CPM5 BISR in reset */
#define XPM_INT_ERR_CPM5_BISR_RST			0x400FU
/* Error during CPM MBIST in reset */
#define XPM_INT_ERR_CPM_MBIST_RST			0x4010U
/* Error during CPM5 MBIST in reset */
#define XPM_INT_ERR_CPM5_MBIST_RST			0x4011U
/* Error during CPM6 init in reset */
#define XPM_INT_ERR_CPM6_INIT_RST		0x4012U
/* Error during LPD-CPM6 DFX isolation enable/disable */
#define XPM_INT_ERR_LPD_CPM6_DFX_ISO		0x4013U
/* Error during CPM6 scan clear in reset */
#define XPM_INT_ERR_CPM6_SCAN_CLEAR_RST		0x4014U
/* Error during LPD-CPM6 isolation enable/disable */
#define XPM_INT_ERR_LPD_CPM6_ISO		0x4015U
/* Error during CPM6 GTMPW MIBST Loop */
#define XPM_INT_ERR_CPM6_GTMPW_MBIST_LOOP	0x4016U

/************************** AIE specific Errors ******************************/
/************************** (0x4100) - (0x41FF) ******************************/
/* Timeout during AIE core status check */
#define XPM_INT_ERR_AIE_CORE_STATUS_TIMEOUT	0x4100U
/* Error during program core */
#define XPM_INT_ERR_PRGRM_CORE			0x4101U
/* AIE program complete error */
#define XPM_INT_ERR_AIE_PCOMPLETE		0x4102U
/* Error during ODISABLE_0 release */
#define XPM_INT_ERR_ODISABLE_0_RELEASE		0x4103U
/* Error during ODISABLE_1 release */
#define XPM_INT_ERR_ODISABLE_1_RELEASE		0x4104U
/* Gate register is unset */
#define XPM_INT_ERR_GATEREG_UNSET		0x4105U
/* Error during MEA BISR repair */
#define XPM_INT_ERR_MEA_BISR_REPAIR		0x4106U
/* Error during MEB BISR repair */
#define XPM_INT_ERR_MEB_BISR_REPAIR		0x4107U
/* Error during MEC BISR repair */
#define XPM_INT_ERR_MEC_BISR_REPAIR		0x4108U
/* Error in ARRAY reset */
#define XPM_INT_ERR_ARRAY_RESET			0x4109U
/* Error during ARRAY reset release */
#define XPM_INT_ERR_ARRAY_RESET_RELEASE		0x410AU
/* Error during INITSTATE release */
#define XPM_INT_ERR_AIE_INITSTATE_RELEASE	0x410BU
/* Error during Memory Zeroization */
#define XPM_INT_ERR_AIE_MEMORY_ZEROISATION	0x410CU
/* Error during AIE Post Scan Clear Hook */
#define XPM_INT_ERR_AIE_POST_SCAN_CLEAR_HOOK	0x410DU
/* Error during AIE Pre Bisr Hook */
#define XPM_INT_ERR_AIE_PRE_BISR_HOOK		0x410EU
/* Error during enable of AIE SHIM rest */
#define XPM_INT_ERR_AIE_SHIM_RST_EN		0x410FU
/* Error during disable of AIE SHIM rest */
#define XPM_INT_ERR_AIE_SHIM_RST_DIS		0x4110U
/* Error during AIE1 Operations */
#define XPM_INT_ERR_AIE1_OPS			0x4111U
/* Error during AIE2 Operations */
#define XPM_INT_ERR_AIE2_OPS			0x4112U
/* Error during zeroization of program memory */
#define XPM_INT_ERR_AIE_PROG_MEM_ZEROISATION	0x4113U
/* Error during zeroization of data memory */
#define XPM_INT_ERR_AIE_DATA_MEM_ZEROISATION	0x4114U
/* Error during zeriozation of uc memory */
#define XPM_INT_ERR_AIE_UC_ZEROISATION		0x4115U
/* Error during NMU config indicating invalid column */
#define XPM_INT_ERR_AIE_NMU_CONFIG		0x4116U
/* Error during Handshake OPS indicating length of data is too large */
#define XPM_INT_ERR_UC_PRIVATE_DM_MAX_SIZE	0x4117U

/************************** PMC specific Errors ******************************/
/************************** (0x4200) - (0x42FF) ******************************/
/* Error during PMC-PL Isolation enable/disable */
#define XPM_INT_ERR_PMC_PL_ISO			0x4200U
/* Error during PMC-PL test Isolation enable/disable */
#define XPM_INT_ERR_PMC_PL_TEST_ISO		0x4201U
/* Error during VCCAUX SOC Isolation enable/disable */
#define XPM_INT_ERR_VCCAUX_SOC_ISO		0x4202U
/* Error during PMC rail control */
#define XPM_INT_ERR_PMC_RAIL_CONTROL		0X4203U

/************************** BISR specific errors *****************************/
/************************** (0x4300) - (0x43FF) ******************************/
/* BISR unsupported ID */
#define XPM_INT_ERR_BISR_UNSUPPORTED_ID		0x4300U
/* BISR done timeout */
#define XPM_INT_ERR_BISR_DONE_TIMEOUT		0x4301U
/* BISR PASS error */
#define XPM_INT_ERR_BISR_PASS			0x4302U
/* BISR trigger timeout */
#define XPM_INT_ERR_BISR_TRIGGER		0x4303U
/* Invalid BISR ID */
#define XPM_INT_ERR_BISR_INVALID_ID		0x4304U
/* Bad tag type */
#define XPM_INT_ERR_BAD_TAG_TYPE		0x4305U
/* Unknown tag ID */
#define XPM_INT_ERR_BISR_UNKN_TAG_ID		0x4306U

/*********************** Protection specific Errors **************************/
/************************** (0x4400) - (0x44FF) ******************************/
/* Invalid region */
#define XPM_INT_ERR_INVALID_REGION		0x4400U
/* Error during setup region */
#define XPM_INT_ERR_SETUP_REGION		0x4401U
/* Error during XPPU dynamic reconfig */
#define XPM_INT_ERR_XPPU_RECONFIG		0x4402U

/************************* Proc specific Errors ******************************/
/************************** (0x4500) - (0x45FF) ******************************/
/* Invalid resume address */
#define XPM_INT_ERR_INVALID_RESUME_ADDR		0x4500U
/* Error during core init */
#define XPM_INT_ERR_CORE_INIT			0x4501U

/************************ Subsys specific Errors *****************************/
/************************** (0x4600) - (0x46FF) ******************************/
/* Invalid subsystem ID */
#define XPM_INT_ERR_INVALID_SUBSYSTEMID		0x4600U
/* Subsystem not allowed to access clock */
#define XPM_INT_ERR_CLOCK_PERMISSION		0x4601U
/* Subsystem not allowed to access reset */
#define XPM_INT_ERR_RESET_PERMISSION		0x4602U
/* Subsystem not allowed to access pin */
#define XPM_INT_ERR_PIN_PERMISSION		0x4603U
/* Subsystem already added */
#define XPM_INT_ERR_SUBSYS_ADDED		0x4604U
/* Error during set subsystem state */
#define XPM_INT_ERR_SUBSYS_SET_STATE		0x4605U
/* Subsystem not allowed access */
#define  XPM_INT_ERR_SUBSYS_ACCESS		0x4606U

/************************* Power specific Errors *****************************/
/************************** (0x4700) - (0x47FF) ******************************/
/* Error during parent power up */
#define XPM_INT_ERR_PWR_PARENT_UP		0x4700U
/* Error during PSM power up request */
#define XPM_INT_ERR_PSM_PWR_UP			0x4701U
/* Error during PSM power down request */
#define XPM_INT_ERR_PSM_PWR_DWN			0x4702U

/*********************** Power rail specific Errors **************************/
/************************** (0x4800) - (0x48FF) ******************************/
/* Error adding Sysmon power good source */
#define XPM_INT_ERR_RAIL_SOURCE			0x4800U
/* Timeout while waiting for new data flag */
#define	XPM_INT_ERR_NEW_DATA_FLAG_TIMEOUT	0x4801U
#if defined (RAIL_CONTROL)
/* I2c initiliazation error */
#define XPM_INT_ERR_I2C_INIT			0x4802U
/* I2c write error */
#define XPM_INT_ERR_I2C_WRITE			0x4803U
#endif /* RAIL_CONTROL */
#if defined (VERSAL_DVS)
/* Failed to set voltage to upper limit */
#define XPM_INT_ERR_RAIL_UPPER_VOLT		0x4804U
/* Failed to set voltage to lower limit */
#define XPM_INT_ERR_RAIL_LOWER_VOLT		0x4805U
/* Controller regulating rail is disabled */
#define XPM_INT_ERR_RAIL_CONTROLLER_DISABLED	0x4806U
#endif /* VERSAL_DVS */
/* VID adjustment failed */
#define XPM_INT_ERR_VID_ADJUST			0x4807U
/* Invalid VID value */
#define XPM_INT_ERR_INVALID_VID			0x4808U
/* Rail control for VID adjustment failed */
#define XPM_INT_ERR_RAIL_VID			0x4809U

/*********************** PL Device specific Errors **************************/
/************************** (0x4900) - (0x497F) ******************************/
/* Error during pldevice init */
#define XPM_INT_ERR_PLDEVICE_INIT		0x4900U
/* Error during pldevice initnode command */
#define XPM_INT_ERR_PLDEVICE_INITNODE		0x4901U
/* Error during pldevice unlinking */
#define XPM_INT_ERR_PLDEVICE_UNLINK_FAIL	0x4902U
/* Error during pldevice unlinking */
#define XPM_INT_ERR_PLDEVICE_SET_BIT		0x4903U
/* Error during pldevice unlinking */
#define XPM_INT_ERR_PLDEVICE_UNSET_BIT		0x4904U
/* Error during pldevice unused to initializing transition */
#define XPM_INT_ERR_PLDEVICE_UNUSED_TO_INIT_EVT		0x4905U
/* Error during pldevice running to initializing transition */
#define XPM_INT_ERR_PLDEVICE_RUNNING_TO_INIT_EVT		0x4906U
/* Error during pldevice initializing to running transition */
#define XPM_INT_ERR_PLDEVICE_INIT_TO_RUNNING_EVT		0x4907U
/* Error during pldevice initializing to unused transition */
#define XPM_INT_ERR_PLDEVICE_INIT_TO_UNUSED_EVT		0x4908U
/* Error managing PLD Power Domain dependency */
#define XPM_INT_ERR_PLDEVICE_PWR_MANAGE			0x4909U
/* Invalid PL Device Parent State */
#define XPM_INT_ERR_INVALID_PLDEVICE_PARENT_STATE		0x490AU
/* Invalid PL Device Parent*/
#define XPM_INT_ERR_INVALID_PLDEVICE_PARENT			0x490BU
/* Unknown PL Power Bit Mask */
#define XPM_INT_ERR_PLDEVICE_INVALID_BITMASK			0x490CU
/* Error when RM is run for child when parent is in unused state */
#define XPM_INT_ERR_RUN_PARENT_IMAGE_FIRST			0x490DU
/* Error during init mem controller map for PLDx */
#define XPM_INT_ERR_PLDEVICE_FUNC_MEM_CTRLR_MAP			0x490EU
/* Error during mem controller request */
#define XPM_INT_ERR_PLDEVICE_MEM_CTRLR_REQUEST			0x490FU
/* Error during mem controller release */
#define XPM_INT_ERR_PLDEVICE_MEM_CTRLR_RELEASE			0x4910U

/*********************** AIE Device specific Errors **************************/
/************************** (0x4980) - (0x49FF) ******************************/
/* Uninitialized Init Node Ops field for AIE Device */
#define XPM_INT_ERR_AIE_UNDEF_INIT_NODE				0x4980U
/* Uninitialized Init Node Start function pointer for AIE Device */
#define XPM_INT_ERR_AIE_UNDEF_INIT_NODE_START			0x4981U
/* Uninitialized Init Node finish function pointer for AIE Device */
#define XPM_INT_ERR_AIE_UNDEF_INIT_NODE_FINISH			0x4982U
/* Error: PM_DEV_AIE is not present in topology */
#define XPM_INT_ERR_DEV_AIE					0x4983U
/* Error due to parentless AIE Device */
#define XPM_INT_ERR_INVALID_AIE_PARENT				0x4984U

/*********************** Register blind write check errors********************/
/************************** (0x5000) - (0x50FF) ******************************/
/* AIE NPI_PCSR_MASK write check error */
#define XPM_INT_ERR_REG_WRT_NPI_PCSR_MASK		0x5000U
/* AIE NPI_PCSR_CONTROL write check error */
#define XPM_INT_ERR_REG_WRT_NPI_PCSR_CONTROL		0x5001U

/* Error during PMC_ANALOG_OD_MBIST_RST register writing in LpdMbist */
#define XPM_INT_ERR_REG_WRT_LPDMBIST_RST		0x5002U
/* Error during PMC_ANALOG_OD_MBIST_SETUP register writing in LpdMbist */
#define XPM_INT_ERR_REG_WRT_LPDMBIST_SETUP		0x5003U
/* Error during PMC_ANALOG_OD_MBIST_PG_EN register writing in LpdMbist */
#define XPM_INT_ERR_REG_WRT_LPDMBIST_PGEN		0x5004U

/* Error during PMC_ANALOG_LBIST_RST_N	register writing in LpdLbist */
#define XPM_INT_ERR_REG_WRT_LPDLBIST_RST_N		0x5005U
/* Error during PMC_ANALOG_LBIST_ISOLATION_EN  register writing in LpdLbist */
#define XPM_INT_ERR_REG_WRT_LPDLBIST_ISO_EN		0x5006U
/* Error during PMC_ANALOG_LBIST_ENABLE register writing in LpdLbist */
#define XPM_INT_ERR_REG_WRT_LPDLBIST_ENABLE		0x5007U

/* Error during PMC_ANALOG_SCAN_CLEAR_TRIGGER register writing in LpdScanClear */
#define XPM_INT_ERR_REG_WRT_LPDLSCNCLR_TRIGGER		0x5008U

/* Error during PMC_ANALOG_SCAN_CLEAR_TRIGGER register writing in NpdScanClear */
#define XPM_INT_ERR_REG_WRT_NPDLSCNCLR_TRIGGER		0x5009U

/* Error during CFU_APB_CFU_MASK register writing in PlHouseClean */
#define XPM_INT_ERR_REG_WRT_PLHOUSECLN_CFU_MASK		0x500AU
/* Error during CFU_APB_CFU_FGCR register writing in PlHouseClean */
#define XPM_INT_ERR_REG_WRT_PLHOUSECLN_CFU_FGCR		0x500BU

/* Error during CPM_SLCR_SECURE_OD_MBIST_RST_N register writing in CpmMbistClear */
#define XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_RST	0x500CU
/* Error during CPM_SLCR_SECURE_MBIST_SETUP register writing in CpmMbistClear */
#define XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_SETUP	0x500DU
/* Error during CPM_SLCR_SECURE_MBIST_PG_EN register writing in CpmMbistClear */
#define XPM_INT_ERR_REG_WRT_CPMMBISTCLR_SLCRSECU_MBIST_PGEN	0x500EU

/* Error during GTY_PCSR_MASK register */
#define XPM_INT_ERR_REG_WRT_GTY_PCSR_MASK			0x500FU
/* Error during MEM_CLEAR_TRIGGER_MASK in GTY_PSCR_CONTROL register */
#define XPM_INT_ERR_REG_WRT_GTY_MEM_CLEAR_TRIGGER_MASK		0x5010U

/* Error during CPM5_GTY_PCSR_MASK register */
#define XPM_INT_ERR_REG_WRT_CPM5_GTY_PCSR_MASK			0x5011U
/* Error during MEM_CLEAR_TRIGGER_MASK in CPM5_GTY_PSCR_CONTROL register */
#define XPM_INT_ERR_REG_WRT_CPM5_GTY_MEM_CLEAR_TRIGGER_MASK	0x5012U

/* Error during XRAM_PCSR_MASK register */
#define XPM_INT_ERR_REG_WRT_XRAM_PCSR_MASK			0x5013U
/* Error during MEM_CLEAR_TRIGGER_MASK in XRAM_PSCR_CONTROL register */
#define XPM_INT_ERR_REG_WRT_XRAM_MEM_CLEAR_TRIGGER_0_MASK	0x5014U

/* Error during PSM_GLOBAL_MBIST_RST register writing in FpdMbistClear */
#define XPM_INT_ERR_REG_WRT_FPDMBISTCLR_RST		0x5015U
/* Error during PSM_GLOBAL_MBIST_SETUP register writing in FpdMbistClear */
#define XPM_INT_ERR_REG_WRT_FPDMBISTCLR_SETUP		0x5016U
/* Error during PSM_GLOBAL_MBIST_PG_EN register writing in FpdMbistClear */
#define XPM_INT_ERR_REG_WRT_FPDMBISTCLR_PGEN		0x5017U

/* Error during CPM_PCSR_MASK register writing in Cpm5ScanClear */
#define XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_MASK	0x5018U
/* Error during CPM_PCSR_PCR register writing in Cpm5ScanClear */
#define XPM_INT_ERR_REG_WRT_CPM5SCNCLR_PCSR_PCR		0x5019U

/* Error during CPM5_SLCR_SECURE register writing in Cpm5MbistClear */
#define XPM_INT_ERR_REG_WRT_CPM5MBISTCLR_SLCRSECU_MBIST_TRIGGER		0x501AU

/************************* Sysmon Specific Errors ****************************/
/************************** (0x5100) - (0x51FF) ******************************/
#define XPM_INT_ERR_DEVICE_NOT_SUPPORTED		0x5100U
/* Secure EFUSE transfer timeout */
#define XPM_INT_ERR_SECURE_EFUSE_DONE_TIMEOUT		0x5101U
/* Error during secure EFUSE transfer */
#define XPM_INT_ERR_SECURE_EFUSE_ERR			0x5102U
/* Error during disable PL SSC interface */
#define XPM_INT_ERR_DISABLE_PL_SSC_INTERFACE		0x5103U
/* Error during writing PCSR mask for SECURE_EFUSE_START */
#define XPM_INT_ERR_SECURE_EFUSE_START_PCSR_MASK	0x5104U
/* Error during de assert SECURE_EFUSE_START bit in PCSR_MASK */
#define XPM_INT_ERR_DE_ASSERT_SECURE_EFUSE_START_PCSR_MASK	0x5106U
/* Error during restore bypass state */
#define XPM_INT_ERR_RESTORE_BYPASS_STATE			0x5108U
/* Error in secure EFUSE done interrupt status clear */
#define XPM_INT_ERR_CLEAR_SECURE_EFUSE_DONE_ISR		0x5109U

/**************************** VDU Specific Errors ****************************/
/************************** (0x5200) - (0x52FF) ******************************/
/* Error releasing VDU_PL isolation */
#define XPM_INT_ERR_VDU_PL_ISO			0x5200U
/* Error running VDU BISR repair */
#define XPM_INT_ERR_VDU_BISR_REPAIR		0x5201U
/* Error running VDU ScanClear */
#define XPM_INT_ERR_VDU_SCAN_CLEAR		0x5202U
/* Error running VDU Mem Clear */
#define XPM_INT_ERR_VDU_MBIST			0x5203U
/* Error running VDU houseclean */
#define XPM_INT_ERR_VDU_HC			0x5204U
/* Error during VDU Init */
#define XPM_INT_ERR_VDU_INIT			0x5205U

/*************************** BFRB Specific Errors ****************************/
/************************** (0x5300) - (0x53FF) ******************************/
/* Error with BFRB SRAM power up */
#define XPM_INT_ERR_BFRB_POWER_STATE		0x5300U
/* Error during BFRB init */
#define XPM_INT_ERR_BFRB_INIT			0x5301U
/* Error running BFRB Mem Clear */
#define XPM_INT_ERR_BFRB_MBIST			0x5302U
/* Error running BFRB BISR repair */
#define XPM_INT_ERR_BFRB_BISR_REPAIR		0x5303U
/* Error running BFRB Scan Clear */
#define XPM_INT_ERR_BFRB_SCAN_CLEAR		0x5304U
/* Error running BFRB houseclean */
#define XPM_INT_ERR_BFRB_HC			0x5305U
/* Error when asserting BFRB INITSTATE */
#define XPM_INT_ERR_BFRB_INITSTATE_ASSERT	0x5306U

/*************************** ADC/DAC Specific Errors ****************************/
/************************** (0x5400) - (0x54FF) ******************************/
/* Error during ADC init */
#define XPM_INT_ERR_ADC_INIT			0x5400U
/* Error running DAC Mem Clear */
#define XPM_INT_ERR_DAC_MBIST			0x5401U
/* Error running DAC Scan Clear */
#define XPM_INT_ERR_DAC_SCAN_CLEAR		0x5402U
/* Error running ADC Scan Clear */
#define XPM_INT_ERR_ADC_SCAN_CLEAR		0x5403U
/* Error running ADC/DAC houseclean */
#define XPM_INT_ERR_ADC_DAC_HC			0x5404U
/* Error during DAC init */
#define XPM_INT_ERR_DAC_INIT			0x5405U
/* Error during DAC Ub Enable */
#define XPM_INT_ERR_SLEEP_UBLAZE		0x5406U
/* Error during unset ADC Test Safe */
#define XPM_INT_ERR_ADC_TEST_SAFE_UNSET		0x5407U
/* Error during unset DAC Test Safe */
#define XPM_INT_ERR_DAC_TEST_SAFE_UNSET		0x5408U
/* Error during DAC Ub Enable */
#define XPM_INT_ERR_DAC_UB_ENABLE		0x5409U
/* Error during resetting MB */
#define XPM_INT_ERR_RST_UBLAZE			0x540AU
/* Error during enabling clock for MB */
#define XPM_INT_ERR_CLK_UBLAZE			0x540BU
/* Error during releasing reset for MB */
#define XPM_INT_ERR_REL_RST_UBLAZE		0x540CU

/*************************** Undefined error *********************************/
#define XPM_INT_ERR_UNDEFINED			0xFFFFU

#define XPm_PrintDbgErr(Status, DbgErr)			\
	do {						\
		if (XST_SUCCESS != (Status)) {		\
			PmErr("0x%x\r\n", (DbgErr));	\
		}					\
	} while (XPM_FALSE_COND)

#endif /* XPM_DEBUG_H_ */
