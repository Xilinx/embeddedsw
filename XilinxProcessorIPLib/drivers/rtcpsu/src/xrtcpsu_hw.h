/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XRTC_HW_H_
#define XRTC_HW_H_

/*****************************************************************************/
/**
*
* @file xrtcpsu_hw.h
* @brief Provides hardware register definitions for the RTC PS driver.
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xrtcpsu.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a kvn	  04/21/15 First release
* 1.1   kvn   09/25/15 Modify control register to enable battery
*                      switching when vcc_psaux is not available.
* 1.18  vn    09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup rtcpsu Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**
 * Xrtc Base Address.
 */
#define XRTC_BASEADDR      0xFFA60000U
/**
 * @name Set Timer Register
 * This register contains bits for configuring
 * Current time.
 * @{
 */
/**
 * XrtcSetTimeWr register offset.
 */
#define XRTC_SET_TIME_WR_OFFSET    0x00000000U
#define XRTC_SET_TIME_WR_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_SET_TIME_WR_VAL_SHIFT   0U /**< Value field shift. */
#define XRTC_SET_TIME_WR_VAL_WIDTH   32U /**< Value field width. */
#define XRTC_SET_TIME_WR_VAL_MASK    0xffffffffU /**< Value field mask. */
#define XRTC_SET_TIME_WR_VAL_DEFVAL  0x0U /**< Value field default value. */

/**
 * XrtcSetTimeRd register offset.
 */
#define XRTC_SET_TIME_RD_OFFSET    0x00000004U
#define XRTC_SET_TIME_RD_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_SET_TIME_RD_VAL_SHIFT   0U /**< Value field shift. */
#define XRTC_SET_TIME_RD_VAL_WIDTH   32U /**< Value field width. */
#define XRTC_SET_TIME_RD_VAL_MASK    0xffffffffU /**< Value field mask. */
#define XRTC_SET_TIME_RD_VAL_DEFVAL  0x0U /**< Value field default value. */
/** @} */

/**
 * @name Calibration  Register
 * This register contains bits for configuring
 * Calibration values.
 * @{
 */
/**
 * XrtcCalibWr register offset.
 */
#define XRTC_CALIB_WR_OFFSET    0x00000008U
#define XRTC_CALIB_WR_RSTVAL   0x00000000U /**< Register reset value. */

/** Fractional enable field shift. */
#define XRTC_CALIB_WR_FRACTN_EN_SHIFT   20U
/** Fractional enable field width. */
#define XRTC_CALIB_WR_FRACTN_EN_WIDTH   1U
/** Fractional enable field mask. */
#define XRTC_CALIB_WR_FRACTN_EN_MASK    0x00100000U
/** Fractional enable field default value. */
#define XRTC_CALIB_WR_FRACTN_EN_DEFVAL  0x0U

/** Fractional data field shift. */
#define XRTC_CALIB_WR_FRACTN_DATA_SHIFT   16U
/** Fractional data field width. */
#define XRTC_CALIB_WR_FRACTN_DATA_WIDTH   4U
/** Fractional data field mask. */
#define XRTC_CALIB_WR_FRACTN_DATA_MASK    0x000f0000U
/** Fractional data field default value. */
#define XRTC_CALIB_WR_FRACTN_DATA_DEFVAL  0x0U

#define XRTC_CALIB_WR_MAX_TCK_SHIFT   0U /**< Maximum tick field shift. */
#define XRTC_CALIB_WR_MAX_TCK_WIDTH   16U /**< Maximum tick field width. */
/** Maximum tick field mask. */
#define XRTC_CALIB_WR_MAX_TCK_MASK    0x0000ffffU
/** Maximum tick field default value. */
#define XRTC_CALIB_WR_MAX_TCK_DEFVAL  0x0U

/**
 * XrtcCalibRd register offset.
 */
#define XRTC_CALIB_RD_OFFSET    0x0000000CU
#define XRTC_CALIB_RD_RSTVAL   0x00000000U /**< Register reset value. */

/** Fractional enable field shift. */
#define XRTC_CALIB_RD_FRACTN_EN_SHIFT   20U
/** Fractional enable field width. */
#define XRTC_CALIB_RD_FRACTN_EN_WIDTH   1U
/** Fractional enable field mask. */
#define XRTC_CALIB_RD_FRACTN_EN_MASK    0x00100000U
/** Fractional enable field default value. */
#define XRTC_CALIB_RD_FRACTN_EN_DEFVAL  0x0U

