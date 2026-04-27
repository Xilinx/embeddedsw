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


#include <stdbool.h>
#define LOGTAG "DEV"
#include "vvbase.h"
#include "memory_manager.h"
#include "vlog.h"
#include "vvdevice.h"
#include "vvbparser.h"
#include "vvpath.h"
#include "submodule_def.h"
#include "cam_common_calib_api.h"
#include "cam_common_submodules.h"
#include "sensor_drv.h"
#include "vmix_hdmi_bridge.h"
#include <stdio.h>
#include <oslayer.h>
#include <builtins.h>
#include <isi_iss.h>
#include "hal_i2c.h"
#include "xvisp_ss.h"
extern int tuning_json;

/* Forward declarations for types from libvisp.a */
typedef struct {
	bool_t calibrationLoaded;
	bool_t imageLoadMetaEn;
	CamDeviceSensorModeInfo_t sensorMode;
	CamDeviceInputType_t inputType;
	void *hDatabase;                    // TDatabaseHandle_t (uintptr_t)
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
	void *pRegisters;                   // TDatabaseRegister_t*
	size_t registerNumber;
} TDatabaseDump_t;

typedef struct {
	char *address;
	char *value;
	int isReadAction;
} TDatabaseRegister_t;

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
	const char *filename;
	const unsigned char *data;
	const unsigned int size;
} EmbeddedJson;

/* Constants from t_database.h */
#define T_DATABASE_DUMP "DUMP"
#define T_DATABASE_META_DRIVER "IOControl"

/* External functions from libvisp.a */
extern int TDatabase_query(void* hDatabase, char const *pCategory, void** result);
extern const EmbeddedJson *find_image_data(const char* name);

extern XVisp_Ss VispSsInst[XPAR_XVISP_SS_NUM_INSTANCES];
extern int image_len;
extern int custom_json;

#ifdef PORTING_25
	//#include <unistd.h>  //usleep conflicting with local bsp and /proj/xbuilds
#endif
#if SD
	#include "xsdps.h"		/* SD device driver */
#endif
#include "ff.h"
#include "iba.h"
#include "oba.h"
#include "cam_device_app.h"
#include "vvbench_gwdr_api.h"
#include "sensor_cmd.h"

VvbenchInstance_t *pBenchInstance = NULL;
VvbenchVvdev_t *caseContext = NULL;
int instanceNumber = 0;
bool_t isVvdeviceReleased = false;

#define START 1
#define STOP 0
#define RDMA_INPUT_PIC_NUM 1
#define ALIGN_16 0x10
int showFrameNum = 0;
BufIdentity outBuf[MAX_CAM_NUM][CAMDEV_BUFCHAIN_RAW];
extern int ATM_ENABLE;
extern int init_lilo;
extern int userVirtualChannelId;
int ISP_ID;
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	extern int noOfFrmbufInstances;
	extern struct aligned_buf Frame_Array_p[XPAR_XV_FRMBUF_WR_NUM_INSTANCES][3];
#endif

volatile uint32_t apuBufferInform[CAMDEV_VIRTUAL_ID_MAX *
				   CAMDEV_HARDWARE_ID_MAX][CAMDEV_PIPE_OUTPATH_RAW + 1] = {0};

#ifdef WITH_FLEXA
	#include <fcntl.h>
	#include <sys/types.h>
	#include <buffer_management/flexamem.h>
#endif
#define ALIGN_16BYTE(width)         (((width) + (0xF)) & (~(0xF)))

uint64_t start, now;

// Timer variables for VsiCamDeviceDeQueBuffer counter (every second)
int dequeue_call_count = 0;

void VsiVvdeviceDelay
(
	int delayInterval
)
{
	sleep(delayInterval);
}
int sensor_stream_flag = -1;
int memory_out_flag = -1, fbwr_flag = -1;
// enum outputformat {
//	RGB888_8bit,
//	YUV420_8bit,
//	YUV422_8bit,
//	RGB888_10bit,
//	YUV420_10bit,
//	YUV422_10bit,
//	YUV444,
//	YUV400_8bit,
//	YUV400_10bit
// };
static inline uint64_t GetTime()
{
	uint64_t cnt;
#if defined(__aarch64__)
	asm volatile("mrs %0, cntpct_el0" : "=r"(cnt));
#elif defined(__arm__)
	// For 32-bit ARM
	uint32_t lo, hi;
	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"(lo), "=r"(hi));
	cnt = ((uint64_t)hi << 32) | lo;
#else
	// Fallback for other architectures - use a simple counter
	static uint64_t fallback_counter = 0;
	cnt = ++fallback_counter;
#endif
	return cnt;
}

