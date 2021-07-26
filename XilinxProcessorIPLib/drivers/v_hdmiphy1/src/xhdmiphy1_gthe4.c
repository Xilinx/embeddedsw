/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_gthe4.c
 *
 * Contains a minimal set of functions for the XHdmiphy1 driver that allow
 * access to all of the Video PHY core's functionality. See xhdmiphy1.h for a
 * detailed description of the driver.
 *
 * @note    None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * 1.1   ku   17/05/20 Adding uniquification to avoid clash with vphy
 * 1.2   ssh  07/26/21 Added definitions for registers and masks
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xhdmiphy1_gt.h"
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static u8 XHdmiphy1_MToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
static u16 XHdmiphy1_NToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 NId);
static u8 XHdmiphy1_DToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
static u8 XHdmiphy1_DrpEncodeQpllMCpllMN2(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeCpllN1(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeCpllTxRxD(u8 AttrEncode);
static u16 XHdmiphy1_DrpEncodeQpllN(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeDataWidth(u8 AttrEncode);
static u8 XHdmiphy1_DrpEncodeIntDataWidth(u8 AttrEncode);
static u16 XHdmiphy1_DrpEncodeClk25(u32 RefClkFreqHz);

u32 XHdmiphy1_Gthe4CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XHdmiphy1_Gthe4OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_Gthe4ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId CmnId);
u32 XHdmiphy1_Gthe4RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4TxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gthe4RxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);

/************************** Constant Definitions ******************************/

/* DRP register space. */
/* Following DRP Registers have been identified that need to be updated
 * to get a reliable link for HDMI RX and TX
 * For more details please refer GT Userguide and/or GT Wiz
 */
#define XHDMIPHY1_DRP_RXCDR_CFG(n)       (0x0E + n)
#define XHDMIPHY1_DRP_RXCDR_CFG_GEN3(n)  (0xA2 + n)
#define XHDMIPHY1_DRP_RXCDR_CFG_GEN4(n)  (0x119 + n)

#define XDRP_GTHE4_CHN_REG_0028          0x0028
#define XDRP_GTHE4_CHN_REG_002A		     0x002A
#define XDRP_GTHE4_CHN_REG_00CB          0x00CB
#define XDRP_GTHE4_CHN_REG_00CC          0x00CC
#define XDRP_GTHE4_CHN_REG_00BC          0x00BC
#define XDRP_GTHE4_CHN_REG_0063          0x0063
#define XDRP_GTHE4_CHN_REG_006D          0x006D
#define XDRP_GTHE4_CHN_REG_007A          0x007A
#define XDRP_GTHE4_CHN_REG_007C          0x007C
#define XDRP_GTHE4_CHN_REG_000E		     0x000E
#define XDRP_GTHE4_CHN_REG_000F		     0x000F
#define XDRP_GTHE4_CHN_REG_0010		     0x0010
#define XDRP_GTHE4_CHN_REG_0011		     0x0011
#define XDRP_GTHE4_CHN_REG_0012		     0x0012
#define XDRP_GTHE4_CHN_REG_00AF		     0x00AF
#define XDRP_GTHE4_CHN_REG_0066			 0x0066
#define XDRP_GTHE4_CHN_REG_0003	       	 0x0003
#define XDRP_GTHE4_CHN_REG_0116          0x0116
#define XDRP_GTHE4_CHN_REG_00FB     	 0x00FB
#define XDRP_GTHE4_CHN_REG_009D          0x009D
#define XDRP_GTHE4_CHN_REG_0100          0x0100
#define XDRP_GTHE4_CHN_REG_003E     	 0x003E
#define XDRP_GTHE4_CHN_REG_0085   		 0x0085
#define XDRP_GTHE4_CHN_REG_007A       	 0x007A
#define XDRP_GTHE4_CHN_REG_0073          0x0073
#define XDRP_GTHE4_CHN_REG_00FF          0x00FF
#define XDRP_GTHE4_CHN_REG_009C     	 0x009C
#define XDRP_GTHE4_CHN_REG_00FB       	 0x00FB

#define XDRP_GTHE4_CMN_REG_0014	         0x0014
#define XDRP_GTHE4_CMN_REG_0018		   	 0x0018
#define XDRP_GTHE4_CMN_REG_0094        	 0x0094
#define XDRP_GTHE4_CMN_REG_0098		     0x0098
#define XDRP_GTHE4_CMN_REG_008D          0x008D
#define XDRP_GTHE4_CMN_REG_0016          0x0016
#define XDRP_GTHE4_CMN_REG_000D          0x000D
#define XDRP_GTHE4_CMN_REG_0096          0x0096
#define XDRP_GTHE4_CMN_REG_0019          0x0019
#define XDRP_GTHE4_CMN_REG_0099          0x0099
#define XDRP_GTHE4_CMN_REG_0030          0x0030
#define XDRP_GTHE4_CMN_REG_00B0          0x00B0

/* DRP data write. */
/* Following Data is written in to the DRP addresses.
 * To know the offsets, affected bit positions
 * and other details, please refer GT Userguide
 */

#define XDRP_GTHE4_CHN_REG_0063_RXOUT_DIV_MASK 			 0x07
#define XDRP_GTHE4_CHN_REG_0063_FLD_RXOUT_DIV_MASK 		 0x7
#define XDRP_GTHE4_CHN_REG_007C_TXOUT_DIV_MASK 			 0x700
#define XDRP_GTHE4_CHN_REG_007C_FLD_TX_RXDETECT_REF_MASK 0x7
#define XDRP_GTHE4_CHN_REG_0028_FLD_CPLL_FBDIV_MASK 	 0xFF
#define XDRP_GTHE4_CHN_REG_0028_FLD_CPLL_FBDIV_45_MASK	 0x1
#define XDRP_GTHE4_CHN_REG_002A_FLD_A_TXDIFFCTRL_MASK 	 0x1F
#define XDRP_GTHE4_CHN_REG_0028_CPLL_FBDIV_MASK      	 0xFF80
#define XDRP_GTHE4_CHN_REG_002A_CPLL_REFCLK_DIV_MASK 	 0xF800
#define XDRP_GTHE4_CMN_REG_0014_FLD_QPLL0_INIT_CFG1_MASK 0xFF
#define XDRP_GTHE4_CMN_REG_0018_QPLLx_REFCLK_DIV_MASK 	 0xF80
#define XDRP_GTHE4_CMN_REG_0018_QPLLx_REFCLK_DIV_MASK1	 0x1F
#define XDRP_GTHE4_CHN_REG_00AF_RXCDR_CGF2_GEN2_MASK	 0x3FF
#define XDRP_GTHE4_CHN_REG_0011_RXCDR_CGF3_GEN2_MASK	 0x3F
#define XDRP_GTHE4_CHN_REG_0066_RX_WIDEMODE_CDR_MASK	 0xC
#define XDRP_GTHE4_CMN_REG_000D_PPFx_CFG_MASK 			 0x0FC0
#define XDRP_GTHE4_CMN_REG_0019_QPLLx_LPF_MASK 			 0x0003
#define XDRP_GTHE4_CMN_REG_0030_QPLLx_CFG4_MASK 		 0x00E7
#define XDRP_GTHE4_CHN_REG_0066_RX_INT_DATAWIDTH_MASK 	 0xF
#define XDRP_GTHE4_CHN_REG_0003_RX_DATAWIDTH_MASK 		 0x1E0
#define XDRP_GTHE4_CHN_REG_0116_CH_RX_HSPMUX_MASK 		 0x00FF
#define XDRP_GTHE4_CHN_REG_00FB_PREIQ_FREQ_BST_MASK 	 0x0030
#define XDRP_GTHE4_CHN_REG_00FB_TXPI_BIASSET_MASK 		 0x0006
#define XDRP_GTHE4_CHN_REG_009C_TXPI_CFG3_CFG4_MASK 	 0x0060
#define XDRP_GTHE4_CHN_REG_0116_CH_TX_HSPMUX_MASK 		 0xFF00
#define XDRP_GTHE4_CHN_REG_007A_TXCLK25_MASK 			 0xF800
#define XDRP_GTHE4_CHN_REG_006D_RXCLK25_MASK 			 0x00F8
#define XDRP_GTHE4_CHN_REG_0066_RX_WIDEMODE_CDR_MASK_VAL 0x3
#define XDRP_GTHE4_CHN_REG_007A_TX_DATA_WIDTH_MASK		 0xF


#define XHDMIPHY1_RXCDR_CFG_WORD0 				0x0000
#define XHDMIPHY1_RXCDR_CFG_WORD1 				0x0000
#define XHDMIPHY1_RXCDR_CFG_WORD2 				0x0262
#define XHDMIPHY1_RXCDR_CFG_WORD2_RXDIV 		0x10
#define XHDMIPHY1_RXCDR_CFG_WORD3 				0x0000
#define XHDMIPHY1_RXCDR_CFG_WORD4 				0x0000
#define XHDMIPHY1_DRP_TXOUT_OFFSET 				8
#define XHDMIPHY1_DRP_CPLL_VCO_RANGE1      		3000
#define XHDMIPHY1_DRP_CPLL_VCO_RANGE2      		4250
#define XHDMIPHY1_DRP_CPLL_CFG0_VAL1      		0x01FA
#define XHDMIPHY1_DRP_CPLL_CFG0_VAL2      		0x0FFA
#define XHDMIPHY1_DRP_CPLL_CFG0_VAL3      		0x03FE
#define XHDMIPHY1_DRP_CPLL_CFG1_VAL1      		0x0023
#define XHDMIPHY1_DRP_CPLL_CFG1_VAL2      		0x0021
#define XHDMIPHY1_DRP_CPLL_CFG2_VAL1      		0x0002
#define XHDMIPHY1_DRP_CPLL_CFG2_VAL2      		0x0202
#define XHDMIPHY1_DRP_CPLL_CFG2_VAL3      		0x0203
#define XHDMIPHY1_DRP_QPLLx_VCO_RANGE1 			15000
#define XHDMIPHY1_DRP_QPLLx_VCO_RANGE2 			13000
#define XHDMIPHY1_DRP_QPLLx_VCO_RANGE3 			11000
#define XHDMIPHY1_DRP_QPLLx_VCO_RANGE4 			7000
#define XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL1 	0x0E00
#define XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL2 	0x0800
#define XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL3 	0x0600
#define XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL4 	0x0400
#define XHDMIPHY1_DRP_PPF_MUX_TERM_CTRL0_VAL1 	0x0100
#define XHDMIPHY1_DRP_PPF_MUX_TERM_CTRL0_VAL2 	0x0000
#define XHDMIPHY1_DRP_QPLLx_CP_VAL1 			0x007F
#define XHDMIPHY1_DRP_QPLLx_CP_VAL2 			0x03FF
#define XHDMIPHY1_DRP_QPLLx_LPF_VAL1 			0x3
#define XHDMIPHY1_DRP_QPLLx_LPF_VAL2 			0x1
#define XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE1 		7500
#define XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE2 		3500
#define XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE3 		5500
#define XHDMIPHY1_DRP_Q_TERM_CLK_VAL1 			0x2
#define XHDMIPHY1_DRP_Q_TERM_CLK_VAL2 			0x0
#define XHDMIPHY1_DRP_Q_TERM_CLK_VAL3 			0x6
#define XHDMIPHY1_DRP_Q_DCRNT_CLK_VAL1 			0x5
#define XHDMIPHY1_DRP_Q_DCRNT_CLK_VAL2 			0x4
#define XHDMIPHY1_DRP_Q_DCRNT_CLK_VAL3 			0x3
#define XHDMIPHY1_DRP_LINERATEKHZ_1 			16400000
#define XHDMIPHY1_DRP_LINERATEKHZ_2 			10400000
#define XHDMIPHY1_DRP_LINERATEKHZ_3 			10000000
#define XHDMIPHY1_DRP_LINERATEKHZ_4 			20000000
#define XHDMIPHY1_DRP_LINERATEKHZ_5 			16375000
#define XHDMIPHY1_DRP_LINERATEKHZ_6 			8000000
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL1 		0x0010
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL2 		0x0018
#define XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL3 		0x0012
#define XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL1 		3
#define XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL2 		2
#define XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL3 		1
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE1 		7500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE2 		3500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE3 		5500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE4 		14110
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE5 		14000
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE6 		10000
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE7 		6000
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE8 		7000
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE9 		6500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10 		5500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE11 		5156
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE12 		4500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE13 		4000
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE14 		3500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE15 		3000
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE16 		2500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE17 		7500
#define XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE18 		2000
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL1 			0x0004
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL2 			0x0104
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL3 			0x2004
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL4 			0x0002
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL5 			0x0102
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL6 			0x2102
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL7 			0x2202
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL8 			0x0200
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL9 			0x1300
#define XHDMIPHY1_DRP_RXPI_CFG0_VAL10 			0x3300
#define XHDMIPHY1_DRP_RXPI_CFG1_VAL1 			0x0000
#define XHDMIPHY1_DRP_RXPI_CFG1_VAL2 			0x0015
#define XHDMIPHY1_DRP_RXPI_CFG1_VAL3 			0x0045
#define XHDMIPHY1_DRP_RXPI_CFG1_VAL4 			0x00FD
#define XHDMIPHY1_DRP_RXPI_CFG1_VAL5 			0x00FF
#define XHDMIPHY1_DRP_TXPH_CFG_VAL1 			0x0723
#define XHDMIPHY1_DRP_TXPH_CFG_VAL2 			0x0323
#define XHDMIPHY1_DRP_TXPI_CFG_VAL1 			0x0000
#define XHDMIPHY1_DRP_TXPI_CFG_VAL2 			0x0054
#define XHDMIPHY1_DRP_TXPI_CFG_VAL3 			0x03DF
#define XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL1 		0x0
#define XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL2 		0x1
#define XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL3 		0x2
#define XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL4 		0x3
#define XHDMIPHY1_DRP_TXPI_BIASSET_VAL1 		3
#define XHDMIPHY1_DRP_TXPI_BIASSET_VAL2 		2
#define XHDMIPHY1_DRP_TXPI_BIASSET_VAL3 		1
#define XHDMIPHY1_DRP_CH_HSPMUX_VAL1 			0x68
#define XHDMIPHY1_DRP_CH_HSPMUX_VAL2 			0x44
#define XHDMIPHY1_DRP_CH_HSPMUX_VAL3 			0x24
#define XHDMIPHY1_DRP_CH_HSPMUX_VAL4 			0x3C
#define XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL1		2
#define XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL2		1
#define XHDMIPHY1_DRP_PLLx_CLKOUT_VAL1			0x68
#define XHDMIPHY1_DRP_PLLx_CLKOUT_VAL2			0x44
#define XHDMIPHY1_DRP_PLLx_CLKOUT_VAL3			0x24
#define XHDMIPHY1_DRP_PLLx_CLKOUT_VAL4			0x3C


/* PLL operating ranges. */
#define XHDMIPHY1_QPLL0_MIN       9800000000LL
#define XHDMIPHY1_QPLL0_MAX      16375000000LL
#define XHDMIPHY1_QPLL1_MIN       8000000000LL
#define XHDMIPHY1_QPLL1_MAX      13000000000LL
#define XHDMIPHY1_CPLL_MIN        2000000000LL
#define XHDMIPHY1_CPLL_MAX        6250000000LL

const u8 XHdmiphy1_Gthe4CpllDivsM[]   = {1, 2, 0};
const u8 XHdmiphy1_Gthe4CpllDivsN1[]  = {4, 5, 0};
const u8 XHdmiphy1_Gthe4CpllDivsN2[]  = {1, 2, 3, 4, 5, 8, 0};
const u8 XHdmiphy1_Gthe4CpllDivsD[]   = {1, 2, 4, 8, 0};

const u8 XHdmiphy1_Gthe4QpllDivsM[]   = {1, 2, 3, 4, 0};
const u8 XHdmiphy1_Gthe4QpllDivsN1[]  = {16, 20, 25, 30, 32, 40, 60, 64, 66, 75, 80, 84,
                   90, 96, 100, 112, 120, 125, 150, 160, 0};
const u8 XHdmiphy1_Gthe4QpllDivsN2[]  = {1, 0};
const u8 XHdmiphy1_Gthe4QpllDivsD[]   = {1, 2, 4, 8, 16, 0};

const XHdmiphy1_GtConfig XHdmiphy1_Gthe4Config = {
    .CfgSetCdr = XHdmiphy1_Gthe4CfgSetCdr,
    .CheckPllOpRange = XHdmiphy1_Gthe4CheckPllOpRange,
    .OutDivChReconfig = XHdmiphy1_Gthe4OutDivChReconfig,
    .ClkChReconfig = XHdmiphy1_Gthe4ClkChReconfig,
    .ClkCmnReconfig = XHdmiphy1_Gthe4ClkCmnReconfig,
    .RxChReconfig = XHdmiphy1_Gthe4RxChReconfig,
    .TxChReconfig = XHdmiphy1_Gthe4TxChReconfig,

    .CpllDivs = {
        .M = XHdmiphy1_Gthe4CpllDivsM,
        .N1 = XHdmiphy1_Gthe4CpllDivsN1,
        .N2 = XHdmiphy1_Gthe4CpllDivsN2,
        .D = XHdmiphy1_Gthe4CpllDivsD,
    },
    .QpllDivs = {
        .M = XHdmiphy1_Gthe4QpllDivsM,
        .N1 = XHdmiphy1_Gthe4QpllDivsN1,
        .N2 = XHdmiphy1_Gthe4QpllDivsN2,
        .D = XHdmiphy1_Gthe4QpllDivsD,
    },
};

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will set the clock and data recovery (CDR) values for a given
* channel.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u8 RxOutDiv;
    u32 Status = XST_SUCCESS;

    /* Set CDR values only for CPLLs. */
    if ((ChId < XHDMIPHY1_CHANNEL_ID_CH1) ||
        (ChId > XHDMIPHY1_CHANNEL_ID_CH4)) {
        return XST_FAILURE;
    }

    ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)];
    RxOutDiv = ChPtr->RxOutDiv;
    ChPtr->PllParams.Cdr[0] = XHDMIPHY1_RXCDR_CFG_WORD0;
    ChPtr->PllParams.Cdr[1] = XHDMIPHY1_RXCDR_CFG_WORD1;
    ChPtr->PllParams.Cdr[3] = XHDMIPHY1_RXCDR_CFG_WORD3;
    ChPtr->PllParams.Cdr[4] = XHDMIPHY1_RXCDR_CFG_WORD4;
    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
        /* RxOutDiv = 1  => Cdr[2] = 0x0262
         * RxOutDiv = 2  => Cdr[2] = 0x0252
         * RxOutDiv = 4  => Cdr[2] = 0x0242
         * RxOutDiv = 8  => Cdr[2] = 0x0232
         * RxOutDiv = 16 => Cdr[2] = 0x0222 */

        ChPtr->PllParams.Cdr[2] = XHDMIPHY1_RXCDR_CFG_WORD2;

        while (RxOutDiv >>= 1) {
            ChPtr->PllParams.Cdr[2] -= XHDMIPHY1_RXCDR_CFG_WORD2_RXDIV;
        }
    }
    else {
        Status = XST_FAILURE;
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will check if a given PLL output frequency is within the
* operating range of the PLL for the GT type.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    PllClkOutFreqHz is the frequency to check.
*
* @return
*       - XST_SUCCESS if the frequency resides within the PLL's range.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 PllClkOutFreqHz)
{
    u32 Status = XST_FAILURE;

    /* Suppress Warning Messages */
    InstancePtr = InstancePtr;
    QuadId = QuadId;

    if (((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) &&
            (XHDMIPHY1_QPLL0_MIN <= PllClkOutFreqHz) &&
            (PllClkOutFreqHz <= XHDMIPHY1_QPLL0_MAX)) ||
        ((ChId == XHDMIPHY1_CHANNEL_ID_CMN1) &&
            (XHDMIPHY1_QPLL1_MIN <= PllClkOutFreqHz) &&
            (PllClkOutFreqHz <= XHDMIPHY1_QPLL1_MAX)) ||
        ((ChId >= XHDMIPHY1_CHANNEL_ID_CH1) &&
            (ChId <= XHDMIPHY1_CHANNEL_ID_CH4) &&
            (XHDMIPHY1_CPLL_MIN <= PllClkOutFreqHz) &&
            (PllClkOutFreqHz <= XHDMIPHY1_CPLL_MAX))) {
        Status = XST_SUCCESS;
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will set the output divider logic for a given channel.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    Dir is an indicator for RX or TX.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
    u16 DrpVal;
    u16 WriteVal;
    u32 Status = XST_SUCCESS;

    if (Dir == XHDMIPHY1_DIR_RX) {
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0063, &DrpVal);
        /* Mask out RXOUT_DIV. */
        DrpVal &= ~XDRP_GTHE4_CHN_REG_0063_RXOUT_DIV_MASK;
        /* Set RXOUT_DIV. */
        WriteVal = (XHdmiphy1_DToDrpEncoding(InstancePtr, QuadId, ChId,
                        XHDMIPHY1_DIR_RX) & XDRP_GTHE4_CHN_REG_0063_FLD_RXOUT_DIV_MASK);
        DrpVal |= WriteVal;
        /* Write new DRP register value for RX dividers. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0063, DrpVal);
    }
    else {
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_007C, &DrpVal);
        /* Mask out TXOUT_DIV. */
        DrpVal &= ~XDRP_GTHE4_CHN_REG_007C_TXOUT_DIV_MASK;
        /* Set TXOUT_DIV. */
        WriteVal = (XHdmiphy1_DToDrpEncoding(InstancePtr, QuadId, ChId,
                        XHDMIPHY1_DIR_TX) & XDRP_GTHE4_CHN_REG_007C_FLD_TX_RXDETECT_REF_MASK);
        DrpVal |= (WriteVal << XHDMIPHY1_DRP_TXOUT_OFFSET);
        /* Write new DRP register value for RX dividers. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_007C, DrpVal);
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel clock settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u16 WriteVal;
    u32 CpllxVcoRateMHz;
    u32 Status = XST_SUCCESS;

    /* Obtain current DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0028, &DrpVal);
    /* Mask out clock divider bits. */
    DrpVal &= ~(XDRP_GTHE4_CHN_REG_0028_CPLL_FBDIV_MASK);
    /* Set CPLL_FBDIV. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId, ChId, 2) & XDRP_GTHE4_CHN_REG_0028_FLD_CPLL_FBDIV_MASK);
    DrpVal |= (WriteVal << 8);
    /* Set CPLL_FBDIV_45. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId, ChId, 1) & XDRP_GTHE4_CHN_REG_0028_FLD_CPLL_FBDIV_45_MASK);
    DrpVal |= (WriteVal << 7);
    /* Write new DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0028, DrpVal);
    /* Write CPLL Ref Clk Div. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_002A, &DrpVal);
    /* Mask out clock divider bits. */
    DrpVal &= ~(XDRP_GTHE4_CHN_REG_002A_CPLL_REFCLK_DIV_MASK);
    /* Set CPLL_REFCLKDIV. */
    WriteVal = (XHdmiphy1_MToDrpEncoding(InstancePtr, QuadId, ChId) & XDRP_GTHE4_CHN_REG_002A_FLD_A_TXDIFFCTRL_MASK);
    DrpVal |= (WriteVal << 11);
    /* Write new DRP register value for PLL dividers. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_002A, DrpVal);

    CpllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, ChId,
                        XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
                            XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX) / 1000000;

    /* CPLL_CFG0 */
    if (CpllxVcoRateMHz <= XHDMIPHY1_DRP_CPLL_VCO_RANGE1) {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG0_VAL1;
    }
    else if (CpllxVcoRateMHz <= XHDMIPHY1_DRP_CPLL_VCO_RANGE2) {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG0_VAL2;
    }
    else {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG0_VAL3;
    }
    /* Write new DRP register value for CPLL_CFG0. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00CB, DrpVal);

    /* CPLL_CFG1 */
    if (CpllxVcoRateMHz <= XHDMIPHY1_DRP_CPLL_VCO_RANGE1) {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG1_VAL1;
    }
    else {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG1_VAL2;
    }
    /* Write new DRP register value for CPLL_CFG1. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00CC, DrpVal);
    /* CPLL_CFG2 */
    if (CpllxVcoRateMHz <= XHDMIPHY1_DRP_CPLL_VCO_RANGE1) {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG2_VAL1;
    }
    else if (CpllxVcoRateMHz <= XHDMIPHY1_DRP_CPLL_VCO_RANGE2) {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG2_VAL2;
    }
    else {
        DrpVal = XHDMIPHY1_DRP_CPLL_CFG2_VAL3;
    }
    /* Write new DRP register value for CPLL_CFG2. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00BC, DrpVal);

    /* Configure CPLL Calibration Registers */
    XHdmiphy1_CfgCpllCalPeriodandTol(InstancePtr, QuadId, ChId,
            (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) ?
                XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX),
            InstancePtr->Config.DrpClkFreq);

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the common channel clock settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    CmnId is the common channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId CmnId)
{
    u16 DrpVal;
    u16 WriteVal;
    u32 QpllxVcoRateMHz;
    u32 QpllxClkOutMHz;
    u32 Status = XST_SUCCESS;

    /* Obtain current DRP register value for QPLLx_FBDIV. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0014 : XDRP_GTHE4_CMN_REG_0094, &DrpVal);
    /* Mask out QPLLx_FBDIV. */
    DrpVal &= ~(XDRP_GTHE4_CMN_REG_0014_FLD_QPLL0_INIT_CFG1_MASK);
    /* Set QPLLx_FBDIV. */
    WriteVal = (XHdmiphy1_NToDrpEncoding(InstancePtr, QuadId, CmnId, 0) &
                    XDRP_GTHE4_CMN_REG_0014_FLD_QPLL0_INIT_CFG1_MASK);
    DrpVal |= WriteVal;
    /* Write new DRP register value for QPLLx_FBDIV. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0014 : XDRP_GTHE4_CMN_REG_0094, DrpVal);

    /* Obtain current DRP register value for QPLLx_REFCLK_DIV. */
    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0018 : XDRP_GTHE4_CMN_REG_0098, &DrpVal);
    /* Mask out QPLLx_REFCLK_DIV. */
    DrpVal &= ~(XDRP_GTHE4_CMN_REG_0018_QPLLx_REFCLK_DIV_MASK);
    /* Disable Intelligent Reference Clock Selection */
    if (XHdmiphy1_GetRefClkSourcesCount(InstancePtr) > 1) {
        DrpVal |= (1 << 6);
    }
    /* Set QPLLx_REFCLK_DIV. */
    WriteVal = (XHdmiphy1_MToDrpEncoding(InstancePtr, QuadId, CmnId) & XDRP_GTHE4_CMN_REG_0018_QPLLx_REFCLK_DIV_MASK1);
    DrpVal |= (WriteVal << 7);
    /* Write new DRP register value for QPLLx_REFCLK_DIV. */
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMN,
                (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0018 : XDRP_GTHE4_CMN_REG_0098, DrpVal);

    if ((XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) ||
        (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX))) {

        QpllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, CmnId,
                        XHdmiphy1_IsTxUsingQpll(InstancePtr, QuadId, CmnId) ?
                        XHDMIPHY1_DIR_TX : XHDMIPHY1_DIR_RX) / 1000000;
        QpllxClkOutMHz = QpllxVcoRateMHz / 2;

        /* PPFx_CFG */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_000D : XDRP_GTHE4_CMN_REG_008D,
                    &DrpVal);

        DrpVal &= ~(XDRP_GTHE4_CMN_REG_000D_PPFx_CFG_MASK);
        /* PPF_MUX_CRNT_CTRL0 */
        if (QpllxVcoRateMHz >= XHDMIPHY1_DRP_QPLLx_VCO_RANGE1) {
            DrpVal |= XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL1;
        }
        else if (QpllxVcoRateMHz >= XHDMIPHY1_DRP_QPLLx_VCO_RANGE3) {
            DrpVal |= XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL2;
        }
        else if (QpllxVcoRateMHz >= XHDMIPHY1_DRP_QPLLx_VCO_RANGE4) {
            DrpVal |= XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL3;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_PPF_MUX_CRNT_CTRL0_VAL4;
        }
        /* PPF_MUX_TERM_CTRL0 */
        if (QpllxVcoRateMHz >= XHDMIPHY1_DRP_QPLLx_VCO_RANGE2) {
            DrpVal |= XHDMIPHY1_DRP_PPF_MUX_TERM_CTRL0_VAL1;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_PPF_MUX_TERM_CTRL0_VAL2;
        }

        /* Write new DRP register value for PPFx_CFG. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_000D : XDRP_GTHE4_CMN_REG_008D,
                    DrpVal);
        /* QPLLx_CP */
        if (InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                PllParams.NFbDiv <= 40) {
            DrpVal = XHDMIPHY1_DRP_QPLLx_CP_VAL1;
        }
        else {
            DrpVal = XHDMIPHY1_DRP_QPLLx_CP_VAL2;
        }
        /* Write new DRP register value for QPLLx_CP. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0016 : XDRP_GTHE4_CMN_REG_0096,
                    DrpVal);

        /* QPLLx_LPF */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0019 : XDRP_GTHE4_CMN_REG_0099,
                    &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CMN_REG_0019_QPLLx_LPF_MASK);
        if (InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                PllParams.NFbDiv <= 40) {
            DrpVal |= XHDMIPHY1_DRP_QPLLx_LPF_VAL1;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_QPLLx_LPF_VAL2;
        }

        /* Write new DRP register value for QPLLx_LPF. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0019 : XDRP_GTHE4_CMN_REG_0099,
                    DrpVal);

        /* QPLLx_CFG4 */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0030 : XDRP_GTHE4_CMN_REG_00B0,
                    &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CMN_REG_0030_QPLLx_CFG4_MASK);
        /* Q_TERM_CLK */
        if (QpllxClkOutMHz >= XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE1) {
            DrpVal |= XHDMIPHY1_DRP_Q_TERM_CLK_VAL1 << 5;
        }
        else if (QpllxClkOutMHz >= XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE2) {
            DrpVal |= XHDMIPHY1_DRP_Q_TERM_CLK_VAL2 << 5;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_Q_TERM_CLK_VAL3 << 5;
        }
        /* Q_DCRNT_CLK */
        if (QpllxClkOutMHz >= XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE1) {
            DrpVal |= XHDMIPHY1_DRP_Q_DCRNT_CLK_VAL1;
        }
        else if (QpllxClkOutMHz >= XHDMIPHY1_DRP_QPLLx_CLKOUT_RANGE3) {
            DrpVal |= XHDMIPHY1_DRP_Q_DCRNT_CLK_VAL2;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_Q_DCRNT_CLK_VAL3;
        }
        /* Write new DRP register value for QPLLx_CFG4. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN,
                    (CmnId == XHDMIPHY1_CHANNEL_ID_CMN0) ? XDRP_GTHE4_CMN_REG_0030 : XDRP_GTHE4_CMN_REG_00B0,
                    DrpVal);
    }

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's RX settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u16 DrpVal;
    u16 WriteVal;
    u8 CfgIndex;
    XHdmiphy1_ChannelId ChIdPll;
    XHdmiphy1_PllType PllType;
    u32 PllxVcoRateMHz;
    u32 PllxClkOutMHz;
    u32 PllxClkOutDiv;
    u64 LineRateKHz;
    u32 Status = XST_SUCCESS;

    ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)];

    /* RXCDR_CFG(CfgIndex) */
    for (CfgIndex = 0; CfgIndex < 5; CfgIndex++) {
        DrpVal = ChPtr->PllParams.Cdr[CfgIndex];
        if (!DrpVal) {
            /* Don't modify RX_CDR configuration. */
            continue;
        }
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId,
                XHDMIPHY1_DRP_RXCDR_CFG(CfgIndex), DrpVal);
        if (CfgIndex == 2) {
            Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId,
                    XHDMIPHY1_DRP_RXCDR_CFG_GEN3(CfgIndex), DrpVal);

        }
    }

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
        /* Determine PLL type. */
        PllType = XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX,
                    ChId);
        /* Determine which channel(s) to operate on. */
        switch (PllType) {
            case XHDMIPHY1_PLL_TYPE_QPLL:
            case XHDMIPHY1_PLL_TYPE_QPLL0:
                ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN0;
                PllxClkOutDiv = XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL1;
                break;
            case XHDMIPHY1_PLL_TYPE_QPLL1:
                ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN1;
                PllxClkOutDiv = XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL1;
                break;
            default:
                ChIdPll = ChId;
                PllxClkOutDiv = XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL2;
                break;
        }

	LineRateKHz = XHdmiphy1_GetLineRateHz(InstancePtr,
					      QuadId, ChIdPll) / 1000;

	/* RXCDR_CFG3 & RXCDR_CFG3_GEN3 */
	if(LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_1) {
		DrpVal  = XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL1;
	} else if((LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_2) && (ChPtr->RxDataWidth == 64)) {
		DrpVal  = XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL2;
	} else if(LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_3) {
		DrpVal  = XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL1;
	} else {
		DrpVal  = XHDMIPHY1_DRP_RXCDR_CFG_WORD3_VAL3;
	}
	/* Write RXCDR_CFG3 Value */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0011, DrpVal);
		/* Write RXCDR_CFG3_GEN3 Value */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_DRP_RXCDR_CFG_GEN3(3), DrpVal);

        /* RXCDR_CFG2_GEN2 & RXCDR_CFG3_GEN2 */
        /* Get [15:10] from RXCDR_CFG3[5:0] */
        DrpVal = (DrpVal & XDRP_GTHE4_CHN_REG_0011_RXCDR_CGF3_GEN2_MASK) << 10;
        /* Get [9:0] from RXCDR_CFG2[9:0] */
        DrpVal &= ~ XDRP_GTHE4_CHN_REG_00AF_RXCDR_CGF2_GEN2_MASK;
        DrpVal |= ChPtr->PllParams.Cdr[2] & XDRP_GTHE4_CHN_REG_00AF_RXCDR_CGF2_GEN2_MASK;
	/* Write RXCDR_CFG2_GEN2 & RXCDR_CFG3_GEN2 Value */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00AF, DrpVal);

        /* RX_WIDEMODE_CDR Encoding */
	switch (ChPtr->RxDataWidth) {
	case 80:
		if(LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_4) {
			WriteVal  = 0x2 << 2;
		} else {
			WriteVal  = 0x1 << 2;
		}
		break;
	case 64:
		if(LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_5) {
			WriteVal = 0x2 << 2;
		} else {
			WriteVal = 0x1 << 2;
		}
		break;
	case 40:
		if(LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_3) {
			WriteVal = 0x1 << 2;
		} else {
			WriteVal = 0x0;
		}
		break;
	case 32:
		if(LineRateKHz > XHDMIPHY1_DRP_LINERATEKHZ_6) {
			WriteVal = 0x1 << 2;
		} else {
			WriteVal = 0x0;
		}
		break;
	default:
		WriteVal = 0x0;
		break;
	}
        /* RX_INT_DATAWIDTH & RX_WIDEMODE_CDR */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0066, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_0066_RX_INT_DATAWIDTH_MASK);
        /* Update RX_WIDEMODE_CDR Value */
        DrpVal |= WriteVal & XDRP_GTHE4_CHN_REG_0066_RX_WIDEMODE_CDR_MASK;
        WriteVal = (XHdmiphy1_DrpEncodeIntDataWidth(ChPtr->RxIntDataWidth) &
			XDRP_GTHE4_CHN_REG_0066_RX_WIDEMODE_CDR_MASK_VAL);
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0066, DrpVal);

        /* RX_DATA_WIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0003, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_0003_RX_DATAWIDTH_MASK);
        WriteVal = (XHdmiphy1_DrpEncodeDataWidth(ChPtr->RxDataWidth) & 0xF);
        WriteVal <<= 5;
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0003, DrpVal);

        PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId,
                            ChIdPll, XHDMIPHY1_DIR_RX) / 1000000;
        PllxClkOutMHz = PllxVcoRateMHz / PllxClkOutDiv;
        /* CH_HSPMUX_RX */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0116, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_0116_CH_RX_HSPMUX_MASK);
        if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE1) {
            DrpVal |= XHDMIPHY1_DRP_PLLx_CLKOUT_VAL1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE3) {
            DrpVal |= XHDMIPHY1_DRP_PLLx_CLKOUT_VAL2;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE2) {
            DrpVal |= XHDMIPHY1_DRP_PLLx_CLKOUT_VAL3;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_PLLx_CLKOUT_VAL4;
        }
        /* Write new DRP register value for CH_HSPMUX_RX. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0116, DrpVal);
        /* PREIQ_FREQ_BST */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00FB, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_00FB_PREIQ_FREQ_BST_MASK);
        if (PllxClkOutMHz > XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE4) {
            DrpVal |= XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL1 << 4;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE5) {
            DrpVal |= XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL2 << 4; /* LPM Mode */
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE6) {
            DrpVal |= XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL2 << 4;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE7) {
            DrpVal |= XHDMIPHY1_DRP_PREIQ_FREQ_BST_VAL3 << 4;
        }
        /* Write new DRP register value for PREIQ_FREQ_BST. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00FB, DrpVal);
        /* RXPI_CFG0 */
        if (PllxClkOutMHz > XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE8) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE9) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL2;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL3;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE11) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL4;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE12) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL5;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE13) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL6;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE14) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL7;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE15) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL8;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE16) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL9;
        }
        else {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG0_VAL10;
        }
        /* Write new DRP register value for RXPI_CFG0. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_009D, DrpVal);
        /* RXPI_CFG1 */
        if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG1_VAL1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE12) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG1_VAL2;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE14) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG1_VAL3;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE18) {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG1_VAL4;
        }
        else {
            DrpVal = XHDMIPHY1_DRP_RXPI_CFG1_VAL5;
        }
        /* Write new DRP register value for RXPI_CFG1. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0100, DrpVal);
    }

    Status |= XHdmiphy1_Gthe4RxPllRefClkDiv1Reconfig(InstancePtr, QuadId,
                ChId);

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    XHdmiphy1_Channel *ChPtr;
    u32 ReturnVal;
    u16 DrpVal;
    u16 WriteVal;
    XHdmiphy1_ChannelId ChIdPll;
    XHdmiphy1_PllType PllType;
    u32 PllxVcoRateMHz;
    u32 PllxClkOutMHz;
    u32 PllxClkOutDiv;
    u32 Status = XST_SUCCESS;

    ReturnVal = XHdmiphy1_Gthe4TxPllRefClkDiv1Reconfig(InstancePtr, QuadId,
                    ChId);
    if (!XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
        return ReturnVal;
    }

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, QuadId,
                XHDMIPHY1_DIR_TX, ChId);
    /* Determine which channel(s) to operate on. */
    switch (PllType) {
        case XHDMIPHY1_PLL_TYPE_QPLL:
        case XHDMIPHY1_PLL_TYPE_QPLL0:
            ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN0;
            PllxClkOutDiv = XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL1;
            break;
        case XHDMIPHY1_PLL_TYPE_QPLL1:
            ChIdPll = XHDMIPHY1_CHANNEL_ID_CMN1;
            PllxClkOutDiv = XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL1;
            break;
        default:
            ChIdPll = ChId;
            PllxClkOutDiv = XHDMIPHY1_DRP_PLL_CLKOUT_DIV_VAL2;
            break;
    }
    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {

        ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)];

        /* Set TX_PROGDIV_CFG to 20/40 */
        if ((PllType == XHDMIPHY1_PLL_TYPE_QPLL) ||
            (PllType == XHDMIPHY1_PLL_TYPE_QPLL0) ||
            (PllType == XHDMIPHY1_PLL_TYPE_QPLL1)) {
            if (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].TxOutDiv != 16) {
                /* TX_PROGDIV_CFG = 20 */
                XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_003E, 57442);
            } else {
                /* TX_PROGDIV_CFG = 40 */
                XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_003E, 57415);
            }
        }

        /* TX_INT_DATAWIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0085, &DrpVal);
        DrpVal &= ~(0x3 << 10);
        WriteVal = ((XHdmiphy1_DrpEncodeIntDataWidth(ChPtr->
                        TxIntDataWidth) & 0x3) << 10);
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0085, DrpVal);
        /* TX_DATA_WIDTH */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_007A, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_007A_TX_DATA_WIDTH_MASK);
        WriteVal = (XHdmiphy1_DrpEncodeDataWidth(ChPtr->TxDataWidth) & 0xF);
        DrpVal |= WriteVal;
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_007A, DrpVal);
        /* TXPH_CFG */
	if (ChPtr->TxOutDiv == 1) {
		if (ChPtr->TxDataWidth > 40) {
			DrpVal = XHDMIPHY1_DRP_TXPH_CFG_VAL1;
		} else {
			DrpVal = XHDMIPHY1_DRP_TXPH_CFG_VAL2;
		}
	} else if (ChPtr->TxOutDiv == 2) {
		if (ChPtr->TxDataWidth > 20) {
			DrpVal = XHDMIPHY1_DRP_TXPH_CFG_VAL1;
		} else {
			DrpVal = XHDMIPHY1_DRP_TXPH_CFG_VAL2;
		}
	} else {
		DrpVal = XHDMIPHY1_DRP_TXPH_CFG_VAL1;
	}
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0073, DrpVal);

        PllxVcoRateMHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr,
                            QuadId, ChIdPll,
                            XHDMIPHY1_DIR_TX) / 1000000;
        PllxClkOutMHz = PllxVcoRateMHz / PllxClkOutDiv;
        /* TXPI_CFG */
        if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG_VAL1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE14) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG_VAL2;
        }
        else {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG_VAL3;
        }
        /* Write new DRP register value for TXPI_CFG. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00FF, DrpVal);

        /* TXPI_CFG3 & TXPI_CFG4*/
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_009C, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_009C_TXPI_CFG3_CFG4_MASK);
        if (PllxClkOutMHz > XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE8) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE9) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL2;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL3;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE11) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE13) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL2;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE15) {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL3;
        }
        else {
            DrpVal = XHDMIPHY1_DRP_TXPI_CFG3_CFG4_VAL4;
        }
        DrpVal = ( DrpVal << 5) & XDRP_GTHE4_CHN_REG_009C_TXPI_CFG3_CFG4_MASK;
        /* Write new DRP register value for TXPI_CFG3 & TXPI_CFG4. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_009C, DrpVal);

        /* TX_PI_BIASSET */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00FB, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_00FB_TXPI_BIASSET_MASK);
        if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE17) {
            DrpVal |= XHDMIPHY1_DRP_TXPI_BIASSET_VAL1 << 1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10) {
            DrpVal |= XHDMIPHY1_DRP_TXPI_BIASSET_VAL2 << 1;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE14) {
            DrpVal |= XHDMIPHY1_DRP_TXPI_BIASSET_VAL3 << 1;
        }
        /* Write new DRP register value for TX_PI_BIASSET. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_00FB, DrpVal);

        /* CH_HSPMUX_TX */
        Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0116, &DrpVal);
        DrpVal &= ~(XDRP_GTHE4_CHN_REG_0116_CH_TX_HSPMUX_MASK);
        if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE17) {
            DrpVal |= XHDMIPHY1_DRP_CH_HSPMUX_VAL1 << 8;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE10) {
            DrpVal |= XHDMIPHY1_DRP_CH_HSPMUX_VAL2 << 8;
        }
        else if (PllxClkOutMHz >= XHDMIPHY1_DRP_PLLx_CLKOUT_RANGE14) {
            DrpVal |= XHDMIPHY1_DRP_CH_HSPMUX_VAL3 << 8;
        }
        else {
            DrpVal |= XHDMIPHY1_DRP_CH_HSPMUX_VAL4 << 8;
        }
        /* Write new DRP register value for CH_HSPMUX_TX. */
        Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_0116, DrpVal);
    }
    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX CLKDIV1 settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4TxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u32 TxRefClkHz;
    u32 Status = XST_SUCCESS;
    XHdmiphy1_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)];

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
        TxRefClkHz = InstancePtr->HdmiTxRefClkHz;
    }
    else {
        TxRefClkHz = XHdmiphy1_GetQuadRefClkFreq(InstancePtr, QuadId,
                                PllPtr->PllRefClkSel);
    }

    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_007A,
                    &DrpVal);
    DrpVal &= ~(XDRP_GTHE4_CHN_REG_007A_TXCLK25_MASK);
    DrpVal |= XHdmiphy1_DrpEncodeClk25(TxRefClkHz) << 11;
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_007A,
                DrpVal);

    return Status;
}

