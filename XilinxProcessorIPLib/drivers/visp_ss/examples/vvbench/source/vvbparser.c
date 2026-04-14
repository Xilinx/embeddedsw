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


#include "vvbench.h"
#include "vvbase.h"
#include "vvbparser.h"
/* memory_manager functions provided by libvisp.a */
extern void *mm_malloc(size_t size);
extern void mm_free(void *ptr);
#include "submodule_def.h"
#include "cam_device_api.h"
#include "cam_common_meta_data_api.h"
#include <stdio.h>
#include "cJSON.h"
#include <string.h>
/* t_database.h types provided by libvisp.a */

typedef uintptr_t TDatabaseHandle_t;


/* Forward declarations for types from libvisp.a */
typedef struct {
	bool_t calibrationLoaded;
	bool_t imageLoadMetaEn;
	CamDeviceSensorModeInfo_t sensorMode;
	CamDeviceInputType_t inputType;
	TDatabaseHandle_t hDatabase;                    // TDatabaseHandle_t (uintptr_t)
	struct {
		bool_t supported;
		bool_t enable;
		uint16_t sensorType;
	} hdr;
	struct {
		bool_t supported;
		bool_t enable;
		bool_t irRawOutEnable;
		uint16_t irBayerPattern;
	} rgbir;
	CamDeviceAwbWorkMode_t  modeAwb;
	CamDeviceAeWorkMode_t   modeAe;
} CamCommonContext_t;


typedef struct {
	bool enable;
	uint16_t id;
	struct {
		char bayerPattern[10];
		uint16_t bit;
		uint16_t crop[4];
		uint16_t frames;
		uint16_t height;
		char path[10][256];
		char metadataInfo[256];
		uint16_t startFrame;
		uint16_t width;
	} input;
	bool nrReloc;
	struct {
		uint16_t dumpYuvVideo;
		uint16_t height;
		char jpgFile[256];
		char path[256];
		char videoFile[256];
		uint16_t width;
		char yuvFile[256];
		char format[256];
		uint16_t dataBits;
		uint16_t yuvOrder;
		uint16_t alpha;
		uint16_t dumpStatistic;
	} output;
} TDatabaseMetaDriver_t;

typedef struct {
	char autoTable[256];
	bool awbEnable;
	bool cacEnable;
	bool ccEnable;
	bool enable;
	float integrationTime;
	bool lscEnable;
	uint16_t lscVersion;
	float sensorGain;
	char xml[256];
	bool metadataEnable;
	float metadataExposureGain[6];
	float metadataExposureTime[6];
	float metadataCCT[6];
	float metadataAwbGains[6][4];
	float maxExposureTime;
	char class_[20];                    // T_DATABASE_CLASS_NAME_LENGTH = 20
} TDatabase3AInterface_2_t;

/* Constants from t_database.h */
#define T_DATABASE_META_DRIVER "IOControl"
#define T_DATABASE_3A_INTERFACE_2 "3AI_2"

/* External functions from libvisp.a */
extern int TDatabase_query(TDatabaseHandle_t handle, const char *pCategory,void **ppData);
#define LOGTAG "PARSER"
#define DEFAULT_DATA_BITS 8
#define DEFAULT_ALPHA 0
#define DEFAULT_SWAP 0
#define DEFAULT_FPS 1
char *pFile = NULL;
cJSON *pRoot = NULL;
VvbenchVvdev_t *pVvdevCtx = NULL;
extern int ATM_ENABLE;
extern int UserSensorDevId;

#include <unistd.h>
#include <time.h>

VvbenchRunMode_t _VsiVvbenchRunMode(const VvbenchRunMode_t * const pVvbenchRunMode)
{
	static VvbenchRunMode_t _vvbenchRunMode = INVALID_MODE;
	static int unModifiedFlag = 0;//0 is unModified
	if (NULL == pVvbenchRunMode) {
		if (0 == unModifiedFlag) {
			return INVALID_MODE;
		}
		return _vvbenchRunMode;
	}
	else {
		if (0 == unModifiedFlag) {
			_vvbenchRunMode = *pVvbenchRunMode;
			++ unModifiedFlag;
			return _vvbenchRunMode;
		}
		return INVALID_MODE;
	}
}

VvbenchRunMode_t VsiVvbenchGetVvbenchRunMode()
{
	return _VsiVvbenchRunMode(NULL);
}

VvbenchRunMode_t VsiVvbenchSetVvbenchRunMode
(
	const VvbenchRunMode_t vvbenchRunMode
)
{
	return _VsiVvbenchRunMode(&vvbenchRunMode);
}

int VsiVvbenchGetPathInfoFromJson
(
	cJSON *pJsonInstancePathCfg,
	VvbenchInstanceCfg_t *pInstanceCfgCtx
)
{
	cJSON *pJsonPathEnable = cJSON_GetObjectItem(pJsonInstancePathCfg, "pathEnable");
	cJSON *pJsonPathName = cJSON_GetObjectItem(pJsonInstancePathCfg, "pathName");
	cJSON *pJsonBufferNumber = cJSON_GetObjectItem(pJsonInstancePathCfg, "bufferNumber");
	if (!(pJsonPathEnable && pJsonPathName && pJsonBufferNumber)) {
		LOGE("\"pathEnable\", \"pathName\", \"bufferNumber\" Must be include in the path json");
		return -1;
	}
	int pathEnable = pJsonPathEnable->valueint;
	char pathName[FILE_LEN];
	strcpy(pathName, pJsonPathName->valuestring);
	int bufferNumber = pJsonBufferNumber->valueint;

	if (0 == pathEnable) {
		LOGW("PATH %s closed", pathName);
	}
	CamDeviceBufChainId_t bufPath = VsiVvbenchStrToBuffIo(pathName);

	LOGI("Parse Path Cfg of hardware pipeline: %d, pathId: %d(%s)", pInstanceCfgCtx->hpId, bufPath,
	     pathName);
	pInstanceCfgCtx->instancePath[bufPath].pathEnable = pathEnable;
	pInstanceCfgCtx->instancePath[bufPath].path = bufPath;
	pInstanceCfgCtx->instancePath[bufPath].bufferNumber = bufferNumber;
	LOGI("path: %s, num: %d", pathName, pInstanceCfgCtx->instancePath[bufPath].bufferNumber);
	cJSON *pathWidth = cJSON_GetObjectItem(pJsonInstancePathCfg, "pathWidth");
	if (pathWidth) {
		pInstanceCfgCtx->instancePath[bufPath].width = pathWidth->valueint;
	}
	cJSON *pathHeight = cJSON_GetObjectItem(pJsonInstancePathCfg, "pathHeight");
	if (pathHeight) {
		pInstanceCfgCtx->instancePath[bufPath].height = pathHeight->valueint;
	}
	cJSON *pathOutType = cJSON_GetObjectItem(pJsonInstancePathCfg, "pathOutType");
	if (pathOutType) {
		pInstanceCfgCtx->instancePath[bufPath].pathOutType = pathOutType->valueint;
	}
	else {
		pInstanceCfgCtx->instancePath[bufPath].pathOutType = 0;
	}

	if (bufPath >= CAMDEV_BUFCHAIN_RDMA) {
		pInstanceCfgCtx->instancePath[bufPath].layout = pInstanceCfgCtx->pictureCfg.layout;
		pInstanceCfgCtx->instancePath[bufPath].format = pInstanceCfgCtx->pictureCfg.format;
		cJSON *format = cJSON_GetObjectItem(pJsonInstancePathCfg, "format");
		if (format) {
			char formatName[FILE_LEN];
			strcpy(formatName, format->valuestring);
			int bufferFormat = VsiVvbenchStrToInputFormat(formatName);
			if (CAMDEV_INPUT_FMT_MAX <= bufferFormat) {
				LOGW("case list VsiVvbenchStrToInputFormat invalid format error");
			}
			LOGI("path: %s, format(%s): %d", pathName, formatName, bufferFormat);
			pInstanceCfgCtx->instancePath[bufPath].format = bufferFormat;
		}
	}
	else {
		cJSON *format = cJSON_GetObjectItem(pJsonInstancePathCfg, "format");
		if (format) {
			char formatName[FILE_LEN];
			strcpy(formatName, format->valuestring);
			int bufferFormat = VsiVvbenchStrToBuffFormat(formatName);
			if (CAMDEV_PIX_FMT_MAX <= bufferFormat) {
				LOGW("case list VsiVvbenchStrToBuffFormat invalid format error");
			}
			LOGI("path: %s, format(%s): %d", pathName, formatName, bufferFormat);
			pInstanceCfgCtx->instancePath[bufPath].format = bufferFormat;
		}
	}
	cJSON *yuvOrder = cJSON_GetObjectItem(pJsonInstancePathCfg, "yuvOrder");
	if (yuvOrder) {
		LOGI("path: %s, yuvOrder: %d", pathName, yuvOrder->valueint);
		pInstanceCfgCtx->instancePath[bufPath].yuvOrder = yuvOrder->valueint;
	}
	else {
		pInstanceCfgCtx->instancePath[bufPath].yuvOrder = 0;
		LOGI("path: %s, yuvOrder: 0", pathName);
	}
	cJSON *dataBits = cJSON_GetObjectItem(pJsonInstancePathCfg, "dataBits");
	if (dataBits) {
		LOGI("path: %s, dataBits: %d", pathName, dataBits->valueint);
		pInstanceCfgCtx->instancePath[bufPath].dataBits = dataBits->valueint;
	}
	else {
		pInstanceCfgCtx->instancePath[bufPath].dataBits = DEFAULT_DATA_BITS;
		LOGI("path: %s, dataBits: %d", pathName, DEFAULT_DATA_BITS);
	}

	cJSON *alpha = cJSON_GetObjectItem(pJsonInstancePathCfg, "alpha");
	if (alpha) {
		LOGI("path: %s, alpha: %d", pathName, alpha->valueint);
		pInstanceCfgCtx->instancePath[bufPath].alpha = alpha->valueint;
	}
	else {
		pInstanceCfgCtx->instancePath[bufPath].alpha = DEFAULT_ALPHA;
		LOGI("path: %s, set default alpha: %d", pathName, DEFAULT_ALPHA);
	}

	cJSON *swap = cJSON_GetObjectItem(pJsonInstancePathCfg, "swap");
	if (swap) {
		cJSON *swapRaw = cJSON_GetObjectItem(swap, "swapRaw");
		if (swapRaw) {
			pInstanceCfgCtx->instancePath[bufPath].swap.rawSwap = swapRaw->valueint;
			LOGI("path: %s, swap Test rawSwap is: %d", pathName,
			     pInstanceCfgCtx->instancePath[bufPath].swap.rawSwap);
		}
		else {
			cJSON *swapY = cJSON_GetObjectItem(swap, "swapY");
			cJSON *swapU = cJSON_GetObjectItem(swap, "swapU");
			cJSON *swapV = cJSON_GetObjectItem(swap, "swapV");
			pInstanceCfgCtx->instancePath[bufPath].swap.yuvSwap.y = swapY ? swapY->valueint : DEFAULT_SWAP;
			pInstanceCfgCtx->instancePath[bufPath].swap.yuvSwap.u = swapU ? swapU->valueint : DEFAULT_SWAP;
			pInstanceCfgCtx->instancePath[bufPath].swap.yuvSwap.v = swapV ? swapV->valueint : DEFAULT_SWAP;
			LOGI("path: %s, swap config swapY is: %d", pathName,
			     pInstanceCfgCtx->instancePath[bufPath].swap.yuvSwap.y);
			LOGI("path: %s, swap config swapu is: %d", pathName,
			     pInstanceCfgCtx->instancePath[bufPath].swap.yuvSwap.u);
			LOGI("path: %s, swap config swapv is: %d", pathName,
			     pInstanceCfgCtx->instancePath[bufPath].swap.yuvSwap.v);
		}
	}
	else {
		pInstanceCfgCtx->instancePath[bufPath].swap = (CamDeviceMiSwap_u) {DEFAULT_SWAP};
		LOGI("path: %s, set default swap: %d", pathName, DEFAULT_SWAP);
	}
	cJSON *stitchMode = cJSON_GetObjectItem(pJsonInstancePathCfg, "stitchMode");
	if (stitchMode) {
		LOGI("path: %s, stitchMode: %d", pathName, stitchMode->valueint);
		pInstanceCfgCtx->instancePath[bufPath].stitchMode = stitchMode->valueint;
	}

	cJSON *cropWindow = cJSON_GetObjectItem(pJsonInstancePathCfg, "cropWindow");
	if (cropWindow) {
		cJSON *horizontalOffset = cJSON_GetObjectItem(cropWindow, "horizontalOffset");
		cJSON *verticalOffset = cJSON_GetObjectItem(cropWindow, "verticalOffset");
		cJSON *cropWidth = cJSON_GetObjectItem(cropWindow, "cropWidth");
		cJSON *cropHeight = cJSON_GetObjectItem(cropWindow, "cropHeight");
		if (!(horizontalOffset && verticalOffset && cropWidth && cropHeight)) {
			LOGE("horizontalOffset, verticalOffset, cropWidth, cropHeight cannot miss when use cropWindow");
			return -1;
		}
		pInstanceCfgCtx->instancePath[bufPath].cropCfg.horizontalOffset = horizontalOffset->valueint;
		pInstanceCfgCtx->instancePath[bufPath].cropCfg.verticalOffset = verticalOffset->valueint;
		pInstanceCfgCtx->instancePath[bufPath].cropCfg.cropWidth = cropWidth->valueint;
		pInstanceCfgCtx->instancePath[bufPath].cropCfg.cropHeight = cropHeight->valueint;
	}
	return 0;
}
int VsiVvbenchSwSimuSingleParseConfig
(
	const char *fileName,
	VvbenchCfg_t *cfgCtx
)
{
	int result = 0;
	char caseName[FILE_LEN] = {0};
	cJSON *item = NULL;
	VvbenchVvdev_t *vvdevCtx = mm_malloc(sizeof(VvbenchVvdev_t));
	MEMSET(vvdevCtx, 0, sizeof(VvbenchVvdev_t));

	LOGI("%s enter \n", __func__);

	if ((NULL == cfgCtx) || (NULL == fileName)) {
		LOGE("null inputs");
		return -1;
	}

	LOGI("Input file name: %s", fileName);

	char *file = VsiVvbenchLoadFileContent(fileName);
	if (file == NULL) {
		LOGE("%s Load file content failed, err: %s", __func__, fileName);
		result = -1;
	}
	pFile = file;
	pVvdevCtx = vvdevCtx;

	cJSON *root = cJSON_Parse(file);
	if (NULL == root) {
		LOGE("%s parser failed, err: %s", __func__, fileName);
		result = -1;
	}
	else {
		pRoot = root;
		cJSON *swSimuCfg = cJSON_GetObjectItem(root, "swSimuCfg");
		if (NULL == swSimuCfg) {
			LOGE("%s parser failed", __func__);
			result = -1;
		}
		else {
			item = cJSON_GetObjectItem(swSimuCfg, "enable");
			if (item) {
				vvdevCtx->swSimuCfg.enable = item->valueint;
				LOGI("swSimuEnable: %d", vvdevCtx->swSimuCfg.enable);
			}
			item = cJSON_GetObjectItem(swSimuCfg, "caseName");
			if (item) {
				strcpy(caseName, item->valuestring);
				LOGI("caseName: %s", caseName);
			}
		}

		cJSON *alignMask = cJSON_GetObjectItem(root, "alignMask");
		if (NULL == alignMask) {
			LOGE("%s parser failed", __func__);
			result = -1;
		}
		else {
			cfgCtx->alignMask = alignMask->valueint;
			LOGI("alignMask is 0x%x", cfgCtx->alignMask);
		}

		LOGI("swSimuCfgFile: %s", cfgCtx->swSimuName);
		snprintf(vvdevCtx->swSimuCfg.swSimuCfgFile, FILE_LEN, "vvbcfg/load_image/Cfg/%s",
			 cfgCtx->swSimuName);
#if defined(ISP_OFFLINE_TEST) || defined(HAL_CMODEL) || defined(DUMP_IMAGE)
		strncpy(vvdevCtx->caseName, cfgCtx->swSimuName, sizeof(vvdevCtx->caseName));
#endif
		result = VsiVvbenchParseCase(caseName, vvdevCtx);
		if (0 != result) {
			LOGE("VsiVvbenchParseCase error, simu cfg name: %s,  exit", caseName);
			result = VVCASE_EXEC_PARSER_FAIL;
		}

		result = VsiVvbenchParseSwSimuDefaultInfo(vvdevCtx);
		if (0 != result) {
			LOGE("VsiVvbenchParseSwSimuDefaultInfo error, simu cfg name: %s,  exit",
			     vvdevCtx->swSimuCfg.defaultJson);
			result = VVCASE_EXEC_PARSER_FAIL;
		}
		if (LOAD_IMAGE_CASE_BY_TOOL == VsiVvbenchGetVvbenchRunMode()) {
			vvdevCtx->fineTuneMode = true;
			strncpy(vvdevCtx->swSimuCfg.fineTuneJson, vvdevCtx->swSimuCfg.swSimuCfgFile,
				sizeof(vvdevCtx->swSimuCfg.fineTuneJson));
			result = VsiVvbenchParseSwSimuInfoByToolOutput(vvdevCtx, cfgCtx->rawName);
			if (0 != result) {
				LOGE("VsiVvbenchParseSwSimuInfoByToolOutput error, simu cfg name: %s,  exit",
				     vvdevCtx->swSimuCfg.fineTuneJson);
				result = VVCASE_EXEC_PARSER_FAIL;
			}
		}
		else {
			result = VsiVvbenchParseSwSimuInfo(vvdevCtx);
			if (0 != result) {
				LOGE("VsiVvbenchParseSwSimuInfo error, simu cfg name: %s,  exit",
				     vvdevCtx->swSimuCfg.swSimuCfgFile);
				result = VVCASE_EXEC_PARSER_FAIL;
			}
		}
		if (result == 0) {
			result = VsiVvdeviceExecuteCaseline(vvdevCtx, cfgCtx);
			if (0 != result) {
				LOGE("VsiVvdeviceExecuteCaseline error, simu cfg name: %s,  exit", caseName);
				result = VVCASE_EXEC_PARSER_FAIL;
			}
		}
		MEMSET(vvdevCtx, 0, sizeof(VvbenchVvdev_t));
		LOGI("%s Case End", __func__);
	}

	mm_free(file);
	file = NULL;
	mm_free(vvdevCtx);
	vvdevCtx = NULL;
	cJSON_Delete(root);

	LOGI("%s exit \n", __func__);

	return result;
}