struct resizer {
	int hpid;
	int mp_enable;
	int mp_format;
	int sp1_enable;
	int sp1_format;
};
//class vvinstance_dev mdev;
int VsiVvdeviceExecuteCaseline
(
	VvbenchVvdev_t *caseCtx,
	VvbenchCfg_t *cfgCtx
)
{
	int result;
	bool oba_flag = false;
	LOGI("%s enter \n", __func__);

	if ((caseCtx == NULL) || (cfgCtx == NULL)) {
		LOGE("%s: NULL pointer", __func__);
		return VVCASE_EXEC_FAIL;
	}
	char version[VERSION_LEN] = "\0";
	CamCommonContext_t *pCamCommonCtx = NULL;

	int totalInstance = caseCtx->totalInstance;
	if (totalInstance > CAMDEV_HARDWARE_ID_MAX * CAMDEV_VIRTUAL_ID_MAX) {
		LOGE("%s: too many instance configuration: %d", __func__, totalInstance);
		return VVCASE_EXEC_FAIL;
	}

	VvbenchInstance_t *pVvbenchInstance = mm_malloc(sizeof(VvbenchInstance_t));
	if (NULL == pVvbenchInstance) {
		return -1;
	}
	MEMSET(pVvbenchInstance, 0, sizeof(VvbenchInstance_t));

	pVvbenchInstance->useSubSystem = caseCtx->useSubSystem;
	pVvbenchInstance->systemId = caseCtx->systemId;

	cfgCtx->streamDuration = 0;

	// pVvbenchInstance->pPostProcesser=mm_malloc(sizeof(VvbenchPostProcessInstance_t));
	// if (NULL == pVvbenchInstance->pPostProcesser)
	// {
	// LOGE("%s: malloc VvbenchDumperInstance fail", __func__);
	// return VVCASE_EXEC_FAIL;
	// }

	// VsiInitPostProcesser(pVvbenchInstance->pPostProcesser, true, true);

#ifdef ISP_OFFLINE_TEST
	VsiCamDeviceSetCaseName(caseCtx->caseName, strlen(caseCtx->caseName));
#endif

	VsiVvdeviceInstanceInit(pVvbenchInstance, caseCtx);
	LOGI("%s: after VsiVvdeviceInstanceInit ", __func__);

	bool_t initSwSimuEnable = caseCtx->swSimuCfg.enable;
	for (int index = 0; index < totalInstance; index++) {
		if (0 == caseCtx->instanceCfgCtx[index].hCamCommon) {
			LOGE("%s: invalid cam common handle", __func__);
			return VVCASE_EXEC_FAIL;
		}
		pCamCommonCtx = (CamCommonContext_t *)caseCtx->instanceCfgCtx[index].hCamCommon;

		if (caseCtx->instanceCfgCtx[index].instanceEnable == 0) {
			continue;
		}

		uint32_t instanceId = index;
		LOGI("Testing instance: %d", instanceId);

		//get common parameters for case settings
		if (caseCtx->instanceCfgCtx[index].streamDuration > cfgCtx->streamDuration) {
			LOGI("Update streaming duration %d to case cfg from hardware pipeline:%d",
			     caseCtx->instanceCfgCtx[index].streamDuration, caseCtx->instanceCfgCtx[index].hpId);
			cfgCtx->streamDuration = caseCtx->instanceCfgCtx[index].streamDuration;
		}

		LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

		CamDeviceConfig_t camConfig;
		MEMSET(&camConfig, 0, sizeof(CamDeviceConfig_t));

		camConfig.ispHwId = caseCtx->instanceCfgCtx[index].hpId;
		camConfig.inputCfg.inputType = caseCtx->instanceCfgCtx[index].inputType;
		MEMCPY(camConfig.inputCfg.inputDevName, caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorName,
		       sizeof(CAMDEV_INPUT_DEV_NAME_LEN));
#ifdef APU_CORE
		if (selectDestinationCore(camConfig.ispHwId) != XST_SUCCESS) {
			LOGE("selectDestinationCore failed for ISP %d", camConfig.ispHwId);
			continue;
		}
#endif
		LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

		camConfig.workCfg.workMode = caseCtx->instanceCfgCtx[index].workCfg.workMode;
		if (caseCtx->instanceCfgCtx[index].workCfg.workMode == CAMDEV_WORK_MODE_MCM) {
			camConfig.workCfg.workMode = CAMDEV_WORK_MODE_MCM;
			camConfig.workCfg.modeCfg.mcm.portId = caseCtx->instanceCfgCtx[index].workCfg.modeCfg.mcm.portId;
			camConfig.workCfg.modeCfg.mcm.mcmOp = caseCtx->instanceCfgCtx[index].workCfg.modeCfg.mcm.mcmOp;
			camConfig.workCfg.modeCfg.mcm.mcmSel = caseCtx->instanceCfgCtx[index].workCfg.modeCfg.mcm.mcmSel;
		}
		else if (caseCtx->instanceCfgCtx[index].workCfg.workMode == CAMDEV_WORK_MODE_STREAM) {
			camConfig.workCfg.modeCfg.stream.portId =
				caseCtx->instanceCfgCtx[index].workCfg.modeCfg.stream.portId;
		}
		camConfig.outputCfg.outputType = caseCtx->instanceCfgCtx[index].outPutType;
		camConfig.priority = caseCtx->instanceCfgCtx[index].priority;

		// Tile Cfg
		camConfig.workCfg.tileCfg.tileOp = caseCtx->instanceCfgCtx[index].workCfg.tileCfg.tileOp;
		camConfig.workCfg.tileCfg.tileNum = caseCtx->instanceCfgCtx[index].workCfg.tileCfg.tileNum;
		LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

#ifdef APU_CORE
		if (selectDestinationCore(caseCtx->instanceCfgCtx[index].hpId) != XST_SUCCESS) {
			LOGE("selectDestinationCore failed for ISP %d", caseCtx->instanceCfgCtx[index].hpId);
			continue;
		}
#endif
			result = SetATM();

		LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

		result = SendFirmwareCompability();
		if (0 != result) {
			LOGE("error sending firmware compat");
			mm_free(pVvbenchInstance);
			return -1;
		}
		LOGI("RPU Compatibility is Set");
		//Open instance for initials
		result = VsiCamDeviceCreate(&camConfig, &pVvbenchInstance->hCamDevice[index]);
		if (0 != result) {
			LOGE("open camera error, release cam_handler");
			VsiCamDeviceDestroy(pVvbenchInstance->hCamDevice[index]);
			mm_free(pVvbenchInstance);
			return -1;
		}


#ifdef USE_SYSTEM
		//Add ISP device into sub system
		if (caseCtx->useSubSystem) {
			pVvbenchInstance->ispSystem[index].moduleNum += 1;
			pVvbenchInstance->ispSystem[index].info[pVvbenchInstance->systemId.ispId].moduelCtrl.startStreaming
				= VsiCamDeviceStartStreaming;
			pVvbenchInstance->ispSystem[index].info[pVvbenchInstance->systemId.ispId].moduelCtrl.stopStreaming =
				VsiCamDeviceStopStreaming;
			pVvbenchInstance->ispSystem[index].info[pVvbenchInstance->systemId.ispId].moduelCtrl.pStartStreamingParam
				= &pVvbenchInstance->systemStreaming[index].ispStream.startConfig;
			pVvbenchInstance->ispSystem[index].info[pVvbenchInstance->systemId.ispId].moduelCtrl.pStopStreamingParam
				= &pVvbenchInstance->systemStreaming[index].ispStream.stopConfig;
		}
#endif

		pVvbenchInstance->camDevInfo[index].ispHwId = caseCtx->instanceCfgCtx[index].hpId;
		pVvbenchInstance->camDevInfo[index].inputType = caseCtx->instanceCfgCtx[index].inputType;
		pVvbenchInstance->camDevInfo[index].workMode = caseCtx->instanceCfgCtx[index].workCfg.workMode;

		//Load image mode: use it when there is a cfg json file
		if (caseCtx->instanceCfgCtx[instanceId].pictureCfg.fileName[0] != '\0') {
			result = VsiVvdeviceLoadImageJsonToDatabase(caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvdeviceLoadParameterToDatabase failed!");
				return result;
			}
		}

		if (CAMDEV_INPUT_TYPE_IMAGE == caseCtx->instanceCfgCtx[index].inputType) {
			//initial user input type
			pVvbenchInstance->mPicCase[index].loopCnt = caseCtx->instanceCfgCtx[index].pictureCfg.loopCnt;
			pVvbenchInstance->mPicCase[index].frameNum = caseCtx->instanceCfgCtx[index].pictureCfg.frameNum;
			pVvbenchInstance->mPicCase[index].pictureName =
				caseCtx->instanceCfgCtx[index].pictureCfg.pictureName;
			//picture file list get status initial
			pVvbenchInstance->mPicCase[index].currLine = caseCtx->instanceCfgCtx[index].pictureCfg.loadIndex;
			pVvbenchInstance->mPicCase[index].readLine = 0;
		}
		//load config json
		if (!initSwSimuEnable) {
			result = VsiVvdeviceLoadSimulatorToDatabase(caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvdeviceLoadSimulatorToDatabase failed!");
				return result;
			}
		}
		//Load XML
		result = VsiVvdeviceCalibControl(pVvbenchInstance, caseCtx, index);
		if (0 == result) {
			LOGI("VsiVvdeviceCalibControl succeed!");
		}
		else if (1 == result) {
			LOGI("VsiVvdeviceCalibControl not load xml!");
		}
		else {
			LOGE("VsiVvdeviceCalibControl failed!");
			return -1;
		}
		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType) {
			sensor_stream_flag = index; // for display buffer queueing in case of streming mode
			if (caseCtx->instanceCfgCtx[index].outPutType == CAMDEV_OUTPUT_TYPE_MEMORY
			    || caseCtx->instanceCfgCtx[index].outPutType == CAMDEV_OUTPUT_TYPE_BOTH) {
				memory_out_flag = index; // for displayincase of dma output type
			}
		}

		int outPutEnable = 0;
		CamDeviceBufChainId_t bufIo = CAMDEV_BUFCHAIN_MP;
		pVvbenchInstance->camDevInfo[index].alignMask = cfgCtx->alignMask;

		for (int i = 0; i < CAMDEV_BUFCHAIN_MAX; ++i) {
			if (caseCtx->instanceCfgCtx[index].instancePath[i].pathEnable == 0) {
				continue;
			}
			else {
				outPutEnable = 1;
			}
		}
		if (0 == outPutEnable) {
			LOGE("At least one of INSTANCE_BUFIO need to be enabled\n");
			mm_free(pVvbenchInstance);
			return -1;
		}
		/************************* FMC ID Selection *************************/
		{
			extern int fmc_selection;
			result = send_fmc_id_select(fmc_selection);
			if (result != RET_SUCCESS) {
				LOGE("send_fmc_id_select failed");
				mm_free(pVvbenchInstance);
				return -1;
			}
		}

		/*******************************PS i2c*******************************/

		HalI2cConfig_t pHalI2cConfig;
		memset(&pHalI2cConfig, 0, sizeof(HalI2cConfig_t));
		pHalI2cConfig.i2cBusId = 0;
		pHalI2cConfig.HalI2cMode = HAL_PS_I2C_MODE;
		HalI2cInit(&pHalI2cConfig);

		/******************************************************************/

		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType) {
			char sensorName[FILE_LEN];
			CamDeviceSensorModuleMapCfg_t sensorModule;
			CamDeviceSensorDrvHandle_t SensorDrvHandle = NULL;
			uint32_t modeIndex = caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex;
			bool_t useExternalDriver = caseCtx->instanceCfgCtx[index].sensorCfg.useExternalDriver;
			if (useExternalDriver == false) {
				MEMCPY(sensorModule.moduleName, caseCtx->instanceCfgCtx[index].sensorCfg.sensorName,
				       sizeof(caseCtx->instanceCfgCtx[index].sensorCfg.sensorName));
				LOGI("Vsi cam device sensor mapping driver for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName,
				     caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex);
				if (caseCtx->instanceCfgCtx[instanceId].workCfg.workMode == CAMDEV_WORK_MODE_MCM) {
					if(caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorDevId == -1) {
						sensorModule.sensorDevId = caseCtx->instanceCfgCtx[instanceId].workCfg.modeCfg.mcm.portId;
					} else {
						sensorModule.sensorDevId = caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorDevId;
					}
				} else if (caseCtx->instanceCfgCtx[instanceId].workCfg.workMode == CAMDEV_WORK_MODE_STREAM) {
					if(caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorDevId == -1) {
						sensorModule.sensorDevId = caseCtx->instanceCfgCtx[instanceId].workCfg.modeCfg.stream.portId;
					} else {
						sensorModule.sensorDevId = caseCtx->instanceCfgCtx[instanceId].sensorCfg.sensorDevId;
					}
				}
				result = VsiCamDeviceSensorMapping(&sensorModule, &SensorDrvHandle);
				if (result != 0 || NULL == SensorDrvHandle) {
					LOGE("Vsi cam device sensor mapping driver failed for sensor name:%s, modeIndex:%d", caseContext->instanceCfgCtx[instanceId].sensorCfg.sensorName, caseContext->instanceCfgCtx[instanceId].sensorCfg.modeIndex);
					mm_free(pVvbenchInstance);
					return -1;
				}
			}

			result = VsiCamDeviceSensorDrvHandleRegister(pVvbenchInstance->hCamDevice[index], SensorDrvHandle);
			if (result != 0) {
				LOGE("Vsi cam device sensor driver register failed %s !!", sensorName);
				mm_free(pVvbenchInstance);
				return -1;
			}


			CamDeviceSensorQuery_t sensorQuery;
			MEMSET(&sensorQuery, 0, sizeof(CamDeviceSensorQuery_t));
			result = VsiCamDeviceSensorQuery(pVvbenchInstance->hCamDevice[index], &sensorQuery);
			if (result != 0) {
				LOGE("Vsi cam device sensor query failed for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName, modeIndex);
				mm_free(pVvbenchInstance);
				return -1;
			}
			else {
				if (modeIndex >= sensorQuery.number) {
					LOGE("Vsi cam device sensor modeIndex out of range !");
					mm_free(pVvbenchInstance);
					return -1;
				}
			}

#ifdef SENSOR_EMU
			CamDeviceSensorModeInfo_t SemuModeInfo;
			MEMSET(&SemuModeInfo, 0, sizeof(CamDeviceSensorModeInfo_t));
			result = VsiCamDeviceSensorGetModeInfo(pVvbenchInstance->hCamDevice[index], &SemuModeInfo);
			if (0 != result) {
				LOGE("Vsi cam device sensor get mode info failed for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName,
				     caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex);
				mm_free(pVvbenchInstance);
				return -1;
			}

			// need to update sensor info here.
			SemuModeInfo.index = 0;
			SemuModeInfo.size.width = caseCtx->instanceCfgCtx[index].sensorCfg.sensorWidth;
			SemuModeInfo.size.height = caseCtx->instanceCfgCtx[index].sensorCfg.sensorHeight;
			//SemuModeInfo.bitWidth = caseCtx->instanceCfgCtx[index].sensorCfg.caseCtx->instanceCfgCtx[index].sensorCfg.;
			//SemuModeInfo.bayerPattern = caseCtx->instanceCfgCtx[index].sensorCfg.caseCtx->instanceCfgCtx[index].sensorCfg.;
			//SemuModeInfo.maxFps = caseCtx->instanceCfgCtx[index].sensorCfg.fps;

			result = VsiCamDeviceSensorSetModeInfo(pVvbenchInstance->hCamDevice[index], &SemuModeInfo);
			if (0 != result) {
				LOGE("Vsi cam device sensor set mode info failed for sensor name:%s",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName);
				mm_free(pVvbenchInstance);
				return -1;
			}
#endif
			result = VsiCamDeviceSensorOpen(pVvbenchInstance->hCamDevice[index], modeIndex);
			if (result != 0) {
				LOGE("Vsi cam device sensor open failed for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName, modeIndex);
				mm_free(pVvbenchInstance);
				return -1;
			}

			CamDeviceSensorModeInfo_t sensorModeInfo;
			MEMSET(&sensorModeInfo, 0, sizeof(sensorModeInfo));
			result = VsiCamDeviceSensorGetModeInfo(pVvbenchInstance->hCamDevice[index], &sensorModeInfo);
			if (0 != result) {
				LOGE("Vsi cam device sensor get mode info failed for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName,
				     caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex);
				mm_free(pVvbenchInstance);
				return -1;
			}
			else {
				caseCtx->instanceCfgCtx[index].sensorCfg.sensorType = sensorModeInfo.sensorType;
				caseCtx->instanceCfgCtx[index].sensorCfg.bitWidth = sensorModeInfo.bitWidth;
				caseCtx->instanceCfgCtx[index].sensorCfg.stitchingMode = sensorModeInfo.stitchingMode;
				LOGI("Vsi cam device sensor type: %d, modeIndex:%d", sensorModeInfo.sensorType,
				     caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex);
			}
			pCamCommonCtx->sensorMode = sensorModeInfo;
		}

		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType) {
			CamDeviceSensorTestPattern_t testPattern;

			if (caseCtx->instanceCfgCtx[index].sensorCfg.testPatternEnable) {
				testPattern.enable = true;
			}
			else {
				testPattern.enable = false;
			}
			result = VsiCamDeviceSensorSetTestPattern(pVvbenchInstance->hCamDevice[index], &testPattern);
			if (0 != result) {
				LOGE("VsiCamDeviceSensorSetTestPattern failed!");
				return -1;
			}

			//VI200
			if (caseCtx->instanceCfgCtx[index].funcCtrl.useVi200Function) {
				if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_METADATA].pathEnable) {
					result = VsiVvdeviceSensorFunc(pVvbenchInstance->hCamDevice[index], caseCtx, index);
					if (0 != result) {
						LOGE("VsiVvdeviceSensorFunc failed!");
						mm_free(pVvbenchInstance);
						return -1;
					}
				}
				else {
					caseCtx->instanceCfgCtx[index].funcCtrl.useVi200MetaWin = false;
				}
			}
		}

		if (CAMDEV_INPUT_TYPE_TPG == caseCtx->instanceCfgCtx[index].inputType) {
			CamDevicePipeInFmt_t inFormat;
			MEMSET(&inFormat, 0, sizeof(CamDevicePipeInFmt_t));

			//TPG metadata judgment
			if (!caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_METADATA].pathEnable) {
				caseCtx->instanceCfgCtx[index].funcCtrl.useVi200MetaWin = false;
			}

			if (caseCtx->instanceCfgCtx[index].funcCtrl.useVi200MetaWin) {
				inFormat.inWidth = caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_RDMA].width;
				inFormat.inHeight = caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_RDMA].height;
			}
			else {
				inFormat.inWidth = caseCtx->instanceCfgCtx[instanceId].tpgCfg.width;
				inFormat.inHeight = caseCtx->instanceCfgCtx[instanceId].tpgCfg.height;
			}
			inFormat.inBit = (CamDeviceBitDepth_t)caseCtx->instanceCfgCtx[instanceId].tpgCfg.colorDepth;
			inFormat.inPattern = (CamDeviceRawPattern_t)caseCtx->instanceCfgCtx[instanceId].tpgCfg.bayerPattern;
			result = VsiCamDeviceSetInFormat(pVvbenchInstance->hCamDevice[index], CAMDEV_PIPE_INPATH_TPG,
							 &inFormat);
			if (result != 0) {
				LOGE("Vsi camdevice set format failed!");
				mm_free(pVvbenchInstance);
				return -1;
			}

		}

		//InPut Path Create
		for (int i = CAMDEV_BUFCHAIN_RDMA; i < CAMDEV_BUFCHAIN_MAX; ++i) {
			bufIo = caseCtx->instanceCfgCtx[instanceId].instancePath[i].path;
			if (caseCtx->instanceCfgCtx[instanceId].instancePath[i].pathEnable == 0) {
				continue;
			}
			else {
				result = VsiVvdeviceInputPathCreate(pVvbenchInstance, caseCtx, index, bufIo);
				if (0 != result) {
					LOGE("VsiVvdeviceInputPathCreate failed !");
					mm_free(pVvbenchInstance);
					return -1;
				}
			}
		}

		//Camera connect
		CamDevicePipeSubmoduleCtrl_u submoduleInitCtrl;
		submoduleInitCtrl.allCtrl = 0xFFFFFFFF;
		submoduleInitCtrl.subCtrl.rgbirEnable = 0;
		submoduleInitCtrl.subCtrl.hdrEnable = 0;
		submoduleInitCtrl.subCtrl.pdafEnable = 0;
		submoduleInitCtrl.subCtrl.dpfEnable = 0;
		submoduleInitCtrl.subCtrl.compressEnable = 0;
		submoduleInitCtrl.subCtrl.expandEnable = 0;
		submoduleInitCtrl.subCtrl.eeEnable = 0;
		submoduleInitCtrl.subCtrl.ynrEnable = 0;
		submoduleInitCtrl.subCtrl.cnrEnable = 0;
		submoduleInitCtrl.subCtrl.lut3dEnable = 0;
		submoduleInitCtrl.subCtrl.dnr2Enable = 0;
		submoduleInitCtrl.subCtrl.dnr3Enable = 0;
		submoduleInitCtrl.subCtrl.gtmEnable = 0;
		submoduleInitCtrl.subCtrl.wdrEnable = 0;
		submoduleInitCtrl.subCtrl.lscEnable = 0;
		if (caseCtx->swSimuCfg.enable) {
			submoduleInitCtrl.allCtrl = 0x0;
		}
		result = VsiCamDeviceConnectCamera(pVvbenchInstance->hCamDevice[index], &submoduleInitCtrl);
		if (0 != result) {
			LOGE("Vsi cam device connect camera failed!");
			mm_free(pVvbenchInstance);
			return -1;
		}

		//OutPut Path Create
		for (int i = 0; i <= CAMDEV_BUFCHAIN_METADATA; ++i) {
			bufIo = caseCtx->instanceCfgCtx[instanceId].instancePath[i].path;
			if (caseCtx->instanceCfgCtx[instanceId].instancePath[i].pathEnable == 0) {
				continue;
			}
			else {
				result = VsiVvdeviceOutPutPathCreate(pVvbenchInstance, caseCtx, index, bufIo);
				if (0 != result) {
					LOGE("VsiVvdeviceOutPutPathCreate failed !");
					mm_free(pVvbenchInstance);
					return -1;
				}
			}
		}


		//FUSA
		if (caseCtx->instanceCfgCtx[index].funcCtrl.useFusaFunction) {
			result = VsiVvbenchFusaFunc(pVvbenchInstance->hCamDevice[index], caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvbenchFusaFunc failed!");
				mm_free(pVvbenchInstance);
				return -1;
			}
		}

		//Nr Reloc Control
		result = VsiVvdeviceNrRelocControl(pVvbenchInstance->hCamDevice[index], caseCtx);
		if (0 != result) {
			LOGE("Vsi cam device nr reloc control failed !");
			mm_free(pVvbenchInstance);
			return -1;
		}

		//Register 3A Lib
		result = VsiVvbenchRegister3ALib(pVvbenchInstance->hCamDevice[index], caseCtx, index);
		if (0 != result) {
			LOGE("Vsi cam device register 3A Lib failed !");
			mm_free(pVvbenchInstance);
			return -1;
		}

		//Auto Control
		CamDeviceModuleAutoCtrlConfig_t autoCtrlCfg;
		MEMSET(&autoCtrlCfg, 0, sizeof(CamDeviceModuleAutoCtrlConfig_t));
		result = VsiCamCommonConfigureAutoCtrl(caseCtx->instanceCfgCtx[index].hCamCommon,
						       pVvbenchInstance->hCamDevice[index]);
		if (0 != result) {
			LOGE("Vsi cam common configure auto control failed !");
			mm_free(pVvbenchInstance);
			return -1;
		}
		result = VsiCamDeviceModuleAutoCtrlGetConfig(pVvbenchInstance->hCamDevice[index], &autoCtrlCfg);
		if (0 != result) {
			LOGE("Vsi cam device get auto control failed !");
			mm_free(pVvbenchInstance);
			return -1;
		}

		//Submodule Control
		result = VsiVvdeviceSubModuleControl(pVvbenchInstance->hCamDevice[index], caseCtx, index);
		if (0 != result) {
			LOGE("Vsi cam device submodule control failed !");
			mm_free(pVvbenchInstance);
			return -1;
		}
	}

	vvbench_vdev_iba_t *iba_vdev = mm_malloc(sizeof(vvbench_vdev_iba_t));
	vvbench_vdev_oba_t *oba_vdev = mm_malloc(sizeof(vvbench_vdev_oba_t));
	for (int index = 0; index < totalInstance; index++) {
		///////////////////////////// IBA //////////////////////////////
		MEMSET(iba_vdev, 0, sizeof(vvbench_vdev_iba_t));
		iba_vdev->inputType = caseCtx->instanceCfgCtx[index].inputType;
		iba_vdev->hpId = caseCtx->instanceCfgCtx[index].hpId;
		ISP_ID = caseCtx->instanceCfgCtx[index].hpId;
		iba_vdev->portId = caseCtx->instanceCfgCtx[index].workCfg.modeCfg.mcm.portId;
		iba_vdev->sensorHeight = caseCtx->instanceCfgCtx[index].sensorCfg.sensorHeight;
		iba_vdev->sensorWidth = caseCtx->instanceCfgCtx[index].sensorCfg.sensorWidth;
		iba_vdev->virtualChannelId = caseCtx->instanceCfgCtx[index].sensorCfg.virtualChannelId; //2026.1
		if(iba_vdev->hpId%2 == 0) {
			iba_vdev->ppc = VispSsInst[index].Config.Iba0Ppc;
		}
		else if(iba_vdev->hpId%2 == 1) {
			iba_vdev->ppc = VispSsInst[index].Config.Iba4Ppc;
		}
		IBA_init_send_command(pVvbenchInstance->hCamDevice[index], iba_vdev, index);

		///////////////////////////////////////////////////////////////////
		CamDeviceContext_t *hcmdv = pVvbenchInstance->hCamDevice[index];

		//OBA Config
		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType
		    && (caseCtx->instanceCfgCtx[index].outPutType == CAMDEV_OUTPUT_TYPE_ONLINE
			|| caseCtx->instanceCfgCtx[index].outPutType == CAMDEV_OUTPUT_TYPE_BOTH
			|| (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].pathOutType == 1)
			|| (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].pathOutType == 0)
			|| (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].pathOutType == 1)
			|| (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].pathOutType == 0)
		       )) {
			if (oba_flag == false) {
				MEMSET(oba_vdev, 0, sizeof(vvbench_vdev_oba_t));
				oba_vdev->path[CAMDEV_BUFCHAIN_MP] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].path;
				oba_vdev->pathEnable[CAMDEV_BUFCHAIN_MP] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].pathEnable;
				oba_vdev->pathOutType[CAMDEV_BUFCHAIN_MP] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].pathOutType;
				oba_vdev->format[CAMDEV_BUFCHAIN_MP] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].format;
				oba_vdev->dataBits[CAMDEV_BUFCHAIN_MP] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].dataBits;

				oba_vdev->hpId = caseCtx->instanceCfgCtx[index].hpId;
				oba_vdev->inputType = caseCtx->instanceCfgCtx[index].inputType;
				oba_vdev->workMode = caseCtx->instanceCfgCtx[index].workCfg.workMode;
				oba_vdev->outPutType = caseCtx->instanceCfgCtx[index].outPutType;


				oba_vdev->path[CAMDEV_BUFCHAIN_SP1] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].path;
				oba_vdev->pathEnable[CAMDEV_BUFCHAIN_SP1] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].pathEnable;
				oba_vdev->pathOutType[CAMDEV_BUFCHAIN_SP1] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].pathOutType;
				oba_vdev->format[CAMDEV_BUFCHAIN_SP1] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].format;
				oba_vdev->dataBits[CAMDEV_BUFCHAIN_SP1] =
					caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].dataBits;


				OBA_init_send_command(pVvbenchInstance->hCamDevice[index], oba_vdev, index);
				oba_flag = true;
			}
		}
	}


	/********/
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	enable_fbwr();
#endif
	for (int index = 0; index < totalInstance; index++) {

		if (caseCtx->instanceCfgCtx[index].instanceEnable == 0) {
			continue;
		}

		uint32_t instanceId = index;
#ifdef APU_CORE
		if (selectDestinationCore(caseCtx->instanceCfgCtx[index].hpId) != XST_SUCCESS) {
			LOGE("selectDestinationCore failed for ISP %d", caseCtx->instanceCfgCtx[index].hpId);
			continue;
		}
#endif
		// Each path callback register
		CamDeviceBufChainId_t bufIoArray[CAMDEV_BUFCHAIN_MAX];
		int bufIoArrayCount = 0;
		for (int i = 0; i < CAMDEV_BUFCHAIN_MAX; i++) {
			if (caseCtx->instanceCfgCtx[index].instancePath[i].pathEnable) {
				CamDeviceBufChainId_t bufIo = caseCtx->instanceCfgCtx[index].instancePath[i].path;
				if ((instanceId >= MAX_CAM_NUM) || (bufIo >= CAMDEV_BUFCHAIN_MAX)) {
					mm_free(pVvbenchInstance);
					return -1;
				}
				bufIoArray[bufIoArrayCount++] = bufIo;
			}
		}
		if (caseCtx->instanceCfgCtx[index].startPathSimultaneous) {
			LOGI("VsiVvdevicePathEnable called Simultaneously");
			result = VsiVvdevicePathEnable(pVvbenchInstance, caseCtx, index, bufIoArray, bufIoArrayCount);
			if (0 != result) {
				LOGE("VsiVvdevicePathEnable failed !");
				mm_free(pVvbenchInstance);
				return -1;
			}
		}
		else {
			for (int i = 0; i < bufIoArrayCount; i++) {
				result = VsiVvdevicePathEnable(pVvbenchInstance, caseCtx, index, bufIoArray + i, 1);
				if (0 != result) {
					LOGE("VsiVvdevicePathEnable failed !");
					mm_free(pVvbenchInstance);
					return -1;
				}
			}
		}


		//TPG
		if (caseCtx->instanceCfgCtx[index].funcCtrl.useIspTpgFunction) {
			if (caseCtx->instanceCfgCtx[index].funcCtrl.useVi200Function) {
			}
			else {
				result = VsiVvbenchTpgFunc(pVvbenchInstance->hCamDevice[index], caseCtx, index);
				if (0 != result) {
					LOGE("VsiVvbenchTpgFunc failed!");
					mm_free(pVvbenchInstance);
					return -1;
				}
			}
		}

		//Register load
		TDatabaseDump_t *pRegisterDump = NULL;
		result = TDatabase_query(pCamCommonCtx->hDatabase, T_DATABASE_DUMP, (void**)&pRegisterDump);
		for (int i = 0; i < pRegisterDump->registerNumber; i++) {
			TDatabaseRegister_t *pRegister = pRegisterDump->pRegisters + i;

			unsigned int address = strtol(pRegister->address, NULL, 16);
			unsigned int value = strtol(pRegister->value, NULL, 16);

			if (pRegister->isReadAction) {
				result = VsiCamDeviceReadRegister(pVvbenchInstance->hCamDevice[index], address, &value);
				if (0 != result) {
					LOGE("VsiCamDeviceReadRegister failed!");
					return -1;
				}
				LOGI("VsiCamDeviceReadRegister: address:0x%x  value:0x%x !", address, value);
			}
			else {
				result = VsiCamDeviceWriteRegister(pVvbenchInstance->hCamDevice[index], address, value);
				if (0 != result) {
					LOGE("VsiCamDeviceWriteRegister failed!");
					return -1;
				}
				LOGI("VsiCamDeviceWriteRegister: address:0x%x  value:0x%x !", address, value);
			}
		}


		//get statistic control
		{
			result = VsiVvdeviceGetStatisticControl(pVvbenchInstance, caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvdeviceGetStatisticControl failed!");
				mm_free(pVvbenchInstance);
				return -1;
			}
		}

		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType) {
			sensor_stream_flag = index;
			//set sensor fps
			result = VsiCamDeviceSensorSetFrameRate(pVvbenchInstance->hCamDevice[index],
								&caseCtx->instanceCfgCtx[index].sensorCfg.fps);
			if (0 != result) {
				LOGE("Vsi cam device sensor set frame rate failed for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName,
				     caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex);
				mm_free(pVvbenchInstance);
				return -1;
			}

			//get sensor fps
			float fps;
			result = VsiCamDeviceSensorGetFrameRate(pVvbenchInstance->hCamDevice[index], &fps);
			if (0 != result) {
				LOGE("Vsi cam device sensor get frame rate failed for sensor name:%s, modeIndex:%d",
				     caseCtx->instanceCfgCtx[index].sensorCfg.sensorName,
				     caseCtx->instanceCfgCtx[index].sensorCfg.modeIndex);
				mm_free(pVvbenchInstance);
				return -1;
			}
			LOGI("SENSOR_FPS_GET Result, fps:%.2f", fps);
		}

		if (CAMDEV_INPUT_TYPE_IMAGE == caseCtx->instanceCfgCtx[index].inputType) {
			int ret;
			MediaBuffer_t *pMediaBuff;
			uint32_t frameNum, bufferNum, rest_frame_num;
			CamDeviceInputRawFmt_t rawFmt;
			CamDeviceBufChainId_t bufferIO;
			if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_RETIMING].pathEnable) {
				bufferIO = CAMDEV_BUFCHAIN_RETIMING;
			}
			else {
				bufferIO = CAMDEV_BUFCHAIN_RDMA;
			}

			int32_t loopCnt = pVvbenchInstance->mPicCase[instanceId].loopCnt;
			frameNum = pVvbenchInstance->mPicCase[instanceId].frameNum;
			bufferNum = pVvbenchInstance->camDevInfo[instanceId].bufCfg[bufferIO].bufferNumber;
			rawFmt = pVvbenchInstance->camDevInfo[instanceId].bufCfg[bufferIO].format;
			rest_frame_num = frameNum;
			int coff = 1; // for HDR load (L + S + VS) into one media buffer
			if (rawFmt >= CAMDEV_INPUT_FMT_2DOL) {
				coff = (rawFmt - CAMDEV_INPUT_FMT_2DOL + 2);
			}
			do {

				ret = VsiCamDeviceDeQueBuffer(pVvbenchInstance->hCamDevice[instanceId], bufferIO, &pMediaBuff);

				if (RET_SUCCESS != ret) {
					LOGW("%s DQBUF failed(%d)!", __func__, ret);
					return ret;
				}
				pVvbenchInstance->mPicCase[instanceId].readLine = RDMA_INPUT_PIC_NUM * coff;

				if (pVvbenchInstance->mPicCase[instanceId].readLine / coff > bufferNum) {
					pVvbenchInstance->mPicCase[instanceId].readLine = bufferNum * coff;
				}

				if (rest_frame_num * coff > pVvbenchInstance->mPicCase[instanceId].readLine) {
					rest_frame_num -= pVvbenchInstance->mPicCase[instanceId].readLine / coff;
				}
				else {
					pVvbenchInstance->mPicCase[instanceId].readLine = rest_frame_num * coff;
					rest_frame_num = frameNum;
				}

				if (RET_SUCCESS != VsiVvdeviceGetInputPicture(pVvbenchInstance, instanceId, bufferIO, pMediaBuff)) {
					LOGE("%s:vvbench get input picture failed", __func__);
					return -1;
				}
				if (RET_SUCCESS != VsiCamDeviceEnQueBuffer(pVvbenchInstance->hCamDevice[instanceId], bufferIO,
						pMediaBuff)) {
					LOGW("%s QBUF failed!", __func__);
					return -1;
				}
#if 1
				{
					for (int i = CAMDEV_BUFCHAIN_MP; i <= CAMDEV_BUFCHAIN_SP2; i ++) {
						if (caseCtx->instanceCfgCtx[index].instancePath[i].pathEnable == 0) {
							continue;
						}
						else {
							MediaBuffer_t *pBuf;
#ifdef APU_CORE
							uint32_t camDeviceInstanceId = CtrlGetCamDeviceInstanceId(pVvbenchInstance->hCamDevice[index]);
							/* Wait for RPU to push a completed output buffer */
							while (apuBufferInform[camDeviceInstanceId][i] == 0) {
								Apu_Mbox_Check_Command();
							}
							apuBufferInform[camDeviceInstanceId][i]--;
							result = ApuDeQueReceivedBuffer(camDeviceInstanceId, i, &pBuf);
#else
							result = VsiCamDeviceDeQueBuffer(pVvbenchInstance->hCamDevice[index], i, &pBuf);
#endif
							if (RET_SUCCESS != result) {
								LOGW("%s DQBUF failed(%d)!", __func__, result);
								return -1;
							}
#ifdef APU_CORE
							/* Populate PicBufMetaData_t Data fields from APU path config
							 * since RPU and APU PicBufMetaData_t have different layouts */
							if (pBuf && pBuf->pMetaData) {
								VvbenchBufferCfg_t *pBufCfg = &pVvbenchInstance->camDevInfo[index].bufCfg[i];
								PicBufMetaData_t *pMeta = (PicBufMetaData_t *)(uintptr_t)pBuf->pMetaData;
								uint32_t w = pBufCfg->width;
								uint32_t h = pBufCfg->height;
								uint32_t bits = pBufCfg->dataBits;
								pMeta->yuvOrder = pBufCfg->yuvOrder;
								switch (pMeta->Type) {
								case PIC_BUF_TYPE_RGB888:
									if (pMeta->Layout == PIC_BUF_LAYOUT_COMBINED) {
										pMeta->Data.RGB.combined.PicWidthPixel = w;
										pMeta->Data.RGB.combined.PicHeightPixel = h;
										pMeta->Data.RGB.combined.PicWidthBytes = w * 3;
										pMeta->Data.RGB.combined.bitWidth = bits;
									} else {
										pMeta->Data.RGB.planar.R.PicWidthPixel = w;
										pMeta->Data.RGB.planar.R.PicHeightPixel = h;
										pMeta->Data.RGB.planar.R.PicWidthBytes = w;
										pMeta->Data.RGB.planar.G.PicWidthPixel = w;
										pMeta->Data.RGB.planar.G.PicHeightPixel = h;
										pMeta->Data.RGB.planar.G.PicWidthBytes = w;
										pMeta->Data.RGB.planar.B.PicWidthPixel = w;
										pMeta->Data.RGB.planar.B.PicHeightPixel = h;
										pMeta->Data.RGB.planar.B.PicWidthBytes = w;
									}
									break;
								default:
									break;
								}
							}
#endif
							LOGI("the VsiCamDeviceDeQueBuffer is 0x%08x\r\n", pBuf->baseAddress);
							VsiVvdeviceShowBuffer(pVvbenchInstance, pBuf, index, i);
							showFrameNum++;
							result = VsiCamDeviceEnQueBuffer(pVvbenchInstance->hCamDevice[index], i, pBuf);
							if (RET_SUCCESS != result) {
								LOGW("%s EQBUF failed(%d)!", __func__, result);
								return -1;
							}
							VsiVvdeviceGetStatistic(pVvbenchInstance, pVvbenchInstance->hCamDevice[index], index);
						}
					}

				}
#endif
				if (frameNum == rest_frame_num) {
					pVvbenchInstance->mPicCase[instanceId].currLine = 0;
					loopCnt --;
				}
				else {
					pVvbenchInstance->mPicCase[instanceId].currLine += pVvbenchInstance->mPicCase[instanceId].readLine;
				}
			} while (loopCnt > 0);
		}
		else if (CAMDEV_INPUT_TYPE_TPG == caseCtx->instanceCfgCtx[index].inputType) {
			uint32_t frameNum;
			frameNum = caseCtx->instanceCfgCtx[index].tpgCfg.frameNum;
			if (frameNum > 0) {
				for (int j = 0; j < frameNum; j++) {
					for (int i = CAMDEV_BUFCHAIN_MP; i <= CAMDEV_BUFCHAIN_SP2; i ++) {
						if (caseCtx->instanceCfgCtx[index].instancePath[i].pathEnable == 0) {
							continue;
						}
						else {
							MediaBuffer_t *pBuf;
							result = VsiCamDeviceDeQueBuffer(pVvbenchInstance->hCamDevice[index], i, &pBuf);
							if (RET_SUCCESS != result) {
								LOGW("%s DQBUF failed(%d)!", __func__, result);
								break;
							}
							LOGI("the VsiCamDeviceDeQueBuffer is 0x%08x\r\n", pBuf->baseAddress);
							VsiVvdeviceShowBuffer(pVvbenchInstance, pBuf, index, i);
							showFrameNum++;
							result = VsiCamDeviceEnQueBuffer(pVvbenchInstance->hCamDevice[index], i, pBuf);
							if (RET_SUCCESS != result) {
								LOGW("%s EQBUF failed(%d)!", __func__, result);
								return -1;
							}
							VsiVvdeviceGetStatistic(pVvbenchInstance, pVvbenchInstance->hCamDevice[index], index);
						}
					}
				}
			}
			else {
				while (1) {}
			}
		}
	}
	MediaBuffer_t *tempBuf[6][CAMDEV_BUFCHAIN_SP2] = {NULL};
