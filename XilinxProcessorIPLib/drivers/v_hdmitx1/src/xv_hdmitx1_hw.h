/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx HDMI TX core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xv_hdmitx1.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     22/05/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_HDMITX1_HW_H_
#define XV_HDMITX1_HW_H_     /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/
/**< DDC Register Address */
#define XV_HDMITX1_DDC_SINK_VER_REG                  0x01
#define XV_HDMITX1_DDC_UPDATE_FLGS_REG               0x10
#define XV_HDMITX1_DDC_CED_REG                       0x50
#define XV_HDMITX1_DDC_STCR_REG                 	 0x35
#define XV_HDMITX1_DDC_STAT_FLGS_REG                 0x40
#define XV_HDMITX1_DDC_STAT_FLGS_LN01_REG            0x41
#define XV_HDMITX1_DDC_STAT_FLGS_LN23_REG            0x42
/*#define XV_HDMITX1_DDC_CFG_1_REG                     0x31*/

/* DDC UPDATE FLAGS register masks and shifts*/
#define XV_HDMITX1_DDC_UPDATE_FLGS_CED_UPDATE_MASK   0x02
#define XV_HDMITX1_DDC_UPDATE_FLGS_STUPDATE_MASK     0x08
#define XV_HDMITX1_DDC_UPDATE_FLGS_FRL_START_MASK    0x10
#define XV_HDMITX1_DDC_UPDATE_FLGS_FLT_UPDATE_MASK   0x20

/* DDC Source Test Configuration Request register masks and shifts*/
#define XV_HDMITX1_DDC_STCR_FLT_NO_TIMEOUT_MASK      0x20

/* DDC STATUS FLAGS register masks and shifts*/
#define XV_HDMITX1_DDC_STAT_FLGS_FLT_RDY_MASK        0x40
#define XV_HDMITX1_DDC_STAT_FLGS_LN01_LN0_MASK       0x0F
#define XV_HDMITX1_DDC_STAT_FLGS_LN01_LN1_SHIFT      4
#define XV_HDMITX1_DDC_STAT_FLGS_LN23_LN2_MASK       0x0F
#define XV_HDMITX1_DDC_STAT_FLGS_LN23_LN3_MASK       0x0F
#define XV_HDMITX1_DDC_STAT_FLGS_LN23_LN3_SHIFT      4

/* DDC Configuration register masks and shifts*/
#define XV_HDMITX1_DDC_CFG_1_FFE_LVLS_MASK      0xF
#define XV_HDMITX1_DDC_CFG_1_FFE_LVLS_SHIFT     4
#define XV_HDMITX1_DDC_CFG_1_FRL_RATE_MASK      0xF

/**< VER (Version Interface) peripheral register offsets */
/**< The VER is the first peripheral on the local bus */
#define XV_HDMITX1_VER_BASE              (0*64)
#define XV_HDMITX1_VER_ID_OFFSET         ((XV_HDMITX1_VER_BASE)+(0*4))/**<
                                    * VER Identification *  Register offset */
#define XV_HDMITX1_VER_VERSION_OFFSET    ((XV_HDMITX1_VER_BASE)+(1*4))/**<
                                    * VER Version Register *  offset */
#define XV_HDMITX1_BRDG_FIFO_LVL_OFFSET  ((XV_HDMITX1_VER_BASE)+(2*4))/**<
                                    * Bridge FIFO Level Register offset */
#define XV_HDMITX1_VCKE_SYS_CNT_OFFSET   ((XV_HDMITX1_VER_BASE)+(3*4))/**<
                                    * VCKE System Count Register offset */
#define XV_HDMITX1_DBG_STS_OFFSET        ((XV_HDMITX1_VER_BASE)+(4*4))/**<
                                    * Debug Status Register offset */
#define XV_HDMITX1_ANLZ_HBP_HS_OFFSET    ((XV_HDMITX1_VER_BASE)+(5*4))/**<
                                    * Analyzer HPB HS Register offset */
#define XV_HDMITX1_ANLZ_LN_ACT_OFFSET    ((XV_HDMITX1_VER_BASE)+(6*4))/**<
                                    * Analyzer LN ACT Register offset */

#define XV_HDMITX1_BRDG_FIFO_LVL_MIN_MASK       0xFFFF     /**< FRL Control Lane 0
                                                  * LTP mask */
#define XV_HDMITX1_BRDG_FIFO_LVL_MIN_SHIFT      0       /**< FRL Control Lane 0
                                                  * LTP shift */
#define XV_HDMITX1_BRDG_FIFO_LVL_MAX_MASK       0xFFFF     /**< FRL Control Lane 0
                                                  * LTP mask */
#define XV_HDMITX1_BRDG_FIFO_LVL_MAX_SHIFT      16       /**< FRL Control Lane 0
                                                  * LTP shift */

