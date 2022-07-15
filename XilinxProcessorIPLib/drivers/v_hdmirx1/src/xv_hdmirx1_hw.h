/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx HDMI RX core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xv_hdmirx1.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     02/05/19 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_HDMIRX1_HW_H_
#define XV_HDMIRX1_HW_H_     /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* VER (Version Interface) peripheral register offsets*/
#define XV_HDMIRX1_VER_BASE                          (0*64)
#define XV_HDMIRX1_VER_ID_OFFSET                     ((XV_HDMIRX1_VER_BASE)+(0*4))    /**< VER Identification Register offset */
#define XV_HDMIRX1_VER_VERSION_OFFSET                ((XV_HDMIRX1_VER_BASE)+(1*4))    /**< VER Version Register offset */
#define XV_HDMIRX1_VCKE_SYS_CNT_OFFSET               ((XV_HDMIRX1_VER_BASE)+(2*4))    /**< VCKE System Counts Register offset */
#define XV_HDMIRX1_SR_SSB_ERR_CNT0_OFFSET            ((XV_HDMIRX1_VER_BASE)+(3*4))    /**< SR/SSB period error 0 counter register offset */
#define XV_HDMIRX1_SR_SSB_ERR_CNT1_OFFSET            ((XV_HDMIRX1_VER_BASE)+(4*4))    /**< SR/SSB period error 1 counter register offset */
#define XV_HDMIRX1_SR_SSB_ERR_CNT2_OFFSET            ((XV_HDMIRX1_VER_BASE)+(5*4))    /**< SR/SSB period error 2 counter register offset */
#define XV_HDMIRX1_SR_SSB_ERR_CNT3_OFFSET            ((XV_HDMIRX1_VER_BASE)+(6*4))    /**< SR/SSB period error 3 counter register offset */
#define XV_HDMIRX1_DBG_STA_OFFSET                     ((XV_HDMIRX1_VER_BASE)+(7*4))    /**< FRL word aligner tap select changed register offset */

#define XV_HDMIRX1_SR_SSB_ERR1_SHIFT                 0         /**< FRL SR/SSB period error during training period shift */
#define XV_HDMIRX1_SR_SSB_ERR1_MASK                  0xFFFF    /**< FRL SR/SSB period error during training period mask */
#define XV_HDMIRX1_SR_SSB_ERR2_SHIFT                 16        /**< FRL SR/SSB period error during NON-training period shift */
#define XV_HDMIRX1_SR_SSB_ERR2_MASK                  0xFFFF    /**< FRL SR/SSB period error during NON-training period mask */

#define XV_HDMIRX1_DBG_STA_WA_TAP_CHGALL_MASK        0xF       /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_WA_TAP_CHG0_MASK          (1<<0)    /**< Word aligner tap select changed lane 0 mask */
#define XV_HDMIRX1_DBG_STA_WA_TAP_CHG1_MASK          (1<<1)    /**< Word aligner tap select changed lane 1 mask */
#define XV_HDMIRX1_DBG_STA_WA_TAP_CHG2_MASK          (1<<2)    /**< Word aligner tap select changed lane 2 mask */
#define XV_HDMIRX1_DBG_STA_WA_TAP_CHG3_MASK          (1<<3)    /**< FRL Word aligner tap select changed lane 3 mask */
#define XV_HDMIRX1_DBG_STA_WA_LOCK_CHGALL_MASK       0xF       /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_WA_LOCK_CHGALL_SHIFT      4         /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_WA_LOCK_CHG0_MASK         (1<<4)    /**< Word aligner tap select changed lane 0 mask */
#define XV_HDMIRX1_DBG_STA_WA_LOCK_CHG1_MASK         (1<<5)    /**< Word aligner tap select changed lane 1 mask */
#define XV_HDMIRX1_DBG_STA_WA_LOCK_CHG2_MASK         (1<<6)    /**< Word aligner tap select changed lane 2 mask */
#define XV_HDMIRX1_DBG_STA_WA_LOCK_CHG3_MASK         (1<<7)    /**< FRL Word aligner tap select changed lane 3 mask */
#define XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHGALL_MASK     0xF       /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHGALL_SHIFT    8         /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHG0_MASK       (1<<8)    /**< Word aligner tap select changed lane 0 mask */
#define XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHG1_MASK       (1<<9)    /**< Word aligner tap select changed lane 1 mask */
#define XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHG2_MASK       (1<<10)    /**< Word aligner tap select changed lane 2 mask */
#define XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHG3_MASK       (1<<11)    /**< FRL Word aligner tap select changed lane 3 mask */
#define XV_HDMIRX1_DBG_STA_LANE_LOCK_CHGALL_MASK     0xF       /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_LANE_LOCK_CHGALL_SHIFT    12         /**< FRL Word aligner tap select changed all lanes mask */
#define XV_HDMIRX1_DBG_STA_LANE_LOCK_CHG0_MASK       (1<<12)    /**< Word aligner tap select changed lane 0 mask */
#define XV_HDMIRX1_DBG_STA_LANE_LOCK_CHG1_MASK       (1<<13)    /**< Word aligner tap select changed lane 1 mask */
#define XV_HDMIRX1_DBG_STA_LANE_LOCK_CHG2_MASK       (1<<14)    /**< Word aligner tap select changed lane 2 mask */
#define XV_HDMIRX1_DBG_STA_LANE_LOCK_CHG3_MASK       (1<<15)    /**< FRL Word aligner tap select changed lane 3 mask */
#define XV_HDMIRX1_DBG_STA_SKEW_LOCK_CHG_MASK        (1<<16)    /**< Word aligner tap select changed lane 0 mask */

/* PIO (Parallel Interface) peripheral register offsets*/
#define XV_HDMIRX1_PIO_BASE                          (1*64)
#define XV_HDMIRX1_PIO_ID_OFFSET                     ((XV_HDMIRX1_PIO_BASE)+(0*4))    /**< PIO Identification register offset */
#define XV_HDMIRX1_PIO_CTRL_OFFSET                   ((XV_HDMIRX1_PIO_BASE)+(1*4))    /**< PIO Control register offset */
#define XV_HDMIRX1_PIO_CTRL_SET_OFFSET               ((XV_HDMIRX1_PIO_BASE)+(2*4))    /**< PIO Control Register Set offset */
#define XV_HDMIRX1_PIO_CTRL_CLR_OFFSET               ((XV_HDMIRX1_PIO_BASE)+(3*4))    /**< PIO Control Register Clear offset */
#define XV_HDMIRX1_PIO_STA_OFFSET                    ((XV_HDMIRX1_PIO_BASE)+(4*4))    /**< PIO Status Register offset */
#define XV_HDMIRX1_PIO_OUT_OFFSET                    ((XV_HDMIRX1_PIO_BASE)+(5*4))    /**< PIO Out Register offset */
#define XV_HDMIRX1_PIO_OUT_SET_OFFSET                ((XV_HDMIRX1_PIO_BASE)+(6*4))    /**< PIO Out Register Set offset */
#define XV_HDMIRX1_PIO_OUT_CLR_OFFSET                ((XV_HDMIRX1_PIO_BASE)+(7*4))    /**< PIO Out Register Clear offset */
#define XV_HDMIRX1_PIO_OUT_MSK_OFFSET                ((XV_HDMIRX1_PIO_BASE)+(8*4))    /**< PIO Out Mask Register  offset */
#define XV_HDMIRX1_PIO_IN_OFFSET                     ((XV_HDMIRX1_PIO_BASE)+(9*4))    /**< PIO In Register offset */
#define XV_HDMIRX1_PIO_IN_EVT_OFFSET                 ((XV_HDMIRX1_PIO_BASE)+(10*4))   /**< PIO In Event Register offset */
#define XV_HDMIRX1_PIO_IN_EVT_RE_OFFSET              ((XV_HDMIRX1_PIO_BASE)+(11*4))   /**< PIO In Event Rising Edge Register offset */
#define XV_HDMIRX1_PIO_IN_EVT_FE_OFFSET              ((XV_HDMIRX1_PIO_BASE)+(12*4))   /**< PIO In Event Falling Edge Register offset */

#define XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT            ((XV_HDMIRX1_PIO_BASE)+(13*4))   /**< DSC original HFront and Hsync values */
#define XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT             ((XV_HDMIRX1_PIO_BASE)+(14*4))   /**< DSC original HBack and HCActive values */
#define XV_HDMIRX1_DSC_CVTEM_HACT_VACT               ((XV_HDMIRX1_PIO_BASE)+(15*4))   /**< DSC original HActive and VActive values */

