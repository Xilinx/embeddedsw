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

class XAieProfileIO {
public:
	XAieProfileIO() = delete;
	XAieProfileIO(std::shared_ptr<XAieDev> Dev):
		Aie(Dev){}
	AieRC setIOPorts(const XAie_LocType &sL, XAie_StrmPortIntf sPortIntf,
			StrmSwPortType sPortType, uint32_t sPortNum,
			const XAie_LocType &eL,
			XAie_StrmPortIntf ePortIntf, StrmSwPortType ePortType,
			uint32_t ePortNum) {
		AieRC RC;

		if (sL.Row != 0 || eL.Row != 0) {
			Logger::log(LogLevel::ERROR) << __func__ <<
				"failed set IO ports, not SHIM Tiles." << endl;
			RC = XAIE_INVALID_ARGS;
		} else {
			std::vector<XAie_LocType> vL;

			vSSelects.clear();
			vProfilers.clear();
			StartLoc = sL;
			StartPortIntf = sPortIntf;
			StartPortType = sPortType;
			StartPortNum = sPortNum;
			EndLoc = eL;
			EndPortIntf = ePortIntf;
			EndPortType = ePortType;
			EndPortNum = ePortNum;
			auto SS0 = Aie->tile(sL).sswitchPort();
			auto SS1 = Aie->tile(eL).sswitchPort();
			SS0->setPortToSelect(StartPortIntf, StartPortType, StartPortNum);
			SS1->setPortToSelect(EndPortIntf, EndPortType, EndPortNum);
			vSSelects.push_back(SS0);
			vSSelects.push_back(SS1);

			auto P0 = Aie->tile(sL).perfCounter();
			auto P1 = Aie->tile(eL).perfCounter();
			vProfilers.push_back(P0);
			vProfilers.push_back(P1);

			for (uint8_t c = sL.Col; c <= eL.Col; c++) {
				vL.push_back(XAie_TileLoc(c, 0));
			}
			BCRsc = Aie->broadcast(vL, XAIE_PL_MOD, XAIE_PL_MOD);

			RC = XAIE_OK;
		}
		return RC;
	}
	AieRC reserve() {
		AieRC RC;

		if (vProfilers.size() == 0) {
			Logger::log(LogLevel::ERROR) << __func__ <<
				"failed to reserve, no IO path configure." << endl;
			RC = XAIE_ERR;
		} else {
			RC = BCRsc->reserve();
			if (RC == XAIE_OK) {
				XAie_Events lStartE, lEndE;
				for (auto R: vSSelects) {
					RC = R->reserve();
					if (RC != XAIE_OK) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"failed to reserve SS" << endl;
						BCRsc->release();
						return RC;
					}
				}

				lStartE = XAIE_EVENT_USER_EVENT_0_PL;
				vSSelects[0]->getSSRunningEvent(lEndE);
				vProfilers[0]->initialize(XAIE_PL_MOD, lStartE, XAIE_PL_MOD, lEndE);
				BCRsc->getEvent(vProfilers[1]->loc(), XAIE_PL_MOD, lStartE);
				vSSelects[1]->getSSRunningEvent(lEndE);
				vProfilers[1]->initialize(XAIE_PL_MOD, lStartE, XAIE_PL_MOD, lEndE);
				for (auto R: vProfilers) {
					RC = R->reserve();
					if (RC != XAIE_OK) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"failed to reserve Profilers" << endl;
						for (auto sR: vSSelects) {
							sR->release();
						}
						BCRsc->release();
					}
				}
			}
		}
		return RC;
	}
	AieRC release() {
		for (auto R: vProfilers) {
			R->release();
		}
		for (auto R: vSSelects) {
			R->release();
		}
		BCRsc->release();
		return XAIE_OK;
	}
	AieRC start() {
		AieRC RC = XAIE_ERR;

		for (auto R: vProfilers) {
			if (!R->isReserved()) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed to start, resource not reserved." << endl;
				RC = XAIE_ERR;
				break;
			} else {
				RC = XAIE_OK;
			}
		}
		if (RC == XAIE_OK) {
			XAie_EventBroadcast(Aie->dev(), StartLoc, XAIE_PL_MOD,
				BCRsc->getBc(),
				XAIE_EVENT_USER_EVENT_0_PL);
			BCRsc->start();
			for (auto R: vProfilers) {
				RC = R->start();
				if (RC != XAIE_OK) {
					return RC;
				}
			}
			for (auto R: vSSelects) {
				RC = R->start();
				if (RC != XAIE_OK) {
					return RC;
				}
			}

			XAie_EventGenerate(Aie->dev(), StartLoc, XAIE_PL_MOD,
				XAIE_EVENT_USER_EVENT_0_PL);
			RC = XAIE_OK;
		}
		return RC;
	}
	AieRC stop() {
		for (auto R: vProfilers) {
			R->stop();
		}
		for (auto R: vSSelects) {
			R->stop();
		}
		BCRsc->stop();
		return XAIE_OK;
	}
	void printResult() {
		Logger::log(LogLevel::INFO) << " === Profile results. ==== " << std::endl;
		if (vProfilers[0]->isRunning() && vProfilers[1]->isRunning()) {
			uint32_t Result;

			vProfilers[0]->readResult(Result);
			Logger::log() << "\t(" << (uint32_t)vProfilers[0]->loc().Col <<
				"," << (uint32_t)vProfilers[0]->loc().Row << "):" <<
				"PerfCount=" << Result << endl;
			vProfilers[1]->readResult(Result);
			Logger::log() << "\t(" << (uint32_t)vProfilers[1]->loc().Col <<
				"," << (uint32_t)vProfilers[1]->loc().Row << "):" <<
				"PerfCount=" << Result << endl;
		}
	}
private:
	std::shared_ptr<XAieDev> Aie;
	XAie_LocType StartLoc;
	XAie_LocType EndLoc;
	XAie_StrmPortIntf StartPortIntf;
	StrmSwPortType StartPortType;
	uint32_t StartPortNum;
	XAie_StrmPortIntf EndPortIntf;
	StrmSwPortType EndPortType;
	uint32_t EndPortNum;
	std::shared_ptr<XAieBroadcast> BCRsc;
	std::vector<std::shared_ptr<XAieStreamPortSelect>> vSSelects;
	std::vector<std::shared_ptr<XAiePerfCounter>> vProfilers;
};

int main(void)
{
	AieRC RC;
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

	Logger::get().setLogLevel(LogLevel::DEBUG);
	AiePtr = std::make_shared<XAieDev>(&DevInst, true);
	XAieProfileIO ProfileIO(AiePtr);
	ProfileIO.setIOPorts(XAie_TileLoc(6,0), XAIE_STRMSW_SLAVE, SOUTH, 0,
			XAie_TileLoc(9, 0), XAIE_STRMSW_MASTER, SOUTH, 0);
	ProfileIO.reserve();
	ProfileIO.start();
	ProfileIO.printResult();
	ProfileIO.stop();
	ProfileIO.release();

	return 0;
}
