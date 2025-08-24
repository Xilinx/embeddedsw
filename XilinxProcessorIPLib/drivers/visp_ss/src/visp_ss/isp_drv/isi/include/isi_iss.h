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

/* VeriSilicon 2022 */

/**
 * @file isi_iss.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup isi_iss CamerIc Driver API
 * @{
 *
 */
#ifndef __ISI_ISS_H__
#define __ISI_ISS_H__

#include <types.h>
#include <return_codes.h>

#include "isi.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef RESULT(*IsiCreateIss_t)(IsiSensorInstanceConfig_t *pConfig, IsiSensorHandle_t *pHandle);
typedef RESULT(*IsiOpenIss_t)(IsiSensorHandle_t handle, uint32_t mode);
typedef RESULT(*IsiCloseIss_t)(IsiSensorHandle_t handle);
typedef RESULT(*IsiReleaseIss_t)(IsiSensorHandle_t handle);
typedef RESULT(*IsiReadRegIss_t)(IsiSensorHandle_t handle, const uint16_t addr, uint16_t *pValue);
typedef RESULT(*IsiWriteRegIss_t)(IsiSensorHandle_t handle, const uint16_t addr,
				  const uint16_t value);
typedef RESULT(*IsiGetModeIss_t)(IsiSensorHandle_t handle, IsiSensorMode_t *pMode);
typedef RESULT(*IsiEnumModeIss_t)(IsiSensorHandle_t handle, IsiSensorEnumMode_t *pEnumMode);
typedef RESULT(*IsiGetCapsIss_t)(IsiSensorHandle_t handle, IsiCaps_t *pCaps);
typedef RESULT(*IsiCheckConnectionIss_t)(IsiSensorHandle_t handle);
typedef RESULT(*IsiGetRevisionIss_t)(IsiSensorHandle_t handle, uint32_t *pRevision);
typedef RESULT(*IsiSetStreamingIss_t)(IsiSensorHandle_t handle, bool_t on);

/* AEC */
typedef RESULT(*IsiGetAeBaseInfoIss_t)(IsiSensorHandle_t handle, IsiAeBaseInfo_t *pAeBaseInfo);
typedef RESULT(*IsiExcuteExpCtrlIss_t)(IsiSensorHandle_t handle,
				       const IsiSensorExpParam_t *pExpParam, IsiSensorExpParam_t *pExpResult);
typedef RESULT(*IsiExpDecomposeCtrlIss_t)(IsiSensorHandle_t handle,
		const IsiExpDecomposeParam_t *pDecParam, IsiExpDecomposeResult_t *pDecResult);
typedef RESULT(*IsiGetAGainIss_t)(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain);
typedef RESULT(*IsiSetAGainIss_t)(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain);
typedef RESULT(*IsiGetDGainIss_t)(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain);
typedef RESULT(*IsiSetDGainIss_t)(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain);
typedef RESULT(*IsiGetIntTimeIss_t)(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime);
typedef RESULT(*IsiSetIntTimeIss_t)(IsiSensorHandle_t handle,
				    const IsiSensorIntTime_t *pSensorIntTime);
typedef RESULT(*IsiGetFpsIss_t)(IsiSensorHandle_t handle, uint32_t *pFps);
typedef RESULT(*IsiSetFpsIss_t)(IsiSensorHandle_t handle, uint32_t fps);

/* SENSOR ISP */
typedef RESULT(*IsiGetIspStatusIss_t)(IsiSensorHandle_t handle, IsiIspStatus_t *pIspStatus);
typedef RESULT(*IsiSetBlcIss_t)(IsiSensorHandle_t handle, const IsiSensorBlc_t *pBlc);
typedef RESULT(*IsiGetBlcIss_t)(IsiSensorHandle_t handle, IsiSensorBlc_t *pBlc);
typedef RESULT(*IsiSetWBIss_t)(IsiSensorHandle_t handle, const IsiSensorWb_t *pWb);
typedef RESULT(*IsiGetWBIss_t)(IsiSensorHandle_t handle, IsiSensorWb_t *pWb);