/* PIO peripheral Control register masks*/
#define XV_HDMIRX1_PIO_CTRL_RUN_MASK                 (1<<0)  /**< PIO Control Run mask */
#define XV_HDMIRX1_PIO_CTRL_IE_MASK                  (1<<1)  /**< PIO Control Interrupt Enable mask */

/* PIO peripheral Status register masks*/
#define XV_HDMIRX1_PIO_STA_IRQ_MASK                  (1<<0)  /**< PIO Status Interrupt mask */
#define XV_HDMIRX1_PIO_STA_EVT_MASK                  (1<<1)  /**< PIO Status Event mask */

/* PIO peripheral PIO Out register masks and shifts*/
#define XV_HDMIRX1_PIO_OUT_RESET_MASK                (1<<0)  /**< PIO Out Reset mask */
#define XV_HDMIRX1_PIO_OUT_LNK_EN_MASK               (1<<1)  /**< PIO Out video enable mask */
#define XV_HDMIRX1_PIO_OUT_VID_EN_MASK               (1<<2)  /**< PIO Out video enable mask */
#define XV_HDMIRX1_PIO_OUT_HPD_MASK                  (1<<3)  /**< PIO Out Hot-Plug Detect mask */
#define XV_HDMIRX1_PIO_OUT_DEEP_COLOR_MASK           0x30    /**< PIO Out Deep Color mask */
#define XV_HDMIRX1_PIO_OUT_PIXEL_RATE_MASK           0xC0    /**< PIO Out Pixel Rate mask */
#define XV_HDMIRX1_PIO_OUT_SAMPLE_RATE_MASK          0x300   /**< PIO Out Sample Rate mask */
#define XV_HDMIRX1_PIO_OUT_COLOR_SPACE_MASK          0xC00   /**< PIO Out Color Space mask */
#define XV_HDMIRX1_PIO_OUT_PP_MASK                   0x70000 /**< PIO Out Pixel Phase mask */
#define XV_HDMIRX1_PIO_OUT_AXIS_EN_MASK              0x80000 /**< PIO Out Axis Enable mask */
#define XV_HDMIRX1_PIO_OUT_DEEP_COLOR_SHIFT          4       /**< PIO Out Deep Color shift */
#define XV_HDMIRX1_PIO_OUT_PIXEL_RATE_SHIFT          6       /**< PIO Out Pixel Rate Shift */
#define XV_HDMIRX1_PIO_OUT_SAMPLE_RATE_SHIFT         8       /**< PIO Out Sample Rate shift */
#define XV_HDMIRX1_PIO_OUT_COLOR_SPACE_SHIFT         10      /**< PIO Out Color Space shift */
#define XV_HDMIRX1_PIO_OUT_PP_SHIFT                  16      /**< PIO Out Pixel Phase shift */
#define XV_HDMIRX1_PIO_OUT_SCRM_MASK                 (1<<12) /**< PIO Out Scrambler mask */
#define XV_HDMIRX1_PIO_OUT_BRIDGE_YUV420_MASK        (1<<29) /**< PIO Out Bridge_YUV420 mask */
#define XV_HDMIRX1_PIO_OUT_BRIDGE_PIXEL_MASK         (1<<30) /**< PIO Out Bridge_Pixel drop mask */

#define XV_HDMIRX1_PIO_OUT_INT_VRST_MASK             (1<<0)  /**< PIO Out INT_VRST
                                                                * mask */
#define XV_HDMIRX1_PIO_OUT_INT_LRST_MASK             (1<<20) /**< PIO Out INT_LRST
                                                               * mask */
#define XV_HDMIRX1_PIO_OUT_EXT_VRST_MASK             (1<<21) /**< PIO Out EXT_VRST
                                                               * mask */
#define XV_HDMIRX1_PIO_OUT_EXT_SYSRST_MASK           (1<<22) /**< PIO Out EXT_SYSRST
                                                         * mask */
/*
 * This bit is used to flush out the Dynamic HDR Data Mover.
 * Write 1 to flush pending transaction and 0 for normal operation.
 */
#define XV_HDMIRX1_PIO_OUT_DYN_HDR_DM_EN_MASK		(1 << 23) /**< PIO Out Dynamic
								    *  HDR Data Mover
								    *  enable mask */

/* PIO peripheral PIO In register masks*/
#define XV_HDMIRX1_PIO_IN_DET_MASK                   (1<<0) /**< PIO In cable detect mask */
#define XV_HDMIRX1_PIO_IN_LNK_RDY_MASK               (1<<1) /**< PIO In link ready mask */
#define XV_HDMIRX1_PIO_IN_VID_RDY_MASK               (1<<2) /**< PIO In video ready mask */
#define XV_HDMIRX1_PIO_IN_MODE_MASK                  (1<<3) /**< PIO In Mode mask */
#define XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCKALLL_MASK    0xF    /**< PIO In Scrambler lock all lanes mask */
#define XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCKALLL_SHIFT   4      /**< PIO In Scrambler lock all lanes shift */
#define XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCK0_MASK       (1<<4) /**< PIO In Scrambler lock 0 mask */
#define XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCK1_MASK       (1<<5) /**< PIO In Scrambler lock 1 mask */
#define XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCK2_MASK       (1<<6) /**< PIO In Scrambler lock 2 mask */
#define XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK (1<<7) /**< PIO In SCDC scrambler enable mask */
#define XV_HDMIRX1_PIO_IN_SCDC_TMDS_CLOCK_RATIO_MASK (1<<8) /**< PIO In SCDC TMDS clock ratio mask */
#define XV_HDMIRX1_PIO_IN_ALIGNER_LOCK_MASK          (1<<9) /**< PIO In alinger lock mask */
#define XV_HDMIRX1_PIO_IN_BRDG_OVERFLOW_MASK         (1<<10) /**< PIO In bridge overflow mask */
#define XV_HDMIRX1_PIO_IN_DSC_EN_STRM_MASK           (1 << 11) /**< PIO In DSC packets present in stream */
#define XV_HDMIRX1_PIO_IN_DSC_EN_STRM_CHG_EVT_MASK   (1 << 11) /**< This bit is present in PIO_IN_EVT reg only.
								    It is set by IP when the DSC packets are present in stream. */
#define XV_HDMIRX1_PIO_IN_DSC_PPS_PKT_ERR_MASK       (1 << 12) /**< This bit is preset in PIO_IN_EVT reg only.
								     This bit is set when DSC packet errors are present. */

#define XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HFRONT_MASK	(0xFFFF)	/**< DSC original HFRONT mask */
#define XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HFRONT_SHIFT	(0)		/**< DSC original HFRONT shift */
#define XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HSYNC_MASK	(0xFFFF)	/**< DSC original HSYNC mask */
#define XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HSYNC_SHIFT	(16)		/**< DSC original HSYNC shift */
#define XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT_HBACK_MASK		(0xFFFF)	/**< DSC original HBACK mask */
#define XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT_HBACK_SHIFT		(0)		/**< DSC original HBACK shift */
#define XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT_HCACT_MASK		(0xFFFF)	/**< DSC original HCACT mask */
#define XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT_HCACT_SHIFT		(16)		/**< DSC original HCACT shift */
#define XV_HDMIRX1_DSC_CVTEM_HACT_VACT_HACT_MASK		(0xFFFF)	/**< DSC original HACT mask */
#define XV_HDMIRX1_DSC_CVTEM_HACT_VACT_HACT_SHIFT		(16)		/**< DSC original HACT shift */
#define XV_HDMIRX1_DSC_CVTEM_HACT_VACT_VACT_MASK		(0xFFFF)	/**< DSC original VACT mask */
#define XV_HDMIRX1_DSC_CVTEM_HACT_VACT_VACT_SHIFT		(0)		/**< DSC original VACT shift */


