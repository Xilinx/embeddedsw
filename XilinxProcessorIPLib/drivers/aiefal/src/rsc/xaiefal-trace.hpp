// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cerrno>
#include <fstream>
#include <functional>
#include <string.h>
#include <vector>
#include <xaiengine.h>
#include <bitset>

#include <xaiefal/rsc/xaiefal-bc.hpp>
#include <xaiefal/rsc/xaiefal-rsc-base.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAieTraceCntr
	 * @brief Trace Event resource class
	 */
	class XAieTraceCntr: public XAieSingleTileRsc {
	public:
		XAieTraceCntr() = delete;
		XAieTraceCntr(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M):
			XAieSingleTileRsc(DevHd, L, M), Pkt() {
			XAie_EventPhysicalToLogicalConv(dev(), Loc, Mod, 0,
				&StartEvent);
			StopEvent = StartEvent;
			Mode = XAIE_TRACE_EVENT_TIME;
			for(uint32_t i = 0;
				i < sizeof(Events)/sizeof(Events[0]);
				i++) {
				Events[i] = StartEvent;
			}
			StartMod = Mod;
			StopMod = Mod;
			State.Initialized = 1;
		}
		XAieTraceCntr(XAieDev &Dev,
			XAie_LocType L, XAie_ModuleType M):
			XAieSingleTileRsc(Dev, L, M), Pkt() {}
		~XAieTraceCntr() {
			if (State.Reserved == 1) {
				if (StartMod != Mod) {
					delete StartBC;
				}
				if (StopMod != Mod) {
					delete StopBC;
				}
			}
		}

		/**
		 * This funtion gets module of the Trace control.
		 *
		 * @return module of the trace control
		 */
		XAie_ModuleType getModule() {
			return Mod;
		}
		/**
		 * This funtion reserve a trace slot.
		 * It will fail if the trace control is already configured in
		 * hardware. This function is supposed to be called before
		 * start().
		 *
		 * @param Slot to return the reserved trace slot.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC reserveTraceSlot(uint8_t &Slot) {
			AieRC RC = XAIE_ERR;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, tracing already started." << std::endl;
			} else {
				for (uint32_t i = 0; i < TraceSlotBits.size(); i++) {
					if (TraceSlotBits.test(i) == 0) {
						TraceSlotBits.set(i);
						Slot = i;
						RC = XAIE_OK;
						break;
					}
				}
			}
			return RC;
		}
		/**
		 * This funtion release a trace slot.
		 *
		 * @param Slot trace slot to release
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC releaseTraceSlot(uint8_t Slot) {
			AieRC RC = XAIE_ERR;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, tracing already started." << std::endl;
			} else if (Slot >= static_cast<uint32_t>(TraceSlotBits.size())) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, invalid slot id " << Slot << "." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				XAie_Events E;

				TraceSlotBits.reset(Slot);
				XAie_EventPhysicalToLogicalConv(dev(), Loc, Mod, 0, &E);
				Events[Slot] = E;
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function sets events to trace to this object.
		 * No hardware configuration is changed.
		 * It will check if trace control is already configured. If
		 * yes, it will fail the setting. It will also fail if the slot
		 * is not yet assigned. User is expected to call
		 * reserveTraceSlot() first before setTraceEvent().
		 *
		 * @param Slot trace slot for the event to configure
		 * @param E event to configure.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC setTraceEvent(uint32_t Slot, XAie_Events E) {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << __func__ << " " <<
				"(" << static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ") Mod=" << Mod <<
				" Slot=" << Slot << " E=" << E << std::endl;
			if (State.Initialized == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace cntr object not initialized, set module first." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace started." << std::endl;
				RC = XAIE_ERR;
			} else if (Slot >= static_cast<uint32_t>(TraceSlotBits.size())) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, invalid slot." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else if ((TraceSlotBits.test(Slot) == 0)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace slot is not reserved." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				if (E == XAIE_EVENT_NONE_CORE) {
					XAie_EventPhysicalToLogicalConv(dev(), Loc, Mod, 0, &E);
				}
				Events[Slot] = E;
				RC = XAIE_OK;
			}
			changeToConfigured();
			return RC;
		}
		/**
		 * This function sets start event of the trace control.
		 * No hardware configuration is changed.
		 * It will check if trace control is already configured. If yes,
		 * it will fail the setting. That is it needs to be called
		 * before start().
		 *
		 * @param StartE start event to set
		 * @param StopE stop event to set
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC setCntrEvent(XAie_Events StartE, XAie_Events StopE) {
			XAie_ModuleType StartM, StopM;
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << __func__ << " " <<
				"(" << static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ") Mod=" << Mod <<
				" StartE=" << StartE << " StopE=" << StopE << std::endl;
			if (State.Initialized == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace cntr object not initialized, set module first." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace started." << std::endl;
				RC = XAIE_ERR;
			} else {
				uint8_t HwE;

				StartM = XAieEstimateModFromEvent(StartE);
				StopM = XAieEstimateModFromEvent(StopE);
				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						StartM, StartE, &HwE);
				if (RC == XAIE_OK) {
					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						StopM, StopE, &HwE);
				}
				if ((StartM != Mod || StopM != Mod) &&
					State.Reserved == 1) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed, trace reserved," <<
						"but start/stop event not of the same module of trace control. " <<
						"Please set control event before reserve() if they are of different mod" <<
						std::endl;
					RC = XAIE_ERR;
				}
			}
			if (RC == XAIE_OK) {
				StartEvent = StartE;
				StopEvent = StopE;
				StartMod = StartM;
				StopMod = StopM;
				changeToConfigured();
			}
			return RC;
		}
		/**
		 * This function sets trace control mode.
		 * This function needs to be called before trace control is
		 * configured in hardware. That is it needs to be called before
		 * start().
		 *
		 * @param M trace control mode
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setMode(XAie_TraceMode M) {
			AieRC RC;

			if (State.Initialized == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace cntr object not initialized, set module first." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace started." << std::endl;
				RC = XAIE_ERR;
			} else {
				Mode = M;
				changeToConfigured();
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function sets trace control packet config.
		 * This function needs to be called before trace control is
		 * configured in hardware. That is it needs to be called before
		 * start().
		 *
		 * @param P trace control packet setting
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setPkt(const XAie_Packet &P) {
			AieRC RC;

			if (State.Initialized == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace cntr object not initialized, set module first." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed, trace started." << std::endl;
				RC = XAIE_ERR;
			} else {
				Pkt = P;
				changeToConfigured();
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This functinon returns the max number of trace events
		 * supported by the trace control.
		 *
		 * @return supported max trace events
		 */
		uint32_t getMaxTraceEvents() const {
			// TODO: It is better to get from C driver.
			return 8;
		}
		/**
		 * This functinon returns the number of trace events
		 * which have been reserved.
		 *
		 * @return number of reserved events.
		 */
		uint32_t getReservedTraceEvents() const {
			return TraceSlotBits.count();
		}
		/**
		 * This function returns the trace control start event broadcast
		 * event if trace event needs broadcasting.
		 *
		 * @return broadcast channel if trace event is reserved, and
		 *	   broadcast is required, error code for failure.
		 */
		int getStartBc() {
			int BcId;

			if (State.Reserved == 1) {
				if (StartMod != Mod) {
					BcId = StartBC->getBc();
				} else {
					BcId = -EINVAL;
				}
			} else {
				Logger::log(LogLevel::ERROR) << "trace control " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") trace control Mod=" << Mod <<
					" not reserved" << Mod << std::endl;
				BcId = -EPERM;
			}

			return BcId;
		}
		/**
		 * This function returns the trace control stop event broadcast
		 * event if trace event needs broadcasting.
		 *
		 * @return broadcast channel if trace event is reserved, and
		 *	   broadcast is required, error code for failure.
		 */
		int getStopBc() {
			int BcId;

			if (State.Reserved == 1) {
				if (StopMod != Mod) {
					BcId = StopBC->getBc();
				} else {
					BcId = -EINVAL;
				}
			} else {
				Logger::log(LogLevel::ERROR) << "trace control " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") trace control Mod=" << Mod <<
					" not reserved" << Mod << std::endl;
				BcId = -EPERM;
			}

			return BcId;
		}
	protected:
		AieRC _reserve() {
			AieRC RC;
			XAie_UserRscReq Req = {Loc, Mod, 1};

			RC = XAie_RequestTraceCtrl(AieHd->dev(), 1, &Req, 1, &Rsc);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "trace control " << __func__ << " (" <<
							static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
							") Mod=" << Mod <<" failed to reserve." << std::endl;
			}
			if (RC == XAIE_OK && StartMod != Mod) {
				std::vector<XAie_LocType> vL;

				vL.push_back(Loc);
				StartBC = new XAieBroadcast(AieHd, vL,
					StartMod, Mod);
				RC = StartBC->reserve();
				if (RC != XAIE_OK) {
					delete StartBC;
				}
			}
			if (RC == XAIE_OK && StopMod != Mod) {
				std::vector<XAie_LocType> vL;

				vL.push_back(Loc);
				StopBC = new XAieBroadcast(AieHd, vL,
						StartMod, Mod);
				RC = StopBC->reserve();
				if (RC != XAIE_OK) {
					delete StopBC;
					if (StartMod != Mod) {
						StartBC->release();
						delete StartBC;
					}
				}
			}
			return RC;
		}

		AieRC _release() {
			AieRC RC;

			if (StartMod != Mod) {
				StartBC->release();
				delete StartBC;
			}
			if (StopMod != Mod) {
				StopBC->release();
				delete StopBC;
			}

			RC = XAie_ReleaseTraceCtrl(AieHd->dev(), 1, &Rsc);
			if(RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "trace control " << __func__ << " (" <<
							static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
							") Mod=" << Mod <<" failed to release." << std::endl;
			}
			return RC;
		}

		AieRC _start() {
			AieRC RC;
			std::vector<XAie_Events> vE;
			std::vector<u8> vSlot;
			XAie_Events lStartE = StartEvent;
			XAie_Events lStopE = StopEvent;

			Logger::log(LogLevel::DEBUG) << "trace control " << __func__ << " (" <<
				static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ") Mod=" << Mod << std::endl;
			for (uint32_t i = 0; i < TraceSlotBits.size(); i++) {
				if (TraceSlotBits.test(i) != 0) {
					vE.push_back(Events[i]);
					vSlot.push_back((uint8_t)i);
				}
			}
			RC = XAIE_OK;
			if (StartMod != Mod) {
				StartBC->getEvent(Loc, Mod, lStartE);
				RC = XAie_EventBroadcast(dev(), Loc, StartMod,
						StartBC->getBc(), StartEvent);
				if (RC == XAIE_OK) {
					RC = StartBC->start();
				}
			}
			if (RC == XAIE_OK && StopMod != Mod) {
				StopBC->getEvent(Loc, Mod, lStopE);
				RC = XAie_EventBroadcast(dev(), Loc, StopMod,
						StopBC->getBc(), StopEvent);
				if (RC == XAIE_OK) {
					RC = StopBC->start();
				}
			}
			if (RC == XAIE_OK) {
				RC = XAie_TraceEventList(dev(), Loc, Mod,
						vE.data(), vSlot.data(),
						vE.size());
			}
			if (RC == XAIE_OK) {
				RC = XAie_TracePktConfig(dev(), Loc, Mod, Pkt);
				if (RC == XAIE_OK) {
					RC = XAie_TraceControlConfig(dev(),
						Loc, Mod, lStartE, lStopE,
						Mode);
				}
			}
			return RC;
		}

		AieRC _stop() {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "trace control " << __func__ << " (" <<
				static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ") Mod=" << Mod << std::endl;
			// Do not reset the packet setting as it can
			// cause issues on outstanding contents in
			// the trace buffer.
			// Reset the start, stop, and the trace events only.
			RC = XAie_TraceControlConfigReset(dev(), Loc, Mod);
			if (RC == XAIE_OK) {
				for (uint8_t s = 0;
					s < (TraceSlotBits.size());
					s++) {
					if (TraceSlotBits.test(s) != 0) {
						XAie_TraceEventReset(dev(), Loc,
							Mod, s);
					}
				}
			}
			if (RC == XAIE_OK && StartMod != Mod) {
				StartBC->getEvent(Loc, Mod, StartEvent);
				RC = XAie_EventBroadcastReset(dev(), Loc,
						StartMod, StartBC->getBc());
				if (RC == XAIE_OK) {
					RC = StartBC->stop();
				}
			}
			if (StopMod != Mod) {
				StopBC->getEvent(Loc, Mod, StopEvent);
				RC = XAie_EventBroadcastReset(dev(), Loc,
						StopMod, StopBC->getBc());
				if (RC == XAIE_OK) {
					RC = StopBC->stop();
				}
			}
			return RC;
		}

		std::bitset<8> TraceSlotBits; /**< trace slots bitmap */
		XAie_Events Events[8]; /**< events to trace */
		XAie_Events StartEvent; /**< trace control start event */
		XAie_Events StopEvent; /**< trace control stop event */
		XAie_ModuleType StartMod; /**< start event module */
		XAie_ModuleType StopMod; /**< stop event module */
		XAieBroadcast *StartBC; /**< start Event braodcast resource */
		XAieBroadcast *StopBC; /**< stop Event braodcast resource */
		XAie_Packet Pkt; /**< trace packet setup */
		XAie_TraceMode Mode; /**< trace operation mode */
	private:
		void changeToConfigured() {
			if (State.Configured == 0 &&
				StartEvent != XAIE_EVENT_NONE_CORE &&
				TraceSlotBits != 0) {
				std::bitset<8> bits(TraceSlotBits);
				for (uint32_t i = 0; i < TraceSlotBits.size(); i++) {
					XAie_Events E;

					if (bits.test(i) == 0) {
						continue;
					}
					XAie_EventPhysicalToLogicalConv(dev(), Loc, Mod, 0, &E);
					if (Events[i] != E) {
						State.Configured = 1;
						break;
					}
				}
			}
		}
	};
	/**
	 * @class XAieTraceEvent
	 * @brief Trace event resource class.
	 *	  Each event to trace is presented as a trace event resource
	 *	  class.
	 */
	class XAieTraceEvent: public XAieSingleTileRsc {
	public:
		XAieTraceEvent() = delete;
		XAieTraceEvent(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M,
			std::shared_ptr<XAieTraceCntr> TCntr):
			XAieSingleTileRsc(DevHd, L, M),
			EventMod(M) {
			if (!TCntr) {
				throw std::invalid_argument("Trace event failed, empty trace control");
			}
			if (TCntr->dev() != DevHd->dev() ||
				TCntr->getModule() != Mod) {
				throw std::invalid_argument("Trace event failed,trace control aiedev or module mismatched");
			}
			TraceCntr = std::move(TCntr);
			State.Initialized = 1;
		}
		XAieTraceEvent(XAieDev &Dev,
			XAie_LocType L, XAie_ModuleType M):
			XAieSingleTileRsc(Dev, L, M),
			EventMod(M) {
			TraceCntr = Dev.tile(L).module(M).traceControl();
			State.Initialized = 1;
		}
		~XAieTraceEvent() {}
		/**
		 * This function sets event to trace.
		 *
		 * @param M module of the event
		 * @param E event to trace
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC setEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;
			uint8_t HwEvent;

			RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
					M, E, &HwEvent);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") Event Mod=" << M << " Event=" << E <<
					" invalid event" << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else if (State.Running == 1) {
				RC = XAIE_ERR;
				Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") Event Mod=" << M << " Event=" << E <<
					" trace event already in used" << std::endl;
			} else if (State.Reserved == 1 && M != EventMod) {
				RC = XAIE_INVALID_ARGS;
				Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") Event Mod=" << M << " Event=" << E <<
					" trace event already reserved, input event module is different to the one already set" << std::endl;
			} else {
				Event = E;
				EventMod = M;
				State.Configured = 1;
			}
			return RC;
		}
		/**
		 * This function returns the event to trace and its module.
		 * If the event is not set, it will return failure.
		 *
		 * @param M returns the module of the event
		 * @param E returns the event to trace
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getEvent(XAie_ModuleType &M, XAie_Events &E) const {
			AieRC RC;

			if (State.Configured == 0) {
				Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") trace control Mod=" << TraceCntr->getModule() <<
					" Event Mod=" << Mod << " no event specified" << std::endl;
				RC = XAIE_ERR;
			} else {
				E = Event;
				M = EventMod;
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function returns the trace event broadcast event if
		 * trace event needs broadcasting.
		 *
		 * @return broadcast channel if trace event is reserved, and
		 *	   broadcast is required, error code for failure.
		 */
		int getBc() {
			int BcId;

			if (State.Reserved == 1) {
				if (EventMod != TraceCntr->getModule()) {
					BcId = BC->getBc();
				} else {
					BcId = -EINVAL;
				}
			} else {
				Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") trace control Mod=" << TraceCntr->getModule() <<
					" Event Mod=" << EventMod <<
					" resource not reserved." << std::endl;
				BcId = -EPERM;
			}

			return BcId;
		}
	protected:
		AieRC _reserve() {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "trace event " << __func__ << " (" <<
				static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
				") trace control Mod=" << TraceCntr->getModule() <<
				" Event Mod=" << Mod << std::endl;
			RC = TraceCntr->reserveTraceSlot(Slot);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "trace event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
					") trace control Mod=" << TraceCntr->getModule() <<
					" Event Mod=" << Mod << " no trace slot" << std::endl;
			} else if (EventMod != TraceCntr->getModule()) {
				std::vector<XAie_LocType> vL;

				vL.push_back(Loc);
				BC = std::make_shared<XAieBroadcast>(AieHd, vL,
					XAIE_CORE_MOD, XAIE_MEM_MOD);
				RC = BC->reserve();
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
						static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
						") trace control Mod=" << TraceCntr->getModule() <<
						" Event Mod=" << Mod << " no broadcast event" << std::endl;
					TraceCntr->releaseTraceSlot(Slot);
				} else {
					Rsc.Mod = Mod;
					Rsc.RscId = Slot;
				}
			} else {
				Rsc.Mod = Mod;
				Rsc.RscId = Slot;
			}
			return RC;
		}
		AieRC _release() {
			Logger::log(LogLevel::DEBUG) << "trace event " << __func__ << " (" <<
				static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
				") trace control Mod=" << TraceCntr->getModule() <<
				" Event Mod=" << Mod << "Event=" << Event << std::endl;
			TraceCntr->releaseTraceSlot(Slot);
			if (EventMod != TraceCntr->getModule()) {
				BC->release();
			}
			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "trace event " << __func__ << " (" <<
				static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
				") trace control Mod=" << TraceCntr->getModule() <<
				" Event Mod=" << EventMod << "Event=" << Event << std::endl;
			if (EventMod != TraceCntr->getModule()) {
				RC = XAie_EventBroadcast(dev(), Loc, EventMod,
						BC->getBc(), Event);
				if (RC == XAIE_OK) {
					XAie_Events BcE;

					BC->getEvent(Loc,
						TraceCntr->getModule(), BcE);
					RC = BC->start();
					if (RC == XAIE_OK) {
						RC = TraceCntr->setTraceEvent(Slot, BcE);
					} else {
						Logger::log(LogLevel::ERROR) << "trace event " << __func__ << " (" <<
							static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
							") trace control Mod=" << TraceCntr->getModule() <<
							" Event Mod=" << EventMod << "Event=" << Event <<
							" failed." << std::endl;
					}
				}
			} else {
				RC = TraceCntr->setTraceEvent(Slot, Event);
			}
			return RC;
		}
		AieRC _stop() {
			AieRC RC, lRC;
			XAie_Events E;

			Logger::log(LogLevel::DEBUG) << "trace event " << __func__ << " (" <<
				static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) <<
				") trace control Mod=" << TraceCntr->getModule() <<
				" Event Mod=" << Mod << "Event=" << Event << std::endl;
			RC = XAIE_OK;
			XAie_EventPhysicalToLogicalConv(dev(), Loc,
					TraceCntr->getModule(), 0, &E);
			lRC = TraceCntr->setTraceEvent(Slot, E);
			if (lRC != XAIE_OK) {
				RC = lRC;
			}
			if (EventMod != TraceCntr->getModule()) {
				lRC = BC->stop();
				if (lRC != XAIE_OK) {
					RC = lRC;
				}
			}
			return RC;
		}
	protected:
		std::shared_ptr<XAieTraceCntr> TraceCntr;
		XAie_Events Event; /**< event to trace */
		XAie_ModuleType EventMod; /**< event module type */
		std::shared_ptr<XAieBroadcast> BC; /**< broadcast resource if need to broadcast event to tracer */
		uint8_t Slot; /**< trace slot */
	};
	/**
	 * @class XAieTracing
	 * @brief class of AI engine event tracing.
	 *	  An XAieTracing object contains AI engine trace control and
	 *	  events to trace.
	 */
	class XAieTracing: public XAieSingleTileRsc {
	public:
		XAieTracing() = delete;
		XAieTracing(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M, std::shared_ptr<XAieTraceCntr> TCntr):
			XAieSingleTileRsc(DevHd, L, M) {
			if (!TCntr) {
				throw std::invalid_argument("Trace event failed, empty trace control");
			}
			if (TCntr->dev() != DevHd->dev() ||
				TCntr->getModule() != Mod) {
				throw std::invalid_argument("Trace event failed,trace control aiedev or module mismatched");
			}
			TraceCntr = std::move(TCntr);
			State.Initialized = 1;
		}
		XAieTracing(XAieDev &Dev, XAie_LocType L,
			XAie_ModuleType M):
			XAieSingleTileRsc(Dev, L, M) {
			TraceCntr = Dev.tile(L).module(M).traceControl();
			State.Initialized = 1;
		}
		~XAieTracing() {
			Events.clear();
		}
		/**
		 * This function adds an event to trace.
		 * This function will needs to be called before the trace
		 * control is configured in hardware. That is it needs to
		 * be called before start().
		 *
		 * @param M module of the event
		 * @param E event to trace
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC addEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") Mod=" << static_cast<uint32_t>(M) << " E=" << E << std::endl;
			if (Events.size() == TraceCntr->getMaxTraceEvents()) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed for tracing, exceeded max num of events." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed for tracing, resource reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				XAieTraceEvent TraceE(AieHd, Loc, Mod, TraceCntr);

				RC = TraceE.setEvent(M, E);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed for tracing, failed to initialize event." << std::endl;
				} else if (State.Reserved == 1) {
					RC = TraceE.reserve();
					if (RC != XAIE_OK) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"failed for tracing, reserved new event failed." << std::endl;
					} else {
						RC = TraceE.start();
						if (RC != XAIE_OK) {
							TraceE.release();
						}
					}
				}
				if (RC == XAIE_OK) {
					Events.push_back(TraceE);
					changeToConfigured();
				}
			}
			return RC;
		}
		/**
		 * This function removes an event.
		 * It needs to be called before start().
		 *
		 * @param M module of the event
		 * @param E event to remove
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC removeEvent(XAie_ModuleType M, XAie_Events E) {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") Mod=" << static_cast<uint32_t>(M) << " E=" << E << std::endl;
			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed for tracing, resource reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_INVALID_ARGS;
				for (uint32_t i = 0; i < Events.size(); i++) {
					XAie_Events lE;
					XAie_ModuleType lM;

					Events[i].getEvent(lM, lE);
					if (lM == M && E == lE) {
						Events.erase(Events.begin() + i);
						RC = XAIE_OK;
						break;
					}
				}
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed for tracing, event doesn't exist." << std::endl;
				} else {
					changeToConfigured();
				}
			}
			return RC;
		}
		/**
		 * This function sets start event of the trace control.
		 * No hardware configuration is changed.
		 * It will check if trace control is already configured. If yes,
		 * it will fail the setting. That is it needs to be called
		 * before start().
		 *
		 * @param StartE start event to set
		 * @param StopE stop event to set
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC setCntrEvent(XAie_Events StartE, XAie_Events StopE) {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") Mod=" << static_cast<uint32_t>(TraceCntr->getModule()) <<
				" StartE=" << StartE << " StopE=" << StopE << std::endl;
			RC = TraceCntr->setCntrEvent(StartE, StopE);
			if (RC == XAIE_OK) {
				changeToConfigured();
			}
			return RC;
		}
		/**
		 * This function sets trace control mode.
		 * This function needs to be called before trace control is
		 * configured in hardware. That is it needs to be called before
		 * start().
		 *
		 * @param M trace control mode
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setMode(XAie_TraceMode M) {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") M=" << M << std::endl;
			RC = TraceCntr->setMode(M);
			if (RC == XAIE_OK) {
				changeToConfigured();
			}
			return RC;
		}
		/**
		 * This function sets trace control packet config.
		 * This function needs to be called before trace control is
		 * configured in hardware. That is it needs to be called before
		 * start().
		 *
		 * @param P trace control packet setting
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setPkt(const XAie_Packet &P) {
			AieRC RC;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") Mod=" << static_cast<uint32_t>(TraceCntr->getModule()) << std::endl;
			RC = TraceCntr->setPkt(P);
			if (RC == XAIE_OK) {
				changeToConfigured();
			}
			return RC;
		}
		/**
		 * This functinon returns the number of available free trace
		 * slots on the trace control.
		 *
		 * @return number of reserved events.
		 */
		uint32_t getTraceControlAvailTraceSlots() const {
			uint32_t MaxEvents, NumReserved;

			MaxEvents = getMaxTraceEvents();
			NumReserved = TraceCntr->getReservedTraceEvents();
			return (MaxEvents - NumReserved);
		}
		/**
		 * This functinon returns the max number of trace events
		 * supported by the trace control.
		 *
		 * @return supported max trace events
		 */
		uint32_t getMaxTraceEvents() const {
			return TraceCntr->getMaxTraceEvents();
		}
	protected:
		AieRC _reserve() {
			AieRC RC = XAIE_OK;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") Mod=" << static_cast<uint32_t>(TraceCntr->getModule()) << std::endl;
			if (RC == XAIE_OK && !(TraceCntr->isReserved())) {
				RC = TraceCntr->reserve();
			}
			for (uint32_t i = 0; i < Events.size(); i++) {
				RC = Events[i].reserve();
				if (RC != XAIE_OK) {
					break;
				}
			}
			if (RC != XAIE_OK) {
				for (uint32_t i = 0; i < Events.size(); i++) {
					Events[i].release();
				}
			} else {
				// After reserved tracing events, configure
				// them so that the trace control state can
				// change to configured.
				for (uint32_t i = 0; i < Events.size(); i++) {
					Events[i].start();
				}
				changeToConfigured();
			}
			return RC;
		}
		AieRC _release() {
			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " ("
				<< static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) <<
				") Mod=" << static_cast<uint32_t>(TraceCntr->getModule()) << std::endl;
			TraceCntr->release();
			for (uint32_t i = 0; i < Events.size(); i++) {
				Events[i].release();
			}
			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC = XAIE_OK;

			Logger::log(LogLevel::DEBUG) << "tracing " << __func__ << " (" <<
				static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) << ")" <<
				" Mod=" << TraceCntr->getModule() << ", " << Events.size() << " events to trace." << std::endl;
			for (uint32_t i = 0; i < Events.size(); i++) {
				RC = Events[i].start();
				if (RC != XAIE_OK) {
					break;
				}
			}
			if (RC == XAIE_OK) {
				RC = TraceCntr->start();
			}
			if (RC != XAIE_OK) {
				for (uint32_t i = 0; i < Events.size(); i++) {
					Events[i].stop();
				}
			}
			return RC;
		}
		AieRC _stop() {
			Logger::log(LogLevel::DEBUG) << "tracing "<< __func__ << " (" <<
				static_cast<uint32_t>(TraceCntr->loc().Col) << "," << static_cast<uint32_t>(TraceCntr->loc().Row) << ")" <<
				" Mod=" << TraceCntr->getModule() << ", " << Events.size() << " events to trace." << std::endl;
			TraceCntr->stop();
			for (uint32_t i = 0; i < Events.size(); i++) {
				Events[i].stop();
			}
			return XAIE_OK;
		}
		std::shared_ptr<XAieTraceCntr> TraceCntr;
		std::vector<XAieTraceEvent> Events;
	private:
		void changeToConfigured() {
			if (State.Configured == 0) {
				if (TraceCntr->isConfigured()) {
					if (Events.size() > 0) {
						State.Configured = 1;
					}
				}
			}
		}
	};
}