#if 1
	/*Sensor display, need change displayInstance to show difference instance*/

	// Initialize timer-based approach for every second reporting
	dequeue_call_count = 0;
	uint64_t displayInstance = 0;
	// Debug: Check the loop condition flags
	xil_printf("Starting main loop: sensor_stream_flag=%d, memory_out_flag=%d\n", sensor_stream_flag,
		   memory_out_flag);

	uint32_t freq;
	asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
	start = GetTime();

	while (sensor_stream_flag >= 0 && memory_out_flag >= 0) {
#ifdef XPAR_XV_MIX_NUM_INSTANCES
		/* Poll HDMI TX state machine for event-driven stream start */
		VmixHdmiBridge_HdmiPoll();
#endif
		MediaBuffer_t *pBuf;
		// Timer-based approach for every second reporting
		now = GetTime();
		u64 elapsed_ticks = now - start;
		double elapsed_seconds = (double)elapsed_ticks / freq;

		if (elapsed_seconds >= 1.0) {
			LOGI("Stream FPS = %dfps", dequeue_call_count);
			xil_printf("================================\n");
			// Reset counters for next second
			start = GetTime();
			dequeue_call_count = 0;
			displayInstance++;
		}
		//	CamDevicePipeOutFmt_t test_0={1920,1080,CAMDEV_PIX_FMT_YUV422SP,0};
		//	int displayInstance = 0;
		uint32_t camDeviceInstanceId = 0;
		for (int index = 0; index < totalInstance; index++) {
			if (caseCtx->instanceCfgCtx[index].instanceEnable == 0) {
				continue;
			}
#ifdef APU_CORE
			if (selectDestinationCore(caseCtx->instanceCfgCtx[index].hpId) != XST_SUCCESS) {
				LOGE("selectDestinationCore failed for ISP %d", caseCtx->instanceCfgCtx[index].hpId);
				continue;
			}
#endif
			if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType) {
				for (int i = CAMDEV_BUFCHAIN_MP; i <= CAMDEV_BUFCHAIN_RAW; i ++) {
					if (caseCtx->instanceCfgCtx[index].instancePath[i].pathEnable == 0
					    || (caseCtx->instanceCfgCtx[index].instancePath[i].pathOutType == 1)) {
						continue;
					}
					else {
#ifdef APU_CORE
						camDeviceInstanceId = CtrlGetCamDeviceInstanceId(pVvbenchInstance->hCamDevice[index]);
						if (apuBufferInform[camDeviceInstanceId][i] == 0) {
							Apu_Mbox_Check_Command();
							continue;
						}
						apuBufferInform[camDeviceInstanceId][i]--;
						result = ApuDeQueReceivedBuffer(camDeviceInstanceId, i, &pBuf);
#else
						result = VsiCamDeviceDeQueBuffer(pVvbenchInstance->hCamDevice[index], i, &pBuf);
#endif
						if (RET_SUCCESS != result) {
							LOGW("%s DQBUF failed(%d)!", __func__, result);
							break;
						}
#ifdef APU_CORE
						/* Populate PicBufMetaData_t Data fields from APU path config
						 * since RPU and APU PicBufMetaData_t have different layouts */
						if (pBuf && pBuf->pMetaData) {
							VvbenchBufferCfg_t *pBufCfg = &pVvbenchInstance->camDevInfo[index].bufCfg[i];
							PicBufMetaData_t *pMeta = pBuf->pMetaData;
							uint32_t w = pBufCfg->width;
							uint32_t h = pBufCfg->height;
							uint32_t fmt = pBufCfg->format;
							uint32_t bits = pBufCfg->dataBits;
							pMeta->yuvOrder = pBufCfg->yuvOrder;
							switch (pMeta->Type) {
							case PIC_BUF_TYPE_RGB888:
								if (pMeta->Layout == PIC_BUF_LAYOUT_COMBINED) {
									pMeta->Data.RGB.combined.PicWidthPixel = w;
									pMeta->Data.RGB.combined.PicHeightPixel = h;
									pMeta->Data.RGB.combined.PicWidthBytes = w * 3;
									pMeta->Data.RGB.combined.bitWidth = bits;
								} else {
									pMeta->Data.RGB.planar.R.PicWidthPixel = w;
									pMeta->Data.RGB.planar.R.PicHeightPixel = h;
								}
								break;
							case PIC_BUF_TYPE_YCbCr422:
								if (pMeta->Layout == PIC_BUF_LAYOUT_SEMIPLANAR) {
									pMeta->Data.YCbCr.semiplanar.Y.PicWidthPixel = w;
									pMeta->Data.YCbCr.semiplanar.Y.PicHeightPixel = h;
									pMeta->Data.YCbCr.semiplanar.Y.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.semiplanar.Y.bitWidth = bits;
									pMeta->Data.YCbCr.semiplanar.CbCr.PicWidthPixel = w;
									pMeta->Data.YCbCr.semiplanar.CbCr.PicHeightPixel = h;
									pMeta->Data.YCbCr.semiplanar.CbCr.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.semiplanar.CbCr.bitWidth = bits;
								} else if (pMeta->Layout == PIC_BUF_LAYOUT_PLANAR) {
									pMeta->Data.YCbCr.planar.Y.PicWidthPixel = w;
									pMeta->Data.YCbCr.planar.Y.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Y.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.planar.Y.bitWidth = bits;
									pMeta->Data.YCbCr.planar.Cb.PicWidthPixel = w / 2;
									pMeta->Data.YCbCr.planar.Cb.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Cb.PicWidthBytes = (w / 2) * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.planar.Cr.PicWidthPixel = w / 2;
									pMeta->Data.YCbCr.planar.Cr.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Cr.PicWidthBytes = (w / 2) * ((bits > 8) ? 2 : 1);
								} else if (pMeta->Layout == PIC_BUF_LAYOUT_COMBINED) {
									pMeta->Data.YCbCr.combined.PicWidthPixel = w;
									pMeta->Data.YCbCr.combined.PicHeightPixel = h;
									pMeta->Data.YCbCr.combined.PicWidthBytes = w * 2 * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.combined.bitWidth = bits;
								}
								break;
							case PIC_BUF_TYPE_YCbCr420:
								if (pMeta->Layout == PIC_BUF_LAYOUT_SEMIPLANAR) {
									pMeta->Data.YCbCr.semiplanar.Y.PicWidthPixel = w;
									pMeta->Data.YCbCr.semiplanar.Y.PicHeightPixel = h;
									pMeta->Data.YCbCr.semiplanar.Y.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.semiplanar.Y.bitWidth = bits;
									pMeta->Data.YCbCr.semiplanar.CbCr.PicWidthPixel = w;
									pMeta->Data.YCbCr.semiplanar.CbCr.PicHeightPixel = h / 2;
									pMeta->Data.YCbCr.semiplanar.CbCr.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.semiplanar.CbCr.bitWidth = bits;
								} else if (pMeta->Layout == PIC_BUF_LAYOUT_PLANAR) {
									pMeta->Data.YCbCr.planar.Y.PicWidthPixel = w;
									pMeta->Data.YCbCr.planar.Y.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Y.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.planar.Y.bitWidth = bits;
									pMeta->Data.YCbCr.planar.Cb.PicWidthPixel = w / 2;
									pMeta->Data.YCbCr.planar.Cb.PicHeightPixel = h / 2;
									pMeta->Data.YCbCr.planar.Cb.PicWidthBytes = (w / 2) * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.planar.Cr.PicWidthPixel = w / 2;
									pMeta->Data.YCbCr.planar.Cr.PicHeightPixel = h / 2;
									pMeta->Data.YCbCr.planar.Cr.PicWidthBytes = (w / 2) * ((bits > 8) ? 2 : 1);
								}
								break;
							case PIC_BUF_TYPE_YCbCr444:
								if (pMeta->Layout == PIC_BUF_LAYOUT_PLANAR) {
									pMeta->Data.YCbCr.planar.Y.PicWidthPixel = w;
									pMeta->Data.YCbCr.planar.Y.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Y.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.planar.Y.bitWidth = bits;
									pMeta->Data.YCbCr.planar.Cb.PicWidthPixel = w;
									pMeta->Data.YCbCr.planar.Cb.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Cb.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.planar.Cr.PicWidthPixel = w;
									pMeta->Data.YCbCr.planar.Cr.PicHeightPixel = h;
									pMeta->Data.YCbCr.planar.Cr.PicWidthBytes = w * ((bits > 8) ? 2 : 1);
								} else if (pMeta->Layout == PIC_BUF_LAYOUT_COMBINED) {
									pMeta->Data.YCbCr.combined.PicWidthPixel = w;
									pMeta->Data.YCbCr.combined.PicHeightPixel = h;
									pMeta->Data.YCbCr.combined.PicWidthBytes = w * 3 * ((bits > 8) ? 2 : 1);
									pMeta->Data.YCbCr.combined.bitWidth = bits;
								}
								break;
							case PIC_BUF_TYPE_RAW8:
							case PIC_BUF_TYPE_RAW10:
							case PIC_BUF_TYPE_RAW12:
							case PIC_BUF_TYPE_RAW14:
							case PIC_BUF_TYPE_RAW16:
							case PIC_BUF_TYPE_RAW24:
								pMeta->Data.raw.PicWidthPixel = w;
								pMeta->Data.raw.PicHeightPixel = h;
								pMeta->Data.raw.PicWidthBytes = pBuf->baseSize / h;
								pMeta->Data.raw.bitWidth = bits;
								break;
							default:
								break;
							}
						}
#endif
						// Increment the dequeue call counter
						dequeue_call_count++;
						VsiVvdeviceShowBuffer(pVvbenchInstance, pBuf, index, i);
						if (tempBuf[index][i] != NULL) {
							result = VsiCamDeviceEnQueBuffer(pVvbenchInstance->hCamDevice[index], i, tempBuf[index][i]);
							if (RET_SUCCESS != result) {
								LOGW("%s EQBUF failed(%d)!", __func__, result);
								return -1;
							}
						}
						tempBuf[index][i] = pBuf;
						//	break;
					}
				}
			}
			if (displayInstance == 15) {
				LOGI("------------------------------------------------------------\r\n");
				LOGI(" |  📥 To retrieve the image dump, use the following command:\r\n");
				if(caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_MP].pathEnable) {
					u64 dumpAddr = (u64)pBuf->baseAddress | ((u64)0x8 << 32);
					LOGI(" |  mrd -bin -file dump_MP.rgb 0x%llx 0x%x\r\n\n", dumpAddr, pBuf->baseSize / 4);
				}
				if(caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_SP1].pathEnable) {
					u64 dumpAddr = (u64)pBuf->baseAddress | ((u64)0x8 << 32);
					LOGI(" |  mrd -bin -file dump_SP.rgb 0x%llx 0x%x\r\n\n", dumpAddr, pBuf->baseSize / 4);
				}
				LOGI("------------------------------------------------------------\r\n");
				displayInstance = 0;
			}
		}
	}
