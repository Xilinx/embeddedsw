// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <unistd.h>

#include "xaiefal/xaiefal.hpp"

#define HW_GEN XAIE_DEV_GEN_AIE
#define XAIE_NUM_ROWS            9
#define XAIE_NUM_COLS            50
#define XAIE_ADDR_ARRAY_OFF      0x800

#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 23
#define XAIE_ROW_SHIFT 18
#define XAIE_SHIM_ROW 0
#define XAIE_MEM_TILE_ROW_START 0
#define XAIE_MEM_TILE_NUM_ROWS 0
#define XAIE_AIE_TILE_ROW_START 1
#define XAIE_AIE_TILE_NUM_ROWS 8

using namespace std;
using namespace xaiefal;

class XAieHeatMap {
public:
	XAieHeatMap() = delete;
	XAieHeatMap(std::shared_ptr<XAieDev> Dev):
		AieDev(Dev) {}
	AieRC addTiles(const std::vector<XAie_LocType> &vL) {
		for (auto L: vL) {
			auto PerfCounterActive = AieDev->tile(L).core().activeCycles();
			vActive.push_back(PerfCounterActive);
			auto PerfCounterStall = AieDev->tile(L).core().stallCycles();
			vStall.push_back(PerfCounterStall);
		}
		return XAIE_OK;
	}
	AieRC reserve() {
		AieRC RC = XAIE_OK;
		for (auto R: vActive) {
			RC = R->reserve();
			if (RC != XAIE_OK) {
				return RC;
			}
		}
		for (auto R: vStall) {
			RC = R->reserve();
			if (RC != XAIE_OK) {
				return RC;
			}
		}

		return RC;
	}
	AieRC release() {
		for (auto R: vActive) {
			R->release();
		}
		for (auto R: vStall) {
			R->release();
		}
		return XAIE_OK;
	}
	AieRC start() {
		AieRC RC = XAIE_OK;
		for (auto R: vActive) {
			RC = R->start();
			if (RC != XAIE_OK) {
				return RC;
			}
		}
		for (auto R: vStall) {
			RC = R->start();
			if (RC != XAIE_OK) {
				return RC;
			}
		}

		return RC;
	}
	AieRC stop() {
		for (auto R: vActive) {
			R->stop();
		}
		for (auto R: vStall) {
			R->stop();
		}
		return XAIE_OK;
	}
	void printResult() {
		Logger::log(LogLevel::INFO) << " === Profile results. ==== " << std::endl;
		for (auto R: vActive) {
			uint32_t Result;

			if (R->isRunning()) {
				R->readResult(Result);
				Logger::log() << "\t(" << static_cast<uint32_t>(R->loc().Col) <<
					"," << static_cast<uint32_t>(R->loc().Row) << "):" <<
					"Active=" << Result << endl;
			}
		}
		for (auto R: vStall) {
			uint32_t Result;

			if (R->isRunning()) {
				R->readResult(Result);
				Logger::log() << "\t(" << static_cast<uint32_t>(R->loc().Col) <<
					"," << static_cast<uint32_t>(R->loc().Row) << "):" <<
					"Stall=" << Result << endl;
			}
		}
	}
private:
	std::shared_ptr<XAieDev> AieDev;
	std::vector<std::shared_ptr<XAieActiveCycles>> vActive;
	std::vector<std::shared_ptr<XAieStallCycles>> vStall;
};

int main(void)
{
	AieRC RC;
	std::vector<XAie_LocType> vL;
	std::shared_ptr<XAieDev> AiePtr;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	if (RC != XAIE_OK) {
		std::cout << "Failed to intialize AI engine partition" << std::endl;
		return -1;
	}


    //Request tiles
    u8 NumTiles = 2;
    XAie_LocType Loc[NumTiles];
    Loc[0] = XAie_TileLoc(1, 1);
    Loc[1] = XAie_TileLoc(1, 2);

    RC = XAie_PmRequestTiles(&(DevInst), Loc, NumTiles);

	vL.push_back(XAie_TileLoc(1,1));
	vL.push_back(XAie_TileLoc(1,2));

	AiePtr = std::make_shared<XAieDev>(&DevInst, true);
	XAieHeatMap HeatMap(AiePtr);
	HeatMap.addTiles(vL);
	HeatMap.reserve();
	HeatMap.start();
	HeatMap.printResult();
	HeatMap.stop();
	HeatMap.release();

	return 0;
}
