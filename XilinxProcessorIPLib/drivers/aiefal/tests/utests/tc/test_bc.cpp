// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace std;
using namespace xaiefal;

static bool is_equal_vLocs(const std::vector<XAie_LocType> vL0,
			   const std::vector<XAie_LocType> vL1)
{
	bool is_equal = false;

	if (vL0.size() != vL1.size()) {
		return false;
	}

	auto l0 = vL0.begin();
	auto l1 = vL1.begin();
	while (l0 != vL0.end() && l1 != vL1.end()) {
		if ((*l0).Col == (*l1).Col && (*l0).Row == (*l1).Row) {
			is_equal = true;
		}
		l0++;
		l1++;
	}
	return is_equal;
}

TEST_GROUP(Broadcast)
{
};

TEST(Broadcast, BroadcstError)
{
	AieRC RC;
	std::vector<XAie_LocType> vL, vL1;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(1,4));
	vL.push_back(XAie_TileLoc(1,5));

	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;
	auto BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;
	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	DevInst.IsReady = 0;

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	DevInst.IsReady = 0;

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_ERR);

	DevInst.IsReady = 1;

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

	vL.clear();
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(1,3));

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	vL.clear();
	vL.push_back(XAie_TileLoc(999,999));

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	vL.clear();
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(1,5));

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	vL.clear();
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(3,3));

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	vL.clear();
	vL.push_back(XAie_TileLoc(1,3));


	auto BC1 = Aie.broadcast(vL, StartM, EndM);
	RC = BC1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC2 = Aie.broadcast(vL, StartM, EndM);
	RC = BC2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC3 = Aie.broadcast(vL, StartM, EndM);
	RC = BC3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC4 = Aie.broadcast(vL, StartM, EndM);
	RC = BC4->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	auto BC5 = Aie.broadcast(vL, StartM, EndM);
	RC = BC5->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC6 = Aie.broadcast(vL, StartM, EndM);
	RC = BC6->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC7 = Aie.broadcast(vL, StartM, EndM);
	RC = BC7->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC8 = Aie.broadcast(vL, StartM, EndM);
	RC = BC8->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC9 = Aie.broadcast(vL, StartM, EndM);
	RC = BC9->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC10 = Aie.broadcast(vL, StartM, EndM);
	RC = BC10->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC11 = Aie.broadcast(vL, StartM, EndM);
	RC = BC11->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC12 = Aie.broadcast(vL, StartM, EndM);
	RC = BC12->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	auto BC13 = Aie.broadcast(vL, StartM, EndM);
	RC = BC13->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC14 = Aie.broadcast(vL, StartM, EndM);
	RC = BC14->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC15 = Aie.broadcast(vL, StartM, EndM);
	RC = BC15->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC16 = Aie.broadcast(vL, StartM, EndM);
	RC = BC16->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

}

