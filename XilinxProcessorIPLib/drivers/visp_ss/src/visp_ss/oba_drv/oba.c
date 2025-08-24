#include <string.h>
// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <stdio.h>
#include "oba.h"
#include "xil_types.h"
#include "sensor_cmd.h"
//#include "vvdevice.h"
#include "xil_assert.h"
#include "mbox_cmd.h"
#include "vlog.h"
#include "cam_device_app.h"
#define LOGTAG "OBA_APU"

oba_inst_t *oba_LookupConfig(oba_isp_instance_t isp_no, oba_id_t oba_no);
int oba_CfgInitialize(oba_inst_t *InstancePtr, const oba_inst_t *ConfigPtr);
int oba_set_resolution(oba_inst_t* InstancePtr, u32 v_res, u32 h_res);
int oba_set_fps(oba_inst_t* InstancePtr, int fps);
int oba_set_virtual_channel(oba_inst_t* InstancePtr, int virtual_channel);
int oba_set_fifo_write_mode(oba_inst_t* InstancePtr, int fifomode);
int oba_init(oba_map_t *InstancePtr);
RESULT OBA_init_send_command
(
	CamDeviceHandle_t hCamDevice,
	vvbench_vdev_oba_t *caseCtx,
	int index
);


oba_inst_t OBA_ConfigTable[XPAR_ISP_INSTANCE][MAX_OBA_PER_ISP] = {

	{
		//isp
		{   /*Tile-0, ISP-0, OBA0 configuration*/
			.BaseAddress[0] = XPAR_ISP0_OBA_PIX_MODE_BASEADDR,
			.DeviceId = OBA_0_MP,
			.path_info = MAIN_PATH,
			.pixle_mode = QUAD_PIXEL_MODE,
			.data_format = RGB888_FORMAT,
			.data_type = YUV_420_8_BIT,
			.oba_tile_id = OBA_TILE_0,
			.oba_is_enabled = OBA_DISABLED,
			.isp_instance = OBA_ISP0
		},
		{
			//Tile-0, ISP-0, oba1 configuration
			.BaseAddress[0] = XPAR_ISP0_OBA_PIX_MODE_BASEADDR,
			.DeviceId = OBA_1_SP,
			.path_info = SELF_PATH,
			.pixle_mode = QUAD_PIXEL_MODE,
			.data_format = RGB888_FORMAT,
			.data_type = YUV_420_8_BIT,
			.oba_tile_id = OBA_TILE_0,
			.oba_is_enabled = OBA_DISABLED,
			.isp_instance = OBA_ISP0
		}

	},
	{
		{
			//Tile-0, ISP-1, oba0 configuration
			.BaseAddress[0] = XPAR_ISP1_OBA_PIX_MODE_BASEADDR,
			.DeviceId = OBA_2_MP,
			.path_info = MAIN_PATH,
			.pixle_mode = QUAD_PIXEL_MODE,
			.data_format = RGB888_FORMAT,
			.data_type = YUV_420_8_BIT,
			.oba_tile_id = OBA_TILE_0,
			.oba_is_enabled = OBA_DISABLED,
			.isp_instance = OBA_ISP1
		},
		{
			//Tile-0, ISP-1, oba1 configuration
			.BaseAddress[0] = XPAR_ISP1_OBA_PIX_MODE_BASEADDR,
			.DeviceId = OBA_3_SP,
			.path_info = SELF_PATH,
			.pixle_mode = QUAD_PIXEL_MODE,
			.data_format = RGB888_FORMAT,
			.data_type = YUV_420_8_BIT,
			.oba_tile_id = OBA_TILE_0,
			.oba_is_enabled = OBA_DISABLED,
			.isp_instance = OBA_ISP1
		}


	}

};


oba_inst_t *oba_LookupConfig(oba_isp_instance_t isp_no, oba_id_t oba_no)
{

	oba_inst_t *ConfigPtr = NULL;
	ConfigPtr = &OBA_ConfigTable[isp_no][oba_no];
	return ConfigPtr;
}


int oba_CfgInitialize(oba_inst_t *InstancePtr, const oba_inst_t *ConfigPtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	*InstancePtr = *ConfigPtr; // Copy structure content
	return 0; // XST_SUCCESS
}


