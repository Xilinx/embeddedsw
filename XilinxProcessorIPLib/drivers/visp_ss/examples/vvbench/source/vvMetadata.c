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



#include "vvdevice.h"
#include "cam_common_awb_api.h"
#include "cam_common_ae_api.h"

#define LOGTAG "METADATA"

#if 0
	#define MLOGD(...) (void)0
	#define MLOGI(...) (void)0
	#define MLOGW(...) (void)0
	#define MLOGE(...) (void)0
#else
	#define MLOGD(...) LOGD(__VA_ARGS__)
	#define MLOGI(...) LOGI(__VA_ARGS__)
	#define MLOGW(...) LOGW(__VA_ARGS__)
	#define MLOGE(...) LOGE(__VA_ARGS__)
#endif


#define LINE2_ROW0_PIXEL_NUM 1949
#define LINE2_ROW1_PIXEL_NUM 1789

#define LINE4_ROW0_PIXEL_NUM 1009
#define LINE4_ROW1_PIXEL_NUM 941
#define LINE4_ROW2_PIXEL_NUM 897
#define LINE4_ROW3_PIXEL_NUM 893

#define METADATA_PIXEL_TO_BYTE 3/2
#define PIXEL_CONVERT_TO_BYTE(x) ((x * METADATA_PIXEL_TO_BYTE) + (x % 2 == 0 ? 0 : 1))

#define PARSER_BYTES_OFFSET 3
#define START_TAG   0x0a
#define REG_MSB_TAG 0xaa
#define REG_LSB_TAG 0xa5
#define VALUE_TAG   0x5a
#define PENDING_TAG 0x55
#define LINE_EOF    0x07

typedef enum MetadataPaserIndex_e {

	CHIP_VERSION,
	FRAME_COUNT2_INDEX,
	FRAME_COUNT_INDEX,
	TEMPERATURE_INDEX,
	AGAIN_INDEX,
	AFINEGAIN_INDEX,
	DCGAIN_INDEX,
	WBGAIN_T1_GR_INDEX,
	WBGAIN_T1_BLUE_INDEX,
	WBGAIN_T1_RED_INDEX,
	WBGAIN_T1_GB_INDEX,
	WBGAIN_T2_GR_INDEX,
	WBGAIN_T2_BLUE_INDEX,
	WBGAIN_T2_RED_INDEX,
	WBGAIN_T2_GB_INDEX,
	WBGAIN_T3_GR_INDEX,
	WBGAIN_T3_BLUE_INDEX,
	WBGAIN_T3_RED_INDEX,
	WBGAIN_T3_GB_INDEX,
	WBGAIN_T4_GR_INDEX,
	WBGAIN_T4_BLUE_INDEX,
	WBGAIN_T4_RED_INDEX,
	WBGAIN_T4_GB_INDEX,
	EXP_RATIO_INEX,
	EXP_LINE_T1_INDEX,
	EXP_LINE_T2_INDEX,
	EXP_LINE_T3_INDEX,
	EXP_LINE_T4_INDEX,
	EXP_NUM_INDEX,
	MAX_INDEX
} MetadataPaserIndex_t;

typedef struct MetadataMatchList_s {
	MetadataPaserIndex_t index;
	uint16_t regAddr;
	uint16_t regValue;
	uint32_t bytesOffset;
} MetadataMatchList_t;

VvbenchInstance_t *pMetaInstance = NULL;

