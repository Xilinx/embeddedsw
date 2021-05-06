// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @param file xaiefal-rsc-base.hpp
 * Base classes for AI engine resources management
 */

#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/common/xaiefal-base.hpp>
#include <xaiefal/common/xaiefal-log.hpp>

#pragma once

namespace xaiefal {
	enum class XAieRscType {
		PCOUNTER,
		BC,
		PCEVENT,
		PCRANGE,
		SSWITCHSELECT,
		USEREVENT,
		TRACEEVENT,
		COMBOEVENT,
	};

	class XAieRsc;
	/**
	 * struct XAieRscState
	 * State of each AIE resource
	 */
	struct XAieRscState {
		uint8_t Initialized: 1; /**< bit set for resource is initialized */
		uint8_t Configured: 1; /**< bit set for resource is configured */
		uint8_t Reserved: 1; /**< bit set for resource is reserved */
		uint8_t Prereserved: 1; /**< bit set for resource is prereserved
					     Prereserved resource is the resource
					     which are supposed to be allocated
					     at compilation. If user specifies
					     particular hardware resources to use
					     for this resource, this bit will be
					     set. */
		uint8_t Running: 1; /**< bit set for resource is running */
	};

	/**
	 * @class XAieRsc
	 * @brief Base AI engine resource class. It defines resource operations
	 *	  and maintains resource state machine.
	 */
	class XAieRsc {
	public:
		XAieRsc() = delete;
		XAieRsc(std::shared_ptr<XAieDevHandle> DevHd):
			State(), AieHd(DevHd) {
			if (!DevHd) {
				throw std::invalid_argument("aie rsc: empty device handle");
			}
			preferredId = XAIE_RSC_ID_ANY;
		}
		XAieRsc(XAieDev &Dev):
			State(), AieHd(Dev.getDevHandle()) {
				preferredId = XAIE_RSC_ID_ANY;
			}
		virtual ~XAieRsc() {
			if (State.Running == 1) {
				stop();
			} else if (State.Reserved == 1) {
				if (State.Prereserved == 1) {
					free();
				} else {
					release();
				}
			}
		}
		/**
		* This function sets a resource's id to a requested id value.
		* Resources that can have their id set always have XAIE_RSC_ID_ANY as
		* a default value.
		*
		* @param id requested id for resource
		* @param isStatticAllocated to indicate if the resource has been allocated at static time.
		*
		* @return XAIE_OK for success, and error code for failure.
		*/

		AieRC setPreferredId(uint32_t id, bool isStaticAllocated = false) {
			AieRC RC;

			if (State.Reserved == 1) {
				RC = XAIE_INVALID_ARGS;
			} else {
				preferredId = id;
				RC = XAIE_OK;
				if (isStaticAllocated) {
					if (id == XAIE_RSC_ID_ANY) {
						RC = XAIE_INVALID_ARGS;
						Logger::log(LogLevel::ERROR) << __func__ << " " <<
							typeid(*this).name() <<
							" If Rsc ID is any, cannot be statically allocated" << std::endl;
					} else {
						State.Prereserved = 1;
					}
				}
			}

			return RC;
		}

