/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2022> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2022 */

/**
 * @file isi_priv.h
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
 * @defgroup isi_priv
 * @{
 *
 */
#ifndef ISI_PRIV_H
#define ISI_PRIV_H

#include <types.h>
#ifdef XPAR_I2C0_BASEADDR
#include <hal_i2c.h>
#endif
#include "isi.h"

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct IsiSensorContext_s {
	IsiSensor_t *pSensor;            /**< point to the sensor device */
#ifdef XPAR_I2C0_BASEADDR
	HalI2cHandle_t halI2cHandle;        /**< Handle of HAL I2C session to use. */
#endif
} IsiSensorContext_t;

#ifdef __cplusplus
}
#endif

#endif /* ISI_PRIV_H */
