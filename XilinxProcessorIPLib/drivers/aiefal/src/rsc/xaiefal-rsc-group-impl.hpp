// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @param file xaiefal-rsc-base.hpp
 * Base classes for AI engine resources management
 */

#include <vector>
#include <xaiengine.h>
#include <xaiefal/rsc/xaiefal-rsc-group.hpp>
#include <xaiefal/common/xaiefal-base.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAieRscGroupRuntime
	 * @brief class to runtime resources functional group
	 * Each element in the group is a resource.
	 */
	class XAieRscGroupRuntime : public XAieRscGroupBase {
	public:
		XAieRscGroupRuntime(std::shared_ptr<XAieDevHandle> DevHd,
				const std::string &Name = ""):
			XAieRscGroupBase(DevHd, Name) {}
		XAieRscGroupRuntime() {}
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
		XAieRscGroupStatic(std::shared_ptr<XAieDevHandle> DevHd,
				const std::string &Name = "Static"):
			XAieRscGroupBase(DevHd, Name) {}
		XAieRscGroupStatic() {};
		~XAieRscGroupStatic() {}
	private:
		XAieRscStat _getRscStat(const std::vector<XAie_LocType> &vLocs,
				uint32_t Mod, uint32_t RscType,
				uint32_t RscId) const {
			XAieRscStat RscStat(FuncName);
			std::vector<XAie_UserRscStat> RscStats;

			(void)RscId;

			if (RscType == static_cast<uint32_t>(XAIE_TRACE_EVENTS_RSC)) {
				// Trace events are only allocated at runtime by AIEFAL
				return RscStat;
			}

			auto AieHdPtr = AieHd.lock();
			if (AieHdPtr == nullptr) {
				throw std::invalid_argument("failed to get Aie ptr to get rsc stat for " +
						FuncName);
			}

			if (vLocs[0].Col == XAIE_LOC_ANY) {
				// Any tiles in the AI engine partition
				auto lvLocs = _getAllTilesLocs(AieHdPtr->dev());
				RscStats = _createDrvRscStats(AieHdPtr->dev(),
						lvLocs, Mod, RscType);
			} else {
				RscStats = _createDrvRscStats(AieHdPtr->dev(),
						vLocs, Mod, RscType);
			}

			if (XAie_GetStaticRscStat(AieHdPtr->dev(),
				RscStats.size(), RscStats.data())) {
				Logger::log(LogLevel::ERROR) << "failed to get static resource stat." << std::endl;
			} else {
				for (auto S: RscStats) {
					if (S.NumRscs != 0) {
						RscStat.addRscStat(S.Loc, S.Mod, S.RscType, S.NumRscs);
					}
				}
			}

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
		XAieRscGroupAvail(std::shared_ptr<XAieDevHandle> DevHd,
				const std::string &Name = "Avail"):
			XAieRscGroupBase(DevHd, Name) {}
		XAieRscGroupAvail() {}

		/**
		 * This function adds a resource to the resource group.
		 *
		 * @param  AI engine resource shared pointer
		 * @return XAIE_OK for success, error code for failure.
		 *
		 * Only the resources availability should should be managed by
		 * AIEFAL should be added to this group. Only trace controller
		 * should be added to this groups, for other resources, it will
		 * go to the lower level driver to get the resource
		 * availability.
		 */
		AieRC addRsc(std::shared_ptr<XAieRsc> R) {
			bool toAdd = true;
			std::vector<std::weak_ptr<XAieRsc>>::iterator it;

			if (R->getRscType() != static_cast<uint32_t>(XAIE_TRACE_CTRL_RSC)) {
				throw std::invalid_argument(
					"failed to add rsc to avail group, only trace controller is allowed");
			}
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
		std::vector<std::weak_ptr<XAieRsc>> vRefs; /**< vector of AI engine resource objects which
								manage the resource availability in AIEFAL
							    */
	private:
		XAieRscStat _getRscStat(const std::vector<XAie_LocType> &vLocs,
				uint32_t Mod, uint32_t RscType,
				uint32_t RscId) const {
			std::vector<XAie_UserRscStat> RscStats;
			XAieRscStat RscStat(FuncName);
			uint32_t lRscType = RscType;

			(void)RscId;
			auto AieHdPtr = AieHd.lock();
			if (AieHdPtr == nullptr) {
				throw std::invalid_argument("failed to get Aie ptr to get rsc stat for " +
						FuncName);
			}

			if (RscType == static_cast<uint32_t>(XAIE_TRACE_EVENTS_RSC)) {
				// If user wants to know the trace events availability,
				// we also needs to know the trace control availability as
				// the trace control is managed by the lower level driver
				// while the trace events avaialability is managed by
				// the AIEFAL.
				lRscType = static_cast<uint32_t>(XAIE_TRACE_CTRL_RSC);
			}

			if (vLocs[0].Col == XAIE_LOC_ANY) {
				// Any tiles in the AI engine partition
				auto lvLocs = _getAllTilesLocs(AieHdPtr->dev());
				RscStats = _createDrvRscStats(AieHdPtr->dev(),
						lvLocs, Mod, lRscType);
			} else {
				RscStats = _createDrvRscStats(AieHdPtr->dev(),
						vLocs, Mod, lRscType);
			}

			if (XAie_GetAvailRscStat(AieHdPtr->dev(),
				RscStats.size(), RscStats.data())) {
				Logger::log(LogLevel::ERROR) << "failed to get avail resource stat." << std::endl;
			} else {
				for (auto S: RscStats) {
					if (S.NumRscs == 0) {
						continue;
					}
					RscStat.addRscStat(S.Loc, S.Mod, S.RscType, S.NumRscs);
				}
			}

			/* Handle resoure whose availability is managed by AIE FAL layer */
			for (auto Ref: vRefs) {
				uint32_t NumRscs;
				XAie_LocType Loc;
				uint32_t Mod;
				uint32_t ManagedRscType;

				auto R = Ref.lock();
				if (R == nullptr) {
					continue;
				}

				ManagedRscType = R->getManagedRscsType();
				if (RscType != XAIE_RSC_TYPE_ANY &&
					ManagedRscType != RscType) {
					continue;
				}
				NumRscs = R->getAvailManagedRscs();
				if (NumRscs == 0) {
					continue;
				}

				Loc = std::static_pointer_cast<XAieSingleTileRsc>(R)->loc();
				Mod = std::static_pointer_cast<XAieSingleTileRsc>(R)->mod();
				if (RscStat.getNumRsc(Loc, Mod, R->getRscType()) == 0 &&
					R->isReserved() == false) {
					// If the parent resource is occupied
					// but not by the current app, number of
					// AIEFAL managed child resources is not
					// available.
					continue;
				}
				RscStat.addRscStat(std::static_pointer_cast<XAieSingleTileRsc>(R)->loc(),
						std::static_pointer_cast<XAieSingleTileRsc>(R)->mod(),
						ManagedRscType, NumRscs);
			}
			return RscStat;
		}
	};
}