/** Fractional data field shift. */
#define XRTC_CALIB_RD_FRACTN_DATA_SHIFT   16U
/** Fractional data field width. */
#define XRTC_CALIB_RD_FRACTN_DATA_WIDTH   4U
/** Fractional data field mask. */
#define XRTC_CALIB_RD_FRACTN_DATA_MASK    0x000f0000U
/** Fractional data field default value. */
#define XRTC_CALIB_RD_FRACTN_DATA_DEFVAL  0x0U

#define XRTC_CALIB_RD_MAX_TCK_SHIFT   0U /**< Maximum tick field shift. */
#define XRTC_CALIB_RD_MAX_TCK_WIDTH   16U /**< Maximum tick field width. */
/** Maximum tick field mask. */
#define XRTC_CALIB_RD_MAX_TCK_MASK    0x0000ffffU
/** Maximum tick field default value. */
#define XRTC_CALIB_RD_MAX_TCK_DEFVAL  0x0U
/** @} */

/**
 * @name Current time  Register
 * This register contains bits for configuring
 * Current time.
 * @{
 */
/**
 * XrtcCurTime register offset.
 */
#define XRTC_CUR_TIME_OFFSET    0x00000010U
#define XRTC_CUR_TIME_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_CUR_TIME_VAL_SHIFT   0U /**< Value field shift. */
#define XRTC_CUR_TIME_VAL_WIDTH   32U /**< Value field width. */
#define XRTC_CUR_TIME_VAL_MASK    0xffffffffU /**< Value field mask. */
#define XRTC_CUR_TIME_VAL_DEFVAL  0x0U /**< Value field default value. */

/**
 * XrtcCurTck register offset.
 */
#define XRTC_CUR_TCK_OFFSET    0x00000014U
#define XRTC_CUR_TCK_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_CUR_TCK_VAL_SHIFT   0U /**< Value field shift. */
#define XRTC_CUR_TCK_VAL_WIDTH   16U /**< Value field width. */
#define XRTC_CUR_TCK_VAL_MASK    0x0000ffffU /**< Value field mask. */
#define XRTC_CUR_TCK_VAL_DEFVAL  0x0U /**< Value field default value. */
/** @} */

/**
 * @name Alarm Register
 * This register contains bits for configuring
 * Alarm.
 * @{
 */
/**
 * XrtcAlrm register offset.
 */
#define XRTC_ALRM_OFFSET    0x00000018U
#define XRTC_ALRM_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_ALRM_VAL_SHIFT   0U /**< Value field shift. */
#define XRTC_ALRM_VAL_WIDTH   32U /**< Value field width. */
#define XRTC_ALRM_VAL_MASK    0xffffffffU /**< Value field mask. */
#define XRTC_ALRM_VAL_DEFVAL  0x0U /**< Value field default value. */
/** @} */

/**
 * @name Interrupt Status  Register
 * This register contains bits for configuring
 * Interrupt.
 * @{
 */
/**
 * XrtcIntSts register offset.
 */
#define XRTC_INT_STS_OFFSET    0x00000020U
#define XRTC_INT_STS_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_INT_STS_ALRM_SHIFT   1U /**< Alarm field shift. */
#define XRTC_INT_STS_ALRM_WIDTH   1U /**< Alarm field width. */
#define XRTC_INT_STS_ALRM_MASK    0x00000002U /**< Alarm field mask. */
#define XRTC_INT_STS_ALRM_DEFVAL  0x0U /**< Alarm field default value. */

#define XRTC_INT_STS_SECS_SHIFT   0U /**< Seconds field shift. */
#define XRTC_INT_STS_SECS_WIDTH   1U /**< Seconds field width. */
#define XRTC_INT_STS_SECS_MASK    0x00000001U /**< Seconds field mask. */
#define XRTC_INT_STS_SECS_DEFVAL  0x0U /**< Seconds field default value. */

/**
 * XrtcIntMsk register offset.
 */
#define XRTC_INT_MSK_OFFSET    0x00000024U
#define XRTC_INT_MSK_RSTVAL   0x00000003U /**< Register reset value. */

