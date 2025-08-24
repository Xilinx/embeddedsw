// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.

#include <cam_device_app.h>
#include "fw_common.h"

RESULT ControlSendResponse(void *data, MboxCoreId core_id);
RESULT ControlSendBackData(void *data, MboxCoreId core_id);


uint32_t CtrlFullBufferInform(void *data);
uint32_t CtrlCamDeviceFusaEventCb(void *data);
CamDeviceHandle_t CtrlGetCamDeviceHandle(uint32_t instanceId);
uint32_t CtrlGetCamDeviceInstanceId(CamDeviceHandle_t hCamDevice);
