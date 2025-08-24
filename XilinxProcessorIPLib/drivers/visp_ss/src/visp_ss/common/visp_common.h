// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <stdio.h>
#include "sleep.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "mbox_cmd.h"
#include "sensor_cmd.h"

//#include "sensor_cfgs.h"
#include "vlog.h"
//#include "vvdevice.h"

#define LOGTAG "VISP_COMMON"

int SetATM(void);

int ATM_ENABLE;
#define ATM_HIGH_MEM_PREFIX 0x8