#define XRTC_INT_MSK_ALRM_SHIFT   1U /**< Alarm field shift. */
#define XRTC_INT_MSK_ALRM_WIDTH   1U /**< Alarm field width. */
#define XRTC_INT_MSK_ALRM_MASK    0x00000002U /**< Alarm field mask. */
#define XRTC_INT_MSK_ALRM_DEFVAL  0x1U /**< Alarm field default value. */

#define XRTC_INT_MSK_SECS_SHIFT   0U /**< Seconds field shift. */
#define XRTC_INT_MSK_SECS_WIDTH   1U /**< Seconds field width. */
#define XRTC_INT_MSK_SECS_MASK    0x00000001U /**< Seconds field mask. */
#define XRTC_INT_MSK_SECS_DEFVAL  0x1U /**< Seconds field default value. */

/**
 * XrtcIntEn register offset.
 */
#define XRTC_INT_EN_OFFSET    0x00000028U
#define XRTC_INT_EN_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_INT_EN_ALRM_SHIFT   1U /**< Alarm field shift. */
#define XRTC_INT_EN_ALRM_WIDTH   1U /**< Alarm field width. */
#define XRTC_INT_EN_ALRM_MASK    0x00000002U /**< Alarm field mask. */
#define XRTC_INT_EN_ALRM_DEFVAL  0x0U /**< Alarm field default value. */

#define XRTC_INT_EN_SECS_SHIFT   0U /**< Seconds field shift. */
#define XRTC_INT_EN_SECS_WIDTH   1U /**< Seconds field width. */
#define XRTC_INT_EN_SECS_MASK    0x00000001U /**< Seconds field mask. */
#define XRTC_INT_EN_SECS_DEFVAL  0x0U /**< Seconds field default value. */

/**
 * XrtcIntDis register offset.
 */
#define XRTC_INT_DIS_OFFSET    0x0000002CU
#define XRTC_INT_DIS_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_INT_DIS_ALRM_SHIFT   1U /**< Alarm field shift. */
#define XRTC_INT_DIS_ALRM_WIDTH   1U /**< Alarm field width. */
#define XRTC_INT_DIS_ALRM_MASK    0x00000002U /**< Alarm field mask. */
#define XRTC_INT_DIS_ALRM_DEFVAL  0x0U /**< Alarm field default value. */

#define XRTC_INT_DIS_SECS_SHIFT   0U /**< Seconds field shift. */
#define XRTC_INT_DIS_SECS_WIDTH   1U /**< Seconds field width. */
#define XRTC_INT_DIS_SECS_MASK    0x00000001U /**< Seconds field mask. */
#define XRTC_INT_DIS_SECS_DEFVAL  0x0U /**< Seconds field default value. */

/**
 * XrtcAddErr register offset.
 */
#define XRTC_ADD_ERR_OFFSET    0x00000030U
#define XRTC_ADD_ERR_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_ADD_ERR_STS_SHIFT   0U /**< Status field shift. */
#define XRTC_ADD_ERR_STS_WIDTH   1U /**< Status field width. */
#define XRTC_ADD_ERR_STS_MASK    0x00000001U /**< Status field mask. */
#define XRTC_ADD_ERR_STS_DEFVAL  0x0U /**< Status field default value. */

/**
 * XrtcAddErrIntMsk register offset.
 */
#define XRTC_ADD_ERR_INT_MSK_OFFSET    0x00000034U
#define XRTC_ADD_ERR_INT_MSK_RSTVAL   0x00000001U /**< Register reset value. */

#define XRTC_ADD_ERR_INT_MSK_SHIFT   0U /**< Mask field shift. */
#define XRTC_ADD_ERR_INT_MSK_WIDTH   1U /**< Mask field width. */
#define XRTC_ADD_ERR_INT_MSK_MASK    0x00000001U /**< Mask field mask. */
#define XRTC_ADD_ERR_INT_MSK_DEFVAL  0x1U /**< Mask field default value. */

/**
 * XrtcAddErrIntEn register offset.
 */
#define XRTC_ADD_ERR_INT_EN_OFFSET    0x00000038U
#define XRTC_ADD_ERR_INT_EN_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_ADD_ERR_INT_EN_MSK_SHIFT   0U /**< Mask field shift. */
#define XRTC_ADD_ERR_INT_EN_MSK_WIDTH   1U /**< Mask field width. */
#define XRTC_ADD_ERR_INT_EN_MSK_MASK    0x00000001U /**< Mask field mask. */
#define XRTC_ADD_ERR_INT_EN_MSK_DEFVAL  0x0U /**< Mask field default value. */

