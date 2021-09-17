// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
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
	 * @class XAieComboEvent
	 * @brief AI engine combo event resource class
	 */
	class XAieComboEvent: public XAieSingleTileRsc {
	public:
		XAieComboEvent() = delete;
		XAieComboEvent(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M, uint32_t ENum = 2):
			XAieSingleTileRsc(DevHd, L, M) {
			if (ENum > 4 || ENum < 2) {
				throw std::invalid_argument("Combo event failed, invalid input events number");
			}
			vEvents.resize(ENum);
			State.Initialized = 1;
		}
		XAieComboEvent(XAieDev &Dev,
			XAie_LocType L, XAie_ModuleType M, uint32_t ENum = 2):
			XAieComboEvent(Dev.getDevHandle(), L, M, ENum) {}
		/**
		 * This function sets input events, and combo operations.
		 *
		 * @param vE vector of input events.Minum 2 events, maximum 4 events
		 *	vE[0] for Event0, vE[1] for Event1,
		 *	vE[2] for Event2, vE[3] for Event3
		 * @param vOp vector of combo operations
		 *	vOp[0] for combo operation for Event0 and Event1
		 *	vOp[1] for combo operation for Event2 and Event3
		 *	vOp[2] for combo operation for (Event0,Event1) and
		 *		(Event2,Event3)
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setEvents(const std::vector<XAie_Events> &vE,
				const std::vector<XAie_EventComboOps> &vOp) {
			AieRC RC;

			if (State.Initialized == 0) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " not initialized with Mod and num of events." << std::endl;
				RC = XAIE_ERR;
			} else if ((vE.size() != vEvents.size()) || (vOp.size() > 3) ||
			    (vE.size() <= 2 && vOp.size() > 1) ||
			    (vE.size() > 2 && vOp.size() < 2)) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " invalid number of input events and ops." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				for (int i = 0; i < (int)vE.size(); i++) {
					uint8_t HwEvent;

					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
							Mod, vE[i], &HwEvent);
					if (RC != XAIE_OK) {
						Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
							(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
							" Mod=" << Mod <<  " invalid E=" << vE[i] << std::endl;
						break;
					} else {
						vEvents[i] = vE[i];
					}
				}
				if (RC == XAIE_OK) {
					vOps.clear();
					for (int i = 0; i < (int)vOp.size(); i++) {
						vOps.push_back(vOp[i]);
					}
					State.Configured = 1;
				}
			}
			return RC;
		}
		/**
		 * This function returns combo events for the input combination.
		 *
		 * @param vE combo events vector
		 *	vE[0] for combination of input events Event0, Event1
		 *	vE[1] for combination of input events Event2, Event3
		 *	vE[2] for combination of input events (Event0,Event1)
		 *		and (Event2,Event3)
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getEvents(std::vector<XAie_Events> &vE) {
			AieRC RC;

			(void)vE;
			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " resource is not reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				XAie_Events BaseEvent;
				XAie_EventGetComboEventBase(dev(), Loc, Mod, &BaseEvent);
				vE.clear();
				if (vOps.size() == 1) {
					vE.push_back((XAie_Events)((uint32_t)BaseEvent + vRscs[0].RscId));
				} else {
					for (uint32_t i = 0; i < (uint32_t)vOps.size(); i++) {
						vE.push_back((XAie_Events)((uint32_t)BaseEvent + i));
					}
				}
				RC = XAIE_OK;
			}
			return RC;
		}

		AieRC getInputEvents(std::vector<XAie_Events> &vE,
				std::vector<XAie_EventComboOps> &vOp) {
			AieRC RC;

			if (State.Configured == 1) {
				vE.clear();
				for (int i = 0; i < (int)vEvents.size(); i++) {
					vE.push_back(vEvents[i]);
				}
				vOp.clear();
				for (int i = 0; i < (int)vOps.size(); i++) {
					vOp.push_back(vOps[i]);
				}
				RC = XAIE_OK;
			} else {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " no input events specified." << std::endl;
				RC = XAIE_ERR;
			}
			return RC;
		}
		uint32_t getRscType() const {
			return static_cast<uint32_t>(XAIE_COMBO_EVENTS_RSC);
		}
	protected:
		std::vector<XAie_Events> vEvents; /**< input events */
		std::vector<XAie_EventComboOps> vOps; /**< combo operations */
		std::vector<XAie_UserRsc> vRscs; /**< combo events resources */
	private:
		AieRC _reserve() {
			AieRC RC;

			for (uint32_t i = 0; i < vEvents.size(); i++) {
				XAie_UserRsc Rsc;
				vRscs.push_back(Rsc);
			}

			XAie_UserRscReq Req = {Loc, Mod, static_cast<uint32_t>(vEvents.size())};
			RC = XAie_RequestComboEvents(AieHd->dev(), 1, &Req, vEvents.size(), &vRscs[0]);
			if (RC != XAIE_OK) {
				vRscs.clear();
				return RC;
			}

			Rsc.Mod = vRscs[0].Mod;
			if (vRscs.size() <= 2) {
				// Only two input events, it can be combo0 or
				// combo1
				if (vRscs[0].RscId < 2) {
					Rsc.RscId = 0;
				} else {
					Rsc.RscId = 1;
				}
			} else {
				Rsc.RscId = 2;
			}
			return RC;
		}
		AieRC _release() {
			XAie_ReleaseComboEvents(AieHd->dev(), vRscs.size(), &vRscs[0]);
			vRscs.clear();
			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;
			XAie_EventComboId StartCId;

			for (uint32_t i = 0 ; i < vEvents.size(); i += 2) {
				XAie_EventComboId ComboId;

				if (vRscs[i].RscId == 0) {
					ComboId = XAIE_EVENT_COMBO0;
				} else {
					ComboId = XAIE_EVENT_COMBO1;
				}
				if (i == 0) {
					StartCId = ComboId;
				}
				RC = XAie_EventComboConfig(dev(), Loc, Mod,
						ComboId, vOps[i/2], vEvents[i], vEvents[i+1]);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
						" Mod=" << Mod <<  " failed to config combo " << ComboId << std::endl;
					for (XAie_EventComboId tId = StartCId;
						tId < ComboId;
						tId = (XAie_EventComboId)((int)tId + i)) {
						XAie_EventComboReset(dev(), Loc, Mod,
								tId);
					}
					break;
				}
			}
			if (RC == XAIE_OK && vOps.size() == 3) {
				RC = XAie_EventComboConfig(dev(), Loc, Mod,
					XAIE_EVENT_COMBO2, vOps[2], vEvents[0], vEvents[0]);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
						" Mod=" << Mod <<  " failed to config combo " << XAIE_EVENT_COMBO2 << std::endl;
				}
			}
			return RC;
		}
		AieRC _stop() {
			XAie_EventComboId ComboId;

			for (int i = 0; i < (int)vOps.size(); i++) {
				if (i == 0) {
					if (vRscs[i].RscId == 0) {
						ComboId = XAIE_EVENT_COMBO0;
					} else {
						ComboId = XAIE_EVENT_COMBO1;
					}
					XAie_EventComboReset(dev(), Loc, Mod, ComboId);
					ComboId = (XAie_EventComboId)((uint32_t)ComboId + 1);
				}
			}
			return XAIE_OK;
		}
		void _getRscs(std::vector<XAie_UserRsc> &vRs) const {
			vRs.insert(vRs.end(), vRscs.begin(), vRscs.end());
		}
	};

	/**
	 * @class XAieUserEvent
	 * @brief AI engine user event resource class
	 */
	class XAieUserEvent: public XAieSingleTileRsc {
	public:
		XAieUserEvent() = delete;
		XAieUserEvent(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M):
			XAieSingleTileRsc(DevHd, L, M) {
			State.Initialized = 1;
			State.Configured = 1;
		}
		XAieUserEvent(XAieDev &Dev,
			XAie_LocType L, XAie_ModuleType M):
			XAieUserEvent(Dev.getDevHandle(), L, M) {}

		/**
		 * This function returns user event.
		 * It needs to be called after reserve() succeeds.
		 *
		 * @param E returns user event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getEvent(XAie_Events &E) const {
			AieRC RC = XAIE_OK;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "User Event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" resource not resesrved." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				E = _getEventFromId(Rsc.RscId);
			}
			return RC;
		}
		uint32_t getRscType() const {
			return static_cast<uint32_t>(XAIE_USER_EVENTS_RSC);
		}
	private:
		AieRC _reserve() {
			AieRC RC;

			if (preferredId == XAIE_RSC_ID_ANY) {
				XAie_UserRscReq Req = {Loc, Mod, 1};

				RC = XAie_RequestUserEvents(AieHd->dev(), 1, &Req, 1, &Rsc);
				if (RC == XAIE_OK) {
					Rsc.RscId = _getIdFromEvent(static_cast<XAie_Events>(Rsc.RscId));
				}
			} else {
				Rsc.RscType = XAIE_USER_EVENTS_RSC;
				Rsc.Loc.Col = Loc.Col;
				Rsc.Loc.Row = Loc.Row;
				Rsc.Mod = Mod;
				Rsc.RscId = _getEventFromId(preferredId);
				RC = XAie_RequestAllocatedUserEvents(AieHd->dev(), 1, &Rsc);
				if (RC == XAIE_OK) {
					Rsc.RscId = preferredId;
				}
			}

			return RC;
		}
		AieRC _release() {
			uint32_t RscId = Rsc.RscId;
			AieRC RC;

			Rsc.RscId = _getEventFromId(Rsc.RscId);
			RC = XAie_ReleaseUserEvents(AieHd->dev(), 1, &Rsc);
			Rsc.RscId = RscId;

			return RC;
		}
		AieRC _start() {
			// As no hardware config is required for user event
			// always succeeds.
			return XAIE_OK;
		}
		AieRC _stop() {
			// As no hardware config is required for user event
			// always succeeds.
			return XAIE_OK;
		}

		/**
		 * This function returns user event resource id from user event.
		 *
		 * @param E user event
		 * @return user event resource id
		 */
		uint32_t _getIdFromEvent(XAie_Events E) const {
			XAie_Events UserEventStart;
			XAie_EventGetUserEventBase(AieHd->dev(), Loc, Mod, &UserEventStart);

			return static_cast<uint32_t>(E - UserEventStart);
		}

		/**
		 * This function returns user event from resource id.
		 *
		 * @param I resource ID
		 * @return user event
		 */
		XAie_Events _getEventFromId(uint32_t I) const {
			XAie_Events UserEventStart;
			XAie_EventGetUserEventBase(AieHd->dev(), Loc, Mod, &UserEventStart);

			return static_cast<XAie_Events>(static_cast<uint32_t>(UserEventStart) + I);
		}
	};
}