MetadataMatchList_t matchList[MAX_INDEX] = {
	{
		.index = CHIP_VERSION,
		.regAddr = 0x3000,
	},
	{
		.index = FRAME_COUNT2_INDEX,
		.regAddr = 0x2000,
	},
	{
		.index = FRAME_COUNT_INDEX,
		.regAddr = 0x2002,
	},
	{
		.index = TEMPERATURE_INDEX,
		.regAddr = 0x20fe,
	},
	{
		.index = EXP_RATIO_INEX,
		.regAddr = 0x3238,
	},
	{
		.index = EXP_LINE_T1_INDEX,
		.regAddr = 0x3012,
	},
	{
		.index = EXP_LINE_T2_INDEX,
		.regAddr = 0x3212,
	},
	{
		.index = EXP_LINE_T3_INDEX,
		.regAddr = 0x3216,
	},
	{
		.index = EXP_LINE_T4_INDEX,
		.regAddr = 0x321a,
	},
	{
		.index = WBGAIN_T1_GR_INDEX,
		.regAddr = 0x3056,
	},
	{
		.index = WBGAIN_T1_BLUE_INDEX,
		.regAddr = 0x3058,
	},
	{
		.index = WBGAIN_T1_RED_INDEX,
		.regAddr = 0x305a,
	},
	{
		.index = WBGAIN_T1_GB_INDEX,
		.regAddr = 0x305c,
	},
	{
		.index = WBGAIN_T2_GR_INDEX,
		.regAddr = 0x35a0,
	},
	{
		.index = WBGAIN_T2_BLUE_INDEX,
		.regAddr = 0x35a2,
	},
	{
		.index = WBGAIN_T2_RED_INDEX,
		.regAddr = 0x35a4,
	},
	{
		.index = WBGAIN_T2_GB_INDEX,
		.regAddr = 0x35a6,
	},
	{
		.index = WBGAIN_T3_GR_INDEX,
		.regAddr = 0x35a8,
	},
	{
		.index = WBGAIN_T3_BLUE_INDEX,
		.regAddr = 0x35aa,
	},
	{
		.index = WBGAIN_T3_RED_INDEX,
		.regAddr = 0x35ac,
	},
	{
		.index = WBGAIN_T3_GB_INDEX,
		.regAddr = 0x35ae,
	},
	{
		.index = WBGAIN_T4_GR_INDEX,
		.regAddr = 0x35b0,
	},
	{
		.index = WBGAIN_T4_BLUE_INDEX,
		.regAddr = 0x35b2,
	},
	{
		.index = WBGAIN_T4_RED_INDEX,
		.regAddr = 0x35b4,
	},
	{
		.index = WBGAIN_T4_GB_INDEX,
		.regAddr = 0x35b6,
	},
	{
		.index = EXP_NUM_INDEX,
		.regAddr = 0x3082,
	},
	{
		.index = DCGAIN_INDEX,
		.regAddr = 0x3362,
	},
	{
		.index = AGAIN_INDEX,
		.regAddr = 0x3366,
	},
	{
		.index = AFINEGAIN_INDEX,
		.regAddr = 0x336a,
	}
};


static inline uint16_t AR0820_GetDecode(uint8_t *pBuffer)
{

	uint16_t value;
	value = (*pBuffer & 0xff) << 8; //tag
	pBuffer += 2;//skip 0x55
	value |= (*pBuffer & 0xff); //value
	return value;//tag+value
}

static RESULT AR0820_FirstPaser
(
	MetadataBufInfo_t *pMetaBuffer,
	MetadataMatchList_t *pResultList,
	uint32_t *pRegNum
)
{

	if (pMetaBuffer == NULL || pResultList == NULL) {
		return RET_NULL_POINTER;
	}

	uint32_t nReg = 0, index = 0;
	uint8_t *pBuffer = pMetaBuffer->pBuffer[0];
	uint32_t bufSize = pMetaBuffer->bufferSize[0];
	uint32_t validBytes = PIXEL_CONVERT_TO_BYTE(LINE2_ROW0_PIXEL_NUM) + PIXEL_CONVERT_TO_BYTE(
				      LINE2_ROW1_PIXEL_NUM) + 1;// add start tag bytes

	uint16_t decode = 0;
	bool_t isMsb = BOOL_TRUE, isNewOffset = BOOL_TRUE;

	if (bufSize < validBytes) {
		MLOGD("%s: Metadata buffer size %d is smaller than validBytes %d \n", __func__, bufSize,
		      validBytes);
	}

	while (pBuffer[index] != START_TAG && index < bufSize) { // find the START TAG
		index ++;
	}

	index += 1; //Skip 'START_TAG'

	do {
		// MLOGI("%s: index %d pBuffer: 0x%x  \n", __func__,index, pBuffer[index]);

		while (pBuffer[index] == LINE_EOF && index < bufSize) { //skip 'LINE_EOF'
			index ++;
		}

		decode = AR0820_GetDecode(pBuffer + index);
		switch ((decode >> 8) & 0xff) {
			case REG_MSB_TAG:
				pResultList[nReg].regAddr = (decode & 0xff) << 8;
				isNewOffset = BOOL_TRUE;
				break;
			case REG_LSB_TAG:
				pResultList[nReg].regAddr |= (decode & 0xff);
				isNewOffset = BOOL_TRUE;
				break;
			case VALUE_TAG:
				if (isMsb) {
					pResultList[nReg].regValue = (decode & 0xff) << 8;
					pResultList[nReg].bytesOffset = index;
					isMsb = BOOL_FALSE;
					if (isNewOffset) {
						isNewOffset = BOOL_FALSE;
					}
					else {
						pResultList[nReg].regAddr = (pResultList[nReg - 1].regAddr + 2);
					}
				}
				else {
					pResultList[nReg++].regValue |= (decode & 0xff);
					isMsb = BOOL_TRUE;
				}
				break;
			default:
				break;
		}
		index += PARSER_BYTES_OFFSET;
	} while (index < bufSize);

	*pRegNum = nReg;
	return RET_SUCCESS;
}

