// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
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
		//TODO: should be replaced with SSW AIE driver rsc manager
		static AieRC XAieAllocRsc(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L,
				XAie_UserRsc &R) {
			uint64_t *bits;
			int bit, sbit;
			AieRC RC = XAIE_OK;

			(void)Dev;
			if (L.Row == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"PCEvent: failed, not core row." << std::endl;
				return XAIE_ERR;
			} else {
				bits = Dev->XAiePcEventBits;
				sbit = (L.Col * 8 + (L.Row - 1)) * 4;
			}
			bit = XAieRsc::alloc_rsc_bit(bits, sbit, 4);

			if (bit < 0) {
				RC = XAIE_ERR;
			} else {
				R.Loc = L;
				if (L.Row == 0) {
					R.Mod = XAIE_PL_MOD;
				} else {
					R.Mod = XAIE_CORE_MOD;
				}
				R.Type = XAieRscType::SSWITCHSELECT;
				R.RscId = bit - sbit;
			}
			return RC;
		}
		static void XAieReleaseRsc(std::shared_ptr<XAieDev> Dev,
				const XAie_UserRsc &R) {
			uint64_t *bits;
			int pos;

			(void)Dev;
			if (R.Loc.Row == 0 || R.RscId >= 4) {
				return;
			}
			bits = Dev->XAiePcEventBits;
			pos = (R.Loc.Col * 8 + (R.Loc.Row - 1)) * 8 + R.RscId;
			XAieRsc::clear_rsc_bit(bits, pos);
		}
	public:
		XAiePCEvent() = delete;
		XAiePCEvent(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L):
			XAieSingleTileRsc(Dev, L), PcAddr(0) {
			uint32_t TType = _XAie_GetTileTypefromLoc(Aie->dev(), Loc);

			if (TType != XAIEGBL_TILE_TYPE_AIETILE) {
				Logger::log(LogLevel::ERROR) << "PC event " <<
					__func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" tile is not core tile." << std::endl;
			} else {
				State.Initialized = 1;
			}
		}
		/**
		 * This function updates PC address of the PC event.
		 *
		 * @param Addr PC address
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC updatePcAddr(uint32_t Addr) {
			PcAddr = Addr;
			if (State.Running == 1) {
				XAie_EventPCDisable(Aie->dev(), Loc, Rsc.RscId);
				XAie_EventPCEnable(Aie->dev(), Loc, Rsc.RscId, PcAddr);
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
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" resource not resesrved." << std::endl;
				RC = XAIE_ERR;
			} else {
				E = XAIE_EVENT_PC_0_CORE;
				E = (XAie_Events)((uint32_t)E + Rsc.RscId);
			}
			return RC;
		}
	protected:
		uint32_t PcAddr; /**< PC address */
	private:
		AieRC _reserve() {
			AieRC RC;

			if (_XAie_GetTileTypefromLoc(Aie->dev(), Loc) != XAIEGBL_TILE_TYPE_AIETILE) {
				Logger::log(LogLevel::ERROR) << "PC event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" tile is not core tile." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAiePCEvent::XAieAllocRsc(Aie, Loc, Rsc);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "PC event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
						" no available resource." << std::endl;
				}
			}
			return RC;
		}
		AieRC _release() {
			XAiePCEvent::XAieReleaseRsc(Aie, Rsc);

			return XAIE_OK;
		}
		AieRC _start() {
			return XAie_EventPCEnable(Aie->dev(), Loc, Rsc.RscId, PcAddr);
		}
		AieRC _stop() {
			return XAie_EventPCDisable(Aie->dev(), Loc, Rsc.RscId);
		}
	};

	/**
	 * @class XAiePCRange
	 * @brief AI engine PC addresses range resource class
	 */
	class XAiePCRange: public XAieSingleTileRsc {
	public:
		XAiePCRange() = delete;
		XAiePCRange(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L):
			XAieSingleTileRsc(Dev, L) {
			for (int i = 0;
				i < (int)(sizeof(PcAddrs)/sizeof(PcAddrs[0]));
				i++) {
				PcAddrs[i] = 0;
			}
			State.Initialized = 1;
		}
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
				XAie_EventPCDisable(Aie->dev(), Loc, Rscs[0].RscId);
				XAie_EventPCDisable(Aie->dev(), Loc, Rscs[1].RscId);
				XAie_EventPCEnable(Aie->dev(), Loc, Rscs[0].RscId, PcAddrs[0]);
				XAie_EventPCEnable(Aie->dev(), Loc, Rscs[1].RscId, PcAddrs[1]);
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
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" resource not resesrved." << std::endl;
				RC = XAIE_ERR;
			} else if (Rscs[0].RscId == 0) {
				E = XAIE_EVENT_PC_RANGE_0_1_CORE;
			} else {
				E = XAIE_EVENT_PC_RANGE_2_3_CORE;
			}
			return RC;
		}
		/**
		 * This function returns counter tile location
		 *
		 * @return counter tile location
		 */
		const XAie_LocType& loc() const {
			return Loc;
		}
	protected:
		uint32_t PcAddrs[2]; /**< starting and end PC addresses */
		XAie_UserRsc Rscs[2]; /**< start and end PC events */
	private:
		AieRC _reserve() {
			AieRC RC;

			if (_XAie_GetTileTypefromLoc(Aie->dev(), Loc) != XAIEGBL_TILE_TYPE_AIETILE) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" not core tile." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAiePCEvent::XAieAllocRsc(Aie, Loc, Rscs[0]);
				if (RC == XAIE_OK) {
					RC = XAiePCEvent::XAieAllocRsc(Aie, Loc, Rscs[1]);
					if (RC == XAIE_OK && Rscs[1].RscId != (Rscs[0].RscId + 1)) {
						XAiePCEvent::XAieReleaseRsc(Aie, Rscs[1]);
						XAiePCEvent::XAieReleaseRsc(Aie, Rscs[0]);
						RC = XAIE_ERR;
					}
				}
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
						" resource not availalble." << std::endl;
				} else {
					Rsc.Mod = XAIE_CORE_MOD;
					Rsc.RscId = Rscs[0].RscId / 2;
				}
			}
			return RC;
		}
		AieRC _release() {
			XAiePCEvent::XAieReleaseRsc(Aie, Rscs[0]);
			XAiePCEvent::XAieReleaseRsc(Aie, Rscs[1]);

			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;

			RC = XAie_EventPCEnable(Aie->dev(), Loc, Rscs[0].RscId, PcAddrs[0]);
			if (RC == XAIE_OK) {
				RC = XAie_EventPCEnable(Aie->dev(), Loc, Rscs[1].RscId, PcAddrs[1]);
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" failed to start." << std::endl;
			}
			return RC;
		}
		AieRC _stop() {
			int iRC;
			AieRC RC;

			iRC = (int)XAie_EventPCDisable(Aie->dev(), Loc, Rscs[0].RscId);
			iRC |= (int)XAie_EventPCDisable(Aie->dev(), Loc, Rscs[1].RscId);

			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "PC range " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" failed to stop." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
			}
			return RC;
		}
	};
}
