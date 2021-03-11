#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace std;
using namespace xaiefal;

TEST_GROUP(Broadcast)
{
};

TEST(Broadcast, Basic)
{
	AieRC RC;
	std::vector<XAie_LocType> vL, vL1;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(0,0));
	vL.push_back(XAie_TileLoc(1,0));
	vL.push_back(XAie_TileLoc(2,0));
	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;
	auto BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->getChannel(vL1, StartM1, EndM1);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(StartM, StartM);
	CHECK_EQUAL(EndM, EndM);
	CHECK_EQUAL(vL.size(), vL1.size());
	auto l0 = vL.begin();
	auto l1 = vL1.begin();
	bool is_equal = false;
	while (l0 != vL.end() && l1 != vL1.end()) {
		if ((*l0).Col == (*l1).Col && (*l0).Row == (*l1).Row) {
			is_equal = true;
		}
		l0++;
		l1++;
	}
	CHECK_TRUE(is_equal);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);


	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);
}