#if 0
int oba_map_init(oba_map_t *InstancePtr, VvbenchVvdev_t *caseCtx)
{

	oba_inst_t *ConfigPtr = NULL;
	uint32_t tile_no = 0, isp_no = 0, oba_no = 0, path = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	LOGI("APU: Sizeof oba_inst is %d \n", sizeof(oba_inst_t));

	for (int index = 0; index < caseCtx->totalInstance; index++) {
		for (tile_no = TILE_0; tile_no < XPAR_TILE_INSTANCES; tile_no++) {
			for (isp_no = ISP0; isp_no < XPAR_ISP_INSTANCE; isp_no++) {
				for (oba_no = OBA_0_MP; oba_no < MAX_OBA_PER_ISP; oba_no++) {
					LOGI("APU: tile :%d  isp_no:%d oba_no:%d\n", tile_no, isp_no, oba_no);
					LOGI("APU: sizeof oba_inst is %d workmode :%d\n", sizeof(oba_inst_t),
					     caseCtx->instanceCfgCtx[index].workCfg.workMode);

					ConfigPtr = oba_LookupConfig(tile_no, isp_no, oba_no);

					if (ConfigPtr == NULL)
						continue;

					/*OBA initialization needed only for non-MCM streaming cases.*/
					if (caseCtx->instanceCfgCtx[index].hpId == isp_no
					    && CAMDEV_INPUT_TYPE_SENSOR == caseCtx->instanceCfgCtx[index].inputType
					    && CAMDEV_WORK_MODE_STREAM == caseCtx->instanceCfgCtx[index].workCfg.workMode
					    && OBA_DISABLED == ConfigPtr->oba_is_enabled
					    && ((caseCtx->instanceCfgCtx[index].outPutType == CAMDEV_OUTPUT_TYPE_ONLINE)
						|| (caseCtx->instanceCfgCtx[index].outPutType == CAMDEV_OUTPUT_TYPE_BOTH)
						|| (caseCtx->instanceCfgCtx[index].instancePath[(oba_no % 2)].pathOutType == 1)
						|| (caseCtx->instanceCfgCtx[index].instancePath[(oba_no % 2)].pathOutType == 0))
					   ) {
						LOGI("AMD-Debug search for tile %d,isp %d,oba %d \n", tile_no, isp_no, oba_no);

						//Ref:Bufpath is MP/SP1	caseCtx->instanceCfgCtx[instanceId].instancePath[bufPath].path = bufPath;

						if (oba_no % 2 == CAMDEV_BUFCHAIN_MP) {

							path = CAMDEV_BUFCHAIN_MP;
							if (caseCtx->instanceCfgCtx[index].instancePath[path].path == path
							    && caseCtx->instanceCfgCtx[index].instancePath[path].pathEnable == TRUE)
								ConfigPtr->oba_is_enabled = OBA_ENABLED;
						}
						if (oba_no % 2 == CAMDEV_BUFCHAIN_SP1) {
							path = CAMDEV_BUFCHAIN_SP1;
							if (caseCtx->instanceCfgCtx[index].instancePath[path].path == path
							    && caseCtx->instanceCfgCtx[index].instancePath[path].pathEnable == TRUE)
								ConfigPtr->oba_is_enabled = OBA_ENABLED;
						}

						if (ConfigPtr->oba_is_enabled == OBA_ENABLED) {
							switch (caseCtx->instanceCfgCtx[index].instancePath[path].format) {
								case CAMDEV_PIX_FMT_RGB888P:
								case CAMDEV_PIX_FMT_RGB888:
									ConfigPtr->data_format = RGB888_FORMAT ;
									ConfigPtr->data_type = RGB888;
									break;

								case CAMDEV_PIX_FMT_YUV422SP:

									if (caseCtx->instanceCfgCtx[index].instancePath[path].dataBits == 8) {
										ConfigPtr->data_format = YUV422_SP_FORMAT;
										ConfigPtr->data_type = YUV_422_8_BIT;

									} else if (caseCtx->instanceCfgCtx[index].instancePath[path].dataBits == 10) {
										ConfigPtr->data_format = YUV422_SP_FORMAT;
										ConfigPtr->data_type = YUV_422_10_BIT;
									}
									break;

								case CAMDEV_PIX_FMT_YUV420SP:
									if (caseCtx->instanceCfgCtx[index].instancePath[path].dataBits == 8) {
										ConfigPtr->data_format = YUV420_SP_FORMAT;
										ConfigPtr->data_type = YUV_420_8_BIT;

									} else if (caseCtx->instanceCfgCtx[index].instancePath[path].dataBits == 10) {
										ConfigPtr->data_format = YUV420_SP_FORMAT;
										ConfigPtr->data_type = YUV_420_10_BIT;
									}

									break;
								case CAMDEV_PIX_FMT_YUV400:
									if (caseCtx->instanceCfgCtx[index].instancePath[path].dataBits == 8) {
										ConfigPtr->data_format = Y_FORMAT;
										ConfigPtr->data_type = YUV_400_8_BIT;

									} else if (caseCtx->instanceCfgCtx[index].instancePath[path].dataBits == 10) {
										ConfigPtr->data_format = Y_FORMAT;
										ConfigPtr->data_type = YUV_400_10_BIT;
									}

									break;

								default:
									LOGI("Unsupported format configured to OBA.\n");
							}

							oba_CfgInitialize(&InstancePtr->oba_inst[tile_no][isp_no][oba_no], ConfigPtr);
							LOGI("%s %d  devieId:%d baseAddress:%x tile_no:%d isp_no:%d   oba_no:%d path_info:%d isp_inst:%d\n",
							     __func__, __LINE__, ConfigPtr->DeviceId,
							     ConfigPtr->BaseAddress[0], tile_no, isp_no, oba_no, ConfigPtr->path_info, ConfigPtr->isp_instance);

						}
						LOGI("%s %d\n", __func__, __LINE__);


					} else
						continue;

				}
			}

		}
	}

	InstancePtr->IsReady = 1; // oba_ENABLED
	return 0;
}
#endif
RESULT OBA_init_send_command
(
	CamDeviceHandle_t hCamDevice,
	vvbench_vdev_oba_t *caseCtx,
	int index
)
{
	RESULT result = RET_SUCCESS;
	CamDeviceBufChainId_t path = 0;
	oba_map_t oba_map;
	oba_path_t oba_no = 0;
	uint32_t instanceid = 0;
	memset(&oba_map, 0, sizeof(oba_map));

	/*OBA initialization needed only for non-MCM streaming cases.*/

	//Ref:Bufpath is MP/SP1	caseCtx->instanceCfgCtx[instanceId].instancePath[bufPath].path = bufPath;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;

	for (path = CAMDEV_BUFCHAIN_MP; path <= CAMDEV_BUFCHAIN_SP1; path++) {
		if (caseCtx->path[path] == CAMDEV_BUFCHAIN_MP && caseCtx->pathEnable[path] == TRUE) {
			oba_no = MAIN_PATH;
			path = CAMDEV_BUFCHAIN_MP;
			xil_printf("APU_DEBUG: OBA  at line %s-%d \n", __func__, __LINE__);

		} else if (caseCtx->path[path] == CAMDEV_BUFCHAIN_SP1 && caseCtx->pathEnable[path] == TRUE) {
			oba_no = SELF_PATH;
			path = CAMDEV_BUFCHAIN_SP1;
			xil_printf("APU_DEBUG: OBA  at line %s-%d \n", __func__, __LINE__);

		} else
			continue;


		int isp_no = caseCtx->hpId;

		LOGI("AMD-Debug search for isp no. in tile %d,oba %d, path %d \n", isp_no % 2, oba_no, path);

		oba_inst_t *ConfigPtr = oba_LookupConfig(isp_no % 2, oba_no);

		if (CAMDEV_INPUT_TYPE_SENSOR == caseCtx->inputType
		    && CAMDEV_WORK_MODE_STREAM == caseCtx->workMode
		    && OBA_DISABLED == ConfigPtr->oba_is_enabled
		    && ((caseCtx->outPutType == CAMDEV_OUTPUT_TYPE_ONLINE) ||
			(caseCtx->outPutType == CAMDEV_OUTPUT_TYPE_BOTH) ||
			(caseCtx->pathOutType[oba_no % 2] == 1) ||
			(caseCtx->pathOutType[oba_no % 2] == 0))
		   ) {

			ConfigPtr->oba_is_enabled = OBA_ENABLED;
			ConfigPtr->isp_instance = isp_no;
			ConfigPtr->path_info = oba_no;
			ConfigPtr->hcamdev_instanceid = pCamDevCtx->instanceId;

			switch (caseCtx->format[path]) {
				case CAMDEV_PIX_FMT_RGB888P:
				case CAMDEV_PIX_FMT_RGB888:
					ConfigPtr->data_format = RGB888_FORMAT ;
					ConfigPtr->data_type = RGB888;
					break;

				case CAMDEV_PIX_FMT_YUV422SP:

					if (caseCtx->dataBits[path] == 8) {
						ConfigPtr->data_format = YUV422_SP_FORMAT;
						ConfigPtr->data_type = YUV_422_8_BIT;

					} else if (caseCtx->dataBits[path] == 10) {
						ConfigPtr->data_format = YUV422_SP_FORMAT;
						ConfigPtr->data_type = YUV_422_10_BIT;
					}
					break;

				case CAMDEV_PIX_FMT_YUV420SP:
					if (caseCtx->dataBits[path] == 8) {
						ConfigPtr->data_format = YUV420_SP_FORMAT;
						ConfigPtr->data_type = YUV_420_8_BIT;

					} else if (caseCtx->dataBits[path] == 10) {
						ConfigPtr->data_format = YUV420_SP_FORMAT;
						ConfigPtr->data_type = YUV_420_10_BIT;
					}

					break;
				case CAMDEV_PIX_FMT_YUV400:
					if (caseCtx->dataBits[path] == 8) {
						ConfigPtr->data_format = Y_FORMAT;
						ConfigPtr->data_type = YUV_400_8_BIT;

					} else if (caseCtx->dataBits[path] == 10) {
						ConfigPtr->data_format = Y_FORMAT;
						ConfigPtr->data_type = YUV_400_10_BIT;
					}

					break;

				default:
					LOGI("Unsupported format configured to OBA.\n");
					return RET_WRONG_CONFIG;
			}

			//oba_CfgInitialize(&InstancePtr->oba_inst[isp_no/2][isp_no][oba_no], ConfigPtr);
			LOGI("%s %d  devieId:%d baseAddress:%x tile_no:%d isp_no:%d   oba_no:%d path_info:%d isp_inst:%d instanceId:%d\n",
			     __func__, __LINE__, ConfigPtr->DeviceId,
			     ConfigPtr->BaseAddress[0], isp_no / 2, isp_no % 2, oba_no, ConfigPtr->path_info,
			     ConfigPtr->isp_instance, ConfigPtr->hcamdev_instanceid);


			LOGI("%s %d\n", __func__, __LINE__);

			Payload_packet packet;
			memset(&packet, 0, sizeof(Payload_packet));
			uint8_t *p_data = &packet.payload_data;
			packet.cookie = 0x99;
			packet.type = CMD;

			memcpy(p_data, &instanceid, sizeof(uint32_t));
			packet.payload_size += sizeof(uint32_t);
			xil_printf("packet.payload_size %d \n", packet.payload_size);
			p_data += sizeof(uint32_t);

			memcpy(p_data, ConfigPtr, sizeof(oba_inst_t));
			packet.payload_size += sizeof(oba_inst_t);
			xil_printf("packet.payload_size %d \n", packet.payload_size);
			p_data += sizeof(oba_inst_t);

			LOGI("Sizeof iba map structure %d \n", sizeof(oba_inst_t));
			result = Send_Command(APU_2_RPU_MB_CMD_OBA_INIT, &packet, sizeof(packet), dest_cpu_id, src_cpu_id);
			if (RET_SUCCESS != result)
				return RET_FAILURE;
			apu_wait_for_ACK(packet.cookie, packet.payload_data); //replace with wait_response();
		}
	}
	return result;
}
