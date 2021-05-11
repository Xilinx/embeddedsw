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
	 * @class XAieBroadcast
	 * @brief class for broadcast channel resource.
	 * A broadcast channel resource represents a broadcast channel between
	 * specified tiles.
	 *
	 * There are three different ways tiles can be broadcasted:
	 *
	 * - Broadcast within a tile if a single tile is specified.
	 *
	 * - Broadcast to the whole partition if no tiles are specified.
	 *
	 * - Broadcast to the tiles on the contiguous path of tiles specified by user.
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
					static_cast<uint32_t>(L.Col) << "," << static_cast<uint32_t>(L.Row) << ")" <<
					" Mod= " << M <<
					" resource not reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				for (int i = 0; i < (int)vLocs.size(); i++) {
					if (L.Col == vLocs[i].Col && L.Row == vLocs[i].Row) {
						RC = XAIE_OK;
						if (L.Row == 0) {
							E = XAIE_EVENT_BROADCAST_A_0_PL;
						} else if (M == XAIE_MEM_MOD) {
							E = XAIE_EVENT_BROADCAST_0_MEM;
						} else {
							E = XAIE_EVENT_BROADCAST_0_CORE;
						}
						E = static_cast<XAie_Events>((static_cast<uint32_t>(E) + vRscs[0].RscId));
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
			AieRC RC = XAIE_INVALID_ARGS;
			uint8_t broadcastAll = 1;
			uint32_t numRscs;

			vRscs.clear();

			if (vLocs.size() == 0)  {
				numRscs = AieHd->dev()->AieTileNumRows * AieHd->dev()->NumCols * 2
				+ (AieHd->dev()->NumRows - AieHd->dev()->AieTileNumRows) * AieHd->dev()->NumCols;
				XAie_UserRsc rsc;
				for (uint32_t i = 0; i < numRscs; i++) {
					rsc.RscType = XAIE_BCAST_CHANNEL_RSC;
					vRscs.push_back(rsc);
				}
				RC = XAIE_OK;
			} else {
				RC = setRscs(AieHd, vLocs, StartMod, EndMod, vRscs);
				numRscs = vRscs.size();
				broadcastAll = 0;
			}

			if (RC == XAIE_OK) {
				if (preferredId == XAIE_RSC_ID_ANY) {
					RC = XAie_RequestBroadcastChannel(AieHd->dev(), &numRscs, &vRscs[0], broadcastAll);
				} else {
					RC = XAie_RequestSpecificBroadcastChannel(AieHd->dev(), preferredId, &numRscs, &vRscs[0], broadcastAll);
				}
				vRscs.resize(numRscs);
			}
			return RC;

		}
		AieRC _release() {
			return XAie_ReleaseBroadcastChannel(AieHd->dev(), vRscs.size(), &vRscs[0]);
		}
		AieRC _start() {
			AieRC RC = XAIE_OK;
			int i = 0;

			for(auto r = vRscs.begin(); r != vRscs.end();) {
				uint8_t un_block = 0;
				if ((*r).Loc.Row == 0) {
					if (r != vRscs.begin()) {
						if  ((*(r - 1)).Loc.Col < ((*r).Loc.Col ))  {
							un_block = XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r - 1)).Loc.Col > ((*r).Loc.Col))  {
							un_block = XAIE_EVENT_BROADCAST_EAST;
						} else {
							// Vertical
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						}
					}
					if ((r + 1) != vRscs.end()) {
						if  ((*(r + 1)).Loc.Col < ((*r).Loc.Col ))  {
							un_block = XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r + 1)).Loc.Col > ((*r).Loc.Col))  {
							un_block = XAIE_EVENT_BROADCAST_EAST;
						} else {
							// Vertical
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						}
					}
					RC = XAie_EventBroadcastUnblockDir(dev(),
						(*r).Loc, static_cast<XAie_ModuleType>((*r).Mod),
						XAIE_EVENT_SWITCH_A,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL);

					RC = XAie_EventBroadcastUnblockDir(dev(),
						(*r).Loc, static_cast<XAie_ModuleType>((*r).Mod),
						XAIE_EVENT_SWITCH_B,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, static_cast<XAie_ModuleType>((*r).Mod),
						XAIE_EVENT_SWITCH_A,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL & (~(un_block | XAIE_EVENT_BROADCAST_EAST)));
					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, static_cast<XAie_ModuleType>((*r).Mod),
						XAIE_EVENT_SWITCH_B,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL & (~(un_block | XAIE_EVENT_BROADCAST_WEST)));
					r++;
				} else { // non SHIM
					uint8_t un_block_n;
					if (r != vRscs.begin()) {
						if  ((*(r - 1)).Loc.Col < ((*r).Loc.Col ))  {
							un_block = XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r - 1)).Loc.Col > ((*r).Loc.Col))  {
							un_block = XAIE_EVENT_BROADCAST_EAST;
						} else if ((*(r - 1)).Loc.Row > ((*r).Loc.Row))  {
							// Vertical
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_SOUTH;
						}
					}

					if ((r + 2) != vRscs.end()) {
						if  ((*(r + 2)).Loc.Col < ((*r).Loc.Col ))  {
							un_block = XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r + 2)).Loc.Col > ((*r).Loc.Col))  {
							un_block = XAIE_EVENT_BROADCAST_EAST;
						} else if ((*(r + 2)).Loc.Row > ((*r).Loc.Row))  {
							// Vertical
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_SOUTH;
						}
					}
					un_block_n = un_block;
					if ((*r).Mod == static_cast<uint32_t>(XAIE_MEM_MOD)) {
						if ((*r).Loc.Row % 2) {
							un_block |= XAIE_EVENT_BROADCAST_WEST;
							un_block_n |= XAIE_EVENT_BROADCAST_EAST;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_EAST;
							un_block_n |= XAIE_EVENT_BROADCAST_WEST;
						}
					} else {
						if ((*r).Loc.Row % 2) {
							un_block |= XAIE_EVENT_BROADCAST_EAST;
							un_block_n |= XAIE_EVENT_BROADCAST_WEST;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_WEST;
							un_block_n |= XAIE_EVENT_BROADCAST_EAST;
						}
					}
					RC = XAie_EventBroadcastUnblockDir(dev(),
									(*r).Loc, static_cast<XAie_ModuleType>((*r).Mod),
									XAIE_EVENT_SWITCH_A,
									(*r).RscId,
									XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastUnblockDir(dev(),
									(*(r+1)).Loc, static_cast<XAie_ModuleType>((*(r + 1)).Mod),
									XAIE_EVENT_SWITCH_B,
									(*(r + 1)).RscId,
									XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastBlockDir(dev(),
									(*r).Loc, static_cast<XAie_ModuleType>((*r).Mod),
									XAIE_EVENT_SWITCH_A,
									(*r).RscId,
									XAIE_EVENT_BROADCAST_ALL & (~un_block));
					RC = XAie_EventBroadcastBlockDir(dev(),
									(*(r + 1)).Loc, static_cast<XAie_ModuleType>((*(r+1)).Mod),
									XAIE_EVENT_SWITCH_A,
									(*(r + 1)).RscId,
									XAIE_EVENT_BROADCAST_ALL & (~un_block_n));
					r += 2;
				}
				if (RC != XAIE_OK) {
					break;
				}
				i++;
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

			if(vLocs.size() == 0) {
				return XAIE_OK;
			}

			for (auto r: vRscs) {
				if (r.Loc.Row == 0) {
					// Unblock SHIM tile switch A and B
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, static_cast<XAie_ModuleType>(r.Mod),
						XAIE_EVENT_SWITCH_A, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, static_cast<XAie_ModuleType>(r.Mod),
						XAIE_EVENT_SWITCH_B, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
				} else {
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, static_cast<XAie_ModuleType>(r.Mod),
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
		std::vector<XAie_UserRsc> vRscs; /**< broadcast channel allocated resources */
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
		static AieRC setRscs(std::shared_ptr<XAieDevHandle> Dev,
				const std::vector<XAie_LocType> &vL,
				XAie_ModuleType startM, XAie_ModuleType endM,
				std::vector<XAie_UserRsc> &vR) {
			uint16_t bits;
			int bci = -1;

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
				uint32_t TType;

				R.RscType = XAIE_BCAST_CHANNEL_RSC;

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

				//check if shim
				if (R.Loc.Row == 0) {
					R.Mod = XAIE_PL_MOD;
					vR.push_back(R);
				} else {
					R.Mod = XAIE_CORE_MOD;
					vR.push_back(R);
					R.Mod = XAIE_MEM_MOD;
					vR.push_back(R);
				}

				if (i != vL.size() - 1) {
					if (vL[i+1].Row == vL[i].Row &&
						vL[i+1].Col == vL[i].Col) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"BC: duplicated tiles in the vector." <<
							std::endl;
						return XAIE_INVALID_ARGS;
					}
					if (vL[i+1].Row != vL[i].Row) {
						if ((vL[i+1].Col != vL[i].Col) ||
							(vL[i+1].Row > vL[i].Row &&
							 (vL[i+1].Row - vL[i].Row) > 1) ||
							(vL[i+1].Row < vL[i].Row &&
							 (vL[i].Row - vL[i+1].Row) > 1)) {
							Logger::log(LogLevel::ERROR) << __func__ <<
								"BC: discontinuous input tiles." <<
								std::endl;
							return XAIE_INVALID_ARGS;
						}
					} else {
						if ((vL[i+1].Col > vL[i].Col &&
						     (vL[i+1].Col - vL[i].Col) > 1) ||
						    (vL[i+1].Col < vL[i].Col &&
						     (vL[i].Col - vL[i+1].Col) > 1)) {
							Logger::log(LogLevel::ERROR) << __func__ <<
								"BC: discontinuous input tiles." <<
								std::endl;
							return XAIE_INVALID_ARGS;
						}
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
	};
}