int VsiVvbenchParseConfig
(
	const char *fileName,
	VvbenchCfg_t *cfgCtx
)
{
	int result = 0;
	int length = 0;
	int swSimuEnable = 0;
	int index;
	char caseName[FILE_LEN] = {0};
	cJSON *item = NULL;
	VvbenchVvdev_t *vvdevCtx = mm_malloc(sizeof(VvbenchVvdev_t));
	MEMSET(vvdevCtx, 0, sizeof(VvbenchVvdev_t));

	LOGI("%s enter \n", __func__);

	if ((NULL == cfgCtx) || (NULL == fileName)) {
		LOGE("null inputs");
		return -1;
	}

	LOGI("Input file name: %s", fileName);

	char *file = VsiVvbenchLoadFileContent(fileName);
	if (file == NULL) {
		LOGE("%s Load file content failed, err: %s", __func__, fileName);
		result = -1;
	}
	pFile = file;
	pVvdevCtx = vvdevCtx;

	cJSON *root = cJSON_Parse(file);
	if (NULL == root) {
		LOGE("%s parser failed, err: %s", __func__, fileName);
		result = -1;
	}
	else {
		cJSON *swSimuCfg = cJSON_GetObjectItem(root, "swSimuCfg");
		if (NULL == swSimuCfg) {
			LOGE("%s parser failed", __func__);
			result = -1;
		}
		else {
			item = cJSON_GetObjectItem(swSimuCfg, "enable");
			if (item) {
				swSimuEnable = item->valueint;
				LOGI("swSimuEnable: %d", swSimuEnable);
			}
			item = cJSON_GetObjectItem(swSimuCfg, "swSimuCaselist");
			if (item) {
				strcpy(vvdevCtx->swSimuCfg.swSimuCaselist, item->valuestring);
				LOGI("cfgFile: %s", vvdevCtx->swSimuCfg.swSimuCaselist);
			}
			item = cJSON_GetObjectItem(swSimuCfg, "caseName");
			if (item) {
				strcpy(caseName, item->valuestring);
				LOGI("caseName: %s", caseName);
			}
		}

		cJSON *alignMask = cJSON_GetObjectItem(root, "alignMask");
		if (NULL == alignMask) {
			LOGE("%s parser failed", __func__);
			result = -1;
		}
		else {
			cfgCtx->alignMask = alignMask->valueint;
			LOGI("alignMask is 0x%x", cfgCtx->alignMask);
		}

		vvdevCtx->swSimuCfg.enable = swSimuEnable;

		osFile *swSimuList = NULL;
		char swSimuName[FILE_NAME] = {0};
		swSimuList = osFopen(vvdevCtx->swSimuCfg.swSimuCaselist, "r");
		if (NULL == swSimuList) {
			LOGE("VsiVvbenchParseCase error, open swSimuCaselist fail: %s,  exit",
			     vvdevCtx->swSimuCfg.swSimuCaselist);
			return -1;
		}

		//count the number of rows
		while (osFgets(swSimuName, sizeof(swSimuName) - 1, swSimuList) != NULL) {
			if (swSimuName[0] != '\n') {
				length++;
			}
		}
		osFclose(swSimuList);
		swSimuList = NULL;

		long int sw_simu_pos = 0;


		for (index = 0; index < length; index++) {
			result = VsiVvbenchParseCase(caseName, vvdevCtx);
			if (0 != result) {
				LOGE("VsiVvbenchParseCase error, simu cfg name: %s,  exit", caseName);
				result = VVCASE_EXEC_PARSER_FAIL;
			}
			swSimuList = osFopen(vvdevCtx->swSimuCfg.swSimuCaselist, "r");
			if (NULL == swSimuList) {
				LOGE("VsiVvbenchParseCase error, open swSimuCaselist fail: %s,  exit",
				     vvdevCtx->swSimuCfg.swSimuCaselist);
				return -1;
			}
			osFseek(swSimuList, sw_simu_pos, SEEK_SET);
			if (osFgets(swSimuName, sizeof(swSimuName) - 1, swSimuList) != NULL) {
				char *comment = strchr(swSimuName, '#');
				if (comment) {
					continue;
				}
				char *find = strrchr(swSimuName, '\n');
				if (find) {
					*find = '\0';
				}
				snprintf(vvdevCtx->swSimuCfg.swSimuCfgFile, FILE_LEN, "vvbcfg/load_image/Cfg/%s", swSimuName);
				LOGI("swSimuCfgFile %d: %s", index, vvdevCtx->swSimuCfg.swSimuCfgFile);


#if defined(ISP_OFFLINE_TEST) || defined(HAL_CMODEL) || defined(DUMP_IMAGE)
				strncpy(vvdevCtx->caseName, swSimuName, sizeof(vvdevCtx->caseName));
#endif
				result = VsiVvbenchParseSwSimuDefaultInfo(vvdevCtx);
				if (0 != result) {
					LOGE("VsiVvbenchParseSwSimuDefaultInfo error, simu cfg name: %s,  exit",
					     vvdevCtx->swSimuCfg.defaultJson);
					result = VVCASE_EXEC_PARSER_FAIL;
				}

				result = VsiVvbenchParseSwSimuInfo(vvdevCtx);
				if (0 != result) {
					LOGE("VsiVvbenchParseSwSimuInfo error, simu cfg name: %s,  exit",
					     vvdevCtx->swSimuCfg.swSimuCfgFile);
					result = VVCASE_EXEC_PARSER_FAIL;
				}

				if (result == 0) {
					result = VsiVvdeviceExecuteCaseline(vvdevCtx, cfgCtx);
					if (0 != result) {
						LOGE("VsiVvdeviceExecuteCaseline error, simu cfg name: %s,  exit", caseName);
						result = VVCASE_EXEC_PARSER_FAIL;
					}
				}
				MEMSET(vvdevCtx->swSimuCfg.swSimuCfgFile, 0, sizeof(vvdevCtx->swSimuCfg.swSimuCfgFile));
				MEMSET(swSimuName, 0, sizeof(swSimuName));


				sw_simu_pos = osFtell(swSimuList);
				osFclose(swSimuList);
				swSimuList = NULL;
			}
		}
		// osFclose(swSimuList);
		// swSimuList = NULL;
		LOGI("%s Case End", __func__);
	}
	mm_free(file);
	file = NULL;
	mm_free(vvdevCtx);
	vvdevCtx = NULL;
	cJSON_Delete(root);

	LOGI("%s exit \n", __func__);

	return result;
}

int VsiVvbenchExecuteList
(
	const char *fileName,
	VvbenchCfg_t *cfgCtx
)
{
	int executeListResult = 0;
	int testLoop = 0;
	int testInterval = 0;
	cJSON *item = NULL;
	LOGI("%s enter \n", __func__);

	if ((NULL == cfgCtx) || (NULL == fileName)) {
		LOGE("null inputs");
		return -1;
	}
	LOGI("Input case list name: %s", fileName);

	char *file = VsiVvbenchLoadFileContent(fileName);
	if (file == NULL) {
		LOGE("%s Load file content failed, err: %s", __func__, fileName);
		executeListResult = -1;
	}
	pFile = file;

	cJSON *listRoot = cJSON_Parse(file);
	if (listRoot) {
		pRoot = listRoot;
		cJSON *caseCfg = cJSON_GetObjectItem(listRoot, "config");
		if (caseCfg) {
			item = cJSON_GetObjectItem(caseCfg, "caseLoop");
			if (item) {
				testLoop = item->valueint;
				LOGI("testLoop is %d", testLoop);
			}
			item = cJSON_GetObjectItem(caseCfg, "caseInterval");
			if (item) {
				testInterval = item->valueint;
				LOGI("caseInterval is %d", testInterval);
			}
			item = cJSON_GetObjectItem(caseCfg, "alignMask");
			if (item) {
				cfgCtx->alignMask = item->valueint;
				LOGI("alignMask is %d", cfgCtx->alignMask);
			}
			item = cJSON_GetObjectItem(caseCfg, "ATMenable");
			if (item && (item->valueint == 1)) {
				ATM_ENABLE = item->valueint;
				LOGI("ATM_ENABLED\n");
			} else {
				//ATM_ENABLE = 1;
				LOGI("ATM_ENABLED\n");
			}
		}
		cJSON *caseLists = cJSON_GetObjectItem(listRoot, "caseLists");
		if (cJSON_Array != caseLists->type) {
			LOGW("case list parser error");
			cJSON_Delete(listRoot);
			return -1;
		}

		int failCaseCnt = 0;
		int passCaseCnt = 0;
		int totalCnt = 0;
		int fullPassCnt = 0;
		int nonFullPassCnt = 0;
		//Create case report object
		cJSON *caseReport = cJSON_CreateObject();
		cJSON *failResult = cJSON_CreateObject();
		cJSON *caseResult = cJSON_CreateArray();

		char reportNameStr[128];
		int count = 0;
		// time_t timeBegin;
		// time (&timeBegin);
		// LOGI("Begin Time: %s",asctime(gmtime(&timeBegin)));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Summary Title", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateString("VVbench test report"));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: VVbench Version", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateString(VVBENCH_VERSION));
		// snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Test begin time", count++);
		// cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateString(asctime(gmtime(&timeBegin))));

		for (int i = 0; i < cJSON_GetArraySize(caseLists); ++i) {
			char caseName[FILE_LEN];
			strcpy(caseName, cJSON_GetArrayItem(cJSON_GetObjectItem(listRoot, "caseLists"), i)->valuestring);
			LOGI("case %d: %s", i, caseName);

			int failedFlag = 0;
			int result = 0;
			cJSON *subCaseResult = cJSON_CreateObject();
			for (int loopIdx = 0; loopIdx < testLoop; ++loopIdx) {
				LOGI("Round %d", loopIdx);
				LOGI("calling cases");
				//run cases
				VvbenchVvdev_t *vvdevCtx = mm_malloc(sizeof(VvbenchVvdev_t));
				MEMSET(vvdevCtx, 0, sizeof(VvbenchVvdev_t));
				result = VsiVvbenchParseCase(caseName, vvdevCtx);
				if (0 != result) {
					LOGE("VsiVvbenchParseCase error, case name: %s,  exit", caseName);
					result = VVCASE_EXEC_PARSER_FAIL;
				}
				else {
					result = VsiVvdeviceExecuteCaseline(vvdevCtx, cfgCtx);
				}
				if (VVCASE_EXEC_PASS == result) {
					cJSON_AddItemToObject(subCaseResult, caseName, cJSON_CreateString("PASS"));
					passCaseCnt++;
					LOGI("case %s Pass, pass cnt: %d", caseName, passCaseCnt);
				}
				else if (VVCASE_EXEC_FAIL == result) {
					cJSON_AddItemToObject(subCaseResult, caseName, cJSON_CreateString("FAILED"));
					failedFlag = 1;
					failCaseCnt++;
					LOGW("case %s Failed, fail cnt: %d", caseName, failCaseCnt);
				}
				else if (VVCASE_EXEC_PARSER_FAIL == result) {
					cJSON_AddItemToObject(subCaseResult, caseName, cJSON_CreateString("PARSER FAIL"));
					failedFlag = 1;
					failCaseCnt++;
					LOGW("case %s Parser Failed, fail cnt: %d", caseName, failCaseCnt);
				}
				else {
					cJSON_AddItemToObject(subCaseResult, caseName, cJSON_CreateString("UNDEFINED FAIL"));
					failedFlag = 1;
					failCaseCnt++;
					LOGW("case %s Parser Failed, fail cnt: %d", caseName, failCaseCnt);
				}
				mm_free(vvdevCtx);
				vvdevCtx = NULL;
			}
			LOGI("calling delay: %d", testInterval);
			sleep(testInterval);
			totalCnt++;
			if (failedFlag) {
				cJSON_AddItemToObject(failResult, caseName, cJSON_CreateString("FAIL"));
				nonFullPassCnt++;
			}
			else {
				fullPassCnt++;
			}
			cJSON_AddItemToArray(caseResult, subCaseResult);
		}
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Result", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, caseResult);
		if (0 == failCaseCnt) {
			snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Failed Cases List", count++);
			cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateString("NONE"));
			cJSON_Delete(failResult);
		}
		else {
			snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Failed Cases List", count++);
			cJSON_AddItemToObject(caseReport, reportNameStr, failResult);
		}
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Total Cases Coverd", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateNumber(totalCnt));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Loop Time for Each", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateNumber(testLoop));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Full Pass Cases Num", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateNumber(fullPassCnt));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Failed Cases Num", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateNumber(nonFullPassCnt));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Total Cases Passed Time(s)", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateNumber(passCaseCnt));
		snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Total Cases Failed Time(s)", count++);
		cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateNumber(failCaseCnt));
		// time_t timeEnd;
		// time (&timeEnd);
		// LOGI("End Time: %s",asctime(gmtime(&timeEnd)));
		// snprintf(reportNameStr, sizeof(reportNameStr), "%2d: Test end time", count++);
		// cJSON_AddItemToObject(caseReport, reportNameStr, cJSON_CreateString(asctime(gmtime(&timeEnd))));
		//output test summary
		char *data = cJSON_Print(caseReport);
		if (NULL == data) {
			LOGE("caseReport cJSON_Print failed!\n");
			mm_free(file);
			file = NULL;
			cJSON_Delete(listRoot);
			return -1;
		}
#if 0
		char reportFileName[FILE_LEN];
		// struct tm *endTime = gmtime(&timeEnd);
		// snprintf(reportFileName, FILE_LEN, "vvb_report_%d_%d_%d_%d_%d.json", endTime->tm_mon + 1, endTime->tm_mday, endTime->tm_hour, endTime->tm_min, endTime->tm_sec);
		LOGI("Output name: %s", reportFileName);
		osFile *fp = osFopen(reportFileName, "w");
		osFwrite(data, sizeof(char), strlen(data) +1, fp);
		osFclose(fp);
		cJSON_free(data);
		data = NULL;
		cJSON_Delete(caseReport);
		LOGI("%s Case End", __func__);
#endif
	}
	else {
		LOGE("list json parse error\r\n");
		executeListResult = -1;
	}
	mm_free(file);
	file = NULL;
	cJSON_Delete(listRoot);

	LOGI("%s exit \n", __func__);
	return executeListResult;
}