#else
	if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[0].inputType) {
		while (1) {
#ifdef XPAR_XV_MIX_NUM_INSTANCES
			VmixHdmiBridge_HdmiPoll();
#endif
		}
	}
#endif
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	start = GetTime();
#endif
	if (fbwr_flag == 1 && memory_out_flag == -1) {
		while (1) {
#ifdef XPAR_XV_MIX_NUM_INSTANCES
			/* Poll HDMI TX state machine for event-driven stream start */
			VmixHdmiBridge_HdmiPoll();
#endif
			now = GetTime();
			u64 elapsed_ticks = now - start;
			double elapsed_seconds = (double)elapsed_ticks / freq;

			if (elapsed_seconds >= 5.0) {
				LOGI("Stream FPS = %dfps", dequeue_call_count/5);
				xil_printf("================================\n");
				// Reset counters for next second
				start = GetTime();
				dequeue_call_count = 0;
				if (displayInstance == 3) {
					LOGI("------------------------------------------------------------\r\n");
					LOGI(" |  📥 To retrieve the image dump, use the following command:\r\n");
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
					if (caseCtx->instanceCfgCtx[ISP_ID].instancePath[CAMDEV_BUFCHAIN_MP].pathEnable) {
						LOGI(" |  mrd -bin -file lilo_ISP%d_MP.rgb 0x%llx 0x17BC00\r\n\n",
						     ISP_ID, (unsigned long long)(uintptr_t)Frame_Array_p[ISP_ID * 2][0].aligned_addr);
					}
					if (caseCtx->instanceCfgCtx[ISP_ID].instancePath[CAMDEV_BUFCHAIN_SP1].pathEnable) {
						LOGI(" |  mrd -bin -file lilo_ISP%d_SP.rgb 0x%llx 0x17BC00\r\n\n",
						     ISP_ID, (unsigned long long)(uintptr_t)Frame_Array_p[ISP_ID * 2 + 1][0].aligned_addr);
					}
#endif
					LOGI("------------------------------------------------------------\r\n");
					displayInstance = 0;
				}
				displayInstance++;
			}
		}
	}


	pBenchInstance = pVvbenchInstance;
	caseContext = caseCtx;
	instanceNumber = totalInstance;
	isVvdeviceReleased = false;

	//Path Switch
	int index = 0;
	if (caseCtx->instanceCfgCtx[index].funcCtrl.usePathSwitch) {

		//Test path switch
		if (caseCtx->instanceCfgCtx[index].pathSwitchCfg.pathIdLength) {
			for (int i = 0; i < caseCtx->instanceCfgCtx[index].pathSwitchCfg.pathIdLength; i++) {
				CamDevicePipeOutPathType_t path = (CamDevicePipeOutPathType_t)(
						caseCtx->instanceCfgCtx[index].pathSwitchCfg.pathId[i]);
				result = VsiVvdevicePathModify(pVvbenchInstance, caseCtx, 0, path);
				if (0 != result) {
					LOGE("Vsi camdevice Path Switch failed!");
					return -1;
				}
			}
		}

		//Test dynamic switch crop windows
		if (caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropWindowLength) {
			CamDevicePipeOutPathType_t path = (CamDevicePipeOutPathType_t)(
					caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropPathId);

			for (int i = 0; i < caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropWindowLength; i++) {
				VsiVvdeviceDelay(5);
				//crop window set
				CamDevicePipeIspWindow_t ispWindow;
				MEMSET(&ispWindow, 0, sizeof(ispWindow));
				ispWindow.cropWindow.hOffset = caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropWindowHOffset[i];
				ispWindow.cropWindow.vOffset = caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropWindowVOffset[i];
				ispWindow.cropWindow.width = caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropWindowWidth[i];
				ispWindow.cropWindow.height = caseCtx->instanceCfgCtx[index].pathSwitchCfg.cropWindowHeight[i];

				if ((ispWindow.cropWindow.width != 0) && (ispWindow.cropWindow.height != 0)) {
					result = VsiCamDeviceSetIspWindow(pVvbenchInstance->hCamDevice[index], path, &ispWindow);
					if (0 != result) {
						LOGE("Vsi camdevice dynamic switch crop windows failed!");
						return -1;
					}
				}
			}
		}
	}

	if (caseCtx->instanceCfgCtx[0].funcCtrl.useFastResetFunction) {
		LOGI("vvbench start fast reset");
		VsiVvdeviceDelay(3);
		for (uint32_t iLoop = 0; iLoop < caseCtx->instanceCfgCtx[0].fastResetCfg.fastResetLoop; ++iLoop) {
			for (int iInstance = 0; iInstance < totalInstance; iInstance++) {
				if (caseCtx->instanceCfgCtx[iInstance].instanceEnable == 0) {
					LOGI("Skip fast reset stop for instance %d", iInstance);
					continue;
				}
				result = VsiCamDeviceSwFastStop(pVvbenchInstance->hCamDevice[iInstance]);
				if (0 != result) {
					LOGE("fastReset for instance: %d, VsiCamDeviceSwFastStop failed! result:%d", iInstance, result);
					return -1;
				}
				else {
					LOGI("success call fast reset stop for instance %d", iInstance);
				}
			}

			for (int iInstance = 0; iInstance < totalInstance; iInstance++) {
				if (caseCtx->instanceCfgCtx[iInstance].instanceEnable == 0) {
					LOGI("Skip fast reset hw system reset for instance %d", iInstance);
					continue;
				}
				result = VsiCamDeviceHwSystemReset(pVvbenchInstance->hCamDevice[iInstance]);
				if (0 != result) {
					LOGE("fastReset VsiCamDeviceHwSystemReset failed! result:%d", result);
					return -1;
				}
				else {
					LOGI("success call fast reset hw system reset for instance %d", iInstance);
				}
			}

			for (int iInstance = 0; iInstance < totalInstance; iInstance++) {
				if (caseCtx->instanceCfgCtx[iInstance].instanceEnable == 0) {
					LOGI("Skip fast reset start for instance %d", iInstance);
					continue;
				}
				result = VsiCamDeviceSwFastStart(pVvbenchInstance->hCamDevice[iInstance]);
				if (0 != result) {
					LOGE("fastReset for instance: %d, VsiCamDeviceSwFastStart failed! result:%d", iInstance, result);
					return -1;
				}
				else {
					LOGI("success call fast reset start for instance %d", iInstance);
				}
			}
			VsiVvdeviceDelay(5);
		}
		LOGI("vvbench end fast reset");
	}

	LOGI("vvbench stream on duration:%d", cfgCtx->streamDuration);
	VsiVvdeviceDelay(cfgCtx->streamDuration);

	//Stop
	result = VsiVvdeviceStop(false);
	if (0 != result) {
		LOGE("Vsi vvdevice stop failed !");
		mm_free(pVvbenchInstance);
		return -1;
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceStop(const bool stopImmediately)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if ((!instanceNumber) || isVvdeviceReleased) {
		return 0;
	}
	else {
		isVvdeviceReleased = true;
	}

	if (NULL == pBenchInstance || NULL == caseContext) {
		return -1;
	}

	VsiVvdeviceDelay(1);
	LOGI("vvbench end of stream");

	//Stop INSTANCEs
	for (int index = 0; index < instanceNumber; index++) {
		if (caseContext->instanceCfgCtx[index].instanceEnable == 0) {
			LOGD("Skip instance %d", index);
			continue;
		}

#ifdef APU_CORE
		if (selectDestinationCore(caseContext->instanceCfgCtx[index].hpId) != XST_SUCCESS) {
			LOGE("selectDestinationCore failed for ISP %d", caseContext->instanceCfgCtx[index].hpId);
			continue;
		}
#endif

		LOGD("End instance %d", index);

		// Disable Path
		CamDeviceBufChainId_t bufIoArray[CAMDEV_BUFCHAIN_MAX];
		int bufIoArrayCount = 0;
		for (int i = 0; i < CAMDEV_BUFCHAIN_MAX; ++i) {
			if (caseContext->instanceCfgCtx[index].instancePath[i].pathState == ENABLE) {
				CamDeviceBufChainId_t bufIo = caseContext->instanceCfgCtx[index].instancePath[i].path;
				bufIoArray[bufIoArrayCount++] = bufIo;
			}
		}


		if (caseContext->instanceCfgCtx[index].stopPathSimultaneous) {
			LOGI("VsiVvdevicePathDisable called Simultaneously");
			result = VsiVvdevicePathDisable(pBenchInstance, caseContext, index, bufIoArray, bufIoArrayCount);
			if (result != 0) {
				LOGE("VsiVvdevicePathDisable failed");
				return -1;
			}
		}
		else {
			for (int i = 0; i < bufIoArrayCount; i++) {
				result = VsiVvdevicePathDisable(pBenchInstance, caseContext, index, bufIoArray + i, 1);
				if (result != 0) {
					LOGE("VsiVvdevicePathDisable failed for bifio:%d", *(bufIoArray + i));
					return -1;
				}
			}
		}

		//Unregister 3A Lib
		result = VsiVvbenchUnRegister3ALib(pBenchInstance->hCamDevice[index], caseContext, index);
		if (0 != result) {
			LOGE("Vsi cam device register 3A Lib failed !");
			return -1;
		}

		//ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT
		{
			result = VsiCamDeviceDisconnectCamera(pBenchInstance->hCamDevice[index]);
			if (0 != result) {
				LOGE("Vsi cam device disconnect camera failed!");
				return -1;
			}
		}

		if (CAMDEV_INPUT_TYPE_SENSOR == caseContext->instanceCfgCtx[index].inputType) {
			result = VsiCamDeviceSensorClose(pBenchInstance->hCamDevice[index]);
			if (result != 0) {
				LOGE("Vsi cam device sensor close failed %d !!", result);
				return -1;
			}
			result = VsiCamDeviceSensorDrvHandleUnRegister(pBenchInstance->hCamDevice[index]);
			if (result != 0) {
				LOGE("Vsi cam device sensor driver un-register failed %d !!", result);
				return -1;
			}
		}

		//deattach buffer chain
		for (int i = 0; i < CAMDEV_BUFCHAIN_MAX; ++i) {
			if (caseContext->instanceCfgCtx[index].instancePath[i].pathState == UN_INIT) {
				continue;
			}
			else {
				CamDeviceBufChainId_t bufIo = caseContext->instanceCfgCtx[index].instancePath[i].path;

				result = VsiVvdevicePathRelease(pBenchInstance, caseContext, index, bufIo);
				if (result != 0) {
					LOGE("VsiVvdevicePathRelease failed for bifio:%d", bufIo);
					return -1;
				}
			}
		}

		result = VsiCamDeviceDestroy(pBenchInstance->hCamDevice[index]);
		if (0 != result) {
			LOGE("releaseHardware camera error return");
			return -1;
		}

		//release CamCommon
		result = VsiCamCommonDestroy(&caseContext->instanceCfgCtx[index].hCamCommon);
		if (0 != result) {
			LOGE("VsiCamCommonDestroy error,  exit");
			result = -1;
		}
	}

	VsiVvdeviceInstanceDeinit(pBenchInstance);
	mm_free(pBenchInstance);
	pBenchInstance = NULL;
	caseContext = NULL;
#ifdef XPAR_XV_MIX_NUM_INSTANCES
	VmixHdmiBridge_Cleanup();
#endif
	LOGI("%s exit \n", __func__);
	return result;
}


int VsiVvdeviceCalibControl
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);

	CamDeviceCalibIllumType_t calibIllum;
	if (!caseCtx->fineTuneMode) {
		char fileName[FILE_LEN] = {0};
		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType &&
		    strlen(caseCtx->instanceCfgCtx[index].sensorCfg.calibrationName) != 0) {
			MEMCPY(fileName, caseCtx->instanceCfgCtx[index].sensorCfg.calibrationName,
			       sizeof(caseCtx->instanceCfgCtx[index].sensorCfg.calibrationName));
		}
		else if (CAMDEV_INPUT_TYPE_IMAGE == caseCtx->instanceCfgCtx[index].inputType &&
			 strlen(caseCtx->instanceCfgCtx[index].pictureCfg.calibXml) != 0) {
			MEMCPY(fileName, caseCtx->instanceCfgCtx[index].pictureCfg.calibXml,
			       sizeof(caseCtx->instanceCfgCtx[index].pictureCfg.calibXml));
		}
		else {
			LOGW("Vsi calib control wrong input type or file null, exit");
			return 1;
		}

		result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon, fileName);
		LOGI("%s VsiCamCommonParseFile %s \n", __func__, fileName);
		if (0 != result) {
			LOGE("Cam Common parse Vsi calibration error, exit");
			return result;
		}
	}


	result = VsiVvdeviceGetIllumType(pVvInstance->hCamDevice[index], caseCtx, index, &calibIllum);
	if (0 != result) {
		LOGE("Vsi camdevice get illum type failed:%s",
		     caseCtx->instanceCfgCtx[index].sensorCfg.calibrationName);
		return -1;
	}


	result = VsiCamCommonLoadCalibration(&caseCtx->instanceCfgCtx[index].hCamCommon,
					     pVvInstance->hCamDevice[index], calibIllum);
	if (0 != result) {
		LOGE("Vsi camdevice calibration load and init failed for xml file:%s",
		     caseCtx->instanceCfgCtx[index].sensorCfg.calibrationName);
		return -1;
	}

	LOGI("%s exit \n", __func__);

	return result;
}

