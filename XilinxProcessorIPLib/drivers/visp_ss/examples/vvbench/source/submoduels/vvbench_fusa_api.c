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


#define LOGTAG "FUSA"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_fusa_api.h"

static void FusaEventCbFunc
(
	const CamDeviceEventId_t evtId,
	CamDeviceFuSaMisVal_t *pParam,
	void *pUserContext
)
{
	RESULT result = RET_SUCCESS;

	LOGI("%s (enter evt=%04x)\n", __func__, (uint32_t)evtId);

	if ((pParam != NULL)) {
		// VvbenchFusaTestInfo_t *fusaTestInfo = (VvbenchFusaTestInfo_t *)pUserContext;
		switch (evtId) {
			case 0x1011: {
					CamDeviceFuSaMisVal_t *pFusaMiss = pParam;
					LOGI("fusa callback: fusaEcc1MisVal 0x%08x, fusaEcc2MisVal 0x%08x, fusaEcc3MisVal 0x%08x, "
					     "fusaEcc4MisVal 0x%08x, fusaEcc5MisVal 0x%08x, fusaEcc6MisVal 0x%08x, "
					     "fusaDupMisVal 0x%08x, fusaParityMisVal 0x%08x, fusaLv1MisVal 0x%08x \n",
					     pFusaMiss->fusaEcc1MisVal, pFusaMiss->fusaEcc2MisVal, pFusaMiss->fusaEcc3MisVal,
					     pFusaMiss->fusaEcc4MisVal, pFusaMiss->fusaEcc5MisVal, pFusaMiss->fusaEcc6MisVal,
					     pFusaMiss->fusaDupMisVal, pFusaMiss->fusaParityMisVal, pFusaMiss->fusaLv1MisVal);
					break;
				}
			default: {
					LOGE("unknown fusa event\n");
					break;
				}
		}
	}
	LOGI("%s (exit res=%d)\n", __func__, result);
}

#ifdef ISP_RDCD
static void FusaRdcdEventCbFunc
(
	const CamDeviceFusaRdcdEventId_t evtId,
	uint32_t *fusaRdcdLv1MisVal,
	void *pUserContext
)
{
	RESULT result = RET_SUCCESS;

	LOGI("%s (enter evt=%04x)\n", __func__, (uint32_t)evtId);

	if ((fusaRdcdLv1MisVal != NULL) && (evtId == CAMDEV_ISP_FUSA_RDCD_EVENT_FUSA)) {
		LOGI("fusa callback: fusaRdcdLv1MisVal 0x%08x \n", *fusaRdcdLv1MisVal);
	}
	else {
		LOGE("unknown fusa rdcd event\n");
	}
	LOGI("%s (exit res=%d)\n", __func__, result);
}
#endif

int VsiVvbenchFusaFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice || 0 == caseCtx->instanceCfgCtx[index].hCamCommon || NULL == caseCtx) {
		return -1;
	}
	result = VsiCamDeviceFusaReset(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA failed!\n");
		return -1;
	}
	result = VsiCamDeviceFusaEccDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA Ecc failed!\n");
		return -1;
	}
	result = VsiCamDeviceFusaPixelCountDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA PixelCount failed!\n");
		return -1;
	}
	result = VsiCamDeviceFusaTimeoutDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA Timeout failed!\n");
		return -1;
	}
	result = VsiCamDeviceFusaBistDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA Bist failed!\n");
		return -1;
	}
	result = VsiCamDeviceFuSaBistPowerUpDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA BistPowerUp failed!\n");
		return -1;
	}
	result = VsiCamDeviceFusaCrcDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable FUSA Crc failed!\n");
		return -1;
	}

	bool_t enable = caseCtx->instanceCfgCtx[index].fusaCfg.enable;
	if (!enable) {
		LOGI("Vvbench FUSA not enabled!\n");
		return 0;
	}