#define XV_HDMITX1_DBG_STS_FRL_ANLZ_VID_TIM_CHG_MASK   (1<<0)
#define XV_HDMITX1_DBG_STS_TRIB_ANLZ_VID_TIM_CHG_MASK  (1<<1)
#define XV_HDMITX1_DBG_STS_NTV_ANLZ_VID_TIM_CHG_MASK   (1<<2)

/* Analyzer register masks*/
#define XV_HDMITX1_ANLZ_HBP_HS_HS_SZ_SHIFT      0       /**< Analyzer hsync size shift */
#define XV_HDMITX1_ANLZ_HBP_HS_HS_SZ_MASK       0xFFFF  /**< Analyzer hsync size mask */
#define XV_HDMITX1_ANLZ_HBP_HS_HPB_SZ_SHIFT     16      /**< Analyzer hbp size shift */
#define XV_HDMITX1_ANLZ_HBP_HS_HPB_SZ_MASK      0xFFFF  /**< Analyzer hbp size mask */

/* Analyzer register masks*/
#define XV_HDMITX1_ANLZ_LN_ACT_ACT_SZ_SHIFT     0       /**< Analyzer analyzer act size shift */
#define XV_HDMITX1_ANLZ_LN_ACT_ACT_SZ_MASK      0xFFFF  /**< Analyzer analyzer act size mask */
#define XV_HDMITX1_ANLZ_LN_ACT_LN_SZ_SHIFT      16      /**< Analyzer analyzer line act shift */
#define XV_HDMITX1_ANLZ_LN_ACT_LN_SZ_MASK       0xFFFF  /**< Analyzer analyzer line act mask */

/**< PIO (Parallel Interface) peripheral register offsets */
/**< The PIO is the first peripheral on the local bus */
#define XV_HDMITX1_PIO_BASE              (1*64)
#define XV_HDMITX1_PIO_ID_OFFSET         ((XV_HDMITX1_PIO_BASE)+(0*4))/**< PIO
                                        * Identification *  Register offset */
#define XV_HDMITX1_PIO_CTRL_OFFSET       ((XV_HDMITX1_PIO_BASE)+(1*4))/**< PIO
                                        * Control Register *  offset */
#define XV_HDMITX1_PIO_CTRL_SET_OFFSET   ((XV_HDMITX1_PIO_BASE)+(2*4))/**< PIO
                                        * Control Register Set *  offset */
#define XV_HDMITX1_PIO_CTRL_CLR_OFFSET   ((XV_HDMITX1_PIO_BASE)+(3*4))/**< PIO
                                        * Control Register Clear *  offset */
#define XV_HDMITX1_PIO_STA_OFFSET        ((XV_HDMITX1_PIO_BASE)+(4*4))/**< PIO
                                        * Status Register *  offset */
#define XV_HDMITX1_PIO_OUT_OFFSET        ((XV_HDMITX1_PIO_BASE)+(5*4))/**< PIO
                                        * Out Register offset */
#define XV_HDMITX1_PIO_OUT_SET_OFFSET    ((XV_HDMITX1_PIO_BASE)+(6*4))/**< PIO
                                        * Out Register Set *  offset */
#define XV_HDMITX1_PIO_OUT_CLR_OFFSET    ((XV_HDMITX1_PIO_BASE)+(7*4))/**< PIO
                                        * Out Register Clear *  offset */
#define XV_HDMITX1_PIO_OUT_MSK_OFFSET    ((XV_HDMITX1_PIO_BASE)+(8*4))/**< PIO
                                        * Out Mask Register *  offset */
#define XV_HDMITX1_PIO_IN_OFFSET         ((XV_HDMITX1_PIO_BASE)+(9*4))/**< PIO
                                        * In Register offset */
#define XV_HDMITX1_PIO_IN_EVT_OFFSET     ((XV_HDMITX1_PIO_BASE)+(10*4))/**< PIO
                                        * In Event Register *  offset */
#define XV_HDMITX1_PIO_IN_EVT_RE_OFFSET  ((XV_HDMITX1_PIO_BASE)+(11*4))/**< PIO
                                        * In Event Rising Edge
					*  Register offset */
#define XV_HDMITX1_PIO_IN_EVT_FE_OFFSET  ((XV_HDMITX1_PIO_BASE)+(12*4))/**< PIO
                                        * In Event Falling Edge
					*  Register offset */
#define XV_HDMITX1_HPD_TIMEGRID_OFFSET   ((XV_HDMITX1_PIO_BASE)+(13*4))/**< PIO
                                        * HPD Config. *  offset */
#define XV_HDMITX1_TOGGLE_CONF_OFFSET    ((XV_HDMITX1_PIO_BASE)+(14*4))/**< PIO
                                        * HPD Config. *  Register offset */
