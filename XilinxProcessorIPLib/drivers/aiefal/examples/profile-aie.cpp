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
	XAieHeatMap(std::shared_ptr<XAieDev> Dev, const std::string &Name = ""):
		vActive(Dev, "ActiveCycles"),
		vStall(Dev, "StallCycles"),
		Container(Name) {}
	AieRC addTiles(const std::vector<XAie_LocType> &vL) {
		AieRC RC;

		RC = vActive.addRsc(vL);
		if (RC == XAIE_OK) {
			RC = vStall.addRsc(vL);
		}
		Container.addRsc(vActive);
		Container.addRsc(vStall);
		return RC;
	}
	AieRC reserve() {
		return Container.reserve();
	}
	AieRC release() {
		return Container.release();
	}
	AieRC start() {
		return Container.start();
	}
	AieRC stop() {
		return Container.stop();
	}
	void printResult() {
		Logger::log(LogLevel::INFO) << " === Profile results. ==== " << std::endl;
		if (vActive.isRunning()) {
			for (int i = 0; i < (int)vActive.size(); i++) {
				uint32_t R;

				vActive[i].readResult(R);
				Logger::log() << "\t(" << (uint32_t)vActive[i].loc().Col <<
					"," << (uint32_t)vActive[i].loc().Row << "):" <<
					"Active=" << R << endl;
			}
		}
		if (vStall.isRunning()) {
			for (int i = 0; i < (int)vStall.size(); i++) {
				uint32_t R;

				vStall[i].readResult(R);
				Logger::log() << "\t(" << (uint32_t)vStall[i].loc().Col <<
					"," << (uint32_t)vStall[i].loc().Row << "):" <<
					"Stall=" << R << endl;
			}
		}
	}
private:
	XAieRscGroup<XAieActiveCycles> vActive;
	XAieRscGroup<XAieStallCycles> vStall;
	XAieRscContainer Container;
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
