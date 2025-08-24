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
 * @file isi_control.h
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
 * @defgroup isi Independent Sensor Interface
 * @{
 *
 */
#ifndef __ISI_CONTROL_H__
#define __ISI_CONTROL_H__

#include "cam_device_common.h"

#define CAMDEV_INSTANCE_MAX (CAMDEV_HARDWARE_ID_MAX*CAMDEV_VIRTUAL_ID_MAX)

typedef struct IsiSensorInst_s {
	IsiSensorHandle_t hIsiSensor[CAMDEV_INSTANCE_MAX];

} IsiSensorInst_t;

RESULT ControlIsiSensorDrvHandleRegisterIss(void *data);
RESULT ControlIsiSensorDrvHandleUnRegisterIss(void *data);
RESULT ControlIsiEnumModeIss(void *data);

RESULT ControlIsiOpenIss(void *data);
RESULT ControlIsiCloseIss(void *data);
RESULT ControlIsiCheckConnectionIss(void *data);
RESULT ControlIsiGetModeIss(void *data);
RESULT ControlIsiGetCapsIss(void *data);
RESULT ControlIsiSetStreamingIss(void *data);
RESULT ControlIsiGetRevisionIss(void *data);

RESULT ControlIsiGetAeBaseInfoIss(void *data);
RESULT ControlIsiGetAGainIss(void *data);
RESULT ControlIsiGetDGainIss(void *data);
RESULT ControlIsiSetAGainIss(void *data);
RESULT ControlIsiSetDGainIss(void *data);
RESULT ControlIsiGetIntTimeIss(void *data);
RESULT ControlIsiSetIntTimeIss(void *data);
RESULT ControlIsiGetFpsIss(void *data);
RESULT ControlIsiSetFpsIss(void *data);

RESULT ControlIsiGetIspStatusIss(void *data);
RESULT ControlIsiSetWBIss(void *data);
RESULT ControlIsiGetWBIss(void *data);
RESULT ControlIsiSetBlcIss(void *data);
RESULT ControlIsiGetBlcIss(void *data);

RESULT ControlIsiSetTpgIss(void *data);
RESULT ControlIsiGetTpgIss(void *data);
RESULT ControlIsiGetExpandCurveIss(void *data);


RESULT ControlIsiWriteRegIss(void *data);
RESULT ControlIsiReadRegIss(void *data);

#endif
