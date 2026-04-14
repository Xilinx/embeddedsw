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


#include "vvformat.h"
#include "memory_manager.h"

#define LOGTAG "FORMAT_CONVERT"

#ifndef MIN
	#define MIN(a, b)   ( ((a)<(b)) ? (a) : (b) )
#endif /* MIN */

#ifndef MAX
	#define MAX(a, b)   ( ((a)>(b)) ? (a) : (b) )
#endif /* MAX */

static void SwapBufferInner(uint8_t *buf, size_t bufLen, PicBufMiDataSwap_t padType)
{
	if (NULL == buf) {
		return;
	}
	if (PIC_BUF_MI_NO_SWAP == padType) {
		return;
	}
	else if (PIC_BUF_MI_SWAP_BYTES == padType) {
		LOGD("%s in SWAP_BYTES", __func__);
		for (int i = 0; i + 1 < bufLen; i += 2) {
			uint8_t t = buf[i];
			buf[i] = buf[i + 1];
			buf[i + 1] = t;
		}
	}
	else if (padType == PIC_BUF_MI_SWAP_WORDS) {
		LOGD("%s in SWAP_WORDS", __func__);
		for (int i = 0; i + 3 < bufLen; i += 4) {
			uint8_t t1 = buf[i];
			uint8_t t2 = buf[i + 1];
			buf[i] = buf[i + 2];
			buf[i + 1] = buf[i + 3];
			buf[i + 2] = t1;
			buf[i + 3] = t2;
		}
	}
	else if (padType == PIC_BUF_MI_SWAP_DOUBLE_WORDS) {
		LOGD("%s in SWAP_DOUBLE_WORDS", __func__);
		for (int i = 0; i + 7 < bufLen; i += 8) {
			uint8_t t1 = buf[i];
			uint8_t t2 = buf[i + 1];
			uint8_t t3 = buf[i + 2];
			uint8_t t4 = buf[i + 3];
			buf[i] = buf[i + 4];
			buf[i + 1] = buf[i + 5];
			buf[i + 2] = buf[i + 6];
			buf[i + 3] = buf[i + 7];
			buf[i + 4] = t1;
			buf[i + 5] = t2;
			buf[i + 6] = t3;
			buf[i + 7] = t4;
		}
	}
	else if (padType == PIC_BUF_MI_SWAP_FOUR_WORDS) {
		LOGD("%s in SWAP_FOUR_WORDS", __func__);
		for (int i = 0; i + 15 < bufLen; i += 16) {
			uint8_t temp[8] = {0};
			memcpy(temp, buf + i, 8);
			memcpy(buf + i, buf + i + 8, 8);
			memcpy(buf + i + 8, temp, 8);
		}
	}
}

