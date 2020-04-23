/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPFW_EVENTS_H_
#define XPFW_EVENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_default.h"
#define    XPFW_EV_MB_FAULT                1U
#define    XPFW_EV_APB_AIB_ERROR                2U
#define    XPFW_EV_AXI_AIB_ERROR                3U
#define    XPFW_EV_ERROR_2                4U
#define    XPFW_EV_ERROR_1                5U
#define    XPFW_EV_ACPU_3_DBG_PWRUP                6U
#define    XPFW_EV_ACPU_2_DBG_PWRUP                7U
#define    XPFW_EV_ACPU_1_DBG_PWRUP                8U
#define    XPFW_EV_ACPU_0_DBG_PWRUP                9U
#define    XPFW_EV_FPD_WAKE_GIC_PROXY                10U
#define    XPFW_EV_MIO_WAKE_5                11U
#define    XPFW_EV_MIO_WAKE_4                12U
#define    XPFW_EV_MIO_WAKE_3                13U
#define    XPFW_EV_MIO_WAKE_2                14U
#define    XPFW_EV_MIO_WAKE_1                15U
#define    XPFW_EV_MIO_WAKE_0                16U
#define    XPFW_EV_DAP_RPU_WAKE                17U
#define    XPFW_EV_DAP_FPD_WAKE                18U
#define    XPFW_EV_USB_1_WAKE                19U
#define    XPFW_EV_USB_0_WAKE                20U
#define    XPFW_EV_R5_1_WAKE                21U
#define    XPFW_EV_R5_0_WAKE                22U
#define    XPFW_EV_ACPU_3_WAKE                23U
#define    XPFW_EV_ACPU_2_WAKE                24U
#define    XPFW_EV_ACPU_1_WAKE                25U
#define    XPFW_EV_ACPU_0_WAKE                26U
#define    XPFW_EV_VCC_INT_FP_DISCONNECT                27U
#define    XPFW_EV_VCC_INT_DISCONNECT                28U
#define    XPFW_EV_VCC_AUX_DISCONNECT                29U
#define    XPFW_EV_DBG_ACPU3_RST_REQ                30U
#define    XPFW_EV_DBG_ACPU2_RST_REQ                31U
#define    XPFW_EV_DBG_ACPU1_RST_REQ                32U
#define    XPFW_EV_DBG_ACPU0_RST_REQ                33U
#define    XPFW_EV_CP_ACPU3_RST_REQ                34U
#define    XPFW_EV_CP_ACPU2_RST_REQ                35U
#define    XPFW_EV_CP_ACPU1_RST_REQ                36U
#define    XPFW_EV_CP_ACPU0_RST_REQ                37U
#define    XPFW_EV_DBG_RCPU1_RST_REQ                38U
#define    XPFW_EV_DBG_RCPU0_RST_REQ                39U
#define    XPFW_EV_R5_1_SLEEP                40U
#define    XPFW_EV_R5_0_SLEEP                41U
#define    XPFW_EV_ACPU_3_SLEEP                42U
#define    XPFW_EV_ACPU_2_SLEEP                43U
#define    XPFW_EV_ACPU_1_SLEEP                44U
#define    XPFW_EV_ACPU_0_SLEEP                45U
#define    XPFW_EV_PL_GPI_31                46U
#define    XPFW_EV_PL_GPI_30                47U
#define    XPFW_EV_PL_GPI_29                48U
#define    XPFW_EV_PL_GPI_28                49U
#define    XPFW_EV_PL_GPI_27                50U
#define    XPFW_EV_PL_GPI_26                51U
#define    XPFW_EV_PL_GPI_25                52U
#define    XPFW_EV_PL_GPI_24                53U
#define    XPFW_EV_PL_GPI_23                54U
#define    XPFW_EV_PL_GPI_22                55U
#define    XPFW_EV_PL_GPI_21                56U
#define    XPFW_EV_PL_GPI_20                57U
#define    XPFW_EV_PL_GPI_19                58U
#define    XPFW_EV_PL_GPI_18                59U
#define    XPFW_EV_PL_GPI_17                60U
#define    XPFW_EV_PL_GPI_16                61U
#define    XPFW_EV_PL_GPI_15                62U
#define    XPFW_EV_PL_GPI_14                63U
#define    XPFW_EV_PL_GPI_13                64U
#define    XPFW_EV_PL_GPI_12                65U
#define    XPFW_EV_PL_GPI_11                66U
#define    XPFW_EV_PL_GPI_10                67U
#define    XPFW_EV_PL_GPI_9                68U
#define    XPFW_EV_PL_GPI_8                69U
#define    XPFW_EV_PL_GPI_7                70U
#define    XPFW_EV_PL_GPI_6                71U
#define    XPFW_EV_PL_GPI_5                72U
#define    XPFW_EV_PL_GPI_4                73U
#define    XPFW_EV_PL_GPI_3                74U
#define    XPFW_EV_PL_GPI_2                75U
#define    XPFW_EV_PL_GPI_1                76U
#define    XPFW_EV_PL_GPI_0                77U
#define    XPFW_EV_RTC_SECONDS             78U
#define    XPFW_EV_RTC_ALARM               79U
#define    XPFW_EV_REQ_PWRUP               80U
#define    XPFW_EV_REQ_PWRDN               81U
#define    XPFW_EV_REQ_ISOLATION           82U

#define    XPFW_EV_MAX                     83U

#define XPFW_EV_GROUP_GPI0
#define XPFW_EV_GROUP_GPI1
#define XPFW_EV_GROUP_GPI2
#define XPFW_EV_GROUP_GPI3

#define XPFW_EV_GROUP_WAKE
#define XPFW_EV_GROUP_SLEEP
#define XPFW_EV_GROUP_WARM_RST
#define XPFW_EV_GROUP_DEBUG_PWRUP
#define XPFW_EV_GROUP_PWR_DISCONNECT
#define XPFW_EV_GROUP_SYS_ERROR

#define XPFW_EV_TYPE_INVALID	0xffffffffU
#define XPFW_EV_TYPE_GPI0	0U
#define XPFW_EV_TYPE_GPI1	1U
#define XPFW_EV_TYPE_GPI2	2U
#define XPFW_EV_TYPE_GPI3	3U
#define XPFW_EV_TYPE_RTC	4U
#define XPFW_EV_TYPE_GEN	5U

struct XPfw_Event_t {
	const u32 RegMask;
	u32 ModMask;
};

u32 XPfw_EventGetModMask(u32 EventId);
u32 XPfw_EventGetRegMask(u32 EventId);

XStatus XPfw_EventAddOwner(u8 ModId, u32 EventId);
XStatus XPfw_EventRemoveOwner(u32 ModId, u32 EventId);

u32 XPfw_EventGetType(u32 EventId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_EVENTS_H_ */
