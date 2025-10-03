// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <cam_device_app.h>
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	#include <xv_frmbufwr.h>
#endif
#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include "cam_device_common.h"


#define TXT_RED     "\x1b[31m"
#define TXT_GREEN   "\x1b[32m"
#define TXT_YELLOW  "\x1b[33m"
#define TXT_BLUE    "\x1b[34m"
#define TXT_MAGENTA "\x1b[35m"
#define TXT_CYAN    "\x1b[36m"
#define TXT_RST   "\x1b[0m"

// Function declarations
void reset_hw_function(void);
int SetupInterruptSystem(void);
int config_visp_ss(u32 baseaddress);
int init_visp_ss(void);
void enable_fbwr();
int init_fbwr(u8 hpId, CamDeviceBufChainId_t bufIo, CamDevicePipeOutFmt_t outFormat, u64 bufsize);
void setup_frmbuf_wr();

void FrmbufwrDoneCallback_0(void *CallbackRef);
void FrmbufwrDoneCallback_1(void *CallbackRef);
void FrmbufwrDoneCallback_2(void *CallbackRef);
void FrmbufwrDoneCallback_3(void *CallbackRef);

void Reset_IP(u8 Ip_ResetBit);
