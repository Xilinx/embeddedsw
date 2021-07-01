// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @param file xaiefal-rsc-base.hpp
 * Base classes for AI engine resources management
 */

#include <fstream>
#include <functional>
#include <map>
#include <unordered_map>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/common/xaiefal-common.hpp>
#include <xaiefal/common/xaiefal-log.hpp>

#pragma once

namespace xaiefal {
	class XAieRsc;
	class XAieRscGetRscsWrapper;

	enum XAieFalRscType {
		XAIE_TRACE_EVENTS_RSC = 0x100U,
	};

	/**
	 * @struct XAieRscStat
	 * @brief struct of resource usage statistics
	 */
	struct XAieRscStat {
		XAieRscStat(const std::string &Name = ""):
			GroupName(Name) {}
		std::string GroupName; /**< name of the group of resources */
		std::map<std::tuple<uint8_t, uint8_t, uint32_t, uint32_t>,
			uint32_t> Rscs; /**< number of resources info:
					       * key: col, row, mod type, rsc type>
					       * value: numer of resources
					       */
		/**
		 * This function adds resource information to the resources structure.
		 * @param Loc tile location
		 * @param Mod module type
		 * @param RscType resource type
		 * @param NumRscs number of resources of the resource type
		 */
		void addRscStat(XAie_LocType Loc, uint32_t Mod, uint32_t RscType,
			uint32_t NumRscs) {
			(void)Mod;
			(void)RscType;
			(void)NumRscs;

			std::tuple<uint8_t, uint8_t, uint32_t, uint32_t> rKey {
				Loc.Col, Loc.Row, Mod, RscType};

			if (Rscs.find(rKey) != Rscs.end()) {
				Rscs[rKey] += NumRscs;
			} else {
				Rscs[rKey] = NumRscs;
			}
		}

		/**
		 * This function returns number of resource of a type of resources
		 * of a module of a tile in this resource stat structure.
		 * @param Loc tile location
		 * @param Mod module type
		 * @param RscType resource type
		 *
		 * @return number of resources of the type of resources of a module
		 *	of a tile.
		 */
		uint32_t getNumRsc(XAie_LocType Loc, uint32_t Mod, uint32_t RscType) {
			uint32_t NumRscs = 0;
			std::tuple<uint8_t, uint8_t, uint32_t, uint32_t> rKey {
				Loc.Col, Loc.Row, Mod, RscType};

			if (Rscs.find(rKey) != Rscs.end()) {
				NumRscs =  Rscs[rKey];
			}

			return NumRscs;
		}

		/**
		 * This function checkes if the resources stat map has resources
		 *
		 * @return true if the resources stat map is not empty, otherwise
		 *	false.
		 */
		bool hasRsc() const {
			return (Rscs.size() == 0) ? false : true;
		}

		/**
		 * This function shows the resources information of this resource group
		 */
		void show() const {
			Logger::log(LogLevel::INFO) << GroupName << ":" << std::endl;
			for (auto const& r : Rscs) {
				std::string Str = "\t(" +
					std::to_string(static_cast<uint32_t>(std::get<0>(r.first))) +
					", " +
					std::to_string(static_cast<uint32_t>(std::get<1>(r.first))) +
					"): ";

				switch (std::get<2>(r.first)) {
				case XAIE_MEM_MOD:
					Str += "Mem";
					break;
				case XAIE_CORE_MOD:
					Str += "Core";
					break;
				case XAIE_PL_MOD:
					Str += "SHIM";
					break;
				default:
					Str += "Unknown";
					break;
				}
				Str += ": ";

				switch (std::get<3>(r.first)) {
				case static_cast<uint32_t>(XAIE_PERFCNT_RSC):
					Str += "Perfcount ";
					break;
				case static_cast<uint32_t>(XAIE_USER_EVENTS_RSC):
					Str += "UserEvents ";
					break;
				case static_cast<uint32_t>(XAIE_TRACE_CTRL_RSC):
					Str += "TraceCntr ";
					break;
				case static_cast<uint32_t>(XAIE_PC_EVENTS_RSC):
					Str += "PCEvents ";
					break;
				case static_cast<uint32_t>(XAIE_SS_EVENT_PORTS_RSC):
					Str += "SSEventsPorts ";
					break;
				case static_cast<uint32_t>(XAIE_BCAST_CHANNEL_RSC):
					Str += "BC ";
					break;
				case static_cast<uint32_t>(XAIE_COMBO_EVENTS_RSC):
					Str += "ComboEvents ";
					break;
				case static_cast<uint32_t>(XAIE_GROUP_EVENTS_RSC):
					Str += "GroupEvents ";
					break;
				case static_cast<uint32_t>(XAIE_TRACE_EVENTS_RSC):
					Str += "TraceEvents ";
					break;
				default:
					Str += "Unknown ";
					break;
				}
				Str += ": " + std::to_string(r.second);
				std::cout << Str << std::endl;
			}
		}
	};

