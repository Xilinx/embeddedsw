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

TEST(PerfCounter, PerfBasic)
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
	auto PCounter = Aie->tile(1,3).core().perfCounter();

	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeStartEvent(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeRstEvent(XAIE_CORE_MOD, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeStopEvent(XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeThreshold(1);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeStartEvent(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeRstEvent(XAIE_CORE_MOD, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeStopEvent(XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
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

	/* Test release without stop */
	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_ERR);
}

/* This test is similar to PerfCoreBasic with preferred ID */
TEST(PerfCounter, PerfCoreID)
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

	RC = PCounter->setPreferredId(2);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->getCounterEvent(Mod, CounterE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* testing start twice to rsc-base.hpp coverage */
	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->readResult(Result);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Test start without reservation */
	RC = PCounter->getCounterEvent(Mod, CounterE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->readResult(Result);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);

  /* Test release without stop */
	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* test reserve twice */
	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE, XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);
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
	auto PCounter0 = Aie.tile(1,3).core().perfCounter();

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

/* This tests negative cases for perf counter */
TEST(PerfCounter, PerfCoreNegative)
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

	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	/* Start without reservation will cause error */
	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Initializing if perf counter is reserved will cause error */
	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE,
			 XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->changeStartEvent(XAIE_MEM_MOD, XAIE_EVENT_LOCK_STALL_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);


	RC = PCounter->changeRstEvent(XAIE_MEM_MOD, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = PCounter->changeStopEvent(XAIE_MEM_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

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


	DevInst.IsReady = 0;

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* change start event if perf funnin will cause error */
	RC = PCounter->changeStartEvent(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->changeRstEvent(XAIE_CORE_MOD, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->changeStopEvent(XAIE_MEM_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->changeThreshold(0);
	CHECK_EQUAL(RC, XAIE_ERR);

	/* releasing without stopping event will cause error */
	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

/* This test reserves broadcasts so that perfCounter cannot be reserved. */
TEST(PerfCounter, PerfCoreBroadcastOverflow)
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
	auto PCounter = Aie->tile(1,1).mem().perfCounter();

	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_LOCK_STALL_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC0 = Aie->tile(1,1).broadcast();
	RC = BC0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC1 = Aie->tile(1,1).broadcast();
	RC = BC1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC2 = Aie->tile(1,1).broadcast();
	RC = BC2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC3 = Aie->tile(1,1).broadcast();
	RC = BC3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC4 = Aie->tile(1,1).broadcast();
	RC = BC4->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC5 = Aie->tile(1,1).broadcast();
	RC = BC5->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC6 = Aie->tile(1,1).broadcast();
	RC = BC6->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC7 = Aie->tile(1,1).broadcast();
	RC = BC7->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC8 = Aie->tile(1,1).broadcast();
	RC = BC8->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC9 = Aie->tile(1,1).broadcast();
	RC = BC9->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC10 = Aie->tile(1,1).broadcast();
	RC = BC10->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC11 = Aie->tile(1,1).broadcast();
	RC = BC11->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC12 = Aie->tile(1,1).broadcast();
	RC = BC12->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC13 = Aie->tile(1,1).broadcast();
	RC = BC13->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC14 = Aie->tile(1,1).broadcast();
	RC = BC14->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC15 = Aie->tile(1,1).broadcast();
	RC = BC15->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->changeRstEvent(XAIE_CORE_MOD, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = BC0->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = BC1->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = BC2->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC1->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC2->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC3->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC4->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC5->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC6->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC7->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC8->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC9->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC10->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC11->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC12->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC13->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC14->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC15->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
