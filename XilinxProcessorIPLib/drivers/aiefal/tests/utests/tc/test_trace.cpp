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
	std::shared_ptr<XAieDev> AiePtr;
	std::shared_ptr<XAieTracing> Tracing;
	XAie_LocType Loc = XAie_TileLoc(1,1);
	AieRC RC;
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
	Tracing = std::make_shared<XAieTracing>(AiePtr, Loc, XAIE_CORE_MOD);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_ERR);
}
