// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
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

using namespace std;

namespace xaiefal {
namespace resource {
using namespace xaiefal::log;
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
	private:
		//TODO: should be replaced with SSW AIE driver rsc manager
		static AieRC XAieAllocRsc(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L,
				XAie_ModuleType M, XAie_UserRsc &R) {
			uint64_t *bits;
			int sbit, bits2check;
			AieRC RC = XAIE_OK;

			(void)Dev;
			if (M == XAIE_CORE_MOD) {
				if (L.Row == 0) {
					Logger::log(LogLevel::ERROR) << __func__
						<< "invalid tile("
						<< (unsigned)L.Col << ","
						<< (unsigned)L.Row
						<< "for core mod." << endl;
					RC = XAIE_INVALID_ARGS;
				} else {
					bits = Dev->XAiePerfCoreBits;
					sbit = (L.Col * 8 + (L.Row - 1)) * 4;
					bits2check = 4;
				}
			} else if (M == XAIE_MEM_MOD) {
				if (L.Row == 0) {
					Logger::log(LogLevel::ERROR) << __func__
						<< "invalid tile("
						<< (unsigned)L.Col << ","
						<< (unsigned)L.Row
						<< "for mem mod." << endl;
					RC = XAIE_INVALID_ARGS;
				} else {
					bits = Dev->XAiePerfMemBits;
					sbit = (L.Col * 8 + (L.Row - 1)) * 2;
					bits2check = 2;
				}
			} else if (M == XAIE_PL_MOD) {
				if (L.Row != 0) {
					Logger::log(LogLevel::ERROR) << __func__
						<< "invalid tile("
						<< (unsigned)L.Col << ","
						<< (unsigned)L.Row
						<< "for PL mod." << endl;
					RC = XAIE_INVALID_ARGS;
				} else {
					bits = Dev->XAiePerfShimBits;
					sbit = L.Col * 2;
					bits2check = 2;
				}
			} else {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"invalid module type" << endl;
				RC = XAIE_INVALID_ARGS;
			}
			if (RC == XAIE_OK) {
				int bit = XAieRsc::alloc_rsc_bit(bits, sbit, bits2check);

				if (bit < 0) {
					RC = XAIE_ERR;
				} else {
					R.Loc = L;
					R.Mod = M;
					R.Type = XAieRscType::PCOUNTER;
					R.RscId = bit - sbit;
				}
			}
			return RC;
		}
		static void XAieReleaseRsc(std::shared_ptr<XAieDev> Dev,
				const XAie_UserRsc &R) {
			uint64_t *bits;
			int pos;

			(void)Dev;
			if ((R.Loc.Row == 0 && R.Mod != XAIE_PL_MOD) ||
			    (R.Loc.Row != 0 && R.Mod == XAIE_PL_MOD)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"PCount: invalid tile and module." << endl;
				return;
			} else if (R.Mod == XAIE_PL_MOD) {
				bits = Dev->XAiePerfShimBits;
				pos = R.Loc.Col * 2 + R.RscId;
			} else if (R.Mod == XAIE_CORE_MOD) {
				bits = Dev->XAiePerfCoreBits;
				pos = (R.Loc.Col * 8 + (R.Loc.Row - 1)) * 4 + R.RscId;
			} else {
				bits = Dev->XAiePerfMemBits;
				pos = (R.Loc.Col * 8 + (R.Loc.Row - 1)) * 2 + R.RscId;
			}
			XAieRsc::clear_rsc_bit(bits, pos);
		}
	public:
		XAiePerfCounter() = delete;
		XAiePerfCounter(std::shared_ptr<XAieDev> &Dev, const XAie_LocType &L):
			XAieSingleTileRsc(Dev, L), CrossMod(false) {
			StartEvent = XAIE_EVENT_NONE_CORE;
			StopEvent = XAIE_EVENT_NONE_CORE;
			RstEvent = XAIE_EVENT_NONE_CORE;
			StartMod = Mod;
			StopMod = Mod;
			RstMod = Mod;
			StartBC = nullptr;
			StopBC = nullptr;
			RstBC = nullptr;
		}
		XAiePerfCounter(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L,
			XAie_ModuleType M):
			XAieSingleTileRsc(Dev, L, M), CrossMod(false) {
			if (_XAie_CheckModule(Aie->dev(), Loc, Mod) == XAIE_OK) {
				State.Initialized = 1;
			} else {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << M << " Invalid tile and module combination" << endl;
			}
		}
		~XAiePerfCounter() {
			if (State.Reserved == 1) {
				if (StartMod != Rsc.Mod) {
					delete StartBC;
				}
				if (StopMod != Rsc.Mod) {
					delete StopBC;
				}
				if (RstEvent != XAIE_EVENT_NONE_CORE &&
					RstMod != Rsc.Mod) {
					delete RstBC;
				}
			}
		}
		/**
		 * This function sets the perfcounter start, stop, reset events.
		 *
		 * @param StartE start event
		 * @param StopE stop event
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
					" Expect Mod= " << Mod << " resource reserved." << endl;
				return XAIE_ERR;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc,
					StartM, StartE, &HwEvent);
				if (RC == XAIE_OK) {
					RC = XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc,
						StopM, StopE, &HwEvent);
				}
				if (RC == XAIE_OK && RstE != XAIE_EVENT_NONE_CORE) {
					RC = XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc,
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
						endl;
				}
			}
			return RC;
		}
		/**
		 * This function sets if allow to allocate perfcounter from a
		 * different module than the expected one in the same tile.
		 *
		 * This function needs to be called before reserving resource.
		 *
		 * @param EnableCrossMod true to allow cross module, false
		 *	  otherwise.
		 * @return XAIE_OK for success, XAIE_ERR if resource has been
		 *	   reserved.
		 */
		AieRC setCrossMod(bool EnableCrossMod)
		{
			AieRC RC;

			if (State.Reserved == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource reserved." << endl;
				RC = XAIE_ERR;
			} else {
				CrossMod = EnableCrossMod;
				RC = XAIE_OK;
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
					" resource is in use" << endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && M != RstMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is reserved, event module type cannot change." <<
					endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(Aie->dev(),
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
					" resource is in use." << endl;
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
					" resource is in use." << endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && M != StartMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is reserved, event module type cannot change." <<
					endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc,
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
		 * @param E stop event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeStopEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" resource is in use." << endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && M != StopMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Counter Mod=" << Rsc.Mod <<
					" resource is reserved, event module type cannot change." <<
					endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc,
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
					" resource not in use." << endl;
				RC = XAIE_ERR;
			} else {
				RC = XAie_PerfCounterGet(Aie->dev(), Loc, Rsc.Mod,
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
					" resource not allocated." << endl;
				RC = XAIE_ERR;
			} else {
				if (Rsc.Mod == XAIE_CORE_MOD) {
					E = XAIE_EVENT_PERF_CNT_0_CORE;
				} else if (Rsc.Mod == XAIE_MEM_MOD) {
					E = XAIE_EVENT_PERF_CNT_0_MEM;
				} else {
					E = XAIE_EVENT_PERF_CNT_0_PL;
				}
				M = Rsc.Mod;
				E = (XAie_Events)((uint32_t)E + Rsc.RscId);
				RC = XAIE_OK;
			}

			return RC;
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
			uint32_t TType = _XAie_GetTileTypefromLoc(Aie->dev(), Loc);

			RC = XAiePerfCounter::XAieAllocRsc(Aie, Loc, Mod, Rsc);
			if (RC != XAIE_OK) {
				if (TType == XAIEGBL_TILE_TYPE_AIETILE &&
					CrossMod) {
					XAie_ModuleType lM;

					if (Mod == XAIE_CORE_MOD) {
						lM = XAIE_MEM_MOD;
					} else {
						lM = XAIE_CORE_MOD;
					}
					RC = XAieAllocRsc(Aie, Loc, lM, Rsc);
				}
			}
			if (RC  == XAIE_OK &&
				TType == XAIEGBL_TILE_TYPE_AIETILE) {
				std::vector<XAie_LocType> vL;

				vL.push_back(Loc);
				if (StartMod != Rsc.Mod) {
					StartBC = new XAieBroadcast(Aie);
					StartBC->initialize(vL, StartMod, Rsc.Mod);
					RC = StartBC->reserve();
					if (RC != XAIE_OK) {
						delete StartBC;
					}
				}
				if (RC == XAIE_OK && StopMod != Rsc.Mod) {
					StopBC = new XAieBroadcast(Aie);
					StopBC->initialize(vL, StartMod, Rsc.Mod);
					RC = StopBC->reserve();
					if (RC != XAIE_OK) {
						delete StopBC;
						if (StartMod != Rsc.Mod) {
							StartBC->release();
							delete StartBC;
						}
					}
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE &&
					RstMod != Rsc.Mod) {
					RstBC = new XAieBroadcast(Aie);
					RstBC->initialize(vL, StartMod, Rsc.Mod);
					RC = RstBC->reserve();
					if (RC != XAIE_OK) {
						delete RstBC;
						if (StartMod != Rsc.Mod) {
							StartBC->release();
							delete StartBC;
						}
						if (StopMod != Rsc.Mod) {
							StopBC->release();
							delete StopBC;
						}
					}
				}
				if (RC != XAIE_OK) {
					XAiePerfCounter::XAieReleaseRsc(Aie, Rsc);
				}
			}

			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not available." << endl;
			}
			return RC;
		}
		AieRC _release() {
			if (StartMod != Rsc.Mod) {
				StartBC->release();
				delete StartBC;
			}
			if (StopMod != Rsc.Mod) {
				StopBC->release();
				delete StopBC;
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && StopMod != Rsc.Mod) {
				RstBC->release();
				delete RstBC;
			}
			XAiePerfCounter::XAieReleaseRsc(Aie, Rsc);
			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;

			RC = _startPrepend();
			if (RC == XAIE_OK) {
				XAie_Events lStartE = StartEvent;
				XAie_Events lStopE = StopEvent;
				XAie_Events lRstE = RstEvent;

				if (StartMod != Rsc.Mod) {
					StartBC->getEvent(Loc, Rsc.Mod, lStartE);
					RC = XAie_EventBroadcast(Aie->dev(), Loc, StartMod, StartBC->getBc(), StartEvent);
				}
				if (RC == XAIE_OK && StopMod != Rsc.Mod) {
					StopBC->getEvent(Loc, Rsc.Mod, lStopE);
					XAie_EventBroadcast(Aie->dev(), Loc, StopMod, StopBC->getBc(), StopEvent);
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE && RstMod != Rsc.Mod) {
					RstBC->getEvent(Loc, Rsc.Mod, lRstE);
					RC = XAie_EventBroadcast(Aie->dev(), Loc, RstMod, RstBC->getBc(), RstEvent);
				}
				if (RC == XAIE_OK) {
					RC = XAie_PerfCounterControlSet(Aie->dev(), Loc, Rsc.Mod,
						Rsc.RscId, lStartE, lStopE);
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE) {
					RC = XAie_PerfCounterResetControlSet(Aie->dev(), Loc,
						Rsc.Mod, Rsc.RscId, lRstE);
				}
			}

			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" failed to start." << endl;
			}
			return RC;
		}
		AieRC _stop() {
			AieRC RC;
			int iRC;

			iRC = (int)XAie_PerfCounterControlReset(Aie->dev(), Loc, Rsc.Mod, Rsc.RscId);
			iRC |= (int)XAie_PerfCounterResetControlReset(Aie->dev(), Loc, Rsc.Mod, Rsc.RscId);
			iRC |= (int)XAie_PerfCounterReset(Aie->dev(), Loc, Rsc.Mod, Rsc.RscId);
			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << Rsc.Mod <<
					" failed to stop." << endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
			}
			return RC;
		}
		AieRC _getRscInfo(std::map<std::string, std::vector<std::string>> &info) const {
			XAie_LocType rL;
			string tmpstr;
			uint8_t HwEvent;

			info.insert(std::make_pair("type", std::vector<std::string>(1, "Counter")));
			Aie->getRelativeLoc(Loc, rL, tmpstr);
			info.insert(std::make_pair("tile_type",
				std::vector<std::string>(1, tmpstr)));
			info.insert(std::make_pair(tmpstr + "_row",
				std::vector<std::string>(1, std::to_string(rL.Row))));
			info.insert(std::make_pair(tmpstr + "_column",
				std::vector<std::string>(1, std::to_string(rL.Col))));
			info.insert(std::make_pair(tmpstr + "_row",
				std::vector<std::string>(1, std::to_string(rL.Row))));
			Aie->modToString(Rsc.Mod, tmpstr);
			info.insert(std::make_pair("module",
				std::vector<std::string>(1, tmpstr)));
			Aie->modToString(Mod, tmpstr);
			info.insert(std::make_pair("expect_module",
				std::vector<std::string>(1, tmpstr)));
			info.insert(std::make_pair("counter_id",
				std::vector<std::string>(1, std::to_string((uint32_t)Rsc.RscId))));
			info.insert(std::make_pair("threshold",
				std::vector<std::string>(1, std::to_string(EventVal))));
			XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc, StartMod, StartEvent, &HwEvent);
			info.insert(std::make_pair("start",
				std::vector<std::string>(1, std::to_string((uint32_t)HwEvent))));
			if (StartMod != Mod) {
				Aie->modToString(StartMod, tmpstr);
				info.insert(std::make_pair("start_module",
					std::vector<std::string>(1, tmpstr)));
				info.insert(std::make_pair("start_broadcast",
					std::vector<std::string>(1, std::to_string((uint32_t)StartBC->getBc()))));
			}
			XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc, StopMod, StopEvent, &HwEvent);
			info.insert(std::make_pair("stop",
				std::vector<std::string>(1, std::to_string((uint32_t)HwEvent))));
			if (StopMod != Mod) {
				Aie->modToString(StopMod, tmpstr);
				info.insert(std::make_pair("stop_module",
					std::vector<std::string>(1, tmpstr)));
				info.insert(std::make_pair("stop_broadcast",
					std::vector<std::string>(1, std::to_string((uint32_t)StopBC->getBc()))));
			}
			XAie_EventLogicalToPhysicalConv(Aie->dev(), Loc, RstMod, RstEvent, &HwEvent);
			info.insert(std::make_pair("reset",
				std::vector<std::string>(1, std::to_string((uint32_t)HwEvent))));
			if (RstMod != Mod) {
				Aie->modToString(RstMod, tmpstr);
				info.insert(std::make_pair("reset_module",
					std::vector<std::string>(1, tmpstr)));
				info.insert(std::make_pair("reset_broadcast",
					std::vector<std::string>(1, std::to_string((uint32_t)RstBC->getBc()))));
			}
			return XAIE_OK;
		}
		virtual AieRC _startPrepend() {
			return XAIE_OK;
		}
	};
}
}
