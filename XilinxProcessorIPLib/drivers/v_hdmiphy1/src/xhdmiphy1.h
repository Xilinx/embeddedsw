/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1.h
 * @addtogroup xhdmiphy1_v2_0
 * @{
 * @details
 * This is main header file of the Xilinx HDMI PHY Controller driver
 *
 * <b>Video PHY Controller Overview</b>
 *
 * The PHY is intended to simplify the use of serial transceivers and adds
 * domain-specific configurability. The Video PHY Controller IP is not intended
 * to be used as a stand alone IP and must be used with Xilinx Video MACs such
 * as HDMI 2.1 Transmitter/Receiver Subsystems. The core enables simpler
 * connectivity between MAC layers for TX and RX paths. However, it is still
 * important to understand the behavior, usage, and any limitations of the
 * transceivers. See the device specific transceiver user guide for details.
 *
 * <b>Video PHY Controller Driver Features</b>
 *
 * Video PHY Controller driver supports following features
 *   - Xilinx HDMI 2.1 MAC IP
 *   - GTHE3, GTHE4 and GTYE4 GT types
 *   - HDMI:
 *   -   4 pixel-wide video interface
 *   -   8/10/12 bits per component
 *   -   RGB & YCbCr color space
 *   -   Up to 10k resolution at both Input and Output interface
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

#ifndef XHDMIPHY1_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIPHY1_H_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(XV_CONFIG_LOG_HDMIPHY1_DISABLE) && \
    !defined(XV_CONFIG_LOG_DISABLE_ALL)
#define XV_HDMIPHY1_LOG_ENABLE
#endif

/******************************* Include Files ********************************/

#include "xil_assert.h"
#include "xparameters.h"
#include "xhdmiphy1_hw.h"
#include "xvidc.h"

/******************* Macros (Inline Functions) Definitions ********************/

#define XHDMIPHY1_GTHE3 4
#define XHDMIPHY1_GTHE4 5
#define XHDMIPHY1_GTYE4 6
#define XHDMIPHY1_GTYE5 7

/****************************** Type Definitions ******************************/

/* This typedef enumerates the different GT types available. */
typedef enum {
    XHDMIPHY1_GT_TYPE_GTHE3 = 4,
    XHDMIPHY1_GT_TYPE_GTHE4 = 5,
    XHDMIPHY1_GT_TYPE_GTYE4 = 6,
    XHDMIPHY1_GT_TYPE_GTYE5 = 7,
} XHdmiphy1_GtType;

/**
 * This typedef enumerates the various protocols handled by the Video PHY
 * controller (HDMIPHY).
 */
typedef enum {
    XHDMIPHY1_PROTOCOL_HDMI   = 1,
    XHDMIPHY1_PROTOCOL_HDMI21 = 2,
    XHDMIPHY1_PROTOCOL_NONE   = 3
} XHdmiphy1_ProtocolType;

/* This typedef enumerates is used to specify RX/TX direction information. */
typedef enum {
    XHDMIPHY1_DIR_RX = 0,
    XHDMIPHY1_DIR_TX,
    XHDMIPHY1_DIR_NONE
} XHdmiphy1_DirectionType;

/**
 * This typedef enumerates the list of available interrupt handler types. The
 * values are used as parameters to the XHdmiphy1_SetIntrHandler function.
 */
typedef enum {
    XHDMIPHY1_INTR_HANDLER_TYPE_TXRESET_DONE =
        XHDMIPHY1_INTR_TXRESETDONE_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_RXRESET_DONE =
        XHDMIPHY1_INTR_RXRESETDONE_MASK,
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    XHDMIPHY1_INTR_HANDLER_TYPE_CPLL_LOCK =
        XHDMIPHY1_INTR_CPLL_LOCK_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_QPLL_LOCK =
        XHDMIPHY1_INTR_QPLL_LOCK_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_QPLL0_LOCK =
        XHDMIPHY1_INTR_QPLL_LOCK_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_TXALIGN_DONE =
        XHDMIPHY1_INTR_TXALIGNDONE_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_QPLL1_LOCK =
        XHDMIPHY1_INTR_QPLL1_LOCK_MASK,
#else
    XHDMIPHY1_INTR_HANDLER_TYPE_LCPLL_LOCK =
        XHDMIPHY1_INTR_LCPLL_LOCK_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_RPLL_LOCK =
        XHDMIPHY1_INTR_RPLL_LOCK_MASK,
#endif
    XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE =
        XHDMIPHY1_INTR_TXCLKDETFREQCHANGE_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE =
        XHDMIPHY1_INTR_RXCLKDETFREQCHANGE_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_TX_MMCM_LOCK_CHANGE =
        XHDMIPHY1_INTR_TXMMCMUSRCLK_LOCK_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_RX_MMCM_LOCK_CHANGE =
        XHDMIPHY1_INTR_RXMMCMUSRCLK_LOCK_MASK,
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
    XHDMIPHY1_INTR_HANDLER_TYPE_TX_GPO_RISING_EDGE =
	XHDMIPHY1_INTR_TXGPO_RE_MASK,
	XHDMIPHY1_INTR_HANDLER_TYPE_RX_GPO_RISING_EDGE =
	XHDMIPHY1_INTR_RXGPO_RE_MASK,
#endif
    XHDMIPHY1_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT =
        XHDMIPHY1_INTR_TXTMRTIMEOUT_MASK,
    XHDMIPHY1_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT =
        XHDMIPHY1_INTR_RXTMRTIMEOUT_MASK,
} XHdmiphy1_IntrHandlerType;

/**
 * This typedef enumerates the list of available hdmi handler types. The
 * values are used as parameters to the XHdmiphy1_SetHdmiCallback function.
 */
typedef enum {
    XHDMIPHY1_HDMI_HANDLER_TXINIT = 1,   /**< TX init handler. */
    XHDMIPHY1_HDMI_HANDLER_TXREADY,      /**< TX ready handler. */
    XHDMIPHY1_HDMI_HANDLER_RXINIT,       /**< RX init handler. */
    XHDMIPHY1_HDMI_HANDLER_RXREADY       /**< RX ready handler. */
} XHdmiphy1_HdmiHandlerType;

/**
 * This typedef enumerates the different PLL types for a given GT channel.
 */
typedef enum {
    XHDMIPHY1_PLL_TYPE_CPLL    = 1,
    XHDMIPHY1_PLL_TYPE_QPLL    = 2,
    XHDMIPHY1_PLL_TYPE_QPLL0   = 3,
    XHDMIPHY1_PLL_TYPE_QPLL1   = 4,
    XHDMIPHY1_PLL_TYPE_LCPLL   = 5,
    XHDMIPHY1_PLL_TYPE_RPLL    = 6,
    XHDMIPHY1_PLL_TYPE_UNKNOWN = 7,
} XHdmiphy1_PllType;

/**
 * This typedef enumerates the available channels.
 */
typedef enum {
    XHDMIPHY1_CHANNEL_ID_CH1    = 1,
    XHDMIPHY1_CHANNEL_ID_CH2    = 2,
    XHDMIPHY1_CHANNEL_ID_CH3    = 3,
    XHDMIPHY1_CHANNEL_ID_CH4    = 4,
    XHDMIPHY1_CHANNEL_ID_CMN0   = 5, /* QPLL, QPLL0, LCPLL */
    XHDMIPHY1_CHANNEL_ID_CMN1   = 6, /* QPLL1, RPLL */
    XHDMIPHY1_CHANNEL_ID_CHA    = 7,
    XHDMIPHY1_CHANNEL_ID_CMNA   = 8,
    XHDMIPHY1_CHANNEL_ID_TXMMCM = 9,
    XHDMIPHY1_CHANNEL_ID_RXMMCM = 10,
    XHDMIPHY1_CHANNEL_ID_CMN = XHDMIPHY1_CHANNEL_ID_CMN0,
} XHdmiphy1_ChannelId;

/**
 * This typedef enumerates the available reference clocks for the PLL clock
 * selection multiplexer.
 */
typedef enum {
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTREFCLK0,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK1 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTREFCLK1,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTNORTHREFCLK0 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTNORTHREFCLK0,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTNORTHREFCLK1 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTNORTHREFCLK1,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTSOUTHREFCLK0 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTSOUTHREFCLK0,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTSOUTHREFCLK1 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTSOUTHREFCLK1,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTEASTREFCLK0 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTEASTREFCLK0,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTEASTREFCLK1 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTEASTREFCLK1,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTWESTREFCLK0 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTWESTREFCLK0,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTWESTREFCLK1 =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTWESTREFCLK1,
    XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTGREFCLK =
        XHDMIPHY1_REF_CLK_SEL_XPLL_GTGREFCLK,
} XHdmiphy1_PllRefClkSelType;

/**
 * This typedef enumerates the available reference clocks used to drive the
 * RX/TX datapaths.
 */
typedef enum {
    XHDMIPHY1_SYSCLKSELDATA_TYPE_PLL0_OUTCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_PLL0,
    XHDMIPHY1_SYSCLKSELDATA_TYPE_PLL1_OUTCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_PLL1,
    XHDMIPHY1_SYSCLKSELDATA_TYPE_CPLL_OUTCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_CPLL,
    XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL_OUTCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_QPLL,
    XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_QPLL0,
    XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_DATA_QPLL1,
} XHdmiphy1_SysClkDataSelType;

/**
 * This typedef enumerates the available reference clocks used to drive the
 * RX/TX output clocks.
 */
typedef enum {
    XHDMIPHY1_SYSCLKSELOUT_TYPE_CPLL_REFCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CH,
    XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL_REFCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN,
    XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL0_REFCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN0,
    XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL1_REFCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN1,
    XHDMIPHY1_SYSCLKSELOUT_TYPE_PLL0_REFCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CH,
    XHDMIPHY1_SYSCLKSELOUT_TYPE_PLL1_REFCLK =
        XHDMIPHY1_REF_CLK_SEL_XXSYSCLKSEL_OUT_CMN,
} XHdmiphy1_SysClkOutSelType;

/**
 * This typedef enumerates the available clocks that are used as multiplexer
 * input selections for the RX/TX output clock.
 */
typedef enum {
    XHDMIPHY1_OUTCLKSEL_TYPE_OUTCLKPCS = 1,
    XHDMIPHY1_OUTCLKSEL_TYPE_OUTCLKPMA,
    XHDMIPHY1_OUTCLKSEL_TYPE_PLLREFCLK_DIV1,
    XHDMIPHY1_OUTCLKSEL_TYPE_PLLREFCLK_DIV2,
    XHDMIPHY1_OUTCLKSEL_TYPE_PROGDIVCLK
} XHdmiphy1_OutClkSelType;

/* This typedef enumerates the possible states a transceiver can be in. */
typedef enum {
    XHDMIPHY1_GT_STATE_IDLE,     /**< Idle state. */
	XHDMIPHY1_GT_STATE_GPO_RE,   /**< GPO RE state. */
    XHDMIPHY1_GT_STATE_LOCK,     /**< Lock state. */
    XHDMIPHY1_GT_STATE_RESET,    /**< Reset state. */
    XHDMIPHY1_GT_STATE_ALIGN,    /**< Align state. */
    XHDMIPHY1_GT_STATE_READY,    /**< Ready state. */
} XHdmiphy1_GtState;

#ifdef XV_HDMIPHY1_LOG_ENABLE
typedef enum {
    XHDMIPHY1_LOG_EVT_NONE = 1,      /**< Log event none. */
    XHDMIPHY1_LOG_EVT_QPLL_EN,       /**< Log event QPLL enable. */
    XHDMIPHY1_LOG_EVT_QPLL_RST,      /**< Log event QPLL reset. */
    XHDMIPHY1_LOG_EVT_QPLL_LOCK,     /**< Log event QPLL lock. */
    XHDMIPHY1_LOG_EVT_QPLL_RECONFIG, /**< Log event QPLL reconfig. */
    XHDMIPHY1_LOG_EVT_QPLL0_EN,      /**< Log event QPLL0 enable. */
    XHDMIPHY1_LOG_EVT_QPLL0_RST,     /**< Log event QPLL0 reset. */
    XHDMIPHY1_LOG_EVT_QPLL0_LOCK,    /**< Log event QPLL0 lock. */
    XHDMIPHY1_LOG_EVT_QPLL0_RECONFIG,/**< Log event QPLL0 reconfig. */
    XHDMIPHY1_LOG_EVT_QPLL1_EN,      /**< Log event QPLL1 enable. */
    XHDMIPHY1_LOG_EVT_QPLL1_RST,     /**< Log event QPLL1 reset. */
    XHDMIPHY1_LOG_EVT_QPLL1_LOCK,    /**< Log event QPLL1 lock. */
    XHDMIPHY1_LOG_EVT_QPLL1_RECONFIG,/**< Log event QPLL1 reconfig. */
    XHDMIPHY1_LOG_EVT_PLL0_EN,       /**< Log event PLL0 reset. */
    XHDMIPHY1_LOG_EVT_PLL0_RST,      /**< Log event PLL0 reset. */
    XHDMIPHY1_LOG_EVT_PLL1_EN,       /**< Log event PLL1 reset. */
    XHDMIPHY1_LOG_EVT_PLL1_RST,      /**< Log event PLL1 reset. */
    XHDMIPHY1_LOG_EVT_CPLL_EN,       /**< Log event CPLL reset. */
    XHDMIPHY1_LOG_EVT_CPLL_RST,      /**< Log event CPLL reset. */
    XHDMIPHY1_LOG_EVT_CPLL_LOCK,     /**< Log event CPLL lock. */
    XHDMIPHY1_LOG_EVT_CPLL_RECONFIG, /**< Log event CPLL reconfig. */
    XHDMIPHY1_LOG_EVT_LCPLL_LOCK,    /**< Log event LCPLL lock. */
    XHDMIPHY1_LOG_EVT_RPLL_LOCK,     /**< Log event RPLL lock. */
    XHDMIPHY1_LOG_EVT_TXPLL_EN,      /**< Log event TXPLL enable. */
    XHDMIPHY1_LOG_EVT_TXPLL_RST,     /**< Log event TXPLL reset. */
    XHDMIPHY1_LOG_EVT_RXPLL_EN,      /**< Log event RXPLL enable. */
    XHDMIPHY1_LOG_EVT_RXPLL_RST,     /**< Log event RXPLL reset. */
    XHDMIPHY1_LOG_EVT_GTRX_RST,      /**< Log event GT RX reset. */
    XHDMIPHY1_LOG_EVT_GTTX_RST,      /**< Log event GT TX reset. */
    XHDMIPHY1_LOG_EVT_VID_TX_RST,    /**< Log event Vid TX reset. */
    XHDMIPHY1_LOG_EVT_VID_RX_RST,    /**< Log event Vid RX reset. */
    XHDMIPHY1_LOG_EVT_TX_ALIGN,      /**< Log event TX align. */
    XHDMIPHY1_LOG_EVT_TX_ALIGN_TMOUT,/**< Log event TX align Timeout. */
    XHDMIPHY1_LOG_EVT_TX_TMR,        /**< Log event TX timer. */
    XHDMIPHY1_LOG_EVT_RX_TMR,        /**< Log event RX timer. */
    XHDMIPHY1_LOG_EVT_GT_RECONFIG,   /**< Log event GT reconfig. */
    XHDMIPHY1_LOG_EVT_GT_TX_RECONFIG,/**< Log event GT reconfig. */
    XHDMIPHY1_LOG_EVT_GT_RX_RECONFIG,/**< Log event GT reconfig. */
    XHDMIPHY1_LOG_EVT_INIT,          /**< Log event init. */
    XHDMIPHY1_LOG_EVT_TXPLL_RECONFIG,/**< Log event TXPLL reconfig. */
    XHDMIPHY1_LOG_EVT_RXPLL_RECONFIG,/**< Log event RXPLL reconfig. */
    XHDMIPHY1_LOG_EVT_RXPLL_LOCK,    /**< Log event RXPLL lock. */
    XHDMIPHY1_LOG_EVT_TXPLL_LOCK,    /**< Log event TXPLL lock. */
    XHDMIPHY1_LOG_EVT_TX_RST_DONE,   /**< Log event TX reset done. */
    XHDMIPHY1_LOG_EVT_RX_RST_DONE,   /**< Log event RX reset done. */
    XHDMIPHY1_LOG_EVT_TX_FREQ,       /**< Log event TX frequency. */
    XHDMIPHY1_LOG_EVT_RX_FREQ,       /**< Log event RX frequency. */
    XHDMIPHY1_LOG_EVT_DRU_EN,        /**< Log event DRU enable/disable. */
	XHDMIPHY1_LOG_EVT_TXGPO_RE,     /**< Log event TX GPO Rising Edge. */
	XHDMIPHY1_LOG_EVT_RXGPO_RE,     /**< Log event RX GPO Rising Edge. */
    XHDMIPHY1_LOG_EVT_FRL_RECONFIG,  /**< Log event FRL TX Reconfig. */
    XHDMIPHY1_LOG_EVT_TMDS_RECONFIG, /**< Log event TMDS TX Reconfig. */
    XHDMIPHY1_LOG_EVT_1PPC_ERR,      /**< Log event 1 PPC Error. */
    XHDMIPHY1_LOG_EVT_PPC_MSMTCH_ERR,/**< Log event PPC MismatchError. */
    XHDMIPHY1_LOG_EVT_VDCLK_HIGH_ERR,/**< Log evt VidClk > 148.5 MHz. */
    XHDMIPHY1_LOG_EVT_NO_DRU,        /**< Log evt Vid not supported no DRU. */
    XHDMIPHY1_LOG_EVT_GT_QPLL_CFG_ERR,/**< Log event QPLL Config not found. */
    XHDMIPHY1_LOG_EVT_GT_CPLL_CFG_ERR,/**< Log evt LCPLL Config not found. */
    XHDMIPHY1_LOG_EVT_GT_LCPLL_CFG_ERR,/**< Log evt RPLL Config not found. */
    XHDMIPHY1_LOG_EVT_GT_RPLL_CFG_ERR,/**< Log event QPLL Config not found. */
    XHDMIPHY1_LOG_EVT_VD_NOT_SPRTD_ERR,/**< Log evt Vid fmt not supported. */
    XHDMIPHY1_LOG_EVT_MMCM_ERR,      /**< Log event MMCM Config not found. */
    XHDMIPHY1_LOG_EVT_HDMI20_ERR,    /**< Log event HDMI2.0 not supported. */
    XHDMIPHY1_LOG_EVT_NO_QPLL_ERR,   /**< Log event QPLL not present. */
    XHDMIPHY1_LOG_EVT_DRU_CLK_ERR,   /**< Log event DRU clk wrong freq. */
    XHDMIPHY1_LOG_EVT_USRCLK_ERR,    /**< Log event usrclk > 297 MHz. */
    XHDMIPHY1_LOG_EVT_SPDGRDE_ERR,   /**< Log event Speed Grade -1 error. */
    XHDMIPHY1_LOG_EVT_DUMMY,         /**< Dummy Event should be last */
} XHdmiphy1_LogEvent;
#endif

/* This typedef enumerates the different MMCM Dividers */
typedef enum {
    XHDMIPHY1_MMCM_CLKFBOUT_MULT_F, /* M */
    XHDMIPHY1_MMCM_DIVCLK_DIVIDE,   /* D */
    XHDMIPHY1_MMCM_CLKOUT_DIVIDE    /* On */
} XHdmiphy1_MmcmDivType;

/* This typedef enumerates the different MMCM CLKINSEL */
typedef enum {
	XHDMIPHY1_MMCM_CLKINSEL_CLKIN1 = 1,
	XHDMIPHY1_MMCM_CLKINSEL_CLKIN2 = 0,
} XHdmiphy1_MmcmClkinsel;

/* This typedef enumerates the Linerate to TMDS Clock ratio
 * for HDMI TX TMDS Clock pattern generator. */
typedef enum {
    XHDMIPHY1_Patgen_Ratio_10    = 0x1,  /**< LR:Clock Ratio = 10 */
    XHDMIPHY1_Patgen_Ratio_20    = 0x2,  /**< LR:Clock Ratio = 20 */
    XHDMIPHY1_Patgen_Ratio_30    = 0x3,  /**< LR:Clock Ratio = 30 */
    XHDMIPHY1_Patgen_Ratio_40    = 0x4,  /**< LR:Clock Ratio = 40 */
    XHDMIPHY1_Patgen_Ratio_50    = 0x5,  /**< LR:Clock Ratio = 50 */
} XHdmiphy1_HdmiTx_Patgen;

/**
 * This typedef enumerates the available PRBS patterns available
 * from the
 */
typedef enum {
    XHDMIPHY1_PRBSSEL_STD_MODE       = 0x0, /**< Pattern gen/mon OFF  */
    XHDMIPHY1_PRBSSEL_PRBS7          = 0x1, /**< PRBS-7  */
    XHDMIPHY1_PRBSSEL_PRBS9          = 0x2, /**< PRBS-9  */
    XHDMIPHY1_PRBSSEL_PRBS15         = 0x3, /**< PRBS-15  */
    XHDMIPHY1_PRBSSEL_PRBS23         = 0x4, /**< PRBS-23  */
    XHDMIPHY1_PRBSSEL_PRBS31         = 0x5, /**< PRBS-31  */
    XHDMIPHY1_PRBSSEL_PCIE           = 0x8, /**< PCIE Compliance Pattern  */
    XHDMIPHY1_PRBSSEL_SQUARE_2UI     = 0x9, /**< Square wave with 2 UI  */
    XHDMIPHY1_PRBSSEL_SQUARE_16UI    = 0xA, /**< Square wave with 16 UI  */
} XHdmiphy1_PrbsPattern;

/******************************************************************************/
/**
 * Callback type which represents the handler for interrupts.
 *
 * @param   InstancePtr is a pointer to the XHdmiphy1 instance.
 *
 * @note    None.
 *
*******************************************************************************/
typedef void (*XHdmiphy1_IntrHandler)(void *InstancePtr);

/******************************************************************************/
/**
 * Callback type which represents a custom timer wait handler. This is only
 * used for Microblaze since it doesn't have a native sleep function. To avoid
 * dependency on a hardware timer, the default wait functionality is
 * implemented using loop iterations; this isn't too accurate. If a custom
 * timer handler is used, the user may implement their own wait implementation
 * using a hardware timer (see example/) for better accuracy.
 *
 * @param   InstancePtr is a pointer to the XHdmiphy1 instance.
 * @param   MicroSeconds is the number of microseconds to be passed to the
 *      timer function.
 *
 * @note    None.
 *
*******************************************************************************/
typedef void (*XHdmiphy1_TimerHandler)(void *InstancePtr, u32 MicroSeconds);

/******************************************************************************/
/**
 * Generic callback type.
 *
 * @param   CallbackRef is a pointer to the callback reference.
 *
 * @note    None.
 *
*******************************************************************************/
typedef void (*XHdmiphy1_Callback)(void *CallbackRef);

/******************************************************************************/
/**
 * Generic callback type.
 *
 * @param   CallbackRef is a pointer to the callback reference.
 *
 * @note    u8 value.
 *
*******************************************************************************/
typedef u64 (*XHdmiphy1_LogCallback)(void *CallbackRef);

/******************************************************************************/
/**
 * Error callback type.
 *
 * @param   CallbackRef is a pointer to the callback reference.
 *
 * @note    None.
 *
*******************************************************************************/
typedef void (*XHdmiphy1_ErrorCallback)(void *CallbackRef);
/**
 * This typedef contains configuration information for CPLL/QPLL programming.
 */
typedef struct {
    u8 MRefClkDiv;
    /* Aliases for N (QPLL) and N1/N2 (CPLL). */
    union {
        u8 NFbDivs[2];
        u8 NFbDiv;
        struct {
            u8 N1FbDiv;
            u8 N2FbDiv;
        };
    };
    u16 Cdr[5];
    u8 IsLowerBand;
} XHdmiphy1_PllParam;

/**
 * This typedef contains configuration information for PLL type and its
 * reference clock.
 */
typedef struct {
    /* Below members are common between CPLL/QPLL. */
    u64 LineRateHz;             /**< The line rate for the
                            channel. */
    union {
        XHdmiphy1_PllParam QpllParams;
        XHdmiphy1_PllParam CpllParams;   /**< Parameters for a CPLL. */
        XHdmiphy1_PllParam PllParams;
        u16 LineRateCfg;
    };
    union {
        XHdmiphy1_PllRefClkSelType CpllRefClkSel;
                        /**< Multiplexer selection for
                            the reference clock of
                            the CPLL. */
        XHdmiphy1_PllRefClkSelType PllRefClkSel;
    };
    /* Below members are CPLL specific. */
    union {
        struct {
            u8 RxOutDiv;        /**< Output clock divider D for
                            the RX datapath. */
            u8 TxOutDiv;        /**< Output clock divider D for
                            the TX datapath. */
        };
        u8 OutDiv[2];
    };
    union {
        struct {
            XHdmiphy1_GtState RxState;   /**< Current state of RX GT. */
            XHdmiphy1_GtState TxState;   /**< Current state of TX GT. */
        };
        XHdmiphy1_GtState GtState[2];
    };
    union {
        struct {
            XHdmiphy1_ProtocolType RxProtocol;
                        /**< The protocol which the RX
                            path is used for. */
            XHdmiphy1_ProtocolType TxProtocol;
                        /**< The protocol which the TX
                            path is used for. */
        };
        XHdmiphy1_ProtocolType Protocol[2];
    };
    union {
        struct {
            XHdmiphy1_SysClkDataSelType RxDataRefClkSel;
                        /**< Multiplexer selection for
                            the reference clock of
                            the RX datapath. */
            XHdmiphy1_SysClkDataSelType TxDataRefClkSel;
                        /**< Multiplexer selection for
                            the reference clock of
                            the TX datapath. */
        };
        XHdmiphy1_SysClkDataSelType DataRefClkSel[2];
    };
    union {
        struct {
            XHdmiphy1_SysClkOutSelType RxOutRefClkSel;
                        /**< Multiplexer selection for
                            the reference clock of
                            the RX output clock. */
            XHdmiphy1_SysClkOutSelType TxOutRefClkSel;
                        /**< Multiplexer selection for
                            the reference clock of
                            the TX output clock. */
        };
        XHdmiphy1_SysClkOutSelType OutRefClkSel[2];
    };
    union {
        struct {
            XHdmiphy1_OutClkSelType RxOutClkSel;
                        /**< Multiplexer selection for
                            which clock to use as
                            the RX output clock. */
            XHdmiphy1_OutClkSelType TxOutClkSel;
                        /**< Multiplexer selection for
                            which clock to use as
                            the TX output clock. */
        };
        XHdmiphy1_OutClkSelType OutClkSel[2];
    };
    union {
        struct {
            u8 RxDelayBypass;   /**< Bypasses the delay
                            alignment block for the
                            RX output clock. */
            u8 TxDelayBypass;   /**< Bypasses the delay
                            alignment block for the
                            TX output clock. */
        };
        u8 DelayBypass;
    };
    u8 RxDataWidth;             /**< In bits. */
    u8 RxIntDataWidth;          /**< In bytes. */
    u8 TxDataWidth;             /**< In bits. */
    u8 TxIntDataWidth;          /**< In bytes. */
} XHdmiphy1_Channel;

/**
 * This typedef contains configuration information for MMCM programming.
 */
typedef struct {
    u16 DivClkDivide;
    u16 ClkFbOutMult;
    u16 ClkFbOutFrac;
    u16 ClkOut0Div;
    u16 ClkOut0Frac;
    u16 ClkOut1Div;
    u16 ClkOut2Div;
} XHdmiphy1_Mmcm;

/**
 * This typedef represents a GT quad.
 */
typedef struct {
    union {
        struct {
            XHdmiphy1_Mmcm RxMmcm;   /**< Mixed-mode clock manager
                            (MMCM) parameters for
                            RX. */
            XHdmiphy1_Mmcm TxMmcm;   /**< MMCM parameters for TX. */
        };
        XHdmiphy1_Mmcm Mmcm[2];      /**< MMCM parameters. */
    };
    union {
        struct {
            XHdmiphy1_Channel Ch1;
            XHdmiphy1_Channel Ch2;
            XHdmiphy1_Channel Ch3;
            XHdmiphy1_Channel Ch4;
            union {
            	XHdmiphy1_Channel Cmn0;
            	XHdmiphy1_Channel Lcpll;
            };
            union {
            	XHdmiphy1_Channel Cmn1;
            	XHdmiphy1_Channel Rpll;
            };
        };
        XHdmiphy1_Channel Plls[6];
    };
    union {
        struct {
            u32 GtRefClk0Hz;
            u32 GtRefClk1Hz;
            u32 GtNorthRefClk0Hz;
            u32 GtNorthRefClk1Hz;
            u32 GtSouthRefClk0Hz;
            u32 GtSouthRefClk1Hz;
            u32 GtgRefClkHz;
        };
        u32 RefClkHz[7];
    };
} XHdmiphy1_Quad;

#ifdef XV_HDMIPHY1_LOG_ENABLE
/**
 * This typedef contains the logging mechanism for debug.
 */
typedef struct {
    u16 DataBuffer[128];        /**< Log buffer with event data. */
    u64 TimeRecord[128];        /**< Log time for the event */
    u8 HeadIndex;           /**< Index of the head entry of the
                        Event/DataBuffer. */
    u8 TailIndex;           /**< Index of the tail entry of the
                        Event/DataBuffer. */
} XHdmiphy1_Log;
#endif

/**
 * This typedef contains the HDMI 2.1 FRL configurations.
 */
typedef struct {
    u8 IsEnabled;       /**< Is HDMI operation is enabled? */
    u64 LineRate;       /**< Linerate in bps. */
    u8 NChannels;       /**< No of Channels. */
} XHdmiphy1_Hdmi21Cfg;

/**
 * This typedef contains configuration information for the Video PHY core.
 */
typedef struct {
    u16 DeviceId;           /**< Device instance ID. */
    UINTPTR BaseAddr;       /**< The base address of the core instance. */
    XHdmiphy1_GtType XcvrType;       /**< HDMIPHY Transceiver Type */
    u8 TxChannels;          /**< No. of active channels in TX */
    u8 RxChannels;          /**< No. of active channels in RX */
    XHdmiphy1_ProtocolType TxProtocol;   /**< Protocol which TX is used for. */
    XHdmiphy1_ProtocolType RxProtocol;   /**< Protocol which RX is used for. */
    XHdmiphy1_PllRefClkSelType TxRefClkSel; /**< TX REFCLK selection. */
    XHdmiphy1_PllRefClkSelType RxRefClkSel; /**< RX REFCLK selection. */
    XHdmiphy1_PllRefClkSelType TxFrlRefClkSel; /**< TX FRL REFCLK selection. */
    XHdmiphy1_PllRefClkSelType RxFrlRefClkSel; /**< RX FRL REFCLK selection. */
    XHdmiphy1_SysClkDataSelType TxSysPllClkSel; /**< TX SYSCLK selection. */
    XHdmiphy1_SysClkDataSelType RxSysPllClkSel; /**< RX SYSCLK selectino. */
    u8 DruIsPresent;        /**< A data recovery unit (DRU) exists
                                 in the design .*/
    XHdmiphy1_PllRefClkSelType DruRefClkSel; /**< DRU REFCLK selection. */
    XVidC_PixelsPerClock Ppc;   /**< Number of input pixels per
                                     clock. */
    u8 TxBufferBypass;      /**< TX Buffer Bypass is enabled in the
                                 design. */
    u8  HdmiFastSwitch;     /**< HDMI fast switching is enabled in the
                                 design. */
    u8  TransceiverWidth;   /**< Transceiver Width seeting in the design */
    u32 ErrIrq;             /**< Error IRQ is enalbed in design */
    u32 AxiLiteClkFreq;     /**< AXI Lite Clock Frequency in Hz */
    u32 DrpClkFreq;         /**< DRP Clock Frequency in Hz */
    u8  UseGtAsTxTmdsClk;   /**< Use 4th GT channel as TX TMDS clock */
} XHdmiphy1_Config;

/* Forward declaration. */
struct XHdmiphy1_GtConfigS;

/**
 * The XHdmiphy1 driver instance data. The user is required to allocate a
 * variable of this type for every XHdmiphy1 device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
    u32 IsReady;                /**< Device is initialized and ready. */
    XHdmiphy1_Config Config;     /**< Configuration structure for
                            the Video PHY core. */
    const struct XHdmiphy1_GtConfigS *GtAdaptor;
#ifdef XV_HDMIPHY1_LOG_ENABLE
    XHdmiphy1_Log Log;               /**< A log of events. */
#endif
    XHdmiphy1_Quad Quads[2];         /**< The quads available to the
                            Video PHY core.*/
    u32 HdmiRxRefClkHz;         /**< HDMI RX refclk. */
    u32 HdmiTxRefClkHz;         /**< HDMI TX refclk. */
    u8 HdmiRxTmdsClockRatio;        /**< HDMI TMDS clock ratio. */
    u8 HdmiTxSampleRate;            /**< HDMI TX sample rate. */
    u8 HdmiRxDruIsEnabled;          /**< The DRU is enabled. */
    u8 HdmiIsQpllPresent;           /**< QPLL is present in HW */
    XHdmiphy1_Hdmi21Cfg TxHdmi21Cfg; /**< TX HDMI Config */
    XHdmiphy1_Hdmi21Cfg RxHdmi21Cfg; /**< TX HDMI Config */
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    XHdmiphy1_IntrHandler IntrCpllLockHandler; /**< Callback function for CPLL
                            lock interrupts. */
    void *IntrCpllLockCallbackRef;      /**< A pointer to the user data
                            passed to the CPLL lock
                            callback function. */
    XHdmiphy1_IntrHandler IntrQpllLockHandler; /**< Callback function for QPLL
                            lock interrupts. */
    void *IntrQpllLockCallbackRef;      /**< A pointer to the user data
                            passed to the QPLL lock
                            callback function. */
    XHdmiphy1_IntrHandler IntrQpll1LockHandler; /**< Callback function for QPLL
                            lock interrupts. */
    void *IntrQpll1LockCallbackRef;     /**< A pointer to the user data
                            passed to the QPLL lock
                            callback function. */
    XHdmiphy1_IntrHandler IntrTxAlignDoneHandler; /**< Callback function for TX
                            align done lock
                            interrupts. */
    void *IntrTxAlignDoneCallbackRef;   /**< A pointer to the user data
                            passed to the TX align
                            done lock callback
                            function. */
#else
    XHdmiphy1_IntrHandler IntrLcpllLockHandler; /**< Callback function for
                            LCPLL lock interrupts. */
    void *IntrLcpllLockCallbackRef;      /**< A pointer to the user data
                            passed to the LCPLL lock callback function. */
    XHdmiphy1_IntrHandler IntrRpllLockHandler;   /**< Callback function for
                            RPLL lock interrupts. */
    void *IntrRpllLockCallbackRef;      /**< A pointer to the user data
                            passed to the RPLL lock callback function. */
    XHdmiphy1_IntrHandler IntrTxGpoRisingEdgeHandler;   /**< Callback function
						for TX GPO [3:0] Rising Edge. */
    void *IntrTxGpoRisingEdgeCallbackRef;      /**< A pointer to the user data
                            passed to the TX GPO [3:0] Rising Edge
                            callback function. */
    XHdmiphy1_IntrHandler IntrRxGpoRisingEdgeHandler;   /**< Callback function
						for RX GPO [7:4] Rising Edge. */
    void *IntrRxGpoRisingEdgeCallbackRef;      /**< A pointer to the user data
                            passed to the RX GPO [7:4] Rising Edge
                            callback function. */
#endif
    XHdmiphy1_IntrHandler IntrTxResetDoneHandler; /**< Callback function for TX
                            reset done lock
                            interrupts. */
    void *IntrTxResetDoneCallbackRef;   /**< A pointer to the user data
                            passed to the TX reset
                            done lock callback
                            function. */
    XHdmiphy1_IntrHandler IntrRxResetDoneHandler; /**< Callback function for RX
                            reset done lock
                            interrupts. */
    void *IntrRxResetDoneCallbackRef;   /**< A pointer to the user data
                            passed to the RX reset
                            done lock callback
                            function. */
    XHdmiphy1_IntrHandler IntrTxClkDetFreqChangeHandler; /**< Callback function
                            for TX clock detector
                            frequency change
                            interrupts. */
    void *IntrTxClkDetFreqChangeCallbackRef; /**< A pointer to the user data
                            passed to the TX clock
                            detector frequency
                            change callback
                            function. */
    XHdmiphy1_IntrHandler IntrRxClkDetFreqChangeHandler; /**< Callback function
                            for RX clock detector
                            frequency change
                            interrupts. */
    void *IntrRxClkDetFreqChangeCallbackRef; /**< A pointer to the user data
                            passed to the RX clock
                            detector frequency
                            change callback
                            function. */
    XHdmiphy1_IntrHandler IntrTxMmcmLockHandler; /**< Callback function
                            for TX MMCM lock
                            interrupts. */
    void *IntrTxMmcmLockCallbackRef; /**< A pointer to the user data
                            passed to the TX MMCM lock
                            callback function. */
    XHdmiphy1_IntrHandler IntrRxMmcmLockHandler; /**< Callback function
                            for RX MMCM lock
                            interrupts. */
    void *IntrRxMmcmLockCallbackRef; /**< A pointer to the user data
                            passed to the RX MMCM lock
                            callback function. */
    XHdmiphy1_IntrHandler IntrTxTmrTimeoutHandler; /**< Callback function for
                            TX timer timeout interrupts. */
    void *IntrTxTmrTimeoutCallbackRef;  /**< A pointer to the user data
                            passed to the TX timer
                            timeout callback
                            function. */
    XHdmiphy1_IntrHandler IntrRxTmrTimeoutHandler; /**< Callback function for
                            RX timer timeout interrupts. */
    void *IntrRxTmrTimeoutCallbackRef;  /**< A pointer to the user data
                            passed to the RX timer timeout
                            callback function. */
        /* Error Condition callbacks. */
    XHdmiphy1_ErrorCallback ErrorCallback; /**< Callback for Error Condition.*/
    void *ErrorRef;         /**< To be passed to the Error condition
                             callback. */
    XHdmiphy1_ErrorCallback PllLayoutErrorCallback;  /**< Callback for Error
                            Condition. */
    void *PllLayoutErrorRef;/**< To be passed to the Error condition
                             callback. */
        /* HDMI callbacks. */
    XHdmiphy1_Callback HdmiTxInitCallback;   /**< Callback for TX init. */
    void *HdmiTxInitRef;            /**< To be passed to the TX init
                            callback. */
    XHdmiphy1_Callback HdmiTxReadyCallback;  /**< Callback for TX ready. */
    void *HdmiTxReadyRef;           /**< To be passed to the TX
                            ready callback. */
    XHdmiphy1_Callback HdmiRxInitCallback;   /**< Callback for RX init. */
    void *HdmiRxInitRef;            /**< To be passed to the RX
                            init callback. */
    XHdmiphy1_Callback HdmiRxReadyCallback;  /**< Callback for RX ready. */
    void *HdmiRxReadyRef;           /**< To be passed to the RX
                            ready callback. */
    XHdmiphy1_LogCallback LogWriteCallback; /**< Callback for log write */
    u32 *LogWriteRef;  /**< To be passed to the log write callback */
    XHdmiphy1_TimerHandler UserTimerWaitUs;  /**< Custom user function for
                            delay/sleep. */
    void *UserTimerPtr;         /**< Pointer to a timer instance
                            used by the custom user
                            delay/sleep function. */
} XHdmiphy1;

