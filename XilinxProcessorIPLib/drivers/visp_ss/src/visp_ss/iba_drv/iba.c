#include <string.h>
// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <stdio.h>
#include "xil_types.h"
#include "iba.h"
#include "sensor_cmd.h"
#include "xil_assert.h"
#include "mbox_cmd.h"
//#include "vvdevice.h"
#include "vlog.h"
#include "cam_device_app.h"

#define LOGTAG "IBA_APU"

iba_inst_t *iba_LookupConfig(iba_isp_instance_t isp_no, iba_id_t iba_no);
int iba_CfgInitialize(iba_inst_t *InstancePtr, const iba_inst_t *ConfigPtr);
int iba_set_resolution(iba_inst_t* InstancePtr, u32 v_res, u32 h_res);
int iba_set_fps(iba_inst_t* InstancePtr, int fps);
int iba_set_virtual_channel(iba_inst_t* InstancePtr, int virtual_channel);
int iba_set_fifo_write_mode(iba_inst_t* InstancePtr, int fifomode);

iba_inst_t iba_instance;

iba_inst_t IBA_ConfigTable[XPAR_ISP_INSTANCE][XPAR_IBA_INSTANCES] = {
	{
		//ISP-0
		{   /*Tile-0, ISP-0, IBA0 configuration*/
			.DeviceId = IBA_0,
			.BaseAddress = XPAR_TILE0_IBA0_BASEADDR,
			.data_format = 0x10, //RAW8
			.hres = 1920,
			.vres = 1080,
			.iba_is_enabled = IBA_DISABLED, /*Disabled by default,
																 it is automatically enabled in the code,
																 depending on port selection*/
			.hblank_prog = 0x50,
			.vblank_prog = 0x23,
			.virtual_channel_id = VC0,
			.input_pixel_width = QUAD_PIXEL_WIDTH,
			.isp_instance = ISP0,
		},
		{
			//Tile-0, ISP-0, IBA1 configuration
			.DeviceId = IBA_1,
			.BaseAddress = XPAR_TILE0_IBA1_BASEADDR,
			.data_format = 0x10, //RAW8
			.hres = 1920,
			.vres = 1080,
			.iba_is_enabled = IBA_DISABLED, /*Disabled by default,
																 it is automatically enabled in the code,
																 depending on port selection*/
			.hblank_prog = 0x50,
			.vblank_prog = 0x23,
			.virtual_channel_id = VC0,
			.input_pixel_width = QUAD_PIXEL_WIDTH,
			.isp_instance = ISP0,
		},
		{
			//Tile-0, ISP-0, IBA2 configuration
			.DeviceId = IBA_2,
			.BaseAddress = XPAR_TILE0_IBA2_BASEADDR,
			.data_format = 0x10, //RAW8
			.hres = 1920,
			.vres = 1080,
			.iba_is_enabled = IBA_DISABLED, /*Disabled by default,
																 it is automatically enabled in the code,
																 depending on port selection*/
			.hblank_prog = 0x50,
			.vblank_prog = 0x23,
			.virtual_channel_id = VC1,
			.input_pixel_width = QUAD_PIXEL_WIDTH,
			.isp_instance = ISP0,
		},
		{
			//Tile-0, ISP-0, IBA3 configuration,This IBA can switch between ISP0-IBA3 <-> ISP1-IBA1.
			.DeviceId = IBA_3,
			.BaseAddress = XPAR_TILE0_IBA3_BASEADDR,
			.data_format = 0x10, //RAW8
			.hres = 1920,
			.vres = 1080,
			.iba_is_enabled = IBA_DISABLED, /*Disabled by default,
																 it is automatically enabled in the code,
																 depending on port selection*/
			.hblank_prog = 0x50,
			.vblank_prog = 0x23,
			.virtual_channel_id = VC0,
			.input_pixel_width = QUAD_PIXEL_WIDTH,
			.isp_instance = ISP0,
		}

	},
	{
		//ISP-1
		{
			//Tile-0, ISP-1, IBA0 configuration
			.DeviceId = IBA_4,
			.BaseAddress = XPAR_TILE0_IBA4_BASEADDR,
			.data_format = 0x10, //RAW8
			.hres = 1920,
			.vres = 1080,
			.iba_is_enabled = IBA_DISABLED, /*Disabled by default,
																 it is automatically enabled in the code,
																 depending on port selection*/
			.hblank_prog = 0x50,
			.vblank_prog = 0x23,
			.virtual_channel_id = VC1,
			.input_pixel_width = QUAD_PIXEL_WIDTH,
			.isp_instance = ISP1,
		},
		{
			//Tile-0, ISP-1, IBA1 configuration.This IBA can switch between ISP0-IBA3 <-> ISP1-IBA1.
			.DeviceId = IBA_3,
			.BaseAddress = XPAR_TILE0_IBA3_BASEADDR,
			.data_format = 0x10, //RAW8
			.hres = 1920,
			.vres = 1080,
			.iba_is_enabled = IBA_DISABLED, /*Disabled by default,
																 it is automatically enabled in the code,
																 depending on port selection*/
			.hblank_prog = 0x50,
			.vblank_prog = 0x23,
			.virtual_channel_id = VC0,
			.input_pixel_width = QUAD_PIXEL_WIDTH,
			.isp_instance = ISP1,
		}

	}
};

