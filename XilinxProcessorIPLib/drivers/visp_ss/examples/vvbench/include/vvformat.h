/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets       *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __VVFORMAT_H__
#define __VVFORMAT_H__


//#include "dom_ctrl_api.h"
#include <oslayer.h>
#include <return_codes.h>

#include "vvbase.h"
#include "vlog.h"
#include "buf_defs.h"
#include "cam_device_common.h"


RESULT ConvertRawToRGBA
(
	BufIdentity *pBuffer
);
#endif //__VVFORMAT_H__
