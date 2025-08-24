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

#ifndef __ISI_METADATA_H__
#define __ISI_METADATA_H__
#include <types.h>
#include <isi_vvsensor.h>

#define ISI_METADATA_WIN_NUM_MAX 3


#ifdef __cplusplus
extern "C"
{
#endif

typedef union IsiMetadataAttr_s {
	struct {
		uint32_t support : 1;   /**< bit 0: 0-disable 1-enable */
		uint32_t regInfo : 1;   /**< bit 1 */
		uint32_t expTime : 1;   /**< bit 2 */
		uint32_t again : 1;   /**< bit 3 */
		uint32_t dgain : 1;   /**< bit 4 */
		uint32_t bls : 1;   /**< bit 5 */
		uint32_t hist : 1;   /**< bit 6 */
		uint32_t meanLuma : 1;   /**< bit 7 */
		uint32_t reservedEnable : 23;/**< bit 8:31 */
	} subAttr;
	uint32_t mainAttr;
} IsiMetadataAttr_t;

typedef struct IsiMetadataWinInfo_s {
	uint8_t winNum;
	IsiSensorWin_t metaWin[ISI_METADATA_WIN_NUM_MAX];
} IsiMetadataWinInfo_t;

typedef struct IsiMetadataParserInfo_s {
	IsiMetadataAttr_t validMask;

	uint32_t regNum;
	IsiSensorReg_t *pReg;

	uint8_t expFrmNum;
	IsiSensorIntTime_t expTime;
	IsiSensorGain_t aGain;
	IsiSensorGain_t dGain;
	IsiSensorBlc_t blc;
	IsiSensorHist_t hist;
	IsiSensorMeanLuma_t meanLuma;

} IsiMetadataParserInfo_t;


typedef struct IsiSensorMetadata_s {
	uint32_t chipId;   //sensor version id
	uint32_t frmCount;

	IsiMetadataParserInfo_t data;
} IsiSensorMetadata_t;


#ifdef __cplusplus
}
#endif

#endif //__ISI_METADATA_H__