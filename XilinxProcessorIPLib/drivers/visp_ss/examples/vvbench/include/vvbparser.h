// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2026 Vivantec Corporation
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
 ******************************************************************************/


#ifndef __VVBPARSER_H__
#define __VVBPARSER_H__
#include "vvdevice.h"

#include "cJSON.h"

#define VVCASE_EXEC_PASS 0
#define VVCASE_EXEC_FAIL -1
#define VVCASE_EXEC_PARSER_FAIL -2

VvbenchRunMode_t _VsiVvbenchRunMode
(
	const VvbenchRunMode_t *const pVvbenchRunMode
);

VvbenchRunMode_t VsiVvbenchGetVvbenchRunMode
(
);

VvbenchRunMode_t VsiVvbenchSetVvbenchRunMode
(
	const VvbenchRunMode_t vvbenchRunMode
);

char *VsiVvbenchLoadFileContent
(
	const char *file
);

int VsiVvbenchSwSimuSingleParseConfig
(
	const char *fileName,
	VvbenchCfg_t *cfgCtx
);

int VsiVvbenchParseConfig
(
	const char *file,
	VvbenchCfg_t *cfgCtx
);

int VsiVvbenchExecuteList
(
	const char *file,
	VvbenchCfg_t *cfgCtx
);

int VsiVvbenchExecuteClose
(
);

int VsiVvbenchParseCase
(
	const char *fileName,
	VvbenchVvdev_t *caseContext
);

CamDeviceBufMode_t VsiVvbenchStrToBuffMode
(
	const char *bufferString
);

CamDeviceBufChainId_t VsiVvbenchStrToBuffIo
(
	const char *bufferString
);

int VsiVvbenchStrToInputFormat
(
	const char *formatString
);

int VsiVvbenchStrToBuffFormat
(
	const char *formatString
);

int VsiVvbenchStrToInputType
(
	const char *inputType
);

int VsiVvbenchStrToPicLayout
(
	const char *patternString
);

int VsiVvbenchParseFusaInfo
(
	cJSON *inputInfo,
	VvbenchInstanceFusaCfg_t *const pFusaCfg
);

int VsiVvbenchParsePathSwitchInfo
(
	cJSON *pathSwitch,
	VvbenchInstancePathSwitchCfg_t *pathSwitchCfg
);

int VsiVvbenchParsePathSwitchInfo
(
	cJSON *pathSwitch,
	VvbenchInstancePathSwitchCfg_t *pathSwitchCfg
);

int VsiVvbenchParseTpgInfo
(
	cJSON *tpgInfo,
	VvbenchInstanceTpgCfg_t *tpgCtrl
);

int VsiVvbenchParseSensorInfo
(
	cJSON *sensorInfo,
	VvbenchInstanceSensorCfg_t *sensorCfg
);

int VsiVvbenchParsePicInfo
(
	cJSON *inputInfo,
	VvbenchInstancePictureCfg_t *pictureCfg
);

int VsiVvbenchParseModeInfo
(
	int workMode,
	cJSON *modeInfo,
	CamDeviceWorkConfig_t *modeCfg
);

int VsiVvbenchParseSwSimuDefaultInfo
(
	VvbenchVvdev_t *caseCtx
);

int VsiVvbenchParseSwSimuInfo
(
	VvbenchVvdev_t *caseCtx
);

int VsiVvbenchParseSwSimuInfoByToolOutput
(
	VvbenchVvdev_t *caseCtx,
	const char *rawName
);

int VsiVvbenchStrToRawMetaInfo
(
	const char *inputName,
	VvbenchRawMetaInfo_t *const pRawMetaInfo
);

int VsiVvbenchParseSubSystemInfo
(
	VvbenchVvdev_t *caseCtx,
	uint32_t instanceId
);

int VsiVvbenchParseSubmoduleInfo
(
	cJSON *instanceCfg,
	VvbenchVvdev_t *caseCtx,
	uint32_t instanceId
);

int VsiVvbenchParseDewarpInfo
(
	cJSON *dewarpInfo,
	VvbenchInstanceDewarpCfg_t *dewarpCtrl
);

int VsiVvbenchInitModuleConfig
(
	VvbenchVvdev_t *caseCtx,
	uint32_t instanceId
);

int VsiVvbenchStrSpliter
(
	char *inputName,
	const char *strDelim,
	char **strList
);

int VsiVvbenchStrToPicSize
(
	const char *inputSizeStr,
	uint32_t *const height,
	uint32_t *const width
);

int VsiVvbenchGetPathInfoFromJson
(
	cJSON *pJsonInstancePathCfg,
	VvbenchInstanceCfg_t *pInstanceCfgCtx
);

#endif //__VVBPARSER_H__