int VsiVvbenchExecuteClose()
{
	int32_t result = 0;
	LOGI("%s enter \n", __func__);

	result = VsiVvdeviceStop(true);
	if (0 != result) {
		LOGE("VsiVvdeviceStop error");
		return -1;
	}

	if (pFile != NULL) {
		mm_free(pFile);
		pFile = NULL;
	}

	if (pVvdevCtx != NULL) {
		mm_free(pVvdevCtx);
		pVvdevCtx = NULL;
	}

	if (pRoot != NULL) {
		cJSON_Delete(pRoot);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseCase
(
	const char *fileName,
	VvbenchVvdev_t *caseCtx
)
{
	int result = 0;
	char strBuf[FILE_LEN];
	cJSON *item = NULL;
	LOGI("%s enter \n", __func__);

	LOGI("Input case list name: %s", fileName);
	if ((NULL == caseCtx) || (NULL == fileName)) {
		LOGE("null inputs");
		return -1;
	}
	snprintf(strBuf, FILE_LEN, "%s", fileName);
	LOGI("Full case list name: %s", strBuf);

	char *file = VsiVvbenchLoadFileContent(strBuf);
	if (file == NULL) {
		LOGE("%s Load file content failed, err: %s", __func__, fileName);
		result = -1;
	}
	cJSON *root = cJSON_Parse(file);
	mm_free(file);
	file = NULL;
	if (NULL == root) {
		LOGE("%s parser failed, err: %s", __func__, fileName);
		result = -1;
	}
	else {
		item = cJSON_GetObjectItem(root, "totalInstance");
		if (item) {
			caseCtx->totalInstance = item->valueint;
			if (caseCtx->totalInstance > MAX_CAM_NUM) {
				LOGE("%s: parameter error: totalInstance", __func__);
				return -1;
			}
		}
		cJSON *useSubSystem = cJSON_GetObjectItem(root, "useSubSystem");
		if (useSubSystem) {
			caseCtx->useSubSystem = useSubSystem->valueint;
			LOGI("%s: useSubSystem:%d", __func__, caseCtx->useSubSystem);
		}
		cJSON *instanceCfgArray = cJSON_GetObjectItem(root, "instanceCfg");
		if (cJSON_Array != instanceCfgArray->type) {
			LOGW("case %s instance config parse error", fileName);
			cJSON_Delete(root);
			return -1;
		}
		for (int i = 0; i < MAX_CAM_NUM; ++i) {
			caseCtx->instanceCfgCtx[i].instanceEnable = 0;

#if defined(ISP_OFFLINE_TEST) || defined(HAL_CMODEL) || defined(DUMP_IMAGE)
			if (!caseCtx->swSimuCfg.enable) {
				strncpy(caseCtx->caseName, fileName, FILE_LEN - 1);
				uint8_t index = strlen(fileName) -1;
				while ((caseCtx->caseName[index] != '.') && (index > 0)) {
					index--;
				}
				caseCtx->caseName[index] = '\0';
			}
#endif
		}
		cJSON *instanceCfg = instanceCfgArray->child;
		uint32_t instanceId = 0;
		while (instanceCfg) {
			uint32_t instanceEnable = cJSON_GetObjectItem(instanceCfg, "instanceEnable")->valueint;
			uint32_t hpId = cJSON_GetObjectItem(instanceCfg, "hpId")->valueint;
			if (CAMDEV_HARDWARE_ID_MAX < hpId) {
				LOGW("case list hpId parser error");
				cJSON_Delete(root);
				return -1;
			}

			if (MAX_CAM_NUM < instanceId) {
				LOGE("instanceId error");
				cJSON_Delete(root);
				return -1;
			}
			caseCtx->instanceCfgCtx[instanceId].instanceEnable = instanceEnable;
			caseCtx->instanceCfgCtx[instanceId].hpId = hpId; // NEXT BUILD 2026.1
			LOGI("Parse hardware pipeline %d", caseCtx->instanceCfgCtx[instanceId].hpId);
			if (instanceEnable == 0) {
				LOGW("Skip instance:%d", instanceId);
				instanceCfg = instanceCfg->next;
				instanceId++;
				continue;
			}
			int inputType = VsiVvbenchStrToInputType(cJSON_GetObjectItem(instanceCfg,
					"inputType")->valuestring);
			if (CAMDEV_INPUT_TYPE_INVALID == inputType) {
				LOGE("Parse inputType info  error");
			}
			caseCtx->instanceCfgCtx[instanceId].inputType = (CamDeviceInputType_t)inputType;
			LOGI("input type parse: %d", caseCtx->instanceCfgCtx[instanceId].inputType);
			if (CAMDEV_INPUT_TYPE_IMAGE == inputType) {
				cJSON *inputInfo = cJSON_GetObjectItem(instanceCfg, "inputInfo");
				if (NULL != inputInfo) {
					result = VsiVvbenchParsePicInfo(inputInfo, &(caseCtx->instanceCfgCtx[instanceId].pictureCfg));
					if (0 != result) {
						LOGE("VsiVvbenchParsePicInfo failed");
					}
					if (caseCtx->instanceCfgCtx[instanceId].pictureCfg.fileName[0] != '\0') {
						caseCtx->swSimuCfg.enable = true;
						caseCtx->swSimuCfg.autoCfg.enable = true;
					}
				}
			}
			else {
				//todo: other input type parse
			}
			//init CamCommon
			result = VsiCamCommonCreate(&caseCtx->instanceCfgCtx[instanceId].hCamCommon,
						    caseCtx->instanceCfgCtx[instanceId].inputType);
			if (0 != result) {
				LOGE("VsiCamCommonCreate error,  exit");
				result = -1;
			}
			char bufType[FILE_LEN];
			strcpy(bufType, cJSON_GetObjectItem(instanceCfg, "bufferType")->valuestring);
			CamDeviceBufMode_t bufMod = VsiVvbenchStrToBuffMode(bufType);
			caseCtx->instanceCfgCtx[instanceId].buffMode = bufMod;
			caseCtx->instanceCfgCtx[instanceId].buffMode = CAMDEV_BUFMODE_USERPTR;

			int workMode = cJSON_GetObjectItem(instanceCfg, "workMode")->valueint;
			cJSON *modeInfo = cJSON_GetObjectItem(instanceCfg, "modeInfo");
			if (modeInfo) {
				result = VsiVvbenchParseModeInfo(workMode, modeInfo,
								 &(caseCtx->instanceCfgCtx[instanceId].workCfg));
				if (0 != result) {
					LOGE("VsiVvbenchParseModeInfo parser error");
				}
			}
			cJSON *outPutType = cJSON_GetObjectItem(instanceCfg, "outPutType");
			if (NULL == outPutType) {
				caseCtx->instanceCfgCtx[instanceId].outPutType = CAMDEV_OUTPUT_TYPE_MEMORY;
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].outPutType = outPutType->valueint;
				if (caseCtx->instanceCfgCtx[instanceId].outPutType) {
					LOGI("outPutType: %d", caseCtx->instanceCfgCtx[instanceId].outPutType);
				}
			}

			cJSON *priority = cJSON_GetObjectItem(instanceCfg, "priority");
			if (NULL == priority) {
				caseCtx->instanceCfgCtx[instanceId].priority = CAMDEV_SEQ_PRI_0;
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].priority = priority->valueint;
				LOGI("priority: %d", caseCtx->instanceCfgCtx[instanceId].priority);
			}

			uint32_t streamDuration = cJSON_GetObjectItem(instanceCfg, "streamDuration")->valueint;
			caseCtx->instanceCfgCtx[instanceId].streamDuration = streamDuration;

			cJSON *startPathSimultaneous = cJSON_GetObjectItem(instanceCfg, "startPathSimultaneous");
			if (NULL == startPathSimultaneous) {
				caseCtx->instanceCfgCtx[instanceId].startPathSimultaneous = false;
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].startPathSimultaneous = startPathSimultaneous->valueint;
			}

			cJSON *stopPathSimultaneous = cJSON_GetObjectItem(instanceCfg, "stopPathSimultaneous");
			if (NULL == stopPathSimultaneous) {
				caseCtx->instanceCfgCtx[instanceId].stopPathSimultaneous = false;
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].stopPathSimultaneous = stopPathSimultaneous->valueint;
			}

			cJSON *instancePathCfgArray = cJSON_GetObjectItem(instanceCfg, "pathCfg");
			if (cJSON_Array != instancePathCfgArray->type) {
				LOGW("case list instancePathCfgArray parser error");
			}

			//disable all data by default
			for (int i = 0; i < CAMDEV_BUFCHAIN_MAX; ++i) {
				caseCtx->instanceCfgCtx[instanceId].instancePath[i].pathEnable = 0;
				caseCtx->instanceCfgCtx[instanceId].instancePath[i].pathState = 0;
				caseCtx->instanceCfgCtx[instanceId].instancePath[i].width = 0;
				caseCtx->instanceCfgCtx[instanceId].instancePath[i].height = 0;
			}
			cJSON *instancePathCfg = instancePathCfgArray->child;
			while (instancePathCfg) {
				VsiVvbenchGetPathInfoFromJson(instancePathCfg, &caseCtx->instanceCfgCtx[instanceId]);
				instancePathCfg = instancePathCfg->next;
			}
			cJSON *sensorInfo = cJSON_GetObjectItem(instanceCfg, "sensorInfo");
			if (sensorInfo) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useSensorFunction = 1;
				result = VsiVvbenchParseSensorInfo(sensorInfo, &caseCtx->instanceCfgCtx[instanceId].sensorCfg);
				if (0 != result) {
					LOGE("VsiVvbenchParseSensorInfo parser error");
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useSensorFunction = 0;
			}

			cJSON *isLowPowerMode = cJSON_GetObjectItem(instanceCfg, "isLowPowerMode");
			if (NULL == isLowPowerMode) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useLowPowerMode = false;
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useLowPowerMode = isLowPowerMode->valueint;
			}

			cJSON *fastResetInfo = cJSON_GetObjectItem(instanceCfg, "fastResetInfo");
			if (fastResetInfo) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useFastResetFunction = 1;
				cJSON *fastResetLoop = cJSON_GetObjectItem(fastResetInfo, "fastResetLoop");
				if (NULL != fastResetLoop) {
					caseCtx->instanceCfgCtx[instanceId].fastResetCfg.fastResetLoop = fastResetLoop->valueint;
					LOGI("fastResetLoop: %d", fastResetLoop->valueint);
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useFastResetFunction = 0;
			}

			cJSON *pathSwitch = cJSON_GetObjectItem(instanceCfg, "pathSwitch");
			if (pathSwitch) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.usePathSwitch = 1;
				result = VsiVvbenchParsePathSwitchInfo(pathSwitch,
								       &caseCtx->instanceCfgCtx[instanceId].pathSwitchCfg);
				if (0 != result) {
					LOGE("VsiVvbenchParsePathSwitchInfo parser error");
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.usePathSwitch = 0;
			}

			cJSON *fusaInfo = cJSON_GetObjectItem(instanceCfg, "fusaInfo");
			if (fusaInfo) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useFusaFunction = 1;
				result = VsiVvbenchParseFusaInfo(fusaInfo, &caseCtx->instanceCfgCtx[instanceId].fusaCfg);
				if (0 != result) {
					LOGE("VsiVvbenchParseFusaInfo parser error");
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useFusaFunction = 0;
			}

			cJSON *tpgInfo = cJSON_GetObjectItem(instanceCfg, "tpgInfo");
			if (tpgInfo) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useIspTpgFunction = 1;
				result = VsiVvbenchParseTpgInfo(tpgInfo, &caseCtx->instanceCfgCtx[instanceId].tpgCfg);
				if (0 != result) {
					LOGE("VsiVvbenchParseTpgInfo parser error");
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useIspTpgFunction = 0;
			}

			cJSON *vi200Info = cJSON_GetObjectItem(instanceCfg, "vi200Info");
			if (vi200Info) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useVi200Function = 1;
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useVi200MetaWin = 1;
				result = VsiVvbenchParseVi200Info(vi200Info, &caseCtx->instanceCfgCtx[instanceId].vi200Cfg);
				if (0 != result) {
					LOGE("VsiVvbenchParseVi200Info parser error");
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useVi200Function = 0;
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useVi200MetaWin = 0;
			}

			cJSON *dewarpInfo = cJSON_GetObjectItem(instanceCfg, "dewarpInfo");
			if (dewarpInfo) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useDewarpFunction = 1;
				result = VsiVvbenchParseDewarpInfo(dewarpInfo, &caseCtx->instanceCfgCtx[instanceId].dewarpCfg);
				if (0 != result) {
					LOGE("VsiVvbenchParseDewarpInfo parser error");
				}
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.useDewarpFunction = 0;
			}

			if (CAMDEV_WORK_MODE_RDMA == caseCtx->instanceCfgCtx[instanceId].workCfg.workMode) {
				VvbenchInstanceSensorCfg_t *pSensorInfo = &(caseCtx->instanceCfgCtx[instanceId].sensorCfg);
				caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].bufferSize =
					((pSensorInfo->sensorWidth) * (pSensorInfo->sensorHeight) << 1) + PIC_BUFFER_ALIGN;
			}
			for (int i = 0; i < CAMDEV_BUFCHAIN_MAX; ++i) {
				if (1 == caseCtx->instanceCfgCtx[instanceId].instancePath[i].pathEnable) {
					if (0 == caseCtx->instanceCfgCtx[instanceId].instancePath[i].width) {
						if (CAMDEV_INPUT_TYPE_IMAGE == caseCtx->instanceCfgCtx[instanceId].inputType) {
							caseCtx->instanceCfgCtx[instanceId].instancePath[i].width =
								caseCtx->instanceCfgCtx[instanceId].pictureCfg.width;
						}
						else if (CAMDEV_INPUT_TYPE_TPG == caseCtx->instanceCfgCtx[instanceId].inputType) {
							caseCtx->instanceCfgCtx[instanceId].instancePath[i].width =
								caseCtx->instanceCfgCtx[instanceId].tpgCfg.width;
						}
						else {
							caseCtx->instanceCfgCtx[instanceId].instancePath[i].width =
								caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorWidth;
						}
					}
					if (0 == caseCtx->instanceCfgCtx[instanceId].instancePath[i].height) {
						if (CAMDEV_INPUT_TYPE_IMAGE == caseCtx->instanceCfgCtx[instanceId].inputType) {
							caseCtx->instanceCfgCtx[instanceId].instancePath[i].height =
								caseCtx->instanceCfgCtx[instanceId].pictureCfg.height;
						}
						else if (CAMDEV_INPUT_TYPE_TPG == caseCtx->instanceCfgCtx[instanceId].inputType) {
							caseCtx->instanceCfgCtx[instanceId].instancePath[i].height =
								caseCtx->instanceCfgCtx[instanceId].tpgCfg.height;
						}
						else {
							caseCtx->instanceCfgCtx[instanceId].instancePath[i].height =
								caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorHeight;
						}
					}
				}
			}

			cJSON *fineTuneJson = cJSON_GetObjectItem(instanceCfg, "fineTuneJson");
			if (fineTuneJson) {
				caseCtx->fineTuneMode = true;
				strncpy(caseCtx->instanceCfgCtx[instanceId].fineTuneJson, fineTuneJson->valuestring, FILE_LEN - 1);
			}

			cJSON *rgbirIrControl = cJSON_GetObjectItem(instanceCfg, "rgbirIrControl");
			if (rgbirIrControl) {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.irRawOutEnable = cJSON_GetObjectItem(rgbirIrControl,
					"irRawOutEnable")->valueint;
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.sp1IrSelect = cJSON_GetObjectItem(rgbirIrControl,
					"sp1IrSelect")->valueint;
			}
			else {
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.irRawOutEnable = 0;
				caseCtx->instanceCfgCtx[instanceId].funcCtrl.sp1IrSelect = 0;
			}

			result = VsiVvbenchParseSubSystemInfo(caseCtx, instanceId);
			if (0 != result) {
				LOGE("VsiVvbenchParseSubSystemInfo parser error");
			}

			result = VsiVvbenchParseSubmoduleInfo(instanceCfg, caseCtx, instanceId);
			if (0 != result) {
				LOGE("VsiVvbenchParseSubmoduleInfo parser error");
			}
			instanceId++;
			instanceCfg = instanceCfg->next;
		}
		cJSON_Delete(root);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

CamDeviceBufMode_t VsiVvbenchStrToBuffMode
(
	const char *bufferString
)
{
	BUFF_MODE buff_mode_value;
	LOGI("%s enter \n", __func__);

	if (0 == strcmp(bufferString, "userptr")) {
		LOGI("Input %s equal to %s", bufferString, "userptr");
		buff_mode_value = CAMDEV_BUFMODE_USERPTR;
	}
	else if (0 == strcmp(bufferString, "resmem")) {
		LOGI("Input %s equal to %s", bufferString, "resmem");
		buff_mode_value = BUFF_MODE_RESMEM;
	}
	else {
		LOGI("Not supprted buffer mode");
		buff_mode_value = CAMDEV_BUFMODE_INVALID;
	}
	LOGI("%s: Buff mode: %d", __func__, buff_mode_value);

	LOGI("%s exit \n", __func__);
	return buff_mode_value;
}

CamDeviceBufChainId_t VsiVvbenchStrToBuffIo
(
	const char *bufferString
)
{
	CamDeviceBufChainId_t buff_io_value;
	LOGI("%s enter \n", __func__);

	if (0 == strcmp(bufferString, "MP")) {
		LOGI("Input %s equal to %s", bufferString, "MP");
		buff_io_value = CAMDEV_BUFCHAIN_MP;
	}
	else if (0 == strcmp(bufferString, "SP1")) {
		LOGI("Input %s equal to %s", bufferString, "SP1");
		buff_io_value = CAMDEV_BUFCHAIN_SP1;
	}
	else if (0 == strcmp(bufferString, "SP2")) {
		LOGI("Input %s equal to %s", bufferString, "SP2");
		buff_io_value = CAMDEV_BUFCHAIN_SP2;
	}
	else if (0 == strcmp(bufferString, "RAW")) {
		LOGI("Input %s equal to %s", bufferString, "RAW");
		buff_io_value = CAMDEV_BUFCHAIN_RAW;
	}
	else if (0 == strcmp(bufferString, "HDR_RAW")) {
		LOGI("Input %s equal to %s", bufferString, "HDR_RAW");
		buff_io_value = CAMDEV_BUFCHAIN_HDR_RAW;
	}
	else if (0 == strcmp(bufferString, "READ")) {
		LOGI("Input %s equal to %s", bufferString, "READ");
		buff_io_value = CAMDEV_BUFCHAIN_RDMA;
	}
	else if (0 == strcmp(bufferString, "RETIMING")) {
		LOGI("Input %s equal to %s", bufferString, "RETIMING");
		buff_io_value = CAMDEV_BUFCHAIN_RETIMING;
	}
	else if (0 == strcmp(bufferString, "METADATA")) {
		LOGI("Input %s equal to %s", bufferString, "METADATA");
		buff_io_value = CAMDEV_BUFCHAIN_METADATA;
	}
	else {
		LOGI("Not supprted buffeio");
		buff_io_value = CAMDEV_BUFCHAIN_MAX;
	}
	LOGI("%s: Buff IO: %d", __func__, buff_io_value);

	LOGI("%s exit \n", __func__);
	return buff_io_value;
}

int VsiVvbenchStrToInputFormat
(
	const char *formatString
)
{
	int imgFormat;
	LOGI("%s enter \n", __func__);

	if (0 == strcmp(formatString, "RAW8") || 0 == strcmp(formatString, "8bits")) {
		LOGI("Input %s equal to %s", formatString, "RAW8");
		imgFormat = CAMDEV_INPUT_FMT_RAW8;
	}
	else if (0 == strcmp(formatString, "RAW10")) {
		LOGI("Input %s equal to %s", formatString, "RAW10");
		imgFormat = CAMDEV_INPUT_FMT_RAW10;
	}
	else if (0 == strcmp(formatString, "RAW10_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RAW10_MODE0");
		imgFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED0;
	}
	else if (0 == strcmp(formatString, "RAW10_MODE1") || 0 == strcmp(formatString, "10bits")) {
		LOGI("Input %s equal to %s", formatString, "RAW10_MODE1");
		imgFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
	}
	else if (0 == strcmp(formatString, "RAW12")) {
		LOGI("Input %s equal to %s", formatString, "RAW12");
		imgFormat = CAMDEV_INPUT_FMT_RAW12;
	}
	else if (0 == strcmp(formatString, "RAW12_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RAW12_MODE0");
		imgFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED0;
	}
	else if (0 == strcmp(formatString, "RAW12_MODE1") || 0 == strcmp(formatString, "12bits")) {
		LOGI("Input %s equal to %s", formatString, "RAW12_MODE1");
		imgFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
	}
	else if (0 == strcmp(formatString, "RAW14")) {
		LOGI("Input %s equal to %s", formatString, "RAW14");
		imgFormat = CAMDEV_INPUT_FMT_RAW14;
	}
	else if (0 == strcmp(formatString, "RAW14_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RAW14_MODE0");
		imgFormat = CAMDEV_INPUT_FMT_RAW14_ALIGNED0;
	}
	else if (0 == strcmp(formatString, "RAW14_MODE1") || 0 == strcmp(formatString, "14bits")) {
		LOGI("Input %s equal to %s", formatString, "RAW14_MODE1");
		imgFormat = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
	}
	else if (0 == strcmp(formatString, "RAW16") || 0 == strcmp(formatString, "16bits")) {
		LOGI("Input %s equal to %s", formatString, "RAW16");
		imgFormat = CAMDEV_INPUT_FMT_RAW16;
	}
	else if (0 == strcmp(formatString, "RAW20_COMPRESS")) {
		LOGI("Input %s equal to %s", formatString, "RAW20_COMPRESS");
		imgFormat = CAMDEV_INPUT_FMT_RAW20_COMPRESS;
	}
	else if (0 == strcmp(formatString, "RAW24")) {
		LOGI("Input %s equal to %s", formatString, "RAW24");
		imgFormat = CAMDEV_INPUT_FMT_RAW24;
	}
	else if (0 == strcmp(formatString, "RAW24_COMPRESS")) {
		LOGI("Input %s equal to %s", formatString, "RAW24_COMPRESS");
		imgFormat = CAMDEV_INPUT_FMT_RAW24_COMPRESS;
	}
	else if (0 == strcmp(formatString, "2DOL")) {
		LOGI("Input %s equal to %s", formatString, "2DOL");
		imgFormat = CAMDEV_INPUT_FMT_2DOL;
	}
	else if (0 == strcmp(formatString, "3DOL")) {
		LOGI("Input %s equal to %s", formatString, "3DOL");
		imgFormat = CAMDEV_INPUT_FMT_3DOL;
	}
	else if (0 == strcmp(formatString, "4DOL")) {
		LOGI("Input %s equal to %s", formatString, "4DOL");
		imgFormat = CAMDEV_INPUT_FMT_4DOL;
	}
	else {
		LOGI("Not supprted format");
		imgFormat = CAMDEV_INPUT_FMT_MAX;
	}
	LOGI("%s: Img Format: %d", __func__, imgFormat);

	LOGI("%s exit \n", __func__);
	return imgFormat;
}