/* Timer peripheral register offsets*/
#define XV_HDMIRX1_TMR_BASE                          (2*64)
#define XV_HDMIRX1_TMR_ID_OFFSET                     ((XV_HDMIRX1_TMR_BASE)+(0*4))    /**< TMR Identification register offset */
#define XV_HDMIRX1_TMR_CTRL_OFFSET                   ((XV_HDMIRX1_TMR_BASE)+(1*4))    /**< TMR Control register offset */
#define XV_HDMIRX1_TMR_CTRL_SET_OFFSET               ((XV_HDMIRX1_TMR_BASE)+(2*4))    /**< TMR Control Register Set offset */
#define XV_HDMIRX1_TMR_CTRL_CLR_OFFSET               ((XV_HDMIRX1_TMR_BASE)+(3*4))    /**< TMR Control Register Clear offset */
#define XV_HDMIRX1_TMR_STA_OFFSET                    ((XV_HDMIRX1_TMR_BASE)+(4*4))    /**< TMR Status Register offset */
#define XV_HDMIRX1_TMR1_CNT_OFFSET                   ((XV_HDMIRX1_TMR_BASE)+(5*4))    /**< TMR Counter Register offset */
#define XV_HDMIRX1_TMR2_CNT_OFFSET                   ((XV_HDMIRX1_TMR_BASE)+(6*4))    /**< TMR Counter Register offset */
#define XV_HDMIRX1_TMR3_CNT_OFFSET                   ((XV_HDMIRX1_TMR_BASE)+(7*4))    /**< TMR Counter Register offset */
#define XV_HDMIRX1_TMR4_CNT_OFFSET                   ((XV_HDMIRX1_TMR_BASE)+(8*4))    /**< TMR Counter Register offset */

/* Timer peripheral Control register masks*/
#define XV_HDMIRX1_TMR1_CTRL_RUN_MASK                 (1<<0)  /**< TMR Control Run mask */
#define XV_HDMIRX1_TMR1_CTRL_IE_MASK                  (1<<1)  /**< TMR Control Interrupt Enable mask */
#define XV_HDMIRX1_TMR2_CTRL_RUN_MASK                 (1<<2)  /**< TMR Control Run mask */
#define XV_HDMIRX1_TMR2_CTRL_IE_MASK                  (1<<3)  /**< TMR Control Interrupt Enable mask */
#define XV_HDMIRX1_TMR3_CTRL_RUN_MASK                 (1<<4)  /**< TMR Control Run mask */
#define XV_HDMIRX1_TMR3_CTRL_IE_MASK                  (1<<5)  /**< TMR Control Interrupt Enable mask */
#define XV_HDMIRX1_TMR4_CTRL_RUN_MASK                 (1<<6)  /**< TMR Control Run mask */
#define XV_HDMIRX1_TMR4_CTRL_IE_MASK                  (1<<7)  /**< TMR Control Interrupt Enable mask */

/* Timer peripheral Status register masks*/
#define XV_HDMIRX1_TMR_STA_IRQ_MASK                   (1<<0)  /**< TMR Status Interrupt mask */
#define XV_HDMIRX1_TMR1_STA_CNT_EVT_MASK              (1<<1)  /**< TMR Status counter Event mask */
#define XV_HDMIRX1_TMR2_STA_CNT_EVT_MASK              (1<<3)  /**< TMR Status counter Event mask */
#define XV_HDMIRX1_TMR3_STA_CNT_EVT_MASK              (1<<5)  /**< TMR Status counter Event mask */
#define XV_HDMIRX1_TMR4_STA_CNT_EVT_MASK              (1<<7)  /**< TMR Status counter Event mask */

/* Video Timing Detector (VTD) peripheral register offsets.*/
#define XV_HDMIRX1_VTD_BASE                          (3*64)
#define XV_HDMIRX1_VTD_ID_OFFSET                     ((XV_HDMIRX1_VTD_BASE)+(0*4))    /**< VTD Identification Register offset */
#define XV_HDMIRX1_VTD_CTRL_OFFSET                   ((XV_HDMIRX1_VTD_BASE)+(1*4))    /**< VTD Control Register offset */
#define XV_HDMIRX1_VTD_CTRL_SET_OFFSET               ((XV_HDMIRX1_VTD_BASE)+(2*4))    /**< VTD Control Set Register offset */
#define XV_HDMIRX1_VTD_CTRL_CLR_OFFSET               ((XV_HDMIRX1_VTD_BASE)+(3*4))    /**< VTD Control Clear Register offset */
#define XV_HDMIRX1_VTD_STA_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(4*4))    /**< VTD Status Register offset */
#define XV_HDMIRX1_VTD_TOT_PIX_OFFSET                ((XV_HDMIRX1_VTD_BASE)+(5*4))    /**< VTD Total Pixels Register offset */
#define XV_HDMIRX1_VTD_ACT_PIX_OFFSET                ((XV_HDMIRX1_VTD_BASE)+(6*4))    /**< VTD Active Pixels Register offset */
#define XV_HDMIRX1_VTD_TOT_LIN_OFFSET                ((XV_HDMIRX1_VTD_BASE)+(7*4))    /**< VTD Total Lines Register offset */
#define XV_HDMIRX1_VTD_ACT_LIN_OFFSET                ((XV_HDMIRX1_VTD_BASE)+(8*4))    /**< VTD Active Lines Register offset */
#define XV_HDMIRX1_VTD_VSW_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(9*4))    /**< VTD Vertical Sync Width Register offset */
#define XV_HDMIRX1_VTD_HSW_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(10*4))   /**< VTD Horizontal Sync Width Register offset */
#define XV_HDMIRX1_VTD_VFP_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(11*4))   /**< VTD Vertical Front Porch Register offset */
#define XV_HDMIRX1_VTD_VBP_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(12*4))   /**< VTD Vertical Back Porch Register offset */
#define XV_HDMIRX1_VTD_HFP_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(13*4))   /**< VTD Horizontal Front Porch Register offset */
#define XV_HDMIRX1_VTD_HBP_OFFSET                    ((XV_HDMIRX1_VTD_BASE)+(14*4))   /**< VTD Horizontal Back Porch Register offset */

/* Video timing detector peripheral Control register masks and shift*/
#define XV_HDMIRX1_VTD_CTRL_RUN_MASK                 (1<<0)  	/**< VTD Control Run mask */
#define XV_HDMIRX1_VTD_CTRL_IE_MASK                  (1<<1)  	/**< VTD Control Interrupt Enable mask */
#define XV_HDMIRX1_VTD_CTRL_FIELD_POL_MASK           (1<<2)  	/**< VTD Control field polarity mask */
#define XV_HDMIRX1_VTD_CTRL_SYNC_LOSS_MASK           (1<<3)    /**< VTD Control field polarity mask */
#define XV_HDMIRX1_VTD_CTRL_VFP_ENABLE_MASK          (1<<4)    /**< VTD VFP change interrupt enable mask */
#define XV_HDMIRX1_VTD_CTRL_TIMEBASE_SHIFT           8      		/**< VTD Control timebase shift */
#define XV_HDMIRX1_VTD_CTRL_TIMERBASE_MASK           0xffffff    /**< VTD Control timebase mask */

/* Video timing detector peripheral Status register masks*/
#define XV_HDMIRX1_VTD_STA_IRQ_MASK                  (1<<0)  /**< VTD Status Interrupt mask */
#define XV_HDMIRX1_VTD_STA_TIMEBASE_EVT_MASK         (1<<1)  /**< VTD Status timebase event mask */
#define XV_HDMIRX1_VTD_STA_VS_POL_MASK               (1<<3)  /**< VTD Status Vsync Polarity mask */
#define XV_HDMIRX1_VTD_STA_HS_POL_MASK               (1<<4)  /**< VTD Status Hsync Polarity mask */
#define XV_HDMIRX1_VTD_STA_FMT_MASK                  (1<<5)  /**< VTD Status Format mask */
#define XV_HDMIRX1_VTD_STA_SYNC_LOSS_EVT_MASK        (1<<6)  /**< VTD Status Sync Loss mask */
#define XV_HDMIRX1_VTD_STA_VFP_CH_EVT_MASK           (1<<7)  /**< VTD Status Vfp value chage mask */

