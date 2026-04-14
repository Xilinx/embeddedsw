// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
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

#ifndef CAMDEV_RGBIR_API_H
#define CAMDEV_RGBIR_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 22_cam_device_rgbir VsCamDevice E01C22 Device_RGBIR Definitions
 * @brief Provides interfaces for controlling the RGB infrared radiation module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_RGBIR_CC_MATRIX_SIZE 12

/*****************************************************************************/
/**
 * @brief   VsCamDevice RGBIR output Bayer pattern.
 *
 *****************************************************************************/
typedef enum CamDeviceRgbirOutPat_e {
    CAMDEV_RGBIR_OUT_PAT_RGGB = 0,    /**< Output RGB RAW pattern RGGB*/
    CAMDEV_RGBIR_OUT_PAT_GRBG,        /**< Output RGB RAW pattern GRBG*/
    CAMDEV_RGBIR_OUT_PAT_GBRG,        /**< Output RGB RAW pattern GBRG*/
    CAMDEV_RGBIR_OUT_PAT_BGGR,        /**< Output RGB RAW pattern BGGR*/
    CAMDEV_RGBIR_OUT_PAT_MAX
}CamDeviceRgbirOutPat_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice RGBIR IR RAW path selection.
 *
 *****************************************************************************/
typedef enum CamDeviceRgbirIrPathSel_e {
    CAMDEV_RGBIR_IR_RAW_SELECT_MP      = 0, /**< Select MP RAW path */
    CAMDEV_RGBIR_IR_RAW_SELECT_SELF1   = 1, /**< Select SP1 yuv only path */
} CamDeviceRgbirIrPathSel_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice RGBIR status structure.
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
 * @brief   VsCamDevice RGBIR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceRgbirStatus_s {
    bool_t enable;                /**< RGBIR enable status */
    bool_t rcccEnable;            /**< RGBIR RCCC enable status */
    bool_t lrEnable;              /**< RGBIR LR enable status */
    CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
    CamDeviceRgbirConfig_t currentCfg;    /**< RGBIR current configuration */
}CamDeviceRgbirStatus_t;

/*****************************************************************************/
/**
 * @brief   This function enables RGBIR.
 * @startuml VsiCamDeviceRgbirEnable
 * !include E01_External/VsiCamDeviceRgbirEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirEnable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables RGBIR.
 * @startuml VsiCamDeviceRgbirDisable
 * !include E01_External/VsiCamDeviceRgbirDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineRgbirDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirDisable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables RCCC.
 * @startuml VsiCamDeviceRgbirRcccEnable
 * !include E01_External/VsiCamDeviceRgbirRcccEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirRcccEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirRcccEnable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables RCCC.
 * @startuml VsiCamDeviceRgbirRcccDisable
 * !include E01_External/VsiCamDeviceRgbirRcccDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineRgbirRcccDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirRcccDisable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables RGBIR IR RAW out.
 * @startuml VsiCamDeviceRgbirIrRawOutEnable
 * !include E01_External/VsiCamDeviceRgbirIrRawOutEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirIrRawOutEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirIrRawOutEnable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables RGBIR IR RAW out.
 * @startuml VsiCamDeviceRgbirIrRawOutDisable
 * !include E01_External/VsiCamDeviceRgbirIrRawOutDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineRgbirIrRawOutdisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirIrRawOutDisable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets RGBIR configuration parameters.
 * @startuml VsiCamDeviceRgbirSetConfig
 * !include E01_External/VsiCamDeviceRgbirSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to RGBIR configuration.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirSetConfig,
 * \ref CamEngineRgbirSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceRgbirReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceRgbirConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR configuration parameters.
 * @startuml VsiCamDeviceRgbirGetConfig
 * !include E01_External/VsiCamDeviceRgbirGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to RGBIR configuration.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirGetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceRgbirConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets RGBIR output pattern parameters.
 * @startuml VsiCamDeviceRgbirSetOutPattern
 * !include E01_External/VsiCamDeviceRgbirSetOutPattern.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       outPattern  RGBIR output pattern parameters.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirSetOutBpt
 * @details This function is called by: User application, \ref VsiCamDeviceRgbirReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirSetOutPattern
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceRgbirOutPat_t   outPattern
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR output pattern parameters.
 * @startuml VsiCamDeviceRgbirGetOutPattern
 * !include E01_External/VsiCamDeviceRgbirGetOutPattern.plantuml
 * @enduml
 * @param[in]       hCamDevice   Handle to the VsCamDevice instance.
 * @param[inout]    pOutPattern  Pointer to RGBIR output pattern parameters.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirGetOutBpt
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetOutPattern
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceRgbirOutPat_t  *pOutPattern
);

/*****************************************************************************/
/**
 * @brief   This function sets output path for RGBIR IR RAW.
 * @startuml VsiCamDeviceRgbirSetIrPathSelect
 * !include E01_External/VsiCamDeviceRgbirSetIrPathSelect.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       irPathSelect  IR path parameters.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirSetSp1IrSel
 * @details This function is called by: User application, \ref VsiCamDeviceRgbirReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirSetIrPathSelect
(
    CamDeviceHandle_t            hCamDevice,
    CamDeviceRgbirIrPathSel_t    irPathSelect
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR IR RAW output path.
 * @startuml VsiCamDeviceRgbirGetIrPathSelect
 * !include E01_External/VsiCamDeviceRgbirGetIrPathSelect.plantuml
 * @enduml
 * @param[in]       hCamDevice     Handle to the VsCamDevice instance.
 * @param[inout]    pIrPathSelect  Pointer to IR path parameters.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirGetSp1IrSel
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetIrPathSelect
(
    CamDeviceHandle_t             hCamDevice,
    CamDeviceRgbirIrPathSel_t     *pIrPathSelect
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR status.
 * @startuml VsiCamDeviceRgbirGetStatus
 * !include E01_External/VsiCamDeviceRgbirGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to RGBIR status.
 * @details This function calls: \ref CamDeviceRgbirCheckInputSupport, \ref CamEngineRgbirGetStatus
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirGetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceRgbirStatus_t  *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets RGBIR version.
 * @startuml VsiCamDeviceRgbirGetVersion
 * !include E01_External/VsiCamDeviceRgbirGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to RGBIR version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
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
 * @startuml VsiCamDeviceRgbirReset
 * !include E01_External/VsiCamDeviceRgbirReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceRgbirSetConfig, \ref VsiCamDeviceRgbirSetOutPattern,
 * \ref VsiCamDeviceRgbirSetIrPathSelect
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceRgbirReset
(
    CamDeviceHandle_t             hCamDevice
);

/** @} 22_cam_device_rgbir */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_RGBIR_API_H */
