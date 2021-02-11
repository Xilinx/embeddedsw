// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @param file xaiefal-rsc-base.hpp
 * Base classes for AI engine resources management
 */

#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/common/xaiefal-log.hpp>

#pragma once

using namespace std;

namespace xaiefal {
using namespace xaiefal::log;
	/**
	 * @class XAieDev
	 * @brief AI engine device class. It defines AI engine device
	 *	  operations.
	 */
	class XAieDev {
	public:
		XAieDev(XAie_DevInst *DevPtr,
			bool TurnOnFinishOnDestruct = false):
			AiePtr(DevPtr),
			FinishOnDestruct(TurnOnFinishOnDestruct) {
			//TODO: the following bitmaps initialization should be removed
			memset(XAieBroadcastCoreBits, 0, sizeof(XAieBroadcastCoreBits));
			memset(XAieBroadcastMemBits, 0, sizeof(XAieBroadcastCoreBits));
			memset(XAieBroadcastShimBits, 0, sizeof(XAieBroadcastCoreBits));

			memset(XAiePerfCoreBits, 0, sizeof(XAiePerfCoreBits));
			memset(XAiePerfMemBits, 0, sizeof(XAiePerfCoreBits));
			memset(XAiePerfShimBits, 0, sizeof(XAiePerfCoreBits));

			memset(XAiePcEventBits, 0, sizeof(XAiePcEventBits));

			memset(XAieSSCoreTBits, 0, sizeof(XAieSSCoreTBits));
			memset(XAieSSShimTBits, 0, sizeof(XAieSSShimTBits));
		}
		~XAieDev() {
			if (FinishOnDestruct) {
				XAie_Finish(AiePtr);
			}
		}

		/**
		 * This function returns module type string.
		 *
		 * @param Mod module type
		 * @param MString returns module type name in string
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC modToString(XAie_ModuleType Mod, string &MString) {
			AieRC RC = XAIE_OK;

			if (Mod == XAIE_CORE_MOD) {
				MString = "core";
			} else if (Mod == XAIE_MEM_MOD) {
				MString = "memory";
			} else if (Mod == XAIE_PL_MOD) {
				MString = "shim";
			} else {
				RC = XAIE_INVALID_ARGS;
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed. invalid module type." << endl;
			}
			return RC;
		}
		/**
		 * This function returns module type from module name string.
		 *
		 * @param MString module type name in string
		 * @param Mod returns module type
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC stringToMod(const string &MString, XAie_ModuleType &Mod) {
			AieRC RC = XAIE_OK;

			if (MString == "core") {
				Mod = XAIE_CORE_MOD;
			} else if (MString == "memory") {
				Mod = XAIE_MEM_MOD;
			} else if (MString == "shim") {
				Mod = XAIE_PL_MOD;
			} else {
				RC = XAIE_INVALID_ARGS;
				Logger::log(LogLevel::ERROR) << __func__ <<
				"failed. invalid module string." << endl;
			}
			return RC;
		}
		/**
		 * This function returns absolute tile location info from
		 * relative tile location mapping.
		 *
		 * @param Dev AI engine device
		 * @param info resource information mapping
		 * @param L returns absolute tile location if information is
		 *	    found in the mapping successfully.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getTileFromInfo(const std::map<std::string,
			std::vector<std::string>> &info,
			XAie_LocType &L) {
			XAie_LocType rL;
			string TTypeStr;
			AieRC RC = XAIE_OK;
			std::map<std::string, std::vector<std::string>>::const_iterator it;
			//auto it;

			it = info.find("tile_type");
			if (it == info.end()) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed perf counter, no tile type is found." << endl;
				RC = XAIE_INVALID_ARGS;
			}
			if (RC == XAIE_OK) {
				TTypeStr = it->second[0];
				it = info.find(TTypeStr + "_column");
				if (it == info.end()) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed perf counter, no tile column is found." << endl;
					RC = XAIE_INVALID_ARGS;
				}
			}
			if (RC == XAIE_OK) {
				rL.Col = (uint8_t)(std::stoi(it->second[0]) & 0xF);
				it = info.find(TTypeStr + "_row");
				if (it == info.end()) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed perf counter, no tile row is found." << endl;
					RC = XAIE_INVALID_ARGS;
				}
			}
			if (RC == XAIE_OK) {
				rL.Row = (uint8_t)(std::stoi(it->second[0]) & 0xF);
			}
			RC = getLoc(rL, TTypeStr, L);
			return RC;
		}
		/**
		 * This function converts absolute tile location into tile type
		 * relative tile location. E.g. tile(1, 1), its relative
		 * location can be tile(1, 0) of core tile depending on the
		 * actual device.
		 *
		 * @param L tile absolute location
		 * @param rL returns tile relative location
		 * @param TTypeStr returns the tile type in string
		 * @return XAIE_OK
		 */
		AieRC getRelativeLoc(const XAie_LocType &L, XAie_LocType &rL,
			string &TTypeStr) const {
			uint8_t Type = _XAie_GetTileTypefromLoc(AiePtr, L);

			rL.Col = L.Col;
			if (Type == XAIEGBL_TILE_TYPE_AIETILE) {
				rL.Row = L.Row - AiePtr->AieTileRowStart;
				TTypeStr = "core";
			} else {
				rL.Row = L.Row;
				TTypeStr = "shim";
			}
			return XAIE_OK;
		}
		/**
		 * This function converts tile type relative tile location into
		 * absolute tile location. E.g. core tile(1, 0), its absolute
		 * location can be tile(1, 1) depending on the actual device.
		 *
		 * @param rL tile type relative location
		 * @param TTypeStr tile type string
		 * @param L returns the absolute tile location
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getLoc(const XAie_LocType &rL, const string &TTypeStr,
			XAie_LocType &L) const {

			AieRC RC;
			if (TTypeStr == "core") {
				if (rL.Row >= AiePtr->AieTileNumRows) {
					RC = XAIE_INVALID_ARGS;
				} else {
					L.Row = rL.Row + AiePtr->AieTileRowStart;
					RC = XAIE_OK;
				}
			} else if (TTypeStr == "shim") {
				if (rL.Row > 0) {
				       RC = XAIE_INVALID_ARGS;
				} else {
					L.Row = rL.Row;
					RC = XAIE_OK;
				}
			} else {
				RC = XAIE_INVALID_ARGS;
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed. invalid location." << endl;
			}
			return RC;
		}
		/**
		 * This function returns AI engine device pointer.
		 *
		 * @return AI engine device pointer.
		 */
		XAie_DevInst *dev() {
			return AiePtr;
		}
		// TODO: the following bitmaps should be removed
		uint16_t XAieBroadcastCoreBits[400*16/sizeof(uint16_t)];
		uint16_t XAieBroadcastMemBits[400*16/sizeof(uint16_t)];
		uint16_t XAieBroadcastShimBits[400*16/sizeof(uint16_t)];

		uint64_t XAiePerfCoreBits[400 * 4/sizeof(uint64_t)];
		uint64_t XAiePerfMemBits[400 * 2/sizeof(uint64_t)];
		uint64_t XAiePerfShimBits[50 * 2/sizeof(uint64_t)];

		uint64_t XAiePcEventBits[400 * 4/sizeof(uint64_t)];

		uint64_t XAieSSCoreTBits[400 * 8/sizeof(uint64_t)];
		uint64_t XAieSSShimTBits[50 * 2/sizeof(uint64_t)];

	private:
		XAie_DevInst *AiePtr;
		bool FinishOnDestruct;
	};
}
