// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/rsc/xaiefal-groupevent.hpp>
#include <xaiefal/rsc/xaiefal-perf.hpp>

#pragma once

namespace xaiefal {
	class XAieActiveCycles: public XAiePerfCounter {
	public:
		XAieActiveCycles() = delete;
		XAieActiveCycles(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, bool CrossM = false):
			XAiePerfCounter(DevHd, L, XAIE_CORE_MOD, CrossM) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
		}
		XAieActiveCycles(XAieDev &Dev,
			XAie_LocType L, bool CrossM = false):
			XAieActiveCycles(Dev.getDevHandle(), L, CrossM) {}
	};
	class XAieStallCycles: public XAiePerfCounter {
	public:
		XAieStallCycles() = delete;
		XAieStallCycles(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L,
			std::shared_ptr<XAieGroupEventHandle> StallG,
			std::shared_ptr<XAieGroupEventHandle> FlowG,
			bool CrossM = false):
			XAiePerfCounter(DevHd, L, XAIE_CORE_MOD, CrossM),
			StallGroupEvent(StallG),
			FlowGroupEvent(FlowG) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
			StallGroupEvent->setGroupEvents(0x19F);
		}
		XAieStallCycles(XAieDev &Dev,
			XAie_LocType L,
			bool CrossM = false):
			XAiePerfCounter(Dev, L, XAIE_CORE_MOD, CrossM) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
			StallGroupEvent = Dev.tile(L).module(XAIE_CORE_MOD).groupEvent(XAIE_EVENT_GROUP_CORE_STALL_CORE);
			FlowGroupEvent = Dev.tile(L).module(XAIE_CORE_MOD).groupEvent(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
			StallGroupEvent->setGroupEvents(0x19F);
			}

		/**
		 * This function sets the stall group events enabling bits.
		 * Must be called before reserve.
		 *
		 * @param C group event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC stallGroupEventConfig(uint32_t C) {
			AieRC RC = XAIE_OK;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "StallCycles " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not allocated." << std::endl;
				RC = XAIE_ERR;
			} else {
				StallGroupEvent->setGroupEvents(C);
			}
			return RC;
		}

		/**
		 * This function sets the flow group events enabling bits.
		 * Must be called before reserve.
		 *
		 * @param C group event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC flowGroupEventConfig(uint32_t C) {
			AieRC RC = XAIE_OK;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "StallCycles " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not allocated." << std::endl;
				RC = XAIE_ERR;
			} else {
				FlowGroupEvent->setGroupEvents(C);
			}
			return RC;
		}
	protected:
		AieRC _reserveAppend() {
			AieRC RC;

			RC = StallGroupEvent->reserve();
			if (RC == XAIE_OK) {
				RC = FlowGroupEvent->reserve();
			}
			return RC;
		}
		AieRC _releaseAppend() {
			AieRC RC;

			RC = StallGroupEvent->release();
			if (RC == XAIE_OK) {
				RC = FlowGroupEvent->release();
			}
			return RC;
		}
		AieRC _startPrepend() {
			AieRC RC;

			RC = StallGroupEvent->start();
			if (RC == XAIE_OK) {
				RC = FlowGroupEvent->start();
			}
			return RC;
		}
		AieRC _stopAppend() {
			AieRC RC;

			RC = StallGroupEvent->stop();
			if (RC == XAIE_OK) {
				RC = FlowGroupEvent->stop();
			}
			return RC;
		}
	private:
		std::shared_ptr<XAieGroupEventHandle> StallGroupEvent;
		std::shared_ptr<XAieGroupEventHandle> FlowGroupEvent;
	};

	/**
	 * @class XAieStallOccurrences
	 * @brief class for number of stall occurences
	 */
	class XAieStallOccurrences: public XAiePerfCounter {
	public:
		XAieStallOccurrences() = delete;
		XAieStallOccurrences(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L,
			std::shared_ptr<XAieGroupEventHandle> StallG,
			bool CrossM = false):
			XAiePerfCounter(DevHd, L, XAIE_CORE_MOD, CrossM),
			StallGroupEvent(StallG) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE);
			StallGroupEvent->setGroupEvents(0x19F);
		}
		XAieStallOccurrences(XAieDev &Dev,
			XAie_LocType L,
			bool CrossM = false):
			XAiePerfCounter(Dev, L, XAIE_CORE_MOD, CrossM) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE);
			StallGroupEvent = Dev.tile(L).module(XAIE_CORE_MOD).groupEvent(XAIE_EVENT_GROUP_CORE_STALL_CORE);
			StallGroupEvent->setGroupEvents(0x19F);
			}

		/**
		 * This function sets the stall group events enabling bits
		 * Must be called before reserve.
		 *
		 * @param C group event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC stallGroupEventConfig(uint32_t C) {
			AieRC RC = XAIE_OK;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "StallCycles " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not allocated." << std::endl;
				RC = XAIE_ERR;
			} else {
				StallGroupEvent->setGroupEvents(C);
			}
			return RC;
		}
	protected:
		AieRC _reserveAppend() {
			AieRC RC;

			RC = StallGroupEvent->reserve();
			return RC;
		}
		AieRC _releaseAppend() {
			AieRC RC;

			RC = StallGroupEvent->release();
			return RC;
		}
		AieRC _startPrepend() {
			AieRC RC;

			RC = StallGroupEvent->start();
			return RC;
		}
		AieRC _stopAppend() {
			AieRC RC;

			RC = StallGroupEvent->stop();
			return RC;
		}
	private:
		std::shared_ptr<XAieGroupEventHandle> StallGroupEvent;
	};
}
