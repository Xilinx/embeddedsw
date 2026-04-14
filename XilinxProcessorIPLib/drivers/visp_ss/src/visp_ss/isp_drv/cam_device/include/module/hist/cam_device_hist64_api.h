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

#ifndef CAMDEV_HIST64_API_H
#define CAMDEV_HIST64_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 29_cam_device_hist64 VsCamDevice E01C29 Device_Hist64 Definitions
 * @brief Provides interfaces for controlling the histogram 64 module working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_HIST64_GRID_ITEMS         25  /**< The number of grid sub windows */
#define CAMDEV_HIST64_NUM_BINS           32  /**< number of bins */

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST64 mode.
 *
 *****************************************************************************/
typedef enum CamDeviceHist64Mode_e {
    CAMDEV_HIST64_MODE_DISABLE       = 0,    /**< Disabled, no measurements  */
    CAMDEV_HIST64_MODE_ONE_FROM_YRGB = 1,    /**< Control Y/R/G/B histogram via coefficients coeff_r/g/b */
    CAMDEV_HIST64_MODE_R             = 2,    /**< R histogram */
    CAMDEV_HIST64_MODE_GR            = 3,    /**< Gr histogram */
    CAMDEV_HIST64_MODE_B             = 4,    /**< B histogram */
    CAMDEV_HIST64_MODE_GB            = 5,    /**< Gb histogram */
    CAMDEV_IST64_MODE_MAX,
} CamDeviceHist64Mode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST64 channel.
 *
 *****************************************************************************/
typedef enum CamDeviceHist64Channel_e
{
    CAMDEV_HIST64_CHANNEL_0             = 0,    /* After De-gamma channel */
    CAMDEV_HIST64_CHANNEL_1             = 1,    /* After LSC channel */
    CAMDEV_HIST64_CHANNEL_2             = 2,    /* After AWB GAIN channel */
    CAMDEV_HIST64_CHANNEL_3             = 3,    /* RGB domain after WDR channel */
    CAMDEV_HIST64_CHANNEL_4             = 4,    /* After Demosaic channel */
    CAMDEV_HIST64_CHANNEL_5             = 5,    /* RGB domain after cross talk channel */
    CAMDEV_HIST64_CHANNEL_6             = 6,    /* Reserved */
    CAMDEV_HIST64_CHANNEL_7             = 7,    /* RGB domain after gamma out channel */
    CAMDEV_HIST64_CHANNEL_MAX,
} CamDeviceHist64Channel_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST64 bins.
 *
 *****************************************************************************/
typedef struct CamDeviceHist64Bins_s
{
    uint32_t bins[CAMDEV_HIST64_NUM_BINS];   /**< HIST64 bins */
} CamDeviceHist64Bins_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST64 configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHist64Config_s
{
    CamDeviceHist64Mode_t    mode;    /**< HIST64 mode */
    CamDeviceHist64Channel_t channel;    /**< HIST64 channel */
    CamDeviceWindow_t        window;    /**< HIST64 window */
    float32_t rCoeff;    /**< Red channel coefficient */
    float32_t gCoeff;    /**< Green channel coefficient */
    float32_t bCoeff;    /**< Blue channel coefficient */
} CamDeviceHist64Config_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice HIST64 status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHist64Status_s
{
    bool_t  enable;    /**< HIST64 enable status*/
}CamDeviceHist64Status_t;

/*****************************************************************************/
/**
 * @brief   This function sets HIST64 configuration parameters.
 * @startuml VsiCamDeviceHist64SetConfig
 * !include E01_External/VsiCamDeviceHist64SetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to HIST64 configuration.
 * @details This function calls: \ref CamEngineHist64SetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceHist64Reset
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
RESULT VsiCamDeviceHist64SetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist64Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64DG configuration parameters.
 * @startuml VsiCamDeviceHist64GetConfig
 * !include E01_External/VsiCamDeviceHist64GetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to HIST64 configuration.
 * @details This function calls: \ref CamEngineHist64GetConfig
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
RESULT VsiCamDeviceHist64GetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist64Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables HIST64.
 * @startuml VsiCamDeviceHist64Enable
 * !include E01_External/VsiCamDeviceHist64Enable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineHist64Enable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64Enable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HIST64.
 * @startuml VsiCamDeviceHist64Disable
 * !include E01_External/VsiCamDeviceHist64Disable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineHist64Disable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64Disable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 statistical data.
 * @startuml VsiCamDeviceHist64GetStatistic
 * !include E01_External/VsiCamDeviceHist64GetStatistic.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pHistBins   Pointer to HIST64 status.
 * @details This function calls: \ref CamEngineHist64GetStatistic
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
RESULT VsiCamDeviceHist64GetStatistic
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceHist64Bins_t   *pHistBins
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 status.
 * @startuml VsiCamDeviceHist64GetStatus
 * !include E01_External/VsiCamDeviceHist64GetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to HIST64 status.
 * @details This function calls: \ref CamEngineHist64IsEnabled
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
RESULT VsiCamDeviceHist64GetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist64Status_t    *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 version.
 * @startuml VsiCamDeviceHist64GetVersion
 * !include E01_External/VsiCamDeviceHist64GetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to HIST64 version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64GetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets HIST64.
 * @startuml VsiCamDeviceHist64Reset
 * !include E01_External/VsiCamDeviceHist64Reset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceHist64SetConfig
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
RESULT VsiCamDeviceHist64Reset
(
    CamDeviceHandle_t         hCamDevice
);

/** @} 30_cam_device_hist64 */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_HIST64_API_H */
