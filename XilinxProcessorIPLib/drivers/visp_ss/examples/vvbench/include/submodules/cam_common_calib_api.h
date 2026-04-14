/* Stub header - function implementations provided by libvisp.a */
#ifndef CAM_COMMON_CALIB_API_H
#define CAM_COMMON_CALIB_API_H

#include "cam_common_api.h"
#include <cam_device_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CamDeviceHandle_t;

RESULT VsiCamCommonLoadCalibration(CamCommonHandle_t *phCamCommon, CamDeviceHandle_t hCamDevice, int calibIllum);

#ifdef __cplusplus
}
#endif

#endif /* CAM_COMMON_CALIB_API_H */
