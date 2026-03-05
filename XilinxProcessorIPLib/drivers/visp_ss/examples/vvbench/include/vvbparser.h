/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets       *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

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