int VsiVvbenchStrToBuffFormat
(
	const char *formatString
)
{
	int imgFormat;
	LOGI("%s enter \n", __func__);

	if (0 == strcmp(formatString, "YUV422SP")) {
		LOGI("Input %s equal to %s", formatString, "YUV422SP");
		imgFormat = CAMDEV_PIX_FMT_YUV422SP;
	}
	else if (0 == strcmp(formatString, "YUV422SP_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "YUV422SP_MODE0");
		imgFormat = CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "YUV422SP_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "YUV422SP_MODE1");
		imgFormat = CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "YUV422I")) {
		LOGI("Input %s equal to %s", formatString, "YUV422I");
		imgFormat = CAMDEV_PIX_FMT_YUV422I;
	}
	else if (0 == strcmp(formatString, "YUV422I_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "YUV422I_MODE1");
		imgFormat = CAMDEV_PIX_FMT_YUV422I_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "YUV420SP")) {
		LOGI("Input %s equal to %s", formatString, "YUV420SP");
		imgFormat = CAMDEV_PIX_FMT_YUV420SP;
	}
	else if (0 == strcmp(formatString, "YUV420SP_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "YUV420SP_MODE0");
		imgFormat = CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "YUV420SP_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "YUV420SP_MODE1");
		imgFormat = CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "YUV444P")) {
		LOGI("Input %s equal to %s", formatString, "YUV444P");
		imgFormat = CAMDEV_PIX_FMT_YUV444P;
	}
	else if (0 == strcmp(formatString, "YUV444I")) {
		LOGI("Input %s equal to %s", formatString, "YUV444I");
		imgFormat = CAMDEV_PIX_FMT_YUV444I;
	}
	else if (0 == strcmp(formatString, "YUV444I_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "YUV444I_MODE0");
		imgFormat = CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "YUV400")) {
		LOGI("Input %s equal to %s", formatString, "YUV400");
		imgFormat = CAMDEV_PIX_FMT_YUV400;
	}
	else if (0 == strcmp(formatString, "YUV400_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "YUV400_MODE0");
		imgFormat = CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "YUV400_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "YUV400_MODE1");
		imgFormat = CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "RGB888")) {
		LOGI("Input %s equal to %s", formatString, "RGB888");
		imgFormat = CAMDEV_PIX_FMT_RGB888;
	}
	else if (0 == strcmp(formatString, "RGB888_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RGB888_MODE0");
		imgFormat = CAMDEV_PIX_FMT_RGB888_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "RGB888P")) {
		LOGI("Input %s equal to %s", formatString, "RGB888P");
		imgFormat = CAMDEV_PIX_FMT_RGB888P;
	}
	else if (0 == strcmp(formatString, "RAW8")) {
		LOGI("Input %s equal to %s", formatString, "RAW8");
		imgFormat = CAMDEV_PIX_FMT_RAW8;
	}
	else if (0 == strcmp(formatString, "RAW10")) {
		LOGI("Input %s equal to %s", formatString, "RAW10");
		imgFormat = CAMDEV_PIX_FMT_RAW10;
	}
	else if (0 == strcmp(formatString, "RAW10_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RAW10_MODE0");
		imgFormat = CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "RAW10_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "RAW10_MODE1");
		imgFormat = CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "RAW12")) {
		LOGI("Input %s equal to %s", formatString, "RAW12");
		imgFormat = CAMDEV_PIX_FMT_RAW12;
	}
	else if (0 == strcmp(formatString, "RAW12_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RAW12_MODE0");
		imgFormat = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "RAW12_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "RAW12_MODE1");
		imgFormat = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "RAW14")) {
		LOGI("Input %s equal to %s", formatString, "RAW14");
		imgFormat = CAMDEV_PIX_FMT_RAW14;
	}
	else if (0 == strcmp(formatString, "RAW14_MODE0")) {
		LOGI("Input %s equal to %s", formatString, "RAW14_MODE0");
		imgFormat = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE0;
	}
	else if (0 == strcmp(formatString, "RAW14_MODE1")) {
		LOGI("Input %s equal to %s", formatString, "RAW14_MODE1");
		imgFormat = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1;
	}
	else if (0 == strcmp(formatString, "RAW16")) {
		LOGI("Input %s equal to %s", formatString, "RAW16");
		imgFormat = CAMDEV_PIX_FMT_RAW16;
	}
	else if (0 == strcmp(formatString, "RAW24")) {
		LOGI("Input %s equal to %s", formatString, "RAW24");
		imgFormat = CAMDEV_PIX_FMT_RAW24;
	}
	else {
		LOGI("Not supprted format");
		imgFormat = CAMDEV_PIX_FMT_MAX;
	}
	LOGI("%s: Img Format: %d", __func__, imgFormat);

	LOGI("%s exit \n", __func__);
	return imgFormat;
}

int VsiVvbenchStrSpliter
(
	char *inputName,
	const char *strDelim,
	char **strList
)
{
	LOGI("%s enter \n", __func__);

	if (NULL == inputName || NULL == strDelim || NULL == strList) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	int index = 0;

	char *token = strtok(inputName, strDelim);
	while (NULL != token) {
		*(strList + index) = token;
		++ index;
		token = strtok(NULL, strDelim);
	}

	LOGI("%s exit \n", __func__);
	return index;
}

int VsiVvbenchStrToRawMetaInfo
(
	const char *rawName,
	VvbenchRawMetaInfo_t *const pRawMetaInfo
)
{
	LOGI("%s enter \n", __func__);

	if (NULL == rawName || NULL == pRawMetaInfo) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGI("raw image name: %s", rawName);

	char strForSplit[FILE_NAME] = {'\0'};
	char *strList[FILE_NAME] = {NULL};

	strcpy(strForSplit, rawName);
	int result = VsiVvbenchStrSpliter(strForSplit, "_", strList);

	if (4 >= result) {
		LOGE("%s split result error , result is %d", __func__, result);
		return -1;
	}

	if (0 == strcmp(strList[2], "8bits")) {
		pRawMetaInfo->bits = 8;
	}
	else if (0 == strcmp(strList[2], "10bits")) {
		pRawMetaInfo->bits = 10;
	}
	else if (0 == strcmp(strList[2], "12bits")) {
		pRawMetaInfo->bits = 12;
	}
	else if (0 == strcmp(strList[2], "14bits")) {
		pRawMetaInfo->bits = 14;
	}
	else if (0 == strcmp(strList[2], "16bits")) {
		pRawMetaInfo->bits = 16;
	}
	else if (0 == strcmp(strList[2], "24bits")) {
		pRawMetaInfo->bits = 24;
	}
	else {
		pRawMetaInfo->bits = 0;
		LOGE("%s input raw meta bits invalid, result is %d", __func__, pRawMetaInfo->bits);
		return -1;
	}

	pRawMetaInfo->imgFormat = VsiVvbenchStrToInputFormat(strList[2]);
	pRawMetaInfo->layout = VsiVvbenchStrToPicLayout(strList[3]);
	VsiVvbenchStrToPicSize(strList[1], &(pRawMetaInfo->height), &(pRawMetaInfo->width));
	if (8 <= result) {
		pRawMetaInfo->frameNum = atoi(strList[7]);
	}
	else {
		pRawMetaInfo->frameNum = 1;
	}

	LOGI("%s exit \n", __func__);
	return 0;
}
int VsiVvbenchStrToPicSize
(
	const char *inputSizeStr,
	uint32_t *const height,
	uint32_t *const width
)
{
	LOGI("%s enter \n", __func__);

	if (NULL == inputSizeStr || NULL == height || NULL == width) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	char strForSplit[FILE_NAME] = {'\0'};
	char *strList[FILE_NAME] = {NULL};

	strcpy(strForSplit, inputSizeStr);
	int result = VsiVvbenchStrSpliter(strForSplit, "x", strList);
	if (2 != result) {
		LOGE("%s split result error , result is %d", __func__, result);
		return -1;
	}
	*width = atoi(strList[0]);
	*height = atoi(strList[1]);

	LOGI("%s exit \n", __func__);
	return 0;
}
int VsiVvbenchStrToInputType
(
	const char *inputType
)
{
	int inputTypeFormat;
	LOGI("%s enter \n", __func__);

	if (0 == strcmp(inputType, "Sensor")) {
		LOGI("Input %s equal to %s", inputType, "Sensor");
		inputTypeFormat = CAMDEV_INPUT_TYPE_SENSOR;
	}
	else if (0 == strcmp(inputType, "User")) {
		LOGI("Input %s equal to %s", inputType, "User");
		inputTypeFormat = CAMDEV_INPUT_TYPE_IMAGE;
	}
	else if (0 == strcmp(inputType, "Tpg")) {
		LOGI("Input %s equal to %s", inputType, "Tpg");
		inputTypeFormat = CAMDEV_INPUT_TYPE_TPG;
	}
	else {
		LOGE("Not supprted format");
		inputTypeFormat = CAMDEV_INPUT_TYPE_INVALID;
	}
	LOGI("%s: inputTypeFormat: %d", __func__, inputTypeFormat);

	LOGI("%s exit \n", __func__);
	return inputTypeFormat;
}

int VsiVvbenchStrToPicLayout
(
	const char *layoutString
)
{
	int inputPicLayout;
	LOGI("%s enter \n", __func__);

	if (0 == strcmp(layoutString, "RGGB")) {
		LOGI("input pic layout pattern is %s ", "RGGB");
		inputPicLayout = CAMDEV_RAW_RGB_PAT_RGGB;
	}
	else if (0 == strcmp(layoutString, "GRBG")) {
		LOGI("input pic layout pattern is %s ", "GRBG");
		inputPicLayout = CAMDEV_RAW_RGB_PAT_GRBG;
	}
	else if (0 == strcmp(layoutString, "GBRG")) {
		LOGI("input pic layout pattern is %s ", "GBRG");
		inputPicLayout = CAMDEV_RAW_RGB_PAT_GBRG;
	}
	else if (0 == strcmp(layoutString, "BGGR")) {
		LOGI("input pic layout pattern is %s ", "BGGR");
		inputPicLayout = CAMDEV_RAW_RGB_PAT_BGGR;
	}
	else if (0 == strcmp(layoutString, "BGGIR")) {
		LOGI("input pic layout pattern is %s ", "BGGIR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_BGGIR;
	}
	else if (0 == strcmp(layoutString, "GRIRG")) {
		LOGI("input pic layout pattern is %s ", "GRIRG");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_GRIRG;
	}
	else if (0 == strcmp(layoutString, "RGGIR")) {
		LOGI("input pic layout pattern is %s ", "RGGIR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_RGGIR;
	}
	else if (0 == strcmp(layoutString, "GBIRG")) {
		LOGI("input pic layout pattern is %s ", "GBIRG");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_GBIRG;
	}
	else if (0 == strcmp(layoutString, "GIRRG")) {
		LOGI("input pic layout pattern is %s ", "GIRRG");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_GIRRG;
	}
	else if (0 == strcmp(layoutString, "IRGGB")) {
		LOGI("input pic layout pattern is %s ", "IRGGB");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_IRGGB;
	}
	else if (0 == strcmp(layoutString, "GIRBG")) {
		LOGI("input pic layout pattern is %s ", "GIRBG");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_GIRBG;
	}
	else if (0 == strcmp(layoutString, "IRGGR")) {
		LOGI("input pic layout pattern is %s ", "IRGGR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_IRGGR;
	}
	else if (0 == strcmp(layoutString, "RGIRB")) {
		LOGI("input pic layout pattern is %s ", "RGIRB");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_RGIRB;
	}
	else if (0 == strcmp(layoutString, "GRBIR")) {
		LOGI("input pic layout pattern is %s ", "GRBIR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_GRBIR;
	}
	else if (0 == strcmp(layoutString, "IRBRG")) {
		LOGI("input pic layout pattern is %s ", "IRBRG");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_IRBRG;
	}
	else if (0 == strcmp(layoutString, "BIRGR")) {
		LOGI("input pic layout pattern is %s ", "BIRGR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_BIRGR;
	}
	else if (0 == strcmp(layoutString, "BGIRR")) {
		LOGI("input pic layout pattern is %s ", "BGIRR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_BGIRR;
	}
	else if (0 == strcmp(layoutString, "GBRIR")) {
		LOGI("input pic layout pattern is %s ", "GBRIR");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_GBRIR;
	}
	else if (0 == strcmp(layoutString, "IRRBG")) {
		LOGI("input pic layout pattern is %s ", "IRRBG");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_IRRBG;
	}
	else if (0 == strcmp(layoutString, "RIRGB")) {
		LOGI("input pic layout pattern is %s ", "RIRGB");
		inputPicLayout = CAMDEV_RAW_RGBIR_PAT_RIRGB;
	}
	else {
		LOGI("Not supprted format");
		inputPicLayout = CAMDEV_RAW_PAT_MAX;
	}

	LOGI("%s exit \n", __func__);
	return inputPicLayout;
}

int VsiVvbenchParseTpgInfo
(
	cJSON *tpgInfo,
	VvbenchInstanceTpgCfg_t *tpgCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == tpgCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	if (NULL != cJSON_GetObjectItem(tpgInfo, "enable")) {
		tpgCtrl->enable = cJSON_GetObjectItem(tpgInfo, "enable")->valueint;
	}
	if (!tpgCtrl->enable) {
		LOGI("TPG feature not enable!");
	}
	else {
		LOGI("TPG feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(tpgInfo, "useCfg")) {
		tpgCtrl->useCfg = cJSON_GetObjectItem(tpgInfo, "useCfg")->valueint;
	}
	if (tpgCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(tpgInfo, "imageType")) {
			tpgCtrl->imageType = cJSON_GetObjectItem(tpgInfo, "imageType")->valueint;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "bayerPattern")) {
			tpgCtrl->bayerPattern = cJSON_GetObjectItem(tpgInfo, "bayerPattern")->valueint;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "colorDepth")) {
			tpgCtrl->colorDepth = cJSON_GetObjectItem(tpgInfo, "colorDepth")->valueint;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "resolution")) {
			tpgCtrl->resolution = cJSON_GetObjectItem(tpgInfo, "resolution")->valueint;
		}

		if (NULL != cJSON_GetObjectItem(tpgInfo, "pixleGap")) {
			tpgCtrl->pixleGap = cJSON_GetObjectItem(tpgInfo, "pixleGap")->valueint;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "lineGap")) {
			tpgCtrl->lineGap = cJSON_GetObjectItem(tpgInfo, "lineGap")->valueint;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "gapStandard")) {
			tpgCtrl->gapStandard = cJSON_GetObjectItem(tpgInfo, "gapStandard")->valueint;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "randomSeed")) {
			tpgCtrl->randomSeed = cJSON_GetObjectItem(tpgInfo, "randomSeed")->valuedouble;
		}
		if (NULL != cJSON_GetObjectItem(tpgInfo, "frameNum")) {
			tpgCtrl->frameNum = cJSON_GetObjectItem(tpgInfo, "frameNum")->valueint;
		}

		cJSON *userMode = cJSON_GetObjectItem(tpgInfo, "userMode");
		if (userMode == NULL || !cJSON_IsObject(userMode)) {
			LOGW("userMode is not an object or not found!");
			cJSON_Delete(userMode);
		}
		else {
			cJSON *arrayH = cJSON_GetObjectItem(userMode, "H");
			cJSON *arrayV = cJSON_GetObjectItem(userMode, "V");

			if (cJSON_IsArray(arrayH) && cJSON_IsArray(arrayH)) {
				int arraySizeH = cJSON_GetArraySize(arrayH);
				int arraySizeV = cJSON_GetArraySize(arrayV);

				for (int i = 0; i < arraySizeH; i++) {
					cJSON *valueH = cJSON_GetArrayItem(arrayH, i);
					if (cJSON_IsNumber(valueH)) {
						LOGI("userMode.H[%d]:%d", i, valueH->valueint);
						*((uint16_t *)&tpgCtrl->userMode.H + i) = (uint16_t)valueH->valueint;
					}
				}

				for (int i = 0; i < arraySizeV; i++) {
					cJSON *valueV = cJSON_GetArrayItem(arrayV, i);
					if (cJSON_IsNumber(valueV)) {
						LOGI("userMode.V[%d]:%d", i, valueV->valueint);
						*((uint16_t *)&tpgCtrl->userMode.V + i) = (uint16_t)valueV->valueint;
					}
				}
			}
		}

		switch (tpgCtrl->resolution) {
			case 0:
				tpgCtrl->width = 1920;
				tpgCtrl->height = 1080;
				break;
			case 1:
				tpgCtrl->width = 1280;
				tpgCtrl->height = 720;
				break;
			case 2:
				tpgCtrl->width = 3840;
				tpgCtrl->height = 2160;
				break;
			case 3:
				tpgCtrl->width = tpgCtrl->userMode.H.act;
				tpgCtrl->height = tpgCtrl->userMode.V.act;
				break;
			default:
				LOGE("invalid tpg resolution mode:%d [%d, %d]\n", tpgCtrl->resolution, tpgCtrl->width,
				     tpgCtrl->height);
				return -1;
		}
	}

	return result;
}

