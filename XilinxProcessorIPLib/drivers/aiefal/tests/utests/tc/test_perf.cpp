// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(PerfCounter)
{
};

TEST(PerfCounter, Basic)
{
	AieRC RC;
	XAie_ModuleType Mod;
	XAie_Events CounterE;
	uint32_t Result;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);
	auto PCounter = Aie->tile(1,1).core().perfCounter();

	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->getCounterEvent(Mod, CounterE);
	CHECK_EQUAL(RC, XAIE_OK);
	XAie_Events ExpectE;
	XAie_LocType PerfCounterLoc;
	XAie_ModuleType PerfCounterMod;
	uint32_t PerfCounterId;
	PCounter->getRscId(PerfCounterLoc, PerfCounterMod, PerfCounterId);
	XAie_PerfCounterGetEventBase(Aie->dev(), PerfCounterLoc,
			PerfCounterMod, &ExpectE);
	CHECK_EQUAL(static_cast<uint32_t>(CounterE),
			static_cast<uint32_t>(ExpectE) + PerfCounterId);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->readResult(Result);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);

	// Test start without reservation
	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_ERR);

	// Test release without stop
	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_ERR);
}

TEST(PerfCounter, CrossMod)
{
	AieRC RC;
	uint32_t Result;
	bool CrossMod;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto PCounter0 = Aie.tile(1,1).core().perfCounter();

	RC = PCounter0->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	// Test Cross Mod
	CrossMod = PCounter0->getCrossMod();
	CHECK_EQUAL(CrossMod, false);

	// Test Perf Counter reservation fail when cross mode
	// is Enabled.
	std::shared_ptr<XAiePerfCounter> PCounter;
	auto PCounter1 = Aie.tile(1,1).perfCounter();
	auto PCounter2 = Aie.tile(1,1).perfCounter();
	auto PCounter3 = Aie.tile(1,1).perfCounter();
	auto PCounter4 = Aie.tile(1,1).perfCounter();
	CrossMod = PCounter1->getCrossMod();
	CHECK_EQUAL(CrossMod, true);

	RC = PCounter1->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter2->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter3->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter4->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter4->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	// Start when cross mode is disabled
	RC = PCounter0->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter0->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter0->release();
	CHECK_EQUAL(RC, XAIE_OK);

	// Start when cross mode is enabled
	RC = PCounter4->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter4->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter4->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