	/**
	 * @class XAieRscGroupBase
	 * @brief base class to resources functional group
	 */
	class XAieRscGroupBase {
	public:
		XAieRscGroupBase(const std::string &Name = ""):
			FuncName(Name) {}
		~XAieRscGroupBase() {}

		/**
		 * This function is to return resources statics of this function
		 * group.
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat() const {
			return getRscStat(XAie_TileLoc(XAIE_LOC_ANY, XAIE_LOC_ANY));
		}
		/**
		 * This function is to return resources statics of this function
		 * group of a specific tile.
		 *
		 * @Loc tile location
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(XAie_LocType Loc) const {
			return _getRscStatTile(Loc, XAIE_MOD_ANY,
					XAIE_RSC_TYPE_ANY, XAIE_RSC_ID_ANY);
		}
		/**
		 * This function is to return resources statics of this function
		 * group of a specific tile of a specific resource type.
		 *
		 * @Loc tile location
		 * @RscType resource type
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(XAie_LocType Loc, uint32_t RscType) const {
			return _getRscStatTile(Loc, XAIE_MOD_ANY,
					RscType, XAIE_RSC_ID_ANY);
		}
		/**
		 * This function is to return resources statics of this function
		 * group of a specific tile, specific module.
		 *
		 * @Loc tile location
		 * @Mod module type
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(XAie_LocType Loc, XAie_ModuleType Mod) const {
			return getRscStat(Loc, Mod, XAIE_RSC_TYPE_ANY);
		}
		/**
		 * This function is to return resources statics of this function
		 * group of a specific tile, specific module, specific resource.
		 *
		 * @Loc tile location
		 * @Mod module type
		 * @RscType resource type
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(XAie_LocType Loc, XAie_ModuleType Mod,
			uint32_t RscType) const {
			return getRscStat(Loc, Mod, RscType, XAIE_RSC_ID_ANY);
		}

		/**
		 * This function is to return resources statics of this function
		 * group of a specific tile, specific module, specific resource.
		 *
		 * @Loc tile location
		 * @Mod module type
		 * @RscType resource type
		 * @RscId resource Id
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(XAie_LocType Loc, XAie_ModuleType Mod,
			uint32_t RscType, uint32_t RscId) const {
			return _getRscStatTile(Loc, static_cast<uint32_t>(Mod),
					RscType, RscId);
		}

		/**
		 * This function is to return resources statics of this function
		 * group of specific tiles, specific resource.
		 *
		 * @vLocs tile locations
		 * @RscType resource type
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(const std::vector<XAie_LocType> &vLocs,
			uint32_t RscType) const {
			return _getRscStat(vLocs, XAIE_MOD_ANY, RscType, XAIE_RSC_ID_ANY);
		}

		/**
		 * This function is to return resources statics of this function
		 * group of specific tiles, specific module, specific resource, specific
		 * resource id.
		 *
		 * @vLocs tile locations
		 * @RscType resource type
		 * @RscId resource Id
		 *
		 * @return resoruce group statics object.
		 */
		XAieRscStat getRscStat(const std::vector<XAie_LocType> &vLocs,
			uint32_t RscType, uint32_t RscId) const {
			return _getRscStat(vLocs, XAIE_MOD_ANY, RscType, RscId);
		}

		virtual AieRC addRsc(std::shared_ptr<XAieRsc> R) {
			(void)R;
			throw std::invalid_argument("Add Rsc not supported, rsc group: " + FuncName);
			return XAIE_ERR;
		}
	protected:
		std::string FuncName; /**< function name of this group */

		/**
		 * This function is to return resources statics of this function
		 * group.
		 *
		 * @vLocs tile locations, (0xFF, 0xFF) for any tile
		 * @Mod module type, 0xFFFFFFFF for any module
		 * @RscType, resource type, 0xFFFFFFFF for any resource type
		 * @RscId, resource Id, 0xFFFFFFFF for any resource id
		 *
		 * @return resoruce group statics object.
		 */
		virtual XAieRscStat _getRscStat(const std::vector<XAie_LocType> &vLocs,
				uint32_t Mod, uint32_t RscType,
				uint32_t RscId) const {
			XAieRscStat RscStat(FuncName);

			(void)vLocs;
			(void)Mod;
			(void)RscType;
			(void)RscId;

			throw std::invalid_argument("Get rsc stat not supported, rsc group: " + FuncName);
			return RscStat;
		}
		XAieRscStat _getRscStatTile(XAie_LocType Loc, uint32_t Mod,
			uint32_t RscType, uint32_t RscId) const {
			std::vector<XAie_LocType> vLocs;

			vLocs.push_back(Loc);
			return _getRscStat(vLocs, Mod, RscType, RscId);
		}
	};

