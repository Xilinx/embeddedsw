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


#ifndef __VVDEVICE_H__
#define __VVDEVICE_H__

#include "vvbase.h"
#ifndef NO_TERMINAL
	#include "vvterminal.h"
#endif
#include "vlog.h"

#include "cam_common_api.h"

#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include "cam_device_common.h"
#include "cam_device_sensor_api.h"
#include "cam_device_isp_system_api.h"

#ifdef DWE_VERSION
	#include "dewarp_system_api.h"
	#include "vvdewarp.h"
#endif

#if defined(ISP_VI200_V2_2) || defined(ISP_VI200_V2_1) || defined(ISP_VI200_V2) || defined(ISP_VI200)
	#include <video_in/video_in_api.h>
#endif

#ifdef USE_SYSTEM
	#include "isp_system_api.h"
#endif

//#include "dom_ctrl_api.h"
// #include <pthread.h>
#include <buf_defs.h>
#include <builtins.h>
#define MAX_CAM_NUM (CAMDEV_HARDWARE_ID_MAX*CAMDEV_VIRTUAL_ID_MAX)
#define MAX_BUFFER_QUEUES 96
#define SWITCH_TIMES 16

typedef struct VvbenchRawMetaInfo_s {
	uint32_t width;
	uint32_t height;
	int bits;
	int imgFormat;
	int layout;
	int frameNum;
} VvbenchRawMetaInfo_t;

typedef struct VvbenchPicCaseCfg_s {
	uint32_t loopCnt;
	uint32_t frameNum;
	uint32_t currLine;
	uint32_t readLine;
	char *pictureName;
} VvbenchPicCaseCfg_t;

typedef struct VvbenchCropWindow_s {
	uint16_t horizontalOffset;
	uint16_t verticalOffset;
	uint16_t cropWidth;
	uint16_t cropHeight;
} VvbenchCropWindow_t;
typedef enum VvbenchPathState_e {
	UN_INIT = 0,
	INIT,
	ENABLE,
	DISABLE
} VvbenchPathState_t;
typedef struct VvbenchInstancePath_s {
	CamDeviceBufChainId_t path;
	int pathEnable;
	VvbenchPathState_t pathState;
	int bufferNumber;
	uint32_t bufferSize;
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t layout;
	uint32_t dataBits;
	uint32_t alpha;
	uint32_t stitchMode;
	uint32_t yuvOrder;
	uint32_t pathOutType;
	CamDeviceMiSwap_u swap;
	VvbenchCropWindow_t cropCfg;
} VvbenchInstancePath_t;

typedef struct VvbenchSensorGain_s {
	float aGain;
	float dGain;
} VvbenchSensorGain_t;

typedef struct VvbenchInstancePathSwitchCfg_s {
	bool isBufferReCreate;
	uint32_t pathIdLength;
	uint32_t pathId[SWITCH_TIMES];
	uint32_t cropWindowLength;
	uint32_t cropPathId;
	uint32_t cropWindowHOffset[SWITCH_TIMES];
	uint32_t cropWindowVOffset[SWITCH_TIMES];
	uint32_t cropWindowWidth[SWITCH_TIMES];
	uint32_t cropWindowHeight[SWITCH_TIMES];
} VvbenchInstancePathSwitchCfg_t;

typedef struct VvbenchInstanceFastResetCfg_s {
	uint32_t fastResetLoop;
} VvbenchInstanceFastResetCfg_t;

typedef struct VvbenchInstanceSensorCfg_s {
	uint32_t sensorWidth;
	uint32_t sensorHeight;
	uint32_t useSensorCfg;
	uint32_t queryEnable;
	uint32_t capsEnable;
	uint32_t infoEnable;
	uint32_t testPatternEnable;
	uint32_t driverListEnable;
	uint32_t statusEnable;
	char sensorName[FILE_LEN];
	char calibrationName[FILE_LEN];
	char simulatorName[FILE_LEN];
	char autoSimulatorName[FILE_LEN];
	char illumType[FILE_LEN];
	uint32_t sensorDevId;
	uint32_t modeIndex;
	CamDeviceSensorType_t sensorType;
	CamDeviceStitchingMode_t stitchingMode;
	uint32_t bitWidth;
	float fps;
	uint32_t registerAddress;
	uint32_t registerValue;
	VvbenchSensorGain_t gain;
	float totalGain;
	float exposurtime;
	char dumpfile[FILE_LEN];
	uint32_t otpEnable;
	bool_t useExternalDriver;
	uint32_t virtualChannelId;
} VvbenchInstanceSensorCfg_t;

