/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2014-2022 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

/**
 * @cond RGBIR_V2_1
 *
 * @defgroup cam_device_rgbir_v2_1 CamDevice RGBIR V2.1 Definitions
 * @{
 *
 */

#ifndef CAMDEV_RGBIR_API_H
#define CAMDEV_RGBIR_API_H

#include "cam_device_common.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_RGBIR_CC_MATRIX_SIZE 12


/*****************************************************************************/
/**
 * @brief   CamDevice RGBIR output Bayer pattern.
 *
 *****************************************************************************/
typedef enum CamDeviceRgbirOutPat_e {
	CAMDEV_RGBIR_OUT_PAT_RGGB = 0,    /**< Output RGB RAW pattern RGGB*/
	CAMDEV_RGBIR_OUT_PAT_GRBG,        /**< Output RGB RAW pattern GRBG*/
	CAMDEV_RGBIR_OUT_PAT_GBRG,        /**< Output RGB RAW pattern GBRG*/
	CAMDEV_RGBIR_OUT_PAT_BGGR,        /**< Output RGB RAW pattern BGGR*/
	CAMDEV_RGBIR_OUT_PAT_MAX,
	CAMDEV_DUMMY_076 = 0xDEADFEED
} CamDeviceRgbirOutPat_t;

/*****************************************************************************/
/**
 * @brief   CamDevice RGBIR IR RAW path selection.
 *
 *****************************************************************************/
typedef enum CamDeviceRgbirIrPathSel_e {
	CAMDEV_RGBIR_IR_RAW_SELECT_MP = 0, /**< Select MP RAW path */
	CAMDEV_RGBIR_IR_RAW_SELECT_SELF1 = 1, /**< Select SP1 yuv only path */
	CAMDEV_DUMMY_077 = 0xDEADFEED
} CamDeviceRgbirIrPathSel_t;

/******************************************************************************/
/**
 * @brief   CamDevice RGBIR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceRgbirConfig_s {
	float32_t ccMatrix[CAMDEV_RGBIR_CC_MATRIX_SIZE];   /**< IR coefficient 3x4 matrix: 03 is the first row of the matrix, 47 is the second row, and 811 is the third row */
	uint16_t dpccMidTh[CAMDEV_RGBIR_CHANNEL_NUM];  /**< Median bad point threshold of the four channels:\n Position 0 corresponds to the IR channel\n Position 1 corresponds to the Red channel\n */
	/**< Position 2 corresponds to the Green channel\n Position 3 corresponds to the Blue channel */
	uint16_t dpccTh[CAMDEV_RGBIR_CHANNEL_NUM];     /**< Bad point detection threshold of four channels:\n Position 0 corresponds to the IR channel\n Position 1 corresponds to the Red channel\n */
	/**< Position 2 corresponds to the Green channel\n Position 3 corresponds to the Blue channel */
	uint32_t irThreshold;                          /**< RGBIR IR threshold */
	uint32_t lThreshold;                           /**< RGBIR L threshold */
} CamDeviceRgbirConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice RGBIR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceRgbirStatus_s {
	bool_t enable;                /**< RGBIR enable status */
	bool_t rcccEnable;            /**< RGBIR RCCC enable status */
	bool_t lrEnable;              /**< RGBIR LR enable status */
	CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
	CamDeviceRgbirConfig_t currentCfg;    /**< RGBIR current configuration */
} CamDeviceRgbirStatus_t;


/*****************************************************************************/
/**
 * @brief   This function enables RGBIR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables RGBIR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables RCCC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirRcccEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables RCCC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirRcccDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables RGBIR IR RAW out.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirIrRawOutEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables RGBIR IR RAW out.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirIrRawOutDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets RGBIR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to RGBIR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to RGBIR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets RGBIR output pattern parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   outPattern          RGBIR output pattern parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/\
RESULT VsiCamDeviceRgbirSetOutPattern
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirOutPat_t outPattern
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR output pattern parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pOutPattern         Pointer to RGBIR output pattern parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/\
RESULT VsiCamDeviceRgbirGetOutPattern
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirOutPat_t *pOutPattern
);

/*****************************************************************************/
/**
 * @brief   This function sets output path for RGBIR IR RAW.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   irPathSelect        IR path parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirSetIrPathSelect
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirIrPathSel_t irPathSelect
);


/*****************************************************************************/
/**
 * @brief    This function gets RGBIR IR RAW output path.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pIrPathSelect       Pointer to IR path parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetIrPathSelect
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirIrPathSel_t *pIrPathSelect
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR status
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to RGBIR status

 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRgbirStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR version
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to RGBIR version

 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets RGBIR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirReset
(
	CamDeviceHandle_t hCamDevice
);

/* @} cam_device_rgbir_v2_1 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_RGBIR_API_H */
