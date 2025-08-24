// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#ifndef __oba_H__
#define __oba_H__


#include "xil_io.h"
#include "cam_device_buffer_api.h"
#include "cam_device_common.h"

#define MAX_OBA_PER_TILE    (4UL)

#define MAX_OBA_PER_ISP     (2UL)
#define XPAR_OBA_INSTANCES  (2UL)
#define XPAR_ISP_INSTANCE   (2UL)
#define XPAR_TILE_INSTANCES (1UL)


typedef enum oba_id {
	OBA_0_MP,
	OBA_1_SP,
	OBA_2_MP,
	OBA_3_SP,
	MAX_OBA,
	DUMMY_OBA_ID = 0XDEADFEED
} oba_id_t;


typedef enum oba_path {
	MAIN_PATH = 0,
	SELF_PATH,
	DUMMY_OBA_PATH_ID = 0XDEADFEED
} oba_path_t;


typedef enum oba_output_data_mode {
	RGB888_FORMAT,
	YUV444_FORMAT,
	YUV422_SP_FORMAT,
	YUV420_SP_FORMAT,
	Y_FORMAT,
	DUMMY_OBA_OUTPUT_DATA_MODE = 0XDEADFEED
} oba_data_mode_t;


typedef enum oba_data_type {
	YUV_420_8_BIT = 0x18,
	YUV_420_10_BIT = 0x19,
	YUV_422_8_BIT = 0x1E,
	YUV_422_10_BIT = 0x1F,
	RGB888 = 0x24,
	YUV_400_8_BIT = 0x20,
	YUV_400_10_BIT = 0x21,
	DUMMY_OBA_DATA_TYPE = 0XDEADFEED
} oba_data_type_t;


typedef enum oba_pixel_mode {
	SINGLE_PIXEL_MODE = 0,
	DUAL_PIXEL_MODE,
	QUAD_PIXEL_MODE,
	DUMMY_OBA_PIXEL_MODE = 0XDEADFEED
} oba_pixel_mode_t;


typedef enum oba_tile_id {
	OBA_TILE_0 = 0,
	OBA_MAX_TILE,
	OBA_DUMMY_TILE_ID = 0XDEADFEED
} oba_tile_id_t;


typedef enum oba_isp_instance {
	OBA_ISP0 = 0,
	OBA_ISP1,
	OBA_MAX_ISP_PER_TILE,
	DUMMY_OBA_ISP = 0XDEADFEED
} oba_isp_instance_t;

typedef enum oba_status {
	OBA_DISABLED = 0,
	OBA_ENABLED,
	DUMMY_OBA_STATUS = 0XDEADFEED
} oba_status_t;

typedef struct oba_isp_combination {

} oba_isp_split_t;

typedef struct oba_instance {

	/*
	 * Hi-addr, Low-addr , padding of 4bytes is required on RPU
	 * as the baseaddress will be aligned to 8 bytes as APU is 64 bit
	 */
	u32 BaseAddress[2];
	u32 hcamdev_instanceid;
	oba_id_t DeviceId;
	oba_path_t path_info;
	oba_pixel_mode_t pixle_mode;
	oba_data_mode_t data_format;
	oba_data_type_t data_type;
	oba_tile_id_t oba_tile_id;
	oba_status_t oba_is_enabled;
	oba_isp_instance_t isp_instance;           /*To which ISP is this oba connected*/

} __attribute((__packed__)) __attribute((aligned(8))) oba_inst_t ;

typedef struct oba_map {
	oba_inst_t oba_inst[OBA_MAX_TILE][OBA_MAX_ISP_PER_TILE][MAX_OBA_PER_ISP];
	int IsReady;
} __attribute((__packed__)) __attribute((aligned(8))) oba_map_t ;

typedef struct vvbench_vdev_oba {
	CamDeviceBufChainId_t path[2];
	int pathEnable[2];
	uint32_t hpId;
	CamDeviceInputType_t inputType;
	CamDeviceWorkMode_t workMode;
	CamDeviceOutputType_t outPutType;
	uint32_t pathOutType[2];
	uint32_t format[2];
	uint32_t dataBits[2];
} vvbench_vdev_oba_t ;


#define oba_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32( (UINTPTR)(((u8 *)BaseAddress) + (RegOffset)), (u32)(Data)) ; TRACE(OBA_RPU_INFO,"RPU:oba reg add 0x%x ,val 0x%x \n",(UINTPTR)(((u8 *)BaseAddress) + (RegOffset)),Data);
#define oba_ReadReg(BaseAddress, RegOffset) \
	Xil_In32( (UINTPTR) (((u8 *)BaseAddress) + (RegOffset)))


/*
* The configuration table for devices
*/
#define XPAR_ISP0_OBA_PIX_MODE_BASEADDR (0xE8540000) //oba0 base address  xE8540004
#define XPAR_ISP1_OBA_PIX_MODE_BASEADDR (0xE8541000)


#endif
