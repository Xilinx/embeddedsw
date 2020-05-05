/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_log.h
* @addtogroup vprocss_v2_7
* @{
* @details
*
* This header file contains the video processing event log
* initialization routines and helper functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  dmc  01/27/16 Initial Release
*       dmc  03/03/16 Add events for VDMA configuration and operational errors
* 2.30  rco  11/15/16 Make debug log optional (can be disabled via makefile)
*
* </pre>
*
******************************************************************************/

#ifndef XVPROCSS_LOG_H /**< prevent circular inclusions by using protection macros*/
#define XVPROCSS_LOG_H

#if !defined(XV_CONFIG_LOG_VPRCOSS_DISABLE) && !defined(XV_CONFIG_LOG_DISABLE_ALL)
#define XV_VPROCSS_LOG_ENABLE
#endif

#ifdef XV_VPROCSS_LOG_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/****************************** Type Definitions ******************************/
/**
 * This typedef contains the Event Log identifiers.
 */
typedef enum {
	XVPROCSS_EVT_NONE = 1,     /**< Log event none. */
	XVPROCSS_EVT_INIT,         /**< Log event init. */
	XVPROCSS_EVT_INIT_RESAXIS, /**< Log event init status of Reset Axis subcore */
	XVPROCSS_EVT_INIT_RESAXIM, /**< Log event init status of Reset Aximm subcore */
	XVPROCSS_EVT_INIT_ROUTER,  /**< Log event init status of Router subcore */
	XVPROCSS_EVT_INIT_LBOX,    /**< Log event init status of Letterbox subcore */
	XVPROCSS_EVT_INIT_VDMA,    /**< Log event init status of VDMA subcore */
	XVPROCSS_EVT_CFGERR_VDMA,  /**< Log event VDMA configuration error */
	XVPROCSS_EVT_OPERR_VDMA,   /**< Log event VDMA operation error */
	XVPROCSS_EVT_CFG_HSCALER,  /**< Log event Scaler config status */
	XVPROCSS_EVT_CFG_VSCALER,  /**< Log event Scaler config status */
	XVPROCSS_EVT_CFG_CSC,      /**< Log event Csc config status */
	XVPROCSS_EVT_CFG_DEINT,    /**< Log event Deinterlacer config status */
	XVPROCSS_EVT_CFG_VCRI,     /**< Log event VCResampler config status */
	XVPROCSS_EVT_CFG_VCRO,     /**< Log event VCResampler config status */
	XVPROCSS_EVT_CFG_HCR,      /**< Log event HCResampler config status */
	XVPROCSS_EVT_CFG_MAX,      /**< Log event Full Fledged config status */
	XVPROCSS_EVT_CFG_VPSS,     /**< Log event VPSS config status */
	XVPROCSS_EVT_CHK_TOPO,     /**< Log event check Topology */
	XVPROCSS_EVT_CHK_BASEADDR, /**< Log event check Frame buffer base address */
	XVPROCSS_EVT_SET_PIPMODE,  /**< Log event Set Pip Mode */
	XVPROCSS_EVT_SET_ZOOMMODE, /**< Log event Set Zoom Mode */
	XVPROCSS_EVT_SET_PIPWIN,   /**< Log event Set Pip window */
	XVPROCSS_EVT_SET_ZOOMWIN,  /**< Log event Set Zoom window */
	XVPROCSS_EVT_GET_ZPWIN,    /**< Log event Get Zoom/Pip window */
	XVPROCSS_EVT_UPDATE_ZPWIN, /**< Log event Update Zoom/Pip window status */
	XVPROCSS_EVT_RESET_VPSS,   /**< Log event Reset the VPSS */
	XVPROCSS_EVT_START_VPSS,   /**< Log event Start the VPSS */
	XVPROCSS_EVT_STOP_VPSS,    /**< Log event Stop the VPSS */
	XVPROCSS_EVT_LAST_ENUM     /**< (dummy event: marks last enum) */
} XVprocSs_LogEvent;

/**
 * Data structure for the event logging mechanism for debug.
 */
#define XVPROCSS_EVT_BUFFSIZE 256

typedef struct {
	u16 DataBuffer[XVPROCSS_EVT_BUFFSIZE]; /**< Log buffer with event data. */
	u8 HeadIndex;        /**< Index of the head entry of the
                              Event/DataBuffer. */
	u8 TailIndex;        /**< Index of the tail entry of the
                              Event/DataBuffer. */
} XVprocSs_Log;

/****************************** Constants ************************************/
/**
 * Event Log status codes.  These are the event data values.
 *
 * 0x00 - 0x0F : Successful events - applicable to VPSS and subcores
 * 0xF0 - 0xFF : Error events - applicable to VPSS and subcores
 *
 * 0x10 ...    : Successful events - applicable to particular subcores
 * ... 0xEF    : Error events - applicable to particular subcores
 */
// General codes
#define XVPROCSS_EDAT_SUCCESS      0x00
#define XVPROCSS_EDAT_FAILURE      0xFF

#define XVPROCSS_EDAT_OFF          0x00
#define XVPROCSS_EDAT_ON           0x01

#define XVPROCSS_EDAT_BEGIN        0x00
#define XVPROCSS_EDAT_END          0x01

// These codes apply to several subcores
#define XVPROCSS_EDAT_VALID        0x02
#define XVPROCSS_EDAT_INITOK       0x03
#define XVPROCSS_EDAT_SETUPOK      0x04
#define XVPROCSS_EDAT_LDCOEF       0x05
#define XVPROCSS_EDAT_INITFAIL     0xF0
#define XVPROCSS_EDAT_IPABSENT     0xF1
#define XVPROCSS_EDAT_CFABSENT     0xF2
#define XVPROCSS_EDAT_BADADDR      0xF3
#define XVPROCSS_EDAT_IGNORE       0xF4
#define XVPROCSS_EDAT_IODIFF       0xF5
#define XVPROCSS_EDAT_VMDIFF       0xF6
#define XVPROCSS_EDAT_CDIFF        0xF7
#define XVPROCSS_EDAT_HDIFF        0xF8
#define XVPROCSS_EDAT_VDIFF        0xF9
#define XVPROCSS_EDAT_INTPRG       0xFA
#define XVPROCSS_EDAT_NO420        0xFB
#define XVPROCSS_EDAT_NO422        0xFC
#define XVPROCSS_EDAT_CFIN         0xFD
#define XVPROCSS_EDAT_CFOUT        0xFE

// These codes are specific to CSC-only mode
#define XVPROCSS_EDAT_CSC_SETWIN   0x10
#define XVPROCSS_EDAT_CSC_BADWIN   0xEF

// These codes are specific to Max (Full) mode
#define XVPROCSS_EDAT_MAX_TABLEOK  0x10
#define XVPROCSS_EDAT_MAX_DFLOWOK  0x11
#define XVPROCSS_EDAT_MAX_ROUTEOK  0x12
#define XVPROCSS_EDAT_MAX_SCALE11  0x13
#define XVPROCSS_EDAT_MAX_SCALEUP  0x14
#define XVPROCSS_EDAT_MAX_SCALEDN  0x15
#define XVPROCSS_EDAT_MAX_WRWINBAD 0xED
#define XVPROCSS_EDAT_MAX_RDWINBAD 0xEE
#define XVPROCSS_EDAT_MAX_SCALEBAD 0xEF

// These codes are specific to VPSS
#define XVPROCSS_EDAT_VPSS_FRDIFF  0xE5
#define XVPROCSS_EDAT_VPSS_IVRANGE 0xE6
#define XVPROCSS_EDAT_VPSS_OVRANGE 0xE7
#define XVPROCSS_EDAT_VPSS_WIDBAD  0xE8
#define XVPROCSS_EDAT_VPSS_RESBAD  0xE9
#define XVPROCSS_EDAT_VPSS_WIDODD  0xEA
#define XVPROCSS_EDAT_VPSS_SIZODD  0xEB
#define XVPROCSS_EDAT_VPSS_NOHCR   0xEC
#define XVPROCSS_EDAT_VPSS_NOVCRI  0xED
#define XVPROCSS_EDAT_VPSS_NOVCRO  0xEE
#define XVPROCSS_EDAT_VPSS_NODEIN  0xEF

// These codes are specific to VDMA
#define XVPROCSS_EDAT_VDMA_WRCFG   0xE8
#define XVPROCSS_EDAT_VDMA_RDCFG   0xE9
#define XVPROCSS_EDAT_VDMA_WRADR   0xEA
#define XVPROCSS_EDAT_VDMA_RDADR   0xEB
#define XVPROCSS_EDAT_VDMA_WRXFR   0xEC
#define XVPROCSS_EDAT_VDMA_RDXFR   0xED
#define XVPROCSS_EDAT_VDMA_WRRES   0xEE
#define XVPROCSS_EDAT_VDMA_RDRES   0xEF

// These codes are specific to PIP setup
#define XVPROCSS_EDAT_BGND_SET     0x10
#define XVPROCSS_EDAT_LBOX_ABSENT  0xEF

#ifdef __cplusplus
}
#endif

#endif //XV_VPROCSS_LOG_ENABLE

#endif /* end of protection macro */
