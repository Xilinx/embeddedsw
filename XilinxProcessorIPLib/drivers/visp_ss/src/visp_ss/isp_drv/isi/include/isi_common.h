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

/* VeriSilicon 2022 */

/**
 * @file isi_common.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup isi_iss CamerIc Driver API
 * @{
 *
 */
#ifndef __ISI_COMMON_H__
#define __ISI_COMMON_H__


#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/*!
 * interface version
 * =================
 * please increment the version if you add something new to the interface.
 * This helps upper layer software to deal with different interface versions.
 */
/*****************************************************************************/
#define ISI_INTERFACE_VERSION               6


/*****************************************************************************/
/* capabilities / configuration
 *****************************************************************************/

/**<  BusWidth */
#define ISI_BUSWIDTH_8BIT_ZZ                0x00000001     /**< to expand to a (possibly higher) resolution in marvin, the LSBs will be set to zero */
#define ISI_BUSWIDTH_8BIT_EX                0x00000002     /**< to expand to a (possibly higher) resolution in marvin, the LSBs will be copied from the MSBs */
#define ISI_BUSWIDTH_10BIT_EX               0x00000004     /**< /formerly known as ISI_BUSWIDTH_10BIT (at times no marvin derivative was able to process more than 10 bit) */
#define ISI_BUSWIDTH_10BIT_ZZ               0x00000008
#define ISI_BUSWIDTH_12BIT                  0x00000010
#define ISI_BUSWIDTH_10BIT      ( ISI_BUSWIDTH_10BIT_EX )

/**<  Vin Interface Type */
#define ISI_ITF_TYPE_MIPI                   0x00000001      /**< MIPI   conform data stream */
#define ISI_ITF_TYPE_LVDS                   0x00000002      /**< LVDS   conform data stream */
#define ISI_ITF_TYPE_DVP                    0x00000004      /**< DVP    conform data stream */
#define ISI_ITF_TYPE_BT601                  0x00000008      /**< BT601  conform data stream */
#define ISI_ITF_TYPE_BT656                  0x00000010      /**< BT656  conform data stream */
#define ISI_ITF_TYPE_BT1120                 0x00000020      /**< BT1120 conform data stream */


/**< Mode, operating mode of the image sensor in terms of output data format and timing data transmission */
#define ISI_MODE_YUV                        0x00000001
#define ISI_MODE_MONO                       0x00000002
#define ISI_MODE_BAYER                      0x00000004      /**< Bayer data with separate h/v sync lines */
#define ISI_MODE_DATA                       0x00000008      /**< Any binary data without line/column-structure, (e.g. already JPEG encoded) h/v sync lines act as data valid signals */

/**< MIPI */
#define ISI_MIPI_OFF                        0x80000000      //!< MIPI is disabled
#define ISI_MIPI_1LANES                     0x00000001
#define ISI_MIPI_2LANES                     0x00000002
#define ISI_MIPI_4LANES                     0x00000004
#define ISI_MIPI_APHY                       0x00000008
#define ISI_MIPI_CPHY                       0x00000010
#define ISI_MIPI_DPHY                       0x00000020

/**< PDAF */
#define ISI_PDAF_TYPE3                      0x00000001

/**< VCM */
#define ISI_MOTOR_VCM                       0x00000001
#define ISI_MOTOR_STEP                      0x00000002

/**< frame rate **/
#define ISI_FPS_QUANTIZE                    1000


/*****************************************************************************/
/**
 *          IsiSensorInputFormat_t
 *
 * @brief   mode of awb control to handle whitebalance during integration of AR082x
 */
/*****************************************************************************/
typedef enum IsiSensorInputFormat_e {
	ISI_FORMAT_YUV420P_8 = 0,      //!< YUV 420P  8-bit
	ISI_FORMAT_YUV420SP_8,         //!< YUV 420SP 8-bit
	ISI_FORMAT_YUV420P_10,         //!< YUV 420P  10-bit
	ISI_FORMAT_YUV420SP_10,        //!< YUV 420SP 10-bit
	ISI_FORMAT_YUV422P_8,          //!< YUV 422P  8-bit
	ISI_FORMAT_YUV422SP_8,         //!< YUV 422SP 8-bit
	ISI_FORMAT_YUV422P_10,         //!< YUV 422P  10-bit
	ISI_FORMAT_YUV422SP_10,        //!< YUV 422SP 10-bit
	ISI_FORMAT_RAW_8,              //!< RAW_8
	ISI_FORMAT_RAW_10,             //!< RAW_10
	ISI_FORMAT_RAW_12,             //!< RAW_12
	ISI_FORMAT_RAW_14,             //!< RAW_14
	ISI_FORMAT_RAW_16,             //!< RAW_16
	ISI_FORMAT_RAW_20,             //!< RAW_20
	ISI_FORMAT_RAW_24,             //!< RAW_24
	ISI_FORMAT_MONO_8,              //!< MONO_8
	DUMMY_ISI_FORMAT = 0xdeadfeed

} IsiSensorInputFormat_t;


/*****************************************************************************/
/**
 *          IsiSensorAwbMode_t
 *
 * @brief   mode of awb control to handle whitebalance during integration of AR082x
 */
/*****************************************************************************/
typedef enum IsiSensorAwbMode_e {
	ISI_SENSOR_AWB_MODE_NORMAL = 0,
	ISI_SENSOR_AWB_MODE_SENSOR,
	DUMMY_ISI_002 = 0xdeadfeed
} IsiSensorAwbMode_t;


