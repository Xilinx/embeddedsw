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
	 * @class XAieStreamPortSelect
	 * @brief class for stream port select resource
	 * There are limited numter of ports events, in order to generate events
	 * for stream port status, applicaiton needs to reserve a stream port
	 * select resource first.
	 */
	class XAieStreamPortSelect: public XAieSingleTileRsc {
	private:
		//TODO: should be replaced with SSW AIE driver rsc manager
		static AieRC XAieAllocRsc(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L,
				XAie_UserRsc &R) {
			uint64_t *bits;
			int bit, sbit;
			AieRC RC = XAIE_OK;

			(void)Dev;
			if (L.Row == 0) {
				bits = Dev->XAieSSShimTBits;
				sbit = L.Col * 8;
			} else {
				bits = Dev->XAieSSCoreTBits;
				sbit = (L.Col * 8 + (L.Row - 1)) * 8;
			}
			bit = XAieRsc::alloc_rsc_bit(bits, sbit, 8);

			if (bit < 0) {
				RC = XAIE_ERR;
			} else {
				R.Loc = L;
				if (L.Row == 0) {
					R.Mod = XAIE_PL_MOD;
				} else {
					R.Mod = XAIE_CORE_MOD;
				}
				R.Type = XAieRscType::SSWITCHSELECT;
				R.RscId = bit - sbit;
			}
			return RC;
		}
		static void XAieReleaseRsc(std::shared_ptr<XAieDev> Dev,
				const XAie_UserRsc &R) {
			uint64_t *bits;
			int pos;

			(void)Dev;
			if (R.Mod == XAIE_PL_MOD) {
				bits = Dev->XAieSSShimTBits;
				pos = R.Loc.Col * 8 + R.RscId;
			} else {
				bits = Dev->XAieSSCoreTBits;
				pos = (R.Loc.Col * 8 + (R.Loc.Row - 1)) * 8 + R.RscId;
			}
			XAieRsc::clear_rsc_bit(bits, pos);
		}
	public:
		XAieStreamPortSelect() = delete;
		XAieStreamPortSelect(std::shared_ptr<XAieDev> Dev, const XAie_LocType &L):
			XAieSingleTileRsc(Dev, L) {
			State.Initialized = 1;
		}
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

			RC = XAieStreamPortSelect::XAieAllocRsc(Aie, Loc, Rsc);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "Stream port select " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" resource not available." << std::endl;
			}
			return RC;
		}
		AieRC _release() {
			XAieStreamPortSelect::XAieReleaseRsc(Aie, Rsc);

			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;

			RC = XAie_EventSelectStrmPort(Aie->dev(), Loc, Rsc.RscId,
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

			RC = XAie_EventSelectStrmPortReset(Aie->dev(), Loc, Rsc.RscId);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "Stream port select " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" failed to stop." << std::endl;
			}
			return RC;
		}
	};
}
