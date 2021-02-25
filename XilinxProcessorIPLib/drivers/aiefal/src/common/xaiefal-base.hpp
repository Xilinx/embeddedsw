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
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/common/xaiefal-log.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAieDevHandle
	 * @brief AI engine low level device pointer handle.
	 */
	class XAieDevHandle: public std::enable_shared_from_this<XAieDevHandle>{
	public:
		XAieDevHandle(XAie_DevInst *DevPtr,
			bool TurnOnFinishOnDestruct = false):
			Dev(DevPtr),
			FinishOnDestruct(TurnOnFinishOnDestruct) {
			if (DevPtr == nullptr) {
				throw std::invalid_argument("AI engine device is NULL");
			}

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
		~XAieDevHandle() {
			if (FinishOnDestruct) {
				XAie_Finish(Dev);
			}
		}
		/**
		 * This function returns AI engine device pointer.
		 *
		 * @return AI engine device instance pointer
		 */
		XAie_DevInst *dev() {
			return Dev;
		}
		/**
		 * This function returns AI engine handle shared pointer
		 *
		 * @return AI engine device shared pointer
		 */
		std::shared_ptr<XAieDevHandle> getPtr() {
			return shared_from_this();
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
		XAie_DevInst *Dev;
		bool FinishOnDestruct;
	};

	class XAieMod;
	class XAieTile;

	/**
	 * @class XAieDev
	 * @brief AI engine device class. It defines AI engine device
	 *	  operations.
	 */
	class XAieDev {
	public:
		XAieDev() = delete;
		XAieDev(XAie_DevInst *DevPtr,
			bool TurnOnFinishOnDestruct = false);
		~XAieDev() {}

		/**
		 * This function returns a tile object reference.
		 *
		 * @param L tile Location
		 * @return tile object reference
		 */
		XAieTile &tile(XAie_LocType L) {
			if (L.Col > NumCols || L.Row > NumRows) {
				throw std::invalid_argument("Invalid tile location");
			}
			return Tiles[L.Col * NumRows + L.Row];
		}

		/**
		 * This function returns a tile object reference.
		 *
		 * @param C absolute column index
		 * @param R absolute row index, 0 is shim row
		 * @return tile object reference
		 */
		XAieTile &tile(uint8_t C, uint32_t R) {
			return tile(XAie_TileLoc(C, R));
		}

		/**
		 * This function returns AI engine device pointer.
		 *
		 * @return AI engine device pointer.
		 */

		XAie_DevInst *dev() {
			return AieHandle->dev();
		}

		/**
		 * This function returns AI engine device handle
		 *
		 * @return AI engine device handle
		 */
		std::shared_ptr<XAieDevHandle> getDevHandle() {
			return AieHandle->getPtr();
		}

		/**
		 * This function returns module type string.
		 *
		 * @param Mod module type
		 * @param MString returns module type name in string
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC modToString(XAie_ModuleType Mod, std::string &MString) {
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
					"failed. invalid module type." <<
					std::endl;
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
		AieRC stringToMod(const std::string &MString,
			XAie_ModuleType &Mod) {
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
				"failed. invalid module string." << std::endl;
			}
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
		AieRC getRelativeLoc(XAie_LocType L, XAie_LocType &rL,
			std::string &TTypeStr) const {
			uint8_t Type = _XAie_GetTileTypefromLoc(
					AieHandle->dev(), L);

			rL.Col = L.Col;
			if (Type == XAIEGBL_TILE_TYPE_AIETILE) {
				rL.Row = L.Row -
					AieHandle->dev()->AieTileRowStart;
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
		AieRC getLoc(XAie_LocType rL, const std::string &TTypeStr,
			XAie_LocType &L) const {

			AieRC RC;
			if (TTypeStr == "core") {
				if (rL.Row >=
					AieHandle->dev()->AieTileNumRows) {
					RC = XAIE_INVALID_ARGS;
				} else {
					L.Row = rL.Row +
						AieHandle->dev()->AieTileRowStart;
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
					"failed. invalid location." <<
					std::endl;
			}
			return RC;
		}
	private:
		std::shared_ptr<XAieDevHandle> AieHandle; /**< AI engine device
							    handle */
		uint32_t NumCols; /**< number of AI engine columns */
		uint32_t NumRows; /**< number of AI engine rows */
		std::vector<XAieTile> Tiles;
	};

	/**
	 * @class XAieMod
	 * @brief AI enigne module. It defines AI engine module operations.
	 *	  such as get resource within a module.
	 */
	class XAieMod {
	public:
		XAieMod() {};
		XAieMod(XAieDev &Dev, XAie_LocType L, XAie_ModuleType M):
			AieHandle(Dev.getDevHandle()), Loc(L), Mod(M) {
			if (_XAie_CheckModule(AieHandle->dev(), Loc, M) !=
				XAIE_OK) {
				throw std::invalid_argument("Invalid module and tile");
			}
			AieHandle = Dev.getDevHandle();
		}
		~XAieMod() {}

		/**
		 * This function returns tile location
		 *
		 * @return tile location
		 */
		XAie_LocType loc() const {
			return Loc;
		}

		/**
		 * This function returns module type
		 *
		 * @return module type
		 */
		XAie_ModuleType mod() const {
			return Mod;
		}
	private:
		std::shared_ptr<XAieDevHandle> AieHandle; /**< AI engine device
							    handle */
		XAie_LocType Loc; /**< Tile location */
		XAie_ModuleType Mod; /**< Module type */
	};
	/**
	 * @class XAieTile
	 * @brief AI enigne tile. It defines AI engine tile operations.
	 *	  such as get resource within a tile, or get a module.
	 */
	class XAieTile {
	public:
		XAieTile() {}
		XAieTile(XAieDev &Dev, XAie_LocType L):
			AieHandle(Dev.getDevHandle()), Loc(L) {
			uint32_t TType;

			TType = _XAie_GetTileTypefromLoc(Dev.dev(), Loc);

			if (TType == XAIEGBL_TILE_TYPE_MAX) {
				throw std::invalid_argument("Invalid tile");
			}
			if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
				Mods.push_back(XAieMod(Dev, L, XAIE_MEM_MOD));
				Mods.push_back(XAieMod(Dev, L, XAIE_CORE_MOD));
			} else {
				if (TType == XAIEGBL_TILE_TYPE_SHIMPL ||
					TType == XAIEGBL_TILE_TYPE_SHIMNOC) {
					Mods.push_back(XAieMod(Dev, L, XAIE_PL_MOD));
				} else {
					Mods.push_back(XAieMod(Dev, L, XAIE_MEM_MOD));
				}
			}
			AieHandle = Dev.getDevHandle();
		}
		~XAieTile() {}

		/**
		 * This function returns tile location
		 *
		 * @return tile location
		 */
		XAie_LocType loc() const {
			return Loc;
		}

		/**
		 * This function returns module reference.
		 *
		 * @param Mod module type
		 * @return module reference
		 */
		XAieMod &module (XAie_ModuleType Mod) {
			if (_XAie_CheckModule(AieHandle->dev(), Loc, Mod) !=
				XAIE_OK) {
				std::string str = "module " +
					std::to_string(Mod) + " not in tile.";

				throw std::invalid_argument(str);
			}
			if (Mod == XAIE_CORE_MOD) {
				return Mods[1];
			} else {
				return Mods[0];
			}
		}

		/**
		 * This function returns core module reference.
		 *
		 * @return core module reference
		 */
		XAieMod &core() {
			return module(XAIE_CORE_MOD);
		}

		/**
		 * This function returns memory module reference.
		 *
		 * @return memory module reference
		 */
		XAieMod &mem() {
			return module(XAIE_MEM_MOD);
		}

		/**
		 * This function returns pl module reference.
		 *
		 * @return pl module reference
		 */
		XAieMod &pl() {
			return module(XAIE_PL_MOD);
		}
	private:
		std::shared_ptr<XAieDevHandle> AieHandle; /**< AI engine device
							    handle */
		XAie_LocType Loc;
		std::vector<XAieMod> Mods;
	};

	inline XAieDev::XAieDev(XAie_DevInst *DevPtr,
		bool TurnOnFinishOnDestruct) {

		AieHandle = std::make_shared<XAieDevHandle>(DevPtr,
			TurnOnFinishOnDestruct);

		NumCols = DevPtr->NumCols;
		NumRows = DevPtr->NumRows;
		for (uint32_t c = 0; c < NumCols; c++) {
			for (uint32_t r = 0; r < NumRows; r++) {
				Tiles.push_back(XAieTile(*this,
					XAie_TileLoc(c, r)));
			}
		}
	}
}
