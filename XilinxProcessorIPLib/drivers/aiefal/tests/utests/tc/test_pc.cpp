// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace std;
using namespace xaiefal;

TEST_GROUP(PC)
{
};

TEST(PC, PCEvent) {
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
  auto pcEvent = Aie->tile(1,1).core().pcEvent();

  RC = pcEvent->reserve();
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcEvent->updatePcAddr(0x4);
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcEvent->start();
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcEvent->stop();
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcEvent->release();
  CHECK_EQUAL(RC, XAIE_OK);

}

TEST(PC, PCRange) {
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
  auto pcRange = Aie->tile(1,1).core().pcRange();

  RC = pcRange->updatePcAddr(0x4, 0x8);
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcRange->reserve();
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcRange->start();
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcRange->stop();
  CHECK_EQUAL(RC, XAIE_OK);

  RC = pcRange->release();
  CHECK_EQUAL(RC, XAIE_OK);

}