#ifdef ISP_RDCD
	CamDeviceRdcdFaultInjectConfig_t rdcFusaInjectCfg;
	MEMSET(&rdcFusaInjectCfg, 0, sizeof(CamDeviceRdcdFaultInjectConfig_t));
	CamDeviceRdcdFusaEnableConfig_t rdcFusaCfg;
	MEMSET(&rdcFusaCfg, 0, sizeof(CamDeviceRdcdFusaEnableConfig_t));

	if (caseCtx->instanceCfgCtx[index].fusaCfg.useCfg) {
		rdcFusaCfg.eccEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcEccEn;
		rdcFusaCfg.pixLossEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcPixLossEn;
		rdcFusaCfg.crcEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcCrcEn;
		rdcFusaCfg.crcInRevEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcCrcInRevEn;
		rdcFusaCfg.crcOutRevEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcCrcOutRevEn;
		rdcFusaCfg.crcXorEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcCrcXorEn;
		result = VsiCamDeviceFusaRdcdEnable(hCamDevice, &rdcFusaCfg);
		if (0 != result) {
			LOGE("Vvbench Enable Rdcd Fusa failed!\n");
			return -1;
		}

		rdcFusaInjectCfg.dupEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcFaultInjectionDupEn;
		rdcFusaInjectCfg.ecc2bitEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcFultInjectionEcc2bitEn;
		rdcFusaInjectCfg.ecc1bitEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcFaultInjectionEcc1bitEn;
		rdcFusaInjectCfg.fifoEn = caseCtx->instanceCfgCtx[index].fusaCfg.RdcFaultInjectionFifoEn;
		result = VsiCamDeviceFusaRdcdFaultInjectEnable(hCamDevice, &rdcFusaInjectCfg);
		if (0 != result) {
			LOGE("Vvbench Rdcd Fusa Fault Inject failed!\n");
			return -1;
		}

		result = VsiCamDeviceFusaRdcdRegisterEventCb(hCamDevice, FusaRdcdEventCbFunc, (void *)NULL);
		if (0 != result) {
			LOGE("Vvbench register FUSA cb func failed!\n");
			return -1;
		}
	}
