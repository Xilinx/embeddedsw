// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace std;
using namespace xaiefal;

TEST_GROUP(RSC)
{
};

TEST(RSC, RSCBasic) {
	AieRC RC;
	std::vector<XAie_LocType> vL;
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

	/*First test SHIM tiles left to right*/
	vL.push_back(XAie_TileLoc(0,0));
	vL.push_back(XAie_TileLoc(1,0));
	vL.push_back(XAie_TileLoc(2,0));
	StartM = XAIE_PL_MOD;
	EndM = XAIE_PL_MOD;
	auto BC = Aie.broadcast(vL, StartM, EndM);
	RC = BC->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto PCounter = Aie.tile(1,1).core().stallCycles();
	RC = PCounter->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto PCounter1 = Aie.tile(1,2).core().perfCounter();
	RC = PCounter1->initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
		   XAIE_MEM_MOD, XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM);
	CHECK_EQUAL(RC, XAIE_OK);
	RC = PCounter1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto PCRange = Aie.tile(1,1).core().pcRange();
	RC = PCRange->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto PCEvent = Aie.tile(1,1).core().pcEvent();
	RC = PCEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto TraceCntr0 = Aie.tile(1,1).core().traceControl();
	TraceCntr0->setCntrEvent(XAIE_EVENT_ACTIVE_CORE, XAIE_EVENT_DISABLED_CORE);
	RC = TraceCntr0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	auto TraceEvent0 = Aie.tile(1,1).core().traceEvent();
	RC = TraceEvent0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto TraceCntr1 = Aie.tile(1,1).mem().traceControl();
	TraceCntr1->setCntrEvent(XAIE_EVENT_ACTIVE_CORE, XAIE_EVENT_DISABLED_CORE);
	RC = TraceCntr1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	auto TraceEvent1 = Aie.tile(1,1).mem().traceEvent();
	TraceEvent1->setEvent(XAIE_CORE_MOD, XAIE_EVENT_MEMORY_STALL_CORE);
	RC = TraceEvent1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto TraceEvent2 = Aie.tile(1,1).core().traceEvent();
	RC = TraceEvent2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto UserEvent = Aie.tile(1,1).core().userEvent();
	RC = UserEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto SPort = Aie.tile(1,1).sswitchPort();
	RC = SPort->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto ComboEvent= Aie.tile(1,1).core().comboEvent();
	RC = ComboEvent->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	auto RscStatAll = Aie.getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	RscStatAll.show();
	uint32_t NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(1,2), XAIE_CORE_MOD, XAIE_PERFCNT_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(1,2), XAIE_MEM_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);

	auto RscStatTile = Aie.tile(1,1).getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	RscStatTile.show();

	auto RscStatMod = Aie.tile(1,1).core().getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	RscStatMod.show();

	bool hasRsc = Aie.tile(1,1).core().perfCounter()->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC).hasRsc();
	CHECK_EQUAL(hasRsc, true);

	auto PerfStat = Aie.tile(1,3).core().perfCounter()->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	hasRsc = PerfStat.hasRsc();
	CHECK_EQUAL(hasRsc, false);

	auto RscStatTrace = Aie.tile(1,1).core().traceEvent()->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	hasRsc = RscStatTrace.hasRsc();
	CHECK_EQUAL(hasRsc, true);
	RscStatTrace.show();
	NumRsc = RscStatTrace.getNumRsc(XAie_TileLoc(1,1), XAIE_CORE_MOD, XAIE_TRACE_EVENTS_RSC);
	CHECK_EQUAL(NumRsc, 2);

	XAieRscStat BcStat = Aie.broadcast(vL, StartM, EndM)->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	BcStat.show();
	hasRsc = BcStat.hasRsc();
	CHECK_EQUAL(hasRsc, true);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(0,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(1,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(2,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(2,0), XAIE_MEM_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 0);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(3,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 0);

	/* Specific resource not allocated yet */
	auto BcPreferred = Aie.broadcast(vL, StartM, EndM);
	BcPreferred->setPreferredId(9);
	BcStat = BcPreferred->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	BcStat.show();
	hasRsc = BcStat.hasRsc();
	CHECK_EQUAL(hasRsc, false);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(0,0), XAIE_MEM_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 0);

	/* Specific resource is allocated */
	RC = BcPreferred->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	BcStat = BcPreferred->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC);
	BcStat.show();
	hasRsc = BcStat.hasRsc();
	CHECK_EQUAL(hasRsc, true);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(0,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(1,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(2,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 1);
	NumRsc = RscStatAll.getNumRsc(XAie_TileLoc(3,0), XAIE_PL_MOD, XAIE_BCAST_CHANNEL_RSC);
	CHECK_EQUAL(NumRsc, 0);

	/* create group delete group */
	auto RGroupCreate = Aie.getRscGroup("CustomerRuntime");
	auto UserEvent1 = Aie.tile(1,2).core().userEvent(RGroupCreate);
	hasRsc = Aie.tile(1,2).core().userEvent()->getRscStat("CustomerRuntime").hasRsc();
	CHECK_EQUAL(hasRsc, false);
	RC = UserEvent1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	hasRsc = Aie.tile(1,2).core().userEvent()->getRscStat(XAIEDEV_DEFAULT_GROUP_GENERIC).hasRsc();
	CHECK_EQUAL(hasRsc, false);
	hasRsc = Aie.tile(1,2).core().userEvent()->getRscStat("CustomerRuntime").hasRsc();
	CHECK_EQUAL(hasRsc, true);
}