/**
 * XrtcAddErrIntDis register offset.
 */
#define XRTC_ADD_ERR_INT_DIS_OFFSET    0x0000003CU
#define XRTC_ADD_ERR_INT_DIS_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_ADD_ERR_INT_DIS_MSK_SHIFT   0U /**< Mask field shift. */
#define XRTC_ADD_ERR_INT_DIS_MSK_WIDTH   1U /**< Mask field width. */
#define XRTC_ADD_ERR_INT_DIS_MSK_MASK    0x00000001U /**< Mask field mask. */
#define XRTC_ADD_ERR_INT_DIS_MSK_DEFVAL  0x0U /**< Mask field default value. */
/** @} */

/**
 * @name Control Register
 * This register contains bits for control register.
 * @{
 */
/**
 * XrtcCtl register offset.
 */
#define XRTC_CTL_OFFSET    0x00000040U
#define XRTC_CTL_RSTVAL   0x01000000U /**< Register reset value. */

#define XRTC_CTL_BATTERY_EN_SHIFT   31U /**< Battery enable field shift. */
#define XRTC_CTL_BATTERY_EN_WIDTH   1U /**< Battery enable field width. */
/** Battery enable field mask. */
#define XRTC_CTL_BATTERY_EN_MASK    0x80000000U
/** Battery enable field default value. */
#define XRTC_CTL_BATTERY_EN_DEFVAL  0x0U

#define XRTC_CTL_OSC_SHIFT   24U /**< Oscillator field shift. */
#define XRTC_CTL_OSC_WIDTH   4U /**< Oscillator field width. */
#define XRTC_CTL_OSC_MASK    0x0f000000U /**< Oscillator field mask. */
#define XRTC_CTL_OSC_DEFVAL  0x1U /**< Oscillator field default value. */

#define XRTC_CTL_SLVERR_EN_SHIFT   0U /**< Slave error enable field shift. */
#define XRTC_CTL_SLVERR_EN_WIDTH   1U /**< Slave error enable field width. */
/** Slave error enable field mask. */
#define XRTC_CTL_SLVERR_EN_MASK    0x00000001U
/** Slave error enable field default value. */
#define XRTC_CTL_SLVERR_EN_DEFVAL  0x0U
/** @} */

/**
 * @name Safety Check Register
 * This register contains bits for configuring
 * safety check.
 * @{
 */
/**
 * XrtcSftyChk register offset.
 */
#define XRTC_SFTY_CHK_OFFSET    0x00000050U
#define XRTC_SFTY_CHK_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_SFTY_CHK_REG_SHIFT   0U /**< Register field shift. */
#define XRTC_SFTY_CHK_REG_WIDTH   32U /**< Register field width. */
#define XRTC_SFTY_CHK_REG_MASK    0xffffffffU /**< Register field mask. */
#define XRTC_SFTY_CHK_REG_DEFVAL  0x0U /**< Register field default value. */

/**
 * XrtcEco register offset.
 */
#define XRTC_ECO_OFFSET    0x00000060U
#define XRTC_ECO_RSTVAL   0x00000000U /**< Register reset value. */

#define XRTC_ECO_REG_SHIFT   0U /**< Register field shift. */
#define XRTC_ECO_REG_WIDTH   32U /**< Register field width. */
#define XRTC_ECO_REG_MASK    0xffffffffU /**< Register field mask. */
#define XRTC_ECO_REG_DEFVAL  0x0U /**< Register field default value. */
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]	RegisterAddr Register address in the address space of the
*		RTC device.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XRtcPsu_ReadReg(UINTPTR RegisterAddr)
*
*****************************************************************************/
#define XRtcPsu_ReadReg(RegisterAddr) Xil_In32(RegisterAddr)

/****************************************************************************/
/**
*
* @brief Writes the given register.
*
* @param[in]	RegisterAddr Register address in the address space of the
*		RTC device.
* @param[in]	Data 32-bit value to write to the register.
*
* @note		C-style signature:
*		void XRtcPsu_WriteReg(UINTPTR RegisterAddr, u32 Data)
*
*****************************************************************************/
#define XRtcPsu_WriteReg(RegisterAddr, Data) Xil_Out32(RegisterAddr, (u32)(Data))

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* XRTC_HW_H_ */