/* SENSOE OTHER FUNC*/
typedef RESULT(*IsiSetTpgIss_t)(IsiSensorHandle_t handle, IsiSensorTpg_t tpg);
typedef RESULT(*IsiGetTpgIss_t)(IsiSensorHandle_t handle, IsiSensorTpg_t *pTpg);
typedef RESULT(*IsiGetExpandCurveIss_t)(IsiSensorHandle_t handle, IsiSensorCompandCurve_t *pCurve);
typedef RESULT(*IsiGetCompressCurveIss_t)(IsiSensorHandle_t handle,
		IsiSensorCompandCurve_t *pCurve);
typedef RESULT(*IsiExtendFuncIss_t)(IsiSensorHandle_t handle, void *pUserData);
typedef RESULT(*IsiGetOtpDataIss_t)(IsiSensorHandle_t handle, IsiOTP_t *pOtpData);

/* AF */
typedef RESULT(*IsiFocusCreateIss_t)(IsiSensorHandle_t handle);
typedef RESULT(*IsiFocusReleaseIss_t)(IsiSensorHandle_t handle);
typedef RESULT(*IsiFocusGetCalibrateIss_t)(IsiSensorHandle_t handle,
		IsiFocusCalibAttr_t *pFocusCalib);
typedef RESULT(*IsiFocusSetIss_t)(IsiSensorHandle_t handle, const IsiFocusPos_t *pPos);
typedef RESULT(*IsiFocusGetIss_t)(IsiSensorHandle_t handle, IsiFocusPos_t *pPos);

/* Infrared Light */
typedef RESULT(*IsiSetIRLightExpIss_t)(IsiSensorHandle_t handle,
				       const IsiIrLightExp_t *pIrExpParam);
typedef RESULT(*IsiGetIRLightExpIss_t)(IsiSensorHandle_t handle, IsiIrLightExp_t *pIrExpParam);


typedef RESULT(*IsiQueryMetadataAttrIss_t)(IsiSensorHandle_t handle, IsiMetadataAttr_t *pAttr);
typedef RESULT(*IsiSetMetadataAttrEnableIss_t)(IsiSensorHandle_t handle, IsiMetadataAttr_t attr);
typedef RESULT(*IsiGetMetadataAttrEnableIss_t)(IsiSensorHandle_t handle, IsiMetadataAttr_t *pAttr);
typedef RESULT(*IsiGetMetadataWindowIss_t)(IsiSensorHandle_t handle,
		IsiMetadataWinInfo_t *pMetaWin);
typedef RESULT(*IsiParserMetadataIss_t)
(
	IsiSensorHandle_t handle,
	const MetadataBufInfo_t *pMetaBuf,
	IsiSensorMetadata_t *pMetaInfo
);


/*****************************************************************************/
/**
 *          IsiSensor_t
 *
 * @brief
 *
 */
/*****************************************************************************/
struct IsiSensor_s {
	const char
	*pszName;                       /**< name of the camera-sensor */
	const IsiCaps_t
	*pIsiCaps;                      /**< pointer to sensor capabilities */

	IsiCreateIss_t pIsiCreateIss;                  /**< create a sensor handle */
	IsiOpenIss_t pIsiOpenIss;                    /**< open sensor */
	IsiCloseIss_t pIsiCloseIss;                   /**< close sensor */
	IsiReleaseIss_t pIsiReleaseIss;                 /**< release a sensor handle */
	IsiReadRegIss_t pIsiReadRegIss;                 /**< read sensor register */
	IsiWriteRegIss_t pIsiWriteRegIss;                /**< write sensor register */
	IsiGetModeIss_t pIsiGetModeIss;
	IsiEnumModeIss_t pIsiEnumModeIss;
	IsiGetCapsIss_t pIsiGetCapsIss;                 /**< get sensor capabilities */
	IsiCheckConnectionIss_t pIsiCheckConnectionIss;
	IsiGetRevisionIss_t
	pIsiGetRevisionIss;             /**< read sensor revision register (if available) */
	IsiSetStreamingIss_t
	pIsiSetStreamingIss;            /**< enable/disable streaming of data once sensor is configured */

