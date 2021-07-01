// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <string.h>
#include <vector>
#include <xaiengine.h>

#include <xaiefal/rsc/xaiefal-rsc-base.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAiePCEvent
	 * @brief AI engine PC event resource class
	 */
	class XAiePCEvent: public XAieSingleTileRsc {
	public:
		XAiePCEvent() = delete;
		XAiePCEvent(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L):
			XAieSingleTileRsc(DevHd, L, XAIE_CORE_MOD), PcAddr(0) {
			State.Initialized = 1;
		}
		XAiePCEvent(XAieDev &Dev, XAie_LocType L):
			XAiePCEvent(Dev.getDevHandle(), L) {};
		/**
		 * This function updates PC address of the PC event.
		 *
		 * @param Addr PC address
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC updatePcAddr(uint32_t Addr) {
			PcAddr = Addr;
			if (State.Running == 1) {
				XAie_EventPCDisable(dev(), Loc, Rsc.RscId);
				XAie_EventPCEnable(dev(), Loc, Rsc.RscId, PcAddr);
			} else {
				State.Configured = 1;
			}
			return XAIE_OK;
		}
		/**
		 * This function returns PC address of the PC event.
		 *
		 * @return PC address of the PC event
		 */
		uint32_t getPcAddr() const {
			return PcAddr;
		}
		/**
		 * This function returns PC event.
		 * It needs to be called after reserve() succeeds.
		 *
		 * @param E return the PC range event.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getEvent(XAie_Events &E) const {
			AieRC RC = XAIE_OK;
			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "PC Event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" resource not resesrved." << std::endl;
				RC = XAIE_ERR;
			} else {
				E = XAIE_EVENT_PC_0_CORE;
				E = (XAie_Events)(static_cast<uint32_t>(E) + Rsc.RscId);
			}
			return RC;
		}
		uint32_t getRscType() const {
			return static_cast<uint32_t>(XAIE_PC_EVENTS_RSC);
		}
	protected:
		uint32_t PcAddr; /**< PC address */
	private:
		AieRC _reserve() {
			AieRC RC;

			if (_XAie_GetTileTypefromLoc(dev(), Loc) != XAIEGBL_TILE_TYPE_AIETILE) {
				Logger::log(LogLevel::ERROR) << "PC event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" tile is not core tile." << std::endl;
				RC = XAIE_ERR;
			} else {
				if (preferredId == XAIE_RSC_ID_ANY) {
					XAie_UserRscReq Req = {Loc, Mod, 1};
					RC = XAie_RequestPCEvents(AieHd->dev(), 1, &Req, 1, &Rsc);
				} else {
					Rsc.RscType = XAIE_PC_EVENTS_RSC;
					Rsc.Loc.Col = Loc.Col;
					Rsc.Loc.Row = Loc.Row;
					Rsc.Mod = static_cast<uint32_t>(Mod);
					Rsc.RscId = preferredId;
					RC = XAie_RequestAllocatedPCEvents(AieHd->dev(), 1, &Rsc);
				 }
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::WARN) << "PC event " << __func__ << " (" <<
						static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
						" no available resource.\n";
				} else {
					Rsc.RscId -= static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
				}
			}
			return RC;
		}
		AieRC _release() {
			AieRC RC;

			Rsc.RscId += static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
			RC = XAie_ReleasePCEvents(AieHd->dev(), 1, &Rsc);
			Rsc.RscId -= static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);

			return RC;
		}
		AieRC _start() {
			return XAie_EventPCEnable(dev(), Loc, Rsc.RscId, PcAddr);
		}
		AieRC _stop() {
			return XAie_EventPCDisable(dev(), Loc, Rsc.RscId);
		}
	};

	/**
	 * @class XAiePCRange
	 * @brief AI engine PC addresses range resource class
	 */
	class XAiePCRange: public XAieSingleTileRsc {
	public:
		XAiePCRange() = delete;
		XAiePCRange(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L):
			XAieSingleTileRsc(DevHd, L) {
			for (int i = 0;
				i < (int)(sizeof(PcAddrs)/sizeof(PcAddrs[0]));
				i++) {
				PcAddrs[i] = 0;
			}
			State.Initialized = 1;
		}
		XAiePCRange(XAieDev &Dev, XAie_LocType L):
			XAiePCRange(Dev.getDevHandle(), L) {};
		/**
		 * This function updates PC addresses of the range.
		 *
		 * @param Addr0 starting address
		 * @param Addr1 ending address
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC updatePcAddr(uint32_t Addr0, uint32_t Addr1) {
			PcAddrs[0] = Addr0;
			PcAddrs[1] = Addr1;
			if (State.Running == 1) {
				XAie_EventPCDisable(dev(), Loc, Rscs[0].RscId);
				XAie_EventPCDisable(dev(), Loc, Rscs[1].RscId);
				XAie_EventPCEnable(dev(), Loc, Rscs[0].RscId, PcAddrs[0]);
				XAie_EventPCEnable(dev(), Loc, Rscs[1].RscId, PcAddrs[1]);
			} else {
				State.Configured = 1;
			}
			return XAIE_OK;
		}
		/** This function returns PC addresses of the range.
		 *
		 * @param Addr0 returns the starting address
		 * @param Addr1 returns the ending address
		 */
		void getPcAddr(uint32_t &Addr0, uint32_t &Addr1) const {
			Addr0 = PcAddrs[0];
			Addr1 = PcAddrs[1];
		}
		/**
		 * This function returns PC range event.
		 * It needs to be called after reserve() succeeds.
		 *
		 * @param E return the PC range event.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getEvent(XAie_Events &E) const {
			AieRC RC = XAIE_OK;
			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" resource not resesrved." << std::endl;
				RC = XAIE_ERR;
			} else if (Rscs[0].RscId == 0) {
				E = XAIE_EVENT_PC_RANGE_0_1_CORE;
			} else {
				E = XAIE_EVENT_PC_RANGE_2_3_CORE;
			}
			return RC;
		}
		uint32_t getRscType() const {
			return static_cast<uint32_t>(XAIE_PC_EVENTS_RSC);
		}
	protected:
		uint32_t PcAddrs[2]; /**< starting and end PC addresses */
		XAie_UserRsc Rscs[2]; /**< start and end PC events */
	private:
		AieRC _reserve() {
			AieRC RC;
			XAie_UserRscReq Req = {Loc, Mod, 2};

			if (_XAie_GetTileTypefromLoc(dev(), Loc) != XAIEGBL_TILE_TYPE_AIETILE) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" not core tile." << std::endl;
				RC = XAIE_ERR;
			} else {
					RC = XAie_RequestPCRangeEvents(AieHd->dev(), 1, &Req, 2, Rscs);
					if (RC == XAIE_OK && Rscs[1].RscId != (Rscs[0].RscId + 1)) {
						XAie_ReleasePCEvents(AieHd->dev(), 1, &Rscs[1]);
						XAie_ReleasePCEvents(AieHd->dev(), 1, &Rscs[0]);

						RC = XAIE_ERR;
					}
				}
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
						static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
						" resource not availalble." << std::endl;
				} else {
					Rscs[0].RscId -= static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
					Rscs[1].RscId -= static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
					Rsc.Mod = XAIE_CORE_MOD;
					Rsc.RscId = Rscs[0].RscId / 2;
				}

			return RC;
		}
		AieRC _release() {
			Rscs[0].RscId += static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
			Rscs[1].RscId += static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
			XAie_ReleasePCEvents(AieHd->dev(), 1, &Rscs[0]);
			XAie_ReleasePCEvents(AieHd->dev(), 1, &Rscs[1]);
			Rscs[0].RscId -= static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);
			Rscs[1].RscId -= static_cast<uint32_t>(XAIE_EVENT_PC_0_CORE);

			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;

			RC = XAie_EventPCEnable(dev(), Loc, Rscs[0].RscId, PcAddrs[0]);
			if (RC == XAIE_OK) {
				RC = XAie_EventPCEnable(dev(), Loc, Rscs[1].RscId, PcAddrs[1]);
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" failed to start." << std::endl;
			}
			return RC;
		}
		AieRC _stop() {
			int iRC;
			AieRC RC;

			iRC = (int)XAie_EventPCDisable(dev(), Loc, Rscs[0].RscId);
			iRC |= (int)XAie_EventPCDisable(dev(), Loc, Rscs[1].RscId);

			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" failed to stop." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
			}
			return RC;
		}
		void _getRscs(std::vector<XAie_UserRsc> &vRscs) const {
			vRscs.push_back(Rscs[0]);
			vRscs.push_back(Rscs[1]);
		}
	};
}