/*****************************************************************************/
/**
* This function will configure the channel's RX CLKDIV1 settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gthe4RxPllRefClkDiv1Reconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u16 DrpVal;
    u32 RxRefClkHz;
    u32 Status = XST_SUCCESS;
    XHdmiphy1_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)];

    if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
        RxRefClkHz = InstancePtr->HdmiRxRefClkHz;
    }
    else {
        RxRefClkHz = XHdmiphy1_GetQuadRefClkFreq(InstancePtr, QuadId,
                                PllPtr->PllRefClkSel);
    }

    Status |= XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_006D,
                    &DrpVal);
    DrpVal &= ~(XDRP_GTHE4_CHN_REG_006D_RXCLK25_MASK);
    DrpVal |= XHdmiphy1_DrpEncodeClk25(RxRefClkHz) << 3;
    Status |= XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XDRP_GTHE4_CHN_REG_006D,
                    DrpVal);

    return Status;
}

/*****************************************************************************/
/**
* This function will translate the configured M value to DRP encoding.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return   The DRP encoding for M.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_MToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u8 MRefClkDiv;
    u8 DrpEncode;

    if ((ChId >= XHDMIPHY1_CHANNEL_ID_CH1) &&
        (ChId <= XHDMIPHY1_CHANNEL_ID_CH4)) {
        MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)]
                .PllParams.MRefClkDiv;
    }
    else if ((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) ||
            (ChId == XHDMIPHY1_CHANNEL_ID_CMN1)) {
        MRefClkDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)]
                .PllParams.MRefClkDiv;
    }
    else {
        MRefClkDiv = 0;
    }

    DrpEncode = XHdmiphy1_DrpEncodeQpllMCpllMN2(MRefClkDiv);

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured D value to DRP encoding.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    Dir is an indicator for RX or TX.
*
* @return   The DRP encoding for D.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
    u8 OutDiv;
    u8 DrpEncode;

    OutDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
            OutDiv[Dir];

    DrpEncode = XHdmiphy1_DrpEncodeCpllTxRxD(OutDiv);

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured N1/N2 value to DRP encoding.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
* @param    NId specified to operate on N1 (if == 1) or N2 (if == 2).
*
* @return   The DRP encoding for N1/N2.
*
* @note     None.
*
******************************************************************************/
static u16 XHdmiphy1_NToDrpEncoding(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 NId)
{
    u8 NFbDiv;
    u16 DrpEncode;

    if ((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) ||
            (ChId == XHDMIPHY1_CHANNEL_ID_CMN1)) {
        NFbDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
                    PllParams.NFbDiv;
        DrpEncode = XHdmiphy1_DrpEncodeQpllN(NFbDiv);
    }
    else if (NId == 1) {
        NFbDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
                    PllParams.N1FbDiv;
        DrpEncode = XHdmiphy1_DrpEncodeCpllN1(NFbDiv);
    }
    else {
        NFbDiv = InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
                    PllParams.N2FbDiv;
        DrpEncode = XHdmiphy1_DrpEncodeQpllMCpllMN2(NFbDiv);
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured QPLL's M or CPLL's M or N2
* values to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the QPLL's M or CPLL's M or N2 values.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeQpllMCpllMN2(u8 AttrEncode)
{
    u8 DrpEncode;

    switch (AttrEncode) {
    case 1:
        DrpEncode = 16;
        break;
    case 6:
        DrpEncode = 5;
        break;
    case 10:
        DrpEncode = 7;
        break;
    case 12:
        DrpEncode = 13;
        break;
    case 20:
        DrpEncode = 15;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 8:
    case 16:
        DrpEncode = (AttrEncode - 2);
        break;
    default:
        DrpEncode = 0xF;
        break;
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CPLL's N1 value to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the CPLL's N1 value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeCpllN1(u8 AttrEncode)
{
    u8 DrpEncode;

    DrpEncode = (AttrEncode - 4) & 0x1;

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CPLL's D values to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the CPLL's D value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeCpllTxRxD(u8 AttrEncode)
{
    u8 DrpEncode;

    switch (AttrEncode) {
    case 1:
        DrpEncode = 0;
        break;
    case 2:
        DrpEncode = 1;
        break;
    case 4:
        DrpEncode = 2;
        break;
    case 8:
        DrpEncode = 3;
        break;
    case 16:
        DrpEncode = 4;
        break;
    default:
        DrpEncode = 0x4;
        break;
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured QPLL's N value to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the QPLL's N value.
*
* @note     None.
*
******************************************************************************/
static u16 XHdmiphy1_DrpEncodeQpllN(u8 AttrEncode)
{
    u16 DrpEncode;

    if ((16 <= AttrEncode) && (AttrEncode <= 160)) {
        DrpEncode = AttrEncode - 2;
    }
    else {
        DrpEncode = 0xFF;
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured RXDATAWIDTH to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the RXDATAWIDTH value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeDataWidth(u8 AttrEncode)
{
    u8 DrpEncode;

    switch (AttrEncode) {
    case 16:
        DrpEncode = 2;
        break;
    case 20:
        DrpEncode = 3;
        break;
    case 32:
        DrpEncode = 4;
        break;
    case 40:
        DrpEncode = 5;
        break;
    case 64:
        DrpEncode = 6;
        break;
    case 80:
        DrpEncode = 7;
        break;
    case 128:
        DrpEncode = 8;
        break;
    case 160:
        DrpEncode = 9;
        break;
    default:
        DrpEncode = 0xF;
        break;
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured RXINTDATAWIDTH to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the RXINTDATAWIDTH value.
*
* @note     None.
*
******************************************************************************/
static u8 XHdmiphy1_DrpEncodeIntDataWidth(u8 AttrEncode)
{
    u8 DrpEncode;

    switch (AttrEncode) {
    case 2:
        DrpEncode = 0;
        break;
    case 4:
        DrpEncode = 1;
        break;
    default:
        DrpEncode = 2;
        break;
    }

    return DrpEncode;
}

/*****************************************************************************/
/**
* This function will translate the configured CLK25 to DRP encoding.
*
* @param    AttrEncode is the attribute to encode.
*
* @return   The DRP encoding for the CLK25 value.
*
* @note     None.
*
******************************************************************************/
static u16 XHdmiphy1_DrpEncodeClk25(u32 RefClkFreqHz)
{
    u16 DrpEncode;
    u32 RefClkFreqMHz = RefClkFreqHz / 1000000;

    DrpEncode = ((RefClkFreqMHz / 25) +
                    (((RefClkFreqMHz % 25) > 0) ? 1 : 0)) - 1;

    return (DrpEncode & 0x1F);
}

/*****************************************************************************/
/**
* This function configures the CPLL Calibration period and the count tolerance
* registers.
*
* CpllCalPeriod    = ((fPLLClkin * N1 * N2) / (20 * M)) /
*                       (16000 / (4 * fFreeRunClk))
* CpllCalTolerance = CpllCalPeriod * 0.10
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    Dir is an indicator for TX or RX.
* @param    FreeRunClkFreq is the freerunning clock freq in Hz
*            driving the GT Wiz instance
*
* @return   XST_SUCCESS / XST_FAILURE
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_CfgCpllCalPeriodandTol(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u32 FreeRunClkFreq)
{
    u64 CpllCalPeriod;
    u64 CpllCalTolerance;
    u64 PllVcoFreqHz;
    u32 RegVal;

    /* Check if ChID is not a GT Channel */
    if (!XHDMIPHY1_ISCH(ChId)) {
        return XST_FAILURE;
    }

    PllVcoFreqHz = XHdmiphy1_GetPllVcoFreqHz(InstancePtr, QuadId, ChId, Dir);
    CpllCalPeriod = PllVcoFreqHz * 200 / (u64)FreeRunClkFreq;
    if (CpllCalPeriod % 10) {
        CpllCalTolerance = (CpllCalPeriod / 10) + 1;
    }
    else {
        CpllCalTolerance = CpllCalPeriod / 10;
    }

    /* Read CPLL Calibration Period Value */
    RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
               XHDMIPHY1_CPLL_CAL_PERIOD_REG) &
               ~XHDMIPHY1_CPLL_CAL_PERIOD_MASK;
    RegVal |= CpllCalPeriod & XHDMIPHY1_CPLL_CAL_PERIOD_MASK;
    /* Write new CPLL Calibration Period Value */
    XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
                    XHDMIPHY1_CPLL_CAL_PERIOD_REG, RegVal);

    /* Read CPLL Calibration Tolerance Value */
    RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
                XHDMIPHY1_CPLL_CAL_TOL_REG) & ~XHDMIPHY1_CPLL_CAL_TOL_MASK;
    RegVal |= CpllCalTolerance & XHDMIPHY1_CPLL_CAL_TOL_MASK;
    /* Write new CPLL Calibration Tolerance Value */
    XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
                    XHDMIPHY1_CPLL_CAL_TOL_REG, RegVal);

    return XST_SUCCESS;
}
#endif
