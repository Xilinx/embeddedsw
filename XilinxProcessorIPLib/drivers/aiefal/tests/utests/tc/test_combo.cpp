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

	std::vector<XAie_Events> vETwo;
	std::vector<XAie_EventComboOps> vOpsTwo;
	std::vector<XAie_Events> vEFour;
	std::vector<XAie_EventComboOps> vOpsFour;

	std::vector<XAie_Events> vEGet;
	std::vector<XAie_EventComboOps> vOpsGet;

	uint32_t RscType;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto ComboTwoEvents = Aie.tile(1,3).core().comboEvent();
	auto ComboTwoEvents2 = Aie.tile(1,3).core().comboEvent();
	auto ComboFourEvents = Aie.tile(1,4).core().comboEvent(4);
	auto ComboFourEventsFail = Aie.tile(1,3).core().comboEvent(4);

	RscType = ComboTwoEvents->getRscType();

	CHECK_THROWS(std::invalid_argument, Aie.tile(1,3).core().comboEvent(5));

	RC = ComboTwoEvents->getEvents(vEGet);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = ComboTwoEvents->getInputEvents(vEGet, vOpsGet);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = ComboTwoEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEventsFail->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboFourEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	vETwo.push_back(XAIE_EVENT_ACTIVE_CORE);
	vETwo.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	vOpsTwo.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	RC = ComboTwoEvents->setEvents(vETwo, vOpsTwo);
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboTwoEvents2->setEvents(vETwo, vOpsTwo);
	CHECK_EQUAL(RC, XAIE_OK);

	vEFour.push_back(XAIE_EVENT_ACTIVE_CORE);
	vEFour.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	vEFour.push_back(XAIE_EVENT_TRUE_CORE);
	vEFour.push_back(XAIE_EVENT_COMBO_EVENT_0_CORE);
	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_AND_NOTE2);
	RC = ComboFourEvents->setEvents(vEFour, vOpsFour);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->getEvents(vEGet);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEvents->getEvents(vEGet);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->getInputEvents(vEGet, vOpsGet);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboTwoEvents2->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboTwoEvents2->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboTwoEvents2->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/*Run PL test for coverage*/
	ComboTwoEvents = Aie.tile(1,0).pl().comboEvent();

	vETwo.clear();
	vETwo.push_back(XAIE_EVENT_TRUE_PL);
	vETwo.push_back(XAIE_EVENT_TIMER_VALUE_REACHED_PL);

	RC = ComboTwoEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->setEvents(vETwo, vOpsTwo);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->getEvents(vEGet);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

TEST(Combo, ComboFail)
{
	AieRC RC;

	std::vector<XAie_Events> vETwo;
	std::vector<XAie_EventComboOps> vOpsTwo;
	std::vector<XAie_Events> vEFour;
	std::vector<XAie_EventComboOps> vOpsFour;

	std::vector<XAie_Events> vEGet;
	std::vector<XAie_EventComboOps> vOpsGet;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto ComboTwoEvents = Aie.tile(1,3).core().comboEvent();
	auto ComboFourEvents = Aie.tile(1,4).core().comboEvent(4);
	auto ComboFourEventsFail = Aie.tile(1,3).core().comboEvent(4);

	CHECK_THROWS(std::invalid_argument, Aie.tile(1,3).core().comboEvent(5));

	RC = ComboTwoEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEventsFail->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboFourEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	DevInst.IsReady = 0;

	vETwo.push_back(XAIE_EVENT_ACTIVE_CORE);
	vETwo.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	vOpsTwo.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	RC = ComboTwoEvents->setEvents(vETwo, vOpsTwo);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = ComboTwoEvents->setEvents(vETwo, vOpsTwo);
	CHECK_EQUAL(RC, XAIE_OK);

	vEFour.push_back(XAIE_EVENT_ACTIVE_CORE);
	vEFour.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	vEFour.push_back(XAIE_EVENT_TRUE_CORE);
	vEFour.push_back(XAIE_EVENT_COMBO_EVENT_0_CORE);
	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_AND_NOTE2);
	RC = ComboFourEvents->setEvents(vEFour, vOpsFour);
	CHECK_EQUAL(RC, XAIE_OK);

	vOpsFour.push_back(XAIE_EVENT_COMBO_E1_AND_E2);

	RC = ComboFourEventsFail->setEvents(vEFour, vOpsFour);
	CHECK_EQUAL(RC, XAIE_OK);

	DevInst.IsReady = 0;
	RC = ComboTwoEvents->start();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;
	RC = ComboTwoEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEvents->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEventsFail->start();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = ComboTwoEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = ComboFourEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
}


/* setEvents function in ComboEvents has many branches. This test takes all those branches */
TEST(Combo, ComboSetEventsBranch)
{
	AieRC RC;

	std::vector<XAie_Events> vE1;
	std::vector<XAie_EventComboOps> vOps1;

	std::vector<XAie_Events> vE2;
	std::vector<XAie_EventComboOps> vOps2;

	std::vector<XAie_Events> vE3;
	std::vector<XAie_EventComboOps> vOps3;

	std::vector<XAie_Events> vE4;
	std::vector<XAie_EventComboOps> vOps4;


	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto ComboTwoEvents1 = Aie.tile(1,3).core().comboEvent();
	auto ComboTwoEvents2 = Aie.tile(1,4).core().comboEvent();
	auto ComboTwoEvents3 = Aie.tile(1,5).core().comboEvent();
	auto ComboFourEvents = Aie.tile(1,6).core().comboEvent(4);

	CHECK_THROWS(std::invalid_argument, Aie.tile(1,3).core().comboEvent(5));

	RC = ComboTwoEvents1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboTwoEvents3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = ComboFourEvents->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

  /* setEvents Condition: vE.size() != vEvents.size()) */

  vE1.push_back(XAIE_EVENT_ACTIVE_CORE);

	RC = ComboTwoEvents1->setEvents(vE1, vOps1);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboTwoEvents1->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* setEvents Condition: vOp.size() > 3 */

	vE2.push_back(XAIE_EVENT_ACTIVE_CORE);
	vE2.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);

	vOps2.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	vOps2.push_back(XAIE_EVENT_COMBO_E1_AND_NOTE2);
	vOps2.push_back(XAIE_EVENT_COMBO_E1_AND_E2);

	RC = ComboTwoEvents2->setEvents(vE2, vOps2);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboTwoEvents2->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* setEvents Condition: (vE.size() <= 2 && vOp.size() > 1) */

	vE3.push_back(XAIE_EVENT_ACTIVE_CORE);
	vE3.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);

	vOps3.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	vOps3.push_back(XAIE_EVENT_COMBO_E1_AND_NOTE2);
	vOps3.push_back(XAIE_EVENT_COMBO_E1_AND_E2);

	RC = ComboTwoEvents3->setEvents(vE3, vOps3);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboTwoEvents3->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* setEvents Condition: (vE.size() > 2 && vOp.size() < 2) */

	vE4.push_back(XAIE_EVENT_ACTIVE_CORE);
	vE4.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	vE4.push_back(XAIE_EVENT_TRUE_CORE);
	vE4.push_back(XAIE_EVENT_COMBO_EVENT_0_CORE);
	vOps4.push_back(XAIE_EVENT_COMBO_E1_OR_E2);

	RC = ComboFourEvents->setEvents(vE4, vOps4);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = ComboFourEvents->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
