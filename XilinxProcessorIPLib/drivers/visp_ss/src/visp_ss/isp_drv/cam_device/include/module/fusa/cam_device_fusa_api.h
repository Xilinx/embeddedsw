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

#ifndef CAMDEV_FUSA_API_H
#define CAMDEV_FUSA_API_H

#include "cam_device_common.h"

/**
 * @cond FUSA
 *
 * @defgroup cam_device_fusa CamDevice FuSa Definitions
 * @{
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa event ID.
 *
 *****************************************************************************/
typedef enum CamDeviceEventId_e {
	CAMDEV_ISP_EVENT_INVALID = 0x0000,                             /**< Invalid event (only for an internal evaluation) */
	CAMDEV_ISP_EVENT_BASE = 0x1000,                             /**< The base ISP events */
	CAMDEV_ISP_EVENT_FUSA = (CAMDEV_ISP_EVENT_BASE + 0x0011),    /**< FuSa events */
	CAMDEV_DUMMY_065 = 0xDEADFEED
} CamDeviceEventId_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa masked interrupt status.
 *
 *****************************************************************************/
typedef struct CamDeviceFuSaMisVal_s {
	uint32_t fusaEcc1MisVal;    /**< ECC1 masked interrupt status */
	uint32_t fusaEcc2MisVal;    /**< ECC2 masked interrupt status */
	uint32_t fusaEcc3MisVal;    /**< ECC3 masked interrupt status */
	uint32_t fusaEcc4MisVal;    /**< ECC4 masked interrupt status */
	uint32_t fusaEcc5MisVal;    /**< ECC5 masked interrupt status */
	uint32_t fusaEcc6MisVal;    /**< ECC6 masked interrupt status */
	uint32_t fusaDupMisVal;    /**< Dump masked interrupt status */
	uint32_t fusaParityMisVal;    /**< Parity masked interrupt status */
	uint32_t fusaLv1MisVal;    /**< Level 1 masked interrupt status */
} CamDeviceFuSaMisVal_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa event callback function pointer.
 *
 *****************************************************************************/
typedef void (*CamDeviceEventFunc_t)
(
	const CamDeviceEventId_t evtId,
	CamDeviceFuSaMisVal_t *param,
	void *pUserContext
);

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa CRC (Cyclic redundancy check) level.
 *
 *****************************************************************************/
typedef enum CamDeviceFusaCrcLevel_e {
	CAMDEV_FUSA_CRC_LINE_ROI_LEVEL = 0,    /**< CRC line ROI level */
	CAMDEV_FUSA_CRC_FRAME_LEVEL = 1,    /**< CRC frame level */
	CAMDEV_FUSA_CRC_MAX = 2,
	CAMDEV_DUMMY_066 = 0xDEADFEED
} CamDeviceFusaCrcLevel_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa CRC reverse mode.
 *
 *****************************************************************************/
typedef enum CamDeviceFusaCrcReverseMode_e {
	CAMDEV_FUSA_CRC_NOT_REVERSE = 0,    /**< Not reverse */
	CAMDEV_FUSA_CRC_TOTAL_REVERSE = 1,    /**< Total reverse mode */
	CAMDEV_FUSA_CRC_BYTE_REVERSE = 2,    /**< Byte reverse mode */
	CAMDEV_DUMMY_067 = 0xDEADFEED
} CamDeviceFusaCrcReverseMode_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa fault injection configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFaultInjectConfig_s {
	bool_t ahbTimeOutEn;    /**< AHB (Advanced high performance bus) timeout enable */
	bool_t dupEn;    /**< Dumping enable */
	bool_t ecc2bitEn;    /**< ECC (Error checking and correcting) 2-bit enable */
	bool_t ecc1bitEn;    /**< ECC 1-bit enable */
} CamDeviceFaultInjectConfig_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa CRC ROI configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaCrcRoiConfig_s {
	uint32_t crcRoiH;    /**< Horizontal CRC ROI */
	uint32_t crcRoiV;    /**< Vertical CRC ROI */
} CamDeviceFusaCrcRoiConfig_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa CRC configuration.
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
 * @brief   CamDevice FuSa pixel count size.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaPixelCountSize_s {
	uint32_t outH;    /**< Horizontal output size */
	uint32_t outV;    /**< Vertical output size */
} CamDeviceFusaPixelCountSize_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa pixel count configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaPixelCountConfig_s {
	CamDeviceFusaPixelCountSize_t mrszSize;    /**< Pixel count size of main path */
	CamDeviceFusaPixelCountSize_t srsz1Size;    /**< Pixel count size of self path 1 */
	CamDeviceFusaPixelCountSize_t srsz2Size;    /**< Pixel count size of self path 2 */
	CamDeviceFusaPixelCountSize_t filtSize;    /**< Pixel count size of filter */
} CamDeviceFusaPixelCountConfig_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa BIST (Built in self test) size.
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
 * @brief   CamDevice FuSa BIST2 path CRC.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaBist2PathCrc_s {
	uint32_t yCrc;    /**< Y channel CRC value */
	uint32_t cbCrc;    /**< Cb channel CRC value */
	uint32_t crCrc;    /**< Cr channel CRC value */
} CamDeviceFusaBist2PathCrc_t;