int VsiVvbenchRegister3ALib
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	//AWB
	if (caseCtx->instanceCfgCtx[index].moduleCfg.awb.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiCamCommonRegisterAwbLib(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("VsiCamCommonRegisterAwbLib failed!");
			return -1;
		}
	}

	//AEC
	if ((caseCtx->instanceCfgCtx[index].moduleCfg.ae.isSupport || caseCtx->fineTuneMode) &&
	    caseCtx->instanceCfgCtx[index].inputType == CAMDEV_INPUT_TYPE_SENSOR) {
		result = VsiCamCommonRegisterAeLib(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("VsiCamCommonRegisterAeLib failed!");
			return -1;
		}
	}

	//AF
	if ((caseCtx->instanceCfgCtx[index].moduleCfg.af.isSupport || caseCtx->fineTuneMode) &&
	    caseCtx->instanceCfgCtx[index].inputType == CAMDEV_INPUT_TYPE_SENSOR) {
		result = VsiCamCommonRegisterAfLib(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("VsiCamCommonRegisterAfLib failed!");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchUnRegister3ALib
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	//AWB
	if (caseCtx->instanceCfgCtx[index].moduleCfg.awb.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiCamCommonUnRegisterAwbLib(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("VsiCamCommonUnRegisterAwbLib failed!");
			return -1;
		}
	}

	//AEC
	if ((caseCtx->instanceCfgCtx[index].moduleCfg.ae.isSupport || caseCtx->fineTuneMode) &&
	    caseCtx->instanceCfgCtx[index].inputType == CAMDEV_INPUT_TYPE_SENSOR) {
		result = VsiCamCommonUnRegisterAeLib(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("VsiCamCommonUnRegisterAeLib failed!");
			return -1;
		}
	}

	//AF
	if ((caseCtx->instanceCfgCtx[index].moduleCfg.af.isSupport || caseCtx->fineTuneMode) &&
	    caseCtx->instanceCfgCtx[index].inputType == CAMDEV_INPUT_TYPE_SENSOR) {
		result = VsiCamCommonUnRegisterAfLib(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("VsiCamCommonUnRegisterAfLib failed!");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceLoadSimulatorToDatabase
(
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	//Get submodules info from t_database
	char manualJson[FILE_LEN];
	char autoJson[FILE_LEN];
	if(tuning_json == 0)
	{
		strcpy(manualJson, "vvbcfg/project_json_file/manual_ext.json");
		strcpy(autoJson, "vvbcfg/project_json_file/auto.json");
		/* Override JSON files based on sensorName */
		const char *sensorName = caseCtx->instanceCfgCtx[index].sensorCfg.sensorName;
		if (strcmp(sensorName, "ox05b1s") == 0) {
			strcpy(autoJson, "auto_ox05b10.json");
			strcpy(manualJson, "vvbcfg/project_json_file/manual_ext.json");
			LOGI("Sensor %s detected, using autoJson: %s", sensorName, autoJson);
		}
		if (strcmp(sensorName, "ox08b40") == 0) {
			strcpy(autoJson, "vvbcfg/project_json_file/auto.json");
			strcpy(manualJson, "manual_ext_0x08b40.json");
			LOGI("Sensor %s detected, using manualJson: %s", sensorName, manualJson);
		}
	}
	else {
		strcpy(manualJson, "ext_manual.json");
		strcpy(autoJson, "ext_auto.json");
	}


	char simulatorJson[FILE_LEN] = {0};

	if (!caseCtx->fineTuneMode) {
		result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon, manualJson);
		LOGI("%s VsiCamCommonParseFile %s \n", __func__, manualJson);
		if (0 != result) {
			LOGE("Cam Common parse manual JSON error, exit");
			return result;
		}
		result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon, autoJson);
		LOGI("%s VsiCamCommonParseFile %s \n", __func__, autoJson);
		if (0 != result) {
			LOGE("Cam Common parse auto JSON error, exit");
			return result;
		}
		if (caseCtx->instanceCfgCtx[index].sensorCfg.simulatorName[0] != '\0') {
			MEMSET(simulatorJson, '0', sizeof(simulatorJson));
			MEMCPY(simulatorJson, caseCtx->instanceCfgCtx[index].sensorCfg.simulatorName,
			       sizeof(caseCtx->instanceCfgCtx[index].sensorCfg.simulatorName));
			result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon, simulatorJson);
			LOGI("%s VsiCamCommonParseFile %s \n", __func__, simulatorJson);
			if (0 != result) {
				LOGE("Cam Common parse sensor simulator JSON error, exit");
				return result;
			}
		}
		if (caseCtx->instanceCfgCtx[index].sensorCfg.autoSimulatorName[0] != '\0') {
			MEMSET(simulatorJson, '0', sizeof(simulatorJson));
			MEMCPY(simulatorJson, caseCtx->instanceCfgCtx[index].sensorCfg.autoSimulatorName,
			       sizeof(caseCtx->instanceCfgCtx[index].sensorCfg.autoSimulatorName));
			result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon, simulatorJson);
			LOGI("%s VsiCamCommonParseFile %s \n", __func__, simulatorJson);
			if (0 != result) {
				LOGE("Cam Common parse sensor auto simulator JSON error, exit");
				return result;
			}
		}
	}
	else {
		caseCtx->swSimuCfg.enable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.enable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.awbEnable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.cacEnable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.ccEnable = caseCtx->fineTuneMode;
		caseCtx->swSimuCfg.autoCfg.lscEnable = caseCtx->fineTuneMode;
		if ('\0' != caseCtx->instanceCfgCtx[index].fineTuneJson[0]) {
			result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon,
						       caseCtx->instanceCfgCtx[index].fineTuneJson);
			LOGI("%s VsiCamCommonParseFile %s \n", __func__, caseCtx->instanceCfgCtx[index].fineTuneJson);
			if (0 != result) {
				LOGE("Cam Common parse fine tune JSON error, exit");
				return result;
			}
		}
		else {
			LOGE("Fine tune JSON file path is empty!");
			return result;
		}
	}
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceLoadImageJsonToDatabase
(
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	CamCommonContext_t *pCamCommonCtx = (CamCommonContext_t *)caseCtx->instanceCfgCtx[index].hCamCommon;

	result = VsiCamCommonParseFile(&caseCtx->instanceCfgCtx[index].hCamCommon,
				       caseCtx->instanceCfgCtx[index].pictureCfg.fileName);
	LOGI("%s VsiCamCommonParseFile %s \n", __func__,
	     caseCtx->instanceCfgCtx[index].pictureCfg.fileName);
	if (0 != result) {
		LOGE("Cam Common parse swSimu config file error, exit");
		return result;
	}

	TDatabaseMetaDriver_t *pDbConfig = NULL;
	result = TDatabase_query(pCamCommonCtx->hDatabase, T_DATABASE_META_DRIVER, (void**)&pDbConfig);

	caseCtx->instanceCfgCtx[index].pictureCfg.width = pDbConfig->input.width;
	caseCtx->instanceCfgCtx[index].pictureCfg.height = pDbConfig->input.height;
	if ((pDbConfig->input.frames) <=
	    (pDbConfig->input.startFrame)) {  // pDbConfig->input.frames now be treat as endFrame
		LOGE("%s error:endFrame must bigger than startFrame", __func__);
		return -1;
	}
	else {
		caseCtx->instanceCfgCtx[index].pictureCfg.frameNum = pDbConfig->input.frames -
			(pDbConfig->input.startFrame);

#if defined(HAL_CMODEL) || defined(DUMP_IMAGE)
		caseCtx->frameIndex[index] = pDbConfig->input.startFrame;
#endif
	}

	//add image frame into picturelist
	osFile *swSimuList = NULL;
	char szBuffer[FILE_LEN];
	swSimuList = osFopen(caseCtx->instanceCfgCtx[index].pictureCfg.pictureName, "w");
	if (NULL == swSimuList) {
		LOGE(" open file fail: %s,  exit", caseCtx->instanceCfgCtx[index].pictureCfg.pictureName);
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

	LOGI("%s exit \n", __func__);
	return result;
}


int VsiVvdeviceSubModuleControl
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	//AWB
	if (caseCtx->instanceCfgCtx[index].moduleCfg.awb.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchAwbFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchAwbFunc failed!");
			return -1;
		}
	}

	if (caseCtx->instanceCfgCtx[index].inputType != CAMDEV_INPUT_TYPE_IMAGE) {
		//AEC
		if (caseCtx->instanceCfgCtx[index].moduleCfg.ae.isSupport || caseCtx->fineTuneMode) {
			result = VsiVvbenchAeFunc(hCamDevice, caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvbenchAeFunc failed!");
				return -1;
			}
		}

		//AF
		if (caseCtx->instanceCfgCtx[index].moduleCfg.af.isSupport || caseCtx->fineTuneMode) {
			result = VsiVvbenchAfFunc(hCamDevice, caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvbenchAeFunc failed!");
				return -1;
			}

			//PDAF
			result = VsiVvbenchPdafFunc(hCamDevice, caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvbenchPdafFunc failed!");
				return -1;
			}
		}
	}
	else {
		if (caseCtx->swSimuCfg.enable) {
			result = VsiVvbenchPdafFunc(hCamDevice, caseCtx, index);
			if (0 != result) {
				LOGE("VsiVvbenchPdafFunc failed!");
				return -1;
			}
		}
	}

	//2DNR
	if (caseCtx->instanceCfgCtx[index].moduleCfg.dnr2.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbench2DnrFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbench2DnrFunc failed!");
			return -1;
		}
	}
	//3DNR
	if (caseCtx->instanceCfgCtx[index].moduleCfg.dnr3.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbench3DnrFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbench3DnrFunc failed!");
			return -1;
		}
	}
	//LSC
	if (caseCtx->instanceCfgCtx[index].moduleCfg.lsc.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchLscFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VvbenchLscFunc failed!");
			return -1;
		}
	}
	//GC
	if (caseCtx->instanceCfgCtx[index].moduleCfg.gc.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchGcFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchGcFunc failed!");
			return -1;
		}
	}
	//EE
	if (caseCtx->instanceCfgCtx[index].moduleCfg.ee.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchEeFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchEeFunc failed!");
			return -1;
		}
	}
	//GE
	if (caseCtx->instanceCfgCtx[index].moduleCfg.ge.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchGeFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchGeFunc failed!");
			return -1;
		}
	}
	//GTM
	if (caseCtx->instanceCfgCtx[index].moduleCfg.gtm.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchGtmFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchGtmFunc failed!");
			return -1;
		}
	}
	//DPCC
	if (caseCtx->instanceCfgCtx[index].moduleCfg.dpcc.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchDpccFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchDpccFunc failed!");
			return -1;
		}
	}

	//DEMOSAIC
	if (caseCtx->instanceCfgCtx[index].moduleCfg.dmsc.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchDmscFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchDmscFunc failed!");
			return -1;
		}
	}
	//RGBIR
	if (caseCtx->instanceCfgCtx[index].moduleCfg.rgbir.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchRgbirFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchRgbirFunc failed!");
			return -1;
		}
	}
	//HDR
	if (caseCtx->instanceCfgCtx[index].moduleCfg.hdr.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchHdrFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchHdrFunc failed!");
			return -1;
		}
	}
	//CPD
	if (caseCtx->instanceCfgCtx[index].moduleCfg.cpd.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchCpdFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchCpdFunc failed!");
			return -1;
		}
	}
	//BLS
	if (caseCtx->instanceCfgCtx[index].moduleCfg.bls.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchBlsFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchBlsFunc failed!");
			return -1;
		}
	}
	//GWDR
	// if (caseCtx->instanceCfgCtx[index].moduleCfg.gwdr.isSupport || caseCtx->swSimuCfg.enable) {
	// result = VsiVvbenchGWdrFunc(hCamDevice, caseCtx, index);
	// if (0 != result) {
	// LOGE("VsiVvbenchGWdrFunc failed!");
	// return -1;
	// }
	// }
	//WDR
	if (caseCtx->instanceCfgCtx[index].moduleCfg.wdr.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchWdrFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchWdrFunc failed!");
			return -1;
		}
	}
	//WB
	if (caseCtx->instanceCfgCtx[index].moduleCfg.wb.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchWbFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchWbFunc failed!");
			return -1;
		}
	}
	//DG
	if (caseCtx->instanceCfgCtx[index].moduleCfg.dg.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchDgFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchDgFunc failed!");
			return -1;
		}
	}
	//CCM
	if (caseCtx->instanceCfgCtx[index].moduleCfg.ccm.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchCcmFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchCcmFunc failed!");
			return -1;
		}
	}
	//CPROC
	if (caseCtx->instanceCfgCtx[index].moduleCfg.cproc.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchCprocFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchCprocFunc failed!");
			return -1;
		}
	}
	//CNR
	if (caseCtx->instanceCfgCtx[index].moduleCfg.cnr.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchCnrFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchCnrFunc failed!");
			return -1;
		}
	}
	//YNR
	// if (caseCtx->instanceCfgCtx[index].moduleCfg.ynr.isSupport || caseCtx->swSimuCfg.enable) {
	// result = VsiVvbenchYnrFunc(hCamDevice, caseCtx, index);
	// if (0 != result) {
	// LOGE("VsiVvbenchYnrFunc failed!");
	// return -1;
	// }
	// }
	// //LUT3D
	if (caseCtx->instanceCfgCtx[index].moduleCfg.lut3d.isSupport || caseCtx->swSimuCfg.enable) {
		result = VsiVvbenchLut3dFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchLut3dFunc failed!");
			return -1;
		}
	}
	//EXPV2
	if (caseCtx->instanceCfgCtx[index].moduleCfg.expv2.isSupport) {
		result = VsiVvbenchExpv2Func(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchExpv2Func failed!");
			return -1;
		}
	}
	//EXPV3
	if (caseCtx->instanceCfgCtx[index].moduleCfg.expv3.isSupport) {
		result = VsiVvbenchExpv3Func(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchExpv3Func failed!");
			return -1;
		}
	}
	//AFM
	if (caseCtx->instanceCfgCtx[index].moduleCfg.afm.isSupport) {
		result = VsiVvbenchAfmFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchAfmFunc failed!");
			return -1;
		}
	}
	//AFM
	if (caseCtx->instanceCfgCtx[index].moduleCfg.afmv3.isSupport) {
		result = VsiVvbenchAfmv3Func(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvbenchAfmv3Func failed!");
			return -1;
		}
	}

	//FLEXA
#ifdef WITH_FLEXA
	if (caseCtx->instanceCfgCtx[index].funcCtrl.useFlexaFunction) {
		result = VsiVvdeviceFlexaFunc(hCamDevice, caseCtx, index);
		if (0 != result) {
			LOGE("VsiVvdeviceFlexaFunc failed!");
			return -1;
		}
	}