typedef struct VvbenchInstancePictureCfg_s {
	uint32_t loopCnt;
	uint32_t frameNum;
	uint32_t width;
	uint32_t height;
	CamDeviceRawPattern_t layout;
	CamDeviceInputRawFmt_t format;
	char pictureName[FILE_LEN];
	char calibXml[FILE_LEN];
	char fileName[FILE_LEN];
	uint32_t loadIndex;
} VvbenchInstancePictureCfg_t;

typedef struct VvbenchInstanceFusaCfg_s {
	bool_t useCfg;
	bool_t enable;
	bool_t eccEn;
	bool_t pixCountEn;
	bool_t timeOutEn;
	bool_t bistEn;
	bool_t bistPowerUpEn;
	uint32_t crcInRevEn;
	bool_t crcOutRevEn;
	bool_t crcXorEn;
	bool_t crcEn;
	bool_t faultInjectionAhbTimeOutEn;
	bool_t faultInjectionDupEn;
	bool_t faultInjectionEcc2bitEn;
	bool_t faultInjectionEcc1bitEn;
	uint32_t crcLevel;
	uint32_t crcMpRoiH;
	uint32_t crcMpRoiV;
	uint32_t crcSp1RoiH;
	uint32_t crcSp1RoiV;
	uint32_t crcSp2RoiH;
	uint32_t crcSp2RoiV;
	uint32_t crcMpRoiOffH;
    uint32_t crcMpRoiOffV;
    uint32_t crcSp1RoiOffH;
    uint32_t crcSp1RoiOffV;
    uint32_t crcSp2RoiOffH;
    uint32_t crcSp2RoiOffV;

#ifdef ISP_RDCD
	bool_t RdcPixLossEn;
	bool_t RdcEccEn;
	bool_t RdcFaultInjectionDupEn;
	bool_t RdcFultInjectionEcc2bitEn;
	bool_t RdcFaultInjectionEcc1bitEn;
	bool_t RdcFaultInjectionFifoEn;
	bool_t RdcCrcEn;
	bool_t RdcCrcInRevEn;
	bool_t RdcCrcOutRevEn;
	bool_t RdcCrcXorEn;
#endif
} VvbenchInstanceFusaCfg_t;

typedef struct VvbenchIspTpgUserDefineMode_s {
	struct {
		uint16_t total;
		uint16_t fp;
		uint16_t syncHeaderWidth;
		uint16_t bp;
		uint16_t act;
	} H, V;

} VvbenchIspTpgUserDefineMode_t;

typedef struct VvbenchInstanceTpgCfg_s {
	bool_t enable;
	bool_t useCfg;
	uint16_t imageType;
	uint16_t bayerPattern;
	uint16_t colorDepth;
	uint16_t resolution;
	uint16_t pixleGap;
	uint16_t lineGap;
	uint16_t gapStandard;
	uint32_t randomSeed;
	uint32_t frameNum;
	uint32_t width;
	uint32_t height;
	VvbenchIspTpgUserDefineMode_t userMode;
} VvbenchInstanceTpgCfg_t;