/* DDC (Display Data Channel) peripheral register offsets.*/
#define XV_HDMIRX1_DDC_BASE                          (4*64)
#define XV_HDMIRX1_DDC_ID_OFFSET                     ((XV_HDMIRX1_DDC_BASE)+(0*4))    /**< DDC Identification Register offset */
#define XV_HDMIRX1_DDC_CTRL_OFFSET                   ((XV_HDMIRX1_DDC_BASE)+(1*4))    /**< DDC Control Register offset */
#define XV_HDMIRX1_DDC_CTRL_SET_OFFSET               ((XV_HDMIRX1_DDC_BASE)+(2*4))    /**< DDC Control Register Set offset */
#define XV_HDMIRX1_DDC_CTRL_CLR_OFFSET               ((XV_HDMIRX1_DDC_BASE)+(3*4))    /**< DDC Control Register Clear offset */
#define XV_HDMIRX1_DDC_STA_OFFSET                    ((XV_HDMIRX1_DDC_BASE)+(4*4))    /**< DDC Status Register offset */
#define XV_HDMIRX1_DDC_EDID_STA_OFFSET               ((XV_HDMIRX1_DDC_BASE)+(5*4))    /**< DDC EDID Status Register offset */
#define XV_HDMIRX1_DDC_HDCP_STA_OFFSET               ((XV_HDMIRX1_DDC_BASE)+(6*4))    /**< DDC HDCP Status Register offset */
#define XV_HDMIRX1_DDC_EDID_SP_OFFSET                ((XV_HDMIRX1_DDC_BASE)+(8*4))    /**< DDC Read EDID segment pointer offset */
#define XV_HDMIRX1_DDC_EDID_WP_OFFSET                ((XV_HDMIRX1_DDC_BASE)+(9*4))    /**< DDC Read EDID write pointer offset */
#define XV_HDMIRX1_DDC_EDID_RP_OFFSET                ((XV_HDMIRX1_DDC_BASE)+(10*4))   /**< DDC Read EDID read pointer offset */
#define XV_HDMIRX1_DDC_EDID_DATA_OFFSET              ((XV_HDMIRX1_DDC_BASE)+(11*4))   /**< DDC Read EDID data offset */
#define XV_HDMIRX1_DDC_HDCP_ADDRESS_OFFSET           ((XV_HDMIRX1_DDC_BASE)+(12*4))   /**< DDC Read HDCP address offset */
#define XV_HDMIRX1_DDC_HDCP_DATA_OFFSET              ((XV_HDMIRX1_DDC_BASE)+(13*4))   /**< DDC Read HDCP data offset */

/* DDC peripheral Control register masks*/
#define XV_HDMIRX1_DDC_CTRL_RUN_MASK                 (1<<0)  /**< DDC Control Run mask */
#define XV_HDMIRX1_DDC_CTRL_IE_MASK                  (1<<1)  /**< DDC Control Interrupt enable mask */
#define XV_HDMIRX1_DDC_CTRL_EDID_EN_MASK             (1<<2)  /**< DDC Control EDID enable mask */
#define XV_HDMIRX1_DDC_CTRL_SCDC_EN_MASK             (1<<3)  /**< DDC Control SCDC enable mask */
#define XV_HDMIRX1_DDC_CTRL_HDCP_EN_MASK             (1<<4)  /**< DDC Control HDCP enable mask */
#define XV_HDMIRX1_DDC_CTRL_SCDC_CLR_MASK            (1<<5)  /**< DDC Control SCDC clear mask */
#define XV_HDMIRX1_DDC_CTRL_WMSG_CLR_MASK            (1<<6)  /**< DDC Control write message clear mask */
#define XV_HDMIRX1_DDC_CTRL_RMSG_CLR_MASK            (1<<7)  /**< DDC Control read message clear mask */
#define XV_HDMIRX1_DDC_CTRL_HDCP_MODE_MASK           (1<<8)  /**< DDC Control HDCP mode mask */
#define XV_HDMIRX1_DDC_CTRL_SCDC_RD_WR_EVT_EN_MASK   (1<<9)  /**< DDC Control SCDC Read Write Event mask */

/* DDC peripheral Status register masks*/
#define XV_HDMIRX1_DDC_STA_IRQ_MASK                  (1<<0)  /**< DDC Status Interrupt mask */
#define XV_HDMIRX1_DDC_STA_EVT_MASK                  (1<<1)  /**< DDC Status Event mask */
#define XV_HDMIRX1_DDC_STA_BUSY_MASK                 (1<<2)  /**< DDC Status Busy mask */
#define XV_HDMIRX1_DDC_STA_SCL_MASK                  (1<<3)  /**< DDC Status state of the SCL input mask */
#define XV_HDMIRX1_DDC_STA_SDA_MASK                  (1<<4)  /**< DDC Status state of the SDA input mask */
#define XV_HDMIRX1_DDC_STA_HDCP_AKSV_EVT_MASK        (1<<5)  /**< DDC Status HDCP AKSV event mask */
#define XV_HDMIRX1_DDC_STA_HDCP_WMSG_NEW_EVT_MASK    (1<<6)  /**< DDC Status HDCP write message buffer new event mask */
#define XV_HDMIRX1_DDC_STA_HDCP_RMSG_END_EVT_MASK    (1<<7)  /**< DDC Status HDCP read message buffer end event mask */
#define XV_HDMIRX1_DDC_STA_HDCP_RMSG_NC_EVT_MASK     (1<<8)  /**< DDC Status HDCP read message buffer not completed event mask */
#define XV_HDMIRX1_DDC_STA_HDCP_1_PROT_MASK          (1<<9)  /**< DDC Status HDCP 1.4 protocol flag */
#define XV_HDMIRX1_DDC_STA_HDCP_2_PROT_MASK          (1<<10) /**< DDC Status HDCP 2.2 protocol flag */
#define XV_HDMIRX1_DDC_STA_HDCP_1_PROT_EVT_MASK      (1<<11) /**< DDC Status HDCP 1.4 protocol event flag */
#define XV_HDMIRX1_DDC_STA_HDCP_2_PROT_EVT_MASK      (1<<12) /**< DDC Status HDCP 2.2 protocol event flag */
#define XV_HDMIRX1_DDC_STA_SCDC_RD_WR_EVT_MASK       (1<<13) /**< DDC Status SCDC Read Write event flag */
#define XV_HDMIRX1_DDC_STA_SCDC_DSC_STS_UPDT_EVT_MASK (1<<14) /**< DDC Status 0x10 SCDC reg bit 0 Status_Update set by sink event flag */
#define XV_HDMIRX1_DDC_STA_EDID_WORDS_SHIFT          0       /**< DDC Status EDID words shift */
#define XV_HDMIRX1_DDC_STA_EDID_WORDS_MASK           0xFFFF  /**< DDC Status EDID words mask */
#define XV_HDMIRX1_DDC_STA_HDCP_WMSG_WORDS_MASK      0x7FF   /**< DDC Status HDCP 2.2 write message buffer words mask */
#define XV_HDMIRX1_DDC_STA_HDCP_WMSG_WORDS_SHIFT     0       /**< DDC Status HDCP 2.2 write message buffer words shift */
#define XV_HDMIRX1_DDC_STA_HDCP_WMSG_EP_MASK         (1<<11) /**< DDC Status HDCP 2.2 write message buffer empty mask */
#define XV_HDMIRX1_DDC_STA_HDCP_RMSG_WORDS_MASK      0x7FF   /**< DDC Status HDCP 2.2 read message buffer words mask */
#define XV_HDMIRX1_DDC_STA_HDCP_RMSG_WORDS_SHIFT     16      /**< DDC Status HDCP 2.2 read message buffer words shift */
#define XV_HDMIRX1_DDC_STA_HDCP_RMSG_EP_MASK         (1<<27) /**< DDC Status HDCP 2.2 read message buffer empty mask */

/* Auxiliary (AUX) peripheral register offsets.*/
#define XV_HDMIRX1_AUX_BASE                          (5*64)
#define XV_HDMIRX1_AUX_ID_OFFSET                     ((XV_HDMIRX1_AUX_BASE)+(0*4))    /**< AUX Identification Register offset */
#define XV_HDMIRX1_AUX_CTRL_OFFSET                   ((XV_HDMIRX1_AUX_BASE)+(1*4))    /**< AUX Control Register offset */
#define XV_HDMIRX1_AUX_CTRL_SET_OFFSET               ((XV_HDMIRX1_AUX_BASE)+(2*4))    /**< AUX Control Register Set offset */
#define XV_HDMIRX1_AUX_CTRL_CLR_OFFSET               ((XV_HDMIRX1_AUX_BASE)+(3*4))    /**< AUX Control Register Clear offset */
#define XV_HDMIRX1_AUX_STA_OFFSET                    ((XV_HDMIRX1_AUX_BASE)+(4*4))    /**< AUX Status Register offset */
#define XV_HDMIRX1_AUX_DAT_OFFSET                    ((XV_HDMIRX1_AUX_BASE)+(5*4))    /**< AUX Data Register offset */
#define XV_HDMIRX1_AUX_VTEM_OFFSET                   ((XV_HDMIRX1_AUX_BASE)+(7*4))    /**< AUX VTEM Register offset */
#define XV_HDMIRX1_AUX_FSYNC_OFFSET                  ((XV_HDMIRX1_AUX_BASE)+(8*4))    /**< AUX FSYNC Register offset */
#define XV_HDMIRX1_AUX_FSYNC_PRO_OF                  ((XV_HDMIRX1_AUX_BASE)+(9*4))    /**< AUX FYNC PRO Register offset */
#define XV_HDMIRX1_AUX_DYN_HDR_INFO_OFFSET		((XV_HDMIRX1_AUX_BASE) + (10 * 4))    /**< AUX Dynamic HDR Info offset */
#define XV_HDMIRX1_AUX_DYN_HDR_STS_OFFSET		((XV_HDMIRX1_AUX_BASE) + (11 * 4))    /**< AUX Dynamic HDR Status offset */
#define XV_HDMIRX1_AUX_DYN_HDR_MEMADDR_LSB_OFFSET	((XV_HDMIRX1_AUX_BASE) + (12 * 4))    /**< AUX Lower address Dynamic HDR Status offset */
#define XV_HDMIRX1_AUX_DYN_HDR_MEMADDR_MSB_OFFSET	((XV_HDMIRX1_AUX_BASE) + (13 * 4))    /**< AUX Higher address Dynamic HDR Status offset */