/*****************************************************************************/
/**
 * @brief   CamDevice FuSa BIST configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceFusaBistConfig_s {
	CamDeviceFusaBistSize_t tpgBistStage[3];    /**< TPG BIST stage */
	uint32_t bistCrcStage1Frm[3];    /**< CRC BIST frames of stage 1 */
	uint32_t bistCrcStage2Frm[3];    /**< CRC BIST frames of stage 2 */
	CamDeviceFusaBist2PathCrc_t bistMpCrcStage3Frm[3];    /**< CRC main path BIST frames of stage 3 */
	CamDeviceFusaBist2PathCrc_t
	bistSp1CrcStage3Frm[3];    /**< CRC self path 1 BIST frames of stage 3 */
	CamDeviceFusaBist2PathCrc_t
	bistSp2CrcStage3Frm[3];    /**< CRC self path 2 BIST frames of stage 3 */
} CamDeviceFusaBistConfig_t;

/****************************************************************************/
/**
 * @brief   This function enables FuSa fault injection.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FuSa fault injection configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaFaultInjectEnable
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceFaultInjectConfig_t *pConfig
);

/****************************************************************************/
/**
 * @brief   This function enables FuSa ECC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaEccEnable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function disables FuSa ECC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaEccDisable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function enables FuSa pixel count.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountEnable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function disables FuSa pixel count.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountDisable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function sets FuSa pixel count configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountSetConfig
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function gets FuSa pixel count configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FuSa pixel count configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaPixelCountGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceFusaPixelCountConfig_t *pConfig
);

/****************************************************************************/
/**
 * @brief   This function enables FuSa timeout.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaTimeoutEnable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function disables FuSa timeout.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaTimeoutDisable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function enables FuSa BIST.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaBistEnable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function disables FuSa BIST.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaBistDisable
(
	CamDeviceHandle_t hCamDevice
);


/****************************************************************************/
/**
 * @brief   This function enables FuSa BIST power-up.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFuSaBistPowerUpEnable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function disables FuSa BIST power-up.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFuSaBistPowerUpDisable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function enables FuSa CRC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcEnable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function disables FuSa CRC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcDisable
(
	CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   This function sets FuSa CRC configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FuSa CRC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceFusaCrcConfig_t *pConfig
);

/****************************************************************************/
/**
 * @brief   This function gets FuSa CRC configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FuSa CRC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaCrcGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceFusaCrcConfig_t *pConfig
);

/****************************************************************************/
/**
 * @brief   This function gets FuSa BIST configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FuSa BIST configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaBistGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceFusaBistConfig_t *pConfig
);

/****************************************************************************/
/**
 * @brief   This function registers FuSa event callback.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   func                Callback function
 * @param   pUserContext        Pointer to user context
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaRegisterEventCb
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceEventFunc_t func,
	void *pUserContext
);

/****************************************************************************/
/**
 * @brief   This function resets FuSa.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaReset
(
	CamDeviceHandle_t hCamDevice
);


/****************************************************************************/
/**
 * @brief   This function gets the FuSa version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to FuSa version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFusaGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_fusa */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif
