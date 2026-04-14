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


#define LOGTAG "DEV"
#include "vvpath.h"
#include "vlog.h"
#include "vvdevice.h"
#include "memory_manager.h"
#include "vmix_hdmi_bridge.h"
#define MAX_BUFFRS_PER_BUFIO (6)
extern int fbwr_flag;

struct aligned_buf_mal {
	void *malloc_ret_addr;
	uintptr_t aligned_addr;
};

#ifdef PORTING_25   // remove this code

// Remove conflicting struct definition - use the one from memory_manager.h

static void *vvpath_aligned_malloc(size_t size, size_t alignment, struct aligned_buf_mal *ab)
{
	struct aligned_buf temp_ab;
	void *ptr = mm_aligned_malloc(size, alignment, &temp_ab);
	if (ptr == NULL) {
		LOGE("no memory available\n");
		return NULL; // Allocation failed
	}
	ab->malloc_ret_addr = temp_ab.original_addr; // Store the original address for freeing
	// Adjust the memory address to meet the alignment requirement
	void *aligned_ptr = ptr; // Already aligned by mm_aligned_malloc

	ab->aligned_addr = (uintptr_t)aligned_ptr;
	return aligned_ptr;
}

#endif


#define MAX_ISP_NO 6
struct aligned_buf_mal tmp_mp_alloc_arry[CAMDEV_VIRTUAL_ID_MAX *
			MAX_ISP_NO][CAMDEV_BUFCHAIN_MAX][MAX_BUFFRS_PER_BUFIO];

int VsiVvdeviceOutPutPathCreate
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
)
{
	int result = 0;
	LOGI("%s enter:%d \n", __func__, bufIo);

	if (NULL == pVvInstance || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}
	if (UN_INIT != caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState) {
		LOGE("%s: Only uninited path can be init\n", __func__);
		return -1;
	}
	switch (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState) {
		case ENABLE:
			LOGE("This instance: %d, path: %d has been Enabled!", instanceId, bufIo);
			return -1;
		case DISABLE:
			LOGW("This instance: %d, path: %d is in disabled state!", instanceId, bufIo);
			return 0;
		case INIT:
			LOGW("This instance: %d, path: %d is already inited!", instanceId, bufIo);
			return 0;
		default:
			break;
	}
	CamDeviceBufMode_t buffMode = caseCtx->instanceCfgCtx[instanceId].buffMode;
	int bufferNumber = (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].bufferNumber >
			    MAX_BUFFER_QUEUES) ? MAX_BUFFER_QUEUES :
			   caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].bufferNumber;
	CamDeviceBufChainConfig_t bufferChain;
	MEMSET(&bufferChain, 0, sizeof(CamDeviceBufChainConfig_t));

	bufferChain.skipInterval = 0;
	bufferChain.bufQueLength = bufferNumber;
	bufferChain.emptyQueOp.blockType = CAMDEV_BUFQUE_TIMEOUT_TYPE;
	bufferChain.emptyQueOp.waitTime = 10;
	bufferChain.fullQueOp.blockType = CAMDEV_BUFQUE_TIMEOUT_TYPE;
#ifndef HAL_CMODEL
	bufferChain.fullQueOp.waitTime = 500;  // 2fps
#else
	bufferChain.fullQueOp.waitTime = 3000;
#endif
	result = VsiCamDeviceInitBufChain(pVvInstance->hCamDevice[instanceId], bufIo, &bufferChain);
	if (result != 0) {
		LOGE("VsiCamDeviceInitBufChain failed for bifio:%d", bufIo);
		return -1;
	}

	if (bufIo == CAMDEV_BUFCHAIN_METADATA) {
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = INIT;
		return 0;
	}


	LOGI("allocation for path %d", bufIo);

	//set json default buffer info
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferNumber = bufferNumber;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferSize =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].bufferSize;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].width =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].width;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].height =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].height;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].format =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].format;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].layout =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].layout;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].dataBits =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].dataBits;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].alpha =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].alpha;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].yuvOrder =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].yuvOrder;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].swap =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].swap;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].pathOutType =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathOutType;
	CamDevicePipeOutFmt_t outFormat;
	if (bufIo < CAMDEV_BUFCHAIN_RDMA) {
		CamDevicePipeOutPathType_t outPath;
		outPath = (CamDevicePipeOutPathType_t)(bufIo - CAMDEV_BUFCHAIN_MP + CAMDEV_PIPE_OUTPATH_MP);
		// Set Format
		// CamDevicePipeOutFmt_t outFormat;
		outFormat.outWidth = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].width;
		outFormat.outHeight = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].height;
		outFormat.outFormat = (CamDevicePipePixOutFmt_t)
				      pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].format;
		outFormat.dataBits = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].dataBits;
		outFormat.alpha = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].alpha;
		outFormat.yuvOrder = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].yuvOrder;
		outFormat.swap = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].swap;
		outFormat.pathOutType = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].pathOutType;
		result = VsiCamDeviceSetOutFormat(pVvInstance->hCamDevice[instanceId], outPath, &outFormat);
		if (result != 0) {
			LOGE("Vsi camdevice set format failed!");
			return -1;
		}

		//Get Format
		result = VsiCamDeviceGetOutFormat(pVvInstance->hCamDevice[instanceId], outPath, &outFormat);
		if (result != 0) {
			LOGE("Vsi camdevice get format failed!");
			return -1;
		}
#ifdef APU_CORE
#ifdef XPAR_XV_MIX_NUM_INSTANCES
		{
			int ret_value = VmixHdmiBridge_MapLayer(outFormat, instanceId,
					caseCtx->instanceCfgCtx[instanceId].hpId, bufIo,
					outFormat.dataBits, 0);
			if (ret_value < 0)
				LOGI("Failed to Enable VMix Layer for MO path!!\n");
		}
#endif
#endif

		LOGI("path %d: output width %d", bufIo, outFormat.outWidth);
		LOGI("path %d: output height %d", bufIo, outFormat.outHeight);
		LOGI("path %d: output format %d", bufIo, outFormat.outFormat);

		//Update buffer info
		pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].width = outFormat.outWidth;
		pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].height = outFormat.outHeight;
		pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].format = outFormat.outFormat;
		pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].alpha = outFormat.alpha;

	}

	//Get Buffer Size
	uint32_t unalignBuffSize = 0;
	result = VsiCamDeviceGetBufferSize(pVvInstance->hCamDevice[instanceId], bufIo, &unalignBuffSize);
	if (result != 0) {
		LOGE("Vsi camdevice get buffer size failed!");
		return -1;
	}
	LOGI("path %d:minimum total size of buffer 0x%08x Bytes", bufIo, unalignBuffSize);

	unsigned int allocateBufSize = ALIGN_UP(unalignBuffSize,
						pVvInstance->camDevInfo[instanceId].alignMask);
	//Update buffer size
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferSize = allocateBufSize;

	if (!caseCtx->useSubSystem) {

		//Initial buffer management for each path
		CamDeviceBufPoolConfig_t config;
		MEMSET(&config, 0, sizeof(CamDeviceBufPoolConfig_t));
		uint32_t phyAddr;
		uint32_t *pIplAddr = NULL;

		config.bufNum = bufferNumber;
		config.bufMode = buffMode;
		config.bufSize = allocateBufSize;
		config.pBaseAddrList = mm_malloc(config.bufNum * sizeof(uint32_t));
		if (config.pBaseAddrList == NULL) {
			LOGE("Malloc failed at %s-%d\n", __func__, __LINE__);
		}
		config.pIplAddrList = mm_malloc(config.bufNum * sizeof(void *));
		if (config.pIplAddrList == NULL) {
			LOGE("Malloc failed at %s-%d\n", __func__, __LINE__);

		}
		config.is_mapped = BOOL_TRUE;  // to support IMAGA_DUMP

		/** FBWR buffer allocation **/
		LOGI("Display Format width: %d height: %d\n", outFormat.outWidth, outFormat.outHeight);
		if ((caseCtx->instanceCfgCtx[instanceId].inputType == CAMDEV_INPUT_TYPE_SENSOR)
		    && ((caseCtx->instanceCfgCtx[instanceId].outPutType == CAMDEV_OUTPUT_TYPE_ONLINE)
			|| (caseCtx->instanceCfgCtx[instanceId].outPutType == CAMDEV_OUTPUT_TYPE_BOTH))) {

			LOGI(" Enabling FBWR for Live \n");
			//sensor_stream_flag=-1; uncomment if LO alone is working without MO
			fbwr_flag = 1;
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
			int result = init_fbwr(caseCtx->instanceCfgCtx[instanceId].hpId, bufIo, outFormat, config.bufSize);
			if (result < 0) {
				LOGE("FBWR FAILED \n\n\n\n");
			}
#endif
#ifdef XPAR_XV_MIX_NUM_INSTANCES
			{
				int ret_value = VmixHdmiBridge_MapLayer(outFormat, instanceId,
						caseCtx->instanceCfgCtx[instanceId].hpId, bufIo,
						outFormat.dataBits, 1);
				if (ret_value < 0)
					LOGI("Failed to Enable VMix Layer for LO path!!\n");
			}
#endif
		}
		if (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathOutType != 1) {
#ifdef XPAR_XV_MIX_NUM_INSTANCES
			{
				int ret_value = VmixHdmiBridge_MapLayer(outFormat, instanceId,
						caseCtx->instanceCfgCtx[instanceId].hpId, bufIo,
						outFormat.dataBits, 0);
				if (ret_value < 0)
					LOGI("Failed to Enable VMix Layer for MO path!!\n");
			}
#endif
		}
		if (CAMDEV_BUFMODE_USERPTR == buffMode) {
			for (int bufIdx = 0; bufIdx < bufferNumber; bufIdx++) {
				LOGI("NOTE: APU is allocating the buffer for output buffer chains!!\n"
				     "Check if Heap size can hold all buffers !!\n", allocateBufSize);

				vvpath_aligned_malloc(allocateBufSize, pVvInstance->camDevInfo[instanceId].alignMask,
						      &tmp_mp_alloc_arry[instanceId][bufIo][bufIdx]);

				if (tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr == NULL) {
					LOGE("%s-%d: APU Malloc failed with bufio %d !\n ", __func__, __LINE__, bufIo);
					mm_free(tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr);
				}

				phyAddr = (uint32_t)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].aligned_addr
					  ; //ALIGN_UP(tmpaddr,pVvInstance->camDevInfo[instanceId].alignMask);

					config.pBaseAddrList[bufIdx] = phyAddr;
					config.pIplAddrList[bufIdx] = (void *)(uintptr_t)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].aligned_addr;
				LOGI("buffer[%d]: Phy_Addr:0x%llx (ATM 32-bit:0x%x), size:0x%x", bufIdx,
				     (unsigned long long)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].aligned_addr,
				     config.pBaseAddrList[bufIdx], config.bufSize);
				LOGI("buffer[%d]: Ipl_Addr:%p", bufIdx, config.pIplAddrList[bufIdx]);
			}
		}
		pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].buffer.pBaseAddrList = config.pBaseAddrList;
		result = VsiCamDeviceCreateBufPool(pVvInstance->hCamDevice[instanceId], bufIo, &config);
		if (result != 0) {
			LOGE("VsiCamDeviceCreateBufPool for bufferIO:%d error", bufIo);
			mm_free(config.pBaseAddrList);
			mm_free(config.pIplAddrList);
			return -1;
		}

		result = VsiCamDeviceSetupBufMgmt(pVvInstance->hCamDevice[instanceId], bufIo);
		if (result != 0) {
			LOGE("VsiCamDeviceSetupBufMgmt for bufferIO:%d error", bufIo);
			mm_free(config.pBaseAddrList);
			mm_free(config.pIplAddrList);
			return -1;
		}
		mm_free(config.pIplAddrList);

		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = INIT;
	}

	caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = INIT;

	//Each path corp config
	if (bufIo <= CAMDEV_BUFCHAIN_SP2) {
		CamDevicePipeOutPathType_t path = (CamDevicePipeOutPathType_t)(bufIo);
		//crop window set
		CamDevicePipeIspWindow_t ispWindow;
		MEMSET(&ispWindow, 0, sizeof(ispWindow));
		ispWindow.cropWindow.hOffset =
			caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].cropCfg.horizontalOffset;
		ispWindow.cropWindow.vOffset =
			caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].cropCfg.verticalOffset;
		ispWindow.cropWindow.width =
			caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].cropCfg.cropWidth;
		ispWindow.cropWindow.height =
			caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].cropCfg.cropHeight;

		if ((ispWindow.cropWindow.width != 0) && (ispWindow.cropWindow.height != 0)) {
			result = VsiCamDeviceSetIspWindow(pVvInstance->hCamDevice[instanceId], path, &ispWindow);
			if (0 != result) {
				LOGE("Vsi camdevice set isp window failed!");
				return -1;
			}
			result = VsiCamDeviceGetIspWindow(pVvInstance->hCamDevice[instanceId], path, &ispWindow);
			if (0 != result) {
				LOGE("Vsi camdevice get ISP window failed!");
				return -1;
			}
			else {
				LOGI("ISP crop window hOffset: %d\n", ispWindow.cropWindow.hOffset);
				LOGI("ISP crop window vOffset: %d\n", ispWindow.cropWindow.vOffset);
				LOGI("ISP crop window width: %d\n", ispWindow.cropWindow.width);
				LOGI("ISP crop window height: %d\n", ispWindow.cropWindow.height);
			}
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}


int VsiVvdevicePathEnable
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *pBufIo,
	int bufCount

)
{
	int result = 0;
	LOGI("%s enter\n", __func__);

	if (NULL == pVvInstance || NULL == caseCtx || NULL == pBufIo) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}

	if ((instanceId >= MAX_CAM_NUM) || 0 >= bufCount) {
		LOGE("Wrong parameter !");
		return -1;
	}

	CamDevicePathStreamingCfg_t config;
	MEMSET(&config, 0, sizeof(CamDevicePathStreamingCfg_t));

	// Get Path streaming
	result = VsiCamDeviceGetPathStreaming(pVvInstance->hCamDevice[instanceId], &config);
	if (0 != result) {
		LOGE("%s: VsiCamDeviceGetPathStreaming Fails %d, return", __func__, result);
		return -1;
	}
	for (int i = 0; i < bufCount; ++i) {
		CamDeviceBufChainId_t bufIo = *(pBufIo + i);
		if (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState == UN_INIT) {
			LOGE("This instance: %d, path: %d has not been inited !", instanceId, bufIo);
			return -1;
		}
		if (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState == ENABLE) {
			LOGW("This instance: %d, path: %d is already enabled !", instanceId, bufIo);
			return 0;
		}
		LOGD("Start prepare to enable path: %d for instance: %d!", bufIo, instanceId);

		if (!caseCtx->instanceCfgCtx[instanceId].dewarpCfg.enable) {
			// register callBack
			switch (bufIo) {
				case CAMDEV_BUFCHAIN_MP:
					// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceMpCallBack;
					break;

				case CAMDEV_BUFCHAIN_SP1:
					// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceSp1CallBack;
					break;

				case CAMDEV_BUFCHAIN_SP2:
					// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceSp2CallBack;
					break;

				case CAMDEV_BUFCHAIN_RAW:
					// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceMpRawCallBack;
					break;

				case CAMDEV_BUFCHAIN_HDR_RAW:
					// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceHdrRawCallBack;
					break;

				case CAMDEV_BUFCHAIN_METADATA:
					if (CAMDEV_INPUT_TYPE_TPG == caseCtx->instanceCfgCtx[instanceId].inputType) {
						// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceTpgMetaDataCallBack;
					}
					else {
						// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceMetaDataCallBack;
					}
					break;
				default:
					// pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].callBack = VsiVvdeviceDummyCallBack;
					break;
			}
		}
		else {
#ifdef DWE_VERSION
			pVvInstance->runningFlagDw = 1;
			pVvInstance->dwCall = VsiVvdeviceDwCallBack;
#endif
		}

		if (!caseCtx->useSubSystem) {
			if (instanceId < caseCtx->totalInstance) {
				if ((bufIo >= CAMDEV_BUFCHAIN_MP && bufIo <= CAMDEV_BUFCHAIN_HDR_RAW)
				    || (bufIo == CAMDEV_BUFCHAIN_METADATA)) {
					// start the callBack
					// result = VsiVvdeviceBufferHandleStart(pVvInstance, caseCtx, instanceId, bufIo);
					if (0 != result) {
						LOGE("%s: BufferHandleStart Fails %d, return", __func__, result);
						return -1;
					}
				}
			}
			else {
				LOGE("##incorrect instance id %d, register failed", instanceId);
			}
			caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = ENABLE;
		}
		if ((bufIo >= CAMDEV_BUFCHAIN_MP && bufIo <= CAMDEV_BUFCHAIN_HDR_RAW)) {
			config.outPathEnable |= 1 << (bufIo - CAMDEV_BUFCHAIN_MP + CAMDEV_PIPE_OUTPATH_MP);
		}
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathEnable = true;
		//caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = ENABLE;
		LOGD("End prepare to enable path: %d for instance: %d!", bufIo, instanceId);
	}
	if (caseCtx->useSubSystem) {
#ifdef USE_SYSTEM
		for (int i = 0; i < bufCount; ++i) {
			CamDeviceBufChainId_t bufIo = *(pBufIo + i);
			if ((bufIo >= CAMDEV_BUFCHAIN_MP && bufIo <= CAMDEV_BUFCHAIN_HDR_RAW)) {
				config.outPathEnable |=
					pVvInstance->systemStreaming[instanceId].ispStream.startConfig.outPathEnable;
				config.outPathEnable |= 1 << (bufIo - CAMDEV_BUFCHAIN_MP + CAMDEV_PIPE_OUTPATH_MP);
				MEMCPY(&pVvInstance->systemStreaming[instanceId].ispStream.startConfig, &config,
				       sizeof(CamDevicePathStreamingCfg_t));
			}
		}
#endif
	}
	else {
		// Set Path streaming
		result = VsiCamDeviceSetPathStreaming(pVvInstance->hCamDevice[instanceId], &config);
		if (0 != result) {
			LOGE("%s: VsiCamDeviceSetPathStreaming Fails, return", __func__);
			return -1;
		}
	}
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdevicePathStart
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *pBufIo,
	int bufCount

)
{
	int result = 0;
	LOGI("%s enter\n", __func__);

	if (NULL == pVvInstance || NULL == caseCtx || NULL == pBufIo) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}

	if ((instanceId >= MAX_CAM_NUM) || 0 >= bufCount) {
		LOGE("Wrong parameter !");
		return -1;
	}

	CamDevicePathStreamingCfg_t config;
	MEMSET(&config, 0, sizeof(CamDevicePathStreamingCfg_t));

	for (int i = 0; i < bufCount; ++i) {
		CamDeviceBufChainId_t bufIo = *(pBufIo + i);
		if (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState == UN_INIT) {
			LOGE("This instance: %d, path: %d has not been inited !", instanceId, bufIo);
			return -1;
		}
		if (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState == ENABLE) {
			LOGW("This instance: %d, path: %d is already enabled !", instanceId, bufIo);
			return 0;
		}
		LOGD("Start prepare to enable path: %d for instance: %d!", bufIo, instanceId);
		// register callBack

		if (instanceId < caseCtx->totalInstance) {
			if (!caseCtx->instanceCfgCtx[instanceId].dewarpCfg.enable) {
				if ((bufIo >= CAMDEV_BUFCHAIN_MP && bufIo <= CAMDEV_BUFCHAIN_HDR_RAW)
				    || (bufIo == CAMDEV_BUFCHAIN_METADATA)) {
					// start the callBack
					// result = VsiVvdeviceBufferHandleStart(pVvInstance, caseCtx, instanceId, bufIo);
					if (0 != result) {
						LOGE("%s: BufferHandleStart Fails %d, return", __func__, result);
						return -1;
					}
				}
			}
			else {
				// start the callBack
				// result = VsiVvdeviceBufferDwHandleStart(pVvInstance, caseCtx, instanceId, bufIo);
				if (0 != result) {
					LOGE("%s: VsiVvdeviceBufferDwHandleStart Fails %d, return", __func__, result);
					return -1;
				}
			}
		}
		else {
			LOGE("##incorrect instance id %d, register failed", instanceId);
		}
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = ENABLE;
		LOGI("End prepare to enable path: %d for instance: %d!", bufIo, instanceId);
	}

	LOGI("%s exit \n", __func__);
	return result;
}


int VsiVvdevicePathDisable
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *pBufIo,
	int bufCount
)
{
	int result = 0;
	LOGI("%s enter\n", __func__);

	if (NULL == pVvInstance || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}
	CamDevicePathStreamingCfg_t config;
	MEMSET(&config, 0, sizeof(CamDevicePathStreamingCfg_t));

	// Get Path streaming
	result = VsiCamDeviceGetPathStreaming(pVvInstance->hCamDevice[instanceId], &config);
	if (0 != result) {
		LOGE("%s: VsiCamDeviceGetPathStreaming Fails %d, return", __func__, result);
		return -1;
	}
	for (int i = 0; i < bufCount; ++i) {
		CamDeviceBufChainId_t bufIo = *(pBufIo + i);

		switch (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState) {
			case UN_INIT:
				LOGE("This instance: %d, path: %d has not been inited !", instanceId, bufIo);
				return -1;
			case DISABLE:
				LOGW("This instance: %d, path: %d is already disabled !", instanceId, bufIo);
				return 0;
			case INIT:
				LOGE("This instance: %d, path: %d has not been enable !", instanceId, bufIo);
				return -1;
			default:
				break;
		}
		if ((bufIo >= CAMDEV_BUFCHAIN_MP && bufIo <= CAMDEV_BUFCHAIN_HDR_RAW)) {
			config.outPathEnable &= ~(0x1 << (bufIo - CAMDEV_BUFCHAIN_MP + CAMDEV_PIPE_OUTPATH_MP));
		}
	}
	// Set Path streaming
	if (!caseCtx->useSubSystem) {
		result = VsiCamDeviceSetPathStreaming(pVvInstance->hCamDevice[instanceId], &config);
		if (0 != result) {
			LOGE("%s: VsiCamDeviceSetPathStreaming Fails, return", __func__);
			return -1;
		}
	}
	for (int i = 0; i < bufCount; ++i) {
		CamDeviceBufChainId_t bufIo = *(pBufIo + i);
		// stop the callBack

		if (!caseCtx->instanceCfgCtx[instanceId].dewarpCfg.enable) {
			VsiVvdeviceBufferHandleStop(pVvInstance, instanceId, bufIo);
		}
		else {
			VsiVvdeviceBufferDwHandleStop(pVvInstance, instanceId, bufIo);
		}

		VsiVvdeviceDelay(1);

		// Close call backs.
		if (bufIo >= CAMDEV_BUFCHAIN_MP && bufIo <= CAMDEV_BUFCHAIN_METADATA) {

		}
		if ((bufIo >= CAMDEV_BUFCHAIN_RDMA && bufIo <= CAMDEV_BUFCHAIN_RETIMING) &&
		    CAMDEV_INPUT_TYPE_IMAGE == caseCtx->instanceCfgCtx[instanceId].inputType) {

		}

		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathEnable = false;
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = DISABLE;
	}
	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvdeviceInputPathCreate
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
)
{
	int result = 0;
	LOGI("%s enter:%d\n", __func__, bufIo);

	if (NULL == pVvInstance || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}

	CamDeviceBufMode_t buffMode = caseCtx->instanceCfgCtx[instanceId].buffMode;
	int bufferNumber = (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].bufferNumber >
			    MAX_BUFFER_QUEUES) ?
			   MAX_BUFFER_QUEUES : caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].bufferNumber;

	CamDeviceBufChainConfig_t bufferChain;
	MEMSET(&bufferChain, 0, sizeof(CamDeviceBufChainConfig_t));

	bufferChain.skipInterval = 0;
	bufferChain.bufQueLength = bufferNumber;
	bufferChain.emptyQueOp.blockType = CAMDEV_BUFQUE_TIMEOUT_TYPE;
	bufferChain.emptyQueOp.waitTime = 10;
	bufferChain.fullQueOp.blockType = CAMDEV_BUFQUE_TIMEOUT_TYPE;
#ifndef HAL_CMODEL
	bufferChain.fullQueOp.waitTime = 500;  // 2fps
#else
	bufferChain.fullQueOp.waitTime = 3000;
#endif
	result = VsiCamDeviceInitBufChain(pVvInstance->hCamDevice[instanceId], bufIo, &bufferChain);
	if (result != 0) {
		LOGE("VsiCamDeviceInitBufChain failed for bifio:%d", bufIo);
		return -1;
	}

	LOGI("allocation for path %d", bufIo);

	//set json default buffer info
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferNumber = bufferNumber;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferSize =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].bufferSize;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].width =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].width;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].height =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].height;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].format =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].format;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].layout =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].layout;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].dataBits =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].dataBits;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].alpha =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].alpha;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].yuvOrder =
		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].yuvOrder;

	CamDevicePipeInPathType_t inPath;
	inPath = (CamDevicePipeOutPathType_t)(bufIo - CAMDEV_BUFCHAIN_RDMA + CAMDEV_PIPE_INPATH_RDMA);
	// Set Format

	CamDevicePipeInFmt_t inFormat;
	MEMSET(&inFormat, 0, sizeof(CamDevicePipeInFmt_t));
	inFormat.inWidth = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].width;
	inFormat.inHeight = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].height;
	inFormat.inFormat = pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].format;
	inFormat.inPattern = (CamDeviceRawPattern_t)
			      pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].layout;
	inFormat.stitchMode = caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].stitchMode;
	result = VsiCamDeviceSetInFormat(pVvInstance->hCamDevice[instanceId], inPath, &inFormat);
	if (result != 0) {
		LOGE("Vsi camdevice set format failed!");
		return -1;
	}

	//Get Format
	result = VsiCamDeviceGetInFormat(pVvInstance->hCamDevice[instanceId], inPath, &inFormat);
	if (result != 0) {
		LOGE("Vsi camdevice get format failed!");
		return -1;
	}
	LOGI("path %d: input width %d", bufIo, inFormat.inWidth);
	LOGI("path %d: input height %d", bufIo, inFormat.inHeight);
	LOGI("path %d: input format %d", bufIo, inFormat.inFormat);

	//Update buffer info
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].width = inFormat.inWidth;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].height = inFormat.inHeight;
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].format = inFormat.inFormat;

	//Get Buffer Size
	uint32_t unalignBuffSize = 0;

	result = VsiCamDeviceGetBufferSize(pVvInstance->hCamDevice[instanceId], bufIo, &unalignBuffSize);
	if (result != 0) {
		LOGE("Vsi camdevice get buffer size failed!");
		return -1;
	}
	LOGI("path %d:minimum total size of buffer 0x%08x Bytes", bufIo, unalignBuffSize);

	unsigned int allocateBufSize = ALIGN_UP(unalignBuffSize,
						pVvInstance->camDevInfo[instanceId].alignMask);
	// Update buffer size
	pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferSize = allocateBufSize;

	if (!caseCtx->useSubSystem) {
		//Initial buffer management for each path
		CamDeviceBufPoolConfig_t config;
		MEMSET(&config, 0, sizeof(CamDeviceBufPoolConfig_t));
		uint32_t phyAddr;
		uint32_t *pIplAddr = NULL;


		config.bufNum = bufferNumber;
		config.bufMode = buffMode;
		config.bufSize = allocateBufSize;
		config.pBaseAddrList = mm_malloc(config.bufNum * sizeof(uint32_t));
		if (config.pBaseAddrList == NULL) {
			LOGE("Malloc failed at %s-%d\n", __func__, __LINE__);
		}
		config.pIplAddrList = mm_malloc(config.bufNum * sizeof(void *));
		config.is_mapped = BOOL_TRUE;  // to support IMAGA_DUMP
#if 0
		if (buffMode == CAMDEV_BUFMODE_USERPTR) {
			for (int bufIdx = 0; bufIdx < config.bufNum; bufIdx++) {
				result = VsiCamDeviceAllocResMemory(pVvInstance->hCamDevice[instanceId], allocateBufSize, &phyAddr,
								    (void **)&pIplAddr);
				if (result != 0) {
					LOGE("VsiCamDeviceAllocResMemory failed for bifio:%d", bufIo);
					mm_free(config.pBaseAddrList);
					mm_free(config.pIplAddrList);
					return -1;
				}
				config.pBaseAddrList[bufIdx] = phyAddr;
				config.pIplAddrList[bufIdx] = (void**)pIplAddr;
				LOGI("buffer[%d]: Phy_Addr:0x%x, size:0x%x", bufIdx, config.pBaseAddrList[bufIdx], config.bufSize);
				LOGI("buffer[%d]: Ipl_Addr:%p", bufIdx, config.pIplAddrList[bufIdx]);
			}
		}

#endif
		/*******************/
		if (caseCtx->instanceCfgCtx[instanceId].buffMode == CAMDEV_BUFMODE_USERPTR) {
			for (int bufIdx = 0; bufIdx < config.bufNum; bufIdx++) {


				LOGI("NOTE: APU is allocating the buffer for input buffer chains!!\n"
				     "Check if Heap size can hold all buffers !!\n", allocateBufSize);
				vvpath_aligned_malloc(allocateBufSize, pVvInstance->camDevInfo[instanceId].alignMask,
						      &tmp_mp_alloc_arry[instanceId][bufIo][bufIdx]);

				if (tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr == NULL) {
					LOGE("%s-%d: APU Malloc failed with bufio %d !\n ", __func__, __LINE__, bufIo);
					memset(tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr, 0, 5);
					mm_free(tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr);
				}

				phyAddr = (uint32_t)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].aligned_addr
					  ; //ALIGN_UP(tmpaddr,pVvInstance->camDevInfo[instanceId].alignMask);

				config.pBaseAddrList[bufIdx] = phyAddr;
				config.pIplAddrList[bufIdx] = (void *)(uintptr_t)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].aligned_addr;
				LOGI("buffer[%d]: Phy_Addr:0x%llx (ATM 32-bit:0x%x), size:0x%x", bufIdx,
				     (unsigned long long)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].aligned_addr,
				     config.pBaseAddrList[bufIdx], config.bufSize);
				LOGI("buffer[%d]: Ipl_Addr:%p", bufIdx, config.pIplAddrList[bufIdx]);


			}
		}
		else if (caseCtx->instanceCfgCtx[instanceId].buffMode == CAMDEV_BUFMODE_RESMEM) {
			for (int bufIdx = 0; bufIdx < config.bufNum; bufIdx++) {
				result = VsiCamDeviceAllocResMemory(pVvInstance->hCamDevice[instanceId], allocateBufSize, &phyAddr,
								    (void **)&pIplAddr);
				if (result != 0) {
					LOGE("VsiCamDeviceAllocResMemory failed for bifio:%d", bufIo);
					mm_free(config.pBaseAddrList);
					mm_free(config.pIplAddrList);
					return -1;
				}
				config.pBaseAddrList[bufIdx] = phyAddr;
				config.pIplAddrList[bufIdx] = (void**)pIplAddr;
				LOGI("buffer[%d]: Phy_Addr:0x%x, size:0x%x", bufIdx, config.pBaseAddrList[bufIdx], config.bufSize);
				LOGI("buffer[%d]: Ipl_Addr:%p", bufIdx, config.pIplAddrList[bufIdx]);
			}
		}
		/****/
		pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].buffer.pBaseAddrList = config.pBaseAddrList;
		result = VsiCamDeviceCreateBufPool(pVvInstance->hCamDevice[instanceId], bufIo, &config);
		if (result != 0) {
			LOGE("VsiCamDeviceCreateBufPool for bufferIO:%d error", bufIo);
			mm_free(config.pBaseAddrList);
			mm_free(config.pIplAddrList);
			return -1;
		}

		result = VsiCamDeviceSetupBufMgmt(pVvInstance->hCamDevice[instanceId], bufIo);
		if (result != 0) {
			LOGE("VsiCamDeviceSetupBufMgmt for bufferIO:%d error", bufIo);
			mm_free(config.pBaseAddrList);
			mm_free(config.pIplAddrList);
			return -1;
		}
		mm_free(config.pIplAddrList);

		caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = INIT;
	}

	caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = INIT;

	LOGI("%s exit \n", __func__);
	return result;
}


