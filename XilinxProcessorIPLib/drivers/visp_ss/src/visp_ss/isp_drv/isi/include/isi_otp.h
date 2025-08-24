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

#ifndef __ISI_OTP_H__
#define __ISI_OTP_H__
#include <types.h>
#include <isi_common.h>
#define ISI_OTP_LSC_TABLE_NUM 33
#define PDAF_OTP_FOCAL_SIZE 48

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum IsiColorTemperature_e {
	ISI_COLOR_TEMPERATURE_3100K = 0,
	ISI_COLOR_TEMPERATURE_4000K = 1,
	ISI_COLOR_TEMPERATURE_5800K = 2,
	ISI_COLOR_TEMPERATURE_MAX = 3,
	DUMMY_ISI_009 = 0xdeadfeed
} IsiColorTemperature_t;
typedef struct IsiOTPLSC_s {
	IsiColorTemperature_t colorTemperature;
	uint16_t r[ISI_OTP_LSC_TABLE_NUM][ISI_OTP_LSC_TABLE_NUM];
	uint16_t gr[ISI_OTP_LSC_TABLE_NUM][ISI_OTP_LSC_TABLE_NUM];
	uint16_t gb[ISI_OTP_LSC_TABLE_NUM][ISI_OTP_LSC_TABLE_NUM];
	uint16_t b[ISI_OTP_LSC_TABLE_NUM][ISI_OTP_LSC_TABLE_NUM];
} IsiOTPLSC_t;
typedef struct IsiOTPAWB_s {
	IsiColorTemperature_t colorTemperature;
	uint16_t r;
	uint16_t gr;
	uint16_t gb;
	uint16_t b;
	uint16_t rgRatio;
	uint16_t bgRatio;
} IsiOTPAWB_t;

typedef struct IsiOTPLightSource_s {
	IsiColorTemperature_t colorTemperature;
	uint16_t xCIE;
	uint16_t yCIE;
	uint16_t intensity;
} IsiOTPLightSource_t;

typedef struct IsiOTPCdaf_s {
	uint16_t minFocal;
	uint16_t maxFocal;
} IsiOTPCdaf_t;

typedef struct IsiOTPPdaf_s {
	int pdFocal[PDAF_OTP_FOCAL_SIZE];
} IsiOTPPdaf_t;
typedef struct IsiOTPAFData_s {
	uint32_t otpVersion;
	bool otpFocusEnable;
	IsiOTPCdaf_t cdafOtp;
	IsiOTPPdaf_t pdafOtp;
} IsiOTPAFData_t;

typedef struct IsiOTPModuleInformation_s {
	uint16_t hwVersion;
	uint16_t eepromRevision;
	uint16_t sensorRevision;
	uint16_t tlensRevision;
	uint16_t ircfRevision;
	uint16_t lensRevision;
	uint16_t caRevision;
	uint16_t moduleInteID;
	uint16_t factoryID;
	uint16_t mirrorFlip;
	uint16_t tlensSlaveID;
	uint16_t eepromSlaveID;
	uint16_t sensorSlaveID;
	uint8_t sensorID[11];
	uint16_t manuDateYear;
	uint16_t manuDateMonth;
	uint16_t manuDateDay;
	uint8_t barcodeModuleSN[12];
	uint16_t mapTotalSize;

} IsiOTPModuleInformation_t;

typedef struct IsiOTP_s {
	IsiOTPModuleInformation_t otpInformation;
	bool otpLSCEnable;
	bool otpAwbEnable;
	bool otpLightSourceEnable;
	bool otpFocusEnable;
	uint8_t lscNum;
	uint8_t awbNum;
	uint8_t goldenAwbNum;
	uint8_t lightSourceNum;
	IsiOTPLSC_t *pLscData;
	IsiOTPAWB_t *pAwbData;
	IsiOTPAWB_t *pGoldenAwbData;
	IsiOTPLightSource_t *pLightSourceData;
	IsiOTPAFData_t focus;
} IsiOTP_t;

#ifdef __cplusplus
}
#endif

#endif
