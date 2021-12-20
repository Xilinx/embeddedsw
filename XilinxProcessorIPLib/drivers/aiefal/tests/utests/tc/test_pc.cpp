// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace std;
using namespace xaiefal;

extern "C" {
	AieRC XAie_RequestPCEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
}

TEST_GROUP(PC)
{
};

TEST(PC, PCEvent) {
	AieRC RC;
	uint32_t pcAddr;
	uint32_t pcEventRscType;
	XAie_Events pcCurrEvent;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);
	auto pcEvent = Aie->tile(1,3).core().pcEvent();

	RC = pcEvent->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = pcEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->updatePcAddr(0x4);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->updatePcAddr(0x4);
	CHECK_EQUAL(RC, XAIE_OK);

	pcEventRscType = pcEvent->getRscType();

	pcAddr = pcEvent->getPcAddr();
	CHECK_EQUAL(pcAddr, 0x4);

	RC = pcEvent->updatePcAddr(0x4);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

TEST(PC, PCEventID) {
	AieRC RC;
	uint32_t pcAddr;
	uint32_t pcEventRscType;
	XAie_Events pcCurrEvent;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);
	auto pcEvent = Aie->tile(1,3).core().pcEvent();

	RC = pcEvent->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = pcEvent->setPreferredId(3);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->updatePcAddr(0x4);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->start();
	CHECK_EQUAL(RC, XAIE_OK);

	pcEventRscType = pcEvent->getRscType();

	pcAddr = pcEvent->getPcAddr();
	CHECK_EQUAL(pcAddr, 0x4);

	RC = pcEvent->updatePcAddr(0x4);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

TEST(PC, PCEventExtra)
{
	AieRC RC;
	uint32_t pcAddr;
	uint32_t pcEventRscType;
	XAie_Events pcCurrEvent;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);

	CHECK_THROWS(std::invalid_argument, Aie->tile(1,0).pl().pcEvent());

	auto pcEvent = Aie->tile(1,3).core().pcEvent();
	auto pcEvent2 = Aie->tile(1,3).core().pcEvent();
	auto pcEvent3 = Aie->tile(1,3).core().pcEvent();
	auto pcEvent4 = Aie->tile(1,3).core().pcEvent();
	auto pcEventFail = Aie->tile(1,3).core().pcEvent();

	DevInst.IsReady = 0;

	RC = pcEvent->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = pcEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEvent4->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcEventFail->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = pcEvent->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = pcEvent2->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = pcEvent3->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = pcEvent4->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

TEST(PC, PCRange) {
	AieRC RC;
	uint32_t PcAddr0, PcAddr1;
	uint32_t pcRscType;
	XAie_Events pcCurrEvent;

	std::vector<XAie_UserRsc> pcRscs;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);
	auto pcRange = Aie->tile(1,1).core().pcRange();
	auto pcRange2 = Aie->tile(1,1).core().pcRange();

	RC = pcRange->updatePcAddr(0x4, 0x8);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcRange2->updatePcAddr(0xC, 0x10);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcRange->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = pcRange->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcRange2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcRange->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(pcCurrEvent, XAIE_EVENT_PC_RANGE_0_1_CORE);

	RC = pcRange2->getEvent(pcCurrEvent);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(pcCurrEvent, XAIE_EVENT_PC_RANGE_2_3_CORE);

	DevInst.IsReady = 0;

	RC = pcRange->start();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = pcRange->start();
	CHECK_EQUAL(RC, XAIE_OK);

	pcRscType = pcRange->getRscType();

	pcRange->getPcAddr(PcAddr0, PcAddr1);
	CHECK_EQUAL(PcAddr0, 0x4);
	CHECK_EQUAL(PcAddr1, 0x8);

	RC = pcRange->updatePcAddr(0x4, 0x8);
	CHECK_EQUAL(RC, XAIE_OK);

	DevInst.IsReady = 0;

	RC = pcRange->stop();
	CHECK_EQUAL(RC, XAIE_ERR);

	DevInst.IsReady = 1;

	RC = pcRange->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcRange->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = pcRange2->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
