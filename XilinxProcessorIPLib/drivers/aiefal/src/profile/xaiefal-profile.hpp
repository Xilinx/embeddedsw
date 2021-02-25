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
		XAieActiveCycles(std::shared_ptr<XAieDev> Dev, XAie_LocType L):
			XAiePerfCounter(Dev, L, XAIE_CORE_MOD) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_ACTIVE_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_DISABLED_CORE);
		}
	};
	class XAieStallCycles: public XAiePerfCounter {
	public:
		XAieStallCycles() = delete;
		XAieStallCycles(std::shared_ptr<XAieDev> Dev, XAie_LocType L):
			XAiePerfCounter(Dev, L, XAIE_CORE_MOD) {
			initialize(XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_STALL_CORE,
				XAIE_CORE_MOD, XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
		}
		AieRC _startPrepend() {
			XAie_Write32(Aie->dev(),
				     _XAie_GetTileAddr(Aie->dev(),
						       Loc.Row,
						       Loc.Col) + 0x00034508,
				     0x19F);
			return XAIE_OK;
		}
	};

}