#define XV_HDMITX1_CONNECT_CONF_OFFSET   ((XV_HDMITX1_PIO_BASE)+(15*4))/**< PIO
                                        * HPD Config. *  Register offset */

/* PIO peripheral Control register masks*/
#define XV_HDMITX1_PIO_CTRL_RUN_MASK     (1<<0)  /**< PIO Control Run mask */
#define XV_HDMITX1_PIO_CTRL_IE_MASK      (1<<1)  /**< PIO Control Interrupt
                                                * Enable mask */

/* PIO peripheral Status register masks*/
#define XV_HDMITX1_PIO_STA_IRQ_MASK      (1<<0) /**< PIO Status Interrupt mask */
#define XV_HDMITX1_PIO_STA_EVT_MASK      (1<<1) /**< PIO Status Event mask */

/* PIO peripheral PIO Out register masks and shifts*/
#define XV_HDMITX1_PIO_OUT_RST_MASK          (1<<0)  /**< PIO Out Reset mask */
#define XV_HDMITX1_PIO_OUT_MODE_MASK         (1<<3)  /**< PIO Out Mode mask */
#define XV_HDMITX1_PIO_OUT_COLOR_DEPTH_MASK  0x30    /**< PIO Out Color Depth
                                                    * mask */
#define XV_HDMITX1_PIO_OUT_PIXEL_RATE_MASK   0xC0    /**< PIO Out Pixel Rate
                                                    * mask */
#define XV_HDMITX1_PIO_OUT_SAMPLE_RATE_MASK  0x300   /**< PIO Out Sample Rate
                                                    * mask */
#define XV_HDMITX1_PIO_OUT_COLOR_SPACE_MASK  0xC00   /**< PIO Out Color Space
                                                    * mask */
#define XV_HDMITX1_PIO_OUT_SCRM_MASK         (1<<12) /**< PIO Out Scrambler
                                                    * mask */
#define XV_HDMITX1_PIO_OUT_COLOR_DEPTH_SHIFT 4   /**< PIO Out Color Depth
                                                    * shift */
#define XV_HDMITX1_PIO_OUT_PIXEL_RATE_SHIFT  6   /**< PIO Out Pixel Rate
                                                    * shift */
#define XV_HDMITX1_PIO_OUT_SAMPLE_RATE_SHIFT 8   /**< PIO Out Sample Rate
                                                    * shift */
#define XV_HDMITX1_PIO_OUT_COLOR_SPACE_SHIFT 10  /**< PIO Out Color Space
                                                    * shift */
#define XV_HDMITX1_PIO_OUT_GCP_CLEARAVMUTE_MASK    (1<<28) /**< PIO Out
						* GCP_CLEARAVMUTE mask */
#define XV_HDMITX1_PIO_OUT_BRIDGE_YUV420_MASK (1<<29) /**< PIO Out Bridge_YUV420
                                                         * mask */
#define XV_HDMITX1_PIO_OUT_BRIDGE_PIXEL_MASK  (1<<30) /**< PIO Out Bridge_Pixel
                                                         * repeat mask */
#define XV_HDMITX1_PIO_OUT_GCP_AVMUTE_MASK    (1<<31) /**< PIO Out GCP_AVMUTE
                                                         * mask */
#define XV_HDMITX1_PIO_OUT_INT_VRST_MASK      (1<<0)  /**< PIO Out INT_VRST
                                                         * mask */
#define XV_HDMITX1_PIO_OUT_INT_LRST_MASK      (1<<20) /**< PIO Out INT_LRST
                                                        * mask */
#define XV_HDMITX1_PIO_OUT_EXT_VRST_MASK      (1<<21) /**< PIO Out EXT_VRST
                                                        * mask */
#define XV_HDMITX1_PIO_OUT_EXT_SYSRST_MASK    (1<<22) /**< PIO Out EXT_SYSRST
                                                         * mask */


/* PIO peripheral PIO In register masks*/
#define XV_HDMITX1_PIO_IN_LNK_RDY_MASK       (1<<0)  /**< PIO In link ready
                                                    * mask */
#define XV_HDMITX1_PIO_IN_VID_RDY_MASK       (1<<1)  /**< PIO In video ready
                                                    * mask */
#define XV_HDMITX1_PIO_IN_HPD_MASK           (1<<2)  /**< PIO In HPD mask */
#define XV_HDMITX1_PIO_IN_VS_MASK            (1<<3)  /**< PIO In Vsync mask */
#define XV_HDMITX1_PIO_IN_PPP_MASK           0x07    /**< PIO In Pixel packing
                                                    * phase mask */
#define XV_HDMITX1_PIO_IN_HPD_TOGGLE_MASK    (1<<8)  /**< PIO In HPD toggle
                                                     * mask */