int VsiVvbenchParsePathSwitchInfo
(
	cJSON *pathSwitch,
	VvbenchInstancePathSwitchCfg_t *pathSwitchCfg
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == pathSwitch) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	cJSON *isBufferReCreate = cJSON_GetObjectItem(pathSwitch, "isBufferReCreate");
	if (NULL != isBufferReCreate) {
		pathSwitchCfg->isBufferReCreate = isBufferReCreate->valueint;
		LOGI("Path switch Cfg isBufferReCreate: %d", pathSwitchCfg->isBufferReCreate);
	}

	cJSON *pathId = cJSON_GetObjectItem(pathSwitch, "pathId");
	if (cJSON_IsArray(pathId)) {
		pathSwitchCfg->pathIdLength = cJSON_GetArraySize(pathId);

		for (int i = 0; i < pathSwitchCfg->pathIdLength; i++) {
			cJSON *pathValue = cJSON_GetArrayItem(pathId, i);
			if (cJSON_IsNumber(pathValue)) {
				LOGI("pathId[%d]:%d", i, pathValue->valueint);
				pathSwitchCfg->pathId[i] = (uint16_t)pathValue->valueint;
			}
		}
	}

	if (NULL != cJSON_GetObjectItem(pathSwitch, "cropPathId")) {
		pathSwitchCfg->cropPathId = cJSON_GetObjectItem(pathSwitch, "cropPathId")->valueint;
		LOGI("crop Path Id: %d", pathSwitchCfg->cropPathId);
	}

	cJSON *cropWindowHOffset = cJSON_GetObjectItem(pathSwitch, "cropWindowHOffset");
	cJSON *cropWindowVOffset = cJSON_GetObjectItem(pathSwitch, "cropWindowVOffset");
	cJSON *cropWindowWidth = cJSON_GetObjectItem(pathSwitch, "cropWindowWidth");
	cJSON *cropWindowHeight = cJSON_GetObjectItem(pathSwitch, "cropWindowHeight");
	if ((cropWindowHOffset == NULL) || (cropWindowVOffset == NULL) ||
	    (cropWindowWidth == NULL) || (cropWindowHeight == NULL)) {
		cJSON_Delete(cropWindowHOffset);
		cJSON_Delete(cropWindowVOffset);
		cJSON_Delete(cropWindowWidth);
		cJSON_Delete(cropWindowHeight);
	}
	else {
		pathSwitchCfg->cropWindowLength = cJSON_GetArraySize(cropWindowHOffset);

		for (int i = 0; i < pathSwitchCfg->cropWindowLength; i++) {
			cJSON *cropWindowHOffsetValue = cJSON_GetArrayItem(cropWindowHOffset, i);
			if (cJSON_IsNumber(cropWindowHOffsetValue)) {
				LOGI("cropWindowHOffsetValue[%d]:%d", i, cropWindowHOffsetValue->valueint);
				pathSwitchCfg->cropWindowHOffset[i] = (uint16_t)cropWindowHOffsetValue->valueint;
			}

			cJSON *cropWindowVOffsetValue = cJSON_GetArrayItem(cropWindowVOffset, i);
			if (cJSON_IsNumber(cropWindowVOffsetValue)) {
				LOGI("cropWindowVOffsetValue[%d]:%d", i, cropWindowVOffsetValue->valueint);
				pathSwitchCfg->cropWindowVOffset[i] = (uint16_t)cropWindowVOffsetValue->valueint;
			}

			cJSON *cropWindowWidthValue = cJSON_GetArrayItem(cropWindowWidth, i);
			if (cJSON_IsNumber(cropWindowWidthValue)) {
				LOGI("cropWindowWidthValue[%d]:%d", i, cropWindowWidthValue->valueint);
				pathSwitchCfg->cropWindowWidth[i] = (uint16_t)cropWindowWidthValue->valueint;
			}

			cJSON *cropWindowHeightValue = cJSON_GetArrayItem(cropWindowHeight, i);
			if (cJSON_IsNumber(cropWindowHeightValue)) {
				LOGI("cropWindowHeightValue[%d]:%d", i, cropWindowHeightValue->valueint);
				pathSwitchCfg->cropWindowHeight[i] = (uint16_t)cropWindowHeightValue->valueint;
			}
		}
	}

	return result;
}


