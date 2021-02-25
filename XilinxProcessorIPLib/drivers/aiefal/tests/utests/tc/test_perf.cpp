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
	std::shared_ptr<XAieDev> AiePtr;
	std::shared_ptr<XAiePerfCounter> PCounter;
	XAie_LocType Loc = XAie_TileLoc(1,1);
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

	AiePtr = std::make_shared<XAieDev>(&DevInst, true);
	PCounter = std::make_shared<XAiePerfCounter>(AiePtr, Loc);

	RC = PCounter->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->getCounterEvent(Mod, CounterE);
	CHECK_EQUAL(RC, XAIE_OK);

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
	std::shared_ptr<XAieDev> AiePtr;
	std::shared_ptr<XAiePerfCounter> PCounter0, PCounter1, PCounter2,
		PCounter3, PCounter4;
	XAie_LocType Loc = XAie_TileLoc(1,1);
	AieRC RC;
	XAie_ModuleType Mod;
	XAie_Events CounterE;
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

	AiePtr = std::make_shared<XAieDev>(&DevInst, true);
	PCounter0 = std::make_shared<XAiePerfCounter>(AiePtr, Loc);

	RC = PCounter0->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	// Test Cross Mod
	CrossMod = PCounter0->getCrossMod();
	CHECK_EQUAL(CrossMod, false);
	RC = PCounter0->setCrossMod(true);
	CHECK_EQUAL(RC, XAIE_OK);
	CrossMod = PCounter0->getCrossMod();
	CHECK_EQUAL(CrossMod, true);

	// Test setting Cross Mod after reservation
	RC = PCounter0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter0->setCrossMod(false);
	CHECK_EQUAL(RC, XAIE_ERR);
	RC = PCounter0->release();
	CHECK_EQUAL(RC, XAIE_OK);

	// Test Perf Counter reservation fail when cross mode
	// is Enabled.
	std::shared_ptr<XAiePerfCounter> PCounter;
	PCounter1 = std::make_shared<XAiePerfCounter>(AiePtr, Loc, XAIE_CORE_MOD);
	PCounter2 = std::make_shared<XAiePerfCounter>(AiePtr, Loc, XAIE_CORE_MOD);
	PCounter3 = std::make_shared<XAiePerfCounter>(AiePtr, Loc, XAIE_CORE_MOD);
	PCounter4 = std::make_shared<XAiePerfCounter>(AiePtr, Loc, XAIE_CORE_MOD);

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
	CHECK_FALSE(RC == XAIE_OK);

	// Enable cross mode to reserve will pass
	RC = PCounter4->setCrossMod(true);
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