#define XV_HDMITX1_PIO_IN_PPP_SHIFT          5       /**< PIO In Pixel packing
                                                    * phase shift */
#define XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK   (1<<9) /**< PIO In Bridge Locked
                                                    * mask */
#define XV_HDMITX1_PIO_IN_BRDG_OVERFLOW_MASK (1<<10) /**< PIO In Bridge Overflow
                                                    * mask */
#define XV_HDMITX1_PIO_IN_BRDG_UNDERFLOW_MASK (1<<11) /**< PIO In Bridge
                                                    * Underflow mask */

/**< DDC (Display Data Channel) peripheral register offsets */
/**< The DDC is the second peripheral on the local bus */
#define XV_HDMITX1_DDC_BASE                  (2*64)
#define XV_HDMITX1_DDC_ID_OFFSET             ((XV_HDMITX1_DDC_BASE)+(0*4))/**< DDC
                                * Identification *  Register offset */
#define XV_HDMITX1_DDC_CTRL_OFFSET           ((XV_HDMITX1_DDC_BASE)+(1*4))/**< DDC
                                * Control Register *  offset */
#define XV_HDMITX1_DDC_CTRL_SET_OFFSET       ((XV_HDMITX1_DDC_BASE)+(2*4))/**< DDC
                                * Control Register Set *  offset */
#define XV_HDMITX1_DDC_CTRL_CLR_OFFSET       ((XV_HDMITX1_DDC_BASE)+(3*4))/**< DDC
                                * Control Register Clear *  offset */
#define XV_HDMITX1_DDC_STA_OFFSET            ((XV_HDMITX1_DDC_BASE)+(4*4))/**< DDC
                                * Status Register *  offset */
#define XV_HDMITX1_DDC_CMD_OFFSET            ((XV_HDMITX1_DDC_BASE)+(5*4))/**< DDC
                                * Command Register *  offset */
#define XV_HDMITX1_DDC_DAT_OFFSET            ((XV_HDMITX1_DDC_BASE)+(6*4))/**< DDC
                                * Data Register *  offset */

/* DDC peripheral Control register masks and shift*/
#define XV_HDMITX1_DDC_CTRL_RUN_MASK         (1<<0)  /**< DDC Control Run mask */
#define XV_HDMITX1_DDC_CTRL_IE_MASK          (1<<1)  /**< DDC Control Interrupt
                                                    *  Enable mask */
#define XV_HDMITX1_DDC_CTRL_TO_STOP_MASK     (1<<2)  /**< DDC Control TO Stop
                                                    *  mask */
#define XV_HDMITX1_DDC_CTRL_CLK_DIV_MASK     0xFFFF  /**< DDC Control Clock
                                                    * Divider mask */
#define XV_HDMITX1_DDC_CTRL_CLK_DIV_SHIFT    16  /**< DDC Control Clock
                                                *Divider shift */ /*@}*/

/* DDC peripheral Status register masks*/
#define XV_HDMITX1_DDC_STA_IRQ_MASK      (1<<0)  /**< DDC Status IRQ mask */
#define XV_HDMITX1_DDC_STA_EVT_MASK      (1<<1)  /**< DDC Status Event mask */
#define XV_HDMITX1_DDC_STA_BUSY_MASK     (1<<2)  /**< DDC Status Busy mask */
#define XV_HDMITX1_DDC_STA_DONE_MASK     (1<<3)  /**< DDC Status Busy mask */
#define XV_HDMITX1_DDC_STA_TIMEOUT_MASK  (1<<4)  /**< DDC Status Timeout mask */
#define XV_HDMITX1_DDC_STA_ACK_MASK      (1<<5)  /**< DDC Status ACK mask */
#define XV_HDMITX1_DDC_STA_SCL_MASK      (1<<6)  /**< DDC State of SCL Input
                                                * mask */
#define XV_HDMITX1_DDC_STA_SDA_MASK      (1<<7)  /**< DDC State of SDA Input
                                                * mask */
#define XV_HDMITX1_DDC_STA_CMD_FULL      (1<<8)  /**< Command fifo full */
#define XV_HDMITX1_DDC_STA_DAT_EMPTY     (1<<9)  /**< Data fifo empty */
#define XV_HDMITX1_DDC_STA_CMD_WRDS_MASK 0xFF /**< Command fifo words mask*/
#define XV_HDMITX1_DDC_STA_CMD_WRDS_SHIFT    16  /**< Command fifo words shift */
#define XV_HDMITX1_DDC_STA_DAT_WRDS_MASK     0xFF /**< Data fifo words mask */
#define XV_HDMITX1_DDC_STA_DAT_WRDS_SHIFT    24  /**< Data fifo words shift */