static RESULT AR0820_RepeatPaser
(
	MetadataBufInfo_t *pMetaBuffer,
	MetadataMatchList_t *pResultList,
	uint32_t regNum
)
{
	if (pMetaBuffer == NULL || pResultList == NULL) {
		return RET_NULL_POINTER;
	}
	if (regNum == 0) {
		return RET_INVALID_PARM;
	}
	uint8_t *pBuffer = pMetaBuffer->pBuffer[0];
	uint32_t index = 0, nReg = 0;
	uint16_t decode;

	for (nReg = 0; nReg < regNum; nReg++) {

		index = pResultList[nReg].bytesOffset;
		decode = AR0820_GetDecode(pBuffer + index);
		pResultList[nReg].regValue = (decode & 0xff) << 8; // reg msb value

		index += PARSER_BYTES_OFFSET;
		decode = AR0820_GetDecode(pBuffer + index);
		pResultList[nReg].regValue |= (decode & 0xff);//reg lsb value

	}
	return RET_SUCCESS;
}

static RESULT AR0820_GetParserReg
(
	MetadataMatchList_t *pResultList,
	uint32_t resultNum,
	MetadataMatchList_t *pMatchList,
	uint32_t matchNum
)
{
	if (pResultList == NULL || pMatchList == NULL) {
		return RET_NULL_POINTER;
	}
	if (resultNum == 0 || matchNum == 0) {
		return RET_INVALID_PARM;
	}

	uint32_t matchId = 0;
	for (matchId = 0; matchId < matchNum; matchId ++) {

		uint32_t mid = resultNum / 2, max = resultNum, min = 0;
		uint16_t midReg = pResultList[mid].regAddr;
		uint16_t matchReg = pMatchList[matchId].regAddr;

		MLOGD("%s: find Start mid %d, max %d, midReg %x, matchReg %x  \n", __func__, mid, max, midReg,
		      matchReg);

		while (midReg != matchReg && (max - min) > 1) {
			if (midReg > matchReg) {
				max = mid;
			}
			else {
				min = mid;
			}
			mid = (max - min) / 2 + min;;
			midReg = pResultList[mid].regAddr;
		}

		MLOGD("%s: find End mid %d, max %d, midReg %x, matchReg %x  \n", __func__, mid, max, midReg,
		      matchReg);
		if ((max - min) > 1) { // find it
			pMatchList[matchId].regValue = pResultList[mid].regValue;
			pMatchList[matchId].bytesOffset = pResultList[mid].bytesOffset;
		}
		else {
			if (pResultList[min].regAddr == matchReg) {
				pMatchList[matchId].regValue = pResultList[min].regValue;
				pMatchList[matchId].bytesOffset = pResultList[min].bytesOffset;

			}
			else if (pResultList[max].regAddr == matchReg) {
				pMatchList[matchId].regValue = pResultList[max].regValue;
				pMatchList[matchId].bytesOffset = pResultList[max].bytesOffset;

			}
			else {
				//not find;
				MLOGI("%s: Not find the match register 0x%x \n", __func__, pMatchList[matchId].regAddr);
				return RET_NOTAVAILABLE;
			}
		}

	}
	return RET_SUCCESS;
}

