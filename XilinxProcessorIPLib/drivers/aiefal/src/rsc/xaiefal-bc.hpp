// (c) Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <string.h>
#include <vector>
#include <xaiengine.h>

#include <xaiefal/rsc/xaiefal-rsc-base.hpp>

#pragma once

using namespace std;

namespace xaiefal {
namespace resource {
using namespace xaiefal::log;
	/**
	 * @class XAieBroadcast
	 * @brief class for broadcast channel resource.
	 * A broadcast channel resource represents a broadcast channel between
	 * specified tiles.
	 */
	class XAieBroadcast: public XAieRsc {
	public:
		XAieBroadcast() = delete;
		XAieBroadcast(std::shared_ptr<XAieDev> &Dev): XAieRsc(Dev) {}
		/**
		 * This function sets the broadcast path.
		 *
		 * @param vL tiles of the broadcast path
		 * @param StartM module type of the starting tile of the path
		 * @param EndM module type of the ending tile of the path
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC initialize(const std::vector<XAie_LocType> &vL,
				XAie_ModuleType StartM, XAie_ModuleType EndM) {
			StartMod = StartM;
			EndMod = EndM;
			vLocs = vL;
			State.Initialized = 1;
			State.Configured = 1;
			return XAIE_OK;
		}
		~XAieBroadcast() {}
		/**
		 * This function returns the broadcast channel ID
		 *
		 * @return broadcast channel ID if resource has been reserved, -1 otherwise.
		 */
		int getBc() {
			int BC = -1;

			if (State.Reserved == 1) {
				BC = vRscs[0].RscId;
			} else {
			}
			return BC;
		}
		/**
		 * This function gets the broadcast event of the specified tile
		 * module on the broadcast channel.
		 *
		 * @param L tile location
		 * @param M module type
		 * @param E for returning broadcast event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getEvent(const XAie_LocType &L, XAie_ModuleType M, XAie_Events &E) {
			AieRC RC = XAIE_INVALID_ARGS;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "broadcast object " << __func__ << " (" <<
					(uint32_t)L.Col << "," << (uint32_t)L.Row << ")" <<
					" Mod= " << M <<
					" resource not reserved." << endl;
				RC = XAIE_ERR;
			} else {
				for (int i = 0; i < (int)vLocs.size(); i++) {
					if (L.Col == vLocs[i].Col && L.Row == vLocs[i].Row) {
						if (L.Row == 0) {
							E = XAIE_EVENT_BROADCAST_A_0_PL;
						} else if (M == XAIE_MEM_MOD) {
							E = XAIE_EVENT_BROADCAST_0_MEM;
						} else {
							E = XAIE_EVENT_BROADCAST_0_CORE;
						}
						E = (XAie_Events)((uint32_t)E + vRscs[0].RscId);
						break;
					}
				}
			}
			return RC;
		}
	private:
		AieRC _reserve() {
			return XAieBroadcast::XAieAllocRsc(Aie, vLocs, StartMod, EndMod, vRscs);
		}
		AieRC _release() {
			for (int i = 0; i < (int)vRscs.size(); i++) {
				XAieBroadcast::XAieReleaseRsc(Aie, vRscs[i]);
			}
			return XAIE_OK;
		}
	private:
		XAie_ModuleType StartMod; /**< module type of the starting module on the channel */
		XAie_ModuleType EndMod; /**< module type of the ending modile on the channel */
		std::vector<XAie_LocType> vLocs; /**< tiles on the channel */
		std::vector<XAie_UserRsc> vRscs; /**< broadcast channel allocated r esources */
	private:
		static void getAieBCTileBits(std::shared_ptr<XAieDev> Dev,
				const XAie_LocType &L, uint16_t &bits) {
			uint32_t i;

			if (L.Row == 0) {
				i = L.Col;
				bits = Dev->XAieBroadcastShimBits[i];
			} else {
				i = L.Col * 8 + L.Row - 1;
				bits = Dev->XAieBroadcastMemBits[i] | Dev->XAieBroadcastCoreBits[i];
			}
		}
	public:
		/**
		 * TODO: will not be required of bitmap is moved to device driver
		 */
		static AieRC XAieAllocRsc(std::shared_ptr<XAieDev> Dev,
				const std::vector<XAie_LocType> &vL,
				XAie_ModuleType startM, XAie_ModuleType endM,
				std::vector<XAie_UserRsc> &vR) {
			uint16_t bits;
			int bci = -1;

			// Validate module and tile
			if (vL.size() == 0) {
				// Broadcast for all tiles
				bits = 0;
				for (int i = 0; i < (int)sizeof(Dev->XAieBroadcastCoreBits); i++) {
					bits |= Dev->XAieBroadcastCoreBits[i];
				}
				for (int i = 0; i < (int)sizeof(Dev->XAieBroadcastMemBits); i++) {
					bits |= Dev->XAieBroadcastMemBits[i];
				}
				for (int i = 0; i < (int)sizeof(Dev->XAieBroadcastShimBits); i++) {
					bits |= Dev->XAieBroadcastShimBits[i];
				}
				for (int i = 0; i < 16; i++) {
					if ((bits & (1 << i)) == 0) {
						bci = i;
					}
				}
				if (bci >= 0) {
					for (int i = 0; i < (int)sizeof(Dev->XAieBroadcastCoreBits); i++) {
						Dev->XAieBroadcastCoreBits[i] |= (1 << bci);
					}
					for (int i = 0; i < (int)sizeof(Dev->XAieBroadcastMemBits); i++) {
						Dev->XAieBroadcastMemBits[i] |= (1 << bci);
					}
					for (int i = 0; i < (int)sizeof(Dev->XAieBroadcastShimBits); i++) {
						Dev->XAieBroadcastShimBits[i] |= (1 << bci);
					}
					for (uint32_t c = 0; c < 400; c++) {
						for (uint32_t r = 0; r < 9; r++) {
							XAie_UserRsc R;

							R.Loc = XAie_TileLoc(c, r);
							if (r != 0) {
								R.Mod = XAIE_CORE_MOD;
								vR.push_back(R);
								R.Mod = XAIE_MEM_MOD;
								vR.push_back(R);
							} else {
								R.Mod = XAIE_PL_MOD;
								vR.push_back(R);
							}
						}
					}
				} else {
					return XAIE_ERR;
				}
			}
			if ((vL[0].Row == 0 && startM != XAIE_PL_MOD) ||
			    (vL.back().Row == 0 && endM != XAIE_PL_MOD) ||
			    (vL[0].Row != 0 && startM == XAIE_PL_MOD) ||
			    (vL.back().Row != 0 && endM == XAIE_PL_MOD)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: invalid tiles and modules combination." << endl;
				return XAIE_ERR;
			}
			bits = 0;
			for (int i = 0; i < (int)vL.size(); i++) {
				uint16_t lbits;

				getAieBCTileBits(Dev, vL[i], lbits);
				bits |= lbits;
			}
			for (int i = 0; i < 16; i++) {
				if ((bits & (1 << i)) == 0) {
					bci = i;
					break;
				}
			}
			if (bci < 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: no free BC." << endl;
				return XAIE_ERR;
			}
			for (int i = 0; i < (int)vL.size(); i++) {
				XAie_UserRsc R;
				if (vL[i].Row == 0) {
					int t = 16 * vL[i].Col;

					Dev->XAieBroadcastShimBits[t] |= 1 << bci;
					R.Loc = vL[i];
					R.Mod = XAIE_PL_MOD;
					R.Type = XAieRscType::BC;
					R.RscId = bci;
					vR.push_back(R);
				} else {
					int t = 16 * (8 * vL[i].Col + vL[i].Row);

					Dev->XAieBroadcastMemBits[t] |= 1 << bci;
					Dev->XAieBroadcastCoreBits[t] |= 1 << bci;
					R.Loc = vL[i];
					R.Mod = XAIE_CORE_MOD;
					R.Type = XAieRscType::BC;
					R.RscId = bci;
					vR.push_back(R);
					R.Mod = XAIE_MEM_MOD;
					vR.push_back(R);
				}
				if (i < (int)vL.size() - 1)  {
					if (vL[i].Row != vL[i + 1].Row) {
						// Vertical
						if (vL[i].Row == 0) {
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
								bci,
								XAIE_EVENT_BROADCAST_WEST);
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
								bci,
								XAIE_EVENT_BROADCAST_EAST);
						} else {
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
								bci,
								XAIE_EVENT_BROADCAST_WEST);
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
								bci,
								XAIE_EVENT_BROADCAST_EAST);
						}
						if (vL[i].Row < vL[i + 1].Row) {
							if (vL[i].Row != 0) {
								XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
									XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
									bci,
									XAIE_EVENT_BROADCAST_SOUTH);
								XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
									XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
									bci,
									XAIE_EVENT_BROADCAST_SOUTH);
							}
						} else {
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
								bci,
								XAIE_EVENT_BROADCAST_NORTH);
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
								bci,
								XAIE_EVENT_BROADCAST_NORTH);
						}
					} else {
						if (vL[i].Row == 0) {
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
								bci,
								XAIE_EVENT_BROADCAST_NORTH | XAIE_EVENT_BROADCAST_SOUTH);
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
								bci,
								XAIE_EVENT_BROADCAST_NORTH | XAIE_EVENT_BROADCAST_SOUTH);
						} else {
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
								bci,
								XAIE_EVENT_BROADCAST_NORTH | XAIE_EVENT_BROADCAST_SOUTH);
							XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
								XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
								bci,
								XAIE_EVENT_BROADCAST_NORTH | XAIE_EVENT_BROADCAST_SOUTH);
						}
						if (vL[i].Col < vL[1 + 1].Col) {
							if (vL[i].Row == 0) {
								XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
									XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
									bci,
									XAIE_EVENT_BROADCAST_WEST);

							} else {
								XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
									XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
									bci,
									XAIE_EVENT_BROADCAST_WEST);
							}
						} else {
							if (vL[i].Row == 0) {
								XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
									XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
									bci,
									XAIE_EVENT_BROADCAST_EAST);

							} else {
								XAie_EventBroadcastBlockDir(Dev->dev(), vL[i],
									XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
									bci,
									XAIE_EVENT_BROADCAST_EAST);
							}
						}
					}
				}
			}
			return XAIE_OK;
		}
		/**
		 * TODO: will not be required of bitmap is moved to device driver
		 */
		static void XAieReleaseRsc(std::shared_ptr<XAieDev> Dev,
				const XAie_UserRsc &R) {

			if ((R.Loc.Row == 0 && R.Mod != XAIE_PL_MOD) ||
			    (R.Loc.Row != 0 && R.Mod == XAIE_PL_MOD)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: invalid tile and module." << endl;
			} else if (R.Mod == XAIE_PL_MOD) {
				int i = R.Loc.Col;

				Dev->XAieBroadcastShimBits[i] &= ~(1 << R.RscId);
				XAie_EventBroadcastUnblockDir(Dev->dev(), R.Loc, R.Mod,
						XAIE_EVENT_SWITCH_A, R.RscId,
						XAIE_EVENT_BROADCAST_ALL);
				XAie_EventBroadcastUnblockDir(Dev->dev(), R.Loc, R.Mod,
						XAIE_EVENT_SWITCH_B, R.RscId,
						XAIE_EVENT_BROADCAST_ALL);
			} else if (R.Mod == XAIE_CORE_MOD) {
				int i = R.Loc.Col * 8 + (R.Loc.Row - 1);

				Dev->XAieBroadcastCoreBits[i] &= ~(1 << R.RscId);
				XAie_EventBroadcastUnblockDir(Dev->dev(), R.Loc, R.Mod,
						XAIE_EVENT_SWITCH_A, R.RscId,
						XAIE_EVENT_BROADCAST_ALL);
			} else {
				int i = R.Loc.Col * 8 + (R.Loc.Row - 1);

				Dev->XAieBroadcastMemBits[i] &= ~(1 << R.RscId);
				XAie_EventBroadcastUnblockDir(Dev->dev(), R.Loc, R.Mod,
						XAIE_EVENT_SWITCH_A, R.RscId,
						XAIE_EVENT_BROADCAST_ALL);
			}
		}
	};
}
}