/* DDC peripheral token*/
#define XV_HDMITX1_DDC_CMD_STR_TOKEN     (0x100) /**< Start token */
#define XV_HDMITX1_DDC_CMD_STP_TOKEN     (0x101) /**< Stop token */
#define XV_HDMITX1_DDC_CMD_RD_TOKEN      (0x102) /**< Read token */
#define XV_HDMITX1_DDC_CMD_WR_TOKEN      (0x103) /**< Write token */

/* Auxiliary (AUX) peripheral register offsets*/
/* The AUX is the third peripheral on the local bus*/
#define XV_HDMITX1_AUX_BASE              (3*64)
#define XV_HDMITX1_AUX_ID_OFFSET         ((XV_HDMITX1_AUX_BASE)+(0*4)) /**< AUX
                                * Identification *  Register offset */
#define XV_HDMITX1_AUX_CTRL_OFFSET       ((XV_HDMITX1_AUX_BASE)+(1*4)) /**< AUX
                                * Control Register *  offset */
#define XV_HDMITX1_AUX_CTRL_SET_OFFSET   ((XV_HDMITX1_AUX_BASE)+(2*4)) /**< AUX
                                * Control Register Set *  offset */
#define XV_HDMITX1_AUX_CTRL_CLR_OFFSET   ((XV_HDMITX1_AUX_BASE)+(3*4)) /**< AUX
                                * Control Register Clear *  offset */
#define XV_HDMITX1_AUX_STA_OFFSET        ((XV_HDMITX1_AUX_BASE)+(4*4)) /**< AUX
                                * Status Register *  offset */
#define XV_HDMITX1_AUX_DAT_OFFSET        ((XV_HDMITX1_AUX_BASE)+(5*4)) /**< AUX
                                * Data Register *  offset */

/* Auxiliary peripheral Control register masks*/
#define XV_HDMITX1_AUX_CTRL_RUN_MASK         (1<<0)  /**< AUX Control Run mask */
#define XV_HDMITX1_AUX_CTRL_IE_MASK          (1<<1)  /**< AUX Control Interrupt
                                                    * Enable mask */

/* Auxiliary peripheral Status register masks and shift*/
#define XV_HDMITX1_AUX_STA_IRQ_MASK          (1<<0)  /**< AUX Status Interrupt
                                                    *  mask */
#define XV_HDMITX1_AUX_STA_FIFO_EMT_MASK     (1<<1)  /**< AUX Status FIFO Empty
                                                    *  mask */
#define XV_HDMITX1_AUX_STA_FIFO_FUL_MASK     (1<<2)  /**< AUX Status FIFO Full
                                                    *  mask */
#define XV_HDMITX1_AUX_STA_PKT_RDY_MASK     (1<<3)  /**< AUX Status FIFO Ready
                                                    *  mask */
#define XV_HDMITX1_AUX_STA_FREE_PKTS_MASK    0x0F    /**< AUX Status Free Packets
                                                    *  mask */
#define XV_HDMITX1_AUX_STA_FREE_PKTS_SHIFT   15  /**< AUX Status Free
                                                    *  Packets shift */


/* Audio (AUD) peripheral register offsets*/
/* The AUD is the forth peripheral on the local bus*/
#define XV_HDMITX1_AUD_BASE              (4*64)
#define XV_HDMITX1_AUD_ID_OFFSET         ((XV_HDMITX1_AUD_BASE)+(0*4)) /**< AUD
                                * Identification *  Register offset */
#define XV_HDMITX1_AUD_CTRL_OFFSET       ((XV_HDMITX1_AUD_BASE)+(1*4)) /**< AUD
                                * Control Register *  offset */
#define XV_HDMITX1_AUD_CTRL_SET_OFFSET   ((XV_HDMITX1_AUD_BASE)+(2*4)) /**< AUD
                                * Control Register Set *  offset */
#define XV_HDMITX1_AUD_CTRL_CLR_OFFSET   ((XV_HDMITX1_AUD_BASE)+(3*4)) /**< AUD
                                * Control Register Clear *  offset */
#define XV_HDMITX1_AUD_STA_OFFSET        ((XV_HDMITX1_AUD_BASE)+(4*4)) /**< AUD
                                * Status Register *  offset */
#define XV_HDMITX1_AUD_ACR_N_OFFSET      ((XV_HDMITX1_AUD_BASE)+(5*4)) /**< AUD
                                * Clock Regeneration CTS *  Register offset */
#define XV_HDMITX1_AUD_ACR_CTS_OFFSET    ((XV_HDMITX1_AUD_BASE)+(6*4)) /**< AUD
                                * Clock Regeneration N *  Register offset */

/* Audio peripheral Control register masks*/
#define XV_HDMITX1_AUD_CTRL_RUN_MASK     (1<<0)  /**< AUD Control Run mask */
#define XV_HDMITX1_AUD_CTRL_IE_MASK      (1<<1)  /**< AUD Control Interrupt
                                                * Enable mask */
