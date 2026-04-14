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


#ifndef CAMDEV_HDR_API_H
#define CAMDEV_HDR_API_H


/**
 * @cond HDR_DUMMY
 * @defgroup cam_device_hdr CamDevice GWDR Dummy Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   CamDevice HDR dummy configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHdrConfig_s {
/**Interpolation*/
    uint32_t colorWeight[3]; /**< Weight of ordered color */
}CamDeviceHdrConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice HDR dummy status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHdrStatus_s {
    bool_t enable;            /**< HDR dummy enable status */
}CamDeviceHdrStatus_t;


/******************************************************************************/
 /**
 * @brief   CamDevice HDR dummy bypass select enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceHdrBypassSelect_e {
    CAMDEV_HDR_BYPASS_SELECT_L = 0,  /**< 0: HDR output long frame */
}CamDeviceHdrBypassSelect_t;

/*****************************************************************************/
/**
 * @brief   This function sets HDR dummy configuration parameters.
 * @startuml VsiCamDeviceHdrSetConfig
 * !include module/VsiCamDeviceHdrSetConfig.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pHdrCfg     Pointer to Hdr configuration.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceHdrConfig_t *pHdrCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets HDR dummy configuration parameters.
 * @startuml VsiCamDeviceHdrGetConfig
 * !include module/VsiCamDeviceHdrGetConfig.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pHdrCfg     Pointer to Hdr configuration.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceHdrConfig_t *pHdrCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables HDR dummy.
 * @startuml VsiCamDeviceHdrEnable
 * !include module/VsiCamDeviceHdrEnable.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HDR dummy.
 * @startuml VsiCamDeviceHdrDisable
 * !include module/VsiCamDeviceHdrDisable.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets HDR dummy.
 * @startuml VsiCamDeviceHdrReset
 * !include module/VsiCamDeviceHdrReset.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrReset
(
    CamDeviceHandle_t             hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HDR dummy status.
 * @startuml VsiCamDeviceHdrGetStatus
 * !include module/VsiCamDeviceHdrGetStatus.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pStatus     Pointer to HDR status.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceHdrStatus_t *pHdrStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets HDR dummy version.
 * @startuml VsiCamDeviceHdrGetVersion
 * !include module/VsiCamDeviceHdrGetVersion.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the CamDevice instance.
 * @param[in]    pVersion    Pointer to HDR version
 *
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   (For debug only) This function disables HDR combination mode and sets the HDR bypass frame.
 * @startuml VsiCamDeviceSetBypassSelectEnable
 * !include module/VsiCamDeviceSetBypassSelectEnable.plantuml
 * @enduml
 * @param[in]    hCamDevice   Handle to the CamDevice instance.
 * @param[in]    bypassFrame  The selected bypass frame.
 * @details this function calls: CamDeviceSetIspLowPower
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetBypassSelectEnable
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceHdrBypassSelect_t  bypassFrame
);

/* @} cam_device_hdr dummy */
/* @endcond */


#endif
