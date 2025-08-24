/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 *   @file dct_assert.c
 *
 *   This file defines the implementation for the assertion facility of the
 *   embedded lib.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "dct_assert.h"


#if defined(ENABLE_ASSERT) || !defined(NDEBUG)

ASSERT_HANDLER assert_handler = 0;

void exit_(const char* file, int line)
{
	(void) fflush(stdout);
	(void) fflush(stderr);
	fprintf(stdout, "\n*** ASSERT: In File %s, line %d ***\n", file, line);

	if (assert_handler != 0) {
		/* If a handler is registered call it. */

		assert_handler();
	} else
		exit(0);
}
#endif