typedef struct VvbenchInstanceVi200Cfg_s {
	bool_t enable;
	bool_t aptHaltEn;
	uint16_t dvpId;
	uint8_t dvpWorkMode;            /**< 0: DVP normal mode; 1: DVP broadcast mode */
	uint8_t broadcastIpiId;       /**< broadcast IPI channel id */
	uint16_t bufSize;
	uint16_t parity;
	uint16_t ppc;
	uint16_t blockFrames;
	uint16_t irqMode;
	uint8_t ipiMode;
	uint32_t bwl;
	uint32_t ecc;
	uint16_t hblank;
	uint16_t vblank;
	uint16_t rtVbp;
	uint16_t rtVfp;
	uint16_t rtVsa;
	uint16_t rtHsa;
	uint16_t rtHfp;
	uint16_t rtHbp;
#if (defined ISP_VI200) || defined(ISP_VI200_V2) || (defined ISP_VI200_V2_1) || defined(ISP_VI200_V2_2)
	CamDeviceSensorMetadataWin_t metaWin;
	uint16_t dataType;
	uint16_t format;
#if defined(ISP_VI200_V2_1) || defined(ISP_VI200_V2_2)
	bool_t frameSkipEn;
	uint16_t m;
	uint16_t n;
	bool_t rdceEn;
	uint16_t mode;
	uint16_t thresh;
	float compressRatio;
	bool_t fusaEn;
	bool_t adapPixCntEn;
	bool_t rdcPixCntEn;
	bool_t eccEn;
	bool_t crcEn;
	bool_t faultInjectionDupEn;
	bool_t faultInjectionEcc2bitEn;
	bool_t faultInjectionEcc1bitEn;
	bool_t bistEn;
	bool_t bistPowerUpEn;
#endif
#endif
} VvbenchInstanceVi200Cfg_t;

typedef struct VvbenchInstanceDewarpCfg_s {
	bool enable;
	uint32_t dewarpMode;
	uint32_t mapSelect;
	uint32_t hflip;
	uint32_t vflip;
	uint32_t bypass;
	bool inputEnable;
	uint32_t inputPath;
	uint32_t inputHeight;
	uint32_t inputWidth;
	uint32_t inputFormat;
	uint32_t inputBit;
	bool outputEnable;
	uint32_t outputPath;
	uint32_t outputWidth;
	uint32_t outputHeight;
	uint32_t outputNumber;
	uint32_t outputFormat;
	uint32_t outputBit;
	uint32_t tempWidth;
	uint32_t tempHeight;
	uint32_t scaleFactor;
#ifdef DWE_VERSION
	double cameraMatrix[CAMERA_MATRIX_NUMBER];
	double distortionCoeff[DISTORTION_COEFF_NUMBER];
#endif
} VvbenchInstanceDewarpCfg_t;

typedef struct VvbenchFuncCfg_s {
	bool_t usePathSwitch;
	bool_t useFastResetFunction;
	bool_t useSensorFunction;
	bool_t useIspTpgFunction;
	bool_t useVi200Function;
	bool_t useVi200MetaWin;
	bool_t useDewarpFunction;
	bool_t irRawOutEnable;
	bool_t sp1IrSelect;
	bool_t useFusaFunction;
	bool_t useLowPowerMode;
} VvbenchFuncCfg_t;

typedef struct VvbenchModuleCfg_s {
	bool_t enable;
	bool_t useCfg;
	bool_t reset;
	bool_t isSupport;
	bool_t status;
	CamDeviceConfigMode_t mode;
} VvbenchModuleCfg_t;

typedef struct VvbenchSubModule_s {
	VvbenchModuleCfg_t awb;
	VvbenchModuleCfg_t ae;
	VvbenchModuleCfg_t af;
	VvbenchModuleCfg_t afm;
	VvbenchModuleCfg_t afmv3;
	VvbenchModuleCfg_t bls;
	VvbenchModuleCfg_t ca;
	VvbenchModuleCfg_t cac;
	VvbenchModuleCfg_t ccm;
	VvbenchModuleCfg_t cnr;
	VvbenchModuleCfg_t cpd;
	VvbenchModuleCfg_t cproc;
	VvbenchModuleCfg_t dci;
	VvbenchModuleCfg_t dg;
	VvbenchModuleCfg_t dmsc;
	VvbenchModuleCfg_t dnr2;
	VvbenchModuleCfg_t dnr3;
	VvbenchModuleCfg_t dpcc;
	VvbenchModuleCfg_t ee;
	VvbenchModuleCfg_t expv2;
	VvbenchModuleCfg_t expv3;
	VvbenchModuleCfg_t filter;
	VvbenchModuleCfg_t gc;
	VvbenchModuleCfg_t ge;
	VvbenchModuleCfg_t gtm;
	VvbenchModuleCfg_t gwdr;
	VvbenchModuleCfg_t hdr;
	VvbenchModuleCfg_t hist64;
	VvbenchModuleCfg_t hist256;
	VvbenchModuleCfg_t lsc;
	VvbenchModuleCfg_t lut3d;
	VvbenchModuleCfg_t pdaf;
	VvbenchModuleCfg_t rgbir;
	VvbenchModuleCfg_t wb;
	VvbenchModuleCfg_t wdr;
	VvbenchModuleCfg_t ynr;
} VvbenchSubModule_t;

typedef struct VvbenchInstanceCfg_s {
	uint32_t hpId;
	int instanceEnable;
	bool_t startPathSimultaneous;
	bool_t stopPathSimultaneous;
	char fineTuneJson[FILE_LEN];
	CamDeviceBufMode_t buffMode;

	VvbenchInstancePath_t instancePath[CAMDEV_BUFCHAIN_MAX];
	CamDeviceInputType_t inputType;
	CamCommonHandle_t hCamCommon;

	VvbenchInstanceSensorCfg_t sensorCfg;
	VvbenchInstancePictureCfg_t pictureCfg;
	VvbenchInstancePathSwitchCfg_t pathSwitchCfg;
	VvbenchInstanceFastResetCfg_t fastResetCfg;
	VvbenchInstanceTpgCfg_t tpgCfg;
	VvbenchInstanceVi200Cfg_t vi200Cfg;
	VvbenchInstanceDewarpCfg_t dewarpCfg;
	int streamDuration;
	CamDeviceWorkConfig_t workCfg;
	CamDeviceOutputType_t outPutType;
	CamDeviceSwitchSeqPriority_t priority;

	VvbenchFuncCfg_t funcCtrl;
	VvbenchSubModule_t moduleCfg;

	VvbenchInstanceFusaCfg_t fusaCfg;
} VvbenchInstanceCfg_t;

typedef struct VvbenchSwSimuAutoCfg_s {
	bool_t enable;
	bool_t awbEnable;
	bool_t cacEnable;
	bool_t ccEnable;
	bool_t lscEnable;
} VvbenchSwSimuAutoCfg_t;

typedef struct VvbenchSwSimuCfg_s {
	bool_t enable;
	char swSimuCaselist[FILE_LEN];
	char swSimuCfgFile[FILE_LEN];
	char fineTuneJson[FILE_LEN];
	char metaDataInfo[FILE_LEN];
	char defaultJson[FILE_LEN];
	VvbenchSwSimuAutoCfg_t autoCfg;
} VvbenchSwSimuCfg_t;

typedef struct VvbenchSystemId_s {
	uint8_t vi200Id;
	uint8_t ispId;
	uint8_t dewarpId;
} VvbenchSystemId_t;

typedef struct VvbenchVvdev_s {
	int totalInstance;
	bool_t useSubSystem;
	VvbenchSystemId_t systemId;
	bool_t fineTuneMode;
	VvbenchSwSimuCfg_t swSimuCfg;
	VvbenchInstanceCfg_t instanceCfgCtx[MAX_CAM_NUM];
#if defined(ISP_OFFLINE_TEST) || defined(HAL_CMODEL) || defined(DUMP_IMAGE)
	char caseName[FILE_LEN];
	size_t frameIndex[MAX_CAM_NUM];
#endif
} VvbenchVvdev_t;

typedef int (*CallBack)
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

typedef struct VvbenchBufferCfg_s {
	int bufferNumber;
	int bufferSize;
	int width;
	int height;
	int format;
	int layout;
	int dataBits;
	int alpha;
	int yuvOrder;
	int pathOutType;
	CamDeviceMiSwap_u swap;
	CamDeviceBufPoolConfig_t buffer;
	CallBack callBack;
} VvbenchBufferCfg_t;

typedef struct VvbenchCamDevCfg_s {
	uint32_t ispHwId;
	unsigned int alignMask;
	VvbenchBufferCfg_t bufCfg[CAMDEV_BUFCHAIN_MAX];
	CamDeviceWorkMode_t workMode;
	CamDeviceInputType_t inputType;
} VvbenchCamDevCfg_t;

#if defined(ISP_VI200_V2_1) || defined(ISP_VI200_V2) || defined(ISP_VI200) || defined(ISP_VI200_V2_2)
typedef struct VvbenchVi200StreamingCfg_s {
	bool_t startConfig;
	bool_t stopConfig;
} VvbenchVi200StreamingCfg_t;
#endif

#ifdef USE_SYSTEM
typedef struct VvbenchIspStreamingCfg_s {
	CamDevicePathStreamingCfg_t startConfig;
	CamDevicePathStreamingCfg_t stopConfig;
} VvbenchIspStreamingCfg_t;

typedef struct VvbenchSystemStreamingCfg_t {
	VvbenchIspStreamingCfg_t ispStream;
#if defined(ISP_VI200_V2_1) || defined(ISP_VI200_V2) || defined(ISP_VI200) || defined(ISP_VI200_V2_2)
	VvbenchVi200StreamingCfg_t vi200Stream;
#endif
} VvbenchSystemStreamingCfg_t;
#endif

typedef struct VvbenchInstance_s {

	int runningFlag[MAX_CAM_NUM][CAMDEV_BUFCHAIN_MAX];
	void *pBufferThreadCtx[MAX_CAM_NUM][CAMDEV_BUFCHAIN_MAX];

	CamDeviceHandle_t hCamDevice[MAX_CAM_NUM];
	VvbenchCamDevCfg_t camDevInfo[MAX_CAM_NUM];
	VvbenchPicCaseCfg_t mPicCase[MAX_CAM_NUM];
#if defined(ISP_VI200_V2_1) || defined(ISP_VI200_V2) || defined(ISP_VI200) || defined(ISP_VI200_V2_2)
	BufMgmtHandle_t bufMgmentHandle[MAX_CAM_NUM];
	CamDeviceVi200Handle_t hVi200[MAX_CAM_NUM];
#endif

#ifdef USE_SYSTEM
	IspSystemHandle_t ispSystemHanle[MAX_CAM_NUM];
	IspSystemCfg_t ispSystem[MAX_CAM_NUM];
	VvbenchSystemStreamingCfg_t systemStreaming[MAX_CAM_NUM];
#endif

#ifdef DWE_VERSION
	DewarpWrapperHandle hDewarpHandle[INSTANCE_NUM];
	DewarpContext_t dewarpCtx[INSTANCE_NUM];
	CallBack dwCall;
	int runningFlagDw;
#endif

	bool_t isSyncReleased[MAX_CAM_NUM];
	bool_t isHdrRetimingUse[MAX_CAM_NUM];
	bool_t loadPicFlag[MAX_CAM_NUM];
	bool isShowExpv2GetStatistic[MAX_CAM_NUM];
	bool isShowExpv3GetStatistic[MAX_CAM_NUM];
	bool isShowAfmGetStatistic[MAX_CAM_NUM];
	bool isShowAfmv3GetStatistic[MAX_CAM_NUM];
	bool isShowAwbPerFrameInfo[MAX_CAM_NUM];
	bool isShowAePerFrameInfo[MAX_CAM_NUM];
	bool isShowHist64GetStatistic[MAX_CAM_NUM];
	bool isShowHist256GetStatistic[MAX_CAM_NUM];

	// display
	// domCtrlHandle_t dom[MAX_CAM_NUM][CAMDEV_BUFCHAIN_RAW + 1];
	// VvbenchPostProcessInstance_t * pPostProcesser;
#ifndef NO_TERMINAL
	VvbenchTerminalThreadContext_t *pTerminalContext;
	size_t terminalFrameCount[MAX_CAM_NUM][CAMDEV_BUFCHAIN_MAX];
#endif
	int useTerminal;
	int useSubSystem;
	VvbenchSystemId_t systemId;
#if defined(ISP_OFFLINE_TEST) || defined(HAL_CMODEL) || defined(DUMP_IMAGE)
	bool_t enableDump;
	bool_t dumpByVideo;
	char caseName[FILE_LEN];
	size_t frameIndex[MAX_CAM_NUM][CAMDEV_BUFCHAIN_MAX];
#endif
} VvbenchInstance_t;