int VsiVvbenchParseFusaInfo
(
	cJSON *inputInfo,
	VvbenchInstanceFusaCfg_t *const pFusaCfg
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == pFusaCfg) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	cJSON *enable = cJSON_GetObjectItem(inputInfo, "enable");
	if (NULL != enable) {
		pFusaCfg->enable = enable->valueint;
	}
	if (!pFusaCfg->enable) {
		LOGI("FUSA feature not enable!");
	}
	else {
		LOGI("FUSA feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(inputInfo, "useCfg")) {
		pFusaCfg->useCfg = cJSON_GetObjectItem(inputInfo, "useCfg")->valueint;
	}
	if (pFusaCfg->useCfg) {
		cJSON *eccEn = cJSON_GetObjectItem(inputInfo, "eccEn");
		if (NULL != eccEn) {
			pFusaCfg->eccEn = eccEn->valueint;
			LOGI("Fusa eccEn: %d", pFusaCfg->eccEn);
		}

		cJSON *pixCountEn = cJSON_GetObjectItem(inputInfo, "pixCountEn");
		if (NULL != pixCountEn) {
			pFusaCfg->pixCountEn = pixCountEn->valueint;
			LOGI("Fusa pixCountEn: %d", pFusaCfg->pixCountEn);
		}

		cJSON *timeOutEn = cJSON_GetObjectItem(inputInfo, "timeOutEn");
		if (NULL != timeOutEn) {
			pFusaCfg->timeOutEn = timeOutEn->valueint;
			LOGI("Fusa timeOutEn: %d", pFusaCfg->timeOutEn);
		}

		cJSON *bistEn = cJSON_GetObjectItem(inputInfo, "bistEn");
		if (NULL != bistEn) {
			pFusaCfg->bistEn = bistEn->valueint;
			LOGI("Fusa bistEn: %d", pFusaCfg->bistEn);
		}

		cJSON *bistPowerUpEn = cJSON_GetObjectItem(inputInfo, "bistPowerUpEn");
		if (NULL != bistPowerUpEn) {
			pFusaCfg->bistPowerUpEn = bistPowerUpEn->valueint;
			LOGI("Fusa bistPowerUpEn: %d", pFusaCfg->bistPowerUpEn);
		}

		cJSON *crcInRevEn = cJSON_GetObjectItem(inputInfo, "crcInRevEn");
		if (NULL != crcInRevEn) {
			pFusaCfg->crcInRevEn = crcInRevEn->valueint;
			LOGI("Fusa crcInRevEn: %d", pFusaCfg->crcInRevEn);
		}

		cJSON *crcOutRevEn = cJSON_GetObjectItem(inputInfo, "crcOutRevEn");
		if (NULL != crcOutRevEn) {
			pFusaCfg->crcOutRevEn = crcOutRevEn->valueint;
			LOGI("Fusa crcOutRevEn: %d", pFusaCfg->crcOutRevEn);
		}

		cJSON *crcXorEn = cJSON_GetObjectItem(inputInfo, "crcXorEn");
		if (NULL != crcXorEn) {
			pFusaCfg->crcXorEn = crcXorEn->valueint;
			LOGI("Fusa crcXorEn: %d", pFusaCfg->crcXorEn);
		}

		cJSON *crcEn = cJSON_GetObjectItem(inputInfo, "crcEn");
		if (NULL != crcEn) {
			pFusaCfg->crcEn = crcEn->valueint;
			LOGI("Fusa crcEn: %d", pFusaCfg->crcEn);
		}

		cJSON *faultInjectionAhbTimeOutEn = cJSON_GetObjectItem(inputInfo, "faultInjectionAhbTimeOutEn");
		if (NULL != faultInjectionAhbTimeOutEn) {
			pFusaCfg->faultInjectionAhbTimeOutEn = faultInjectionAhbTimeOutEn->valueint;
			LOGI("Fusa faultInjectionAhbTimeOutEn: %d", pFusaCfg->faultInjectionAhbTimeOutEn);
		}

		cJSON *faultInjectionDupEn = cJSON_GetObjectItem(inputInfo, "faultInjectionDupEn");
		if (NULL != faultInjectionDupEn) {
			pFusaCfg->faultInjectionDupEn = faultInjectionDupEn->valueint;
			LOGI("Fusa faultInjectionDupEn: %d", pFusaCfg->faultInjectionDupEn);
		}

		cJSON *faultInjectionEcc2bitEn = cJSON_GetObjectItem(inputInfo, "faultInjectionEcc2bitEn");
		if (NULL != faultInjectionEcc2bitEn) {
			pFusaCfg->faultInjectionEcc2bitEn = faultInjectionEcc2bitEn->valueint;
			LOGI("Fusa faultInjectionEcc2bitEn: %d", pFusaCfg->faultInjectionEcc2bitEn);
		}

		cJSON *faultInjectionEcc1bitEn = cJSON_GetObjectItem(inputInfo, "faultInjectionEcc1bitEn");
		if (NULL != faultInjectionEcc1bitEn) {
			pFusaCfg->faultInjectionEcc1bitEn = faultInjectionEcc1bitEn->valueint;
			LOGI("Fusa faultInjectionEcc1bitEn: %d", pFusaCfg->faultInjectionEcc1bitEn);
		}

		cJSON *crcLevel = cJSON_GetObjectItem(inputInfo, "crcLevel");
		if (NULL != crcLevel) {
			pFusaCfg->crcLevel = crcLevel->valueint;
			LOGI("Fusa crcLevel: %d", pFusaCfg->crcLevel);
		}

		cJSON *crcMpRoiH = cJSON_GetObjectItem(inputInfo, "crcMpRoiH");
		if (NULL != crcMpRoiH) {
			pFusaCfg->crcMpRoiH = crcMpRoiH->valueint;
			LOGI("Fusa crcMpRoiH: %d", pFusaCfg->crcMpRoiH);
		}

		cJSON *crcMpRoiV = cJSON_GetObjectItem(inputInfo, "crcMpRoiV");
		if (NULL != crcMpRoiV) {
			pFusaCfg->crcMpRoiV = crcMpRoiV->valueint;
			LOGI("Fusa crcMpRoiV: %d", pFusaCfg->crcMpRoiV);
		}

		cJSON *crcSp1RoiH = cJSON_GetObjectItem(inputInfo, "crcSp1RoiH");
		if (NULL != crcSp1RoiH) {
			pFusaCfg->crcSp1RoiH = crcSp1RoiH->valueint;
			LOGI("Fusa crcSp1RoiH: %d", pFusaCfg->crcSp1RoiH);
		}

		cJSON *crcSp1RoiV = cJSON_GetObjectItem(inputInfo, "crcSp1RoiV");
		if (NULL != crcSp1RoiV) {
			pFusaCfg->crcSp1RoiV = crcSp1RoiV->valueint;
			LOGI("Fusa crcSp1RoiV: %d", pFusaCfg->crcSp1RoiV);
		}

		cJSON *crcSp2RoiH = cJSON_GetObjectItem(inputInfo, "crcSp2RoiH");
		if (NULL != crcSp2RoiH) {
			pFusaCfg->crcSp2RoiH = crcSp2RoiH->valueint;
			LOGI("Fusa crcSp2RoiH: %d", pFusaCfg->crcSp2RoiH);
		}

		cJSON *crcSp2RoiV = cJSON_GetObjectItem(inputInfo, "crcSp2RoiV");
		if (NULL != crcSp2RoiV) {
			pFusaCfg->crcSp2RoiV = crcSp2RoiV->valueint;
			LOGI("Fusa crcSp2RoiV: %d", pFusaCfg->crcSp2RoiV);
		}

#ifdef ISP_RDCD
		cJSON *RdcPixLossEn = cJSON_GetObjectItem(inputInfo, "RdcPixLossEn");
		if (NULL != RdcPixLossEn) {
			pFusaCfg->RdcPixLossEn = RdcPixLossEn->valueint;
			LOGI("Fusa RdcPixLossEn: %d", pFusaCfg->RdcPixLossEn);
		}

		cJSON *RdcEccEn = cJSON_GetObjectItem(inputInfo, "RdcEccEn");
		if (NULL != RdcEccEn) {
			pFusaCfg->RdcEccEn = RdcEccEn->valueint;
			LOGI("Fusa RdcEccEn: %d", pFusaCfg->RdcEccEn);
		}

		cJSON *RdcFaultInjectionDupEn = cJSON_GetObjectItem(inputInfo, "RdcFaultInjectionDupEn");
		if (NULL != RdcFaultInjectionDupEn) {
			pFusaCfg->RdcFaultInjectionDupEn = RdcFaultInjectionDupEn->valueint;
			LOGI("Fusa RdcFaultInjectionDupEn: %d", pFusaCfg->RdcFaultInjectionDupEn);
		}

		cJSON *RdcFultInjectionEcc2bitEn = cJSON_GetObjectItem(inputInfo, "RdcFultInjectionEcc2bitEn");
		if (NULL != RdcFultInjectionEcc2bitEn) {
			pFusaCfg->RdcFultInjectionEcc2bitEn = RdcFultInjectionEcc2bitEn->valueint;
			LOGI("Fusa RdcFultInjectionEcc2bitEn: %d", pFusaCfg->RdcFultInjectionEcc2bitEn);
		}

		cJSON *RdcFaultInjectionEcc1bitEn = cJSON_GetObjectItem(inputInfo, "RdcFaultInjectionEcc1bitEn");
		if (NULL != RdcFaultInjectionEcc1bitEn) {
			pFusaCfg->RdcFaultInjectionEcc1bitEn = RdcFaultInjectionEcc1bitEn->valueint;
			LOGI("Fusa RdcFaultInjectionEcc1bitEn: %d", pFusaCfg->RdcFaultInjectionEcc1bitEn);
		}

		cJSON *RdcFaultInjectionFifoEn = cJSON_GetObjectItem(inputInfo, "RdcFaultInjectionFifoEn");
		if (NULL != RdcFaultInjectionFifoEn) {
			pFusaCfg->RdcFaultInjectionFifoEn = RdcFaultInjectionFifoEn->valueint;
			LOGI("Fusa RdcFaultInjectionFifoEn: %d", pFusaCfg->RdcFaultInjectionFifoEn);
		}

		cJSON *RdcCrcInRevEn = cJSON_GetObjectItem(inputInfo, "RdcCrcInRevEn");
		if (NULL != RdcCrcInRevEn) {
			pFusaCfg->RdcCrcInRevEn = RdcCrcInRevEn->valueint;
			LOGI("Fusa RdcCrcInRevEn: %d", pFusaCfg->RdcCrcInRevEn);
		}

		cJSON *RdcCrcOutRevEn = cJSON_GetObjectItem(inputInfo, "RdcCrcOutRevEn");
		if (NULL != RdcCrcOutRevEn) {
			pFusaCfg->RdcCrcOutRevEn = RdcCrcOutRevEn->valueint;
			LOGI("Fusa RdcCrcOutRevEn: %d", pFusaCfg->RdcCrcOutRevEn);
		}

		cJSON *RdcCrcXorEn = cJSON_GetObjectItem(inputInfo, "RdcCrcXorEn");
		if (NULL != RdcCrcXorEn) {
			pFusaCfg->RdcCrcXorEn = RdcCrcXorEn->valueint;
			LOGI("Fusa RdcCrcXorEn: %d", pFusaCfg->RdcCrcXorEn);
		}

		cJSON *RdcCrcEn = cJSON_GetObjectItem(inputInfo, "RdcCrcEn");
		if (NULL != RdcCrcEn) {
			pFusaCfg->RdcCrcEn = RdcCrcEn->valueint;
			LOGI("Fusa RdcCrcEn: %d", pFusaCfg->RdcCrcEn);
		}
#endif
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseSensorInfo
(
	cJSON *sensorInfo,
	VvbenchInstanceSensorCfg_t *sensorCfg
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == sensorCfg) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	cJSON *sensorName = cJSON_GetObjectItem(sensorInfo, "sensorName");
	if (NULL != sensorName) {
		strcpy(sensorCfg->sensorName, sensorName->valuestring);
		// char* ts_name = "virtual_ox03f10";
		// int i = 0;
		// for(i = 0; ts_name[i] != '\0'; ++i){
		//	sensorCfg->sensorName[i] = ts_name[i];
		// }
		// sensorCfg->sensorName[i] = '\0';

		LOGI("sensorName: %s", sensorCfg->sensorName);
	}
	cJSON *sensorDevId = cJSON_GetObjectItem(sensorInfo, "sensorDevId");
	if (NULL != sensorDevId) {
		sensorCfg->sensorDevId = (uint32_t)sensorDevId->valueint; //NEXT BUILD 2026.1
		LOGI("sensorDevId: %d", sensorCfg->sensorDevId);
	}
	cJSON *calibrationName = cJSON_GetObjectItem(sensorInfo, "calibrationName");
	if (NULL != calibrationName) {
		strcpy(sensorCfg->calibrationName, calibrationName->valuestring);
		LOGI("calibrationName: %s", sensorCfg->calibrationName);
	}

	cJSON *simulatorName = cJSON_GetObjectItem(sensorInfo, "simulatorName");
	if (NULL != simulatorName) {
		strcpy(sensorCfg->simulatorName, simulatorName->valuestring);
		LOGI("simulatorName: %s", sensorCfg->simulatorName);
	}

	cJSON *autoSimulatorName = cJSON_GetObjectItem(sensorInfo, "autoSimulatorName");
	if (NULL != autoSimulatorName) {
		strcpy(sensorCfg->autoSimulatorName, autoSimulatorName->valuestring);
		LOGI("autoSimulatorName: %s", sensorCfg->autoSimulatorName);
	}

	cJSON *modeIndex = cJSON_GetObjectItem(sensorInfo, "modeIndex");
	if (NULL != modeIndex) {
		sensorCfg->modeIndex = modeIndex->valueint;
		if (sensorCfg->modeIndex) {
			LOGI("modeIndex: %d", sensorCfg->modeIndex);
		}
	}

	cJSON *illumType = cJSON_GetObjectItem(sensorInfo, "illumType");
	if (NULL != illumType) {
		strcpy(sensorCfg->illumType, illumType->valuestring);
		LOGI("illumType: %s", sensorCfg->illumType);
	}

	cJSON *resolutionWidth = cJSON_GetObjectItem(sensorInfo, "resolutionWidth");
	if (NULL != resolutionWidth) {
		sensorCfg->sensorWidth = resolutionWidth->valueint;
		if (sensorCfg->sensorWidth) {
			LOGI("sensor width: %d", sensorCfg->sensorWidth);
		}
	}

	cJSON *resolutionHeight = cJSON_GetObjectItem(sensorInfo, "resolutionHeight");
	if (NULL != resolutionHeight) {
		sensorCfg->sensorHeight = resolutionHeight->valueint;
		if (sensorCfg->sensorHeight) {
			LOGI("sensor height: %d", sensorCfg->sensorHeight);
		}
	}
	LOGI("sensor resolution: %dx%d", sensorCfg->sensorWidth, sensorCfg->sensorHeight);

	cJSON *fps = cJSON_GetObjectItem(sensorInfo, "fps");
	if (NULL != fps) {
		sensorCfg->fps = fps->valuedouble;
		if (sensorCfg->fps) {
			LOGI("sensor fps: %.2f", sensorCfg->fps);
		}
	}

	cJSON *otpEnable = cJSON_GetObjectItem(sensorInfo, "otpEnable");
	if (NULL != otpEnable) {
		sensorCfg->otpEnable = otpEnable->valueint;
		if (sensorCfg->otpEnable) {
			LOGI("sensor otpEnable: %d", sensorCfg->otpEnable);
		}
	}

	cJSON *queryEnable = cJSON_GetObjectItem(sensorInfo, "queryEnable");
	if (NULL != queryEnable) {
		sensorCfg->queryEnable = queryEnable->valueint;
		if (sensorCfg->queryEnable) {
			LOGI("sensor queryEnable: %d", sensorCfg->queryEnable);
		}
	}

	cJSON *capsEnable = cJSON_GetObjectItem(sensorInfo, "capsEnable");
	if (NULL != capsEnable) {
		sensorCfg->capsEnable = capsEnable->valueint;
		if (sensorCfg->capsEnable) {
			LOGI("sensor capsEnable: %d", sensorCfg->capsEnable);
		}
	}

	cJSON *useSensorCfg = cJSON_GetObjectItem(sensorInfo, "useSensorCfg");
	if (NULL != useSensorCfg) {
		sensorCfg->useSensorCfg = useSensorCfg->valueint;
		if (sensorCfg->useSensorCfg) {
			LOGI("sensor useSensorCfg: %d", sensorCfg->useSensorCfg);
		}
	}

	cJSON *statusEnable = cJSON_GetObjectItem(sensorInfo, "statusEnable");
	if (NULL != statusEnable) {
		sensorCfg->statusEnable = statusEnable->valueint;
		if (sensorCfg->statusEnable) {
			LOGI("sensor statusEnable: %d", sensorCfg->statusEnable);
		}
	}

	cJSON *testPatternEnable = cJSON_GetObjectItem(sensorInfo, "testPatternEnable");
	if (NULL != testPatternEnable) {
		sensorCfg->testPatternEnable = testPatternEnable->valueint;
		if (sensorCfg->testPatternEnable) {
			LOGI("sensor testPatternEnable: %d", sensorCfg->testPatternEnable);
		}
	}

	cJSON *driverListEnable = cJSON_GetObjectItem(sensorInfo, "driverListEnable");
	if (NULL != driverListEnable) {
		sensorCfg->driverListEnable = driverListEnable->valueint;
		if (sensorCfg->driverListEnable) {
			LOGI("sensor driverListEnable: %d", sensorCfg->driverListEnable);
		}
	}

	cJSON *infoEnable = cJSON_GetObjectItem(sensorInfo, "infoEnable");
	if (NULL != infoEnable) {
		sensorCfg->infoEnable = infoEnable->valueint;
		if (sensorCfg->infoEnable) {
			LOGI("sensor infoEnable: %d", sensorCfg->infoEnable);
		}
	}

	cJSON *registerAddress = cJSON_GetObjectItem(sensorInfo, "registerAddress");
	if (NULL != registerAddress) {
		sensorCfg->registerAddress = registerAddress->valueint;
		if (sensorCfg->registerAddress) {
			LOGI("sensor registerAddress:0x%x", sensorCfg->registerAddress);
			sensorCfg->registerValue = cJSON_GetObjectItem(sensorInfo, "registerValue")->valueint;
			LOGI("sensor registerValue:0x%x", sensorCfg->registerValue);
		}
	}

	cJSON *exposurtime = cJSON_GetObjectItem(sensorInfo, "exposurtime");
	if (NULL != exposurtime) {
		sensorCfg->exposurtime = exposurtime->valuedouble;
		if (sensorCfg->exposurtime) {
			LOGI("sensor exposurtime: %f", sensorCfg->exposurtime);
		}
	}

	cJSON *totalGain = cJSON_GetObjectItem(sensorInfo, "totalGain");
	if (NULL != totalGain) {
		sensorCfg->totalGain = totalGain->valuedouble;
		if (sensorCfg->totalGain) {
			LOGI("sensor totalGain: %f", sensorCfg->totalGain);
		}
	}

	cJSON *gain = cJSON_GetObjectItem(sensorInfo, "gain");
	if (NULL != gain) {
		cJSON *aGain = cJSON_GetObjectItem(gain, "aGain");
		if (NULL != aGain) {
			sensorCfg->gain.aGain = aGain->valuedouble;
			if (sensorCfg->gain.aGain) {
				LOGI("sensor aGain: %f", sensorCfg->gain.aGain);
			}
		}
		cJSON *dGain = cJSON_GetObjectItem(gain, "dGain");
		if (NULL != dGain) {
			sensorCfg->gain.dGain = dGain->valuedouble;
			if (sensorCfg->gain.dGain) {
				LOGI("sensor dGain: %f", sensorCfg->gain.dGain);
			}
		}
	}

	cJSON *useExternalDriver = cJSON_GetObjectItem(sensorInfo, "useExternalDriver");
	if (NULL != useExternalDriver) {
		sensorCfg->useExternalDriver = useExternalDriver->valueint;
		if (sensorCfg->useExternalDriver) {
			LOGI("sensor useExternalDriver: %d", sensorCfg->useExternalDriver);
		}
	}

	cJSON *virtualChannelId = cJSON_GetObjectItem(sensorInfo, "virtualChannelId");
	if (NULL != virtualChannelId) {
		sensorCfg->virtualChannelId = (uint32_t)virtualChannelId->valueint;
		LOGI("virtualChannelId: %d", sensorCfg->virtualChannelId);
	}
	else {
		sensorCfg->virtualChannelId = 0xFFFFFFFF;
	}
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParsePicInfo
(
	cJSON *inputInfo,
	VvbenchInstancePictureCfg_t *pictureCfg
)
{
	LOGI("%s enter \n", __func__);
	if (NULL == pictureCfg) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	cJSON *loopCnt = cJSON_GetObjectItem(inputInfo, "loopCnt");
	if (NULL != loopCnt) {
		pictureCfg->loopCnt = loopCnt->valueint;
		LOGI("loopCnt: %d", pictureCfg->loopCnt);
	}

	cJSON *frameNum = cJSON_GetObjectItem(inputInfo, "frameNum");
	if (NULL != frameNum) {
		pictureCfg->frameNum = frameNum->valueint;
		LOGI("frameNum: %d", pictureCfg->frameNum);
	}

	cJSON *loadIndex = cJSON_GetObjectItem(inputInfo, "loadIndex");
	if (NULL != loadIndex) {
		pictureCfg->loadIndex = loadIndex->valueint;
		LOGI("loadIndex: %d", pictureCfg->loadIndex);
	}
	else {
		pictureCfg->loadIndex = 1;
	}

	cJSON *PicName = cJSON_GetObjectItem(inputInfo, "PicName");
	if (NULL != PicName) {
		strcpy(pictureCfg->pictureName, PicName->valuestring);
		LOGI("pictureName: %s", pictureCfg->pictureName);
	}

	cJSON *fileName = cJSON_GetObjectItem(inputInfo, "fileName");
	if (NULL != fileName) {
		strcpy(pictureCfg->fileName, fileName->valuestring);
		LOGI("fileName: %s", pictureCfg->fileName);
	}

	cJSON *width = cJSON_GetObjectItem(inputInfo, "width");
	if (NULL != width) {
		pictureCfg->width = width->valueint;
		LOGI("width: %d", pictureCfg->width);
	}

	cJSON *height = cJSON_GetObjectItem(inputInfo, "height");
	if (NULL != height) {
		pictureCfg->height = height->valueint;
		LOGI("height: %d", pictureCfg->height);
	}

	cJSON *format = cJSON_GetObjectItem(inputInfo, "format");
	if (NULL != format) {
		pictureCfg->format = VsiVvbenchStrToInputFormat(format->valuestring);
		if (CAMDEV_INPUT_FMT_MAX <= pictureCfg->format || CAMDEV_INPUT_FMT_RAW8 > pictureCfg->format) {
			LOGE("%s: parse input picture format error", __func__);
			return -1;
		}
	}

	cJSON *layout = cJSON_GetObjectItem(inputInfo, "layout");
	if (NULL != layout) {
		pictureCfg->layout = VsiVvbenchStrToPicLayout(layout->valuestring);
		if (CAMDEV_RAW_PAT_MAX <= pictureCfg->layout || CAMDEV_RAW_RGB_PAT_RGGB > pictureCfg->layout) {
			LOGE("%s: parse input picture layout error", __func__);
			return -1;
		}
	}
	cJSON *calibXml = cJSON_GetObjectItem(inputInfo, "calibrationName");
	if (NULL != calibXml) {
		strcpy(pictureCfg->calibXml, calibXml->valuestring);
		LOGD("%s: calibXml %s", __func__, pictureCfg->calibXml);
	}

	LOGI("%s exit \n", __func__);
	return 0;
}

int VsiVvbenchParseModeInfo
(
	int workMode,
	cJSON *modeInfo,
	CamDeviceWorkConfig_t *workCfg
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	workCfg->workMode = (CamDeviceWorkMode_t)workMode;
	switch (workMode) {
		case 1: {
				workCfg->workMode = CAMDEV_WORK_MODE_STREAM;
				workCfg->modeCfg.stream.portId = 1;
				LOGI("workMode: Stream mode");
				break;
			}
		case 2: {
				LOGI("workMode: Mcm mode");
				workCfg->workMode = CAMDEV_WORK_MODE_MCM;
				cJSON *portId = cJSON_GetObjectItem(modeInfo, "portId");
				if (NULL == portId) {
					LOGE("mcm portId is not configured !");
				}
				else {
					workCfg->modeCfg.mcm.portId = portId->valueint;
					if (workCfg->modeCfg.mcm.portId) {
						LOGI("mcm portId: %d", workCfg->modeCfg.mcm.portId);
					}
				}

				cJSON *mcmOp = cJSON_GetObjectItem(modeInfo, "mcmOp");
				if (NULL == mcmOp) {
					LOGE("mcm mcmOp is not configured !");
				}
				else {
					workCfg->modeCfg.mcm.mcmOp = mcmOp->valueint;
					if (workCfg->modeCfg.mcm.mcmOp) {
						LOGI("mcm mcmOp: %d", workCfg->modeCfg.mcm.mcmOp);
					}
				}

				cJSON *mcmSel = cJSON_GetObjectItem(modeInfo, "mcmSel");
				if (NULL == mcmSel) {
					LOGW("mcm mcmSel is not configured !");
				}
				else {
					workCfg->modeCfg.mcm.mcmSel = mcmSel->valueint;
					if (workCfg->modeCfg.mcm.mcmSel) {
						LOGI("mcm mcmSel: %d", workCfg->modeCfg.mcm.mcmSel);
					}
				}
				break;
			}
		case 3: {
				workCfg->workMode = CAMDEV_WORK_MODE_RDMA;
				LOGI("workMode: Rdma mode");
				break;
			}
		default: {
				LOGI("workMode:NONE");
			}
	}

	cJSON *tileMode = cJSON_GetObjectItem(modeInfo, "tileMode");
	if (NULL == tileMode) {
		workCfg->tileCfg.tileOp = CAMDEV_TILE_OP_GENERAL;
	}
	else {
		workCfg->tileCfg.tileOp = tileMode->valueint;
		if (workCfg->tileCfg.tileOp) {
			LOGI("tileMode: %d", workCfg->tileCfg.tileOp);
		}
	}

	cJSON *tileJointMode = cJSON_GetObjectItem(modeInfo, "tileJointMode");
	if (NULL == tileJointMode) {
		workCfg->tileCfg.tileNum = CAMDEV_TILE_JOINT_INVALID;
	}
	else {
		workCfg->tileCfg.tileNum = tileJointMode->valueint;
		if (workCfg->tileCfg.tileNum) {
			LOGI("tileJointMode: %d", workCfg->tileCfg.tileNum);
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseSwSimuDefaultInfo
(
	VvbenchVvdev_t *caseCtx
)
{
	if (NULL == caseCtx || NULL == caseCtx->swSimuCfg.defaultJson) {
		LOGE("%s NULL pointer", __func__);
		return -1;
	}

	LOGI("%s enter \n", __func__);

	int result = 0;
	char *temp = "vvbcfg/load_image/Default.json";
	size_t spaceSize = sizeof(caseCtx->swSimuCfg.defaultJson) - 1;
	strncpy(caseCtx->swSimuCfg.defaultJson, temp, spaceSize);

	for (int32_t instanceId = 0; instanceId < caseCtx->totalInstance; ++instanceId) {
		if (caseCtx->instanceCfgCtx[instanceId].hCamCommon) {
			result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[instanceId].hCamCommon,
						       caseCtx->swSimuCfg.defaultJson);
			LOGI("%s VsiCamCommonParseFile %s \n", __func__, caseCtx->swSimuCfg.defaultJson);
			if (0 != result) {
				LOGE("Cam Common parse swSimu config file error, exit");
				return result;
			}
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseSwSimuInfo
(
	VvbenchVvdev_t *caseCtx
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	CamCommonContext_t *pCamCommonCtx = NULL;
	uint32_t instanceId = 0;
	size_t spaceSize = 0;
	char formatString[20] = {0};
	char inputPattern[20] = {0};
	char autoJson[FILE_LEN] = "vvbcfg/load_image/Auto/";
	char xml[FILE_LEN] = "vvbcfg/load_image/Auto/";
	char metaList[FILE_LEN] = "\0";

	if (NULL == caseCtx || NULL == caseCtx->swSimuCfg.swSimuCfgFile
	    || 0 == caseCtx->instanceCfgCtx[instanceId].hCamCommon) {
		LOGE("%s NULL pointer", __func__);
		return -1;
	}

	pCamCommonCtx = (CamCommonContext_t *)caseCtx->instanceCfgCtx[instanceId].hCamCommon;

	result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[instanceId].hCamCommon,
				       caseCtx->swSimuCfg.swSimuCfgFile);
	LOGI("%s VsiCamCommonParseFile %s \n", __func__, caseCtx->swSimuCfg.swSimuCfgFile);
	if (0 != result) {
		LOGE("Cam Common parse swSimu config file error, exit");
		return result;
	}

	result = VsiCamCommonQueryMetaDataInfo(caseCtx->instanceCfgCtx[instanceId].hCamCommon, metaList);
	if (0 != result) {
		LOGE("Cam Common query meta data info error, exit");
		return -1;
	}

	TDatabase3AInterface_2_t *p3AIFConfig = NULL;
	result = TDatabase_query(pCamCommonCtx->hDatabase, T_DATABASE_3A_INTERFACE_2, (void**)&p3AIFConfig);

	if (p3AIFConfig->enable) {
		caseCtx->swSimuCfg.autoCfg.enable = true;
		caseCtx->swSimuCfg.autoCfg.awbEnable = p3AIFConfig->awbEnable;
		caseCtx->swSimuCfg.autoCfg.cacEnable = p3AIFConfig->cacEnable;
		caseCtx->swSimuCfg.autoCfg.ccEnable = p3AIFConfig->ccEnable;
		caseCtx->swSimuCfg.autoCfg.lscEnable = p3AIFConfig->lscEnable;

		if (strlen(p3AIFConfig->xml) != 0) {
			spaceSize = sizeof(xml) - strlen(xml) + 1;
			if ((strlen(p3AIFConfig->xml) + strlen(autoJson)) >= FILE_LEN) {
				LOGE("xml length is out of range");
				return -1;
			}
			strncat(xml, p3AIFConfig->xml, spaceSize);

			strcpy(caseCtx->instanceCfgCtx[instanceId].pictureCfg.calibXml, xml);

		}

		if ((strlen(p3AIFConfig->autoTable) + strlen(autoJson)) >= FILE_LEN) {
			LOGE("xml length is out of range");
			return -1;
		}
		spaceSize = sizeof(autoJson) - strlen(autoJson) + 1;
		strncat(autoJson, p3AIFConfig->autoTable, spaceSize);


		result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[instanceId].hCamCommon,
					       autoJson);
		LOGI("%s VsiCamCommonParseFile %s \n", __func__, autoJson);
		if (0 != result) {
			LOGE("Cam Common parse auto JSON error, exit");
			return -1;
		}
	}

	TDatabaseMetaDriver_t *pDbConfig = NULL;
	result = TDatabase_query(pCamCommonCtx->hDatabase, T_DATABASE_META_DRIVER, (void**)&pDbConfig);

	strcpy(inputPattern, pDbConfig->input.bayerPattern);

	result = VsiCamCommonRgbirQueryConfig(caseCtx->instanceCfgCtx[instanceId].hCamCommon);
	if (0 != result) {
		LOGE("Cam Common query RGBIR config error, exit");
		return result;
	}
	if (pCamCommonCtx->rgbir.supported &&
	    pCamCommonCtx->rgbir.enable) {
		switch (pCamCommonCtx->rgbir.irBayerPattern) {
			case 0: {
					strcpy(inputPattern, "BGGIR");
					break;
				}
			case 1: {
					strcpy(inputPattern, "GRIRG");
					break;
				}
			case 2: {
					strcpy(inputPattern, "RGGIR");
					break;
				}
			case 3: {
					strcpy(inputPattern, "GBIRG");
					break;
				}
			case 4: {
					strcpy(inputPattern, "GIRRG");
					break;
				}
			case 5: {
					strcpy(inputPattern, "IRGGB");
					break;
				}
			case 6: {
					strcpy(inputPattern, "GIRBG");
					break;
				}
			case 7: {
					strcpy(inputPattern, "IRGGR");
					break;
				}
			case 8: {
					strcpy(inputPattern, "RGIRB");
					break;
				}
			case 9: {
					strcpy(inputPattern, "GRBIR");
					break;
				}
			case 10: {
					strcpy(inputPattern, "IRBRG");
					break;
				}
			case 11: {
					strcpy(inputPattern, "BIRGR");
					break;
				}
			case 12: {
					strcpy(inputPattern, "BGIRR");
					break;
				}
			case 13: {
					strcpy(inputPattern, "GBRIR");
					break;
				}
			case 14: {
					strcpy(inputPattern, "IRRBG");
					break;
				}
			case 15: {
					strcpy(inputPattern, "RIRGB");
					break;
				}
			default: {
					return -1;
				}
		}
	}
	//update load picture parameters
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.width = pDbConfig->input.width;
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.height = pDbConfig->input.height;
	if ((pDbConfig->input.frames) <=
	    (pDbConfig->input.startFrame)) {  // pDbConfig->input.frames now be treat as endFrame
		LOGE("%s error:endFrame must bigger than startFrame", __func__);
		return -1;
	}
	else {
		caseCtx->instanceCfgCtx[instanceId].pictureCfg.frameNum = pDbConfig->input.frames -
			(pDbConfig->input.startFrame);

#if defined(HAL_CMODEL) || defined(DUMP_IMAGE)
		caseCtx->frameIndex[instanceId] = pDbConfig->input.startFrame;
#endif
	}

	caseCtx->instanceCfgCtx[instanceId].pictureCfg.layout = VsiVvbenchStrToPicLayout(inputPattern);
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.loopCnt = 1;
	switch (pDbConfig->input.bit) {
		case 8: {
				strcpy(formatString, "RAW8");
				break;
			}
		case 10: {
				strcpy(formatString, "RAW10_MODE1");
				break;
			}
		case 12: {
				strcpy(formatString, "RAW12_MODE1");
				break;
			}
		case 14: {
				strcpy(formatString, "RAW14_MODE1");
				break;
			}
		case 16: {
				strcpy(formatString, "RAW16");
				break;
			}
		default: {
				return -1;
			}
	}

	//update mp/read path resolution
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].width = pDbConfig->input.width;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].height =
		pDbConfig->input.height;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].width =
		pDbConfig->input.width;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].height =
		pDbConfig->input.height;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].format =
		VsiVvbenchStrToInputFormat(formatString);
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].layout =
		VsiVvbenchStrToPicLayout(inputPattern);

	result = VsiCamCommonHdrQueryConfig(caseCtx->instanceCfgCtx[instanceId].hCamCommon);
	if (0 != result) {
		LOGE("Cam Common query HDR config error, exit");
		return result;
	}
	if (pCamCommonCtx->hdr.supported) {
		char hdrFormatString[20] = {0};
		uint16_t stitchMode = 0;

		stitchMode = pCamCommonCtx->hdr.sensorType;
		if (stitchMode > 10) {
			stitchMode = 100;
		}

		switch (stitchMode) {
			case 0:
			case 1:
			case 2: {
					strcpy(hdrFormatString, "3DOL");
					break;
				}
			case 3:
			case 4:
			case 5:
			case 6: {
					strcpy(hdrFormatString, "2DOL");
					break;
				}
			case 7: {
					strcpy(hdrFormatString, "4DOL");
					break;
				}
			case 8: {
					strcpy(hdrFormatString, "3DOL");
					stitchMode = 1;
					break;
				}
			case 9: {
					strcpy(hdrFormatString, "2DOL");
					stitchMode = 6;
					break;
				}
			case 10: {
					strcpy(hdrFormatString, "2DOL");
					stitchMode = 3;
					break;
				}
			case 100: {
					strcpy(hdrFormatString, "RAW12");
					stitchMode = 0;
					break;
				}
			default: {
					return -1;
				}
		}
		if (pCamCommonCtx->hdr.enable) {
			caseCtx->instanceCfgCtx[instanceId].workCfg.workMode = CAMDEV_WORK_MODE_STREAM;
			caseCtx->instanceCfgCtx[instanceId].workCfg.modeCfg.stream.portId = CAMDEV_MCM_PORT_0;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].pathEnable = false;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].pathEnable = true;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].path =
				CAMDEV_BUFCHAIN_RETIMING;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].width =
				pDbConfig->input.width;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].height =
				pDbConfig->input.height;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].bufferNumber = 4;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].format =
				VsiVvbenchStrToInputFormat(hdrFormatString);
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].layout =
				VsiVvbenchStrToPicLayout(inputPattern);
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].stitchMode =
				(uint32_t)stitchMode;
		}
		else {
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].pathEnable = false;
		}
	}

	osFile *swSimuList = NULL;
	char szBuffer[FILE_LEN];
	swSimuList = osFopen(caseCtx->instanceCfgCtx[instanceId].pictureCfg.pictureName, "w");
	if (NULL == swSimuList) {
		LOGE("VsiVvbenchParseSwSimuInfo error, open swSimuList fail: %s,  exit",
		     caseCtx->instanceCfgCtx[instanceId].pictureCfg.pictureName);
		return -1;
	}

	osFseek(swSimuList, 0, SEEK_SET);

	for (int index = pDbConfig->input.startFrame; index < pDbConfig->input.frames ; index++) {
		for (int pathId = 0; pathId < sizeof(pDbConfig->input.path) / sizeof(pDbConfig->input.path[0]) ;
		     pathId++) {
			if (strcmp(pDbConfig->input.path[pathId], "%d.raw") == 0
			    || strcmp(pDbConfig->input.path[pathId], "") == 0) {
				continue;
			}
			else if (strstr(pDbConfig->input.path[pathId], "%d") == NULL) {
				osFputs(pDbConfig->input.path[pathId], swSimuList);
				osFputs("\n", swSimuList);
			}
			else {
				snprintf(szBuffer, FILE_LEN, pDbConfig->input.path[pathId], index);
				osFputs(szBuffer, swSimuList);
				osFputs("\n", swSimuList);
			}
		}
	}

	osFclose(swSimuList);
	swSimuList = NULL;
	// parse and overwrite the output config
	if (0 < strlen(pDbConfig->output.format)) {
		LOGI("%s append sw json output config, overwrite main path, Format:%s, dataBits:%u, yuvOrder:%u\n",
		     __func__, pDbConfig->output.format, pDbConfig->output.dataBits, pDbConfig->output.yuvOrder);
		CamDevicePipePixOutFmt_t bufferFormat = VsiVvbenchStrToBuffFormat(pDbConfig->output.format);
		if (CAMDEV_PIX_FMT_MAX <= bufferFormat) {
			LOGE("%s Format out of range, overwrite fail", __func__);
			return -1;
		}
		caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].format = bufferFormat;
		caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].path = CAMDEV_BUFCHAIN_MP;
		caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].pathEnable = 1;
		if (CAMDEV_PIX_FMT_RGB888_ALIGNED_MODE0 == bufferFormat
		    || CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0 == bufferFormat) {
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].alpha =
				pDbConfig->output.alpha;
		}
		if (0 < pDbConfig->output.dataBits) {
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].dataBits =
				pDbConfig->output.dataBits;
		}
		else {
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].dataBits = 8;
		}
		caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].yuvOrder =
			pDbConfig->output.yuvOrder;
	}
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseSwSimuInfoByToolOutput
(
	VvbenchVvdev_t *caseCtx,
	const char *rawName
)
{
	LOGI("%s enter \n", __func__);

	if (NULL == caseCtx || NULL == caseCtx->swSimuCfg.fineTuneJson || NULL == rawName) {
		LOGE("%s NULL pointer", __func__);
		return -1;
	}

	int result = 0;
	uint32_t instanceId = 0;
	CamCommonContext_t *pCamCommonCtx = NULL;
	if (0 == caseCtx->instanceCfgCtx[instanceId].hCamCommon) {
		LOGE("%s: invalid cam common handle", __func__);
		return -1;
	}
	pCamCommonCtx = (CamCommonContext_t *)caseCtx->instanceCfgCtx[instanceId].hCamCommon;

	VvbenchRawMetaInfo_t *pRawMetaInfo = mm_malloc(sizeof(VvbenchRawMetaInfo_t));
	if (NULL == pRawMetaInfo) {
		LOGE("[vvbench]--%s:malloc VvbenchRawMetaInfo_t failed!", __func__);
		return RET_FAILURE;
	}
	MEMSET(pRawMetaInfo, 0, sizeof(VvbenchRawMetaInfo_t));
	result = VsiVvbenchStrToRawMetaInfo(rawName, pRawMetaInfo);
	if (0 != result) {
		LOGE("get meta info error, exit");
		return result;
	}

	result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[instanceId].hCamCommon,
				       caseCtx->swSimuCfg.fineTuneJson);
	LOGI("%s VsiCamCommonParseFile %s \n", __func__, caseCtx->swSimuCfg.fineTuneJson);
	if (0 != result) {
		LOGE("Cam Common parse swSimu config file error, exit");
		return result;
	}

	if (caseCtx->fineTuneMode) {
		caseCtx->swSimuCfg.autoCfg.enable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.awbEnable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.cacEnable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.ccEnable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.lscEnable = caseCtx->fineTuneMode;
	}

	int inputLayout = pRawMetaInfo->layout;

	result = VsiCamCommonRgbirQueryConfig(caseCtx->instanceCfgCtx[instanceId].hCamCommon);
	if (0 != result) {
		LOGE("Cam Common query RGBIR config error, exit");
		return result;
	}
	if (pCamCommonCtx->rgbir.supported &&
	    pCamCommonCtx->rgbir.enable) {
		char inputPattern[20] = {0};
		switch (pCamCommonCtx->rgbir.irBayerPattern) {
			case 0: {
					strcpy(inputPattern, "BGGIR");
					break;
				}
			case 1: {
					strcpy(inputPattern, "GRIRG");
					break;
				}
			case 2: {
					strcpy(inputPattern, "RGGIR");
					break;
				}
			case 3: {
					strcpy(inputPattern, "GBIRG");
					break;
				}
			case 4: {
					strcpy(inputPattern, "GIRRG");
					break;
				}
			case 5: {
					strcpy(inputPattern, "IRGGB");
					break;
				}
			case 6: {
					strcpy(inputPattern, "GIRBG");
					break;
				}
			case 7: {
					strcpy(inputPattern, "IRGGR");
					break;
				}
			case 8: {
					strcpy(inputPattern, "RGIRB");
					break;
				}
			case 9: {
					strcpy(inputPattern, "GRBIR");
					break;
				}
			case 10: {
					strcpy(inputPattern, "IRBRG");
					break;
				}
			case 11: {
					strcpy(inputPattern, "BIRGR");
					break;
				}
			case 12: {
					strcpy(inputPattern, "BGIRR");
					break;
				}
			case 13: {
					strcpy(inputPattern, "GBRIR");
					break;
				}
			case 14: {
					strcpy(inputPattern, "IRRBG");
					break;
				}
			case 15: {
					strcpy(inputPattern, "RIRGB");
					break;
				}
			default: {
					return -1;
				}
		}
		inputLayout = VsiVvbenchStrToPicLayout(inputPattern);
	}
	//update load picture parameters
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.width = pRawMetaInfo->width;
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.height = pRawMetaInfo->height;
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.frameNum = pRawMetaInfo->frameNum;
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.layout = inputLayout;
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.loopCnt = 1;
	caseCtx->instanceCfgCtx[instanceId].pictureCfg.format = pRawMetaInfo->imgFormat;

	//update mp/read path resolution
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].width = pRawMetaInfo->width;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_MP].height = pRawMetaInfo->height;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].width = pRawMetaInfo->width;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].height =
		pRawMetaInfo->height;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].format =
		pRawMetaInfo->imgFormat;
	caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].layout = inputLayout;

	result = VsiCamCommonHdrQueryConfig(caseCtx->instanceCfgCtx[instanceId].hCamCommon);
	if (0 != result) {
		LOGE("Cam Common query HDR config error, exit");
		return result;
	}
	if (pCamCommonCtx->hdr.supported) {
		char hdrFormatString[20] = {0};
		uint16_t stitchMode = 0;

		stitchMode = pCamCommonCtx->hdr.sensorType;
		if (stitchMode > 10) {
			stitchMode = 100;
		}

		switch (stitchMode) {
			case 0:
			case 1:
			case 2: {
					strcpy(hdrFormatString, "3DOL");
					break;
				}
			case 3:
			case 4:
			case 5:
			case 6: {
					strcpy(hdrFormatString, "2DOL");
					break;
				}
			case 7: {
					strcpy(hdrFormatString, "4DOL");
					break;
				}
			case 8: {
					strcpy(hdrFormatString, "3DOL");
					stitchMode = 1;
					break;
				}
			case 9: {
					strcpy(hdrFormatString, "2DOL");
					stitchMode = 6;
					break;
				}
			case 10: {
					strcpy(hdrFormatString, "2DOL");
					stitchMode = 3;
					break;
				}
			case 100: {
					strcpy(hdrFormatString, "RAW12");
					stitchMode = 0;
					break;
				}
			default: {
					return -1;
				}
		}

		if (pCamCommonCtx->hdr.enable) {
			caseCtx->instanceCfgCtx[instanceId].workCfg.workMode = CAMDEV_WORK_MODE_STREAM;
			caseCtx->instanceCfgCtx[instanceId].workCfg.modeCfg.stream.portId = CAMDEV_MCM_PORT_0;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RDMA].pathEnable = false;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].pathEnable = true;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].path =
				CAMDEV_BUFCHAIN_RETIMING;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].width =
				pRawMetaInfo->width;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].height =
				pRawMetaInfo->height;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].bufferNumber = 4;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].format =
				VsiVvbenchStrToInputFormat(hdrFormatString);
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].layout = inputLayout;
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].stitchMode =
				(uint32_t)stitchMode;
		}
		else {
			caseCtx->instanceCfgCtx[instanceId].instancePath[CAMDEV_BUFCHAIN_RETIMING].pathEnable = false;
		}
	}

	osFile *pFSwSimuList = NULL;

	pFSwSimuList = osFopen(caseCtx->instanceCfgCtx[instanceId].pictureCfg.pictureName, "w");
	if (NULL == pFSwSimuList) {
		LOGE("VsiVvbenchParseSwSimuInfoByToolOutput error, open swSimuList fail: %s,  exit",
		     caseCtx->instanceCfgCtx[instanceId].pictureCfg.pictureName);
		mm_free(pRawMetaInfo);
		pRawMetaInfo = NULL;
		return -1;
	}
	osFseek(pFSwSimuList, 0, SEEK_SET);
	osFputs(rawName, pFSwSimuList);
	osFclose(pFSwSimuList);
	pFSwSimuList = NULL;
	mm_free(pRawMetaInfo);
	pRawMetaInfo = NULL;
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseSubSystemInfo
(
	VvbenchVvdev_t *caseCtx,
	uint32_t instanceId
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (caseCtx == NULL) {
		LOGE("%s NULL pointer", __func__);
		return -1;
	}

	if (caseCtx->instanceCfgCtx[instanceId].vi200Cfg.enable &&
	    caseCtx->instanceCfgCtx[instanceId].dewarpCfg.enable) {
		caseCtx->systemId.vi200Id = 0;
		caseCtx->systemId.ispId = 1;
		caseCtx->systemId.dewarpId = 2;
	}
	else if (caseCtx->instanceCfgCtx[instanceId].vi200Cfg.enable) {
		caseCtx->systemId.vi200Id = 0;
		caseCtx->systemId.ispId = 1;
	}
	else if (caseCtx->instanceCfgCtx[instanceId].dewarpCfg.enable) {
		caseCtx->systemId.ispId = 0;
		caseCtx->systemId.dewarpId = 1;
	}

	LOGI("%s exit \n", __func__);
	return result;
}


