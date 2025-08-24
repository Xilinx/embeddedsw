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


#ifndef CAMDEV_LOADCALI_API_H
#define CAMDEV_LOADCALI_API_H


#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/**
 * @brief   This function loads the calibration parameters into the database of
 *          software stack.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   defCalibIllum       Select the default illumination when pipeline is
 *                              connected.
 * @param   pCalibCfg           Configuration structure for calibration parameter.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceLoadCalibration
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCalibIllumType_t defCalibIllum,
	CamDeviceCalibCfg_t *pCalibCfg
);

#ifdef __cplusplus
}
#endif

#endif    //CAMDEV_LOADCALI_API_H