static RESULT AR0820_FillParserData
(
	int showChannel,
	MetadataMatchList_t *pMatchList,
	uint32_t matchNum,
	CamDeviceMetadataInfo_t *pParserData
)
{
	if (pMatchList == NULL || matchNum == 0 || pParserData == NULL) {
		return RET_NULL_POINTER;
	}

	uint32_t matchId = 0;
	uint32_t ratio = 0;
	uint16_t expRatio = 0;
	uint8_t cgain_pow = 0, fgain_pow = 0;
	float again = 1.0, fgain = 0.0625;

	float onelineExpTime = 0.000075;//;(uints: S)
	CamDeviceSensorRegister_t reg;

	for (matchId = 0; matchId < matchNum; matchId ++) {

		uint16_t regValue = (pMatchList[matchId].regValue & 0xffff);

		switch (pMatchList[matchId].index) {
			case CHIP_VERSION:
				pParserData->chipId = regValue;
				MLOGI("%s: regValue: 0x%x, ChipVerison 0x%x \n", __func__, regValue, pParserData->chipId);
				break;
			case FRAME_COUNT2_INDEX:
				pParserData->frameCount = regValue;
				pParserData->frameCount = (pParserData->frameCount << 16) & 0xffff0000;
				MLOGI("%s: regValue: 0x%x, FrameCount2 %lu \n", __func__, regValue, pParserData->frameCount);
				break;
			case FRAME_COUNT_INDEX:
				pParserData->frameCount |= regValue & 0xffff;
				MLOGI("%s: regValue: 0x%x, FrameCount %lu \n", __func__, regValue, pParserData->frameCount);
				break;
			case TEMPERATURE_INDEX:
				pParserData->junctionTemperature = (float)regValue / 4 - 273.15; // 1/4 k
				MLOGI("%s: regValue: 0x%x, Temperature %f \n", __func__, regValue,
				      pParserData->junctionTemperature);
				break;
			case EXP_LINE_T1_INDEX:
				pParserData->integrationTime[CAMDEV_EXPOSURE_LONG_FRAME] = (uint32_t)(onelineExpTime * 1000000) *
					(uint32_t)regValue;
				MLOGI("%s: regValue: 0x%x, integrationTime T1 %d (uints: us)\n", __func__,
				      regValue, pParserData->integrationTime[CAMDEV_EXPOSURE_LONG_FRAME]);
				reg.addr = 0x3012;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: EXP_LINE_T1_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case EXP_RATIO_INEX:
				if ((regValue & 0x8000) == 0) {

					ratio = (uint32_t)(2 ^ (regValue & 0x7));
					pParserData->integrationTime[CAMDEV_EXPOSURE_SHORT_FRAME] =
						pParserData->integrationTime[CAMDEV_EXPOSURE_LONG_FRAME] / ratio;
					ratio = 2 ^ ((regValue >> 0x4) & 0x7);
					pParserData->integrationTime[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] =
						pParserData->integrationTime[CAMDEV_EXPOSURE_SHORT_FRAME] / ratio;
					ratio = 2 ^ ((regValue >> 0x8) & 0x7);
					pParserData->integrationTime[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] =
						pParserData->integrationTime[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] / ratio;

					MLOGI("%s: EXP_RATIO_INEX regValue: 0x%x, integration T2 %d, T3 %d, T4 %d\n", __func__, regValue,
					      pParserData->integrationTime[CAMDEV_EXPOSURE_SHORT_FRAME],
					      pParserData->integrationTime[CAMDEV_EXPOSURE_VERY_SHORT_FRAME],
					      pParserData->integrationTime[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME]);
				}
				expRatio = regValue;
				reg.addr = 0x3238;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: EXP_RATIO_INEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case EXP_LINE_T2_INDEX:
				if ((expRatio & 0x8000) != 0) {
					pParserData->integrationTime[CAMDEV_EXPOSURE_SHORT_FRAME] = (uint32_t)(onelineExpTime * 1000000) *
						(uint32_t)regValue;
					MLOGI("%s: regValue: 0x%x, integrationTime T2 %d (uints: us)\n", __func__,
					      regValue, pParserData->integrationTime[CAMDEV_EXPOSURE_SHORT_FRAME]);
					reg.addr = 0x3212;
					VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
					MLOGI("%s: EXP_LINE_T2_INDEX regValue: 0x%x\n", __func__, reg.value);
				}
				break;
			case EXP_LINE_T3_INDEX:
				if ((expRatio & 0x8000) != 0) {
					pParserData->integrationTime[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] = (uint32_t)(
							onelineExpTime * 1000000) * (uint32_t)regValue;
					MLOGI("%s: regValue: 0x%x, integrationTime T3 %d (uints: us)\n", __func__,
					      regValue, pParserData->integrationTime[CAMDEV_EXPOSURE_VERY_SHORT_FRAME]);
					reg.addr = 0x3216;
					VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
					MLOGI("%s: EXP_LINE_T3_INDEX regValue: 0x%x\n", __func__, reg.value);
				}
				break;
			case EXP_LINE_T4_INDEX:
				if ((expRatio & 0x8000) != 0) {
					pParserData->integrationTime[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] = (uint32_t)(
							onelineExpTime * 1000000) * (uint32_t)regValue;
					MLOGI("%s: regValue: 0x%x, integrationTime T4 %d (uints: us)\n", __func__,
					      regValue, pParserData->integrationTime[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME]);
					reg.addr = 0x321a;
					VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
					MLOGI("%s: EXP_LINE_T4_INDEX regValue: 0x%x\n", __func__, reg.value);
				}
				break;
			case WBGAIN_T1_GR_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].grChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gr T1 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].grChannel);
				reg.addr = 0x3056;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T1_GR_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T1_BLUE_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].blueChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain blue T1 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].blueChannel);
				reg.addr = 0x3058;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T1_BLUE_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T1_RED_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].redChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain red T1 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].redChannel);

				reg.addr = 0x305a;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T1_RED_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T1_GB_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].gbChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gb T1 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_LONG_FRAME].gbChannel);
				reg.addr = 0x305c;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T1_GB_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T2_GR_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].grChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gr T2 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].grChannel);
				reg.addr = 0x35a0;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T2_GR_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T2_BLUE_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].blueChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain blue T2 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].blueChannel);
				reg.addr = 0x35a2;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T2_BLUE_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T2_RED_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].redChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain red T2 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].redChannel);
				reg.addr = 0x35a4;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T2_RED_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T2_GB_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].gbChannel = (float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gb T2 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_SHORT_FRAME].gbChannel);
				reg.addr = 0x35a6;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T2_GB_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T3_GR_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].grChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gr T3 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].grChannel);
				reg.addr = 0x35a8;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T3_GR_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T3_BLUE_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].blueChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain blue T3 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].blueChannel);
				reg.addr = 0x35aa;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T3_BLUE_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T3_RED_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].redChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain red T3 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].redChannel);
				reg.addr = 0x35ac;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T3_RED_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T3_GB_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].gbChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gb T3 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME].gbChannel);
				reg.addr = 0x35ae;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T3_GB_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T4_GR_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].grChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gr T4 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].grChannel);
				reg.addr = 0x35b0;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T4_GR_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T4_BLUE_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].blueChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain blue T4 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].blueChannel);
				reg.addr = 0x35b2;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T4_BLUE_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T4_RED_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].redChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain red T4 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].redChannel);
				reg.addr = 0x35b4;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T4_RED_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case WBGAIN_T4_GB_INDEX:
				pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].gbChannel =
					(float)regValue / 128;
				MLOGI("%s: regValue: 0x%x, wbGain gb T4 %f\n", __func__,
				      regValue, pParserData->wbGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME].gbChannel);
				reg.addr = 0x35b6;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: WBGAIN_T4_GB_INDEX regValue: 0x%x\n", __func__, reg.value);
				break;
			case DCGAIN_INDEX:
				if ((regValue & 0x1) == 1) {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_LONG_FRAME] = 2.9;
				}
				else {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_LONG_FRAME] = 1.0;
				}

				if ((regValue & 0x2) == 0x2) {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_SHORT_FRAME] = 2.9;
				}
				else {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_SHORT_FRAME] = 1.0;
				}

				if ((regValue & 0x4) == 0x4) {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] = 2.9;
				}
				else {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] = 1.0;
				}

				if ((regValue & 0x8) == 0x8) {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] = 2.9;
				}
				else {
					pParserData->dualConvGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] = 1.0;
				}

				MLOGI("%s: regValue: 0x%x, Conversion gain L %f, S %f, VS %f, XS %f \n", __func__, regValue,
				      pParserData->dualConvGain[CAMDEV_EXPOSURE_LONG_FRAME],
				      pParserData->dualConvGain[CAMDEV_EXPOSURE_SHORT_FRAME],
				      pParserData->dualConvGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME],
				      pParserData->dualConvGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME]);

				reg.addr = 0x3362;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: DCGAIN_INDEX regValue: 0x%x\n", __func__, reg.value);

				break;
			case AGAIN_INDEX:
				cgain_pow = regValue & 0xf;
				again = (float)(2 ^ cgain_pow);
				pParserData->analogGain[CAMDEV_EXPOSURE_LONG_FRAME] = again;
				cgain_pow = (regValue >> 4) & 0xf;
				again = (float)(2 ^ cgain_pow);
				pParserData->analogGain[CAMDEV_EXPOSURE_SHORT_FRAME] = again;
				cgain_pow = (regValue >> 8) & 0xf;
				again = (float)(2 ^ cgain_pow);
				pParserData->analogGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] = again;
				cgain_pow = (regValue >> 12) & 0xf;
				again = (float)(2 ^ cgain_pow);
				pParserData->analogGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] = again;

				MLOGI("%s: regValue: %d, again L %f, S %f, VS %f, XS %f \n", __func__, regValue,
				      pParserData->analogGain[CAMDEV_EXPOSURE_LONG_FRAME],
				      pParserData->analogGain[CAMDEV_EXPOSURE_SHORT_FRAME],
				      pParserData->analogGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME],
				      pParserData->analogGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME]);
				reg.addr = 0x3366;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: AGAIN_INDEX regValue: %d,\n", __func__, reg.value);

				break;
			case AFINEGAIN_INDEX:
				fgain_pow = regValue & 0xf;
				fgain = (float)fgain_pow * pParserData->analogGain[CAMDEV_EXPOSURE_LONG_FRAME] * 0x0625;
				pParserData->analogGain[CAMDEV_EXPOSURE_LONG_FRAME] += fgain;
				pParserData->analogGain[CAMDEV_EXPOSURE_LONG_FRAME] *=
					pParserData->dualConvGain[CAMDEV_EXPOSURE_LONG_FRAME];
				fgain_pow = (regValue >> 4) & 0xf;
				fgain = (float)fgain_pow * pParserData->analogGain[CAMDEV_EXPOSURE_SHORT_FRAME] * 0x0625;
				pParserData->analogGain[CAMDEV_EXPOSURE_SHORT_FRAME] += fgain;
				pParserData->analogGain[CAMDEV_EXPOSURE_SHORT_FRAME] *=
					pParserData->dualConvGain[CAMDEV_EXPOSURE_SHORT_FRAME];
				fgain_pow = (regValue >> 8) & 0xf;
				fgain = (float)fgain_pow * pParserData->analogGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] * 0x0625;
				pParserData->analogGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] += fgain;
				pParserData->analogGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] *=
					pParserData->dualConvGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME];
				fgain_pow = (regValue >> 12) & 0xf;
				fgain = (float)fgain_pow * pParserData->analogGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] * 0x0625;
				pParserData->analogGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] += fgain;
				pParserData->analogGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] *=
					pParserData->dualConvGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME];
				MLOGI("%s: regValue: 0x%x, again L %f, S %f, VS %f, XS %f \n", __func__, regValue,
				      pParserData->analogGain[CAMDEV_EXPOSURE_LONG_FRAME],
				      pParserData->analogGain[CAMDEV_EXPOSURE_SHORT_FRAME],
				      pParserData->analogGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME],
				      pParserData->analogGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME]);
				reg.addr = 0x336a;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: AFINEGAIN_INDEX regValue: 0x%x,\n", __func__, reg.value);

				break;
			case EXP_NUM_INDEX:
				pParserData->exposureNum = ((regValue >> 2) & 0x3) +1;
				MLOGI("%s: regValue: 0x%x, expnum %d \n", __func__, regValue, pParserData->exposureNum);
				reg.addr = 0x3082;
				VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[showChannel], &reg);
				MLOGI("%s: EXP_NUM_INDEX regValue: 0x%x \n", __func__, reg.value);
				break;
			default:
				break;
		}
	}

	pParserData->digitalGain[CAMDEV_EXPOSURE_LONG_FRAME] = 1.0;
	pParserData->digitalGain[CAMDEV_EXPOSURE_SHORT_FRAME] = 1.0;
	pParserData->digitalGain[CAMDEV_EXPOSURE_VERY_SHORT_FRAME] = 1.0;
	pParserData->digitalGain[CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME] = 1.0;

	return RET_SUCCESS;
}