RESULT ConvertRawToRGBA
(
	BufIdentity *pBuffer
)
{
	RESULT result = RET_SUCCESS;
	if (NULL == pBuffer) {
		return RET_NULL_POINTER;
	}
	LOGD("%s in", __func__);
	// allocate local buffer
	uint8_t *pLocBuf = pBuffer->bufferInstance;
	if (NULL == pLocBuf) {
		LOGE("%s get NULL pLocBuf\n", __func__);
		return RET_NULL_POINTER;
	}
	SwapBufferInner(pLocBuf, pBuffer->buff_size, pBuffer->swap.rawSwap);
	uint8_t *pYBase = pLocBuf;
	switch (pBuffer->format) {
		case CAMDEV_PIX_FMT_RAW8: {
				break;
			}
		case CAMDEV_PIX_FMT_RAW10: {
				uint32_t pY = 0, startY = 0;
				uint8_t Y[5] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; rowIndex++) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t x = 4; x <= (pBuffer->widthBytes); x += 5) {
						Y[0] = pYBase[pY++];
						Y[1] = pYBase[pY++];
						Y[2] = pYBase[pY++];
						Y[3] = pYBase[pY++];
						Y[4] = pYBase[pY++];
						pYBase[startY++] = (Y[1] << 6) + (Y[0] >> 2);
						pYBase[startY++] = (Y[2] << 4) + (Y[1] >> 4);
						pYBase[startY++] = (Y[3] << 2) + (Y[2] >> 6);
						pYBase[startY++] = Y[4];
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE1: {
				uint32_t pY = 0, startY = 0;
				uint8_t channelY[2] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 1; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 2) {
						channelY[0] = pYBase[pY++];
						channelY[1] = pYBase[pY++];
						pYBase[startY++] = (channelY[1] << 6) + (channelY[0] >> 2);
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE0: {
				int pY = 0, startY = 0;
				uint8_t channelY[4] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 3; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 4) {
						channelY[0] = pYBase[pY++];
						channelY[1] = pYBase[pY++];
						channelY[2] = pYBase[pY++];
						channelY[3] = pYBase[pY++];
						pYBase[startY++] = (channelY[1] << 6) + (channelY[0] >> 2);
						pYBase[startY++] = (channelY[2] << 4) + (channelY[1] >> 4);
						pYBase[startY++] = (channelY[3] << 2) + (channelY[2] >> 6);
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW12: {
				uint32_t pY = 0, startY = 0;
				uint8_t Y[3] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; rowIndex++) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t x = 0; x < (pBuffer->widthBytes); x += 3) {
						Y[0] = pYBase[pY++];
						Y[1] = pYBase[pY++];
						Y[2] = pYBase[pY++];
						pYBase[startY++] = (Y[1] << 4) + (Y[0] >> 4);
						pYBase[startY++] = Y[2];
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1: {
				uint32_t pY = 0, startY = 0;
				uint8_t channelY[2] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 1; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 2) {
						channelY[0] = pYBase[pY++];
						channelY[1] = pYBase[pY++];
						pYBase[startY++] = (channelY[1] << 4) + (channelY[0] >> 4);
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE0: {
				int pY = 0, startY = 0;
				uint8_t channelY[8] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 7; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 8) {
						memcpy(channelY, pYBase + pY, 8);
						pY += 8;
						pYBase[startY++] = (channelY[1] << 4) | (channelY[0] >> 4);
						pYBase[startY++] = channelY[2];
						pYBase[startY++] = (channelY[4] << 4) | (channelY[3] >> 4);
						pYBase[startY++] = channelY[5];
						pYBase[startY++] = (channelY[7] << 4) | (channelY[6] >> 4);
					}
				}
				break;
			}

		case CAMDEV_PIX_FMT_RAW14: {
				uint32_t pY = 0, startY = 0;
				uint8_t Y[7] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; rowIndex++) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t x = 6; x <= (pBuffer->widthBytes); x += 7) {
						memcpy(Y, pYBase + pY, 7);
						pY += 7;
						pYBase[startY++] = (Y[1] << 2) | (Y[0] >> 6);
						pYBase[startY++] = (Y[3] << 4) | (Y[2] >> 4);
						pYBase[startY++] = (Y[5] << 6) | (Y[4] >> 2);
						pYBase[startY++] = Y[6];
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE0: {
				int pY = 0, startY = 0;
				uint8_t channelY[16] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 15; colByteIndex < (pBuffer->widthBytes); colByteIndex += 16) {
						memcpy(channelY, pYBase + pY, 16);
						pY += 16;
						pYBase[startY++] = (channelY[1] << 2) | (channelY[0] >> 6);
						pYBase[startY++] = (channelY[3] << 4) | (channelY[2] >> 4);
						pYBase[startY++] = (channelY[5] << 6) | (channelY[4] >> 2);
						pYBase[startY++] = channelY[6];
						pYBase[startY++] = (channelY[8] << 2) | (channelY[7] >> 6);
						pYBase[startY++] = (channelY[10] << 4) | (channelY[9] >> 4);
						pYBase[startY++] = (channelY[12] << 6) | (channelY[11] >> 2);
						pYBase[startY++] = channelY[13];
						pYBase[startY++] = (channelY[15] << 2) | (channelY[14] >> 6);
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1: {
				uint32_t pY = 0, startY = 0;
				uint8_t channelY[2] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 1; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 2) {
						channelY[0] = pYBase[pY++];
						channelY[1] = pYBase[pY++];
						pYBase[startY++] = (channelY[1] << 2) + (channelY[0] >> 6);
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW16: {
				uint32_t pY = 0, startY = 0;
				uint8_t channelY[2] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 1; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 2) {
						channelY[0] = pYBase[pY++];
						channelY[1] = pYBase[pY++];
						pYBase[startY++] = channelY[1];
					}
				}
				break;
			}
		case CAMDEV_PIX_FMT_RAW24: {
				uint32_t pY = 0, startY = 0;
				uint8_t channelY[3] = {0};
				for (uint32_t rowIndex = 0; rowIndex < pBuffer->height; ++rowIndex) {
					pY = rowIndex * pBuffer->widthBytes;
					startY = rowIndex * pBuffer->width;
					for (uint32_t colByteIndex = 2; colByteIndex <= (pBuffer->widthBytes); colByteIndex += 3) {
						channelY[0] = pYBase[pY++];
						channelY[1] = pYBase[pY++];
						channelY[2] = pYBase[pY++];
						pYBase[startY++] = channelY[2];
					}
				}
				break;
			}
		default: {
				result = RET_OUTOFRANGE;
				LOGE("%s get unsupported type", __func__);
				break;
			}
	}

	uint8_t *pRGBA = mm_malloc(4 * pBuffer->width * pBuffer->height);
	uint8_t *pRGBATmp = pRGBA;
	uint32_t x, y;
	int posY = 0;
	for (y = 0; y < pBuffer->height; y++) {
		for (x = 0; x < pBuffer->width; x++) {
			uint8_t Y = pYBase[posY++];
			*pRGBATmp++ = (unsigned char)MAX(0, MIN(255, Y));
			*pRGBATmp++ = (unsigned char)MAX(0, MIN(255, Y));
			*pRGBATmp++ = (unsigned char)MAX(0, MIN(255, Y));
			*pRGBATmp++ = 0xFF;
		}
	}
	mm_free(pLocBuf);
	pBuffer->bufferInstance = pRGBA;
	pBuffer->buff_size = 4 * pBuffer->width * pBuffer->height;
	LOGD("%s out", __func__);
	return result;
}