iba_inst_t *iba_LookupConfig(iba_isp_instance_t isp_no, iba_id_t iba_no)
{

	iba_inst_t *ConfigPtr = NULL;
	ConfigPtr = &IBA_ConfigTable[isp_no][iba_no];
	return ConfigPtr;
}


int iba_CfgInitialize(iba_inst_t *InstancePtr, const iba_inst_t *ConfigPtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	*InstancePtr = *ConfigPtr; // Copy structure content
	return 0; // XST_SUCCESS
}


Payload_packet packet;

RESULT IBA_init_send_command
(
	CamDeviceHandle_t hCamDevice,
	vvbench_vdev_iba_t *caseCtx,
	int index
)
{
	RESULT result = RET_SUCCESS;
	iba_inst_t *ConfigPtr;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;

	uint8_t *p_data;
	uint32_t instanceid = 0, port_no = 0;


	if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->inputType) {
		instanceid = pCamDevCtx->instanceId;

		LOGI("APU Instanceid[%d] -> %d , ISP-%d \n", index, pCamDevCtx->instanceId, caseCtx->hpId);

		LOGI("APU: Sizeof iba_inst is %d portid:%d\n", sizeof(iba_inst_t),
		     caseCtx->portId);

		port_no = caseCtx->portId - 1;

		if ((port_no > 3 || port_no < 0) && caseCtx->hpId == ISP0) {
			LOGE("ERROR: port_no = %d is invalid. Valid range-> [0,1,2,3] \n", port_no);
			return -1;
		} else if ((port_no > 1 || port_no < 0) && caseCtx->hpId == ISP1) {
			LOGE("ERROR: port_no = %d is invalid. Valid range-> [0,1] \n", port_no);
			return -1;
		}

		ConfigPtr = iba_LookupConfig(caseCtx->hpId, port_no);

		ConfigPtr->iba_is_enabled = IBA_ENABLED;
		ConfigPtr->vres = caseCtx->sensorHeight;
		ConfigPtr->hres = caseCtx->sensorWidth;
		ConfigPtr->isp_instance = caseCtx->hpId;
		ConfigPtr->input_pixel_width = caseCtx->ppc;
		if (caseCtx->virtualChannelId != 0xFFFFFFFF)
			ConfigPtr->virtual_channel_id = caseCtx->virtualChannelId;
		LOGI("APU DEBUG: hcamdev->instanceid %d vcId:%d\n", pCamDevCtx->instanceId,
		     ConfigPtr->virtual_channel_id);

		if (ConfigPtr->iba_is_enabled == IBA_ENABLED) {
			LOGI("APU-Debug: hres-%d,vres-%d,instanceid %d, ConfigPtr->isp_instance %d \n",
			     ConfigPtr->hres, ConfigPtr->vres, instanceid, ConfigPtr->isp_instance);

			LOGI("APU debug :inputtypes %d,%d", caseCtx->inputType);

			memset(&packet, 0, sizeof(Payload_packet));
			p_data = packet.payload_data;
			packet.cookie = 0x99;
			packet.type = CMD;

			memcpy(p_data, &instanceid, sizeof(uint32_t));
			p_data += sizeof(uint32_t);
			packet.payload_size += sizeof(uint32_t);
			xil_printf("packet.payload_size %d \n", packet.payload_size);

			memcpy(p_data, ConfigPtr, sizeof(iba_inst_t));
			packet.payload_size += sizeof(iba_inst_t);
			xil_printf("packet.payload_size %d \n", packet.payload_size);
			p_data += sizeof(iba_inst_t);


			LOGI("IBA_init_send_command,sizeof iba_inst_t = %d, packet.payload_size %d \n", sizeof(iba_inst_t),
			     packet.payload_size);

			result = Send_Command(APU_2_RPU_MB_CMD_IBA_INIT, &packet, sizeof(packet), dest_cpu_id, src_cpu_id);
			if (RET_SUCCESS != result)
				return RET_FAILURE;
			apu_wait_for_ACK(packet.cookie, packet.payload_data); //replace with wait_response();


		}
	}


	return result;
}
