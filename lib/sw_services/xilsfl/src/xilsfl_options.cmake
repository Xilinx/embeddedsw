# Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# xilsfl library options: BSP CACHE selection, CONFIG_ADVANCED_SECTOR_PROTECTION_NUM,
# and (when included from src/CMakeLists.txt) generation of xilsfl_config.h.
#
# Vitis / pyesw merges BSP lib_config from cmake -LH output only for cache variables whose names
# match the library prefix (^xilsfl, case-insensitive). Expose ASP as CACHE
# xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION ("true"|"false") so the picker label carries
# CONFIG_ADVANCED_SECTOR_PROTECTION next to "xilsfl", matching the enabled C preprocessor symbol.
#
# CONFIG_ADVANCED_SECTOR_PROTECTION_NUM (0 or 1) drives the C macro CONFIG_ADVANCED_SECTOR_PROTECTION
# (header + PRIVATE/PUBLIC compile definitions). ASP .c/.h bodies are guarded in source with
# #if CONFIG_ADVANCED_SECTOR_PROTECTION so they compile to empty units when disabled.

set(xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION "false" CACHE STRING
	"CONFIG_ADVANCED_SECTOR_PROTECTION: Micron OCTAL ASP - true|false maps to CPP CONFIG_ADVANCED_SECTOR_PROTECTION=1|0.")
set_property(CACHE xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION PROPERTY STRINGS "true" "false")

# Legacy BSP/domain YAML keys may still pass -DXILSFL_CONFIG_ADVANCED_SECTOR_PROTECTION=...; fold into primary if needed.
set(_legacy_asp "")
if(DEFINED XILSFL_CONFIG_ADVANCED_SECTOR_PROTECTION)
	set(_legacy_asp "${XILSFL_CONFIG_ADVANCED_SECTOR_PROTECTION}")
endif()

string(TOLOWER "${xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION}" _prim_l)
string(TOLOWER "${_legacy_asp}" _leg_l)

# Prefer explicitly enabled legacy when primary is still the default ("false").
if(_prim_l STREQUAL "false" AND NOT _leg_l STREQUAL "" AND
		(_leg_l STREQUAL "true" OR _leg_l STREQUAL "on" OR _leg_l STREQUAL "1" OR _leg_l STREQUAL "yes"))
	set(xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION "${_legacy_asp}" CACHE STRING
		"CONFIG_ADVANCED_SECTOR_PROTECTION: Micron OCTAL ASP - true|false maps to CPP CONFIG_ADVANCED_SECTOR_PROTECTION=1|0." FORCE)
endif()

string(TOLOWER "${xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION}" _asp_cfg)

set(CONFIG_ADVANCED_SECTOR_PROTECTION_NUM 0)
if(_asp_cfg STREQUAL "true" OR _asp_cfg STREQUAL "on" OR _asp_cfg STREQUAL "1" OR _asp_cfg STREQUAL "yes")
	set(CONFIG_ADVANCED_SECTOR_PROTECTION_NUM 1)
endif()

# Generate xilsfl_config.h only when configuring the library (src/), not standalone examples/.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR)
	configure_file(
		${CMAKE_CURRENT_LIST_DIR}/xilsfl_config.h.in
		${CMAKE_BINARY_DIR}/include/xilsfl_config.h
	)
endif()
