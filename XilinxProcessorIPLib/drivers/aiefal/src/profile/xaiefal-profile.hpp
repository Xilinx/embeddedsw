// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <vector>
#include <xaiengine.h>
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
			XAie_LocType L, bool CrossM = false):
			XAiePerfCounter(DevHd, L, XAIE_CORE_MOD, CrossM) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
		}
		XAieStallCycles(XAieDev &Dev,
			XAie_LocType L, bool CrossM = false):
			XAieStallCycles(Dev.getDevHandle(), L, CrossM) {}
		AieRC _startPrepend() {
			XAie_Write32(dev(),
				     _XAie_GetTileAddr(dev(),
						       Loc.Row,
						       Loc.Col) + 0x00034508,
				     0x19F);
			return XAIE_OK;
		}
	};

}