/**************************** Function Prototypes *****************************/

/* xhdmiphy1.c: Setup and initialization functions. */
void XHdmiphy1_CfgInitialize(XHdmiphy1 *InstancePtr,
		XHdmiphy1_Config *ConfigPtr,
        UINTPTR EffectiveAddr);
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
u32 XHdmiphy1_PllInitialize(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId,
        XHdmiphy1_PllRefClkSelType QpllRefClkSel,
        XHdmiphy1_PllRefClkSelType CpllxRefClkSel,
        XHdmiphy1_PllType TxPllSelect, XHdmiphy1_PllType RxPllSelect);
#endif
u32 XHdmiphy1_GetVersion(XHdmiphy1 *InstancePtr);
void XHdmiphy1_WaitUs(XHdmiphy1 *InstancePtr, u32 MicroSeconds);
void XHdmiphy1_SetUserTimerHandler(XHdmiphy1 *InstancePtr,
        XHdmiphy1_TimerHandler CallbackFunc, void *CallbackRef);

/* xhdmiphy1.c: Channel configuration functions - setters. */
u32 XHdmiphy1_CfgLineRate(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 LineRateHz);

/* xhdmiphy1.c: Channel configuration functions - getters. */
XHdmiphy1_PllType XHdmiphy1_GetPllType(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, XHdmiphy1_ChannelId ChId);
u64 XHdmiphy1_GetLineRateHz(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);

