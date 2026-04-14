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

#ifndef CAMDEV_EXP_API_H
#define CAMDEV_EXP_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 26_cam_device_exp VsCamDevice E01C26 Device_EXP Definitions
 * @brief Provides interfaces for controlling the auto exposure statistics
 * module working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_AAA_EXP_GRID_SIZE     32    /**< 3A EXP grid size */
#define CAMDEV_AAA_PIXEL_CHANNEL     4

/******************************************************************************/
/**
 * @brief   VsCamDevice ISP EXP input select.
 *
 *****************************************************************************/
typedef enum CamDeviceExpV2Sel_e {
    CAMDEV_EXP_INPUT_SEL_DEGAMMA   = 0,    /**< Input select of degamma */
    CAMDEV_EXP_INPUT_SEL_AWBGAIN   = 1,    /**< Input select of AWB gain */
    CAMDEV_EXP_INPUT_SEL_WDR3      = 2,    /**< Input select of WDR3 module */
    CAMDEV_EXP_INPUT_SEL_LSC       = 3,    /**< Input select of LSC module */
    CAMDEV_EXP_STATE_MAX
} CamDeviceExpV2Sel_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice ISP EXP statistics data bit type.
 *
 *****************************************************************************/
typedef enum CamDeviceExpV2StatisticsType_e
{
    CAMDEV_EXP_8BIT_DATA      = 0,    /**< EXP 8 bit data */
    CAMDEV_EXP_16BIT_DATA     = 1,    /**< EXP 16 bit data */
    CAMDEV_EXP_24BIT_DATA     = 2,    /**< EXP 24 bit data */
    CAMDEV_EXP_BIT_DATA_MAX
} CamDeviceExpV2StatisticsType_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice EXP statistics configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceExpV2Statistics_s
{
    uint32_t statistics[CAMDEV_AAA_EXP_GRID_SIZE * CAMDEV_AAA_EXP_GRID_SIZE * CAMDEV_AAA_PIXEL_CHANNEL];  /**< EXP statistics data */
    CamDeviceExpV2StatisticsType_t type;    /**< Statistics bit data type */
} CamDeviceExpV2Statistics_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EXP configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceExpV2Config_s
{
    CamDeviceExpV2Sel_t     inputSelect;   /**< EXP input select */
    bool_t                  windowSetCustomEnable;
} CamDeviceExpV2Config_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EXP status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceExpV2Status_s
{
    bool_t  enable;    /**< EXP enable status*/
}CamDeviceExpV2Status_t;

/*****************************************************************************/
/**
 * @brief   This function sets EXP configuration parameters.
 * @startuml VsiCamDeviceExpV2SetConfig
 * !include E01_External/VsiCamDeviceExpV2SetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to EXP configuration.
 * @details This function calls: \ref CamEngineExpV2SetInputSel
 * @details This function is called by: User application, \ref VsiCamDeviceExpV2Reset
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
RESULT VsiCamDeviceExpV2SetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceExpV2Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP configuration parameters.
 * @startuml VsiCamDeviceExpV2GetConfig
 * !include E01_External/VsiCamDeviceExpV2GetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to EXP configuration.
 * @details This function calls: \ref CamEngineExpv2GetInputSel
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
RESULT VsiCamDeviceExpV2GetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceExpV2Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables EXP.
 * @startuml VsiCamDeviceExpV2Enable
 * !include E01_External/VsiCamDeviceExpV2Enable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineExpv2Enable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2Enable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables EXP.
 * @startuml VsiCamDeviceExpV2Disable
 * !include E01_External/VsiCamDeviceExpV2Disable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineExpv2Disable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2Disable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets EXP statistics window.
 * @startuml VsiCamDeviceExpV2SetMeasureWindow
 * !include E01_External/VsiCamDeviceExpV2SetMeasureWindow.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pWindow     Pointer to window configuration.
 * @details This function calls: \ref CamEngineExpv2SetMeasuringWindow
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
RESULT VsiCamDeviceExpV2SetMeasureWindow
(
    CamDeviceHandle_t         hCamDevice,
    const CamDeviceWindow_t   *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP statistics window.
 * @startuml VsiCamDeviceExpV2GetMeasureWindow
 * !include E01_External/VsiCamDeviceExpV2GetMeasureWindow.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pWindow     Pointer to window configuration.
 * @details This function calls: \ref CamEngineExpv2GetMeasuringWindow
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
RESULT VsiCamDeviceExpV2GetMeasureWindow
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceWindow_t        *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP statistic parameters.
 * @startuml VsiCamDeviceExpV2GetStatistics
 * !include E01_External/VsiCamDeviceExpV2GetStatistics.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pExpStatic  Pointer to the EXP statistics.
 * @details This function calls: \ref CamEngineExpV2GetStatistics
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
RESULT VsiCamDeviceExpV2GetStatistics
(
    CamDeviceHandle_t           hCamDevice,
    CamDeviceExpV2Statistics_t  *pExpStatic
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP status.
 * @startuml VsiCamDeviceExpV2GetStatus
 * !include E01_External/VsiCamDeviceExpV2GetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to EXP status.
 * @details This function calls: \ref CamEngineExpv2IsEnable
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
RESULT VsiCamDeviceExpV2GetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceExpV2Status_t    *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP version.
 * @startuml VsiCamDeviceExpV2GetVersion
 * !include E01_External/VsiCamDeviceExpV2GetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to EXP version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2GetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t          *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets EXP.
 * @startuml VsiCamDeviceExpV2Reset
 * !include E01_External/VsiCamDeviceExpV2Reset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceExpV2SetConfig
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
RESULT VsiCamDeviceExpV2Reset
(
    CamDeviceHandle_t         hCamDevice
);

/** @} 26_cam_device_exp */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_EXP_API_H */
