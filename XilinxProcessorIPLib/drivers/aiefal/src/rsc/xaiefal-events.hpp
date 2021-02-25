// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
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
	class XAieComboEvent: public XAieRsc {
	public:
		XAieComboEvent() = delete;
		XAieComboEvent(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L):
			XAieRsc(Dev), Loc(L) {}
		XAieComboEvent(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L,
			XAie_ModuleType M):
			XAieComboEvent(Dev, L) {
			Mod = M;
			if (_XAie_CheckModule(Aie->dev(), Loc, Mod) == XAIE_OK) {
				vEvents.resize(2);
				State.Initialized = 1;
			}
		}
		/**
		 * This function sets the module and number of input events.
		 *
		 * @param M module
		 * @param ENum number of input events it. (2 to 4)
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC initialize(XAie_ModuleType M, uint32_t ENum) {
			AieRC RC;

			(void)M;
			if (State.Reserved == 1) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " already reserved." << std::endl;
				RC = XAIE_ERR;
			} else if (ENum < 2 || ENum > 4) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" invalid number of events." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				RC = _XAie_CheckModule(Aie->dev(), Loc, M);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
						" set Mod=" << M <<  " invalid tile module combination." << std::endl;
					RC = XAIE_INVALID_ARGS;
				} else {
					vEvents.resize(ENum);
					State.Initialized = 1;
					State.Configured = 1;
					RC = XAIE_OK;
				}
			}
			return RC;
		}
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
			    (vE.size() > 2 && vOps.size() < 2)) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " invalid number of input events and ops." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				for (int i = 0; i < (int)vE.size(); i++) {
					uint8_t HwEvent;

					RC = XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc,
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
				XAie_Events BaseE;
				uint32_t TType = _XAie_GetTileTypefromLoc(Aie->dev(), Loc);

				if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
					if (Mod == XAIE_CORE_MOD) {
						BaseE = XAIE_EVENT_COMBO_EVENT_0_CORE;
					} else {
						BaseE = XAIE_EVENT_COMBO_EVENT_0_MEM;
					}
				} else {
					BaseE = XAIE_EVENT_COMBO_EVENT_0_PL;
				}
				vE.clear();
				if (vOps.size() == 1) {
					vE.push_back((XAie_Events)((uint32_t)BaseE + vRscs[0].RscId));
				} else {
					for (uint32_t i = 0; i < (uint32_t)vOps.size(); i++) {
						vE.push_back((XAie_Events)((uint32_t)BaseE + i));
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
	protected:
		XAie_LocType Loc; /**< tile location */
		XAie_ModuleType Mod; /**< module */
		std::vector<XAie_Events> vEvents; /**< input events */
		std::vector<XAie_EventComboOps> vOps; /**< combo operations */
		std::vector<XAie_UserRsc> vRscs; /**< combo events resources */
	private:
		AieRC _reserve() {
			return XAIE_OK;
		}
		AieRC _release() {
			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;
			XAie_EventComboId StartCId;

			for (int i = 0 ; i < (int)vEvents.size(); i += 2) {
				XAie_EventComboId ComboId;

				if (vRscs[i].RscId == 0) {
					ComboId = XAIE_EVENT_COMBO0;
				} else {
					ComboId = XAIE_EVENT_COMBO1;
				}
				if (i == 0) {
					StartCId = ComboId;
				}
				RC = XAie_EventComboConfig(Aie->dev(), Loc, Mod,
						ComboId, vOps[i/2], vEvents[i], vEvents[i+1]);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
						" Mod=" << Mod <<  " failed to config combo " << ComboId << std::endl;
					for (XAie_EventComboId tId = StartCId;
						tId < ComboId;
						tId = (XAie_EventComboId)((int)tId + i)) {
						XAie_EventComboReset(Aie->dev(), Loc, Mod,
								tId);
					}
					break;
				}
			}
			if (RC == XAIE_OK && vOps.size() == 3) {
				RC = XAie_EventComboConfig(Aie->dev(), Loc, Mod,
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
					XAie_EventComboReset(Aie->dev(), Loc, Mod, ComboId);
					ComboId = (XAie_EventComboId)((uint32_t)ComboId + 1);
				}
			}
			return XAIE_OK;
		}
	};
}