		/**
		 * This function reserves the resource.
		 * Once the resource is reserved, the underline hardware
		 * resources belong to this resource will be occupied
		 * until it is released.
		 *
		 * If no prereserved hardware resources have been specified
		 * it allocates resources from SSW AIE driver.
		 *
		 * If prereserved hardware resources have been specified, it
		 * will request to use the specified resources. If the resources
		 * are not statically allocated, SSW AIE driver will return
		 * failure.
		 *
		 * @return XAIE_OK for success, and error code for failure.
		 */
		AieRC reserve() {
			AieRC RC;

			if (State.Reserved == 1) {
				Logger::log(LogLevel::ERROR) << __func__ << " " <<
					typeid(*this).name() << " resource has been allocated." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Initialized == 0) {
				Logger::log(LogLevel::ERROR) << __func__ << " " <<
					typeid(*this).name() << " resource not configured." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = _reserve();
				if (RC == XAIE_OK) {
					State.Reserved = 1;
				}
			}
			return RC;
		}
		/**
		 * This function releases the resource.
		 * Once the resource is released, the underline hardware
		 * resources belong to this resource will be released so that
		 * they can be allcated for other entity.
		 *
		 * @return XAIE_OK for success, and error code for failure.
		 */
		AieRC release() {
			AieRC RC = XAIE_OK;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << __func__ << " " <<
					typeid(*this).name() << "resource is running." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1) {
				RC = _release();
				State.Reserved = 0;
				State.Prereserved = 0;
			}
			return RC;
		}
		/**
		 * This function free the prereserved resource.
		 *
		 * Once the prereserved resource is freed, the underline
		 * hardware resources are freed to be used by other entity
		 * but these hardware resources are still prereserved. That is
		 * if some enity tries to allocate for resource, the prereserved
		 * resources will not be allocated.
		 *
		 * @return XAIE_OK for success, and error code for failure.
		 */
		AieRC free() {
			AieRC RC = XAIE_OK;

			if (State.Prereserved == 0) {
				Logger::log(LogLevel::ERROR) << __func__ << " " <<
					typeid(*this).name() << " resource is not preserved." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				if (State.Running == 1) {
					Logger::log(LogLevel::ERROR) << __func__ << " " <<
						typeid(*this).name() << " resource is running." << std::endl;
					RC = XAIE_INVALID_ARGS;
				} else if (State.Reserved == 1) {
					RC = _free();
					State.Reserved = 0;
				}
			}
			return RC;
		}
		/**
		 * This function starts using the resource by configuring the
		 * hardware registers. It needs to be called after the resource
		 * is reserved.
		 *
		 * @return XAIE_OK for success, and error code for failure.
		 */
		AieRC start() {
			AieRC RC = XAIE_OK;

			if (State.Running == 1) {
				RC = XAIE_OK;
			} else if (State.Reserved == 1) {
				if (State.Configured == 1) {
					RC = _start();
					if (RC == XAIE_OK) {
						State.Running = 1;
					}
				} else {
					Logger::log(LogLevel::ERROR) << __func__ << " " <<
						typeid(*this).name() << " resource is not configured." << std::endl;
					RC = XAIE_ERR;
				}
			} else {
				Logger::log(LogLevel::ERROR) << __func__ << " " <<
					typeid(*this).name() << " resource is not allocated." << std::endl;
				RC = XAIE_ERR;
			}
			return RC;
		}
		/**
		 * This function stops using the resource by removing the config
		 * from hardware. But the resource are still occupied after this
		 * funtion.
		 *
		 * @return XAIE_OK for success, and error code for failure.
		 */
		AieRC stop() {
			AieRC RC = XAIE_OK;

			if (State.Running == 0) {
				RC = XAIE_OK;
			} else {
				RC = _stop();
				if (RC == XAIE_OK) {
					State.Running = 0;
				}
			}
			return RC;
		}
		/**
		 * This function checks if resource has been initialized.
		 *
		 * @return true if resource has been initialized, false otherwise.
		 */
		bool isInitialized() const {
			return State.Initialized;
		}
		/**
		 * This function checks if resource has been reserved.
		 *
		 * @return true if resource has been reserved, false otherwise.
		 */
		bool isReserved() const {
			return State.Reserved;
		}
		/**
		 * This function checks if resource has been configured.
		 *
		 * @return true if resource configure in software is done,
		 *	   false otherwise.
		 */
		bool isConfigured() const {
			return State.Configured;
		}
		/**
		 * This function checks if resource is in use.
		 *
		 * @return true if resource is in use, falsei otherwise.
		 */
		bool isRunning() const {
			return State.Running;
		}
		/**
		 * This function sets function name this resource is used for.
		 */
		void setFuncName(const std::string &Name) {
			FuncName = Name;
		}
		/**
		 * This function returns function name this resource used for.
		 *
		 * @return function name
		 */
		const std::string& getFuncName() const {
			return FuncName;
		}

		/**
		 * This funtion returns AI engine device
		 */
		XAie_DevInst *dev() {
			return AieHd->dev();
		}

