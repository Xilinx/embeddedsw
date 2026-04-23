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


#include "vvbench.h"
#include "vvbase.h"
#include <stdio.h>
/* memory_manager functions provided by libvisp.a */
extern int mm_init(void);
#include <signal.h>
#include <stdlib.h>
#include <builtins.h>
#include "cJSON.h"
/* vvbench_json_loader provided by libvisp.a */
#include "xvisp_ss.h"
#include "cam_device_api.h"
extern int ATM_ENABLE;
#define LOGTAG "VVBENCH"
//#include <unistd.h>
#ifdef PORTING_25
	//#include "hal_api.h"
#endif
// #include "mbox_cmd.h"
#if SD
	#include "xsdps.h"		/* SD device driver */
#endif
#include "ff.h"
#ifndef APU_CORE
	#include "xil_mpu.h"
#endif
bool isMounted = false;

#define BLOCK_SIZE_2MB 0x200000U

extern int custom_json;

int vvbenchLogLevel = VVLOG_LEVEL_VERBOSE;//-1;
#define ISP_VERSION "ISP8200_V2204"

extern int VsiVvbenchExecuteList
(
	const char *file,
	VvbenchCfg_t *cfgCtx
);

extern int VsiVvbenchExecuteClose
(
);

#if 0
char *VsiVvbenchLoadFileContent
(
	const char *fileName
)
{
	FIL jsonFile;
	FRESULT Res;
	u32 NumBytesRead;
	Res = f_open(&jsonFile, fileName, FA_WRITE | FA_READ);
	if (Res) {
		LOGE("read failed\n");
		return NULL;
	}
	f_lseek(&jsonFile, 0);
	char readBuf[256] = {0};
	NumBytesRead = 1;
	u32 len = 0;
	while (NumBytesRead) {
		Res = f_read(&jsonFile, (void*)readBuf, 256,
			     &NumBytesRead);
		if (Res) {
			return XST_FAILURE;
		}
		len += NumBytesRead;
		Res = f_lseek(&jsonFile, len);
	}

	f_lseek(&jsonFile, 0);
	char *content = mm_malloc(len + 1);
	Res = f_read(&jsonFile, (void*)content, len, &NumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}

	f_close(&jsonFile);

	return content;
}
#endif


void VsiVvbenchShutDown
(
	int signum
)
{
	int result = 0;
	static int count = 0;
	LOGI("%s enter \n", __func__);

	count++;
	if (count == 1) {
		result = VsiVvbenchExecuteClose();
		if (0 != result) {
			LOGE("VsiVvbenchExecuteClose error");
		}
		LOGI("%s exit \n", __func__);
		exit(0);
	}
}