#define XV_HDMITX1_AUD_CTRL_CH_MASK      0x03 /**< AUD Control channels mask */
#define XV_HDMITX1_AUD_CTRL_CH_SHIFT     2   /**< AUD Control channels mask */
#define XV_HDMITX1_AUD_CTRL_AUDFMT_MASK  (1<<4) /**< AUD Control AUD Format
                                                * mask */
#define XV_HDMITX1_AUD_CTRL_AUDFMT_SHIFT 4   /**< AUD Control AUD Format
                                              * shift */
#define XV_HDMITX1_AUD_CTRL_AUDRST_MASK  (1<<5)
#define XV_HDMITX1_AUD_CTRL_ACR_EN_MASK  (1<<6)
#define XV_HDMITX1_AUD_CTRL_ACR_SEL_MASK (1<<7)
#define XV_HDMITX1_AUD_CTRL_DATACLK_LNKCLK_RATIO_MASK  0xF
#define XV_HDMITX1_AUD_CTRL_DATACLK_LNKCLK_RATIO_SHIFT 8
#define XV_HDMITX1_AUD_CTRL_AUDCLK_RATIO_MASK  0xF
#define XV_HDMITX1_AUD_CTRL_AUDCLK_RATIO_SHIFT 12

#define XV_HDMITX1_AUD_CTRL_ACR_N_MASK    0xFFFFF
#define XV_HDMITX1_AUD_CTRL_ACR_CTS_MASK  0xFFFFF

/* Audio peripheral Status register masks*/
#define XV_HDMITX1_AUD_STA_IRQ_MASK      (1<<0) /**< AUD Status Interrupt mask */

/* Audio peripheral ACR N register masks*/
#define XV_HDMITX1_AUD_ACR_N_MASK        0xFFFFF /**< AUD ACR N mask */

/* Audio peripheral ACR CTS register masks*/
#define XV_HDMITX1_AUD_ACR_CTS_MASK      0xFFFFF /**< AUD ACR CTS mask */
#define XV_HDMITX1_AUD_ACR_CTS_VLD_MASK  (1<<31) /**< AUD ACR CTS Valid mask */

/* Video Mask (MASK) peripheral register offsets*/
/* The mask is the fifth peripheral on the local bus*/
#define XV_HDMITX1_MASK_BASE              (5*64)
#define XV_HDMITX1_MASK_ID_OFFSET         ((XV_HDMITX1_MASK_BASE)+(0*4)) /**< MASK
                                * Identification Register offset */
#define XV_HDMITX1_MASK_CTRL_OFFSET       ((XV_HDMITX1_MASK_BASE)+(1*4)) /**< MASK
                                * Control Register offset */
#define XV_HDMITX1_MASK_CTRL_SET_OFFSET   ((XV_HDMITX1_MASK_BASE)+(2*4)) /**< MASK
                                * Control Register Set offset */
#define XV_HDMITX1_MASK_CTRL_CLR_OFFSET   ((XV_HDMITX1_MASK_BASE)+(3*4)) /**< MASK
                                * Control Register Clear offset */
#define XV_HDMITX1_MASK_STA_OFFSET        ((XV_HDMITX1_MASK_BASE)+(4*4)) /**< MASK
                                * Status Register offset */
#define XV_HDMITX1_MASK_RED_OFFSET        ((XV_HDMITX1_MASK_BASE)+(5*4)) /**< MASK
                                * Red Component Register offset */
#define XV_HDMITX1_MASK_GREEN_OFFSET        ((XV_HDMITX1_MASK_BASE)+(6*4)) /**< MASK
                                * Green Component Register offset */
#define XV_HDMITX1_MASK_BLUE_OFFSET        ((XV_HDMITX1_MASK_BASE)+(7*4)) /**< MASK
                                * Blue Component Register offset */

/* Video mask peripheral Control register masks*/
#define XV_HDMITX1_MASK_CTRL_RUN_MASK     (1<<0)  /**< MASK Control Run mask */
#define XV_HDMITX1_MASK_CTRL_NOISE_MASK   (1<<2)  /**< MASK Control Noise */

/* Fixed Rate Link (FRL) peripheral register offsets*/
/* The FRL is the sixth peripheral on the local bus*/
#define XV_HDMITX1_FRL_BASE              (6*64)
#define XV_HDMITX1_FRL_ID_OFFSET         ((XV_HDMITX1_FRL_BASE)+(0*4)) /**< FRL
                                * Identification Register offset */
#define XV_HDMITX1_FRL_CTRL_OFFSET       ((XV_HDMITX1_FRL_BASE)+(1*4)) /**< FRL
                                * Control Register offset */