/* xhdmiphy1.c: Reset functions. */
u32 XHdmiphy1_ResetGtPll(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Hold);
u32 XHdmiphy1_ResetGtTxRx(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Hold);

/* xhdmiphy1.c: TX|RX Control functions. */
u32 XHdmiphy1_SetPolarity(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Polarity);
u32 XHdmiphy1_SetPrbsSel(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        XHdmiphy1_PrbsPattern Pattern);
u32 XHdmiphy1_TxPrbsForceError(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 ForceErr);
void XHdmiphy1_SetTxVoltageSwing(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Vs);
void XHdmiphy1_SetTxPreEmphasis(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Pe);
void XHdmiphy1_SetTxPostCursor(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Pc);
void XHdmiphy1_SetRxLpm(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Enable);

/* xhdmiphy1.c: GT/MMCM DRP access. */
u32 XHdmiphy1_DrpWr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u16 Addr, u16 Val);
u16 XHdmiphy1_DrpRd(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u16 Addr, u16 *RetVal);
void XHdmiphy1_MmcmPowerDown(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u8 Hold);
void XHdmiphy1_MmcmStart(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_IBufDsEnable(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u8 Enable);
void XHdmiphy1_Clkout1OBufTdsEnable(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir, u8 Enable);

/* xhdmiphy1.c: Error Condition. */
void XHdmiphy1_SetErrorCallback(XHdmiphy1 *InstancePtr,
        void *CallbackFunc, void *CallbackRef);

/* xhdmiphy1_log.c: Logging functions. */
void XHdmiphy1_SetLogCallback(XHdmiphy1 *InstancePtr,
        u64 *CallbackFunc, void *CallbackRef);
void XHdmiphy1_LogDisplay(XHdmiphy1 *InstancePtr);
void XHdmiphy1_LogReset(XHdmiphy1 *InstancePtr);
u16 XHdmiphy1_LogRead(XHdmiphy1 *InstancePtr);
#ifdef XV_HDMIPHY1_LOG_ENABLE
void XHdmiphy1_LogWrite(XHdmiphy1 *InstancePtr, XHdmiphy1_LogEvent Evt,
		u8 Data);
#else
#define XHdmiphy1_LogWrite(...)
#endif

/* xhdmiphy1_intr.c: Interrupt handling functions. */
void XHdmiphy1_InterruptHandler(XHdmiphy1 *InstancePtr);

/* xhdmiphy1_selftest.c: Self test function. */
u32 XHdmiphy1_SelfTest(XHdmiphy1 *InstancePtr);

/* xhdmiphy1_sinit.c: Configuration extraction function. */
XHdmiphy1_Config *XHdmiphy1_LookupConfig(u16 DeviceId);

/* xhdmiphy1_hdmi.c, xhdmiphy1_hdmi_intr.c: Protocol specific functions. */
u32 XHdmiphy1_Hdmi_CfgInitialize(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_Config *CfgPtr);
u32 XHdmiphy1_SetHdmiTxParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId,
        XVidC_PixelsPerClock Ppc, XVidC_ColorDepth Bpc,
        XVidC_ColorFormat ColorFormat);
