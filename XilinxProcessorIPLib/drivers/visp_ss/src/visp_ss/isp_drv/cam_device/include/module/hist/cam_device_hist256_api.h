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

#ifndef CAMDEV_HIST256_API_H
#define CAMDEV_HIST256_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 04_cam_device_hist256 VsCamDevice E01C04 Device_Hist256 Definitions
 * @brief Provides interfaces for controlling the histogram 256 module working
 * in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_HIST256_GRID_ITEMS         25  /**< The number of grid sub windows */
#define CAMDEV_HIST256_NUM_BINS           256  /**< The number of bins */

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST256 mode.
 *
 *****************************************************************************/
typedef enum CamDeviceHist256Mode_e {
    CAMDEV_HIST256_MODE_DISABLE      = 0,    /**< Disabled, no measurements  */
    CAMDEV_HIST256_MODE_RGB_COMBINED = 1,    /**< RGB combined histogram */
    CAMDEV_HIST256_MODE_R            = 2,    /**< R histogram */
    CAMDEV_HIST256_MODE_G            = 3,    /**< G histogram */
    CAMDEV_HIST256_MODE_B            = 4,    /**< B histogram */
    CAMDEV_HIST256_MODE_Y            = 5,    /**< Luminance histogram */
    CAMDEV_HIST256_MODE_MAX,
} CamDeviceHist256Mode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST256 bins.
 *
 *****************************************************************************/
typedef struct CamDeviceHist256Bins_s
{
    uint32_t bins[CAMDEV_HIST256_NUM_BINS];   /**< HIST256 bins */
} CamDeviceHist256Bins_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST256 configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHIST256Config_s
{
    CamDeviceHist256Mode_t    mode;    /**< HIST256 mode */
    CamDeviceWindow_t        window;    /**< HIST256 window */
} CamDeviceHist256Config_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST256 status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHist256Status_s
{
    bool_t  enable;    /**< HIST256 enable status*/
}CamDeviceHist256Status_t;

/*****************************************************************************/
/**
 * @brief   This function sets HIST256 configuration parameters.
 * @startuml VsiCamDeviceHist256SetConfig
 * !include E01_External/VsiCamDeviceHist256SetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to HIST256 configuration.
 * @details This function calls: \ref CamEngineHistSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceHist256Reset
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
RESULT VsiCamDeviceHist256SetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist256Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256DG configuration parameters.
 * @startuml VsiCamDeviceHist256GetConfig
 * !include E01_External/VsiCamDeviceHist256GetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to HIST256 configuration.
 * @details This function calls: \ref CamEngineHistGetConfig
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
RESULT VsiCamDeviceHist256GetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist256Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables HIST256.
 * @startuml VsiCamDeviceHist256Enable
 * !include E01_External/VsiCamDeviceHist256Enable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineHistEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Enable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HIST256.
 * @startuml VsiCamDeviceHist256Disable
 * !include E01_External/VsiCamDeviceHist256Disable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineHistDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Disable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 statistical data.
 * @startuml VsiCamDeviceHist256GetStatistic
 * !include E01_External/VsiCamDeviceHist256GetStatistic.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pHistBins   Pointer to HIST256 status.
 * @details This function calls: \ref CamEngineHistGetStatistic
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
RESULT VsiCamDeviceHist256GetStatistic
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceHist256Bins_t   *pHistBins
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 status.
 * @startuml VsiCamDeviceHist256GetStatus
 * !include E01_External/VsiCamDeviceHist256GetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to HIST256 status.
 * @details This function calls: \ref CamEngineHistIsEnabled
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
RESULT VsiCamDeviceHist256GetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist256Status_t    *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 version.
 * @startuml VsiCamDeviceHist256GetVersion
 * !include E01_External/VsiCamDeviceHist256GetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to HIST256 version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets HIST256.
 * @startuml VsiCamDeviceHist256Reset
 * !include E01_External/VsiCamDeviceHist256Reset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceHist256SetConfig
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
RESULT VsiCamDeviceHist256Reset
(
    CamDeviceHandle_t         hCamDevice
);

/** @} 31_cam_device_hist256 */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_HIST256_API_H */
