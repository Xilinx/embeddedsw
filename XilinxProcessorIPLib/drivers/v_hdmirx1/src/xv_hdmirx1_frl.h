/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1.c
*
* This is the main file for Xilinx HDMI RX core. Please see xv_hdmirx1.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00  EB     22/06/18 Initial release.
* </pre>
*
******************************************************************************/

#ifndef XV_HDMIRX1_FRL_H_
#define XV_HDMIRX1_FRL_H_        /**< Prevent circular inclusions
				   *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/** @name HDMI RX FRL SCDC Fields
* @ {
*/
typedef enum {
	XV_HDMIRX1_SCDCFIELD_SINK_VER = 0,
	XV_HDMIRX1_SCDCFIELD_SOURCE_VER,
	XV_HDMIRX1_SCDCFIELD_CED_UPDATE,
	XV_HDMIRX1_SCDCFIELD_SOURCE_TEST_UPDATE,
	XV_HDMIRX1_SCDCFIELD_FRL_START,
	XV_HDMIRX1_SCDCFIELD_FLT_UPDATE,
	XV_HDMIRX1_SCDCFIELD_RSED_UPDATE,
	XV_HDMIRX1_SCDCFIELD_SCRAMBLER_EN,
	XV_HDMIRX1_SCDCFIELD_SCRAMBLER_STAT,
	XV_HDMIRX1_SCDCFIELD_FLT_NO_RETRAIN,
	XV_HDMIRX1_SCDCFIELD_FRL_RATE,
	XV_HDMIRX1_SCDCFIELD_FFE_LEVELS,
	XV_HDMIRX1_SCDCFIELD_FLT_NO_TIMEOUT,
	XV_HDMIRX1_SCDCFIELD_LNS_LOCK,
	XV_HDMIRX1_SCDCFIELD_FLT_READY,
	XV_HDMIRX1_SCDCFIELD_LN0_LTP_REQ,
	XV_HDMIRX1_SCDCFIELD_LN1_LTP_REQ,
	XV_HDMIRX1_SCDCFIELD_LN2_LTP_REQ,
	XV_HDMIRX1_SCDCFIELD_LN3_LTP_REQ,
	XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_LSB,
	XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_MSB,
	XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_LSB,
	XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_MSB,
	XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_LSB,
	XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_MSB,
	XV_HDMIRX1_SCDCFIELD_CED_CHECKSUM,
	XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_LSB,
	XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_MSB,
	XV_HDMIRX1_SCDCFIELD_RSCCNT_LSB,
	XV_HDMIRX1_SCDCFIELD_RSCCNT_MSB,
	XV_HDMIRX1_SCDCFIELD_DSC_DECODE_FAIL,
	XV_HDMIRX1_SCDCFIELD_DSC_FRL_MAX,
	XV_HDMIRX1_SCDCFIELD_SIZE
} XV_HdmiRx1_FrlScdcFieldType;

#ifdef XPAR_XV_HDMI_RX_FRL_ENABLE
/** @name HDMI RX FRL training state
* @ {
*/
typedef enum {
	XV_HDMIRX1_FRLSTATE_LTS_L,		/* LTS:L*/
/*	XV_HDMIRX1_FRLSTATE_LTS_1,		// LTS:1*/
	XV_HDMIRX1_FRLSTATE_LTS_2,		/* LTS:2*/
	XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH,	/* LTS:3 (Rate Change)*/
	XV_HDMIRX1_FRLSTATE_LTS_3_ARM_LNK_RDY,	/* LTS:3 (ARM Link Ready)*/
	XV_HDMIRX1_FRLSTATE_LTS_3_ARM_VID_RDY,	/* LTS:3 (ARM Video Ready)*/
	XV_HDMIRX1_FRLSTATE_LTS_3_LTP_DET,	/* LTS:3 (LTP Detected)*/
	XV_HDMIRX1_FRLSTATE_LTS_3_TMR,		/* LTS:3 (Timer Event)*/
	XV_HDMIRX1_FRLSTATE_LTS_3,		/* LTS:3*/
	XV_HDMIRX1_FRLSTATE_LTS_3_RDY,		/* LTS:3 (Ready)*/
/*	XV_HDMIRX1_FRLSTATE_LTS_4,		// LTS:4*/
/*	XV_HDMIRX1_FRLSTATE_LTS_P_ARM,		// LTS:P (Step 1)*/
	XV_HDMIRX1_FRLSTATE_LTS_P,		/* LTS:P*/
	XV_HDMIRX1_FRLSTATE_LTS_P_TIMEOUT,	/* LTS:P (Timeout)*/
	XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY,	/* LTS:P (FRL_START = 1)*/
	XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY	/* LTS:P (Skew Locked)*/
} XV_HdmiRx1_FrlTrainingState;

