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
	XAieTracePcRange(std::shared_ptr<XAieDev> Dev): Aie(Dev) {}
	AieRC addPcRange(uint32_t PcAddr0, uint32_t PcAddr1) {
		string PcRangeGrName = "PcRange" + std::to_string(vPcAddr0.size());
		vPcAddr0.push_back(PcAddr0);
		vPcAddr1.push_back(PcAddr1);
		std::vector<std::shared_ptr<XAiePCRange>> PcRange;
		std::vector<std::shared_ptr<XAieTraceEvent>> PcTraceEvents;
		std::vector<std::shared_ptr<XAieTraceEvent>> TimeTraceEvents;
		for (auto R: vTracePC) {
			auto PcRangeR = Aie->tile(R->loc()).core().pcRange();
			PcRangeR->updatePcAddr(PcAddr0, PcAddr1);
			PcRange.push_back(PcRangeR);

			auto PcTraceE = Aie->tile(R->loc()).core().traceEvent();
			PcTraceEvents.push_back(PcTraceE);
			auto TimeTraceE = Aie->tile(R->loc()).mem().traceEvent();
			TimeTraceEvents.push_back(TimeTraceE);
		}
		vPcRange.push_back(PcRange);
		vTracePCE.push_back(PcTraceEvents);
		vTraceTimeE.push_back(TimeTraceEvents);
		return XAIE_OK;
	}
	AieRC addTile(const std::vector<XAie_LocType> &vL) {
		AieRC RC = XAIE_OK;
		for (auto L: vL) {
			auto rTimeTrace = Aie->tile(L).mem().traceControl();
			vTraceTime.push_back(rTimeTrace);
			auto rPcTrace = Aie->tile(L).core().traceControl();
			vTracePC.push_back(rPcTrace);
			for (int j = 0; j < (int)vPcAddr0.size(); j++) {
				cout << "main " << __func__ << " added to go PcRange group[" << j << "]" << endl;
				auto rPcRange = Aie->tile(L).core().pcRange();
				rPcRange->updatePcAddr(vPcAddr0[j], vPcAddr1[j]);
				vPcRange[j].push_back(rPcRange);
				auto pcTraceEvents = Aie->tile(L).core().traceEvent();
				vTracePCE[j].push_back(pcTraceEvents);
				auto timeTraceEvents = Aie->tile(L).mem().traceEvent();
				vTraceTimeE[j].push_back(timeTraceEvents);
			}
		}
		return RC;
	}
	AieRC reserve() {
		AieRC RC;

		if (vPcRange.size() != 0) {
			RC = XAIE_OK;
		} else {
			Logger::log(LogLevel::ERROR) << __func__ <<
				"failed, no PC range is specified." << endl;
			return XAIE_ERR;
		}
		for (auto R: vTracePC) {
			R->setCntrEvent(XAIE_EVENT_ACTIVE_CORE, XAIE_EVENT_DISABLED_CORE);
			RC = R->reserve();
			if (RC != XAIE_OK) {
				return RC;
			}
		}
		for (auto R: vTraceTime) {
			R->setCntrEvent(XAIE_EVENT_USER_EVENT_0_MEM, XAIE_EVENT_USER_EVENT_0_MEM);
			RC = R->reserve();
			if (RC != XAIE_OK) {
				return RC;
			}
		}

		for (int i = 0; i < (int)vPcRange.size(); i++) {
			for (int j = 0; j < (int)vPcRange[i].size(); j++) {
				XAie_Events E;
				RC = (vPcRange[i])[j]->reserve();
				if (RC != XAIE_OK) {
					break;
				}
				(vPcRange[i])[j]->getEvent(E);
				RC = (vTracePCE[i])[j]->setEvent(XAIE_CORE_MOD, E);
				if (RC != XAIE_OK) {
					break;
				}
				RC = (vTracePCE[i])[j]->reserve();
				if (RC != XAIE_OK) {
					break;
				}
				RC = (vTraceTimeE[i])[j]->setEvent(XAIE_CORE_MOD, E);
				if (RC != XAIE_OK) {
					break;
				}
				RC = (vTraceTimeE[i])[j]->reserve();
				if (RC != XAIE_OK) {
					break;
				}
			}
		}
		return RC;
	}
	AieRC release() {
		for (auto R: vTracePC) {
			R->release();
		}
		for (auto R: vTraceTime) {
			R->release();
		}
		for (auto vTraceE: vTracePCE) {
			for (auto R: vTraceE) {
				R->release();
			}
		}
		for (auto vTraceE: vTraceTimeE) {
			for (auto R: vTraceE) {
				R->release();
			}
		}
		for (auto vPC: vPcRange) {
			for (auto R: vPC) {
				R->release();
			}
		}
		return XAIE_OK;
	}
	AieRC start() {
		AieRC RC = XAIE_OK;
		for (int i = 0; i < (int)vPcAddr0.size(); i++) {
			for (auto R: vPcRange[i]) {
				RC = R->start();
				if (RC != XAIE_OK) {
					break;
				}
			}
		}
		if (RC == XAIE_OK) {
			for (auto vTraceE: vTracePCE) {
				for (auto R: vTraceE) {
					R->start();
				}
			}
			// Configure the PC trace control with start event event
			for (auto R: vTracePC) {
				RC = R->start();
				if (RC != XAIE_OK) {
					break;
				}
			}
		}
		if (RC == XAIE_OK) {
			for (auto vTraceE: vTraceTimeE) {
				for (auto R: vTraceE) {
					R->start();
				}
			}
			for (auto R: vTraceTime) {
				RC = R->start();
				if (RC != XAIE_OK) {
					break;
				}
			}
		}
		if (RC != XAIE_OK) {
			stop();
		}
		return RC;
	}
	AieRC stop() {
		for (auto R: vTracePC) {
			R->stop();
		}
		for (auto R: vTraceTime) {
			R->stop();
		}
		for (auto vTraceE: vTracePCE) {
			for (auto R: vTraceE) {
				R->stop();
			}
		}
		for (auto vTraceE: vTraceTimeE) {
			for (auto R: vTraceE) {
				R->stop();
			}
		}
		for (auto vPC: vPcRange) {
			for (auto R: vPC) {
				R->stop();
			}
		}
		return XAIE_OK;
	}
private:
	std::shared_ptr<XAieDev> Aie;
	std::vector<std::vector<std::shared_ptr<XAiePCRange>>> vPcRange;
	std::vector<std::shared_ptr<XAieTraceCntr>> vTracePC;
	std::vector<std::shared_ptr<XAieTraceCntr>> vTraceTime;
	std::vector<std::vector<std::shared_ptr<XAieTraceEvent>>> vTracePCE;
	std::vector<std::vector<std::shared_ptr<XAieTraceEvent>>> vTraceTimeE;
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