	/* AEC */
	IsiGetAeBaseInfoIss_t pIsiGetAeBaseInfoIss;
	IsiExcuteExpCtrlIss_t pIsiExcuteExpCtrlIss;
	IsiExpDecomposeCtrlIss_t pIsiExpDecomposeCtrlIss;
	IsiGetAGainIss_t pIsiGetAGainIss;
	IsiSetAGainIss_t pIsiSetAGainIss;
	IsiGetDGainIss_t pIsiGetDGainIss;
	IsiSetDGainIss_t pIsiSetDGainIss;
	IsiGetIntTimeIss_t pIsiGetIntTimeIss;
	IsiSetIntTimeIss_t pIsiSetIntTimeIss;
	IsiGetFpsIss_t pIsiGetFpsIss;
	IsiSetFpsIss_t pIsiSetFpsIss;

	/* SENSOR ISP */
	IsiGetIspStatusIss_t pIsiGetIspStatusIss;
	IsiSetBlcIss_t pIsiSetBlcIss;
	IsiGetBlcIss_t pIsiGetBlcIss;
	IsiSetWBIss_t pIsiSetWBIss;
	IsiGetWBIss_t pIsiGetWBIss;

	/* SENSOE OTHER FUNC*/
	IsiSetTpgIss_t pIsiSetTpgIss;
	IsiGetTpgIss_t pIsiGetTpgIss;
	IsiGetExpandCurveIss_t pIsiGetExpandCurveIss;
	IsiGetCompressCurveIss_t pIsiGetCompressCurveIss;
	IsiExtendFuncIss_t pIsiExtendFuncIss;
	IsiGetOtpDataIss_t pIsiGetOtpDataIss;

	/* AF */
	IsiFocusCreateIss_t pIsiFocusCreateIss;
	IsiFocusReleaseIss_t pIsiFocusReleaseIss;
	IsiFocusGetCalibrateIss_t pIsiFocusGetCalibrateIss;
	IsiFocusSetIss_t pIsiFocusSetIss;
	IsiFocusGetIss_t pIsiFocusGetIss;

	/* Infrared Light */
	IsiSetIRLightExpIss_t pIsiSetIRLightExpIss;
	IsiGetIRLightExpIss_t pIsiGetIRLightExpIss;


	/* Metadata*/
	IsiQueryMetadataAttrIss_t pIsiQueryMetadataAttrIss;
	IsiSetMetadataAttrEnableIss_t pIsiSetMetadataAttrEnableIss;
	IsiGetMetadataAttrEnableIss_t pIsiGetMetadataAttrEnableIss;
	IsiGetMetadataWindowIss_t pIsiGetMetadataWinIss;
	IsiParserMetadataIss_t pIsiParserMetadataIss;
};

typedef RESULT(*IsiGetSensorIss_t)(IsiSensor_t *pIsiSensor);


/*****************************************************************************/
/**
 *          IsiCamDrvConfig_t
 *
 * @brief   Camera sensor driver specific data
 *
 */
/*****************************************************************************/
typedef struct IsiCamDrvConfig_s {
	uint32_t cameraDriverID;
	IsiGetSensorIss_t pIsiGetSensorIss;
	uint32_t cameraDevId;
	uint32_t instanceId;
} IsiCamDrvConfig_t;

typedef struct IsiCamDrvConfigMbox_s {
	uint32_t cameraDriverID;
	uint32_t pIsiGetSensorIss;
	uint32_t cameraDevId;
	uint32_t instanceId;
} IsiCamDrvConfigMbox_t;

/*****************************************************************************/
/**
 *          IsiSensorDrvHandleRegisterIss
 *
 * @brief   Sensor deiver handle register.
 *
 * @param   pCamDrvConfig     configuration of the isi camera drv
 * @param   pSensorHandle     produced sensor handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT IsiSensorDrvHandleRegisterIss
(
	IsiCamDrvConfig_t *pCamDrvConfig,
	IsiSensorHandle_t *pSensorHandle
);

/*****************************************************************************/
/**
 *          IsiSensorDrvHandleUnRegisterIss
 *
 * @brief   Sensor deiver handle register.
 *
 * @param   handle          isi sensor handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT IsiSensorDrvHandleUnRegisterIss
(
	IsiSensorHandle_t handle
);


#ifdef __cplusplus
}
#endif
#endif /* __ISI_ISS_H__ */
