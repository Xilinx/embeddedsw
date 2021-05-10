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
	 * @class XAieStreamPortSelect
	 * @brief class for stream port select resource
	 * There are limited numter of ports events, in order to generate events
	 * for stream port status, applicaiton needs to reserve a stream port
	 * select resource first.
	 */
	class XAieStreamPortSelect: public XAieSingleTileRsc {
	public:
		XAieStreamPortSelect() = delete;
		XAieStreamPortSelect(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L):
			XAieSingleTileRsc(DevHd, L) {
			State.Initialized = 1;
		}
		XAieStreamPortSelect(XAieDev &Dev, XAie_LocType L):
			XAieSingleTileRsc(Dev.getDevHandle(), L) {}
		/**
		 * This function sets which port to select.
		 * It needs to be called before start() which configures the hardware.
		 *
		 * @param PIntf port interface
		 * @param PType port type
		 * @param PNum port number
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setPortToSelect(XAie_StrmPortIntf PIntf, StrmSwPortType PType,
				uint32_t PNum) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "Stream port select " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else {
				PortIntf = PIntf;
				PortType = PType;
				PortNum = PNum;
				State.Configured = 1;
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function returns stream port idle event.
		 *
		 * @param E store the selected stream port idle event
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getSSIdleEvent(XAie_Events &E) const {
			AieRC RC;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "Stream port select " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" resource not reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				if (Loc.Row == 0) {
					E = XAIE_EVENT_PORT_IDLE_0_PL;
				} else {
					E = XAIE_EVENT_PORT_IDLE_0_CORE;
				}
				E = (XAie_Events)((uint32_t)E + Rsc.RscId * 4);
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function returns stream port running event.
		 *
		 * @param E store the selected stream port running event
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getSSRunningEvent(XAie_Events &E) const {
			AieRC RC;

			RC = getSSIdleEvent(E);
			if (RC == XAIE_OK) {
				E = (XAie_Events)((uint32_t)E + 1);
			}
			return RC;
		}
		/**
		 * This function returns stream port stalled event.
		 *
		 * @param E store the selected stream port stalled event
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getSSStalledEvent(XAie_Events &E) const {
			AieRC RC;

			RC = getSSIdleEvent(E);
			if (RC == XAIE_OK) {
				E = (XAie_Events)((uint32_t)E + 2);
			}
			return RC;
		}
		/**
		 * This function returns stream port tlast event.
		 *
		 * @param E store the selected stream port tlast event
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getSSTlastEvent(XAie_Events &E) const {
			AieRC RC;

			RC = getSSIdleEvent(E);
			if (RC == XAIE_OK) {
				E = (XAie_Events)((uint32_t)E + 3);
			}
			return RC;
		}
	private:
		XAie_StrmPortIntf PortIntf; /**< port interface */
		StrmSwPortType PortType; /**< port type master, or slave */
		uint8_t PortNum; /**< port number */
	protected:
		AieRC _reserve() {
			AieRC RC;

			if (preferredId == XAIE_RSC_ID_ANY) {
				XAie_UserRscReq Req = {Loc, Mod, 1};
				RC = XAie_RequestSSEventPortSelect(AieHd->dev(), 1, &Req, 1, &Rsc);
			} else {
				Rsc.RscType = XAIE_SS_EVENT_PORTS_RSC;
				Rsc.Loc.Col = Loc.Col;
				Rsc.Loc.Row = Loc.Row;
				Rsc.Mod = static_cast<uint32_t>(Mod);
				Rsc.RscId = preferredId;
				RC = XAie_RequestAllocatedSSEventPortSelect(AieHd->dev(), 1, &Rsc);
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "Stream port select " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" resource not available.\n";
			}
			return RC;
		}
		AieRC _release() {
			return XAie_ReleaseSSEventPortSelect(AieHd->dev(), 1, &Rsc);
		}
		AieRC _start() {
			AieRC RC;

			RC = XAie_EventSelectStrmPort(dev(), Loc, Rsc.RscId,
					PortIntf, PortType, PortNum);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "Stream port select " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" failed to start." << std::endl;
			}
			return RC;
		}
		AieRC _stop() {
			AieRC RC;

			RC = XAie_EventSelectStrmPortReset(dev(), Loc, Rsc.RscId);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "Stream port select " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" failed to stop." << std::endl;
			}
			return RC;
		}
	};
}
