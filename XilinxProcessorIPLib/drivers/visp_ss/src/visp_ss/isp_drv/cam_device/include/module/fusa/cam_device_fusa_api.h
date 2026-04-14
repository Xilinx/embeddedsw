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

#ifndef CAMDEV_FUSA_API_H
#define CAMDEV_FUSA_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 23_cam_device_fusa VsCamDevice E01C23 Device_FUSA Definitions
 * @brief Provides interfaces for controlling the functional safety module
 * working in the ISP pipeline.
 * @{
 *
 */

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa event ID.
 *
 *****************************************************************************/
typedef enum CamDeviceEventId_e
{
    CAMDEV_ISP_EVENT_INVALID               = 0x0000,                             /**< Invalid event (only for an internal evaluation) */
    CAMDEV_ISP_EVENT_BASE                  = 0x1000,                             /**< The base ISP events */
    CAMDEV_ISP_EVENT_FUSA                  = (CAMDEV_ISP_EVENT_BASE + 0x0011)    /**< FuSa events */
} CamDeviceEventId_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa masked interrupt status.
 *
 *****************************************************************************/
typedef struct CamDeviceFuSaMisVal_s {
    uint32_t    fusaEcc1MisVal;    /**< ECC1 masked interrupt status */
    uint32_t    fusaEcc2MisVal;    /**< ECC2 masked interrupt status */
    uint32_t    fusaEcc3MisVal;    /**< ECC3 masked interrupt status */
    uint32_t    fusaEcc4MisVal;    /**< ECC4 masked interrupt status */
    uint32_t    fusaEcc5MisVal;    /**< ECC5 masked interrupt status */
    uint32_t    fusaEcc6MisVal;    /**< ECC6 masked interrupt status */
    uint32_t    fusaDupMisVal;    /**< Dump masked interrupt status */
    uint32_t    fusaParityMisVal;    /**< Parity masked interrupt status */
    uint32_t    fusaLv1MisVal;    /**< Level 1 masked interrupt status */
} CamDeviceFuSaMisVal_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa event callback function pointer.
 *
 *****************************************************************************/
typedef void (*CamDeviceEventFunc_t)
(
    const CamDeviceEventId_t  evtId,
    CamDeviceFuSaMisVal_t     *param,
    void                      *pUserContext
);

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa CRC (Cyclic redundancy check) level.
 *
 *****************************************************************************/
typedef enum CamDeviceFusaCrcLevel_e
{
    CAMDEV_FUSA_CRC_LINE_ROI_LEVEL    = 0,    /**< CRC line ROI level */
    CAMDEV_FUSA_CRC_FRAME_LEVEL       = 1,    /**< CRC frame level */
    CAMDEV_FUSA_CRC_MAX               = 2
} CamDeviceFusaCrcLevel_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa CRC reverse mode.
 *
 *****************************************************************************/
typedef enum CamDeviceFusaCrcReverseMode_e
{
    CAMDEV_FUSA_CRC_NOT_REVERSE       = 0,    /**< Not reverse */
    CAMDEV_FUSA_CRC_TOTAL_REVERSE     = 1,    /**< Total reverse mode */
    CAMDEV_FUSA_CRC_BYTE_REVERSE      = 2    /**< Byte reverse mode */
} CamDeviceFusaCrcReverseMode_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa fault injection configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFaultInjectConfig_s
{
    bool_t ahbTimeOutEn;    /**< AHB (Advanced high performance bus) timeout enable */
    bool_t dupEn;    /**< Dumping enable */
    bool_t ecc2bitEn;    /**< ECC (Error checking and correcting) 2-bit enable */
    bool_t ecc1bitEn;    /**< ECC 1-bit enable */
} CamDeviceFaultInjectConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa CRC ROI configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaCrcRoiConfig_s {
    uint32_t    crcRoiH;    /**< Horizontal CRC ROI */
    uint32_t    crcRoiV;    /**< Vertical CRC ROI */
    uint32_t    crcRoiOffH;    /**< Horizontal CRC ROI Offset */
    uint32_t    crcRoiOffV;    /**< Vertical CRC ROI Offset */
} CamDeviceFusaCrcRoiConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa CRC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaCrcConfig_s {
    CamDeviceFusaCrcRoiConfig_t crcMpRoi;    /**< CRC ROI for main path */
    CamDeviceFusaCrcRoiConfig_t crcSp1Roi;    /**< CRC ROI for self path 1 */
    CamDeviceFusaCrcRoiConfig_t crcSp2Roi;    /**< CRC ROI for self path 2 */

    CamDeviceFusaCrcLevel_t crcLevel;    /**< CRC level */

    CamDeviceFusaCrcReverseMode_t crcInRevEnMode;    /**< CRC input reverse mode */
    bool_t crcOutRevEn;    /**< CRC output reverse enable */
    bool_t crcXorEn;    /**< CRC XOR operation enable */
} CamDeviceFusaCrcConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa pixel count size.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaPixelCountSize_s {
    uint32_t outH;    /**< Horizontal output size */
    uint32_t outV;    /**< Vertical output size */
} CamDeviceFusaPixelCountSize_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa pixel count configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaPixelCountConfig_s {
    CamDeviceFusaPixelCountSize_t   mrszSize;    /**< Pixel count size of main path */
    CamDeviceFusaPixelCountSize_t   srsz1Size;    /**< Pixel count size of self path 1 */
    CamDeviceFusaPixelCountSize_t   srsz2Size;    /**< Pixel count size of self path 2 */
    CamDeviceFusaPixelCountSize_t   filtSize;    /**< Pixel count size of filter */
} CamDeviceFusaPixelCountConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa BIST (Built in self test) size.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaBistSize_s {
    uint32_t tpgBistImgHblank;    /**< Horizontal TPG BIST image blank */
    uint32_t tpgBistImgVblank;    /**< Vertical TPG BIST image blank */
    uint32_t tpgBistImgHsize;    /**< Horizontal TPG BIST image size */
    uint32_t tpgBistImgVsize;    /**< Vertical TPG BIST image size */
    uint32_t tpgBistSeed;    /**< TPG BIST seed */
} CamDeviceFusaBistSize_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa BIST2 path CRC.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaBist2PathCrc_s {
    uint32_t yCrc;    /**< Y channel CRC value */
    uint32_t cbCrc;    /**< Cb channel CRC value */
    uint32_t crCrc;    /**< Cr channel CRC value */
} CamDeviceFusaBist2PathCrc_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice FuSa BIST configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaBistConfig_s {
    CamDeviceFusaBistSize_t tpgBistStage[3];    /**< TPG BIST stage */
    uint32_t bistCrcStage1Frm[3];    /**< CRC BIST frames of stage 1 */
    uint32_t bistCrcStage2Frm[3];    /**< CRC BIST frames of stage 2 */
    CamDeviceFusaBist2PathCrc_t bistMpCrcStage3Frm[3];    /**< CRC main path BIST frames of stage 3 */
    CamDeviceFusaBist2PathCrc_t bistSp1CrcStage3Frm[3];    /**< CRC self path 1 BIST frames of stage 3 */
    CamDeviceFusaBist2PathCrc_t bistSp2CrcStage3Frm[3];    /**< CRC self path 2 BIST frames of stage 3 */
} CamDeviceFusaBistConfig_t;

/*****************************************************************************/
/**
 * @brief   This function enables FuSa fault injection.
 * @startuml VsiCamDeviceFusaFaultInjectEnable
 * !include E01_External/VsiCamDeviceFusaFaultInjectEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to FuSa fault injection configuration.
 * @details This function calls: \ref CamEngineIspFuSaFaultInjectEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaFaultInjectEnable
(
    CamDeviceHandle_t                   hCamDevice,
    const CamDeviceFaultInjectConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables FuSa ECC.
 * @startuml VsiCamDeviceFusaEccEnable
 * !include E01_External/VsiCamDeviceFusaEccEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaEccEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaEccEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables FuSa ECC.
 * @startuml VsiCamDeviceFusaEccDisable
 * !include E01_External/VsiCamDeviceFusaEccDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaEccDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaEccDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables FuSa pixel count.
 * @startuml VsiCamDeviceFusaPixelCountEnable
 * !include E01_External/VsiCamDeviceFusaPixelCountEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaPixelCountEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables FuSa pixel count.
 * @startuml VsiCamDeviceFusaPixelCountDisable
 * !include E01_External/VsiCamDeviceFusaPixelCountDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaPixelCountDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets FuSa pixel count configuration.
 * @startuml VsiCamDeviceFusaPixelCountSetConfig
 * !include E01_External/VsiCamDeviceFusaPixelCountSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaPixelCountSetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountSetConfig
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets FuSa pixel count configuration.
 * @startuml VsiCamDeviceFusaPixelCountGetConfig
 * !include E01_External/VsiCamDeviceFusaPixelCountGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to FUSA pixel count configuration.
 * @details This function calls: \ref CamEngineIspFuSaPixelCountGetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountGetConfig
(
    CamDeviceHandle_t                hCamDevice,
    CamDeviceFusaPixelCountConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables FuSa timeout.
 * @startuml VsiCamDeviceFusaTimeoutEnable
 * !include E01_External/VsiCamDeviceFusaTimeoutEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pathEnable  path enable status
 * @details This function calls: \ref CamEngineIspFuSaTimeOutEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.

 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaTimeoutEnable
(
    CamDeviceHandle_t  hCamDevice,
    uint8_t            pathEnable
);

/*****************************************************************************/
/**
 * @brief   This function disables FuSa timeout.
 * @startuml VsiCamDeviceFusaTimeoutDisable
 * !include E01_External/VsiCamDeviceFusaTimeoutDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaTimeOutDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaTimeoutDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables FuSa BIST.
 * @startuml VsiCamDeviceFusaBistEnable
 * !include E01_External/VsiCamDeviceFusaBistEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaBistEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaBistEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables FuSa BIST.
 * @startuml VsiCamDeviceFusaBistDisable
 * !include E01_External/VsiCamDeviceFusaBistDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaBistDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaBistDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables FuSa BIST power-up.
 * @startuml VsiCamDeviceFuSaBistPowerUpEnable
 * !include E01_External/VsiCamDeviceFuSaBistPowerUpEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaBistPowerUpEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFuSaBistPowerUpEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables FuSa BIST power-up.
 * @startuml VsiCamDeviceFuSaBistPowerUpDisable
 * !include E01_External/VsiCamDeviceFuSaBistPowerUpDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaBistPowerUpDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFuSaBistPowerUpDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables FuSa CRC.
 * @startuml VsiCamDeviceFusaCrcEnable
 * !include E01_External/VsiCamDeviceFusaCrcEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaCrcEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables FuSa CRC.
 * @startuml VsiCamDeviceFusaCrcDisable
 * !include E01_External/VsiCamDeviceFusaCrcDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaCrcDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets FuSa CRC configuration.
 * @startuml VsiCamDeviceFusaCrcSetConfig
 * !include E01_External/VsiCamDeviceFusaCrcSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to FuSa CRC configuration.
 * @details This function calls: \ref CamEngineIspFuSaCrcSetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcSetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceFusaCrcConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets FuSa CRC configuration.
 * @startuml VsiCamDeviceFusaCrcGetConfig
 * !include E01_External/VsiCamDeviceFusaCrcGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to FuSa CRC configuration.
 * @details This function calls: \ref CamEngineIspFuSaCrcGetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcGetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceFusaCrcConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets FuSa BIST configuration.
 * @startuml VsiCamDeviceFusaBistGetConfig
 * !include E01_External/VsiCamDeviceFusaBistGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to FuSa BIST configuration.
 * @details This function calls: \ref CamEngineIspFuSaBistGetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaBistGetConfig
(
    CamDeviceHandle_t          hCamDevice,
    CamDeviceFusaBistConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function registers FuSa event callback.
 * @startuml VsiCamDeviceFusaRegisterEventCb
 * !include E01_External/VsiCamDeviceFusaRegisterEventCb.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       func          Callback function.
 * @param[in]       pUserContext  Pointer to user context.
 * @details This function calls: \ref CamEngineIspFuSaRegisterEventCb
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_BUSY            Operation failed due to system occupied
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaRegisterEventCb
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceEventFunc_t    func,
    void                    *pUserContext
);

/*****************************************************************************/
/**
 * @brief   This function deregisters FuSa event callback.
 * @startuml VsiCamDeviceFusaDeRegisterEventCb
 * !include E01_External/VsiCamDeviceFusaDeRegisterEventCb.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineIspFuSaDeRegisterEventCb
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_BUSY            Operation failed due to system occupied
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaDeRegisterEventCb
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets FuSa.
 * @startuml VsiCamDeviceFusaReset
 * !include E01_External/VsiCamDeviceFusaReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets FuSa version.
 * @startuml VsiCamDeviceFusaGetVersion
 * !include E01_External/VsiCamDeviceFusaGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to FuSa version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaGetVersion
(
    CamDeviceHandle_t    hCamDevice,
    uint32_t             *pVersion
);

/** @} 23_cam_device_fusa */

#ifdef __cplusplus
}
#endif

#endif
