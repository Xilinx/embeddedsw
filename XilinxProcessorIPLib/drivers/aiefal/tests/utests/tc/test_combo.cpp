#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(Combo)
{
};

TEST(Combo, Basic)
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
	auto Combo = Aie.tile(1,1).core().comboEvent();

	RC = Combo->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	std::vector<XAie_Events> vE;
	vE.push_back(XAIE_EVENT_ACTIVE_CORE);
	vE.push_back(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	std::vector<XAie_EventComboOps> vOps;
	vOps.push_back(XAIE_EVENT_COMBO_E1_OR_E2);
	RC = Combo->setEvents(vE, vOps);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Combo->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = Combo->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = Combo->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