#endif
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceNrRelocControl
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx
)
{
	int result = 0;
	uint32_t instanceId = 0;
	CamCommonContext_t *pCamCommonCtx = NULL;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice || 0 == caseCtx->instanceCfgCtx[instanceId].hCamCommon || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}
	pCamCommonCtx = (CamCommonContext_t *)caseCtx->instanceCfgCtx[instanceId].hCamCommon;

	//NR post-position in ISP
	result = VsiCamDeviceNrRelocDisable(hCamDevice);
	if (0 != result) {
		LOGE("%s: Vvbench Disable Nr Reloc failed!\n", __func__);
		return -1;
	}

	TDatabaseMetaDriver_t *pMetaConfig = NULL;
	result = TDatabase_query(pCamCommonCtx->hDatabase, T_DATABASE_META_DRIVER, (void**)&pMetaConfig);

	//NR pre-position in ISP
	if (pMetaConfig->nrReloc) {
		result = VsiCamDeviceNrRelocEnable(hCamDevice);
		if (0 != result) {
			LOGE("%s: Vvbench Enable Nr Reloc failed!\n", __func__);
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceInstanceInit
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
#if defined(ISP_OFFLINE_TEST) || defined(HAL_CMODEL) || defined(DUMP_IMAGE)
	if (caseCtx->swSimuCfg.enable) {
		char temp[FILE_NAME] = "\0";
		strncpy(temp, caseCtx->caseName, sizeof(temp) - 1);
		temp[sizeof(temp) - 1] = '\0';
		sprintf(pVvInstance->caseName, "load_image_case/%s", temp);
	}
	else {
		strncpy(pVvInstance->caseName, caseCtx->caseName, sizeof(pVvInstance->caseName));
	}
	for (int instance = 0; instance < MAX_CAM_NUM; instance++) {
		pVvInstance->frameIndex[instance][CAMDEV_BUFCHAIN_MP] = caseCtx->frameIndex[instance];
	}
	pVvInstance->enableDump = false;
	if (pVvInstance->useTerminal) {
		pVvInstance->enableDump = false;
		LOGD("%s enableDump useTerminald set dump false\n", __func__);
	}
	LOGD("%s enableDump or not ?:%d \n", __func__, pVvInstance->enableDump);

#endif

	for (int instance = 0; instance < MAX_CAM_NUM; instance++) {
		for (int bufferIO = 0; bufferIO <= CAMDEV_BUFCHAIN_RAW; bufferIO++) {
			// pVvInstance->dom[instance][bufferIO] = (domCtrlHandle_t)NULL;
			pVvInstance->pBufferThreadCtx[instance][bufferIO] = NULL;
		}
	}

	if (caseCtx) {
		int totalInstance = caseCtx->totalInstance;
		for (int instance = 0; instance < totalInstance; instance++) {
			if (caseCtx->instanceCfgCtx[instance].instanceEnable) {
				for (int bufferIO = 0; bufferIO <= CAMDEV_BUFCHAIN_RAW; bufferIO++) {
					if (caseCtx->instanceCfgCtx[instance].instancePath[bufferIO].pathEnable) {
						bool allowSkip = false;
						if (DEFAULT_CASE_LIST == VsiVvbenchGetVvbenchRunMode() &&
						    (0 != strcmp(caseCtx->instanceCfgCtx[instance].sensorCfg.sensorName, "UserInput"))) {
							allowSkip = true;
						}
						// result = VsiVvdeviceInitDom(pVvInstance, instance,
						// (CamDeviceBufChainId_t)bufferIO, allowSkip);
						// if (0 != result) {
						// LOGE("%s: bufio %d init dom failed\n", __func__, bufferIO);
						// return result;
						// }
					}
				}
			}
		}
	}
	else {
		// result = VsiVvdeviceInitDom(pVvInstance, 0, CAMDEV_BUFCHAIN_MP, false);
		// if (0 != result) {
		// LOGE("%s: bufio %d init dom failed\n", __func__, CAMDEV_BUFCHAIN_MP);
		// return result;
		// }
	}
	sleep(3);
	LOGI("%s exit \n", __func__);

	return result;
}

void VsiVvdeviceInstanceDeinit
(
	VvbenchInstance_t *pVvInstance
)
{
	LOGI("%s enter \n", __func__);


	LOGI("%s exit \n", __func__);

}


RESULT VsiVvdeviceGetIllumType
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index,
	CamDeviceCalibIllumType_t *pllumType
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (hCamDevice == NULL || caseCtx == NULL || pllumType == NULL) {
		return -1;
	}

	if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "A")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_A;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "D50")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_D50;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "D65")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_D65;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "D75")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_D75;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "F2")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_F2;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "F11")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_F11;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "F12")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_F12;
	}
	else if (0 == strcmp(caseCtx->instanceCfgCtx[index].sensorCfg.illumType, "H")) {
		LOGI("Illumination type: %s ", caseCtx->instanceCfgCtx[index].sensorCfg.illumType);
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_H;
	}
	else {
		LOGI("Use default Illumination type: D50");
		*pllumType = CAMDEV_CALIB_ILLUM_TYPE_D50;
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceSensorFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}

	CamDeviceSensorMetadataAttr_t metaAttr;
	MEMSET(&metaAttr, 0, sizeof(CamDeviceSensorMetadataAttr_t));

	CamDeviceSensorMetadataWin_t metaWin;
	MEMSET(&metaWin, 0, sizeof(CamDeviceSensorMetadataWin_t));

	//TODO:Need update the control which  metadata info need enable
	metaAttr.subAttr.again = 1;
	metaAttr.subAttr.dgain = 1;
	metaAttr.subAttr.expTime = 1;
	result = VsiCamDeviceSensorSetMetadataAttr(hCamDevice, metaAttr);
	if (result != RET_NOTSUPP && result != 0) {
		LOGE("vvbench VsiCamDeviceSensorSetMetadataAttr failed: %d", result);
		return -1;
	}
	if (result == RET_NOTSUPP) {
		LOGE("vvbench Sensor Metadata Attr Unsupported!");
		caseCtx->instanceCfgCtx[index].funcCtrl.useVi200MetaWin = 0;
		return RET_SUCCESS;
	}

	CamDeviceSensorMetadataAttr_t mtAttr;
	result = VsiCamDeviceSensorGetMetadataAttr(hCamDevice, &mtAttr);
	if (0 != result) {
		LOGE("vvbench VsiCamDeviceSensorGetMetadataAttr failed: %d", result);
		return -1;
	}

	result = VsiCamDeviceSensorGetMetadataWin(hCamDevice, &metaWin);
	if (0 != result) {
		LOGE("vvbench VsiCamDeviceSensorGetMetadataWin failed: %d", result);
		return -1;
	}

#if (defined ISP_VI200) ||defined(ISP_VI200_V2)|| (defined ISP_VI200_V2_1) || defined(ISP_VI200_V2_2)
	caseCtx->instanceCfgCtx[index].vi200Cfg.metaWin.winNum = metaWin.winNum;
	for (int i = 0; i < metaWin.winNum; ++i) {
		MEMCPY(&caseCtx->instanceCfgCtx[index].vi200Cfg.metaWin.metaWin[i], &metaWin.metaWin[i],
		       sizeof(CamDeviceWindow_t));
	}
#endif

	if (!caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_BUFCHAIN_METADATA].pathEnable) {
		caseCtx->instanceCfgCtx[index].funcCtrl.useVi200MetaWin = false;
	}

	LOGI("%s exit \n", __func__);
	return RET_SUCCESS;
}

int VsiVvdeviceGetStatisticControl
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if ((pVvInstance == NULL) || (caseCtx == NULL)) {
		LOGE("%s: NULL pointer", __func__);
		result = -1;
		return result;
	}

	//EXPV2 is ShowExpv2GetStatistic
	pVvInstance->isShowExpv2GetStatistic[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.expv2.enable) {
		pVvInstance->isShowExpv2GetStatistic[index] = true;
	}

	//EXPV3 is ShowExpv3GetStatistic
	pVvInstance->isShowExpv3GetStatistic[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.expv3.enable) {
		pVvInstance->isShowExpv3GetStatistic[index] = true;
	}

	//AFM is ShowAfmGetStatistic
	pVvInstance->isShowAfmGetStatistic[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.afm.enable) {
		pVvInstance->isShowAfmGetStatistic[index] = true;
	}

	//AFM is ShowAfmv3GetStatistic
	pVvInstance->isShowAfmv3GetStatistic[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.afmv3.enable) {
		pVvInstance->isShowAfmv3GetStatistic[index] = true;
	}

	//AWB is ShowAwbPerFrameInfo
	pVvInstance->isShowAwbPerFrameInfo[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.awb.enable) {
		pVvInstance->isShowAwbPerFrameInfo[index] = true;
	}

	//AE is ShowAePerFrameInfo
	pVvInstance->isShowAePerFrameInfo[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.ae.enable) {
		pVvInstance->isShowAePerFrameInfo[index] = true;
	}

	//Hist64 is isShowHist64GetStatistic
	pVvInstance->isShowHist64GetStatistic[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.hist64.enable) {
		pVvInstance->isShowHist64GetStatistic[index] = true;
	}

	//Hist256 is ShowHist256GetStatistic
	pVvInstance->isShowHist256GetStatistic[index] = false;
	if (caseCtx->instanceCfgCtx[index].moduleCfg.hist256.enable) {
		pVvInstance->isShowHist256GetStatistic[index] = true;
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceGetStatistic
(
	VvbenchInstance_t *pVvInstance,
	CamDeviceHandle_t hCamDevice,
	int instanceId
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if ((pVvInstance == NULL) || (hCamDevice == NULL)) {
		LOGE("%s: NULL pointer", __func__);
		result = -1;
		return result;
	}

	VvbenchExpV2Statistics_t ExpV2Statistics;
	if (pVvInstance->isShowExpv2GetStatistic[instanceId]) {
		result = VsiVvbenchExpv2GetStatistic(hCamDevice, &ExpV2Statistics);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchExpv2GetStatistic failed: %d", result);
			return -1;
		}
	}

#if 1
	if (pVvInstance->isShowExpv2GetStatistic[instanceId]) {
		char dump_buf[512];
		char szFileName[512];
		osFile *staFp = NULL;
		uint32_t *Aev2Static = (uint32_t *)ExpV2Statistics;
		snprintf(szFileName, sizeof(szFileName),
			 "vvbcfg/case/dump_image/%s_dump_instance%d_frame%d_exposure_statistics.txt", pVvInstance->caseName,
			 instanceId, showFrameNum);
		staFp = osFopen(szFileName, "w");
		if (staFp == NULL) {
			LOGE("vvbench osFopen exp statistic file failed: %d", result);
			return -1;
		}
		snprintf(dump_buf, 512, "EXPM DATA BELOW:\n");
		osFseek(staFp, 0, SEEK_SET);
		osFwrite(dump_buf, strlen(dump_buf), 1, staFp);
		for (uint32_t i = 0; i < sizeof(VvbenchExpV2Statistics_t) / 4; i++) {
			snprintf(dump_buf, 512, "%08x %08x\n", i, Aev2Static[i]);
			osFwrite(dump_buf, strlen(dump_buf), 1, staFp);
		}
		snprintf(dump_buf, 512, "EXPM DATA END\n");
		osFwrite(dump_buf, strlen(dump_buf), 1, staFp);
		Xil_DCacheFlush();
		osFclose(staFp);
		staFp = NULL;
	}
#endif

	VVbenchDeviceHist256Bin_t pVvHistBin;
	if (pVvInstance->isShowHist256GetStatistic[instanceId]) {
		result = VsiVvbenchHist256GetStatistic(hCamDevice, &pVvHistBin);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchHist256GetStatistic failed: %d", result);
			return -1;
		}
	}

#if 1
	if (pVvInstance->isShowHist256GetStatistic[instanceId]) {
		char dump_buf[512];
		char szFileName[512];
		osFile *staFp = NULL;
		uint32_t *Hist256Static = (uint32_t *)pVvHistBin;
		snprintf(szFileName, sizeof(szFileName),
			 "vvbcfg/case/dump_image/%s_dump_instance%d_frame%d_hist256_statistics.txt", pVvInstance->caseName,
			 instanceId, showFrameNum);
		staFp = osFopen(szFileName, "w");
		if (staFp == NULL) {
			LOGE("vvbench osFopen hist256 statistic file failed: %d", result);
			return -1;
		}
		snprintf(dump_buf, 512, "HISTM DATA BELOW:\n");
		osFseek(staFp, 0, SEEK_SET);
		osFwrite(dump_buf, strlen(dump_buf), 1, staFp);
		for (uint32_t i = 0; i < sizeof(VVbenchDeviceHist256Bin_t) / 4; i++) {
			snprintf(dump_buf, 512, "%08x %08x\n", i, Hist256Static[i]);
			osFwrite(dump_buf, strlen(dump_buf), 1, staFp);
		}
		snprintf(dump_buf, 512, "HISTM DATA END\n");
		osFwrite(dump_buf, strlen(dump_buf), 1, staFp);

		osFclose(staFp);
		staFp = NULL;
	}