RESULT VsiVvdeviceParserMetadataNew
(
	void *pInstance,
	int showChannel,
	MediaBuffer_t *buffer,
	CamDeviceBufChainId_t bufferIO
)
{
	RESULT ret = RET_SUCCESS;
	uint32_t regNum = 0, matchNum = 0;
	static uint32_t frameNum = 0;
	MetadataBufInfo_t *pMetaBuf = &(((PicBufMetaData_t *)buffer ->pMetaData)->Data.meta.metaBufInfo);
	MetadataMatchList_t resultList[1000]; // [window num][reg num]
	CamDeviceMetadataInfo_t parserData;
	VvbenchInstance_t *pVvInstance = (VvbenchInstance_t *)pInstance;
	VvbenchThreadContext_t *pBufferThreadCtx = (VvbenchThreadContext_t *)
		pVvInstance->pBufferThreadCtx[showChannel][bufferIO];
	VvbenchVvdev_t *pCaseCtx = pBufferThreadCtx->pCaseCtx;
	CamDeviceSensorRegister_t reg;

	pMetaInstance = pVvInstance;

	if (pMetaBuf->pBuffer[0] == NULL) {
		return RET_NULL_POINTER;
	}

	// MLOGI("%s:instance %d, address: 0x%x, start tag: 0x%x, bufSize:0x%x \n", __func__,
	// showChannel, pMetaBuf->address[0],*(pMetaBuf->pBuffer[0]),pMetaBuf->bufferSize[0]);

	matchNum = MAX_INDEX;
	if (frameNum == 0) {
		ret = AR0820_FirstPaser(pMetaBuf, resultList, &regNum);
		ret |= AR0820_GetParserReg(resultList, regNum, matchList, matchNum);
	}
	else {
		MLOGI("%s: Repeat frame %d \n", __func__, frameNum);
		ret = AR0820_RepeatPaser(pMetaBuf, matchList, matchNum);
	}

	if (ret != RET_SUCCESS) {
		MLOGE("%s: AR0820_Paser data error ret %d \n", __func__, ret);
		return ret;
	}

	MLOGD("%s: Match Reg 0x%x, value 0x%x", __func__, matchList[2].regAddr, matchList[2].regValue);
	reg.addr = 0x2002;
	VsiCamDeviceSensorGetRegister(pVvInstance->hCamDevice[showChannel], &reg);
	MLOGD("%s: read Reg 0x%x,  value 0x%x\n", __func__, reg.addr, reg.value);

	ret = AR0820_FillParserData(showChannel, matchList, matchNum, &parserData);
	if (ret != RET_SUCCESS) {
		MLOGE("%s: AR0820_FillParserData error ret %d \n", __func__, ret);
		return ret;
	}

	CamDeviceMetadataConfig_t MatedataCfg;
	MEMSET(&MatedataCfg, 0, sizeof(CamDeviceMetadataConfig_t));
	MEMCPY(&MatedataCfg.metaInfo, &parserData, sizeof(CamDeviceMetadataInfo_t));

	ret = VsiCamCommonAeGetMode(pCaseCtx->instanceCfgCtx[showChannel].hCamCommon, &MatedataCfg.modeAe);
	if (ret != RET_SUCCESS) {
		MLOGE("%s: VsiCamCommonAeGetMode error ret %d \n", __func__, ret);
		return ret;
	}
	ret = VsiCamCommonAwbGetMode(pCaseCtx->instanceCfgCtx[showChannel].hCamCommon,
				     &MatedataCfg.modeAwb);
	if (ret != RET_SUCCESS) {
		MLOGE("%s: VsiCamCommonAwbGetMode error ret %d \n", __func__, ret);
		return ret;
	}

	ret = VsiCamDeviceConfigMetadata(pVvInstance->hCamDevice[showChannel], &MatedataCfg);
	if (ret != RET_SUCCESS) {
		MLOGE("%s: VsiCamDeviceConfigMetadata error ret %d \n", __func__, ret);
		return ret;
	}

	frameNum ++;
	return RET_SUCCESS;
}