int VsiVvdevicePathRelease
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
)
{
	int result = 0;
	LOGI("%s enter:%d \n", __func__, bufIo);

	if (NULL == pVvInstance || NULL == caseCtx) {
		LOGE("%s: NUll pointer\n", __func__);
		return -1;
	}

	switch (caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState) {
		case UN_INIT:
			LOGW("This instance: %d, path: %d is already released!", instanceId, bufIo);
			return 0;
		case ENABLE:
			LOGE("This instance: %d, path: %d should be disable first!", instanceId, bufIo);
			return -1;
		default:
			break;
	}

	if (!caseCtx->useSubSystem) {
		result = VsiCamDeviceReleaseBufMgmt(pVvInstance->hCamDevice[instanceId], bufIo);
		if (result != 0) {
			LOGE("VsiCamDeviceReleaseBufMgmt Fails %d", bufIo);
			return -1;
		}

		result = VsiCamDeviceDestroyBufPool(pVvInstance->hCamDevice[instanceId], bufIo);
		if (result != 0) {
			LOGE("VsiCamDeviceDestoryBufPool failed for bifio:%d", bufIo);
			return -1;
		}

		for (int bufIdx = 0; bufIdx < pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].bufferNumber;
		     bufIdx++) {

			if (caseCtx->instanceCfgCtx[instanceId].buffMode == CAMDEV_BUFMODE_USERPTR) {
				if ((uint32_t *)tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr != NULL) {
					memset(tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr, 0, 5);
					mm_free(tmp_mp_alloc_arry[instanceId][bufIo][bufIdx].malloc_ret_addr);
				}

			}
			else if (caseCtx->instanceCfgCtx[instanceId].buffMode == CAMDEV_BUFMODE_RESMEM) {

				result = VsiCamDeviceFreeResMemory(pVvInstance->hCamDevice[instanceId],
								   pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].buffer.pBaseAddrList[bufIdx]);
				if (result != 0) {
					LOGE("VsiCamDeviceFreeResMemory failed for bifio:%d", bufIo);
					mm_free(pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].buffer.pBaseAddrList);
					return -1;
				}
			}


		}
		mm_free(pVvInstance->camDevInfo[instanceId].bufCfg[bufIo].buffer.pBaseAddrList);
	}


	result = VsiCamDeviceDeInitBufChain(pVvInstance->hCamDevice[instanceId], bufIo);
	if (result != 0) {
		LOGE("VsiCamDeviceDeInitBufChain failed for bifio:%d", bufIo);
		return -1;
	}

	caseCtx->instanceCfgCtx[instanceId].instancePath[bufIo].pathState = UN_INIT;
	//Close Path Dom
	// domCtrlStop(pVvInstance->dom[instanceId][(CamDeviceBufChainId_t)bufIo]);
	// domCtrlShutDown(pVvInstance->dom[instanceId][(CamDeviceBufChainId_t)bufIo]);

	LOGI("%s exit \n", __func__);
	return result;
}