/*****************************************************************************/
/**
 *          IsiColorComponent_t
 *
 * @brief   color components
 */
/*****************************************************************************/
typedef enum IsiColorComponent_e {
	ISI_COLOR_COMPONENT_RED = 0,
	ISI_COLOR_COMPONENT_GREENR = 1,
	ISI_COLOR_COMPONENT_GREENB = 2,
	ISI_COLOR_COMPONENT_BLUE = 3,
	ISI_COLOR_COMPONENT_MAX = 4,
	DUMMY_ISI_003 = 0xdeadfeed
} IsiColorComponent_t;


/*****************************************************************************/
/**
 *          IsiBayerPattern_t
 *
 * @brief   Bayer pattern of sensor
 */
/*****************************************************************************/
typedef enum IsiBayerPattern_e {
	ISI_BPAT_RGGB = 0x00,
	ISI_BPAT_GRBG = 0x01,
	ISI_BPAT_GBRG = 0x02,
	ISI_BPAT_BGGR = 0x03,
	ISI_BPAT_BGGIR = 0x10,
	ISI_BPAT_GRIRG = 0x11,
	ISI_BPAT_RGGIR = 0x12,
	ISI_BPAT_GBIRG = 0x13,
	ISI_BPAT_GIRRG = 0x14,
	ISI_BPAT_IRGGB = 0x15,
	ISI_BPAT_GIRBG = 0x16,
	ISI_BPAT_IRGGR = 0x17,
	ISI_BPAT_RGIRB = 0x18,
	ISI_BPAT_GRBIR = 0x19,
	ISI_BPAT_IRBRG = 0x20,
	ISI_BPAT_BIRGR = 0x21,
	ISI_BPAT_BGIRR = 0x22,
	ISI_BPAT_GBRIR = 0x23,
	ISI_BPAT_IRRBG = 0x24,
	ISI_BPAT_RIRGB = 0x25,
	ISI_BPAT_RCCC = 0x30,
	ISI_BPAT_RCCB = 0x40,
	ISI_BPAT_RYYCY = 0x50,
	DUMMY_ISI_004 = 0xdeadfeed
} IsiBayerPattern_t;

typedef struct IsiGainInfo_s {
	float32_t min;
	float32_t max;
	float32_t step;
} IsiGainInfo_t;

typedef struct IsiRange_s {
	float32_t min;
	float32_t max;
} IsiRange_t;

typedef struct IsiExpLineRange_s {
	uint16_t max;
	uint16_t min;
} IsiExpLineRange_t;

typedef struct IsiSensorGain_s {
	float32_t gain[4];
} IsiSensorGain_t;

typedef struct IsiSensorIntTime_s {
	float32_t intTime[4];
} IsiSensorIntTime_t;

typedef struct IsiSensorWin_s {
	uint16_t hStart;
	uint16_t vStart;
	uint16_t hSize;
	uint16_t vSize;
} IsiSensorWin_t;

typedef struct IsiSensorReg_s {
	uint16_t regAddr;
	uint16_t regVal;
} IsiSensorReg_t;

typedef struct IsiSensorBuffer_s {
	uint32_t pdata;
	uint32_t dataSize;
} IsiSensorBuffer_t;

/*****************************************************************************/
/**
 *          IsiSyncSignalPolarity_t
 *
 * @brief   sensor dvp output H/V sync polarity
 */
/*****************************************************************************/
typedef enum IsiSyncSignalPolarity_e {
	ISI_SYNC_POL_HIGH_ACIVE = 0,
	ISI_SYNC_POL_LOW_ACIVE = 1,
	DUMMY_ISI_005 = 0xdeadfeed
} IsiSyncSignalPolarity_t;

/*****************************************************************************/
/**
 *          IsiSyncSignalPolarity_t
 *
 * @brief   sensor output sample edge polarity
 */
/*****************************************************************************/
typedef enum IsiSampleEdgePolarity_e {
	ISI_SAMPLE_EDGE_POL_NEGATIVE = 0,
	ISI_SAMPLE_EDGE_POL_POSITIVE = 1,
	DUMMY_ISI_006 = 0xdeadfeed
} IsiSampleEdgePolarity_t;

/*****************************************************************************/
/**
 *          IsiSensorCCIRequence_t
 *
 * @brief   sensor output ccir sequence.
 */
/*****************************************************************************/
typedef enum IsiSensorCCIRSequence_e {
	ISI_SENSOR_CCIR_SEQUENCE_YCbYCr = 0,        /**< YCbYCr */
	ISI_SENSOR_CCIR_SEQUENCE_YCrYCb = 1,        /**< YCrYCb */
	ISI_SENSOR_CCIR_SEQUENCE_CbYCrY = 2,        /**< CbYCrY */
	ISI_SENSOR_CCIR_SEQUENCE_CrYCbY = 3,        /**< CrYCbY */
	DUMMY_ISI_007 = 0xdeadfeed
} IsiSensorCCIRSequence_t;


/*****************************************************************************/
/**
 *          IsiI2cBitWidth_t
 *
 * @brief   sensor I2C width information
 */
/*****************************************************************************/
typedef enum IsiI2cBitWidth_e {
	ISI_I2C_NONE = 0,
	ISI_I2C_8BIT = 1,
	ISI_I2C_16BIT = 2,
	DUMMY_ISI_008 = 0xdeadfeed
} IsiI2cBitWidth_t;

#ifdef __cplusplus
}
#endif

#endif /* __ISI_COMMON_H__ */
