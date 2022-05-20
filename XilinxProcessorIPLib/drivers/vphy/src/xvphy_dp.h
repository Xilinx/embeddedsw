/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvphy_hdmi.h
 *
 * The Xilinx Video PHY (VPHY) driver. This driver supports the Xilinx Video PHY
 * IP core.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   vkd  10/19/15 Initial release.
 * 1.7   gm   09/01/18 Added 8.1 Gbps support
 * </pre>
 *
 * @addtogroup xvphy_v1_11
 * @{
*******************************************************************************/

#ifndef XVPHY_DP_H_
/* Prevent circular inclusions by using protection macros. */
#define XVPHY_DP_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ******************************/

/* GTXE2. */
#define XVPHY_GTXE2_DIFF_SWING_DP_L0		0x03
#define XVPHY_GTXE2_DIFF_SWING_DP_L1		0x06
#define XVPHY_GTXE2_DIFF_SWING_DP_L2		0x09
#define XVPHY_GTXE2_DIFF_SWING_DP_L3		0x0F

#define XVPHY_GTXE2_PREEMP_DP_L0		0x00
#define XVPHY_GTXE2_PREEMP_DP_L1		0x0E
#define XVPHY_GTXE2_PREEMP_DP_L2		0x14
#define XVPHY_GTXE2_PREEMP_DP_L3		0x14

/* GTHE3. */
#define XVPHY_GTHE3_DIFF_SWING_DP_L0		0x03
#define XVPHY_GTHE3_DIFF_SWING_DP_L1		0x06
#define XVPHY_GTHE3_DIFF_SWING_DP_L2		0x09
#define XVPHY_GTHE3_DIFF_SWING_DP_L3		0x0F

#define XVPHY_GTHE3_PREEMP_DP_L0		0x00
#define XVPHY_GTHE3_PREEMP_DP_L1		0x0E
#define XVPHY_GTHE3_PREEMP_DP_L2		0x14
#define XVPHY_GTHE3_PREEMP_DP_L3		0x14

/* DP line rate coding. */
#define XVPHY_DP_LINK_BW_SET_162GBPS		0x06
#define XVPHY_DP_LINK_BW_SET_270GBPS		0x0A
#define XVPHY_DP_LINK_BW_SET_540GBPS		0x14
#define XVPHY_DP_LINK_BW_SET_810GBPS		0x1E

#define XVPHY_DP_LINK_RATE_HZ_162GBPS	1620000000LL
#define XVPHY_DP_LINK_RATE_HZ_270GBPS	2700000000LL
#define XVPHY_DP_LINK_RATE_HZ_540GBPS	5400000000LL
#define XVPHY_DP_LINK_RATE_HZ_810GBPS	8100000000LL
#define XVPHY_DP_LINK_RATE_HZ_10GBPS	10000000000LL

#define XVPHY_DP_REF_CLK_FREQ_HZ_162	 162000000LL
#define XVPHY_DP_REF_CLK_FREQ_HZ_135	 135000000LL
#define XVPHY_DP_REF_CLK_FREQ_HZ_81	  81000000LL
#define XVPHY_DP_REF_CLK_FREQ_HZ_270	 270000000LL

#ifdef __cplusplus
}
#endif

#endif /* XVPHY_HDMI_H_ */
/** @} */
