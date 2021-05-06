// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(Combo)
{
};

TEST(Combo, ComboBasic)
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
	auto ComboTwoEvents = Aie.tile(1,1).core().comboEvent();
	auto ComboFourEvents = Aie.tile(1,2).core().comboEvent(4);
	auto ComboFourEventsFail = Aie.tile(1,1).core().comboEvent(4);

	RC = ComboTwoEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEventsFail->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboFourEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	std::vector<XAie_Events> vETwo;
	vETwo.push_back(XAIE_EVENT_ACTIVE_CORE);
	vETwo.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	std::vector<XAie_EventComboOps> vOpsTwo;
	vOpsTwo.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	RC = ComboTwoEvents->setEvents(vETwo, vOpsTwo);
	CHECK_EQUAL(RC, XAIE_OK);

	std::vector<XAie_Events> vEFour;
	vEFour.push_back(XAIE_EVENT_ACTIVE_CORE);
	vEFour.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	vEFour.push_back(XAIE_EVENT_TRUE_CORE);
	vEFour.push_back(XAIE_EVENT_COMBO_EVENT_0_CORE);
	std::vector<XAie_EventComboOps> vOpsFour;
	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_AND_NOTE2);
	RC = ComboFourEvents->setEvents(vEFour, vOpsFour);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