int get_argc(int argc, int min, int max)
{
	FIL pFile;
	FRESULT Res;
	char readBuf[256] = {0};
	int32_t NumBytesRead = 0;
	bool user_input = false;
	int new_argc = 0;

	xil_printf("Priority of argc.txt file input precedes user input\n");
	Res = f_open(&pFile, "argc.txt", FA_READ);
	if (Res != FR_OK) {
		xil_printf("argc.txt file not present, provide user input\n");
		user_input = true;
	}
	else {
		Res = f_read(&pFile, (void*)readBuf, 8,
			     &NumBytesRead); //argc digits in the file are not greater than two digits
		if (Res != FR_OK) {
			xil_printf("argc.txt file not present, provide user input\n");
			user_input = true;
		}
		else {
			new_argc = atoi(readBuf);
			if (new_argc < min || new_argc > max) {
				xil_printf("argc.txt input is not in available case list range '%d' and '%d', provide user input\n",
					   min, max);
				user_input = true;
			}
			else {
				argc = new_argc;
				xil_printf("argc.txt is present in SD Card, case selected=%d\n", argc);
				user_input = false;
			}
		}
		Res = f_close(&pFile);
		if (Res != FR_OK) {
			xil_printf("argc.txt file not present, provide user input\n");
			user_input = true;
		}
	}

	if (user_input == true) {
		xil_printf("case 0: vvbcfg/vvbench_sensor_single_ox08b40_4k_1080P_sensor_case_list.json\n");
		xil_printf("case 1: vvbcfg/vvbench_sensor_single_ox08b40_4k_1080P_2a_sensor_case_list.json\n");
		xil_printf("case 2: vvbcfg/vvbench_sensor_single_ox05b1s_rgbir_1080p_sensor_case_list.json\n");
		xil_printf("case 3: vvbcfg/vvbench_sensor_single_ox05b1s_rgbir_1080p_sensor_2a_case_list.json\n");
		xil_printf("case 4: vvbcfg/vvbench_sensor_single_ox05b1s_rgbir_1080p_sensor_ir_sp1_out_case_list.json\n");
		xil_printf("case 5: vvbcfg/vvbench_sensor_single_ox03f10_sensor_case_list.json\n");
		xil_printf("case 6: vvbcfg/vvbench_sensor_single_ox03f10_sensor_fusa_case_list.json\n");
		xil_printf("case 7: vvbcfg/vvbench_sensor_single_ox03f10_sensor_sp1_case_list.json\n");
		xil_printf("case 8: vvbcfg/vvbench_sensor_single_ox03f10_sensor_auto_mode_case_list.json\n");
		xil_printf("case 9: vvbcfg/vvbench_sensor_single_ox03f10_sensor_2a_mode_case_list.json\n");
		xil_printf("case 10: vvbcfg/vvbench_sensor_dual_ox03f10_sensor_dual_core_case_list.json\n");
		xil_printf("case 11: vvbcfg/vvbench_sensor_mcm_dual_ox03f10_sensor_case_list.json\n");
		xil_printf("case 12: vvbcfg/vvbench_sensor_mcm_four_ox03f10_sensor_single_isp_case_list.json\n");
		xil_printf("case 13: vvbcfg/vvbench_sensor_mcm_four_ox03f10_sensor_dual_isp_case_list.json\n");
		xil_printf("case 14: vvbcfg/vvbench_rdma_sw_simulator_list.json\n");
		xil_printf("case 15: vvbcfg/vvbench_rdma_mcm_rdma_case_list.json\n");
		xil_printf("case 16: vvbcfg/vvbench_rdma_format_case_list.json\n");
		xil_printf("case 17: vvbcfg/vvbench_additional_case_list.json\n");
		xil_printf("case 18: vvbcfg/vvbench_dual_rpu0_rpu1_core_dual_isp_AMP_test_case_list.json\n");
		xil_printf("case 19: vvbcfg/vvbench_sensor_single_ox03f10_sensor_LILO_case_list.json\n");
		xil_printf("case 20: vvbcfg/vvbench_sensor_mcm_dual_ox05b1s_ABmode_1080p_sensor_case_list.json\n");
		xil_printf("case 21: vvbcfg/vvbench_sensor_mcm_dual_ox05b1s_ABmode_2A_1080p_sensor_case_list.json\n");
		xil_printf("case 22: vvbcfg/vvbench_sensor_single_ox03f10_720p_sensor_case_list.json\n");
		xil_printf("case 23: vvbcfg/vvbench_sensor_single_ox03f10_vga_sensor_case_list.json\n");
		xil_printf("case 24: vvbcfg/vvbench_sensor_hw_mcm_four_ox03f10_sensor_single_isp_case_list.json\n");
		xil_printf("case 25: vvbcfg/vvbench_sensor_single_ox03f10_virtual_sensor_2a_mode_case_list.json\n");
		xil_printf("case 26: vvbcfg/vvbench_sensor_mcm_four_ox03f10_virtual_sensor_single_isp_case_list.json\n");
		xil_printf("Enter case number and press 'Enter' or press 'Enter' to select default case=%d\n\r",
			   argc);

		u8 Response;
		new_argc = 0;
		int tmp = -1, rd = false;
		do {
			Response = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);
			if ((Response >= 0x30) && (Response <= 0x39)) {
				tmp = Response - 0x30;
				new_argc *= 10;
				new_argc += tmp;
				rd = true;
				xil_printf("%d", tmp);
			}
			else if ((Response == 0xA) || (Response == 0xD)) {
				if (rd == true) {
					if ((new_argc >= min) && (new_argc <= max)) {
						argc = new_argc;
					}
					else {
						xil_printf("Enter valid case number or press 'Enter' to select default case=%d\n", argc);
						rd = false;
						new_argc = 0;
						Response = 0;
						continue;
					}
				}
				else if (rd == false) {
					xil_printf("'Enter' key input received, running default case=%d\n", argc);
				}
			}
			else {
				xil_printf("Enter numerical input to select case\n");
			}

		} while ((Response != 0xA) && (Response != 0xD));
	}

	return argc;
}