	protected:
		XAieRscState State; /**< resource state */
		std::string FuncName; /**< function name which resource is used
					   for */
		std::shared_ptr<XAieDevHandle> AieHd; /**< AI engine device instance */
		uint32_t preferredId; /**< preferred resource Id*/
	private:
		/**
		 * This function will be called by reserve(). It allows child
		 * class to implement its own resource reservation.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		virtual AieRC _reserve() {return XAIE_OK;}
		/**
		 * This function will be called by release(). It allows child
		 * class to implement its own resource release.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		virtual AieRC _release() {return XAIE_OK;}
		/**
		 * This function will be called by free(). It allows child
		 * class to implement its own prereserved resource free.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		virtual AieRC _free() {return XAIE_OK;}
		/**
		 * This function will be called by start(). It allows child
		 * class to implement its own resource start.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		virtual AieRC _start() {return XAIE_OK;}
		/**
		 * This function will be called by stop(). It allows child
		 * class to implement its own resource stop.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		virtual AieRC _stop() {return XAIE_OK;}
	public:
		/**
		 * this function should goes into c driver
		 */
		static int check_rsc_avail_bit(uint64_t *bits, int start, int bits2check) {
			uint64_t *sbits = &bits[start / 64];
			int ret = -1;

			do {
				int lstart, lend;
				uint64_t mask;

				lstart = start % 64;
				if (bits2check > (int)(64 - lstart)) {
					lend = 64 - 1;
				} else {
					lend = lstart + bits2check - 1;
				}
				mask = 1 << lstart;
				for (int b = lstart; b <= lend; b++, mask <<= 1) {
					if ((*sbits & mask) == 0) {
						ret = b + start - lstart;
						break;
					}
				}
				start += lend - lstart + 1;
				bits2check -= lend - lstart + 1;
				sbits += 1;
			} while(ret < 0 && bits2check > 0);

			return ret;
		}
		/**
		 * this function should goes into c driver
		 */
		static void set_rsc_bit(uint64_t *bits, int bit) {
			bits[bit/64] |= 1 << (bit % 64);
		}
		/**
		 * this function should goes into c driver
		 */
		static void clear_rsc_bit(uint64_t *bits, int bit) {
			bits[bit/64] &= ~(1 << (bit % 64));
		}
		/**
		 * this function should goes into c driver
		 */
		static int alloc_rsc_bit(uint64_t *bits, int start, int bits2check) {
			int bit = check_rsc_avail_bit(bits, start, bits2check);

			if (bit >= 0) {
				set_rsc_bit(bits, bit);
			}
			return bit;
		}
	};

	/**
	 * @class XAieSingleTileRsc
	 * @brief class of resource of a single tile
	 */
	class XAieSingleTileRsc: public XAieRsc {
	public:
		XAieSingleTileRsc() = delete;
		XAieSingleTileRsc(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L, XAie_ModuleType M):
			XAieRsc(DevHd), Loc(L), Mod(M) {
			if (_XAie_CheckModule(AieHd->dev(), Loc, M) !=
				XAIE_OK) {
				throw std::invalid_argument("invalid module tile");
			}
		}
		XAieSingleTileRsc(XAieDev &Dev,
			XAie_LocType L, XAie_ModuleType M):
			XAieSingleTileRsc(Dev.getDevHandle(), L, M) {}
		XAieSingleTileRsc(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType L):
			XAieRsc(DevHd), Loc(L) {
			uint8_t TType = _XAie_GetTileTypefromLoc(
					AieHd->dev(), L);
			if (TType == XAIEGBL_TILE_TYPE_MAX) {
				throw std::invalid_argument("Invalid tile");
			}
			if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
				Mod = XAIE_CORE_MOD;
			} else if (TType == XAIEGBL_TILE_TYPE_SHIMPL ||
					TType == XAIEGBL_TILE_TYPE_SHIMNOC) {
				Mod = XAIE_PL_MOD;
			} else {
				Mod = XAIE_MEM_MOD;
			}
		}
		XAieSingleTileRsc(XAieDev &Dev, XAie_LocType L):
			XAieSingleTileRsc(Dev.getDevHandle(), L) {}
		virtual ~XAieSingleTileRsc() {}
		/**
		 * This function returns tile location
		 *
		 * @return tile location
		 */
		XAie_LocType loc() const {
			return Loc;
		}
		/**
		 * This function returns resource id
		 *
		 * @param L returns tile location
		 * @param M returns module type
		 * @param I returns resource ID
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getRscId(XAie_LocType &L, XAie_ModuleType &M,
				uint32_t &I) {
			AieRC RC;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << typeid(*this).name() << " " <<
					__func__ << "(" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				L = Loc;
				M = static_cast<XAie_ModuleType>(Rsc.Mod);
				I = Rsc.RscId;
				RC = XAIE_OK;
			}
			return RC;
		}
	protected:
		XAie_LocType Loc; /**< tile location */
		XAie_ModuleType Mod; /**< expected resource module */
		XAie_UserRsc Rsc; /**< resource */
	};
	/**
	 * @class XAieRscGroup
	 * @brief template class to group the resources of the smae function for
	 *	  different tiles.
	 * Each element in the group is a functional resource of a tile.
	 */
	template <class T>
	class XAieRscGroup {
	public:
		XAieRscGroup() = delete;
		XAieRscGroup(std::shared_ptr<XAieDev> Dev, const std::string &Name = ""):
			Aie(Dev), FuncName(Name) {}
		~XAieRscGroup() {}

		/**
		 * This function adds a resource to the resource group.
		 *
		 * @param R AI engine resource shared pointer
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC addRsc(std::shared_ptr<T> R) {
			bool toAdd = true;

			if (R->dev() != Aie->dev()) {
				Logger::log(LogLevel::ERROR) <<
					"failed to add resource to group, ai dev mistached." <<
					std::endl;
				return XAIE_INVALID_ARGS;
			}
			for (auto lR: vRscs) {
				if (lR->loc().Col == R->loc().Col &&
					lR->loc().Row == R->loc().Row) {
					toAdd = false;
					break;
				}
			}
			if (toAdd) {
				vRscs.push_back(R);
			}
			return XAIE_OK;
		}
		/**
		 * This function adds a tile to the resource group.
		 * It will construct resource for the specified tile and add it
		 * to the group.
		 *
		 * @param L Tile location
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC addRsc(XAie_LocType L) {
			bool toAdd = true;

			for (auto R: vRscs) {
				if (R->loc().Col == L.Col &&
					R->loc().Row == L.Row) {
					toAdd = false;
					break;
				}
			}
			if (toAdd) {
				vRscs.push_back(std::make_shared<T>(*(Aie.get()), L));
			}
			return XAIE_OK;
		}
		/**
		 * This function adds a tile to the resource group.
		 * It will construct resource for the specified tile and add it
		 * to the group.
		 *
		 * @param L Tile location
		 * @param Args other arguments such as module type
		 * @return XAIE_OK for success, error code for failure.
		 */
		template <typename... A>
		AieRC addRsc(XAie_LocType L, A... Args) {
			bool toAdd = true;

			for (auto R: vRscs) {
				if (R->loc().Col == L.Col &&
					R->loc().Row == L.Row) {
					toAdd = false;
					break;
				}
			}
			if (toAdd) {
				vRscs.push_back(std::make_shared<T>(*(Aie.get()), L, Args...));
			}
			return XAIE_OK;
		}
		/**
		 * This function adds tiles to the resource group.
		 * It will construct resources for the specified tiles and add
		 * them to the group.
		 *
		 * @param vL Tiles locations vector
		 * @return XAIE_OK.
		 */
		AieRC addRsc(const std::vector<XAie_LocType> &vL) {
			for (auto L: vL) {
				addRsc(L);
			}
			return XAIE_OK;
		}
		/**
		 * This function adds a tile with specified extra argument to
		 * the resource group. E.g. the extra argument can be module type.
		 * It will construct resource for the specified tile and add it
		 * to the group.
		 *
		 * @param vL Tiles location
		 * @param Args other arguments, such as module type
		 * @return XAIE_OK for success, error code for failure.
		 */
		template <typename... A>
		AieRC addRsc(const std::vector<XAie_LocType> &vL, A... Args) {
			for (auto L: vL) {
				addRsc(L, Args...);
			}
			return XAIE_OK;
		}
		/**
		 * This function removes tiles from the resource group.
		 * It will remove the resources of the specified tiles from the
		 * group.
		 *
		 * @param vL Tiles locations vector
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC removeRsc(const std::vector<XAie_LocType> &vL) {
			//TODO: validate tile
			for (auto L: vL) {
				for (auto i = vRscs.begin();
					i != vRscs.end(); ++i) {
					if ((*i)->loc().Col == L.Col &&
						(*i)->loc().Row == L.Row) {
						auto j = i;
						i--;
						vRscs.erase(j);
					}
				}
			}
			return XAIE_OK;
		}
		/**
		 * This function removes tiles from the resource group.
		 * It will remove the resources of the specified tiles from the
		 * group.
		 *
		 * @param L Tile location
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC removeRsc(const XAie_LocType &L) {
			std::vector<XAie_LocType> vL;

			vL.push_back(L);
			return removeRsc(vL);
		}
		/**
		 * This function reserve resources of the tiles in the group.
		 * If this function is called successfully, the resources used
		 * by this group is occupied untile release() function is calle.
		 * As long as one tile failed to reserve the resource, even
		 * though other tiles in the group has successfully reserved the
		 * resource, all tiles in this group will release the reserved
		 * resources before return from this function.
		 *
		 * @param failedL used to return the first tile which failed
		 *	  resource allocation.
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC reserve(XAie_LocType &failedL) {
			AieRC RC;

			 if (vRscs.size() == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed to reserve, no resource specified." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
				for (auto i = vRscs.begin(), e = vRscs.end();
					i != e; ++i) {
					AieRC lRC;

					lRC = (*i)->reserve();
					if (lRC != XAIE_OK) {
						for (auto j = vRscs.begin(),
							ne = vRscs.end();
							j != ne; ++j) {
							(*j)->release();
						}
						Logger::log(LogLevel::ERROR) << __func__ <<
							"failed to reserve tile(" <<
							(uint32_t)(*i)->loc().Col << "," <<
							(uint32_t)(*i)->loc().Row << ")." << std::endl;
						failedL = (*i)->loc();
						RC = XAIE_ERR;
						break;
					}
				}
			}
			return RC;
		}
		/**
		 * This function reserve resources of the tiles in the group.
		 * If this function is called successfully, the resources used
		 * by this group is occupied untile release() function is called.
		 * As long as one tile failed to reserve the resource, even
		 * though other tiles in the group has successfully reserved the
		 * resource, all tiles in this group will release the reserved
		 * resources before return from this function.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC reserve() {
			XAie_LocType failedL;
			return reserve(failedL);
		}
		/**
		 * This function release resources of the tiles in the group.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC release() {
			AieRC RC = XAIE_OK;

			for (auto R: vRscs) {
				AieRC lRC;

				lRC = R->release();
				if (lRC != XAIE_OK) {
					RC = XAIE_ERR;
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed to release tile(" <<
						(uint32_t)R->loc().Col << "," <<
						(uint32_t)R->loc().Row << ")." << std::endl;
				}
			}
			return RC;
		}
		/**
		 * This function starts using the resource by configuring the
		 * hardware.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC start() {
			AieRC RC;

			 if (vRscs.size() == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed to reserve, no resource specified." << std::endl;
				RC = XAIE_ERR;
			} else {
				for (auto i = vRscs.begin(), e = vRscs.end();
					i != e; ++i) {
					RC = (*i)->start();
					if (RC != XAIE_OK) {
						for (auto j = vRscs.begin(),
							ne = vRscs.end();
							j != ne; ++j) {
							(*j)->stop();
						}
						Logger::log(LogLevel::ERROR) << __func__ <<
							"failed to start tile(" <<
							(uint32_t)(*i)->loc().Col << "," <<
							(uint32_t)(*i)->loc().Row << ")." << std::endl;
						break;
					}
				}
			}
			return RC;
		}
		/**
		 * This function stop using the resource by resetting the
		 * configuration in hardware of the physical resources.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC stop() {
			AieRC RC = XAIE_OK;

			for (auto R: vRscs) {
				AieRC lRC;

				lRC = R->stop();
				if (lRC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"failed to stop tile(" <<
						(uint32_t)R->loc().Col << "," <<
						(uint32_t)R->loc().Row << ")." << std::endl;
					RC = lRC;
				}
			}
			return RC;
		}
		/**
		 * This function checks if the resources of the group is
		 * reserved.
		 *
		 * @return true if all of resources of the group are reserved,
		 *	   false otherwise.
		 */
		bool isReserved() const {
			bool Ret = true;

			if (vRscs.size() == 0) {
				Ret = false;
			} else {
				for (auto R: vRscs) {
					if (R->isReserved() == false) {
						Ret = false;
						break;
					}
				}
			}
			return Ret;
		}
		/**
		 * This function checks if the resources of the group is
		 * in use.
		 *
		 * @return true if all of resources of the group are in use,
		 *	   false otherwise.
		 */
		bool isRunning() const {
			bool Ret = true;

			if (vRscs.size() == 0) {
				Ret = false;
			} else {
				for (auto R: vRscs) {
					if (R->isRunning() == false) {
						Ret = false;
						break;
					}
				}
			}
			return Ret;
		}
		/**
		 * This function returns the number of elements of this group.
		 * Each element is a functional resource of a tile.
		 *
		 * @return number of elements of this group
		 */
		size_t size() const {
			return vRscs.size();
		}
		/**
		 * This function clears all elements of this group.
		 * The elements will not be cleared if the resources are
		 * reserved.
		 *
		 * @return XAIE_OK for successs, error code for failure
		 */
		AieRC clear() {
			AieRC RC;
			bool canClear = true;

			for (auto R: vRscs) {
				if (R->isReserved() == true) {
					canClear = false;
					break;
				}
			}
			if (canClear == false) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed resource group, rsources allocated." << std::endl;
				RC = XAIE_ERR;
			} else {
				vRscs.clear();
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function returns reference pointer of the resource
		 * referred by the index.
		 *
		 * @param i resource index
		 * @return weak pointer of the resource for success, weak
		 *	   pointer of null.
		 */
		std::shared_ptr<T> getRef(int i) {
			if (i >= 0 && i < (int)vRscs.size()) {
				return vRscs[i];
			} else {
				return std::shared_ptr<T>();
			}
		}
		/**
		 * This function returns reference to AI resource class object
		 * of the specified tile.
		 *
		 * @return reference to AI resource class object of the
		 *	   specified tile if tile is in the group, reference to
		 *	   nullptr object.
		 */
		T& tile(const XAie_LocType &L) {
			T *R = nullptr;
			for (auto lR: vRscs) {
				if (lR->loc().Col == L.Col &&
					lR->loc().Row == L.Row) {
					R = lR.get();
				}
			}
			return *R;
		}
		T& operator[] (int i) {
			return *(vRscs[i].get());
		}
	private:
		std::vector<std::shared_ptr<T>> vRscs; /**< vector of AI engine resource objects */
		std::shared_ptr<XAieDev> Aie; /**< pointer to AI enigne device */
		std::string FuncName; /**< function name of this group */
	};

	/**
	 * @class XAieRscContainer
	 * @brief class to provide helpers to generate meta data for AI engine
	 *	  resources.
	 *a
	 * It provides functions to generate meta data information for the
	 * resources. These are the functions to print the resource
	 * information to different format.
	 */
	class XAieRscContainer {
	public:
		XAieRscContainer() = delete;
		XAieRscContainer(const std::string &N): Name(N) {}
		~XAieRscContainer() {}
		/**
		 * This function returns the name of this meta data container.
		 * The name of this meta data container is set by the
		 * constructor.
		 *
		 * @return Name of the meta data container
		 */
		const std::string& getName() const {
			return Name;
		}
		/**
		 * This function adds a vector of AI engine resource objects
		 * references to the container.
		 *
		 * @param vR vector of AI engine resource objects whose
		 *	  references to add
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC addRsc(std::vector<std::shared_ptr<XAieRsc>> &vR) {
			for (auto R: vR) {
				addRsc(R);
			}
			return XAIE_OK;
		}
		/**
		 * This function adds a group of AI engine resources references
		 * to the container.
		 *
		 * @param gR group of AI engine resources of which references to
		 *	  add
		 * @return XAIE_OK for success, error code for failure
		 */
		template <class T>
		AieRC addRsc(XAieRscGroup<T> &gR) {
			for (int i = 0; i < (int)gR.size(); i++) {
				bool toAdd = true;
				auto R = gR.getRef(i);

				for (auto Ref: vRefs) {
					if (Ref.lock() == R) {
						toAdd = false;
						break;
					}
				}
				if (toAdd) {
					vRefs.push_back(R);
				}
			}
			return XAIE_OK;
		}
		/**
		 * This function adds AI engine resource object to the meta
		 * data container.
		 *
		 * @param R AI engine resource object reference to add
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC addRsc(std::shared_ptr<XAieRsc> R) {
			bool toAdd = true;

			for (auto Ref: vRefs) {
				if (Ref.lock() == R) {
					toAdd = false;
					break;
				}
			}
			if (toAdd) {
				vRefs.push_back(R);
			}
			return XAIE_OK;
		}
		/**
		 * This function clear all the resources references from the
		 * meta data container.
		 */
		void clear() {
			vRefs.clear();
		}
		/**
		 * This function removes the empty AI engine empty references
		 * from the container.
		 */
		void clean() {
			for (size_t i = 0; i < vRefs.size(); i++) {
				if (vRefs[i].lock() == nullptr) {
					vRefs.erase(vRefs.begin() + i);
				}
			}
		}
		/**
		 * This function reserve resources of the container
		 *
		 * @param vfailedRefs vector to keep the resource references
		 *	  which failed to reserve.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC reserve(std::vector<const XAieRsc *> vfailedRefs) {
			AieRC RC;

			 if (vRefs.size() == 0) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"failed to reserve, no resource references specified." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
				for (auto Ref: vRefs) {
					auto R = Ref.lock();

					if (R != nullptr) {
						AieRC lRC;

						lRC = R->reserve();
						if (lRC != XAIE_OK) {
							RC = XAIE_ERR;
							vfailedRefs.push_back(R.get());
						}
					}
				}
				if (RC != XAIE_OK) {
					for (auto Ref: vRefs) {
						auto R = Ref.lock();

						if (R != nullptr) {
							R->release();
						}
					}
				}
			}
			return RC;
		}
		/**
		 * This function reserve resources of the tiles in the container.
		 * If this function is called successfully, the resources used
		 * by this group is occupied untile release() function is called.
		 * As long as one tile failed to reserve the resource, even
		 * though other tiles in the container has successfully reserved the
		 * resource, all tiles in this container will release the reserved
		 * resources before return from this function.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC reserve() {
			std::vector<const XAieRsc *> vfailedRs;
			return reserve(vfailedRs);
		}
		/**
		 * This function release resources of the tiles in the
		 * container.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC release() {
			AieRC RC = XAIE_OK;

			for (auto Ref: vRefs) {
				AieRC lRC;
				auto R = Ref.lock();

				if (R != nullptr) {
					lRC = R->release();
					if (lRC != XAIE_OK) {
						RC = XAIE_ERR;
					}
				}
			}
			return RC;
		}
		/**
		 * This function starts using the resource by configuring the
		 * hardware.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC start() {
			AieRC RC;

			for (auto Ref: vRefs) {
				auto R = Ref.lock();

				if (R != nullptr) {
					RC = R->start();
					if (RC != XAIE_OK) {
						stop();
						break;
					}
				}
			}
			return RC;
		}
		/**
		 * This function stop using the resource by resetting the
		 * configuration in hardware of the physical resources.
		 *
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC stop() {
			for (auto Ref: vRefs) {
				auto R = Ref.lock();

				if (R != nullptr) {
					R->stop();
				}
			}
			return XAIE_OK;
		}
		/**
		 * This function checks if the resources of the container is
		 * reserved.
		 *
		 * @return true if at least one of resources of the container is
		 *	   reserved, false otherwise.
		 */
		bool isReserved() const {
			bool Ret = false;

			for (auto Ref: vRefs) {
				auto R = Ref.lock();

				if (R != nullptr) {
					if (R->isReserved()) {
						Ret = true;
						break;
					}
				}
			}
			return Ret;
		}
		/**
		 * This function checks if the resources of the container is
		 * in use.
		 *
		 * @return true if at least one of resources of the container is
		 *	   in use, false otherwise.
		 */
		bool isRunning() const {
			bool Ret = false;

			for (auto Ref: vRefs) {
				auto R = Ref.lock();

				if (R != nullptr) {
					if (R->isRunning()) {
						Ret = true;
						break;
					}
				}
			}
			return Ret;
		}
	private:
		std::string Name; /**< Name of the meta data resource container */
		std::vector<std::weak_ptr<XAieRsc>> vRefs;
	};
}