typedef struct VvbenchThreadContext_s {
	VvbenchInstance_t *pVvInstance;
	VvbenchVvdev_t *pCaseCtx;
	int instanceId;
	CamDeviceBufChainId_t bufferIO;
} VvbenchThreadContext_t;

void VsiVvdeviceDelay
(
	int delayInterval
);

int VsiVvdeviceInstanceInit
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseContext
);

void VsiVvdeviceInstanceDeinit
(
	VvbenchInstance_t *pVvInstance
);

int VsiVvdeviceStop
(
	const bool stopImmediately
);

int VsiVvdeviceExecuteCaseline
(
	VvbenchVvdev_t *caseContext,
	VvbenchCfg_t *cfgCtx
);

#if defined(ISP_VI200_V2_1) || defined(ISP_VI200_V2) || defined(ISP_VI200) || defined(ISP_VI200_V2_2)
int VsiVvdeviceMetaDataBufferManagmentSetup
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int index
);
#endif

int VsiVvdeviceCalibControl
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvbenchRegister3ALib
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvbenchUnRegister3ALib
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvdeviceLoadSimulatorToDatabase
(
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvdeviceLoadImageJsonToDatabase
(
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvdeviceNrRelocControl
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseContext
);

int VsiVvdeviceSubModuleControl
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseContext,
	int index
);

int VsiVvdeviceGetStatisticControl
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvdeviceGetStatistic
(
	VvbenchInstance_t *pVvInstance,
	CamDeviceHandle_t hCamDevice,
	int instanceId
);

//Todo: extension to Hardware Pipeline ID + Virtual Device ID
int VsiVvdeviceInitDom
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	CamDeviceBufChainId_t chain,
	const bool mcmRdma
);

int VsiVvdevicePathModify
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

void VsiVvdeviceDwShowBuffer
(
	VvbenchInstance_t *pVvInstance,
	MediaBuffer_t *pBuffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

void VsiVvdeviceShowBuffer
(
	VvbenchInstance_t *pVvInstance,
	MediaBuffer_t *pBuffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

RESULT VsiVvdeviceGetIllumType
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index,
	CamDeviceCalibIllumType_t *pllumType
);

int VsiVvdeviceSensorFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseContext,
	int index
);

int VsiVvdeviceBufferHandleStart
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufferIO
);

void VsiVvdeviceBufferHandleStop
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceBufferDwHandleStart
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufferIO
);

void VsiVvdeviceBufferDwHandleStop
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceGetInputMetadata
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	uint32_t lineNumber,
	char *pMetaList,
	char *pMetaFileName
);

int VsiVvdeviceGetInputPicture
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	int bufId,
	MediaBuffer_t *pBuff
);

int VsiVvdeviceGetInputPictureForTool
(
	VvbenchInstance_t *pVvInstance,
	int instanceId,
	int bufId,
	MediaBuffer_t *pBuff
);

int VsiVvdeviceMpCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceSp1CallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceSp2CallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceMpRawCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceHdrRawCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceMetaDataCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceTpgMetaDataCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceDwCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceDummyCallBack
(
	void *pInstance,
	MediaBuffer_t *buffer,
	int showChannel,
	CamDeviceBufChainId_t bufferIO
);

int VsiVvdeviceInitShm
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseContext,
	int index
);

void VsiVvdeviceDeInitShm
(
	VvbenchVvdev_t *caseContext,
	int index
);

RESULT VsiVvdeviceParserMetadataNew
(
	void *pInstance,
	int showChannel,
	MediaBuffer_t *buffer,
	CamDeviceBufChainId_t bufferIO
);

RESULT VsiVvdeviceParserMetadata
(
	MediaBuffer_t *buffer
);

RESULT VsiVvdeviceParserTpgMetadata
(
	MediaBuffer_t *buffer
);

#endif //__VVDEVICE_H__