/* AUX peripheral Control register masks*/
#define XV_HDMIRX1_AUX_CTRL_RUN_MASK                 (1<<0)  /**< AUX Control Run mask */
#define XV_HDMIRX1_AUX_CTRL_IE_MASK                  (1<<1)  /**< AUX Control Interrupt Enable mask */
#define XV_HDMIRX1_AUX_CTRL_FSYNC_VRR_CH_EVT_MASK    (1<<2)  /**< AUX Control FSync/VRR change event enable mask */

/* AUX peripheral Status register masks and shifts*/
#define XV_HDMIRX1_AUX_STA_IRQ_MASK                  (1<<0)  /**< AUX Status Interrupt mask */
#define XV_HDMIRX1_AUX_STA_NEW_MASK                  (1<<1)  /**< AUX Status New Packet mask */
#define XV_HDMIRX1_AUX_STA_ERR_MASK                  (1<<2)  /**< AUX Status New Packet mask */
#define XV_HDMIRX1_AUX_STA_AVI_MASK                  (1<<3)  /**< AUX Status AVI infoframe mask */
#define XV_HDMIRX1_AUX_STA_GCP_MASK                  (1<<4)  /**< AUX Status General control packet mask */
#define XV_HDMIRX1_AUX_STA_FIFO_EP_MASK              (1<<5)  /**< AUX Status FIFO Empty mask */
#define XV_HDMIRX1_AUX_STA_FIFO_FL_MASK              (1<<6)  /**< AUX Status FIFO Full mask */
#define XV_HDMIRX1_AUX_STA_DYN_HDR_EVT_MASK	(1 << 20) /**< AUX Status Dynamic HDR packet received event mask */
#define XV_HDMIRX1_AUX_STA_VRR_CD_EVT_MASK           (1<<21) /**< AUX Status VRR CD mask */
#define XV_HDMIRX1_AUX_STA_FSYNC_CD_EVT_MASK         (1<<22) /**< AUX Status FSYNC CD mask */
#define XV_HDMIRX1_AUX_STA_GCP_CD_EVT_MASK           (1<<25) /**< AUX Status GCP ColorDepth mask */
#define XV_HDMIRX1_AUX_STA_GCP_AVMUTE_MASK           (1<<31) /**< AUX Status GCP avmute mask */
#define XV_HDMIRX1_AUX_STA_AVI_VIC_MASK              0xFF    /**< AUX Status AVI VIC mask */
#define XV_HDMIRX1_AUX_STA_AVI_CS_MASK               0x03    /**< AUX Status AVI colorspace mask */
#define XV_HDMIRX1_AUX_STA_GCP_CD_MASK               0x03    /**< AUX Status GCP colordepth mask */
#define XV_HDMIRX1_AUX_STA_GCP_PP_MASK               0x07    /**< AUX Status GCP pixel phase mask */
#define XV_HDMIRX1_AUX_STA_AVI_VIC_SHIFT             8       /**< AUX Status AVI VIC Shift */
#define XV_HDMIRX1_AUX_STA_AVI_CS_SHIFT              16      /**< AUX Status AVI colorspace Shift */
#define XV_HDMIRX1_AUX_STA_FSYNC_RDY_SHIFT           24      /**< AUX Status FSYNC RDY shift */
#define XV_HDMIRX1_AUX_STA_VRR_RDY_SHIFT             23      /**< AUX Status VRR RDY shift */
#define XV_HDMIRX1_AUX_STA_GCP_CD_SHIFT              26      /**< AUX Status GCP colordepth Shift */
#define XV_HDMIRX1_AUX_STA_GCP_PP_SHIFT              28      /**< AUX Status GCP pixel phase Shift */

/* AUX VTEM register masks and shifts*/
#define XV_HDMIRX1_AUX_VTEM_VRR_EN_MASK              (1<<0)    /**< AUX VTEM VRR Enable mask */
#define XV_HDMIRX1_AUX_VTEM_M_CONST_MASK             (1<<1)    /**< AUX VTEM M_CONST mask */
#define XV_HDMIRX1_AUX_VTEM_FVA_FACT_M1_MASK         ((0xF) << 2)   /**< AUX VTEM FVA Factor minus 1 mask */
#define XV_HDMIRX1_AUX_VTEM_RB_MASK                  (1<<26)    /**< AUX VTEM Reduced blanking mask */
#define XV_HDMIRX1_AUX_VTEM_BASE_VFRONT_MASK         ((0xFF) << 8)   /**< AUX VTEM Reduced blanking mask */
#define XV_HDMIRX1_AUX_VTEM_BASE_REFRESH_RATE_MASK   ((0x3FF) << 16) /**< AUX VTEM Base refresh rate mask */
#define XV_HDMIRX1_AUX_VTEM_QMS_EN_MASK              (1<<3) /**< AUX VTEM QMS Enable mask */
#define XV_HDMIRX1_AUX_VTEM_NEXT_TFR_MASK            ((0x1F) << 27) /**< AUX VTEM Next transfer rate mask */
#define XV_HDMIRX1_AUX_VTEM_M_CONST_SHIFT            1
#define XV_HDMIRX1_AUX_VTEM_FVA_FACT_M1_SHIFT        2
#define XV_HDMIRX1_AUX_VTEM_QMS_EN_SHIFT	     3
#define XV_HDMIRX1_AUX_VTEM_RB_SHIFT                 26
#define XV_HDMIRX1_AUX_VTEM_NEXT_TFR_SHIFT	     27
#define XV_HDMIRX1_AUX_VTEM_BASE_VFRONT_SHIFT        8
#define XV_HDMIRX1_AUX_VTEM_BASE_REFRESH_RATE_SHIFT  16


/* AUX FSYNC register masks and shifts*/
#define XV_HDMIRX1_AUX_FSYNC_VERSION_MASK            0xFF    /**< AUX FSYNC Version mask */
#define XV_HDMIRX1_AUX_FSYNC_SUPPORT_MASK            (1<<8)    /**< AUX FSYNC Support mask */
#define XV_HDMIRX1_AUX_FSYNC_ENABLED_MASK            (1<<9)    /**< AUX FSYNC Enabled mask */
#define XV_HDMIRX1_AUX_FSYNC_ACTIVE_MASK             (1<<10)    /**< AUX FSYNC Active mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_NTV_CS_ACT_MASK     (1<<11)    /**< AUX FSYNC Pro Native Color space active mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_ACT_MASK    (1<<12)    /**< AUX FSYNC Pro Brightness Control Active mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_LDIMM_CTRL_ACT_MASK     (1<<13)    /**< AUX FSYNC Pro Seamless Local Dimming Disable Control mask */
#define XV_HDMIRX1_AUX_FSYNC_MIN_REF_RATE_MASK       (0xFF << 16)    /**< AUX FSYNC FreeSync Minimum refresh rate mask */
#define XV_HDMIRX1_AUX_FSYNC_MAX_REF_RATE_MASK       (0xFF << 24)    /**< AUX FSYNC FreeSync Maximum refresh rate mask */
#define XV_HDMIRX1_AUX_FSYNC_SUPPORT_SHIFT            8
#define XV_HDMIRX1_AUX_FSYNC_ENABLED_SHIFT            9
#define XV_HDMIRX1_AUX_FSYNC_ACTIVE_SHIFT             10
#define XV_HDMIRX1_AUX_FSYNC_PRO_NTV_CS_ACT_SHIFT     11
#define XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_ACT_SHIFT    12
#define XV_HDMIRX1_AUX_FSYNC_PRO_LDIMM_CTRL_ACT_SHIFT     13
#define XV_HDMIRX1_AUX_FSYNC_MIN_REF_RATE_SHIFT       16
#define XV_HDMIRX1_AUX_FSYNC_MAX_REF_RATE_SHIFT       24