int main_vvbench(XVisp_Ss *InstancePtr)
{
	int result = 0;
	LOGI("@@vvbench started@@");
	VsiVvbenchVersion();

	VvbenchCtx_t vvctx;
	VvbenchCfg_t vvcfg;
	MEMSET(vvcfg.swSimuName, 0, sizeof(vvcfg.swSimuName));

	vvctx.cfgJsonFile = "\0";
	vvctx.caseJsonFile = "vvbcfg/vvbench_list.json";
	vvctx.swSimuName = "vvb_000_sw_simulator_rdma_single_frame_dom.json";

	FATFS fatfs_test;
	TCHAR *Path = "/";
	if (!isMounted) {
		f_mount(&fatfs_test, Path, 0);
		isMounted = true;
	}

#ifndef APU_CORE
	int num_region = HAL_RESERVED_MEM_SIZE / BLOCK_SIZE_2MB;

	for (int i = 0; i < (num_region); i++) {
		Xil_SetTlbAttributes(HAL_RESERVED_MEM_START + i * BLOCK_SIZE_2MB,
				     STRONG_ORDERD_SHARED | PRIV_RW_USER_RW);
	}
#endif
	LOGI("USE default configure file: %s\n", vvctx.caseJsonFile);
	LOGI("IOMODE %s\n", InstancePtr->Config.IoMode);
#ifndef JSON_CHANGE
	if (strcmp(InstancePtr->Config.IoMode, "mimo") == 0) {
		ATM_ENABLE = 1;
		if (custom_json == 1)
			vvctx.caseJsonFile = "MIMO_L1_JSON.json";
		else
			vvctx.caseJsonFile = "MIMO_1920x1080_RAW12_MODE1_OUTPUT_RGB888P.json";
		LOGI("USE default configure file: %s", vvctx.caseJsonFile);
		LOGI("Parse configuration settings...");
		result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
		if (0 != result) {
			LOGE("Parser config file error, exit");
			exit(1);
		}
	} else if (strcmp(InstancePtr->Config.IoMode, "limo") == 0) {
		ATM_ENABLE = 1;
		if (custom_json == 1)
			vvctx.caseJsonFile = "LIMO_L1_JSON.json";
		else
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_auto_mode_tuning_case_list.json";
		LOGI("USE default configure file: %s", vvctx.caseJsonFile);
		LOGI("Parse configuration settings...");
		result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
		if (0 != result) {
			LOGE("Parser config file error, exit");
			exit(1);
		}
	} else if (strcmp(InstancePtr->Config.IoMode, "lilo") == 0) {
		if (custom_json == 1)
			vvctx.caseJsonFile = "LILO_L1_JSON.json";
		else
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_auto_mode_tuning_case_lilo_list.json";
		LOGI("USE default configure file: %s", vvctx.caseJsonFile);
		LOGI("Parse configuration settings...");
		result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
		if (0 != result) {
			LOGE("Parser config file error, exit");
			exit(1);
		}
	}
#else
	vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_LILO_case_list.json";
	LOGI("USE default configure file: %s", vvctx.caseJsonFile);
	LOGI("Parse configuration settings...");
	result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
	if (0 != result) {
		LOGE("Parser config file error, exit");
		exit(1);
	}
#endif

#if 0
	int argc = 15, min = 0, max = 26;
	argc = get_argc(argc, min, max);
	xil_printf("argc:%d\n", argc);

	switch (argc) {
		case 0:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox08b40_4k_1080P_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 1:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox08b40_4k_1080P_2a_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 2:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox05b1s_rgbir_1080p_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 3:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox05b1s_rgbir_1080p_sensor_2a_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 4:
			vvctx.caseJsonFile =
				"vvbcfg/vvbench_sensor_single_ox05b1s_rgbir_1080p_sensor_ir_sp1_out_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 5:
			vvctx.caseJsonFile = "vvbench_sensor_single_ox03f10_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 6:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_fusa_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 7:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_sp1_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 8:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_auto_mode_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 9:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_2a_mode_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 10:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_dual_ox03f10_sensor_dual_core_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 11:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_mcm_dual_ox03f10_sensor_case_list.json";
			LOGI("USE default case list file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("execute list error, exit");
				exit(1);
			}
			break;
		case 12:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_mcm_four_ox03f10_sensor_single_isp_case_list.json";
			LOGI("USE default case list file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("execute list error, exit");
				exit(1);
			}
			break;
		case 13:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_mcm_four_ox03f10_sensor_dual_isp_case_list.json";
			LOGI("USE default case list file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("execute list error, exit");
				exit(1);
			}
			break;
		case 14:
			strncpy(vvcfg.swSimuName, vvctx.swSimuName, strlen(vvctx.swSimuName));
			vvctx.cfgJsonFile = "vvbcfg/vvbench_rdma_sw_simulator_list.json";
			LOGI("USE default configure file: %s", vvctx.cfgJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchParseConfig(vvctx.cfgJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 15:
			vvctx.caseJsonFile = "vvbcfg/vvbench_rdma_mcm_rdma_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 16:
			vvctx.caseJsonFile = "vvbcfg/vvbench_rdma_format_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 17:
			vvctx.caseJsonFile = "vvbcfg/vvbench_additional_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 18:
			vvctx.caseJsonFile = "vvbcfg/vvbench_dual_rpu0_rpu1_core_dual_isp_AMP_test_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 19:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_sensor_LILO_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 20:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_mcm_dual_ox05b1s_ABmode_1080p_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 21:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_mcm_dual_ox05b1s_ABmode_2A_1080p_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 22:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_720p_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 23:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_vga_sensor_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 24:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_hw_mcm_four_ox03f10_sensor_single_isp_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 25:
			vvctx.caseJsonFile = "vvbcfg/vvbench_sensor_single_ox03f10_virtual_sensor_2a_mode_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		case 26:
			vvctx.caseJsonFile =
				"vvbcfg/vvbench_sensor_mcm_four_ox03f10_virtual_sensor_single_isp_case_list.json";
			LOGI("USE default configure file: %s", vvctx.caseJsonFile);
			LOGI("Parse configuration settings...");
			result = VsiVvbenchExecuteList(vvctx.caseJsonFile, &vvcfg);
			if (0 != result) {
				LOGE("Parser config file error, exit");
				exit(1);
			}
			break;
		default:
			LOGE("error argc parameter, exit");
			return -1;
	}
#endif
	LOGI("@@vvbench end@@");
	return 0;
}

void usage()
{
	LOGI(" vvbench command:");
	LOGI("    vvbench: execute vvbench by default config files");
	LOGI("    vvbench new_config_file.json");
	LOGI("    vvbench new_config_file.json case_name.json");
}

void VsiVvbenchVersion()
{
	LOGI("--------------------------------------");
	LOGI("ISP Pipeline CFG: %s", ISP_VERSION);
	LOGI("VVBench Version:%s", VVBENCH_VERSION);
	// LOGI("Build time: %s, %s", __DATE__, __TIME__);
	LOGI("--------------------------------------");
}

int loadBinApp()
{
	int result = 0;
	LOGI("@@loadBinApp started@@");

	if (selectDestinationCore(0) != 0) {
		LOGE("selectDestinationCore failed");
		return -1;
	}

	result = SendLoadBinStart();
	if (result != 0){
		LOGE("SendLoadBinStart failed.");
		return result;
	}

	LOGI("@@loadBinApp end@@");
	return 0;
}

#ifndef APU_CORE
void main_(void)
{
	osThread pThread;
	pThread.usStackDepth = 1024 * 12000;
	osThreadCreate(&pThread, main_vvbench, &pThread);

	vTaskStartScheduler();
	while (1) {}
}
#endif