int VsiVvbenchParseSubmoduleInfo
(
	cJSON *instanceCfg,
	VvbenchVvdev_t *caseCtx,
	uint32_t instanceId
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == instanceCfg && caseCtx == NULL) {
		LOGE("%s NULL pointer", __func__);
		return -1;
	}

	result = VsiVvbenchInitModuleConfig(caseCtx, instanceId);
	if (0 != result) {
		LOGE("VsiVvbenchInitModuleConfig error");
	}

	cJSON *aeInfo = cJSON_GetObjectItem(instanceCfg, "aeInfo");
	if (aeInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseAeInfo(aeInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.ae);
		if (0 != result) {
			LOGE("VsiVvbenchParseAeInfo parser error");
		}
	}

	cJSON *awbInfo = cJSON_GetObjectItem(instanceCfg, "awbInfo");
	if (awbInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseAwbInfo(awbInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.awb);
		if (0 != result) {
			LOGE("VsiVvbenchParseAwbInfo parser error");
		}
	}

	cJSON *afInfo = cJSON_GetObjectItem(instanceCfg, "afInfo");
	if (afInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseAfInfo(afInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.af);
		if (0 != result) {
			LOGE("VsiVvbenchParseAfInfo parser error");
		}
	}

	cJSON *wbInfo = cJSON_GetObjectItem(instanceCfg, "wbInfo");
	if (wbInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseWbInfo(wbInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.wb);
		if (0 != result) {
			LOGE("VsiVvbenchParseWbInfo parser error");
		}
	}

	cJSON *wdrInfo = cJSON_GetObjectItem(instanceCfg, "wdrInfo");
	if (wdrInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseWdrInfo(wdrInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.wdr);
		if (0 != result) {
			LOGE("VsiVvbenchParseWdrInfo parser error");
		}
	}

	cJSON *hdrInfo = cJSON_GetObjectItem(instanceCfg, "hdrInfo");
	if (hdrInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseHdrInfo(hdrInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.hdr);
		if (0 != result) {
			LOGE("VsiVvbenchParseHdrInfo parser error");
		}
	}

	cJSON *hist64Info = cJSON_GetObjectItem(instanceCfg, "hist64Info");
	if (hist64Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseHist64Info(hist64Info,
						   &caseCtx->instanceCfgCtx[instanceId].moduleCfg.hist64);
		if (0 != result) {
			LOGE("VsiVvbenchParseHist64Info parser error");
		}
	}

	cJSON *hist256Info = cJSON_GetObjectItem(instanceCfg, "hist256Info");
	if (hist256Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseHist256Info(hist256Info,
						    &caseCtx->instanceCfgCtx[instanceId].moduleCfg.hist256);
		if (0 != result) {
			LOGE("VsiVvbenchParseHist256Info parser error");
		}
	}

	cJSON *dnr2Info = cJSON_GetObjectItem(instanceCfg, "2dnrInfo");
	if (dnr2Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParse2dnrInfo(dnr2Info, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.dnr2);
		if (0 != result) {
			LOGE("VsiVvbenchParse2dnrInfo parser error");
		}
	}

	cJSON *dnr3Info = cJSON_GetObjectItem(instanceCfg, "3dnrInfo");
	if (dnr3Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParse3dnrInfo(dnr3Info, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.dnr3);
		if (0 != result) {
			LOGE("VsiVvbenchParse3dnrInfo parser error");
		}
	}

	cJSON *blsInfo = cJSON_GetObjectItem(instanceCfg, "blsInfo");
	if (blsInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseBlsInfo(blsInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.bls);
		if (0 != result) {
			LOGE("VsiVvbenchParseBlsInfo parser error");
		}
	}

	cJSON *lscInfo = cJSON_GetObjectItem(instanceCfg, "lscInfo");
	if (lscInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseLscInfo(lscInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.lsc);
		if (0 != result) {
			LOGE("VsiVvbenchParseLscInfo parser error");
		}
	}

	cJSON *gcInfo = cJSON_GetObjectItem(instanceCfg, "gcInfo");
	if (gcInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseGcInfo(gcInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.gc);
		if (0 != result) {
			LOGE("VvbenchParseGcInfo parser error");
		}
	}

	cJSON *rgbirInfo = cJSON_GetObjectItem(instanceCfg, "rgbirInfo");
	if (rgbirInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseRgbirInfo(rgbirInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.rgbir);
		if (0 != result) {
			LOGE("VsiVvbenchParseRgbirInfo parser error");
		}
	}

	cJSON *eeInfo = cJSON_GetObjectItem(instanceCfg, "eeInfo");
	if (eeInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseEeInfo(eeInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.ee);
		if (0 != result) {
			LOGE("VsiVvbenchParseEeInfo parser error");
		}
	}

	cJSON *geInfo = cJSON_GetObjectItem(instanceCfg, "geInfo");
	if (geInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseGeInfo(geInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.ge);
		if (0 != result) {
			LOGE("VsiVvbenchParseGeInfo parser error");
		}
	}

	cJSON *gtmInfo = cJSON_GetObjectItem(instanceCfg, "gtmInfo");
	if (gtmInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseGtmInfo(gtmInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.gtm);
		if (0 != result) {
			LOGE("VsiVvbenchParseGtmInfo parser error");
		}
	}

	cJSON *dpccInfo = cJSON_GetObjectItem(instanceCfg, "dpccInfo");
	if (dpccInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseDpccInfo(dpccInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.dpcc);
		if (0 != result) {
			LOGE("VsiVvbenchParseDpccInfo parser error");
		}
	}

	cJSON *dgInfo = cJSON_GetObjectItem(instanceCfg, "dgInfo");
	if (dgInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseDgInfo(dgInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.dg);
		if (0 != result) {
			LOGE("VsiVvbenchParseDgInfo parser error");
		}
	}

	cJSON *ccmInfo = cJSON_GetObjectItem(instanceCfg, "ccmInfo");
	if (ccmInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseCcmInfo(ccmInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.ccm);
		if (0 != result) {
			LOGE("VsiVvbenchParseCcmInfo parser error");
		}
	}

	cJSON *cpdInfo = cJSON_GetObjectItem(instanceCfg, "cpdInfo");
	if (cpdInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseCpdInfo(cpdInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.cpd);
		if (0 != result) {
			LOGE("VsiVvbenchParseCpdInfo parser error");
		}
	}

	cJSON *cprocInfo = cJSON_GetObjectItem(instanceCfg, "cprocInfo");
	if (cprocInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseCprocInfo(cprocInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.cproc);
		if (0 != result) {
			LOGE("VsiVvbenchParseCprocInfo parser error");
		}
	}

	cJSON *dmscInfo = cJSON_GetObjectItem(instanceCfg, "dmscInfo");
	if (dmscInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseDmscInfo(dmscInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.dmsc);
		if (0 != result) {
			LOGE("VsiVvbenchParseDmscInfo parser error");
		}
	}

	cJSON *cnrInfo = cJSON_GetObjectItem(instanceCfg, "cnrInfo");
	if (cnrInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseCnrInfo(cnrInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.cnr);
		if (0 != result) {
			LOGE("VsiVvbenchParseCnrInfo parser error");
		}
	}


	cJSON *lut3dInfo = cJSON_GetObjectItem(instanceCfg, "lut3dInfo");
	if (lut3dInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseLut3dInfo(lut3dInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.lut3d);
		if (0 != result) {
			LOGE("VsiVvbenchParseLut3dInfo parser error");
		}
	}

	cJSON *pdafInfo = cJSON_GetObjectItem(instanceCfg, "pdafInfo");
	if (pdafInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParsePdafInfo(pdafInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.pdaf);
		if (0 != result) {
			LOGE("VsiVvbenchParsePdafInfo parser error");
		}
	}

	cJSON *expv2Info = cJSON_GetObjectItem(instanceCfg, "expv2Info");
	if (expv2Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseExpv2Info(expv2Info, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.expv2);
		if (0 != result) {
			LOGE("VsiVvbenchParseExpv2Info parser error");
		}
	}

	cJSON *expv3Info = cJSON_GetObjectItem(instanceCfg, "expv3Info");
	if (expv3Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseExpv3Info(expv3Info, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.expv3);
		if (0 != result) {
			LOGE("VsiVvbenchParseExpv3Info parser error");
		}
	}

	cJSON *afmInfo = cJSON_GetObjectItem(instanceCfg, "afmInfo");
	if (afmInfo && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseAfmInfo(afmInfo, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.afm);
		if (0 != result) {
			LOGE("VsiVvbenchParseAfmInfo parser error");
		}
	}

	cJSON *afmv3Info = cJSON_GetObjectItem(instanceCfg, "afmv3Info");
	if (afmv3Info && (!caseCtx->swSimuCfg.enable)) {
		result = VsiVvbenchParseAfmv3Info(afmv3Info, &caseCtx->instanceCfgCtx[instanceId].moduleCfg.afmv3);
		if (0 != result) {
			LOGE("VsiVvbenchParseAfmv3Info parser error");
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchParseDewarpInfo
(
	cJSON *dewarpInfo,
	VvbenchInstanceDewarpCfg_t *dewarpCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == dewarpCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "enable")) {
		dewarpCtrl->enable = cJSON_GetObjectItem(dewarpInfo, "enable")->valueint;
	}
	if (!dewarpCtrl->enable) {
		LOGI("dewarp feature not enable!");
		return result;
	}
	else {
		LOGI("dewarp feature enable!");
	}

#ifdef DWE_VERSION
	cJSON *map = cJSON_GetObjectItem(dewarpInfo, "map");
	if (map) {
		cJSON *dewarpMode = cJSON_GetObjectItem(map, "dewarpMode");
		if (dewarpMode) {
			char dewarpModeName[FILE_LEN];
			strcpy(dewarpModeName, dewarpMode->valuestring);

			if (0 == strcmp(dewarpModeName, "LENS_CORRECTION")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "LENS_CORRECTION");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_LENS_DISTORTION_CORRECTION;
			}
			else if (0 == strcmp(dewarpModeName, "FISHEYE_EXPAND")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "FISHEYE_EXPAND");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_FISHEYE_EXPAND;
			}
			else if (0 == strcmp(dewarpModeName, "SPLIT_SCREEN")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "SPLIT_SCREEN");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_SPLIT_SCREEN;
			}
			else if (0 == strcmp(dewarpModeName, "FISHEYE_DEWARP")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "FISHEYE_DEWARP");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_FISHEYE_DEWARP;
			}
			else if (0 == strcmp(dewarpModeName, "PERSPECTIVE")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "PERSPECTIVE");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_PERSPECTIVE;
			}
			else if (0 == strcmp(dewarpModeName, "FOV")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "FOV");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_FOV;
			}
			else if (0 == strcmp(dewarpModeName, "USER_COORDINATE")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "USER_COORDINATE");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_USER_COORDINATE;
			}
			else if (0 == strcmp(dewarpModeName, "OMNI_FISHEYE_DEWARP")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "OMNI_FISHEYE_DEWARP");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_OMNI_FISHEYE_DEWARP;
			}
			else if (0 == strcmp(dewarpModeName, "USER_STITCH")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "USER_STITCH");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_USER_STITCH;
			}
			else if (0 == strcmp(dewarpModeName, "ROTATION")) {
				LOGI("dewarp mode %s equal to %s", dewarpModeName, "ROTATION");
				dewarpCtrl->dewarpMode = DEWARP_MODEL_ROTATION;
			}
			else {
				LOGE("no dewarp mode, please input dewarp mode\n");
				return -1;
			}
		}

		cJSON *mapSelect = cJSON_GetObjectItem(map, "mapSelect");
		if (mapSelect) {
			char mapSelectName[FILE_LEN];
			strcpy(mapSelectName, mapSelect->valuestring);

			if (0 == strcmp(mapSelectName, "16x16")) {
				LOGI("dewarp map select %s equal to %s", mapSelectName, "16x16");
				dewarpCtrl->mapSelect = 0;
			}
			else if (0 == strcmp(mapSelectName, "1x1")) {
				LOGI("dewarp map select %s equal to %s", mapSelectName, "1x1");
				dewarpCtrl->mapSelect = 1;
			}
			else if (0 == strcmp(mapSelectName, "4x4")) {
				LOGI("dewarp map select %s equal to %s", mapSelectName, "4x4");
				dewarpCtrl->mapSelect = 2;
			}
			else if (0 == strcmp(mapSelectName, "64x64")) {
				LOGI("dewarp map select %s equal to %s", mapSelectName, "64x64");
				dewarpCtrl->mapSelect = 3;
			}
			else {
				LOGE("no dewarp map select, please input dewarp map select\n");
				return -1;
			}
		}

		if (NULL != cJSON_GetObjectItem(map, "tempWidth")) {
			dewarpCtrl->tempWidth = cJSON_GetObjectItem(map, "tempWidth")->valueint;
			LOGI("dewarp tempWidth: %d", dewarpCtrl->tempWidth);
		}

		if (NULL != cJSON_GetObjectItem(map, "tempHeight")) {
			dewarpCtrl->tempHeight = cJSON_GetObjectItem(map, "tempHeight")->valueint;
			LOGI("dewarp tempHeight: %d", dewarpCtrl->tempHeight);
		}
	}

	cJSON *cameraMatrix = cJSON_GetObjectItem(dewarpInfo, "cameraMatrix");
	if (cameraMatrix == NULL) {
		LOGE("cameraMatrix is not an object or not found!");
		cJSON_Delete(cameraMatrix);
	}
	else {
		if (cJSON_IsArray(cameraMatrix)) {
			for (uint32_t i = 0; i < CAMERA_MATRIX_NUMBER; i++) {
				dewarpCtrl->cameraMatrix[i] = cJSON_GetArrayItem(cameraMatrix, i)->valuedouble;
				LOGI("dewarp cameraMatrix[%d]: %.2f", i, dewarpCtrl->cameraMatrix[i]);
			}
		}
	}

	cJSON *distortionCoeff = cJSON_GetObjectItem(dewarpInfo, "distortionCoeff");
	if (distortionCoeff == NULL) {
		LOGE("distortionCoeff is not an object or not found!");
		cJSON_Delete(distortionCoeff);
	}
	else {
		if (cJSON_IsArray(distortionCoeff)) {
			for (uint32_t i = 0; i < DISTORTION_COEFF_NUMBER; i++) {
				dewarpCtrl->distortionCoeff[i] = cJSON_GetArrayItem(distortionCoeff, i)->valuedouble;
				LOGI("dewarp distortionCoeff[%d]: %.2f", i, dewarpCtrl->distortionCoeff[i]);
			}
		}
	}