#endif
	CamDeviceFaultInjectConfig_t fusaInjectCfg;
	CamDeviceFusaCrcConfig_t fusaCrcCfg;
	uint32_t version;
	MEMSET(&fusaInjectCfg, 0, sizeof(CamDeviceFaultInjectConfig_t));
	MEMSET(&fusaCrcCfg, 0, sizeof(CamDeviceFusaCrcConfig_t));

	if (caseCtx->instanceCfgCtx[index].fusaCfg.useCfg) {
		fusaInjectCfg.ahbTimeOutEn =
			caseCtx->instanceCfgCtx[index].fusaCfg.faultInjectionAhbTimeOutEn;
		fusaInjectCfg.dupEn =
			caseCtx->instanceCfgCtx[index].fusaCfg.faultInjectionDupEn;
		fusaInjectCfg.ecc2bitEn =
			caseCtx->instanceCfgCtx[index].fusaCfg.faultInjectionEcc2bitEn;
		fusaInjectCfg.ecc1bitEn =
			caseCtx->instanceCfgCtx[index].fusaCfg.faultInjectionEcc1bitEn;
		VsiCamDeviceFusaFaultInjectEnable(hCamDevice, &fusaInjectCfg);

		if (caseCtx->instanceCfgCtx[index].fusaCfg.eccEn) {
			VsiCamDeviceFusaEccEnable(hCamDevice);
		}

		if (caseCtx->instanceCfgCtx[index].fusaCfg.pixCountEn) {
			VsiCamDeviceFusaPixelCountEnable(hCamDevice);
			VsiCamDeviceFusaPixelCountSetConfig(hCamDevice);
		}

		if (caseCtx->instanceCfgCtx[index].fusaCfg.timeOutEn) {
            int pathEnable = 0;
            if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_PIPE_OUTPATH_MP].pathEnable) {
                pathEnable |= CAMDEV_OUT_MP_PATH_MASK;
            }
            if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_PIPE_OUTPATH_SP1].pathEnable) {
                pathEnable |= CAMDEV_OUT_SP1_PATH_MASK;
            }
            if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_PIPE_OUTPATH_SP2].pathEnable) {
                pathEnable |= CAMDEV_OUT_SP2_PATH_MASK;
            }
            if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_PIPE_OUTPATH_RAW].pathEnable) {
                pathEnable |= CAMDEV_OUT_RAW_PATH_MASK;
            }
            if (caseCtx->instanceCfgCtx[index].instancePath[CAMDEV_PIPE_OUTPATH_HDR_RAW].pathEnable) {
                pathEnable |= CAMDEV_OUT_HDR_RAW_PATH_MASK;
            }

            VsiCamDeviceFusaTimeoutEnable(hCamDevice, pathEnable);
        }

		if (caseCtx->instanceCfgCtx[index].fusaCfg.bistEn) {
			fusaCrcCfg.crcOutRevEn =
				caseCtx->instanceCfgCtx[index].fusaCfg.crcOutRevEn;
			fusaCrcCfg.crcXorEn = caseCtx->instanceCfgCtx[index].fusaCfg.crcXorEn;
			fusaCrcCfg.crcInRevEnMode =
				caseCtx->instanceCfgCtx[index].fusaCfg.crcInRevEn;
			result = VsiCamDeviceFusaBistEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable Fusa Bist failed!\n");
				return -1;
			}
			if (caseCtx->instanceCfgCtx[index].fusaCfg.bistPowerUpEn) {
				result = VsiCamDeviceFuSaBistPowerUpEnable(hCamDevice);
				if (0 != result) {
					LOGE("Vvbench Enable Fusa BistPowerUp failed!\n");
					return -1;
				}
			}
			result = VsiCamDeviceFusaCrcSetConfig(hCamDevice, &fusaCrcCfg);
			if (0 != result) {
				LOGE("Vvbench Set FUSA Crc Config failed!\n");
				return -1;
			}
		}

		if (caseCtx->instanceCfgCtx[index].fusaCfg.crcEn) {
            fusaCrcCfg.crcOutRevEn =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcOutRevEn;
            fusaCrcCfg.crcXorEn = caseCtx->instanceCfgCtx[index].fusaCfg.crcXorEn;
            fusaCrcCfg.crcInRevEnMode =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcInRevEn;
            fusaCrcCfg.crcLevel = caseCtx->instanceCfgCtx[index].fusaCfg.crcLevel;
            fusaCrcCfg.crcMpRoi.crcRoiH =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcMpRoiH;
            fusaCrcCfg.crcMpRoi.crcRoiV =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcMpRoiV;
            fusaCrcCfg.crcSp1Roi.crcRoiH =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp1RoiH;
            fusaCrcCfg.crcSp1Roi.crcRoiV =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp1RoiV;
            fusaCrcCfg.crcSp2Roi.crcRoiH =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp2RoiH;
            fusaCrcCfg.crcSp2Roi.crcRoiV =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp2RoiV;
            fusaCrcCfg.crcMpRoi.crcRoiOffH =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcMpRoiOffH;
            fusaCrcCfg.crcMpRoi.crcRoiOffV =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcMpRoiOffV;
            fusaCrcCfg.crcSp1Roi.crcRoiOffH =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp1RoiOffH;
            fusaCrcCfg.crcSp1Roi.crcRoiOffV =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp1RoiOffV;
            fusaCrcCfg.crcSp2Roi.crcRoiOffH =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp2RoiOffH;
            fusaCrcCfg.crcSp2Roi.crcRoiOffV =
                caseCtx->instanceCfgCtx[index].fusaCfg.crcSp2RoiOffV;
            VsiCamDeviceFusaCrcEnable(hCamDevice);
            VsiCamDeviceFusaCrcSetConfig(hCamDevice, &fusaCrcCfg);
        }
		result = VsiCamDeviceFusaRegisterEventCb(hCamDevice, FusaEventCbFunc, (void *)NULL);
		if (0 != result) {
			LOGE("Vvbench register FUSA cb func failed!\n");
			return -1;
		}
	}

	CamDeviceFusaCrcConfig_t crcGetCfg;
	MEMSET(&crcGetCfg, 0, sizeof(CamDeviceFusaCrcConfig_t));
	result = VsiCamDeviceFusaCrcGetConfig(hCamDevice, &crcGetCfg);
	if (0 != result) {
		LOGE("Vvbench Get Fusa Crc config failed!\n");
		return -1;
	}
	else {
		LOGI("crcOutRevEn: %d ", crcGetCfg.crcOutRevEn);
		LOGI("crcXorEn: %d ", crcGetCfg.crcXorEn);
		LOGI("crcInRevEnMode: %d ", crcGetCfg.crcInRevEnMode);
		LOGI("crcLevel: %d ", crcGetCfg.crcLevel);
		LOGI("crcMpRoi.crcRoiH: %d ", crcGetCfg.crcMpRoi.crcRoiH);
		LOGI("crcMpRoi.crcRoiV: %d ", crcGetCfg.crcMpRoi.crcRoiV);
		LOGI("crcSp1Roi.crcRoiH: %d ", crcGetCfg.crcSp1Roi.crcRoiH);
		LOGI("crcSp1Roi.crcRoiV: %d ", crcGetCfg.crcSp1Roi.crcRoiV);
		LOGI("crcSp2Roi.crcRoiH: %d ", crcGetCfg.crcSp2Roi.crcRoiH);
		LOGI("crcSp2Roi.crcRoiV: %d ", crcGetCfg.crcSp2Roi.crcRoiV);
	}

	CamDeviceFusaBistConfig_t bistGetCfg;
	MEMSET(&bistGetCfg, 0, sizeof(CamDeviceFusaBistConfig_t));
	result = VsiCamDeviceFusaBistGetConfig(hCamDevice, &bistGetCfg);
	if (0 != result) {
		LOGE("Vvbench Get Fusa Bist config failed!\n");
		return -1;
	}
	else {
		for (int i = 0; i < 3; ++i) {
			LOGI("tpgBistStage[%d] tpgBistImgHblank: %d ", i, bistGetCfg.tpgBistStage[i].tpgBistImgHblank);
			LOGI("tpgBistStage[%d] tpgBistImgVblank: %d ", i, bistGetCfg.tpgBistStage[i].tpgBistImgVblank);
			LOGI("tpgBistStage[%d] tpgBistImgHsize: %d ", i, bistGetCfg.tpgBistStage[i].tpgBistImgHsize);
			LOGI("tpgBistStage[%d] tpgBistImgVsize: %d ", i, bistGetCfg.tpgBistStage[i].tpgBistImgVsize);
			LOGI("tpgBistStage[%d] tpgBistSeed: %d ", i, bistGetCfg.tpgBistStage[i].tpgBistSeed);

			LOGI("bistCrcStage1Frm[%d]: %d ", i, bistGetCfg.bistCrcStage1Frm[i]);
			LOGI("bistCrcStage2Frm[%d]: %d ", i, bistGetCfg.bistCrcStage2Frm[i]);

			LOGI("bistMpCrcStage3Frm[%d] yCrc: %d ", i, bistGetCfg.bistMpCrcStage3Frm[i].yCrc);
			LOGI("bistMpCrcStage3Frm[%d] cbCrc: %d ", i, bistGetCfg.bistMpCrcStage3Frm[i].cbCrc);
			LOGI("bistMpCrcStage3Frm[%d] crCrc: %d ", i, bistGetCfg.bistMpCrcStage3Frm[i].crCrc);

			LOGI("bistSp1CrcStage3Frm[%d] yCrc: %d ", i, bistGetCfg.bistSp1CrcStage3Frm[i].yCrc);
			LOGI("bistSp1CrcStage3Frm[%d] cbCrc: %d ", i, bistGetCfg.bistSp1CrcStage3Frm[i].cbCrc);
			LOGI("bistSp1CrcStage3Frm[%d] crCrc: %d ", i, bistGetCfg.bistSp1CrcStage3Frm[i].crCrc);

			LOGI("bistSp2CrcStage3Frm[%d] yCrc: %d ", i, bistGetCfg.bistSp2CrcStage3Frm[i].yCrc);
			LOGI("bistSp2CrcStage3Frm[%d] cbCrc: %d ", i, bistGetCfg.bistSp2CrcStage3Frm[i].cbCrc);
			LOGI("bistSp2CrcStage3Frm[%d] crCrc: %d ", i, bistGetCfg.bistSp2CrcStage3Frm[i].crCrc);
		}
	}

	CamDeviceFusaPixelCountConfig_t pixelCountGetCfg;
	MEMSET(&pixelCountGetCfg, 0, sizeof(CamDeviceFusaPixelCountConfig_t));

	result = VsiCamDeviceFusaPixelCountGetConfig(hCamDevice, &pixelCountGetCfg);
	if (0 != result) {
		LOGE("Vvbench Get Fusa Pixel Count config failed!\n");
		return -1;
	}
	else {
		LOGI("mrszSize.outH: %d ", pixelCountGetCfg.mrszSize.outH);
		LOGI("mrszSize.outV: %d ", pixelCountGetCfg.mrszSize.outV);
		LOGI("srsz1Size.outH: %d ", pixelCountGetCfg.srsz1Size.outH);
		LOGI("srsz1Size.outV: %d ", pixelCountGetCfg.srsz1Size.outV);
		LOGI("srsz2Size.outH: %d ", pixelCountGetCfg.srsz2Size.outH);
		LOGI("srsz2Size.outV: %d ", pixelCountGetCfg.srsz2Size.outV);
		LOGI("filtSize.outH: %d ", pixelCountGetCfg.filtSize.outH);
		LOGI("filtSize.outV: %d ", pixelCountGetCfg.filtSize.outV);
	}


	result = VsiCamDeviceFusaGetVersion(hCamDevice, &version);
	if (0 != result) {
		LOGE("Vvbench Get FUSA version failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench FUSA version: %x", version);
	}
	LOGI("%s exit \n", __func__);
	return result;
}
