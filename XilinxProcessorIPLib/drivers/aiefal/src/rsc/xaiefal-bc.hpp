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
	 * @class XAieBroadcast
	 * @brief class for broadcast channel resource.
	 * A broadcast channel resource represents a broadcast channel between
	 * specified tiles.
	 */
	class XAieBroadcast: public XAieRsc {
	public:
		XAieBroadcast() = delete;
		XAieBroadcast(std::shared_ptr<XAieDevHandle> DevHd,
			const std::vector<XAie_LocType> &vL,
			XAie_ModuleType StartM, XAie_ModuleType EndM):
			XAieRsc(DevHd) {
			StartMod = StartM;
			EndMod = EndM;
			vLocs = vL;
			State.Initialized = 1;
			State.Configured = 1;
		}
		XAieBroadcast(XAieDev &Dev,
			const std::vector<XAie_LocType> &vL,
			XAie_ModuleType StartM, XAie_ModuleType EndM):
			XAieBroadcast(Dev.getDevHandle(), vL, StartM, EndM) {}
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
		AieRC getEvent(XAie_LocType L, XAie_ModuleType M, XAie_Events &E) {
			AieRC RC = XAIE_INVALID_ARGS;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "broadcast object " << __func__ << " (" <<
					(uint32_t)L.Col << "," << (uint32_t)L.Row << ")" <<
					" Mod= " << M <<
					" resource not reserved." << std::endl;
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
		/**
		 * This function returns broadcast channel information
		 *
		 * @param vL returns vector of tiles of the channel
		 * @param StartM returns start module
		 * @param EndM returns end module
		 * @return XAIE_OK
		 */
		AieRC getChannel(std::vector<XAie_LocType> &vL,
				 XAie_ModuleType &StartM,
				 XAie_ModuleType &EndM) {
			vL = vLocs;
			StartM = StartMod;
			EndM = EndMod;

			return XAIE_OK;
		}
	private:
		AieRC _reserve() {
			return XAieBroadcast::XAieAllocRsc(AieHd, vLocs, StartMod, EndMod, vRscs);
		}
		AieRC _release() {
			for (auto r: vRscs) {
				XAieBroadcast::XAieReleaseRsc(AieHd, r);
			}
			vRscs.clear();
			return XAIE_OK;
		}
		AieRC _start() {
			AieRC RC;

			for (auto r = vRscs.begin(); r != vRscs.end(); r++) {
				auto nr = r + 1;

				u8 bc_block = XAIE_EVENT_BROADCAST_ALL;

				if (nr != vRscs.end()) {
					if ((*nr).Loc.Col > (*r).Loc.Col) {
						bc_block &= ~XAIE_EVENT_BROADCAST_EAST;
					} else if ((*nr).Loc.Col < (*r).Loc.Col) {
						bc_block &= ~XAIE_EVENT_BROADCAST_WEST;
					} else if ((*nr).Loc.Row == (*r).Loc.Row) {
						if ((*r).Mod == XAIE_CORE_MOD) {
							bc_block &= ~XAIE_EVENT_BROADCAST_EAST;
						} else {
							bc_block &= ~XAIE_EVENT_BROADCAST_WEST;
						}
					} else if ((*nr).Loc.Row < (*r).Loc.Row) {
						bc_block &= ~XAIE_EVENT_BROADCAST_SOUTH;
					} else {
						bc_block &= ~XAIE_EVENT_BROADCAST_NORTH;
					}
				}

				if (r != vRscs.begin()) {
					auto pr = r - 1;

					if ((*pr).Loc.Col > (*r).Loc.Col) {
						bc_block &= ~XAIE_EVENT_BROADCAST_EAST;
					} else if ((*pr).Loc.Col < (*r).Loc.Col) {
						bc_block &= ~XAIE_EVENT_BROADCAST_WEST;
					} else if ((*pr).Loc.Row == (*r).Loc.Row) {
						if ((*r).Mod == XAIE_CORE_MOD) {
							bc_block &= ~XAIE_EVENT_BROADCAST_EAST;
						} else {
							bc_block &= ~XAIE_EVENT_BROADCAST_WEST;
						}
					} else if ((*pr).Loc.Row < (*r).Loc.Row) {
						bc_block &= ~XAIE_EVENT_BROADCAST_SOUTH;
					} else {
						bc_block &= ~XAIE_EVENT_BROADCAST_NORTH;
					}
				}

				if ((*r).Mod == XAIE_PL_MOD) {
					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_A,
						(*r).RscId,
						bc_block & (~XAIE_EVENT_BROADCAST_EAST));
					if (RC != XAIE_OK) {
						break;
					}
					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_B,
						(*r).RscId,
						bc_block & (~XAIE_EVENT_BROADCAST_WEST));
					if (RC != XAIE_OK) {
						break;
					}
				} else {
					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_A,
						(*r).RscId,
						bc_block);
					if (RC != XAIE_OK) {
						break;
					}
				}
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "BC: " <<
					" failed to stop." << std::endl;
			}
			return RC;
		}
		AieRC _stop() {
			AieRC RC;
			int iRC = XAIE_OK;

			for (auto r: vRscs) {
				if (r.Loc.Row == 0) {
					// Unblock SHIM tile switch A and B
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, r.Mod,
						XAIE_EVENT_SWITCH_A, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, r.Mod,
						XAIE_EVENT_SWITCH_B, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
				} else {
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, r.Mod,
						XAIE_EVENT_SWITCH_A, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
				}
			}
			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "BC: " <<
					" failed to stop." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
			}
			return RC;
		}
	private:
		XAie_ModuleType StartMod; /**< module type of the starting module on the channel */
		XAie_ModuleType EndMod; /**< module type of the ending modile on the channel */
		std::vector<XAie_LocType> vLocs; /**< tiles on the channel */
		std::vector<XAie_UserRsc> vRscs; /**< broadcast channel allocated r esources */
	private:
		/**
		 * TODO: Following function will not be required.
		 * Bitmap will be moved to device driver
		 */
		static void getAieBCTileBits(std::shared_ptr<XAieDevHandle> Dev,
				XAie_LocType L, const XAie_ModuleType M, uint16_t &bits) {
			uint32_t i;

			if (L.Row == 0) {
				i = L.Col;
				bits = Dev->XAieBroadcastShimBits[i];
			} else if (M == XAIE_MEM_MOD) {
				i = L.Col * 8 + L.Row - 1;
				bits = Dev->XAieBroadcastMemBits[i];
			} else {
				i = L.Col * 8 + L.Row - 1;
				bits = Dev->XAieBroadcastCoreBits[i];
			}
		}

		/**
		 * TODO: Following function will not be required.
		 * Bitmap will be moved to device driver
		 */
		static AieRC XAieAllocRsc(std::shared_ptr<XAieDevHandle> Dev,
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
					return XAIE_OK;
				} else {
					return XAIE_ERR;
				}
			}
			if ((vL[0].Row == 0 && startM != XAIE_PL_MOD) ||
			    (vL.back().Row == 0 && endM != XAIE_PL_MOD) ||
			    (vL[0].Row != 0 && startM == XAIE_PL_MOD) ||
			    (vL.back().Row != 0 && endM == XAIE_PL_MOD)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: invalid tiles and modules combination." << std::endl;
				return XAIE_INVALID_ARGS;
			}
			bits = 0;
			for (size_t i = 0; i < vL.size(); i++) {
				XAie_UserRsc R;
				size_t ni;
				uint16_t lbits;
				XAie_ModuleType M = startM;
				uint32_t TType;

				TType = _XAie_GetTileTypefromLoc(Dev->dev(),
						vL[i]);
				if (TType == XAIEGBL_TILE_TYPE_MAX) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"BC: invalid tile (" << vL[i].Col <<
						"," << vL[i].Row <<")" <<
						std::endl;
					return XAIE_INVALID_ARGS;
				}

				R.Loc.Col = vL[i].Col;
				R.Loc.Row = vL[i].Row;
				R.Mod = M;
				getAieBCTileBits(Dev, vL[i], M, lbits);
				bits |= lbits;
				vR.push_back(R);

				if (i == (vL.size() - 1) && endM != M) {
					getAieBCTileBits(Dev, vL[i], endM, lbits);
					bits |= lbits;
					R.Mod = endM;
					vR.push_back(R);
				}
				ni = i + 1;
				if (ni >= vL.size()) {
					break;
				}
				if (vL[ni].Row == vL[i].Row &&
					vL[ni].Col == vL[i].Col) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"BC: duplicated tiles in the vector." <<
						std::endl;
					return XAIE_INVALID_ARGS;
				}
				if (vL[ni].Row != vL[i].Row) {
					if ((vL[ni].Col != vL[i].Col) ||
						(vL[ni].Row > vL[i].Row &&
						 (vL[ni].Row - vL[i].Row) > 1) ||
						(vL[ni].Row < vL[i].Row &&
						 (vL[i].Row - vL[ni].Row) > 1)) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"BC: discontinuous input tiles." <<
							std::endl;
						return XAIE_INVALID_ARGS;
					}
				} else {
					if ((vL[ni].Col > vL[i].Col &&
					     (vL[ni].Col - vL[i].Col) > 1) ||
					    (vL[ni].Col < vL[i].Col &&
					     (vL[i].Col - vL[ni].Col) > 1)) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"BC: discontinuous input tiles." <<
							std::endl;
						return XAIE_INVALID_ARGS;
					}
					if (vL[ni].Col > vL[i].Col &&
						startM == XAIE_CORE_MOD) {
						getAieBCTileBits(Dev, vL[i],
							 XAIE_MEM_MOD, lbits);
						bits |= lbits;
						R.Mod = XAIE_MEM_MOD;
						vR.push_back(R);
					} else if (vL[ni].Col < vL[i].Col &&
						startM == XAIE_MEM_MOD) {
						getAieBCTileBits(Dev, vL[i],
							 XAIE_CORE_MOD, lbits);
						bits |= lbits;
						R.Mod = XAIE_CORE_MOD;
						vR.push_back(R);
					}
				}

			}
			for (int i = 0; i < 16; i++) {
				if ((bits & (1 << i)) == 0) {
					bci = i;
					break;
				}
			}
			if (bci < 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: no free BC." << std::endl;
				return XAIE_INVALID_ARGS;
			}
			for (auto r = vR.begin(); r != vR.end(); r++) {
				uint16_t *lbits_ptr;
				uint32_t j;

				if ((*r).Loc.Row == 0) {
					j = (*r).Loc.Col;
					lbits_ptr = &Dev->XAieBroadcastShimBits[j];
				} else if ((*r).Mod == XAIE_MEM_MOD) {
					j = (*r).Loc.Col * 8 + (*r).Loc.Row - 1;
					lbits_ptr = &Dev->XAieBroadcastMemBits[j];
				} else {
					j = (*r).Loc.Col * 8 + (*r).Loc.Row - 1;
					lbits_ptr = &Dev->XAieBroadcastCoreBits[j];
				}
				*lbits_ptr |= (1 << bci);
				(*r).RscId = (uint32_t)bci;
			}
			return XAIE_OK;
		}
		/**
		 * TODO: Following function will not be required.
		 * Bitmap will be moved to device driver
		 */
		static void XAieReleaseRsc(std::shared_ptr<XAieDevHandle> Dev,
				const XAie_UserRsc &R) {

			if ((R.Loc.Row == 0 && R.Mod != XAIE_PL_MOD) ||
			    (R.Loc.Row != 0 && R.Mod == XAIE_PL_MOD)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: invalid tile and module." << std::endl;
			} else if (R.Mod == XAIE_PL_MOD) {
				int i = R.Loc.Col;

				Dev->XAieBroadcastShimBits[i] &= ~(1 << R.RscId);
			} else if (R.Mod == XAIE_CORE_MOD) {
				int i = R.Loc.Col * 8 + (R.Loc.Row - 1);

				Dev->XAieBroadcastCoreBits[i] &= ~(1 << R.RscId);
			} else {
				int i = R.Loc.Col * 8 + (R.Loc.Row - 1);

				Dev->XAieBroadcastMemBits[i] &= ~(1 << R.RscId);
			}
		}
	};
}