/* AUX FSYNC PRO register masks and shifts*/
#define XV_HDMIRX1_AUX_FSYNC_PRO_SRGB_EOTF_MASK      1         /**< AUX FSYNC SRGB_EOTF mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_BT709_EOTF_MASK     (1<<1)    /**< AUX FSYNC BT709_EOTF mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_2_EOTF_MASK (1<<2)    /**< AUX FSYNC GAMMA_2_2_EOTF mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_6_EOTF_MASK (1<<3)    /**< AUX FSYNC GAMMA_2_6_EOTF mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_PQ_EOTF_MASK        (1<<4)    /**< AUX FSYNC PQ_EOTF mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_MASK    (0xFF << 16)    /**< AUX FSYNC BRIGHT_CTRL mask */
#define XV_HDMIRX1_AUX_FSYNC_PRO_BT709_EOTF_SHIFT     1
#define XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_2_EOTF_SHIFT 2
#define XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_6_EOTF_SHIFT 3
#define XV_HDMIRX1_AUX_FSYNC_PRO_PQ_EOTF_SHIFT        4
#define XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_SHIFT    16

/* AUX Dynamic HDR Info register masks and shifts */
#define XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_LEN_MASK	(0xffff << 16)
#define XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_TYPE_MASK	(0xffff)
#define XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_LEN_SHIFT	(16)
#define XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_TYPE_SHIFT	(0)

/* AUX Dynamic HDR Status register masks and shifts */
#define XV_HDMIRX1_AUX_DYN_HDR_STS_GOF_MASK	(1)		/**< Graphics Overlay Flag */
#define XV_HDMIRX1_AUX_DYN_HDR_STS_ERR_MASK	(0x3 << 1)	/**< Errors asserted while writing ot memory */
#define XV_HDMIRX1_AUX_DYN_HDR_STS_GOF_SHIFT	(0)
#define XV_HDMIRX1_AUX_DYN_HDR_STS_ERR_SHIFT	(1)

/* Audio (AUD) peripheral register offsets.*/
#define XV_HDMIRX1_AUD_BASE                          (6*64)
#define XV_HDMIRX1_AUD_ID_OFFSET                     ((XV_HDMIRX1_AUD_BASE)+(0*4))    /**< AUD Identification Register offset */
#define XV_HDMIRX1_AUD_CTRL_OFFSET                   ((XV_HDMIRX1_AUD_BASE)+(1*4))    /**< AUD Control Register offset */
#define XV_HDMIRX1_AUD_CTRL_SET_OFFSET               ((XV_HDMIRX1_AUD_BASE)+(2*4))    /**< AUD Control Register Set offset */
#define XV_HDMIRX1_AUD_CTRL_CLR_OFFSET               ((XV_HDMIRX1_AUD_BASE)+(3*4))    /**< AUD Control Register Clear offset */
#define XV_HDMIRX1_AUD_STA_OFFSET                    ((XV_HDMIRX1_AUD_BASE)+(4*4))    /**< AUD Status Register offset */
#define XV_HDMIRX1_AUD_CTS_OFFSET                    ((XV_HDMIRX1_AUD_BASE)+(5*4))    /**< AUD CTS Register offset */
#define XV_HDMIRX1_AUD_N_OFFSET                      ((XV_HDMIRX1_AUD_BASE)+(6*4))    /**< AUD N Register offset */

/* Audio peripheral Control register masks*/
#define XV_HDMIRX1_AUD_CTRL_RUN_MASK                 (1<<0)  /**< AUD Control Run mask */
#define XV_HDMIRX1_AUD_CTRL_IE_MASK                  (1<<1)  /**< AUD Control Interrupt Enable mask */
#define XV_HDMIRX1_AUD_CTRL_ACR_UPD_EVT_EN_MASK      (1<<2)  /**< AUD Control ACR Update Event Enable mask */

/* AUD peripheral Status register masks and shift*/
#define XV_HDMIRX1_AUD_STA_IRQ_MASK                  (1<<0)  /**< AUD Status Interrupt mask */
#define XV_HDMIRX1_AUD_STA_ACT_EVT_MASK              (1<<1)  /**< AUD Status Event mask */
#define XV_HDMIRX1_AUD_STA_CH_EVT_MASK               (1<<2)  /**< AUD Status Event mask */
#define XV_HDMIRX1_AUD_STA_ACT_MASK                  (1<<3)  /**< AUD Status Active mask */
#define XV_HDMIRX1_AUD_STA_AUD_CH_MASK               0x03    /**< AUD Status Audio channel mask */
#define XV_HDMIRX1_AUD_STA_AUD_CH_SHIFT              4       /**< AUD Status Audio channel Shift */
#define XV_HDMIRX1_AUD_STA_3DAUD_CH_MASK               0x07    /**< AUD Status Audio channel mask */
#define XV_HDMIRX1_AUD_STA_3DAUD_CH_SHIFT              10       /**< AUD Status Audio channel Shift */
#define XV_HDMIRX1_AUD_STA_AUD_FMT_MASK              0x07    /**< AUD Status Audio Format mask */
#define XV_HDMIRX1_AUD_STA_AUD_FMT_SHIFT             6       /**< AUD Status Audio Format Shift */
#define XV_HDMIRX1_AUD_STA_ACR_UPD_MASK              (1<<9)  /**< AUD Status ACR Update mask */

/* Link Status (LNKSTA) peripheral register offsets.*/
#define XV_HDMIRX1_LNKSTA_BASE                       (7*64)
#define XV_HDMIRX1_LNKSTA_ID_OFFSET                  ((XV_HDMIRX1_LNKSTA_BASE)+(0*4)) /**< LNKSTA Identification Register offset */
#define XV_HDMIRX1_LNKSTA_CTRL_OFFSET                ((XV_HDMIRX1_LNKSTA_BASE)+(1*4)) /**< LNKSTA Control Register offset */
#define XV_HDMIRX1_LNKSTA_CTRL_SET_OFFSET            ((XV_HDMIRX1_LNKSTA_BASE)+(2*4)) /**< LNKSTA Control Register Set offset */
#define XV_HDMIRX1_LNKSTA_CTRL_CLR_OFFSET            ((XV_HDMIRX1_LNKSTA_BASE)+(3*4)) /**< LNKSTA Control Register Clear offset */
#define XV_HDMIRX1_LNKSTA_STA_OFFSET                 ((XV_HDMIRX1_LNKSTA_BASE)+(4*4)) /**< LNKSTA Status Register offset */
#define XV_HDMIRX1_LNKSTA_LNK_ERR0_OFFSET            ((XV_HDMIRX1_LNKSTA_BASE)+(5*4)) /**< LNKSTA Link Error Counter Channel 0 Register offset */
#define XV_HDMIRX1_LNKSTA_LNK_ERR1_OFFSET            ((XV_HDMIRX1_LNKSTA_BASE)+(6*4)) /**< LNKSTA Link Error Counter Channel 1 Register offset */
#define XV_HDMIRX1_LNKSTA_LNK_ERR2_OFFSET            ((XV_HDMIRX1_LNKSTA_BASE)+(7*4)) /**< LNKSTA Link Error Counter Channel 2 Register offset */
#define XV_HDMIRX1_PKT_ECC_ERR_OFFSET                ((XV_HDMIRX1_LNKSTA_BASE)+(8*4)) /**< Packet ECC Error Register offset */
#define XV_HDMIRX1_TRIB_ANLZ_TIM_OFFSET              ((XV_HDMIRX1_LNKSTA_BASE)+(9*4)) /**< Tri-byte Analyzer Timing Register offset */
#define XV_HDMIRX1_TRIB_HBP_HS_OFFSET                ((XV_HDMIRX1_LNKSTA_BASE)+(10*4)) /**< Tri-byte HBP_HS Register offset */
#define XV_HDMIRX1_TRIB_ANLZ_LN_ACT_OFFSET           ((XV_HDMIRX1_LNKSTA_BASE)+(11*4)) /**< Tri-byte Analyzer Line Size Register offset */

/* Link Status (LNKSTA) peripheral Control register masks*/
#define XV_HDMIRX1_LNKSTA_CTRL_RUN_MASK              (1<<0)  /**< LNKSTA Control Run mask */
#define XV_HDMIRX1_LNKSTA_CTRL_IE_MASK               (1<<1)  /**< LNKSTA Control Interrupt Enable mask */
#define XV_HDMIRX1_LNKSTA_CTRL_ERR_CLR_MASK          (1<<2)  /**< LNKSTA Control Error Clear mask */