u32 XHdmiphy1_SetHdmiRxParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);

u32 XHdmiphy1_HdmiCfgCalcMmcmParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        XVidC_PixelsPerClock Ppc, XVidC_ColorDepth Bpc);

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
void XHdmiphy1_HdmiUpdateClockSelection(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_SysClkDataSelType TxSysPllClkSel,
        XHdmiphy1_SysClkDataSelType RxSysPllClkSel);
#endif
void XHdmiphy1_ClkDetFreqReset(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_ClkDetGetRefClkFreqHz(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_DruGetRefClkFreqHz(XHdmiphy1 *InstancePtr);
void XHdmiphy1_HdmiDebugInfo(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
void XHdmiphy1_SetHdmiCallback(XHdmiphy1 *InstancePtr,
        XHdmiphy1_HdmiHandlerType HandlerType,
        void *CallbackFunc, void *CallbackRef);
u32 XHdmiphy1_Hdmi20Config(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_Hdmi21Config(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir,
        u64 LineRate, u8 NChannels);
void XHdmiphy1_RegisterDebug(XHdmiphy1 *InstancePtr);

/******************* Macros (Inline Functions) Definitions ********************/

#define XHDMIPHY1_CH2IDX(Id) ((Id) - XHDMIPHY1_CHANNEL_ID_CH1)
#define XHDMIPHY1_ISCH(Id)       (((Id) == XHDMIPHY1_CHANNEL_ID_CHA) || \
	((XHDMIPHY1_CHANNEL_ID_CH1 <= (Id)) && ((Id) <= XHDMIPHY1_CHANNEL_ID_CH4)))
#define XHDMIPHY1_ISCMN(Id)      (((Id) == XHDMIPHY1_CHANNEL_ID_CMNA) || \
  ((XHDMIPHY1_CHANNEL_ID_CMN0 <= (Id)) && ((Id) <= XHDMIPHY1_CHANNEL_ID_CMN1)))
#define XHDMIPHY1_ISTXMMCM(Id)   ((Id) == XHDMIPHY1_CHANNEL_ID_TXMMCM)
#define XHDMIPHY1_ISRXMMCM(Id)   ((Id) == XHDMIPHY1_CHANNEL_ID_RXMMCM)

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
#define XHdmiphy1_IsTxUsingQpll(InstancePtr, QuadId, ChId) \
        ((XHDMIPHY1_PLL_TYPE_QPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, ChId)) || \
        (XHDMIPHY1_PLL_TYPE_QPLL0 == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, ChId)) || \
        (XHDMIPHY1_PLL_TYPE_QPLL1 == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, ChId)))