#endif

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "hflip")) {
		dewarpCtrl->hflip = cJSON_GetObjectItem(dewarpInfo, "hflip")->valueint;
		LOGI("dewarp hflip: %d", dewarpCtrl->hflip);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "vflip")) {
		dewarpCtrl->vflip = cJSON_GetObjectItem(dewarpInfo, "vflip")->valueint;
		LOGI("dewarp vflip: %d", dewarpCtrl->vflip);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "bypass")) {
		dewarpCtrl->bypass = cJSON_GetObjectItem(dewarpInfo, "bypass")->valueint;
		LOGI("dewarp bypass: %d", dewarpCtrl->bypass);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "inputEnable")) {
		dewarpCtrl->inputEnable = cJSON_GetObjectItem(dewarpInfo, "inputEnable")->valueint;
		LOGI("dewarp inputEnable: %d", dewarpCtrl->inputEnable);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "inputPath")) {
		dewarpCtrl->inputPath = cJSON_GetObjectItem(dewarpInfo, "inputPath")->valueint;
		LOGI("dewarp inputPath: %d", dewarpCtrl->inputPath);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "inputWidth")) {
		dewarpCtrl->inputWidth = cJSON_GetObjectItem(dewarpInfo, "inputWidth")->valueint;
		LOGI("dewarp inputWidth: %d", dewarpCtrl->inputWidth);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "inputHeight")) {
		dewarpCtrl->inputHeight = cJSON_GetObjectItem(dewarpInfo, "inputHeight")->valueint;
		LOGI("dewarp inputHeight: %d", dewarpCtrl->inputHeight);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "inputFormat")) {
		dewarpCtrl->inputFormat = cJSON_GetObjectItem(dewarpInfo, "inputFormat")->valueint;
		LOGI("dewarp inputFormat: %d", dewarpCtrl->inputFormat);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "inputBit")) {
		dewarpCtrl->inputBit = cJSON_GetObjectItem(dewarpInfo, "inputBit")->valueint;
		LOGI("dewarp inputBit: %d", dewarpCtrl->inputBit);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputEnable")) {
		dewarpCtrl->outputEnable = cJSON_GetObjectItem(dewarpInfo, "outputEnable")->valueint;
		LOGI("dewarp outputEnable: %d", dewarpCtrl->outputEnable);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputPath")) {
		dewarpCtrl->outputPath = cJSON_GetObjectItem(dewarpInfo, "outputPath")->valueint;
		LOGI("dewarp outputPath: %d", dewarpCtrl->outputPath);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputFormat")) {
		dewarpCtrl->outputFormat = cJSON_GetObjectItem(dewarpInfo, "outputFormat")->valueint;
		LOGI("dewarp outputFormat: %d", dewarpCtrl->outputFormat);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputWidth")) {
		dewarpCtrl->outputWidth = cJSON_GetObjectItem(dewarpInfo, "outputWidth")->valueint;
		LOGI("dewarp outputWidth: %d", dewarpCtrl->outputWidth);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputHeight")) {
		dewarpCtrl->outputHeight = cJSON_GetObjectItem(dewarpInfo, "outputHeight")->valueint;
		LOGI("dewarp outputHeight: %d", dewarpCtrl->outputHeight);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputNumber")) {
		dewarpCtrl->outputNumber = cJSON_GetObjectItem(dewarpInfo, "outputNumber")->valueint;
		LOGI("dewarp outputNumber: %d", dewarpCtrl->outputNumber);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputFormat")) {
		dewarpCtrl->outputFormat = cJSON_GetObjectItem(dewarpInfo, "outputFormat")->valueint;
		LOGI("dewarp outputFormat: %d", dewarpCtrl->outputFormat);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "outputBit")) {
		dewarpCtrl->outputBit = cJSON_GetObjectItem(dewarpInfo, "outputBit")->valueint;
		LOGI("dewarp outputBit: %d", dewarpCtrl->outputBit);
	}

	if (NULL != cJSON_GetObjectItem(dewarpInfo, "scaleFactor")) {
		dewarpCtrl->scaleFactor = cJSON_GetObjectItem(dewarpInfo, "scaleFactor")->valueint;
		LOGI("dewarp scaleFactor: %d", dewarpCtrl->scaleFactor);
	}

	return result;

}

int VsiVvbenchInitModuleConfig
(
	VvbenchVvdev_t *caseCtx,
	uint32_t instanceId
)
{
	int result = 0;
	VvbenchSubModule_t *index = &caseCtx->instanceCfgCtx[instanceId].moduleCfg;
	LOGI("%s enter \n", __func__);

	if (caseCtx == NULL) {
		LOGE("%s NULL pointer", __func__);
		return -1;
	}

	for (uint32_t i = 0; i < sizeof(VvbenchSubModule_t) / sizeof(VvbenchModuleCfg_t); ++i) {
		switch (i) {
			case 0://awb
			case 1://ae
			case 2://af
				*((VvbenchModuleCfg_t*)index + i) = (VvbenchModuleCfg_t) {false, false, false, false, false, CAMDEV_CFG_MODE_AUTO};
				break;
			default:
				*((VvbenchModuleCfg_t*)index + i) = (VvbenchModuleCfg_t) {false, false, false, false, false, CAMDEV_CFG_MODE_MANUAL};
				break;
		}
	}

	return result;
}
