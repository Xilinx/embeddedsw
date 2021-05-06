// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(Tracing)
{
};

TEST(Tracing, Basic)
{
	AieRC RC;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto Tracing = Aie.tile(1,1).mem().traceControl();
	auto TraceE = Aie.tile(1,1).mem().traceEvent();

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	int TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	int TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Cross module test */
	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, 0);

	TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, 0);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EPERM);

	TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EPERM);

}
