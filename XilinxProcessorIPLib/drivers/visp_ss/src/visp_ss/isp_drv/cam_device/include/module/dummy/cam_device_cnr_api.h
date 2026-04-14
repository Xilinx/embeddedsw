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

/**
 *
 * @cond CNR_DUMMY
 * @defgroup cam_device_cnr_api CamDevice CNR Dummy API
 * @{
 *
 */

#ifndef CAMDEV_CNR_API_H
#define CAMDEV_CNR_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct CamDeviceCnrConfig_s {
    uint8_t nop;
} CamDeviceCnrConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice CNR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCnrStatus_s {
    bool_t enable;    /**< EE enable status */
}CamDeviceCnrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets CNR configuration parameters.
 * @startuml VsiCamDeviceCnrSetConfig
 * !include module/VsiCamDeviceCnrSetConfig.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pCnrCfg     Pointer to CNR Configuration
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceCnrConfig_t *pCnrCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CNR configuration parameters.
 * @startuml VsiCamDeviceCnrGetConfig
 * !include module/VsiCamDeviceCnrGetConfig.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pCnrCfg     Pointer to CNR Configuration
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceCnrConfig_t *pCnrCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables CNR.
 * @startuml VsiCamDeviceCnrEnable
 * !include module/VsiCamDeviceCnrEnable.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrEnable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CNR.
 * @startuml VsiCamDeviceCnrDisable
 * !include module/VsiCamDeviceCnrDisable.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrDisable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CNR status.
 * @startuml VsiCamDeviceCnrGetStatus
 * !include module/VsiCamDeviceCnrGetStatus.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pStatus     Pointer to CNR status.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceCnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CNR.
 * @startuml VsiCamDeviceCnrReset
 * !include module/VsiCamDeviceCnrReset.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrReset
(
    CamDeviceHandle_t    hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CNR version.
 * @startuml VsiCamDeviceCnrGetVersion
 * !include module/VsiCamDeviceCnrGetVersion.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pVersion    Pointer to CNR version
 *
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

#ifdef __cplusplus
}
#endif


/* @} cam_device_cnr_api dummy*/
/* @endcond */

#endif /* CAMDEV_CNR_API_H */
