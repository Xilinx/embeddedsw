// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(Tracing)
{
};

TEST(Tracing, TraceBasicCoreEvent)
{
	AieRC RC;
	uint32_t MaxTraceEvents;
	uint32_t ReservedTraceEvents;
	uint32_t stopBcId;
	uint32_t AvailRscs;
	uint32_t RscType;
	XAie_ModuleType TraceMod;
	XAie_Events TraceEvent;
	XAie_Packet Packet = {0,0};

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	auto TraceFail = Aie.tile(1,3).core().traceEvent();
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE = Aie.tile(1,3).core().traceEvent();

	MaxTraceEvents = Tracing->getMaxTraceEvents();
	CHECK_EQUAL(MaxTraceEvents, 8);

	AvailRscs = Tracing->getAvailManagedRscs();
	RscType = Tracing->getManagedRscsType();

	ReservedTraceEvents = Tracing->getReservedTraceEvents();
	CHECK_EQUAL(ReservedTraceEvents, 0);

	stopBcId = Tracing->getStopBc();
	CHECK_EQUAL(stopBcId, -EPERM);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	stopBcId = Tracing->getStopBc();

	RC = Tracing->setMode(XAIE_TRACE_EVENT_TIME);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setPkt(Packet);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->getEvent(TraceMod, TraceEvent);
	CHECK_EQUAL(TraceMod, XAIE_CORE_MOD);
	CHECK_EQUAL(TraceEvent, XAIE_EVENT_USER_EVENT_1_CORE);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setMode(XAIE_TRACE_EVENT_TIME);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	int TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	int TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Cross module test */
	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	stopBcId = Tracing->getStopBc();

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, 0);

	TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, 0);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EPERM);

	TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EPERM);
}

TEST(Tracing, TraceBasicMemEvent)
{
	AieRC RC;
	uint32_t MaxTraceEvents;
	uint32_t ReservedTraceEvents;
	uint32_t stopBcId;
	uint32_t AvailRscs;
	uint32_t RscType;
	XAie_ModuleType TraceMod;
	XAie_Events TraceEvent;
	XAie_Packet Packet = {0,0};

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	auto TraceFail = Aie.tile(1,3).core().traceEvent();
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE = Aie.tile(1,3).core().traceEvent();

	MaxTraceEvents = Tracing->getMaxTraceEvents();
	CHECK_EQUAL(MaxTraceEvents, 8);

	AvailRscs = Tracing->getAvailManagedRscs();
	RscType = Tracing->getManagedRscsType();

	ReservedTraceEvents = Tracing->getReservedTraceEvents();
	CHECK_EQUAL(ReservedTraceEvents, 0);

	stopBcId = Tracing->getStopBc();
	CHECK_EQUAL(stopBcId, -EPERM);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	stopBcId = Tracing->getStopBc();

	RC = Tracing->setMode(XAIE_TRACE_EVENT_TIME);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setPkt(Packet);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->getEvent(TraceMod, TraceEvent);
	CHECK_EQUAL(TraceMod, XAIE_MEM_MOD);
	CHECK_EQUAL(TraceEvent, XAIE_EVENT_USER_EVENT_1_MEM);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setMode(XAIE_TRACE_EVENT_TIME);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	int TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	int TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Cross module test */
	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	stopBcId = Tracing->getStopBc();

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, 0);

	TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, 0);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EPERM);

	TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EPERM);
}