/* Link Status (LNKSTA) peripheral Status register masks*/
#define XV_HDMIRX1_LNKSTA_STA_IRQ_MASK               (1<<0)  /**< LNKSTA Status Interrupt mask */
#define XV_HDMIRX1_LNKSTA_STA_ERR_MAX_MASK           (1<<1)  /**< LNKSTA Status Maximum Errors mask */
#define XV_HDMIRX1_LNKSTA_STA_DCS_8CD_LOCK_MASK      (1<<2)
#define XV_HDMIRX1_LNKSTA_STA_DCS_DEEP_LOCK_MASK     (1<<3)

/* Tri-byte Analyzer register masks*/
#define XV_HDMIRX1_TRIB_ANLZ_TIM_CHGD_CNT_MASK       0xFFFF  /**< Tri-byte analyzer timing changed count mask */
#define XV_HDMIRX1_TRIB_ANLZ_TIM_CHGD_CNT_SHIFT      0       /**< Tri-byte analyzer timing changed count shift */
#define XV_HDMIRX1_TRIB_ANLZ_TIM_VS_POL_MASK         (1<<16) /**< Tri-byte analyzer timing vsync polarity mask */
#define XV_HDMIRX1_TRIB_ANLZ_TIM_HS_POL_MASK         (1<<17) /**< Tri-byte analyzer timing hsync polarity mask */

/* Tri-byte Analyzer register masks*/
#define XV_HDMIRX1_TRIB_HBP_HS_HS_SZ_SHIFT           0       /**< Tri-byte hsync size shift */
#define XV_HDMIRX1_TRIB_HBP_HS_HS_SZ_MASK            0xFFFF  /**< Tri-byte hsync size mask */
#define XV_HDMIRX1_TRIB_HBP_HS_HBP_SZ_SHIFT          16      /**< Tri-byte hbp size shift */
#define XV_HDMIRX1_TRIB_HBP_HS_HBP_SZ_MASK           0xFFFF  /**< Tri-byte hbp size mask */

/* Tri-byte Analyzer register masks*/
#define XV_HDMIRX1_TRIB_ANLZ_LN_ACT_ACT_SZ_SHIFT     0       /**< Tri-byte analyzer act size shift */
#define XV_HDMIRX1_TRIB_ANLZ_LN_ACT_ACT_SZ_MASK      0xFFFF  /**< Tri-byte analyzer act size mask */
#define XV_HDMIRX1_TRIB_ANLZ_LN_ACT_LN_SZ_SHIFT      16      /**< Tri-byte analyzer line act shift */
#define XV_HDMIRX1_TRIB_ANLZ_LN_ACT_LN_SZ_MASK       0xFFFF  /**< Tri-byte analyzer line act mask */

/* Fixed Rate Link (FRL) peripheral register offsets*/
#define XV_HDMIRX1_FRL_BASE              (8*64)
#define XV_HDMIRX1_FRL_ID_OFFSET         ((XV_HDMIRX1_FRL_BASE)+(0*4)) /**< FRL
                                * Identification Register offset */
#define XV_HDMIRX1_FRL_CTRL_OFFSET       ((XV_HDMIRX1_FRL_BASE)+(1*4)) /**< FRL
                                * Control Register offset */
#define XV_HDMIRX1_FRL_CTRL_SET_OFFSET   ((XV_HDMIRX1_FRL_BASE)+(2*4)) /**< FRL
                                * Control Register Set offset */
#define XV_HDMIRX1_FRL_CTRL_CLR_OFFSET   ((XV_HDMIRX1_FRL_BASE)+(3*4)) /**< FRL
                                * Control Register Clear offset */
#define XV_HDMIRX1_FRL_STA_OFFSET        ((XV_HDMIRX1_FRL_BASE)+(4*4)) /**< FRL
                                * Status Register offset */
#define XV_HDMIRX1_FRL_VCLK_VCKE_RATIO_OFFSET ((XV_HDMIRX1_FRL_BASE)+(7*4)) /**< FRL
                                * Video Clock  to VCKE Ratio Register offset */
#define XV_HDMIRX1_FRL_SCDC_OFFSET       ((XV_HDMIRX1_FRL_BASE)+(8*4)) /**< FRL
                                * Video Clock Register offset */
#define XV_HDMIRX1_FRL_RATIO_TOT_OFFSET     ((XV_HDMIRX1_FRL_BASE)+(9*4)) /**<
                                * FRL Total Data Register offset */
#define XV_HDMIRX1_FRL_RATIO_ACT_OFFSET     ((XV_HDMIRX1_FRL_BASE)+(10*4)) /**<
                                * FRL Total Active Data Register offset */
#define XV_HDMIRX1_FRL_RSFC_CNT_OFFSET      ((XV_HDMIRX1_FRL_BASE)+(11*4)) /**<
                                * Reed-Solomon FEC Counter Data Register offset */
#define XV_HDMIRX1_FRL_ERR_CNT1_OFFSET      ((XV_HDMIRX1_FRL_BASE)+(12*4)) /**<
                                * FRL Error Count Data Register offset */
#define XV_HDMIRX1_FRL_VID_LOCK_CNT_OFFSET  ((XV_HDMIRX1_FRL_BASE)+(13*4)) /**<
                                * Video Lock Count Data Register offset */

/* FRL Control register masks*/
#define XV_HDMIRX1_FRL_CTRL_RSTN_MASK                        (1<<0)  /**< FRL Control Resetn
                                                                       * mask */
#define XV_HDMIRX1_FRL_CTRL_IE_MASK                          (1<<1)  /**< FRL Control Interrupt
                                                                       * Enable mask */
#define XV_HDMIRX1_FRL_CTRL_CLK_RATIO_UPD_EVT_EN_MASK        (1<<2)  /**< FRL Control Clock Ratio
                                                                       * Update Event Enable mask */
#define XV_HDMIRX1_FRL_CTRL_SKEW_EVT_EN_MASK                 (1<<3)  /**< FRL Control Skew Event
                                                                       * Enable mask */
#define XV_HDMIRX1_FRL_CTRL_RSCC_RSFC_DISP_CLR_MASK          (1<<4)  /**< FRL RSCC Disparity
                                                                       * Clear mask */
#define XV_HDMIRX1_FRL_CTRL_FLT_CLR_MASK                     (1<<5)  /**< FRL Control FLT Clear
                                                                       * mask */
#define XV_HDMIRX1_FRL_CTRL_FRL_RATE_WR_EVT_EN_MASK          (1<<14)  /**< FRL Control FRL Rate
                                                                        * Write Event Enable */
#define XV_HDMIRX1_FRL_CTRL_DPACK_RST_MASK                   (1<<15)  /**< FRL Control DPACK
                                                                        * Reset mask */
#define XV_HDMIRX1_FRL_CTRL_DPACK_ERR_CNT_CLR_MASK           (1<<16)  /**< FRL Control DPACK
                                                                        * Error Counter Clear mask */
#define XV_HDMIRX1_FRL_CTRL_DPACK_AUTO_RST_DIS_MASK          (1<<17)  /**< FRL Control DPACK Auto
                                                                        * Reset Disable mask */
#define XV_HDMIRX1_FRL_CTRL_VID_LOCK_CNT_CLR_MASK            (1<<18)  /**< FRL Control Video Lock
                                                                        * Counter Clear Mask */
#define XV_HDMIRX1_FRL_CTRL_VID_LOCK_RST_DIS_MASK            (1<<19)  /**< FRL Control Video Lock
                                                                        * Reset Disable Mask */
#define XV_HDMIRX1_FRL_CTRL_FLT_THRES_MASK                   0xFF     /**< FRL Control FLT
                                                                        * Threshold mask */
#define XV_HDMIRX1_FRL_CTRL_FLT_THRES_SHIFT                  6        /**< FRL Config LTP
                                                                        * Threshold shift */

/* FRL Status register masks*/
#define XV_HDMIRX1_FRL_STA_IRQ_MASK                 (1<<0)   /**< FRL Status
                                                               * Interrupt mask */
#define XV_HDMIRX1_FRL_STA_EVT_MASK                 (1<<1)   /**< FRL Status
                                                               * Event mask */
#define XV_HDMIRX1_FRL_STA_FLT_PM_EVT_MASK          (1<<2)   /**< FRL Status FLT
                                                               * Pattern Match
                                                               * event mask */
#define XV_HDMIRX1_FRL_STA_FLT_PM_ALLL_MASK         0xF      /**< FRL Status FLT
                                                               * Pattern Match
                                                               * All Lanes mask */
