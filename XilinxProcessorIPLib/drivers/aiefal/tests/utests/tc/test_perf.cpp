#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

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