RESULT VsiVvdeviceParserMetadata
(
	MediaBuffer_t *buffer
)
{
	MetadataBufInfo_t *pMetaBuf = &(((PicBufMetaData_t *)buffer ->pMetaData)->Data.meta.metaBufInfo);
	// CamDeviceSensorRegister_t reg;
	if (pMetaBuf->pBuffer[0] == NULL) {
		return RET_NULL_POINTER;
	}

	// MLOGI("%s: address: 0x%x, start tag: 0x%x, bufSize:0x%x \n", __func__,
	// pMetaBuf->address[0],*(pMetaBuf->pBuffer[0]),pMetaBuf->bufferSize[0]);


	/* hard code paser for 4k*/
#define VI200_ONE_PIEXL_BYTES  3/2
	/*frame count*/
	uint32_t frmCountOffset = 5;//(pixel)
	uint8_t *pfrmCount = pMetaBuf->pBuffer[0];
	uint32_t frmcount = 0;


	pfrmCount += frmCountOffset * VI200_ONE_PIEXL_BYTES;
	pfrmCount += 2; //pass 5a
	frmcount = (*pfrmCount & 0xff) << 24;
	pfrmCount += 3; //pass value 55 5a
	frmcount |= (*pfrmCount & 0xff) << 16;
	pfrmCount += 3; //pass value 55 5a
	frmcount |= (*pfrmCount & 0xff) << 8;
	pfrmCount += 3; //pass value 55 5a
	frmcount |= (*pfrmCount & 0xff) ;

	MLOGI("%s: frmcount: %d\n", __func__, frmcount);

	{
#define AR0820_REG_3000_PiXEL_NUM 673
		//int 3012 offset
		uint32_t offset = 36;//(pixel)
		uint16_t expValue = 0;
		uint8_t *pExptime = pMetaBuf->pBuffer[0];

		pExptime += (AR0820_REG_3000_PiXEL_NUM + offset) * VI200_ONE_PIEXL_BYTES;
		pExptime += 2; //pass 5a
		expValue = (*pExptime & 0xff) << 8;
		pExptime += 3; //pass value 55 5a
		expValue |= *pExptime & 0xff;

		//reg.addr = 0x3012;
		// VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[0], &reg);

		// MLOGI("%s: expTime metadata 0x%x, register value 0x%x\n", __func__, expValue, reg.value);
	}

	{
#define AR0820_META_4K_ONELINE_PIXEL 1949
#define AR0820_REG_335C_PiXEL_NUM 5
#define VI200_META_MCM_WIDTH_BYTE 2

		uint32_t onelineByte = (AR0820_META_4K_ONELINE_PIXEL * VI200_META_MCM_WIDTH_BYTE + 16) & (~0xf);
		uint32_t dGainOffset = 12;//(pixel)
		uint16_t dgValue = 0;
		uint8_t *pdgain = pMetaBuf->pBuffer[0];

		MLOGI("%s: online tag: %x\n", __func__, *(pdgain + onelineByte));

		pdgain += onelineByte + (AR0820_REG_335C_PiXEL_NUM + dGainOffset) * VI200_ONE_PIEXL_BYTES;
		pdgain += 2; //pass 5a
		dgValue = (*pdgain & 0xff) << 8;
		pdgain += 3; //pass value 55 5a
		dgValue |= *pdgain & 0xff;

		// reg.addr = 0x3362;
		// VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[0], &reg);

		// MLOGI("%s: DConvGain metadata 0x%x, register value 0x%x\n", __func__, dgValue, reg.value);

		uint32_t aGainOffset = 20;//(pixel)
		uint16_t aGValue = 0;
		uint8_t *pAgain = pMetaBuf->pBuffer[0];

		pAgain += onelineByte + (AR0820_REG_335C_PiXEL_NUM + aGainOffset) * VI200_ONE_PIEXL_BYTES;
		pAgain += 2; //pass 5a
		aGValue = (*pAgain & 0xff) << 8;
		pAgain += 3; //pass value 55 5a
		aGValue |= *pAgain & 0xff;

		// reg.addr = 0x3366;
		// VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[0], &reg);

		// MLOGI("%s: AGain metadata 0x%x, register value 0x%x\n", __func__, aGValue, reg.value);

		uint32_t aFineGainOffset = 28;//(pixel)
		uint16_t aFGValue = 0;
		uint8_t *pAFgain = pMetaBuf->pBuffer[0];

		pAFgain += onelineByte + (AR0820_REG_335C_PiXEL_NUM + aFineGainOffset) * VI200_ONE_PIEXL_BYTES;
		pAFgain += 2; //pass 5a
		aFGValue = (*pAgain & 0xff) << 8;
		pAFgain += 3; //pass value 55 5a
		aFGValue |= *pAgain & 0xff;

		// reg.addr = 0x336a;
		// VsiCamDeviceSensorGetRegister(pMetaInstance->hCamDevice[0], &reg);

		// MLOGI("%s: AfineGain metadata 0x%x, register value 0x%x\n", __func__, aFGValue, reg.value);

	}

	MLOGI("%s: (exit)\n", __func__);
	return RET_SUCCESS;
}

RESULT VsiVvdeviceParserTpgMetadata
(
	MediaBuffer_t *buffer
)
{
	MetadataBufInfo_t *pMetaBuf = &(((PicBufMetaData_t *)buffer->pMetaData)->Data.meta.metaBufInfo);

	if (pMetaBuf->pBuffer[0] == NULL) {
		return RET_NULL_POINTER;
	}

	// MLOGI("%s: address: 0x%x, start tag: 0x%x, bufSize:0x%x \n", __func__,
	// pMetaBuf->address[0], *(pMetaBuf->pBuffer[0]), pMetaBuf->bufferSize[0]);

	uint8_t *pBuffer = pMetaBuf->pBuffer[0];
	for (uint8_t i = 0; i < 32 ; i += 8) {
		MLOGI("0x%x 0x%x 0x%x 0x%x", pBuffer[i], pBuffer[i + 1], pBuffer[i + 2], pBuffer[i + 3]);
		// MLOGI("0x%x 0x%x 0x%x 0x%x",pBuffer[i+4], pBuffer[i+5], pBuffer[i+6], pBuffer[i+7]);
	}

	return RET_SUCCESS;
}
