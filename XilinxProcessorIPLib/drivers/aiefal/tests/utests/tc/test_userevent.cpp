// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(UserEvent)
{
};

TEST(UserEvent, Basic)
{
	AieRC RC;
	XAie_Events E;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);
	XAieDev Aie(&DevInst, true);

	auto UserEvent = Aie.tile(1,1).core().userEvent();
	RC = UserEvent->getEvent(E);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = UserEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->getEvent(E);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_0_CORE);
	RC = UserEvent->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->release();
	CHECK_EQUAL(RC, XAIE_OK);

	auto UserEvent0 = Aie.tile(1,1).core().userEvent();
	RC = UserEvent0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent0->getEvent(E);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_0_CORE);
	auto UserEvent1 = Aie.tile(1,1).core().userEvent();
	RC = UserEvent1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent1->getEvent(E);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_1_CORE);
	auto UserEvent2 = Aie.tile(1,1).core().userEvent();
	RC = UserEvent2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent2->getEvent(E);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_2_CORE);
	auto UserEvent3 = Aie.tile(1,1).core().userEvent();
	RC = UserEvent3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent3->getEvent(E);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_3_CORE);
	auto UserEvent4 = Aie.tile(1,1).core().userEvent();
	RC = UserEvent4->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	UserEvent = Aie.tile(1,1).mem().userEvent();
	RC = UserEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->getEvent(E);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_0_MEM);
	RC = UserEvent->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->release();
	CHECK_EQUAL(RC, XAIE_OK);

	UserEvent = Aie.tile(1,0).pl().userEvent();
	RC = UserEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->getEvent(E);
	CHECK_EQUAL(E, XAIE_EVENT_USER_EVENT_0_PL);
	RC = UserEvent->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = UserEvent->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