TEST(Tracing, TraceNegative)
{
	AieRC RC;
	uint32_t bcId;
	XAie_ModuleType TraceModule;
	XAie_Events TraceEvent;
	XAie_Packet Packet = {0,0};
	uint8_t TraceSlot;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE = Aie.tile(1,3).core().traceEvent();

	bcId = TraceE->getBc();
	CHECK_EQUAL(bcId, -EPERM);

	RC = TraceE->getEvent(TraceModule, TraceEvent);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = Tracing->setTraceEvent(8, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 0;

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->releaseTraceSlot(8);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setPkt(Packet);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->releaseTraceSlot(0);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->reserveTraceSlot(TraceSlot);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	int TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	int TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	DevInst.IsReady = 0;

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Cross module test */
	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC1 = Aie.tile(1,3).broadcast();
	BC1->reserve();

	auto BC2 = Aie.tile(1,3).broadcast();
	BC2->reserve();

	auto BC3 = Aie.tile(1,3).broadcast();
	BC3->reserve();

	auto BC4 = Aie.tile(1,3).broadcast();
	BC4->reserve();

	auto BC5 = Aie.tile(1,3).broadcast();
	BC5->reserve();

	auto BC6 = Aie.tile(1,3).broadcast();
	BC6->reserve();

	auto BC7 = Aie.tile(1,3).broadcast();
	BC7->reserve();

	auto BC8 = Aie.tile(1,3).broadcast();
	BC8->reserve();

	auto BC9 = Aie.tile(1,3).broadcast();
	BC9->reserve();

	auto BC10 = Aie.tile(1,3).broadcast();
	BC10->reserve();

	auto BC11 = Aie.tile(1,3).broadcast();
	BC11->reserve();

	auto BC12 = Aie.tile(1,3).broadcast();
	BC12->reserve();

	auto BC13 = Aie.tile(1,3).broadcast();
	BC13->reserve();

	auto BC14 = Aie.tile(1,3).broadcast();
	BC14->reserve();

	auto BC15 = Aie.tile(1,3).broadcast();
	BC15->reserve();

	auto BC16 = Aie.tile(1,3).broadcast();
	RC = BC16->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	BC16->release();
	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
}

TEST(Tracing, TraceNegativeBCOverflow)
{
	AieRC RC;
	uint32_t bcId;
	XAie_ModuleType TraceModule;
	XAie_Events TraceEvent;
	XAie_Packet Packet = {0,0};
	uint8_t TraceSlot;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE = Aie.tile(1,3).core().traceEvent();

	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC1 = Aie.tile(1,3).broadcast();
	BC1->reserve();

	auto BC2 = Aie.tile(1,3).broadcast();
	BC2->reserve();

	auto BC3 = Aie.tile(1,3).broadcast();
	BC3->reserve();

	auto BC4 = Aie.tile(1,3).broadcast();
	BC4->reserve();

	auto BC5 = Aie.tile(1,3).broadcast();
	BC5->reserve();

	auto BC6 = Aie.tile(1,3).broadcast();
	BC6->reserve();

	auto BC7 = Aie.tile(1,3).broadcast();
	BC7->reserve();

	auto BC8 = Aie.tile(1,3).broadcast();
	BC8->reserve();

	auto BC9 = Aie.tile(1,3).broadcast();
	BC9->reserve();

	auto BC10 = Aie.tile(1,3).broadcast();
	BC10->reserve();

	auto BC11 = Aie.tile(1,3).broadcast();
	BC11->reserve();

	auto BC12 = Aie.tile(1,3).broadcast();
	BC12->reserve();

	auto BC13 = Aie.tile(1,3).broadcast();
	BC13->reserve();

	auto BC14 = Aie.tile(1,3).broadcast();
	BC14->reserve();

	auto BC15 = Aie.tile(1,3).broadcast();
	BC15->reserve();

	auto BC16 = Aie.tile(1,3).broadcast();
	RC = BC16->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	BC16->release();
	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
}

TEST(Tracing, TraceNegativeMemEvent)
{
	AieRC RC;
	XAie_Packet Packet = {0,0};
	uint8_t TraceSlot;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE = Aie.tile(1,3).core().traceEvent();

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = Tracing->setTraceEvent(8, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 0;

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->releaseTraceSlot(8);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setPkt(Packet);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->releaseTraceSlot(0);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->reserveTraceSlot(TraceSlot);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	int TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	int TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Cross module test */
	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC1 = Aie.tile(1,3).broadcast();
	BC1->reserve();

	auto BC2 = Aie.tile(1,3).broadcast();
	BC2->reserve();

	auto BC3 = Aie.tile(1,3).broadcast();
	BC3->reserve();

	auto BC4 = Aie.tile(1,3).broadcast();
	BC4->reserve();

	auto BC5 = Aie.tile(1,3).broadcast();
	BC5->reserve();

	auto BC6 = Aie.tile(1,3).broadcast();
	BC6->reserve();

	auto BC7 = Aie.tile(1,3).broadcast();
	BC7->reserve();

	auto BC8 = Aie.tile(1,3).broadcast();
	BC8->reserve();

	auto BC9 = Aie.tile(1,3).broadcast();
	BC9->reserve();

	auto BC10 = Aie.tile(1,3).broadcast();
	BC10->reserve();

	auto BC11 = Aie.tile(1,3).broadcast();
	BC11->reserve();

	auto BC12 = Aie.tile(1,3).broadcast();
	BC12->reserve();

	auto BC13 = Aie.tile(1,3).broadcast();
	BC13->reserve();

	auto BC14 = Aie.tile(1,3).broadcast();
	BC14->reserve();

	auto BC15 = Aie.tile(1,3).broadcast();
	BC15->reserve();

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);
}

TEST(Tracing, TraceNegative3)
{
	AieRC RC;
	XAie_Packet Packet = {0,0};
	uint8_t TraceSlot;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE = Aie.tile(1,3).core().traceEvent();

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = Tracing->setTraceEvent(8, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 0;

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = TraceE->setEvent(XAIE_CORE_MOD, XAIE_EVENT_USER_EVENT_1_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_CORE, XAIE_EVENT_USER_EVENT_3_CORE);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->releaseTraceSlot(8);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = TraceE->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setPkt(Packet);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->releaseTraceSlot(0);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->reserveTraceSlot(TraceSlot);
	CHECK_EQUAL(RC, XAIE_ERR);

	RC = Tracing->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	int TracingStartBc = Tracing->getStartBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	int TraceBc = TraceE->getBc();
	CHECK_EQUAL(TracingStartBc, -EINVAL);

	RC = Tracing->release();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* Cross module test */
	RC = TraceE->setEvent(XAIE_MEM_MOD, XAIE_EVENT_USER_EVENT_1_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	auto BC1 = Aie.tile(1,3).broadcast();
	BC1->reserve();

	auto BC2 = Aie.tile(1,3).broadcast();
	BC2->reserve();

	auto BC3 = Aie.tile(1,3).broadcast();
	BC3->reserve();

	auto BC4 = Aie.tile(1,3).broadcast();
	BC4->reserve();

	auto BC5 = Aie.tile(1,3).broadcast();
	BC5->reserve();

	auto BC6 = Aie.tile(1,3).broadcast();
	BC6->reserve();

	auto BC7 = Aie.tile(1,3).broadcast();
	BC7->reserve();

	auto BC8 = Aie.tile(1,3).broadcast();
	BC8->reserve();

	auto BC9 = Aie.tile(1,3).broadcast();
	BC9->reserve();

	auto BC10 = Aie.tile(1,3).broadcast();
	BC10->reserve();

	auto BC11 = Aie.tile(1,3).broadcast();
	BC11->reserve();

	auto BC12 = Aie.tile(1,3).broadcast();
	BC12->reserve();

	auto BC13 = Aie.tile(1,3).broadcast();
	BC13->reserve();

	auto BC14 = Aie.tile(1,3).broadcast();
	BC14->reserve();

	auto BC15 = Aie.tile(1,3).broadcast();
	BC15->reserve();

	auto BC16 = Aie.tile(1,3).broadcast();
	BC16->reserve();

	RC = TraceE->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);
}

TEST(Tracing, TraceNegative4)
{
	AieRC RC;
	XAie_Packet Packet = {0,0};
	uint8_t TraceSlot;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto Tracing = Aie.tile(1,3).core().traceControl();
	auto TraceE1 = Aie.tile(1,3).core().traceEvent();
	auto TraceE2 = Aie.tile(1,3).core().traceEvent();
	auto TraceE3 = Aie.tile(1,3).core().traceEvent();
	auto TraceE4 = Aie.tile(1,3).core().traceEvent();
	auto TraceE5 = Aie.tile(1,3).core().traceEvent();
	auto TraceE6 = Aie.tile(1,3).core().traceEvent();
	auto TraceE7 = Aie.tile(1,3).core().traceEvent();
	auto TraceE8 = Aie.tile(1,3).core().traceEvent();
	auto TraceEFail = Aie.tile(1,3).core().traceEvent();

	RC = Tracing->setTraceEvent(0, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = Tracing->setTraceEvent(8, XAIE_EVENT_TRUE_CORE);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	RC = Tracing->setCntrEvent(XAIE_EVENT_USER_EVENT_2_MEM, XAIE_EVENT_USER_EVENT_3_MEM);
	CHECK_EQUAL(RC, XAIE_OK);

	RC = Tracing->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE2->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE3->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE4->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE5->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE6->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE7->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceE8->reserve();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = TraceEFail->reserve();
	CHECK_EQUAL(RC, XAIE_ERR);
}