#endif

	if (pVvInstance->isShowExpv3GetStatistic[instanceId]) {
		result = VsiVvbenchExpv3GetStatistic(hCamDevice);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchExpv3GetStatistic failed: %d", result);
			return -1;
		}
	}

	if (pVvInstance->isShowAfmGetStatistic[instanceId]) {
		result = VsiVvbenchAfmGetStatistic(hCamDevice);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchAfmGetStatistic failed: %d", result);
			return -1;
		}
	}

	if (pVvInstance->isShowAfmv3GetStatistic[instanceId]) {
		result = VsiVvbenchAfmv3GetStatistic(hCamDevice);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchAfmv3GetStatistic failed: %d", result);
			return -1;
		}
	}

	if (pVvInstance->isShowAwbPerFrameInfo[instanceId]) {
		result = VsiVvbenchAwbGetStatistic(hCamDevice);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchAwbGetStatistic failed: %d", result);
			return -1;
		}
		result = VsiVvbenchAwbGetResult(hCamDevice);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchAwbGetResult failed: %d", result);
			return -1;
		}
	}

	if (pVvInstance->isShowAePerFrameInfo[instanceId]) {
		result = VsiVvbenchAeGetResult(hCamDevice);
		if (result != 0) {
			LOGE("vvbench VsiVvbenchAeGetResult failed: %d", result);
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdevicePathModify
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == pVvInstance || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}

	if (caseCtx->instanceCfgCtx[instanceId].pathSwitchCfg.isBufferReCreate) {
		result = VsiVvdeviceOutPutPathCreate(pVvInstance, caseCtx, instanceId, bufIo);
		if (0 != result) {
			LOGE("VsiVvdevicePathCreate failed !");
			return -1;
		}
	}

	result = VsiVvdevicePathEnable(pVvInstance, caseCtx, instanceId, &bufIo, 1);
	if (0 != result) {
		LOGE("VsiVvdevicePathEnable failed !");
		return -1;
	}

	VsiVvdeviceDelay(5);

	result = VsiVvdevicePathDisable(pVvInstance, caseCtx, instanceId, &bufIo, 1);
	if (0 != result) {
		LOGE("VsiVvdevicePathDisable failed !");
		return -1;
	}

	VsiVvdeviceDelay(2);

	if (caseCtx->instanceCfgCtx[instanceId].pathSwitchCfg.isBufferReCreate) {
		result = VsiVvdevicePathRelease(pVvInstance, caseCtx, instanceId, bufIo);
		if (0 != result) {
			LOGE("VsiVvdevicePathRelease failed !");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

void VsiVvdeviceShowBuffer
(
	VvbenchInstance_t *pVvInstance,
	MediaBuffer_t *pBuffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
)
{
	if (NULL == pVvInstance || NULL == pBuffer) {
		return ;
	}
	uint8_t bufferNum = 1;

	for (int iBuf = 0; iBuf < bufferNum; ++iBuf) {
		MediaBuffer_t *pNowBuffer = pBuffer + iBuf;
		PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pNowBuffer->pMetaData);
		int width = 0;
		int height = 0;
		int yuvOrder = 0;
		int bufferSize = 0;
		int alignMode = 0;
		int format = 0;
		uint8_t bitWidth = 0;

		int widthBytes = 0;
		int cbCrWidthBytes = 0;
		int cbCrHeightPixel = 0;
		int cbWidthBytes = 0;
		int cbHeightPixel = 0;
		int crWidthBytes = 0;
		int crHeightPixel = 0;

		int ysize = 0;
		int offset = 0;
		bool_t type = 0;  // 0:yuv,1:raw
		char typeStr[64];


		if (showChannel >= MAX_CAM_NUM) {
			return;
		}

		bool formatIsVaild = true;

		switch (pPicBufMetaData->Type) {
			case PIC_BUF_TYPE_YCbCr444: {
					switch (pPicBufMetaData->Layout) {
						case PIC_BUF_LAYOUT_PLANAR: {
								format = (int)CAMDEV_PIX_FMT_YUV444P;
								alignMode = pPicBufMetaData->Data.YCbCr.planar.Y.PixelDataAlignMode;
								width = pPicBufMetaData->Data.YCbCr.planar.Y.PicWidthPixel;
								height = pPicBufMetaData->Data.YCbCr.planar.Y.PicHeightPixel;
								widthBytes = pPicBufMetaData->Data.YCbCr.planar.Y.PicWidthBytes;
								cbHeightPixel = pPicBufMetaData->Data.YCbCr.planar.Cb.PicHeightPixel;
								cbWidthBytes = pPicBufMetaData->Data.YCbCr.planar.Cb.PicWidthBytes;
								crWidthBytes = pPicBufMetaData->Data.YCbCr.planar.Cr.PicHeightPixel;
								crHeightPixel = pPicBufMetaData->Data.YCbCr.planar.Cr.PicWidthBytes;
								bufferSize = widthBytes * height + cbWidthBytes * cbHeightPixel + crWidthBytes * crHeightPixel;

								ysize = width * height;
								offset = ysize % ALIGN_16;
								sprintf(typeStr, "yuv444p");
								break;
							}
						case PIC_BUF_LAYOUT_COMBINED: {
								alignMode = pPicBufMetaData->Data.YCbCr.combined.PixelDataAlignMode;
								bitWidth = pPicBufMetaData->Data.YCbCr.combined.bitWidth;
								width = pPicBufMetaData->Data.YCbCr.combined.PicWidthPixel;
								height = pPicBufMetaData->Data.YCbCr.combined.PicHeightPixel;
								widthBytes = pPicBufMetaData->Data.YCbCr.combined.PicWidthBytes;
								bufferSize = widthBytes * height;

								switch (alignMode) {
									case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
											format = (int)CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0;
											sprintf(typeStr, "yuv444comb_alignMode0");
											break;
										}
									case PIC_BUF_DATA_UNALIGN_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV444I;
											sprintf(typeStr, "yuv444comb");
											break;
										}
									default: {
											formatIsVaild = false;
											break;
										}
								}
								break;
							}
						default: {
								formatIsVaild = false;
								break;
							}
					}
					break;
				}
			case PIC_BUF_TYPE_YCbCr422: {
					switch (pPicBufMetaData->Layout) {
						case PIC_BUF_LAYOUT_SEMIPLANAR: {
								alignMode = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PixelDataAlignMode;
								bitWidth = pPicBufMetaData->Data.YCbCr.semiplanar.Y.bitWidth;

								width = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthPixel;
								height = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicHeightPixel;
								widthBytes = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthBytes;

								cbCrWidthBytes = pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.PicWidthBytes;
								cbCrHeightPixel = pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.PicHeightPixel;

								bufferSize = widthBytes * height + cbCrWidthBytes * cbCrHeightPixel;
								ysize = width * height;
								offset = ysize % ALIGN_16;

								switch (alignMode) {
									case PIC_BUF_DATA_UNALIGN_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV422SP;
											sprintf(typeStr, "yuv422sp_unalign");
											break;
										}
									case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
											format = (int)CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0;
											sprintf(typeStr, "yuv422sp_alignMode0");
											break;
										}
									case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1;
											sprintf(typeStr, "yuv422sp_alignMode1");
											break;
										}
									default: {
											formatIsVaild = false;
											LOGE("%s: get invalid image format", __func__);
											break;
										}
								}
								break;
							}
						case PIC_BUF_LAYOUT_COMBINED: {
								alignMode = pPicBufMetaData->Data.YCbCr.combined.PixelDataAlignMode;
								bitWidth = pPicBufMetaData->Data.YCbCr.combined.bitWidth;
								width = pPicBufMetaData->Data.YCbCr.combined.PicWidthPixel;
								height = pPicBufMetaData->Data.YCbCr.combined.PicHeightPixel;
								widthBytes = pPicBufMetaData->Data.YCbCr.combined.PicWidthBytes;
								bufferSize = widthBytes * height;

								switch (alignMode) {
									case PIC_BUF_DATA_UNALIGN_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV422I;
											sprintf(typeStr, "yuv422comb_unalign");
											break;
										}
									case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV422I_ALIGNED_MODE1;
											sprintf(typeStr, "yuv422comb_alignMode1");
											break;
										}
									default: {
											formatIsVaild = false;
											break;
										}
								}
								break;
							}
						default: {
								formatIsVaild = false;
								break;
							}
					}
					break;
				}
			case PIC_BUF_TYPE_YCbCr420: {
					switch (pPicBufMetaData->Layout) {
						case PIC_BUF_LAYOUT_SEMIPLANAR: {
								alignMode = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PixelDataAlignMode;
								bitWidth = pPicBufMetaData->Data.YCbCr.semiplanar.Y.bitWidth;
								width = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthPixel;
								height = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicHeightPixel;
								widthBytes = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthBytes;
								cbCrWidthBytes = pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.PicWidthBytes;
								cbCrHeightPixel = pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.PicHeightPixel;

								bufferSize = widthBytes * height + cbCrWidthBytes * cbCrHeightPixel;
								ysize = width * height;
								offset = ysize % ALIGN_16;

								switch (alignMode) {
									case PIC_BUF_DATA_UNALIGN_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV420SP;
											sprintf(typeStr, "yuv420sp_unalign");
											break;
										}
									case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
											format = (int)CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0;
											sprintf(typeStr, "yuv420sp_alignMode0");
											break;
										}
									case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1;
											sprintf(typeStr, "yuv420sp_alignMode1");
											break;
										}
									default: {
											formatIsVaild = false;
											LOGE("%s: get invalid image format", __func__);
											break;
										}
								}
								break;
							}
						default: {
								formatIsVaild = false;
								break;
							}
					}
					break;
				}
			case PIC_BUF_TYPE_YCbCr400: {
					switch (pPicBufMetaData->Layout) {
						case PIC_BUF_LAYOUT_PLANAR: {
								alignMode = pPicBufMetaData->Data.YCbCr.planar.Y.PixelDataAlignMode;
								bitWidth = pPicBufMetaData->Data.YCbCr.planar.Y.bitWidth;

								width = pPicBufMetaData->Data.YCbCr.planar.Y.PicWidthPixel;
								height = pPicBufMetaData->Data.YCbCr.planar.Y.PicHeightPixel;
								widthBytes = pPicBufMetaData->Data.YCbCr.planar.Y.PicWidthBytes;

								cbHeightPixel = pPicBufMetaData->Data.YCbCr.planar.Cb.PicHeightPixel;
								cbWidthBytes = pPicBufMetaData->Data.YCbCr.planar.Cb.PicWidthBytes;
								crWidthBytes = pPicBufMetaData->Data.YCbCr.planar.Cr.PicHeightPixel;
								crHeightPixel = pPicBufMetaData->Data.YCbCr.planar.Cr.PicWidthBytes;
								bufferSize = widthBytes * height;
								ysize = width * height;
								offset = ysize % ALIGN_16;
								switch (alignMode) {
									case PIC_BUF_DATA_UNALIGN_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV400;
											sprintf(typeStr, "yuv400_unalign");
											break;
										}
									case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
											format = (int)CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE0;
											sprintf(typeStr, "yuv400_alignMode0");
											break;
										}
									case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
											format = (int)CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE1;
											sprintf(typeStr, "yuv400_alignMode1");
											break;
										}
									default: {
											formatIsVaild = false;
											LOGE("%s: get invalid image format", __func__);
											break;
										}
								}
								break;
							}
						default: {
								formatIsVaild = false;
								break;
							}
					}
					break;
				}
			case PIC_BUF_TYPE_RGB888: {
					if (pPicBufMetaData->Layout == PIC_BUF_LAYOUT_PLANAR) {
						width = pPicBufMetaData->Data.RGB.planar.R.PicWidthPixel;
						height = pPicBufMetaData->Data.RGB.planar.R.PicHeightPixel;
						format = (int)CAMDEV_PIX_FMT_RGB888P;
						bufferSize = width * height * 3;
						ysize = width * height;
						offset = ysize % ALIGN_16;
						sprintf(typeStr, "rgb888p");
					}
					else if (pPicBufMetaData->Layout == PIC_BUF_LAYOUT_COMBINED) {
						width = pPicBufMetaData->Data.RGB.combined.PicWidthPixel;
						height = pPicBufMetaData->Data.RGB.combined.PicHeightPixel;
						widthBytes = pPicBufMetaData->Data.RGB.combined.PicWidthBytes;
						bitWidth = pPicBufMetaData->Data.RGB.combined.bitWidth;
						alignMode = pPicBufMetaData->Data.RGB.combined.PixelDataAlignMode;
						yuvOrder = pPicBufMetaData->yuvOrder;
						format = (int)CAMDEV_PIX_FMT_RGB888;
						bufferSize = widthBytes * height;
						sprintf(typeStr, "rgb888");
					}
					break;
				}
			case PIC_BUF_TYPE_RAW8:
				width = pPicBufMetaData->Data.raw.PicWidthPixel;
				height = pPicBufMetaData->Data.raw.PicHeightPixel;
				widthBytes = pPicBufMetaData->Data.raw.PicWidthBytes;
				format = (int)CAMDEV_PIX_FMT_RAW8;
				bufferSize = widthBytes * height;
				sprintf(typeStr, "raw8");
				type = 1;
				break;
			case PIC_BUF_TYPE_RAW10: {
					width = pPicBufMetaData->Data.raw.PicWidthPixel;
					height = pPicBufMetaData->Data.raw.PicHeightPixel;
					widthBytes = pPicBufMetaData->Data.raw.PicWidthBytes;
					bufferSize = widthBytes * height;
					switch (pPicBufMetaData->Data.raw.PixelDataAlignMode) {
						case PIC_BUF_DATA_UNALIGN_MODE: {
								format = (int)CAMDEV_PIX_FMT_RAW10;
								sprintf(typeStr, "raw10_unalign");
								break;
							}
						case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
								format = (int)CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE0;
								sprintf(typeStr, "raw10_alignMode0");
								break;
							}
						case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
								format = (int)CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE1;
								sprintf(typeStr, "raw10_alignMode1");
								break;
							}
						default: {
								formatIsVaild = false;
								LOGE("%s: get invalid image format", __func__);
								break;
							}
					}
					type = 1;
					break;
				}
			case PIC_BUF_TYPE_RAW12:
				width = pPicBufMetaData->Data.raw.PicWidthPixel;
				height = pPicBufMetaData->Data.raw.PicHeightPixel;
				widthBytes = pPicBufMetaData->Data.raw.PicWidthBytes;
				bufferSize = widthBytes * height;
				switch (pPicBufMetaData->Data.raw.PixelDataAlignMode) {
					case PIC_BUF_DATA_UNALIGN_MODE: {
							format = (int)CAMDEV_PIX_FMT_RAW12;
							sprintf(typeStr, "raw12_unalign");
							break;
						}
					case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
							format = (int)CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE0;
							sprintf(typeStr, "raw12_alignMode0");
							break;
						}
					case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
							format = (int)CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1;
							sprintf(typeStr, "raw12_alignMode1");
							break;
						}
				}
				type = 1;
				break;
			case PIC_BUF_TYPE_RAW14:
				width = pPicBufMetaData->Data.raw.PicWidthPixel;
				height = pPicBufMetaData->Data.raw.PicHeightPixel;
				widthBytes = pPicBufMetaData->Data.raw.PicWidthBytes;
				bufferSize = widthBytes * height;
				switch (pPicBufMetaData->Data.raw.PixelDataAlignMode) {
					case PIC_BUF_DATA_UNALIGN_MODE: {
							format = (int)CAMDEV_PIX_FMT_RAW14;
							sprintf(typeStr, "raw14_unalign");
							break;
						}
					case PIC_BUF_DATA_ALIGN_DOUBLE_WORD: {
							format = (int)CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE0;
							sprintf(typeStr, "raw14_alignMode0");
							break;
						}
					case PIC_BUF_DATA_ALIGN_16BIT_MODE: {
							format = (int)CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1;
							sprintf(typeStr, "raw14_alignMode1");
							break;
						}
				}
				type = 1;
				break;
			case PIC_BUF_TYPE_RAW16:
				width = pPicBufMetaData->Data.raw.PicWidthPixel;
				height = pPicBufMetaData->Data.raw.PicHeightPixel;
				widthBytes = pPicBufMetaData->Data.raw.PicWidthBytes;
				format = (int)CAMDEV_PIX_FMT_RAW16;
				bufferSize = widthBytes * height;
				sprintf(typeStr, "raw16");
				type = 1;
				break;
			case PIC_BUF_TYPE_RAW24:
				width = pPicBufMetaData->Data.raw.PicWidthPixel;
				height = pPicBufMetaData->Data.raw.PicHeightPixel;
				widthBytes = pPicBufMetaData->Data.raw.PicWidthBytes;
				format = (int)CAMDEV_PIX_FMT_RAW24;
				bufferSize = widthBytes * height;
				sprintf(typeStr, "raw24");
				type = 1;
				break;
			default:
				LOGE("type error, unsupported type=%d!", pPicBufMetaData->Type);
				return;
		}
		if (!formatIsVaild) {
			LOGE("%s: get invalid image format", __func__);
			return;
		}
		// free by dom
		BufIdentity *pOutBuf = mm_malloc(sizeof(BufIdentity));
		MEMSET(pOutBuf, 0, sizeof(BufIdentity));
		pOutBuf->width = width;
		pOutBuf->height = height;
		pOutBuf->format = format;
		pOutBuf->buff_size = bufferSize;
		pOutBuf->widthBytes = widthBytes;
		pOutBuf->bitWidth = bitWidth;
		pOutBuf->yuvOrder = yuvOrder;
		memcpy(&pOutBuf->swap, &pPicBufMetaData->swap, sizeof(pPicBufMetaData->swap));
		pOutBuf->alignMode = alignMode;
		pOutBuf->cbCrWidthBytes = cbCrWidthBytes;
		pOutBuf->cbCrHeightPixel = cbCrHeightPixel;
		pOutBuf->cbWidthBytes = cbWidthBytes;
		pOutBuf->cbHeightPixel = cbHeightPixel;
		pOutBuf->crWidthBytes = crWidthBytes;
		pOutBuf->crHeightPixel = crHeightPixel;
		// LOGI("showbuffer: bufferSize:%d,", bufferSize);
		// LOGI("showbuffer: Phy_Addr:0x%x,", pNowBuffer->baseAddress);
		// LOGI("showbuffer: Ipl_Addr:%p", pNowBuffer->pIplAddress);
		if (pBuffer->pIplAddress == NULL && pBuffer->baseAddress == 0) {
			//LOGE("%s: show buffer pIplAddress error", __func__);
			return;
		}

		/* Reconstruct full 64-bit CPU address from 32-bit ATM address */
		uint64_t showFullAddr;
		if (ATM_ENABLE) {
			showFullAddr = (uint64_t)pNowBuffer->baseAddress | ((uint64_t)0x8 << 32);
		} else {
			showFullAddr = (uint64_t)pNowBuffer->pIplAddress;
		}
		uint8_t *pReadbuf = (uint8_t *)(uintptr_t)showFullAddr;
		static uint8_t dolShift = 0;
		if (bufferIO != CAMDEV_BUFCHAIN_HDR_RAW) {
		}
		else {
			uint32_t bufferSize = pOutBuf->widthBytes * pOutBuf->height;
			pOutBuf->buff_size = bufferSize;
			if (dolShift != 0) {
				if ((pNowBuffer->baseSize - bufferSize * dolShift) < bufferSize) {
					dolShift = 0;
				}
				else {
					pReadbuf += (bufferSize * dolShift);
				}
			}
			dolShift++;
		}

#if defined(HAL_CMODEL) || defined(DUMP_IMAGE)
		if ((pVvInstance->camDevInfo[showChannel].inputType == CAMDEV_INPUT_TYPE_IMAGE)
		    || (pVvInstance->camDevInfo[showChannel].inputType == CAMDEV_INPUT_TYPE_TPG)) {
#if 0
			char szFileName[1024];
			size_t nowFrameID = pVvInstance->frameIndex[showChannel][bufferIO]++;
			if (0 == type) {  // yuv
				snprintf(szFileName, sizeof(szFileName),
					 "vvbcfg/case/dump_image/%s_dump_%s_instance%d_path%d_frame%lu.yuv", pVvInstance->caseName, typeStr,
					 showChannel, bufferIO, nowFrameID);
			}
			else {    // raw
				if (bufferIO != CAMDEV_BUFCHAIN_HDR_RAW) {
					snprintf(szFileName, sizeof(szFileName),
						 "vvbcfg/case/dump_image/%s_dump_%s_instance%d_path%d_frame%lu.raw", pVvInstance->caseName, typeStr,
						 showChannel, bufferIO, nowFrameID);
				}
				else {
					char hdrStr[4][3] = {{"L"}, {"S"}, {"VS"}, {"XS"}};
					snprintf(szFileName, sizeof(szFileName),
						 "vvbcfg/case/dump_image/%s_dump_%s_%s_instance%d_path%d_frame%lu.raw",
						 pVvInstance->caseName, typeStr, hdrStr[dolShift - 1], showChannel, bufferIO, nowFrameID);
				}
			}

			osFile* pFile = NULL;
			pFile = osFopen(szFileName, "wb");
			if (NULL == pFile) {
				LOGE("%s osFopen %s fail", __func__, szFileName);
				return -1;
			}
			if (offset) {
				if (format == (int)CAMDEV_PIX_FMT_YUV444P) {
					int n = 0;
					do {
						osFwrite(pReadbuf, ysize, 1, pFile);
						++n;
						pReadbuf = pReadbuf + ysize + (ALIGN_16 - offset);
					} while (n < 3);
				}
				else {
					osFwrite(pReadbuf, ysize, 1, pFile);
					pReadbuf = pReadbuf + ysize + (ALIGN_16 - offset);
					osFwrite(pReadbuf, (bufferSize - ysize), 1, pFile);
				}
			}
			else {
				osFwrite(pReadbuf, bufferSize, 1, pFile);
			}
			Xil_DCacheInvalidateRange(pNowBuffer->baseAddress, bufferSize);
			osFclose(pFile);
			pFile = NULL;
#endif
			LOGI("------------------------------------------------------------\r\n");
			LOGI(" |            MIMO CASE RUN SUCCESSFULLY \r\n");
			LOGI(" |  📥 To retrieve the image dump, use the following command:\r\n");
			LOGI(" |  mrd -bin -file dump.rgb 0x%llx 0x%x\r\n\n",
			     (unsigned long long)showFullAddr,
			     pNowBuffer->baseSize / 4);
			LOGI("------------------------------------------------------------\r\n");
		}
#else
		if (0 == type) {  // yuv
			// LOGI("vvbench not open image dump format-%d-%s", type, typeStr);
			// LOGI("offset value %d", offset);
		}
		else {    // raw
			// LOGI("vvbench dump image format-%d-%s", type, typeStr);
		}
#endif


#ifdef APU_CORE
		VmixHdmiBridge_InputBuffer Vmix_buff;
		CamDevicePipeOutFmt_t Vmix_format;
		Vmix_format.outFormat = format;
		Vmix_format.outWidth = width;
		Vmix_format.outHeight = height;
		Vmix_buff.baseAddress = pBuffer->baseAddress;
#ifdef XPAR_XV_MIX_NUM_INSTANCES
		int ret_value = VmixHdmiBridge_UpdateBufferAddr(Vmix_buff.baseAddress, Vmix_format, showChannel, bufferIO);
		if (ret_value == 0) {
			LOGI("Unsupported Resolution to Display, Read it from SDcard !!\r\n");
		} else if (ret_value < 0) {
			LOGI("Failed to update the buffer address with layer!!");
		}
#endif
#endif

		mm_free(pOutBuf);
	}

}


int32_t VsiVvdeviceGetPGMHead(osFile *file)
{
	LOGI("%s enter \n", __func__);
	int fullHeadCount = 0;
	int32_t offset = 0;
	char line[1024];
	while (2 >= fullHeadCount) {
		if (NULL == osFgets(line, sizeof(line), file)) {
			LOGE("Error, Miss PGM Head.");
			return -1;
		}
		if (line[0] == '#') {
			offset += strlen(line);
			continue;
		}
		if (fullHeadCount == 0 && 0 != strncmp(line, "P5", 2)) {
			LOGE("Error, Only Support P5 Format PGM.");
			return -1;
		}
		offset += strlen(line);
		++fullHeadCount;
	}
	LOGI("%s exit \n", __func__);
	return offset;
}