#define XV_HDMITX1_FRL_CTRL_SET_OFFSET   ((XV_HDMITX1_FRL_BASE)+(2*4)) /**< FRL
                                * Control Register Set offset */
#define XV_HDMITX1_FRL_CTRL_CLR_OFFSET   ((XV_HDMITX1_FRL_BASE)+(3*4)) /**< FRL
                                * Control Register Clear offset */
#define XV_HDMITX1_FRL_STA_OFFSET        ((XV_HDMITX1_FRL_BASE)+(4*4)) /**< FRL
                                * Status Register offset */
#define XV_HDMITX1_FRL_TMR_OFFSET        ((XV_HDMITX1_FRL_BASE)+(5*4)) /**< FRL
                                * Timer Register offset */
#define XV_HDMITX1_FRL_LNK_CLK_OFFSET        ((XV_HDMITX1_FRL_BASE)+(6*4)) /**< FRL
                                * Link Clock Register offset */
#define XV_HDMITX1_FRL_VID_CLK_OFFSET        ((XV_HDMITX1_FRL_BASE)+(7*4)) /**< FRL
                                * Video Clock Register offset */
#define XV_HDMITX1_FRL_VP_FIFO_THRD_OFFSET   ((XV_HDMITX1_FRL_BASE)+(8*4)) /**< FRL
                                * Video Packetizer FIFO Threshold Register offset */
#define XV_HDMITX1_FRL_DISP_ERR_INJ_OFFSET   ((XV_HDMITX1_FRL_BASE)+(9*4)) /**< FRL
                                * Disparity Error Injector Register offset */
#define XV_HDMITX1_FRL_FEC_ERR_INJ_OFFSET    ((XV_HDMITX1_FRL_BASE)+(10*4)) /**< FRL
                                * FEC Error Injector Register offset */

/* FRL Control register masks*/
#define XV_HDMITX1_FRL_CTRL_RSTN_MASK      (1<<0)  /**< FRL Control Resetn
                                                  * mask */
#define XV_HDMITX1_FRL_CTRL_IE_MASK        (1<<1)  /**< FRL Control Interrupt
                                                  * Enable mask */
#define XV_HDMITX1_FRL_CTRL_OP_MODE_MASK   (1<<2)  /**< FRL Control Operation
                                                  * Mode mask */
#define XV_HDMITX1_FRL_CTRL_LN_OP_MASK     (1<<3)  /**< FRL Control Lane
                                                  * Operation mask */
#define XV_HDMITX1_FRL_CTRL_EXEC_MASK      (1<<4)  /**< FRL execute
                                                  * mask */
#define XV_HDMITX1_FRL_CTRL_TST_RC_MASK    (1<<5)  /**< FRL RC Compress
                                                  * mask */
#define XV_HDMITX1_FRL_ACT_MASK            (1<<7)  /**< FRL Control Active
                                                  * mask */
#define XV_HDMITX1_FRL_LTP0_REQ_MASK       0xF     /**< FRL Control Lane 0
                                                  * LTP mask */
#define XV_HDMITX1_FRL_LTP0_REQ_SHIFT      8       /**< FRL Control Lane 0
                                                  * LTP shift */
#define XV_HDMITX1_FRL_LTP1_REQ_MASK       0xF     /**< FRL Control Lane 1
                                                  * LTP mask */
#define XV_HDMITX1_FRL_LTP1_REQ_SHIFT      12      /**< FRL Control Lane 1
                                                  * LTP shift */
#define XV_HDMITX1_FRL_LTP2_REQ_MASK       0xF     /**< FRL Control Lane 2
                                                  * LTP mask */
#define XV_HDMITX1_FRL_LTP2_REQ_SHIFT      16      /**< FRL Control Lane 2
                                                  * LTP shift */
#define XV_HDMITX1_FRL_LTP3_REQ_MASK       0xF     /**< FRL Control Lane 3
                                                  * LTP mask */
#define XV_HDMITX1_FRL_LTP3_REQ_SHIFT      20      /**< FRL Control Lane 3
                                                  * LTP shift */
#define XV_HDMITX1_FRL_VCKE_EXT_MASK       (1<<24) /**< FRL Control Lane 3
                                                  * LTP mask */

/* FRL Status register masks*/
#define XV_HDMITX1_FRL_STA_IRQ_MASK          (1<<0) /**< FRL Status Interrupt
                                                * mask */
#define XV_HDMITX1_FRL_STA_TMR_EVT_MASK      (1<<1) /**< FRL Status Timer Event
                                                * mask */
#define XV_HDMITX1_FRL_STA_TMR_ZERO_MASK     (1<<2) /**< FRL Status Timer Zero
                                                * mask */
#define XV_HDMITX1_FRL_STA_FRL_RST_MASK      (1<<3) /**< FRL Status FRL Reset
                                                * mask */