/** @name HDMI RX LTP Type
* @ {
*/
typedef enum {
	XV_HDMIRX1_LTP_SUCCESS = 0,
	XV_HDMIRX1_LTP_ALL_ONES,
	XV_HDMIRX1_LTP_ALL_ZEROES,
	XV_HDMIRX1_LTP_NYQUIST_CLOCK,
	XV_HDMIRX1_LTP_RXDDE_COMPLIANCE,
	XV_HDMIRX1_LTP_LFSR0,
	XV_HDMIRX1_LTP_LFSR1,
	XV_HDMIRX1_LTP_LFSR2,
	XV_HDMIRX1_LTP_LFSR3,
	XV_HDMIRX1_LTP_FFE_CHANGE = 0xE,
	XV_HDMIRX1_LTP_RATE_CHANGE
} XV_HdmiRx1_FrlLtpType;

typedef union {
	u32 Data;    				/**< All the 4 lanes */
	u8 Byte[4];    				/**< Each of the lanes */
} XV_HdmiRx1_FrlFfeAdjType;

typedef union {
	u32 Data;
	u8  Byte[4];
} XV_HdmiRx1_FrlLtp;
#endif

/**
* This typedef contains DDC registers offset, mask, shift.
*/
typedef struct {
/*	XV_HdmiRx1_ScdcFieldType Field;	/\**< SCDC Field *\/ */
	u8 Offset;				/**< Register offset */
	u8 Mask;				/**< Bits mask */
	u8 Shift;				/**< Bits shift */
} XV_HdmiRx1_FrlScdcField;

#if defined(XPAR_XV_HDMI_RX_FRL_ENABLE)
/**
* This typedef contains audio stream specific data structure
*/
typedef struct {
	XV_HdmiRx1_FrlTrainingState TrainingState;	/**< Fixed Rate Link
							  *  State */
	u32                         TimerCnt;		/**< FRL Timer */
	u8                          LineRate;		/**< Current Line
							  * Rate from FRL
							  * rate*/
	u32                          CurFrlRate;	/**< Current FRL Rate
							  * supported */
	u8                          Lanes;		/**< Current number of
							  *  lanes used */
	u8                          FfeLevels;		/**< Number of Supported
							  *  FFE Levels	for the
							  *  current FRL Rate */
	u8                          FfeSuppFlag;	/**< RX Core's support
							  *  of FFE Levels */
	u8                          FltUpdateAsserted;
	XV_HdmiRx1_FrlLtp           Ltp;		/**< LTP to be detected
							  *  by the RX core and
							  *  queried by source. */
	XV_HdmiRx1_FrlLtp           DefaultLtp;		/**< LTP which will be
							  *  used by RX core for
							  *  link training */
	XV_HdmiRx1_FrlFfeAdjType    LaneFfeAdjReq;	/**< The RxFFE for each
							  *  of the lanes */
	u8                          FltNoTimeout;
	u8                          FltNoRetrain;
	u8                          LtpMatchWaitCounts;
	u8                          LtpMatchedCounts;
	u8                          LtpMatchPollCounts;
} XV_HdmiRx1_Frl;

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI RX FRL peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiRx1_FrlIntrEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_FrlIntrEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
	(XV_HDMIRX1_FRL_CTRL_SET_OFFSET), (XV_HDMIRX1_FRL_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI RX FRL peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiRx1_FrlIntrDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_FrlIntrDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
	(XV_HDMIRX1_FRL_CTRL_CLR_OFFSET), (XV_HDMIRX1_FRL_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro sets the ratio of video clock  to video clock enable of RX Core's
* FRL peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @param	Value specifies the Video Clock
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_SetFrlVClkVckeRatio(XV_HdmiRx1 *InstancePtr,
*		                                    u16 Value)
*
******************************************************************************/
#define XV_HdmiRx1_SetFrlVClkVckeRatio(InstancePtr, Value) \
{ \
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMIRX1_FRL_VCLK_VCKE_RATIO_OFFSET), \
						   (Value)); \
}

/*****************************************************************************/
/**
*
* This macro enables FRL Skew Lock Event.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_SkewLockEvtEnable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_SkewLockEvtEnable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XV_HDMIRX1_FRL_CTRL_SET_OFFSET), \
			(XV_HDMIRX1_FRL_CTRL_SKEW_EVT_EN_MASK))

/*****************************************************************************/
/**
*
* This macro disables FRL Skew Lock Event.
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiRx1_SkewLockEvtDisable(XV_HdmiRx1 *InstancePtr)
*
******************************************************************************/
#define XV_HdmiRx1_SkewLockEvtDisable(InstancePtr) \
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XV_HDMIRX1_FRL_CTRL_CLR_OFFSET), \
			(XV_HDMIRX1_FRL_CTRL_SKEW_EVT_EN_MASK))

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#endif /* XPAR_XV_HDMI_RX_FRL_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
