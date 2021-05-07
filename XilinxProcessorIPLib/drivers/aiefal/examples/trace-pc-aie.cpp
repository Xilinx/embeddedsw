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

class XAieTracePcRange {
public:
	XAieTracePcRange() = delete;
	XAieTracePcRange(std::shared_ptr<XAieDev> Dev, const std::string &Name = ""):
		Aie(Dev),
		Container(Name),
		vTracePC(Dev, "TracePCs"),
		vTraceTime(Dev, "TraceTime") {}
	AieRC addPcRange(uint32_t PcAddr0, uint32_t PcAddr1) {
		string PcRangeGrName = "PcRange" + std::to_string(vPcAddr0.size());
		vPcAddr0.push_back(PcAddr0);
		vPcAddr1.push_back(PcAddr1);
		vPcRange.push_back(XAieRscGroup<XAiePCRange>(Aie, PcRangeGrName));
		for (int i = 0; i < (int)vTracePC.size(); i++) {
			vPcRange.back().addRsc(vTracePC[i].loc());
			(vPcRange.back())[i].updatePcAddr(PcAddr0, PcAddr1);
		}
		return XAIE_OK;
	}
	AieRC removePcRange(uint32_t PcAddr0, uint32_t PcAddr1) {
		if (vTracePC.isReserved()) {
			Logger::log(LogLevel::ERROR) << __func__ <<
				"failed, resource are reserved." << endl;
			return XAIE_ERR;
		}
		for (int i = 0 ; i < (int)vPcAddr0.size(); i++) {
			if (PcAddr0 != vPcAddr0[i] || PcAddr1 != vPcAddr1[i]) {
				continue;
			}
			vPcAddr0.erase(vPcAddr0.begin() + i);
			vPcAddr1.erase(vPcAddr1.begin() + i);
			vPcRange[i].clear();
			vPcRange.erase(vPcRange.begin() + i);
		}
		return XAIE_OK;
	}
	AieRC addTile(const std::vector<XAie_LocType> &vL) {
		AieRC RC = XAIE_OK;
		for (int i = 0; i < (int)vL.size(); i++) {
			RC = vTraceTime.addRsc<XAie_ModuleType>(vL[i], XAIE_MEM_MOD);
			if (RC != XAIE_OK) {
				return RC;
			}
			RC = vTracePC.addRsc<XAie_ModuleType>(vL[i], XAIE_CORE_MOD);
			if (RC != XAIE_OK) {
				vTraceTime.removeRsc(vL[i]);
				return RC;
			}
			for (int j = 0; j < (int)vPcAddr0.size(); j++) {
				cout << "main " << __func__ << " added to go PcRange group[" << j << "]" << endl;
				RC = vPcRange[j].addRsc(vL[i]);
				if (RC != XAIE_OK) {
					for (int k = 0; k < j; k++) {
						vPcRange[k].removeRsc(vL[i]);
					}
					vTraceTime.removeRsc(vL[i]);
					vTracePC.removeRsc(vL[i]);
					return RC;
				}
				(vPcRange[j])[i].updatePcAddr(vPcAddr0[j], vPcAddr1[j]);
			}
		}
		if (RC == XAIE_OK) {
			Container.addRsc(vTracePC);
			Container.addRsc(vTraceTime);
			for (int i = 0; i < (int)vPcAddr0.size(); i++) {
				Container.addRsc(vPcRange[i]);
			}
		}
		return RC;
	}
	AieRC removeTile(const std::vector<XAie_LocType> &vL) {
		if (vTracePC.isReserved()) {
			Logger::log(LogLevel::ERROR) << __func__ <<
				"failed for PC tracing, resource already reserved." << endl;
			return XAIE_ERR;
		}
		for (int l = 0; l < (int)vL.size(); l++) {
			vTracePC.removeRsc(vL[l]);
			vTraceTime.removeRsc(vL[l]);
			for (int j = 0; j < (int)vPcAddr0.size(); j++) {
				vPcRange[j].removeRsc(vL[l]);
			}
		}
		return XAIE_OK;
	}
	AieRC reserve() {
		AieRC RC;

		if (vTracePC.isReserved()) {
			return XAIE_OK;
		}
		if (vPcRange.size() != 0) {
			RC = XAIE_OK;
		} else {
			Logger::log(LogLevel::ERROR) << __func__ <<
				"failed, no PC range is specified." << endl;
			return XAIE_ERR;
		}
		for (int i = 0; i < (int)vPcRange.size(); i++) {
			for (int j = 0; j < (int)vPcRange[i].size(); j++) {
				XAie_Events E;
				RC = (vPcRange[i])[j].reserve();
				if (RC != XAIE_OK) {
					break;
				}
				(vPcRange[i])[j].getEvent(E);
				RC = vTracePC[j].addEvent(XAIE_CORE_MOD, E);
				if (RC != XAIE_OK) {
					break;
				}
				RC = vTraceTime[j].addEvent(XAIE_CORE_MOD, E);
				if (RC != XAIE_OK) {
					break;
				}
			}
			if (RC != XAIE_OK) {
				break;
			}
		}
		if (RC == XAIE_OK) {
			RC = vTracePC.reserve();
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed to reserve pc trace." << endl;
			}
		}
		if (RC == XAIE_OK) {
			RC = vTraceTime.reserve();
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed to reserve time trace." << endl;
			}
		}
		if (RC != XAIE_OK) {
			vTraceTime.release();
			vTracePC.release();
			for (int i = 0; i < (int)vPcRange.size(); i++) {
				for (int j = 0; j < (int)vPcRange[i].size(); j++) {
					(vPcRange[i])[j].release();
				}
			}
		}
		return RC;
	}
	AieRC release() {
		return Container.release();
	}
	AieRC start() {
		AieRC RC = XAIE_OK;
		for (int i = 0; i < (int)vPcAddr0.size(); i++) {
			RC = vPcRange[i].start();
			if (RC != XAIE_OK) {
				break;
			}
		}
		if (RC == XAIE_OK) {
			// Configure the PC trace control with start event event
			for (int i = 0; i < (int)vTracePC.size(); i++) {
				vTracePC[i].setCntrEvent(XAIE_EVENT_ACTIVE_CORE, XAIE_EVENT_DISABLED_CORE);
			}
			RC = vTracePC.start();
		}
		if (RC == XAIE_OK) {
			for (int i = 0; i < (int)vTraceTime.size(); i++) {
				vTraceTime[i].setCntrEvent(XAIE_EVENT_USER_EVENT_0_MEM, XAIE_EVENT_USER_EVENT_0_MEM);
			}
			RC = vTraceTime.start();
		}
		if (RC != XAIE_OK) {
			stop();
		}
		return RC;
	}
	AieRC stop() {
		return Container.stop();
	}
private:
	std::shared_ptr<XAieDev> Aie;
	XAieRscContainer Container;
	std::vector<XAieRscGroup<XAiePCRange>> vPcRange;
	XAieRscGroup<XAieTracing> vTracePC;
	XAieRscGroup<XAieTracing> vTraceTime;
	std::vector<uint32_t> vPcAddr0;
	std::vector<uint32_t> vPcAddr1;
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

	//Logger::get().setLogLevel(LogLevel::DEBUG);
	AiePtr = std::make_shared<XAieDev>(&DevInst, true);
	XAieTracePcRange TracePcRange(AiePtr);
	TracePcRange.addTile(vL);
	TracePcRange.addPcRange(0x800, 0x900);
	TracePcRange.addPcRange(0x600, 0x700);
	TracePcRange.reserve();
	TracePcRange.start();
	TracePcRange.stop();
	TracePcRange.release();

	return 0;
}