/* This test selects a list of tiles to broadcast to */
TEST(Broadcast, BroadcastSelectTiles)
{
	AieRC RC;
	std::vector<XAie_LocType> vL, vL1;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	/*First test SHIM tiles left to right*/
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
	CHECK_TRUE(is_equal_vLocs(vL, vL1));

	RC = BC->getEvent(XAie_TileLoc(1,0), XAIE_PL_MOD, event);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->getEvent(XAie_TileLoc(1,0), XAIE_PL_MOD, event);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->getEvent(XAie_TileLoc(1,1), XAIE_PL_MOD, event);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

	vL.clear();

	/*Test SHIM right to left shim*/
	vL.push_back(XAie_TileLoc(2,0));
	vL.push_back(XAie_TileLoc(1,0));
	vL.push_back(XAie_TileLoc(0,0));

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

	vL.clear();

	/*Test non shim left to right*/
	vL.push_back(XAie_TileLoc(0,3));
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(2,3));
	vL.push_back(XAie_TileLoc(3,3));
	vL.push_back(XAie_TileLoc(4,3));
	vL.push_back(XAie_TileLoc(5,3));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

	vL.clear();

	/*Test non shim right to left*/
	vL.push_back(XAie_TileLoc(5,3));
	vL.push_back(XAie_TileLoc(4,3));
	vL.push_back(XAie_TileLoc(3,3));
	vL.push_back(XAie_TileLoc(2,3));
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(0,3));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

	vL.clear();

	/*Test non shim up to down*/
	vL.push_back(XAie_TileLoc(1,6));
	vL.push_back(XAie_TileLoc(1,5));
	vL.push_back(XAie_TileLoc(1,4));
	vL.push_back(XAie_TileLoc(1,3));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

	vL.clear();

	/*Test shim to non-shim up to down*/
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(1,2));
	vL.push_back(XAie_TileLoc(1,1));
	vL.push_back(XAie_TileLoc(1,0));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_PL_MOD;

	BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastSingleTile)
{
	AieRC RC;
	std::vector<XAie_LocType> vL, vL1;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;

	auto BC0 = Aie.tile(1,3).broadcast();
	RC = BC0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	std::vector<XAie_LocType> vL0;
	vL0.push_back(XAie_TileLoc(1,3));
	RC = BC0->getChannel(vL1, StartM, EndM);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_TRUE(is_equal_vLocs(vL0, vL1));
	int BcId0 = BC0->getBc();

	auto BC1 = Aie.tile(1,3).broadcast();
	RC = BC1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC1->getEvent(XAie_TileLoc(1,3), XAIE_CORE_MOD, event);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC1->getChannel(vL1, StartM, EndM);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_TRUE(is_equal_vLocs(vL0, vL1));
	int BcId1 = BC1->getBc();
	CHECK_EQUAL(BcId0 + 1, BcId1);

	auto BC2 = Aie.tile(1,3).broadcast();
	RC = BC2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = BC2->getChannel(vL1, StartM, EndM);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_TRUE(is_equal_vLocs(vL0, vL1));
	int BcId2 = BC2->getBc();
	CHECK_EQUAL(BcId1 + 1, BcId2);

	auto BC3 = Aie.tile(1,3).broadcast();
	RC = BC3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = BC3->getChannel(vL1, StartM, EndM);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_TRUE(is_equal_vLocs(vL0, vL1));
	int BcId3 = BC3->getBc();
	CHECK_EQUAL(BcId2 + 1, BcId3);

	RC = BC3->release();
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC3_ = Aie.tile(1,3).broadcast();
	RC = BC3_->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = BC3_->getChannel(vL1, StartM, EndM);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_TRUE(is_equal_vLocs(vL0, vL1));
	int BcId3_ = BC3_->getBc();
	CHECK_EQUAL(BcId2 + 1, BcId3_);

	BC0->release();
	BC1->release();
	BC2->release();
	BC3_->release();

	/*Set preferred ID for bc*/
	auto BCPreferred = Aie.tile(1,3).broadcast();
	RC = BCPreferred->setPreferredId(0);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BCPreferred->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BCPreferred->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BCPreferred->start();
	CHECK_EQUAL(RC, XAIE_OK);


}

TEST(Broadcast, BroadcastAllTiles)
{
	AieRC RC;
	std::vector<XAie_LocType> vL, vL1;
	XAie_ModuleType StartM, EndM;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;
	auto BC = Aie.broadcast(vL, StartM, EndM);

	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}