int VsiVvdeviceGetInputPicture
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	int bufId,
	MediaBuffer_t *pBuff
)
{
	int result = RET_SUCCESS;
	LOGI("%s enter \n", __func__);

	uint32_t loadCount = pVvInstance->mPicCase[instanceId].readLine;
	uint32_t jumpLine = pVvInstance->mPicCase[instanceId].currLine;
	uint32_t bufferSize = pVvInstance->camDevInfo[instanceId].bufCfg[bufId].bufferSize;
	CamDeviceInputRawFmt_t rawFmt = pVvInstance->camDevInfo[instanceId].bufCfg[bufId].format;
	uint32_t index = 0;
	uint32_t coff = (rawFmt >= CAMDEV_INPUT_FMT_2DOL) ? (rawFmt - CAMDEV_INPUT_FMT_2DOL + 2) : 1;
	coff = bufferSize / coff;

	osFile *ifPicList = NULL;/*raw_picture_file name list info*/
	osFile *rawpicture = NULL;/*raw picture binary file*/
	char imageFile[FILE_LEN] = "";/*raw_picture_file name*/
	char *pRawbuff;/*raw picture data buffer*/

	if (pBuff == NULL) {
		LOGE("[vvbench]--%s: media buffer is null pointer!!", __func__);
		return RET_NULL_POINTER;
	}
	/*open picture list file*/
	LOGI("[vvbench]--%s:picture list file path:'%s' ", __func__,
	     pVvInstance->mPicCase[instanceId].pictureName);
#if 0
	ifPicList = osFopen(pVvInstance->mPicCase[instanceId].pictureName, "rb");

	if (!ifPicList) {
		LOGE("[vvbench]--%s:open picture file failed!", __func__);
		return RET_FAILURE;
	}

	/*jump to current read line*/
	osFseek(ifPicList, 0, SEEK_SET);
	for (index = 1; index < jumpLine; index++) {
		int fscanfLength = osFscanf(ifPicList, "%255s\r\n", imageFile); //TODO: length need to update
		if (EOF == fscanfLength) {
			LOGE("[vvbench]--%s: read input pic list fail ", __func__);
			osFclose(ifPicList);
			return RET_FAILURE;
		}
	}
#endif
	/*****************************json change ********************/
	const EmbeddedJson* json = find_image_data(pVvInstance->mPicCase[instanceId].pictureName);
	memcpy(imageFile, json->data, json->size);
	//memcpy(imageFile,&picture_raw_output_format_case_txt[0],picture_raw_output_format_case_txt_len);
	imageFile[json->size] = '\0';
	//imageFile[picture_raw_output_format_case_txt_len] = '\0';
	LOGI("[vvbench]--%s:picture list file path:'%s' ", __func__,
	     pVvInstance->mPicCase[instanceId].pictureName);
	/**********************************************************/

	/*get one new buffer for raw picture*/
	pRawbuff = (char *)mm_malloc(bufferSize);
	if (!pRawbuff) {
		//osFclose(ifPicList);
		mm_free(pRawbuff);
		return RET_OUTOFMEM;
	}

	for (index = 0; index < loadCount /*&& osFscanf(ifPicList,"%255s\r\n", imageFile)*/;
	     index++) {//TODO: length need to update
		if ('\0' == imageFile[0]) {
			LOGE("[vvbench]--%s: No picture file path in line[%d]!", __func__, jumpLine + index);
			result = RET_FAILURE;
			break;
		}

		LOGI("[vvbench]--%s:[%d],fileName:'%s'", __func__, index, imageFile);

		enum Type_RawFormat {
			RAW,
			PGM
		} rawFormat;
		if (3 >= strlen(imageFile) || 0 != strncmp(imageFile + strlen(imageFile) - 3, "pgm", 3)) {
			rawFormat = RAW;
		}
		else {
			rawFormat = PGM;
		}

		/*****************************json change ********************/

		const EmbeddedJson* json = find_image_data(imageFile);
		if (!json) {
			LOGE("[vvbench]--%s: find_image_data failed for '%s'", __func__, imageFile);
			result = RET_FAILURE;
			break;
		}
#if 0
		/*step1.open raw_picture_file*/
		rawpicture = osFopen(imageFile, "rb");
		if (!rawpicture) {
			LOGE("[vvbench]--%s:open raw picture failed!", __func__);
			result = RET_FAILURE;
			break;
		}
#endif
		uint32_t RawpicLen = 0;
		if (PGM == rawFormat) {

			LOGI("%s enter \n", __func__);
			int fullHeadCount = 0;
			int32_t offset = 0;
			char line[1024];
			while (2 >= fullHeadCount) {
				memcpy(line, json->data, sizeof(line));
				if (NULL == line) {
					LOGE("Error, Miss PGM Head.");
					return -1;
				}
				if (line[0] == '#') {
					offset += strlen(line);
					continue;
				}
				if (fullHeadCount == 0 && 0 != strncmp(line, "P5", 2)) {
					LOGE("Error, Only Support P5 Format PGM.");
					return -1;
				}
				offset += strlen(line);
				++fullHeadCount;
			}
			LOGI("%s exit \n", __func__);
			if(json->size == 4321)
			{
				RawpicLen = image_len - offset;
			}
			else {
				RawpicLen = json->size - offset;
			}
			memset(pRawbuff, 0, RawpicLen);
			memcpy(pRawbuff, &json->data[offset], RawpicLen);

#if 0

			int32_t offset = VsiVvdeviceGetPGMHead(rawpicture);
			if (0 >= offset) {
				LOGE("[vvbench]--%s:pgm picture file head is invalid", __func__);
			}
			osFseek(rawpicture, 0, SEEK_END);
			RawpicLen = osFtell(rawpicture) - offset;
			memset(pRawbuff, 0, RawpicLen);
			osFseek(rawpicture, offset, SEEK_SET);
#endif

		}
		else {
			/*step2.get and check raw picture file size*/
#if 0
			osFseek(rawpicture, 0, SEEK_END);
			RawpicLen = osFtell(rawpicture);
#endif
			if(json->size == 4321)
			{
				RawpicLen = image_len;
			}
			else {
				RawpicLen = json->size;
			}

			if (bufferSize < RawpicLen) {
				LOGE("[vvbench]--%s:raw picture file size(0x%x) is larger than buffersize(0x%x)", __func__,
				     RawpicLen, bufferSize);
				result = RET_OUTOFRANGE;
				break;
			}


			/*step3.read raw picture */
			memset(pRawbuff, 0, RawpicLen);
			memcpy(pRawbuff, json->data, RawpicLen);
			//pRawbuff[json->size] = '\0';
			//memcpy(pRawbuff, __2DNR_Case_0_raw, RawpicLen);
			//pRawbuff[RawpicLen] = '\0';
			sleep(1);

#if 0
			osFseek(rawpicture, 0, SEEK_SET);
#endif
		}

		if (pRawbuff != NULL) {
			//swap end for pgm
			if (PGM == rawFormat) {
				for (int i = 0; i + 1 < RawpicLen; i += 2) {
					char t = pRawbuff[i];
					pRawbuff[i] = pRawbuff[i + 1];
					pRawbuff[i + 1] = t;
				}
			}
			LOGD("[vvbench]--%s:media buffer: phybase:0x%x, virtual_ptr:0x%p", __func__, pBuff->baseAddress,
			     pBuff->pIplAddress);

			/* Reconstruct the full 64-bit CPU address from the 32-bit ATM address.
			 * The RPU returns 32-bit addresses; the actual DDR is at 0x800000000+offset.
			 * pIplAddress is uint32_t and cannot hold 64-bit addr, so reconstruct here. */
			uint64_t fullAddr;
			if (ATM_ENABLE) {
				fullAddr = (uint64_t)pBuff->baseAddress | ((uint64_t)0x8 << 32);
			} else {
				fullAddr = (uint64_t)pBuff->pIplAddress;
			}
			if (fullAddr != 0) {

				/*step4.sync picture to phybase mapped virtual address*/
				MEMCPY((uint8_t *)(uintptr_t)fullAddr + (index * coff), (uint8_t *)pRawbuff, RawpicLen);

				Xil_DCacheInvalidateRange((UINTPTR)fullAddr, RawpicLen);

			}
			else {
				result = RET_FAILURE;
				break;
			}
		}
		else {
			LOGE("[vvbench]--%s: read raw picture failed!", __func__);
			result = RET_FAILURE;
			break;
		}
		//osFclose(rawpicture);
		bufferSize -= coff;
	}

	//osFclose(ifPicList);
	mm_free(pRawbuff);
	if (result != RET_SUCCESS) {
		//osFclose(rawpicture);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceGetInputPictureForTool
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	int bufId,
	MediaBuffer_t *pBuff
)
{
	int result = RET_SUCCESS;

	LOGI("%s enter \n", __func__);

	uint32_t loadCount = pVvInstance->mPicCase[instanceId].readLine;
	uint32_t jumpLine = pVvInstance->mPicCase[instanceId].currLine;
	uint32_t bufferSize = pVvInstance->camDevInfo[instanceId].bufCfg[bufId].bufferSize;
	CamDeviceInputRawFmt_t rawFmt = pVvInstance->camDevInfo[instanceId].bufCfg[bufId].format;
	uint32_t index = 0;
	uint32_t coff = (rawFmt >= CAMDEV_INPUT_FMT_2DOL) ? (rawFmt - CAMDEV_INPUT_FMT_2DOL + 2) : 1;
	coff = bufferSize / coff;

	osFile *ifPicList;/*raw_picture_file name list info*/
	osFile *rawpicture;/*raw picture binary file*/
	char imageFile[FILE_LEN] = "";/*raw_picture_file name*/
	char *pRawbuff; /*raw picture data buffer*/

	if (pBuff == NULL) {
		LOGE("[vvbench]--%s: media buffer is null pointer!!", __func__);
		return RET_NULL_POINTER;
	}

	LOGI("[vvbench]--%s:picture list file path:'%s' ", __func__,
	     pVvInstance->mPicCase[instanceId].pictureName);
	ifPicList = osFopen(pVvInstance->mPicCase[instanceId].pictureName, "rb");

	if (!ifPicList) {
		LOGE("[vvbench]--%s:open picture file failed!", __func__);
		return RET_FAILURE;
	}

	osFseek(ifPicList, 0, SEEK_SET);
	if (!osFscanf(ifPicList, "%255s\r\n", imageFile)) {
		LOGE("[vvbench]--%s: read input file name failed!", __func__);
		osFclose(ifPicList);
		return RET_FAILURE;
	}
	/*get one new buffer for raw picture*/
	pRawbuff = (char *)mm_malloc(bufferSize);
	if (!pRawbuff) {
		osFclose(ifPicList);
		return RET_OUTOFMEM;
	}
	char imageFilePath[FILE_LEN] = "vvbcfg/load_image/Raw/ToolOutRaw/";
	strncat(imageFilePath, imageFile, FILE_LEN - strlen(imageFilePath));
	rawpicture = osFopen(imageFilePath, "rb");
	if (!rawpicture) {
		LOGE("[vvbench]--%s:open raw picture failed!", __func__);
		osFclose(ifPicList);
		mm_free(pRawbuff);
		return RET_FAILURE;
	}
	VvbenchRawMetaInfo_t *pRawMetaInfo = mm_malloc(sizeof(VvbenchRawMetaInfo_t));
	if (NULL == pRawMetaInfo) {
		osFclose(ifPicList);
		mm_free(pRawbuff);
		osFclose(rawpicture);
		LOGE("[vvbench]--%s:malloc VvbenchRawMetaInfo_t failed!", __func__);
		return RET_FAILURE;
	}
	MEMSET(pRawMetaInfo, 0, sizeof(VvbenchRawMetaInfo_t));
	if (0 != VsiVvbenchStrToRawMetaInfo(imageFile, pRawMetaInfo)) {
		LOGE("get meta info error, exit");
		osFclose(ifPicList);
		mm_free(pRawbuff);
		osFclose(rawpicture);
		mm_free(pRawMetaInfo);
		pRawMetaInfo = NULL;
		return RET_FAILURE;
	}

	uint32_t singleFrameLen = (pRawMetaInfo->width) * (pRawMetaInfo->height) * ((
					  pRawMetaInfo->bits + 8 - 1) / 8);
	if (rawFmt >= CAMDEV_INPUT_FMT_2DOL) {
		singleFrameLen = (((pRawMetaInfo->width) * (pRawMetaInfo->height) * 12) +8 - 1) / 8;
	}
	mm_free(pRawMetaInfo);
	pRawMetaInfo = NULL;

	osFseek(rawpicture, 0, SEEK_END);
	uint32_t RawpicLen = osFtell(rawpicture);

	if (bufferSize < singleFrameLen) {
		osFclose(ifPicList);
		mm_free(pRawbuff);
		osFclose(rawpicture);
		LOGE("[vvbench]--%s:raw picture file size(0x%x) is larger than buffersize(0x%x)", __func__,
		     singleFrameLen, bufferSize);
		return RET_OUTOFRANGE;
	}
	uint32_t loadStartLoc = (jumpLine - 1) * singleFrameLen;
	if (loadStartLoc > RawpicLen) {
		osFclose(ifPicList);
		mm_free(pRawbuff);
		osFclose(rawpicture);
		LOGE("[vvbench]--%s: video size error[%d]!", __func__, jumpLine + index);
		return RET_FAILURE;
	}
	LOGI("%s init picture file pointer offset [%ld]\n", __func__, osFtell(rawpicture));
	osFseek(rawpicture, loadStartLoc, SEEK_SET);
	LOGI("%s loadStartLoc picture file pointer offset [%ld]\n", __func__, osFtell(rawpicture));
	for (index = 0; index < loadCount; ++index) {
		memset(pRawbuff, 0, singleFrameLen);
		LOGI("%s load count %d picture file pointer offset [%ld] before read\n", __func__, index,
		     osFtell(rawpicture));
		if (singleFrameLen == osFread(pRawbuff, 1, singleFrameLen, rawpicture)) {
			LOGI("%s load count %d picture file pointer offset [%ld] after read\n", __func__, index,
			     osFtell(rawpicture));
			LOGD("[vvbench]--%s:media buffer: phybase:0x%x, virtual_ptr:0x%p", __func__, pBuff->baseAddress,
			     pBuff->pIplAddress);
			if (pBuff->pIplAddress != NULL) {
				MEMCPY((uint8_t *)(pBuff->pIplAddress) + (index * coff), (uint8_t *)pRawbuff, singleFrameLen);
			}
			else {
				result = RET_FAILURE;
				break;
			}
		}
		else {
			LOGE("[vvbench]--%s: read raw picture failed!", __func__);
			result = RET_FAILURE;
			break;
		}
	}
	osFclose(rawpicture);
	osFclose(ifPicList);
	mm_free(pRawbuff);
	LOGI("%s exit \n", __func__);
	return result;
}
void VsiVvdeviceBufferHandleStop
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	CamDeviceBufChainId_t bufferIO
)
{
	return;
}
void VsiVvdeviceBufferDwHandleStop
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	CamDeviceBufChainId_t bufferIO
)
{
	LOGI("%s enter \n", __func__);

	LOGI("%s exit \n", __func__);
}

int send_fmc_id_select(int fmc_id)
{
	LOGI(" %s-ENTER fmc_id=%d\n", __func__, fmc_id);
	RESULT result = RET_SUCCESS;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));
	packet.cookie = 0;
	packet.type = CMD;
	packet.payload_size = sizeof(uint32_t) + sizeof(uint32_t);

	uint8_t *p_data = packet.payload_data;
	uint32_t instance_id = 0;
	memcpy(p_data, &instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	uint32_t fmc_id_val = (uint32_t)fmc_id;
	memcpy(p_data, &fmc_id_val, sizeof(uint32_t));

	result = Send_Command(APU_2_RPU_MB_CMD_FMC_ID_SELECT, &packet,
			      packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result) {
		LOGE("Failed to send FMC_ID_SELECT, ret=%d", result);
		return RET_FAILURE;
	}

	uint8_t ack_ret = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	if (ack_ret != RET_SUCCESS) {
		LOGE("FMC_ID_SELECT ACK failed, ret=%u", ack_ret);
		return RET_FAILURE;
	}

	LOGI(" %s-EXIT fmc_id=%d confirmed\n", __func__, fmc_id);
	return RET_SUCCESS;
}

int configure_resizer(struct resizer resizer_t)
{

	{
		LOGI(" %s-ENTER %d\n", __func__, __LINE__);
		RESULT result = RET_SUCCESS;

		Payload_packet packet;
		memset(&packet, 0, sizeof(Payload_packet));
		packet.cookie = 0x99;
		packet.type = CMD;
		packet.payload_size = sizeof(resizer_t);


		memcpy(&packet.payload_data, &resizer_t, packet.payload_size);

		LOGI("Sizeof resizer structure %d \n", sizeof(resizer_t));
		result = Send_Command(APU_2_RPU_MB_CMD_CONFIG_RESIZER, &packet, sizeof(packet), dest_cpu_id,
				      src_cpu_id);
		if (RET_SUCCESS != result) {
			return RET_FAILURE;
		}

		apu_wait_for_ACK(packet.cookie, packet.payload_data);
		LOGI(" %s-EXIT %d\n", __func__, __LINE__);
		return result;
	}
}
