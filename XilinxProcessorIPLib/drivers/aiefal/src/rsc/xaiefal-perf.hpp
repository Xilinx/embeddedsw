// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <map>
#include <string.h>
#include <vector>
#include <xaiengine.h>

#include <xaiefal/rsc/xaiefal-bc.hpp>
#include <xaiefal/rsc/xaiefal-rsc-base.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAiePerfCounter
	 * @brief class for Perfcounter resource.
	 * Each perfcounter resources contains the resource ID of in a module of
	 * a tile, start, stop, and reset events, and counter value for the
	 * counter event.
	 * If there is not enough perfcounter of the specified module, it will
	 * check if there is counter available from the adjacent module if it is
	 * core tile. If there is available counter in the adjacent module of
	 * the same tile, broadcast channels within the tile of the
	 * start/stop/reset events will be reserved.
	 */
	class XAiePerfCounter : public XAieSingleTileRsc {
	public:
		XAiePerfCounter() = delete;
		XAiePerfCounter(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M,
			bool CrossM = false, uint32_t Threshold = 0):
			XAieSingleTileRsc(DevHd, L, M), CrossMod(CrossM) {
			StartMod = Mod;
			StopMod = Mod;
			RstMod = Mod;
			RstEvent = XAIE_EVENT_NONE_CORE;
			State.Initialized = 1;
			EventVal = Threshold;
		}
		XAiePerfCounter(XAieDev &Dev,
			XAie_LocType L, XAie_ModuleType M,
			bool CrossM = false):
			XAiePerfCounter(Dev.getDevHandle(), L, M, CrossM) {}
		~XAiePerfCounter() {
			if (State.Reserved == 1) {
				if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					delete StartBC;
				}
				if (StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					delete StopBC;
				}
				if (RstEvent != XAIE_EVENT_NONE_CORE &&
					RstMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					delete RstBC;
				}
			}
		}
		/**
		 * This function sets the perfcounter start, stop, reset events.
		 *
		 * @param StartM start event module type
		 * @param StartE start event
		 * @param StopM stop event module type
		 * @param StopE stop event
		 * @param RstM reset event module type
		 * @param RstE reset event, default is XAIE_EVENT_NONE_CORE,
		 *	       that is no reset event.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC initialize(XAie_ModuleType StartM, XAie_Events StartE,
				 XAie_ModuleType StopM, XAie_Events StopE,
				 XAie_ModuleType RstM = XAIE_CORE_MOD, XAie_Events RstE = XAIE_EVENT_NONE_CORE) {
			AieRC RC;

			if (State.Reserved == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource reserved." << std::endl;
				return XAIE_ERR;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
					StartM, StartE, &HwEvent);
				if (RC == XAIE_OK) {
					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						StopM, StopE, &HwEvent);
				}
				if (RC == XAIE_OK && RstE != XAIE_EVENT_NONE_CORE) {
					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						RstM, RstE, &HwEvent);
				}
				if (RC == XAIE_OK) {
					StartEvent = StartE;
					StopEvent = StopE;
					RstEvent = RstE;
					StartMod = StartM;
					StopMod = StopM;
					if (RstE != XAIE_EVENT_NONE_CORE) {
						RstMod = RstM;
					}
					State.Initialized = 1;
					State.Configured = 1;
				} else {
					Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
						" Expect Mod= " << Mod <<
						" StartEvent=" <<StartM << "," << StartE << " " <<
						" StopEvent=" <<StopM << "," << StopE << " " <<
						" RstEvent=" <<RstM << "," << RstE << " " <<
						std::endl;
				}
			}
			return RC;
		}
		/**
		 * This function gets if allow to allocate perfcounter from a
		 * different module than the expected one in the same tile.
		 *
		 * @return true if cross module is enabled, false otherwise.
		 */
		bool getCrossMod()
		{
			return CrossMod;
		}
		/**
		 * This function change the reset event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param M reset event module
		 * @param E reset event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeRstEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is in use" << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && M != RstMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is reserved, event module type cannot change." <<
					std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(),
						Loc, M, E, &HwEvent);
				if (RC == XAIE_OK) {
					RstEvent = E;
					RstMod = M;
				}
			}
			return RC;
		}
		/**
		 * This function change threshold to generate counter event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param Val counter value to generate counter event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeThreshold(uint32_t Val) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else {
				EventVal = Val;
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function change the start event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param M reset event module
		 *
		 * @param E start event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeStartEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && M != StartMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is reserved, event module type cannot change." <<
					std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						M, E, &HwEvent);
				if (RC == XAIE_OK) {
					StartEvent = E;
					StartMod = M;
				}
			}
			return RC;
		}
		/**
		 * This function change the stop event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param M module type of the event
		 * @param E stop event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeStopEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && M != StopMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Counter Mod=" << Rsc.Mod <<
					" resource is reserved, event module type cannot change." <<
					std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						M, E, &HwEvent);
				if (RC == XAIE_OK) {
					StopEvent = E;
					StopMod = M;
				}
			}
			return RC;
		}
		/**
		 * This function reads the perfcounter value if counter is
		 * in use.
		 *
		 * @param R counter value if counter is in use.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC readResult(uint32_t &R) {
			AieRC RC;

			if (State.Running == 0) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not in use." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAie_PerfCounterGet(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod),
					Rsc.RscId, &R);
			}
			return RC;
		}
		/**
		 * This function returns the counter event and the event module.
		 *
		 * @param M module of the counter
		 * @param E counter event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getCounterEvent(XAie_ModuleType &M, XAie_Events &E) const {
			AieRC RC;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not allocated." << std::endl;
				RC = XAIE_ERR;
			} else {
				M = static_cast<XAie_ModuleType>(Rsc.Mod);
				XAie_PerfCounterGetEventBase(AieHd->dev(), Loc, M, &E);
				E = static_cast<XAie_Events>(static_cast<uint32_t>(E) + Rsc.RscId);
				RC = XAIE_OK;
			}

			return RC;
		}
		uint32_t getRscType() const {
			return static_cast<uint32_t>(XAIE_PERFCNT_RSC);
		}
		XAieRscStat getRscStat(const std::string &GName) const {
			XAieRscStat RscStat(GName);
			(void) GName;

			if (preferredId == XAIE_RSC_ID_ANY) {
				if (CrossMod) {
					return AieHd->getRscGroup(GName).getRscStat(Loc,
						getRscType());
				} else {
					return AieHd->getRscGroup(GName).getRscStat(Loc,
						Mod, getRscType());
				}
			} else {
				return AieHd->getRscGroup(GName).getRscStat(Loc,
						Mod, getRscType(), preferredId);
			}
		}
	protected:
		XAie_Events StartEvent; /**< start event */
		XAie_Events StopEvent; /**< stop event */
		XAie_Events RstEvent; /**< reset event */
		XAie_ModuleType StartMod; /**< start event module */
		XAie_ModuleType StopMod; /**< stop event module */
		XAie_ModuleType RstMod; /**< Reset event module */
		uint32_t EventVal; /**< counter event value */
		bool CrossMod; /**< true to indicate can try allocating counter
				    from different module in the same tile */
		XAieBroadcast *StartBC; /**< start Event braodcast resource */
		XAieBroadcast *StopBC; /**< stop Event braodcast resource */
		XAieBroadcast *RstBC; /**< reset Event braodcast resource */
	private:
		AieRC _reserve() {
			AieRC RC;
			uint32_t TType = _XAie_GetTileTypefromLoc(dev(), Loc);

			XAie_UserRscReq Req = {Loc, Mod, 1};

			if (preferredId == XAIE_RSC_ID_ANY) {
				RC = XAie_RequestPerfcnt(AieHd->dev(), 1, &Req, 1, &Rsc);
			} else {
				Rsc.RscType = XAIE_PERFCNT_RSC;
				Rsc.Loc.Col = Loc.Col;
				Rsc.Loc.Row = Loc.Row;
				Rsc.Mod = Mod;
				Rsc.RscId = preferredId;
				RC = XAie_RequestAllocatedPerfcnt(AieHd->dev(), 1, &Rsc);
			}
			if (RC != XAIE_OK && preferredId == XAIE_RSC_ID_ANY) {
				if (TType == XAIEGBL_TILE_TYPE_AIETILE &&
					CrossMod) {
					XAie_ModuleType lM;

					if (Mod == XAIE_CORE_MOD) {
						lM = XAIE_MEM_MOD;
					} else {
						lM = XAIE_CORE_MOD;
					}
					Req.Mod = lM;
					RC = XAie_RequestPerfcnt(AieHd->dev(), 1, &Req, 1, &Rsc);
				}
			}
			if (RC  == XAIE_OK &&
				TType == XAIEGBL_TILE_TYPE_AIETILE) {
				std::vector<XAie_LocType> vL;

				vL.push_back(Loc);
				if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					StartBC = new XAieBroadcast(AieHd, vL,
						StartMod, static_cast<XAie_ModuleType>(Rsc.Mod));
					RC = StartBC->reserve();
					if (RC != XAIE_OK) {
						delete StartBC;
					}
				}
				if (RC == XAIE_OK && StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					StopBC = new XAieBroadcast(AieHd, vL,
							StartMod, static_cast<XAie_ModuleType>(Rsc.Mod));
					RC = StopBC->reserve();
					if (RC != XAIE_OK) {
						delete StopBC;
						if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
							StartBC->release();
							delete StartBC;
						}
					}
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE &&
					RstMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					RstBC = new XAieBroadcast(AieHd, vL,
							StartMod, static_cast<XAie_ModuleType>(Rsc.Mod));
					RC = RstBC->reserve();
					if (RC != XAIE_OK) {
						delete RstBC;
						if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
							StartBC->release();
							delete StartBC;
						}
						if (StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
							StopBC->release();
							delete StopBC;
						}
					}
				}
				if (RC != XAIE_OK) {
					RC = XAie_ReleasePerfcnt(AieHd->dev(), 1, &Rsc);
				}
			}

			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource not available.\n";
			} else {
				RC = _reserveAppend();
			}
			return RC;
		}
		AieRC _release() {
			if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				StartBC->release();
				delete StartBC;
			}
			if (StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				StopBC->release();
				delete StopBC;
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				RstBC->release();
				delete RstBC;
			}
			_releaseAppend();
			return XAie_ReleasePerfcnt(AieHd->dev(), 1, &Rsc);
		}
		AieRC _start() {
			AieRC RC;

			RC = _startPrepend();
			if (RC == XAIE_OK) {
				XAie_Events lStartE = StartEvent;
				XAie_Events lStopE = StopEvent;
				XAie_Events lRstE = RstEvent;

				if(EventVal != 0) {
					RC = XAie_PerfCounterEventValueSet(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod),
						Rsc.RscId, EventVal);
				}

				if (RC == XAIE_OK && StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					StartBC->getEvent(Loc, static_cast<XAie_ModuleType>(Rsc.Mod), lStartE);
					RC = XAie_EventBroadcast(dev(), Loc, StartMod, StartBC->getBc(), StartEvent);
					if (RC == XAIE_OK) {
						RC = StartBC->start();
					}
				}
				if (RC == XAIE_OK && StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					StopBC->getEvent(Loc, static_cast<XAie_ModuleType>(Rsc.Mod), lStopE);
					XAie_EventBroadcast(dev(), Loc, StopMod, StopBC->getBc(), StopEvent);
					if (RC == XAIE_OK) {
						RC = StopBC->start();
					}
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE && RstMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
					RstBC->getEvent(Loc, static_cast<XAie_ModuleType>(Rsc.Mod), lRstE);
					RC = XAie_EventBroadcast(dev(), Loc, RstMod, RstBC->getBc(), RstEvent);
					if (RC == XAIE_OK) {
						RC = RstBC->start();
					}
				}
				if (RC == XAIE_OK) {
					RC = XAie_PerfCounterControlSet(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod),
						Rsc.RscId, lStartE, lStopE);
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE) {
					RC = XAie_PerfCounterResetControlSet(dev(), Loc,
						static_cast<XAie_ModuleType>(Rsc.Mod), Rsc.RscId, lRstE);
				}
			}

			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" failed to start." << std::endl;
			}
			return RC;
		}
		AieRC _stop() {
			AieRC RC;
			int iRC;

			iRC = (int)XAie_PerfCounterControlReset(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod), Rsc.RscId);
			iRC |= (int)XAie_PerfCounterResetControlReset(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod), Rsc.RscId);
			iRC |= (int)XAie_PerfCounterReset(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod), Rsc.RscId);
			iRC |= (int)XAie_PerfCounterEventValueReset(dev(), Loc, static_cast<XAie_ModuleType>(Rsc.Mod), Rsc.RscId);

			if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				StartBC->getEvent(Loc, static_cast<XAie_ModuleType>(Rsc.Mod), StartEvent);
				iRC |= XAie_EventBroadcastReset(dev(), Loc,
						StartMod, StartBC->getBc());
				iRC |= StartBC->stop();
			}
			if (StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				StopBC->getEvent(Loc, static_cast<XAie_ModuleType>(Rsc.Mod), StopEvent);
				iRC |= XAie_EventBroadcastReset(dev(), Loc,
						StopMod, StopBC->getBc());
				iRC |= StopBC->stop();
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && RstMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				RstBC->getEvent(Loc, static_cast<XAie_ModuleType>(Rsc.Mod), RstEvent);
				iRC |= XAie_EventBroadcastReset(dev(), Loc,
						RstMod, RstBC->getBc());
				iRC |= RstBC->stop();
			}
			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" failed to stop." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = _stopAppend();
			}
			return RC;
		}

		void _getRscs(std::vector<XAie_UserRsc> &vRscs) const {
			vRscs.push_back(Rsc);
			if (StartMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				StartBC->getRscs(vRscs);
			}
			if (StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				StopBC->getRscs(vRscs);
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && StopMod != static_cast<XAie_ModuleType>(Rsc.Mod)) {
				RstBC->getRscs(vRscs);
			}
			_getRscsAppend(vRscs);
		}
		virtual AieRC _startPrepend() {
			return XAIE_OK;
		}
		virtual AieRC _reserveAppend() {
			return XAIE_OK;
		}
		virtual AieRC _stopAppend() {
			return XAIE_OK;
		}
		virtual AieRC _releaseAppend() {
			return XAIE_OK;
		}
		virtual void _getRscsAppend(std::vector<XAie_UserRsc> &vRscs) const {
			(void)vRscs;
		}
	};
}