#ifdef TEST_HARDWARE
TEST(Broadcast, BroadcastSingleTileShim)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,0));
	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,0), XAIE_PL_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	/* Make sure neighboring tiles are still zero */

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

        #else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastSingleTileCore)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,4));
	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_MEM_MOD,
			XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,4), XAIE_CORE_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_MEM_MOD,
			XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastHorizontalCore)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,4));
	vL.push_back(XAie_TileLoc(2,4));
	vL.push_back(XAie_TileLoc(3,4));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);


	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_MEM_MOD,
	        XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,4), XAIE_CORE_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastHorizontalCoreInverse)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(3,4));
	vL.push_back(XAie_TileLoc(2,4));
	vL.push_back(XAie_TileLoc(1,4));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_CORE_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(3,4), XAIE_CORE_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(3,4), XAIE_CORE_MOD,
		XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,4), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,5), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,3), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastHorizontalShim)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,0));
	vL.push_back(XAie_TileLoc(2,0));
	vL.push_back(XAie_TileLoc(3,0));

	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);


	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,0), XAIE_PL_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);


	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastHorizontalShimInverse)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(3,0));
	vL.push_back(XAie_TileLoc(2,0));
	vL.push_back(XAie_TileLoc(1,0));

	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);


	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(3,0), XAIE_PL_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

        #else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastVertical)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,0));
	vL.push_back(XAie_TileLoc(1,1));
	vL.push_back(XAie_TileLoc(1,2));
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(1,4));

	StartM = XAIE_PL_MOD;
	EndM = XAIE_CORE_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,0), XAIE_PL_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_USER_EVENT_0_PL);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);


	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastVerticalInverse)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,4));
	vL.push_back(XAie_TileLoc(1,3));
	vL.push_back(XAie_TileLoc(1,2));
	vL.push_back(XAie_TileLoc(1,1));
	vL.push_back(XAie_TileLoc(1,0));

	StartM = XAIE_CORE_MOD;
	EndM = XAIE_PL_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#if AIE_GEN == 1

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#else

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	#endif

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,4), XAIE_CORE_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,4), XAIE_CORE_MOD,
		XAIE_EVENT_USER_EVENT_0_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_CORE_MOD,
		XAIE_EVENT_BROADCAST_0_CORE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);


	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}


#if AIE_GEN != 1
TEST(Broadcast, BroadcastSingleMemTile)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,1));
	StartM = XAIE_MEM_MOD;
	EndM = XAIE_MEM_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,1), XAIE_MEM_MOD,
		BC->getBc(), XAIE_EVENT_USER_EVENT_0_MEM_TILE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_USER_EVENT_0_MEM_TILE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastHorizontalMemTile)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(1,1));
	vL.push_back(XAie_TileLoc(2,1));
	vL.push_back(XAie_TileLoc(3,1));

	StartM = XAIE_MEM_MOD;
	EndM = XAIE_MEM_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(1,1), XAIE_MEM_MOD,
	 BC->getBc(), XAIE_EVENT_USER_EVENT_0_MEM_TILE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_USER_EVENT_0_MEM_TILE);
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}

TEST(Broadcast, BroadcastHorizontalMemTileInverse)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	XAie_ModuleType StartM, EndM, StartM1, EndM1;

	XAie_Events event;
	u8 status;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_PmRequestTiles(&DevInst, NULL, 0);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	vL.push_back(XAie_TileLoc(3,1));
	vL.push_back(XAie_TileLoc(2,1));
	vL.push_back(XAie_TileLoc(1,1));

	StartM = XAIE_MEM_MOD;
	EndM = XAIE_MEM_MOD;

	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->start();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventBroadcast(&DevInst,  XAie_TileLoc(3,1), XAIE_MEM_MOD,
	 BC->getBc(), XAIE_EVENT_USER_EVENT_0_MEM_TILE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = XAie_EventGenerate(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_USER_EVENT_0_MEM_TILE);
	CHECK_EQUAL(RC, XAIE_OK);

	/* Check that event status is 0 before broadcast */
	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 1);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,1), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,0), XAIE_PL_MOD,
		XAIE_EVENT_BROADCAST_A_0_PL, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(1,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(2,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(3,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(0,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = XAie_EventReadStatus(&DevInst, XAie_TileLoc(4,2), XAIE_MEM_MOD,
		XAIE_EVENT_BROADCAST_0_MEM_TILE, &status);
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(status, 0);

	RC = BC->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = BC->release();
	CHECK_EQUAL(RC, XAIE_OK);

}


#endif
#endif
