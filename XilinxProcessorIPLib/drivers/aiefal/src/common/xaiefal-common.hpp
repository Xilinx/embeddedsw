// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace xaiefal {
	#define XAIE_RSC_ID_ANY (-1U)
	#define XAIE_RSC_TYPE_ANY (-1U)
	#define XAIE_MOD_ANY (-1U)
	#define XAIE_LOC_ANY 0xFFU
}

#ifdef __linux__
#define __COMPILER_SUPPORTS_LOCKS__
#endif

#include <fstream>
#include <functional>
#ifdef __COMPILER_SUPPORTS_LOCKS__
#include <mutex>
#endif

#ifdef __COMPILER_SUPPORTS_LOCKS__
#define _XAIEFAL_MUTEX_ACQUIRE(L) const std::lock_guard<std::mutex> lock(L)
#define _XAIEFAL_MUTEX_DECLARE(L) std::mutex L
#else
#define _XAIEFAL_MUTEX_ACQUIRE(...)
#define _XAIEFAL_MUTEX_DECLARE(...)
#endif