	/**
	 * @class XAieRscGroupRuntime
	 * @brief class to runtime resources functional group
	 * Each element in the group is a resource.
	 */
	class XAieRscGroupRuntime : public XAieRscGroupBase {
	public:
		XAieRscGroupRuntime(const std::string &Name = ""):
			XAieRscGroupBase(Name) {}
		~XAieRscGroupRuntime() {}

		/**
		 * This function adds a resource to the resource group.
		 *
		 * @param  AI engine resource shared pointer
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC addRsc(std::shared_ptr<XAieRsc> R) {
			bool toAdd = true;
			std::vector<std::weak_ptr<XAieRsc>>::iterator it;

			for (it = vRefs.begin(); it != vRefs.end();) {
				auto lR = it->lock();

				if (lR == nullptr) {
					vRefs.erase(it);
					continue;
				}
				it++;
				if (lR == R) {
					toAdd = false;
					break;
				}
			}
			if (toAdd) {
				vRefs.push_back(R);
			}
			return XAIE_OK;
		}
	private:
		std::vector<std::weak_ptr<XAieRsc>> vRefs; /**< vector of AI engine resource objects */

		XAieRscStat _getRscStat(const std::vector<XAie_LocType> &vLocs,
				uint32_t Mod, uint32_t RscType,
				uint32_t RscId) const {
			XAieRscStat RscStat(FuncName);
			std::vector<XAie_UserRsc> vRscs;

			for (auto Ref: vRefs) {
				auto R = Ref.lock();

				if (R == nullptr) {
					continue;
				}

				auto RscWrapper = std::make_shared<
					XAieRscGetRscsWrapper>(R, vRscs);
			}

			for (auto R: vRscs) {
				for (auto L: vLocs) {
					if ((L.Col != XAIE_LOC_ANY && R.Loc.Col != L.Col) ||
						(L.Row != XAIE_LOC_ANY && R.Loc.Row != L.Row) ||
						(Mod != XAIE_MOD_ANY && R.Mod != Mod) ||
						(RscType != XAIE_RSC_TYPE_ANY && R.RscType != RscType) ||
						(RscId != XAIE_RSC_ID_ANY && R.RscId != RscId)) {
						continue;
					}
					RscStat.addRscStat(R.Loc, R.Mod, R.RscType, 1);
				}
			}

			return RscStat;
		}
	};

	/**
	 * @class XAieRscGroupStatic
	 * @brief class to statically allocated resources group
	 * Each element in the group is a resource.
	 */
	class XAieRscGroupStatic : public XAieRscGroupBase {
	public:
		XAieRscGroupStatic(): XAieRscGroupBase("Static") {};
		XAieRscGroupStatic(const std::string &Name = ""):
			XAieRscGroupBase(Name) {}
		~XAieRscGroupStatic() {}
	private:
		XAieRscStat _getRscStat(const std::vector<XAie_LocType> &vLocs,
				uint32_t Mod, uint32_t RscType,
				uint32_t RscId) const {
			XAieRscStat RscStat(FuncName);

			(void)vLocs;
			(void)Mod;
			(void)RscType;
			(void)RscId;
			/* TODO: get static resources from driver */
			return RscStat;
		}
	};

	/**
	 * @class XAieRscGroupAvail
	 * @brief class to runtime resources functional group
	 * Each element in the group is a resource.
	 */
	class XAieRscGroupAvail : public XAieRscGroupBase {
	public:
		XAieRscGroupAvail(): XAieRscGroupBase("Avail") {};
		XAieRscGroupAvail(const std::string &Name = ""):
			XAieRscGroupBase(Name) {}
	private:
		XAieRscStat _getRscStat(const std::vector<XAie_LocType> &vLocs,
				uint32_t Mod, uint32_t RscType,
				uint32_t RscId) const {
			XAieRscStat RscStat(FuncName);

			(void)vLocs;
			(void)Mod;
			(void)RscType;
			(void)RscId;
			/* TODO: get available resources from driver */
			return RscStat;
		}
	};
}
