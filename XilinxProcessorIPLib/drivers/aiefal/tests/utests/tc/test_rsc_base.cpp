#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

class XAieRscTester: public XAieRsc {
public:
  XAieRscTester() = delete;
  XAieRscTester(std::shared_ptr<XAieDevHandle> DevHd):
    XAieRsc(DevHd) {
      State.Initialized = 1;
    }

};

class XAieSingleTileTester: public XAieSingleTileRsc {
public:
	XAieSingleTileTester() = delete;
	XAieSingleTileTester(std::shared_ptr<XAieDevHandle> DevHd, XAie_LocType Loc):
		XAieSingleTileRsc(DevHd, Loc) {
			State.Initialized = 1;
		}

};

TEST_GROUP(XAieRsc)
{
};

TEST(XAieRsc, XAieRscBase)
{
	AieRC RC;

	bool isChecker;

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

	CHECK_THROWS(std::invalid_argument, XAieRsc(NULL));

	RC = PCounter->setPreferredId(XAIE_RSC_ID_ANY, true);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->setPreferredId(XAIE_RSC_ID_ANY, true);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = PCounter->free();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 0;

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;
	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	isChecker = PCounter->isInitialized();
	CHECK_EQUAL(isChecker, 1);

	isChecker = PCounter->isReserved();
	CHECK_EQUAL(isChecker, 1);

	isChecker = PCounter->isConfigured();
	CHECK_EQUAL(isChecker, 1);

	isChecker = PCounter->isRunning();
	CHECK_EQUAL(isChecker, 1);

	RC = PCounter->release();
	CHECK_EQUAL(RC, XAIE_ERR);

}

TEST(XAieRsc, XAieRscPrereserved){
	AieRC RC;

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

	RC = PCounter->setPreferredId(1, true);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = PCounter->free();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

}

TEST(XAieRsc, XAieRscBaseError)
{
	AieRC RC;

	bool isChecker;
	XAieRscStat StatChecker;
	std::string RscString;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);

	CHECK_THROWS(std::invalid_argument, XAieRsc(NULL));
	XAieRscTester Rsc = XAieRscTester(Aie->getDevHandle());

	RC = Rsc.reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Rsc.reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Rsc.start();
	CHECK_EQUAL(RC, XAIE_ERR);

	CHECK_THROWS(std::invalid_argument, Rsc.getRscType());
	CHECK_THROWS(std::invalid_argument, Rsc.getRscStat(RscString));
	CHECK_THROWS(std::invalid_argument, Rsc.getAvailManagedRscs());
	CHECK_THROWS(std::invalid_argument, Rsc.getManagedRscsType());

}

TEST(XAieRsc, XAieRscSingleTile)
{
	AieRC RC;

	bool isChecker;
	XAieRscStat StatChecker;
	std::string RscString;
	XAie_LocType Loc;
	XAie_ModuleType Mod;
	uint32_t id;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);

	CHECK_THROWS(std::invalid_argument, XAieSingleTileRsc(Aie->getDevHandle(), XAie_TileLoc(999,3)));
	XAieSingleTileRsc RscCore = XAieSingleTileTester(Aie->getDevHandle(), XAie_TileLoc(1,3));
	XAieSingleTileRsc RscShim = XAieSingleTileTester(Aie->getDevHandle(), XAie_TileLoc(1,0));

	RC = RscCore.getRscId(Loc, Mod, id);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = RscCore.setPreferredId(1);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = RscCore.reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = RscCore.getRscId(Loc, Mod, id);
	CHECK_EQUAL(RC, XAIE_OK);

	CHECK_THROWS(std::invalid_argument, RscCore.getRscStat(RscString));

}

#if AIE_GEN != 1

TEST(XAieRsc, XAieRscSingleMemTile)
{
	AieRC RC;

	bool isChecker;
	XAieRscStat StatChecker;
	std::string RscString;
	XAie_LocType Loc;
	XAie_ModuleType Mod;
	uint32_t id;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	auto Aie = std::make_shared<XAieDev>(&DevInst, true);

	XAieSingleTileRsc RscCore = XAieSingleTileTester(Aie->getDevHandle(), XAie_TileLoc(1,1));

	RC = RscCore.getRscId(Loc, Mod, id);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = RscCore.reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = RscCore.getRscId(Loc, Mod, id);
	CHECK_EQUAL(RC, XAIE_OK);

}

#endif