#define XV_HDMITX1_FRL_STA_TRIB_RST_MASK     (1<<4) /**< FRL Status TRIB Reset
                                                * mask */
#define XV_HDMITX1_FRL_STA_LNK_CLK_OOS_MASK  (1<<5) /**< FRL Status Link Clock
                                                * OOS mask */
#define XV_HDMITX1_FRL_STA_VID_CLK_OOS_MASK  (1<<6) /**< FRL Status Video
                                                * Clock OOS mask */
#define XV_HDMITX1_FRL_STA_GB_EP_MASK        (1<<7) /**< FRL Status Gearbox EP
                                                * mask */
#define XV_HDMITX1_FRL_STA_GB_SYNC_ERR_MASK  (1<<8) /**< FRL Status Gearbox
                                                * Sync Error mask */

/* FRL Link Clock register masks*/
#define XV_HDMITX1_FRL_LNK_CLK_MASK       0xFFFFF /**< FRL Link Clock mask */
#define XV_HDMITX1_FRL_VID_CLK_MASK       0xFFFFF /**< FRL Video Clock mask */

#define XV_HDMITX1_FRL_VP_FIFO_THRD_OFFSET   ((XV_HDMITX1_FRL_BASE)+(8*4)) /**< FRL
                                * Video Packetizer FIFO Threshold Register offset */
#define XV_HDMITX1_FRL_DISP_ERR_INJ_OFFSET   ((XV_HDMITX1_FRL_BASE)+(9*4)) /**< FRL
                                * Disparity Error Injector Register offset */
#define XV_HDMITX1_FRL_FEC_ERR_INJ_OFFSET    ((XV_HDMITX1_FRL_BASE)+(10*4)) /**< FRL
                                * FEC Error Injector Register offset */

/* FRL Video Depacketizer FIFO Threshold register masks*/
#define XV_HDMITX1_FRL_VP_FIFO_THRD_PKTZ_FIFO_INIT_FILL_LVL_MASK    0x3F

/* FRL Disparity Error Injector register masks*/
#define XV_HDMITX1_FRL_DISP_ERR_INJ_DISP_ERR_INJ_EN_MASK    (1<<0)
#define XV_HDMITX1_FRL_DISP_ERR_INJ_ERR_TYPE_MASK           0x7
#define XV_HDMITX1_FRL_DISP_ERR_INJ_ERR_TYPE_SHIFT          4
#define XV_HDMITX1_FRL_DISP_ERR_INJ_NUM_ERR_CHAR_MASK       0xFF
#define XV_HDMITX1_FRL_DISP_ERR_INJ_NUM_ERR_CHAR_SHIFT      8
#define XV_HDMITX1_FRL_DISP_ERR_INJ_NUM_ERR_CB_MASK         0xFFFF
#define XV_HDMITX1_FRL_DISP_ERR_INJ_NUM_ERR_CB_SHIFT        16

/* FRL FEC Error Injector register masks*/
#define XV_HDMITX1_FRL_FEC_ERR_INJ_FEC_ERR_INJ_EN_MASK      (1<<0)
#define XV_HDMITX1_FRL_FEC_ERR_INJ_NUM_ERR_CHAR_MASK        0xF
#define XV_HDMITX1_FRL_FEC_ERR_INJ_NUM_ERR_CHAR_SHIFT       4
#define XV_HDMITX1_FRL_FEC_ERR_INJ_NUM_ERR_CB_MASK          0xFF
#define XV_HDMITX1_FRL_FEC_ERR_INJ_NUM_ERR_CB_SHIFT         8
#define XV_HDMITX1_FRL_FEC_ERR_INJ_ERR_CB_LOC_MASK          0x3FF
#define XV_HDMITX1_FRL_FEC_ERR_INJ_ERR_CB_LOC_SHIFT         16

/* Peripheral ID and General shift values.*/
#define XV_HDMITX1_SHIFT_16  16  /**< 16 shift value */
#define XV_HDMITX1_MASK_16   0xFFFF  /**< 16 bit mask value */
#define XV_HDMITX1_PIO_ID    0x2200  /**< TX's PIO ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/* Register access macro definition*/
#define XV_HdmiTx1_In32  Xil_In32    /**< Input Operations */
#define XV_HdmiTx1_Out32 Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a HDMI TX register. A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the HDMI TX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*           u32 XV_HdmiTx1_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XV_HdmiTx1_ReadReg(BaseAddress, RegOffset) \
	XV_HdmiTx1_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a HDMI TX register. A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the HDMI TX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*           void XV_HdmiTx1_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XV_HdmiTx1_WriteReg(BaseAddress, RegOffset, Data) \
	XV_HdmiTx1_Out32((BaseAddress) + (RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