#define XV_HDMIRX1_FRL_STA_FLT_PM_ALLL_SHIFT        3        /**< FRL Status FLT
                                                               * Pattern Match
                                                               * All Lanes shift */
#define XV_HDMIRX1_FRL_STA_FLT_PM_L0_MASK           (1<<3)   /**< FRL Status FLT
                                                               * Pattern Match
                                                               * Lane 0 mask */
#define XV_HDMIRX1_FRL_STA_FLT_PM_L1_MASK           (1<<4)   /**< FRL Status FLT
                                                               * Pattern Match
                                                               * Lane 0 mask */
#define XV_HDMIRX1_FRL_STA_FLT_PM_L2_MASK           (1<<5)   /**< FRL Status FLT
                                                               * Pattern Match
                                                               * Lane 0 mask */
#define XV_HDMIRX1_FRL_STA_FLT_PM_L3_MASK           (1<<6)   /**< FRL Status FLT
                                                               * Pattern Match
                                                               * Lane 0 mask */
#define XV_HDMIRX1_FRL_STA_FLT_UPD_EVT_MASK         (1<<7)   /**< FRL Status FLT
                                                               * Update event
                                                               * mask */
#define XV_HDMIRX1_FRL_STA_RATE_EVT_MASK            (1<<8)   /**< FRL Status FRL Rate
                                                               * change event mask */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_EVT_MASK       (1<<9)   /**< FRL Status Lane
                                                              * Lock event mask */
#define XV_HDMIRX1_FRL_STA_CLK_RATIO_UPD_EVT_MASK   (1<<10)  /**< FRL Status Clock
                                                               * Ratio Update event
                                                               * mask */
#define XV_HDMIRX1_FRL_STA_SKEW_LOCK_EVT_MASK       (1<<11)  /**< FRL Status Skew
                                                               * Lock event mask */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_ALLL_MASK      0xF      /**< FRL Status Lane
                                                               * Lock All Lanes
                                                               * mask */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_ALLL_SHIFT     12        /**< FRL Status Lane
                                                               * Lock All Lanes
                                                               * shift */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_L0_MASK        (1<<12)  /**< FRL Status Lane
                                                               * Lock L0 mask */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_L1_MASK        (1<<13)  /**< FRL Status Lane
                                                               * Lock L1 mask */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_L2_MASK        (1<<14)  /**< FRL Status Lane
                                                               * Lock L2 mask */
#define XV_HDMIRX1_FRL_STA_LANE_LOCK_L3_MASK        (1<<15)  /**< FRL Status Lane
                                                               * Lock L3 mask */
#define XV_HDMIRX1_FRL_STA_WA_LOCK_ALLL_MASK       0xF      /**< FRL Status Aligner
                                                               * Lock All Lanes
                                                               * mask */
#define XV_HDMIRX1_FRL_STA_WA_LOCK_ALLL_SHIFT      16        /**< FRL Status Aligner
                                                               * Lock All Lanes
                                                               * shift */
#define XV_HDMIRX1_FRL_STA_WA_LOCK_L0_MASK         (1<<16)  /**< FRL Status Aligner
                                                               * Lock L0 mask */
#define XV_HDMIRX1_FRL_STA_WA_LOCK_L1_MASK         (1<<17)  /**< FRL Status Aligner
                                                               * Lock L1 mask */
#define XV_HDMIRX1_FRL_STA_WA_LOCK_L2_MASK         (1<<18)  /**< FRL Status Aligner
                                                               * Lock L2 mask */
#define XV_HDMIRX1_FRL_STA_WA_LOCK_L3_MASK         (1<<19)  /**< FRL Status Aligner
                                                               * Lock L3 mask */
#define XV_HDMIRX1_FRL_STA_SCRM_LOCK_ALLL_MASK      0xF      /**< FRL Status Scrambler
                                                               * All Lanes mask */
#define XV_HDMIRX1_FRL_STA_SCRM_LOCK_ALLL_SHIFT     20        /**< FRL Status Scrambler
                                                               * All Lanes shift */
#define XV_HDMIRX1_FRL_STA_SCRM_LOCK_L0_MASK        (1<<20)  /**< FRL Status Scrambler
                                                               * Lock L0 mask */
#define XV_HDMIRX1_FRL_STA_SCRM_LOCK_L1_MASK        (1<<21)  /**< FRL Status Scrambler
                                                               * Lock L1 mask */
#define XV_HDMIRX1_FRL_STA_SCRM_LOCK_L2_MASK        (1<<22)  /**< FRL Status Scrambler
                                                               * Lock L2 mask */
#define XV_HDMIRX1_FRL_STA_SCRM_LOCK_L3_MASK        (1<<23)  /**< FRL Status Scrambler
                                                               * Lock L3 mask */
#define XV_HDMIRX1_FRL_STA_SKEW_LOCK_MASK           (1<<24)  /**< FRL Status Skew
                                                              * Lock mask */
#define XV_HDMIRX1_FRL_STA_STR_MASK                 (1<<25)  /**< FRL Status Video
                                                              * STR mask */
#define XV_HDMIRX1_FRL_STA_VID_LOCK_MASK            (1<<26)  /**< FRL Status Video
                                                              * Lock mask */
#define XV_HDMIRX1_FRL_STA_FRL_MODE_MASK            (1<<27)  /**< FRL Status Mode
                                                              * mask */
#define XV_HDMIRX1_FRL_STA_FRL_LANES_MASK           (1<<28)  /**< FRL Status Lanes
                                                              * mask */
#define XV_HDMIRX1_FRL_STA_FRL_RATE_MASK            0x7      /**< FRL Status Rate
                                                              * mask */
#define XV_HDMIRX1_FRL_STA_FRL_RATE_SHIFT           29       /**< FRL Status Rate
                                                              * shift */
/* FRL Link Clock register masks*/
#define XV_HDMIRX1_FRL_LNK_CLK_MASK       0xFFFFF /**< FRL Link Clock mask */
#define XV_HDMIRX1_FRL_VID_CLK_MASK       0xFFFFF /**< FRL Video Clock mask */

/* FRL SCDC register masks*/
#define XV_HDMIRX1_FRL_SCDC_ADDR_MASK           0xFF    /**< FRL SCDC Address
                                                       * Interrupt mask */
#define XV_HDMIRX1_FRL_SCDC_ADDR_SHIFT          0       /**< FRL SCDC Address
                                                       * Interrupt mask */
#define XV_HDMIRX1_FRL_SCDC_DAT_MASK            0xFF    /**< FRL SCDC Data
                                                       * mask */
#define XV_HDMIRX1_FRL_SCDC_DAT_SHIFT           8       /**< FRL SCDC Data
                                                       * mask */
#define XV_HDMIRX1_FRL_SCDC_WR_MASK             (1<<16) /**< FRL SCDC Write
                                                       * mask */
#define XV_HDMIRX1_FRL_SCDC_RD_MASK             (1<<17) /**< FRL SCDC Read
                                                       * mask */
#define XV_HDMIRX1_FRL_SCDC_RDY_MASK            (1<<18) /**< FRL SCDC Ready
                                                       * mask */

#define XV_HDMIRX1_FRL_RATIO_TOT_MASK           0xFFFFFF
#define XV_HDMIRX1_FRL_RATIO_ACT_MASK           0xFFFFFF

#define XV_HDMIRX1_FRL_ERR_CNT1_DPACK_ERR_CNT_MASK  0xFFFF
#define XV_HDMIRX1_FRL_ERR_CNT1_DPACK_ERR_CNT_SHIFT 16
#define XV_HDMIRX1_FRL_ERR_CNT1_RSCC_ERR_CNT_MASK   0xFFFF
#define XV_HDMIRX1_FRL_ERR_CNT1_RSCC_ERR_CNT_SHIFT  0

/* Peripheral ID and General shift values.*/
#define XV_HDMIRX1_SHIFT_16      16  /**< 16 shift value */
#define XV_HDMIRX1_MASK_16       0xFFFF  /**< 16 bit mask value */
#define XV_HDMIRX1_PIO_ID        0x2200  /**< PIO ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XV_HdmiRx1_In32  Xil_In32    /**< Input Operations */
#define XV_HdmiRx1_Out32 Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a HDMI RX register. A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the HDMI RX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*           u32 XV_HdmiRx1_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XV_HdmiRx1_ReadReg(BaseAddress, RegOffset) \
	XV_HdmiRx1_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a HDMI RX register. A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the HDMI RX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*           void XV_HdmiRx1_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XV_HdmiRx1_WriteReg(BaseAddress, RegOffset, Data) \
	XV_HdmiRx1_Out32((BaseAddress) + (RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