#define XHdmiphy1_IsRxUsingQpll(InstancePtr, QuadId, ChId) \
        ((XHDMIPHY1_PLL_TYPE_QPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, ChId)) || \
        (XHDMIPHY1_PLL_TYPE_QPLL0 == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, ChId)) || \
        (XHDMIPHY1_PLL_TYPE_QPLL1 == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, ChId)))
#define XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) \
        (XHDMIPHY1_PLL_TYPE_CPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, ChId))
#define XHdmiphy1_IsRxUsingCpll(InstancePtr, QuadId, ChId) \
        (XHDMIPHY1_PLL_TYPE_CPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, ChId))
#else
#define XHdmiphy1_IsTxUsingLcpll(InstancePtr, QuadId, ChId) \
        (XHDMIPHY1_PLL_TYPE_LCPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, ChId))
#define XHdmiphy1_IsRxUsingLcpll(InstancePtr, QuadId, ChId) \
        (XHDMIPHY1_PLL_TYPE_LCPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, ChId))
#define XHdmiphy1_IsTxUsingRpll(InstancePtr, QuadId, ChId) \
        (XHDMIPHY1_PLL_TYPE_RPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, ChId))
#define XHdmiphy1_IsRxUsingRpll(InstancePtr, QuadId, ChId) \
        (XHDMIPHY1_PLL_TYPE_RPLL == \
        XHdmiphy1_GetPllType(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, ChId))
#endif

#ifdef __cplusplus
}
#endif

#endif /* XHDMIPHY1_H_ */
/** @} */
