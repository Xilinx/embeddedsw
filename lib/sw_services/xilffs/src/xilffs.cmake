# Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

SET(XILFFS_fs_interface 1 CACHE STRING "Enables file system with selected interface. Enter 1 for SD. Enter 2 for RAM")
SET_PROPERTY(CACHE XILFFS_fs_interface PROPERTY STRINGS 1 2)
option(XILFFS_read_only "Enables the file system in Read_Only mode if true. ZynqMP fsbl will set this to true" OFF)
option(XILFFS_enable_exfat "0:Disable exFAT, 1:Enable exFAT(Also Enables LFN)" OFF)
SET(XILFFS_use_lfn 0 CACHE STRING "Enables the Long File Name(LFN) support if non-zero. Disabled by default: 0, LFN with static working buffer: 1, Dynamic working buffer: 2 (on stack) or 3 (on heap)")
SET_PROPERTY(CACHE XILFFS_use_lfn PROPERTY STRINGS 0 1 2 3)
option(XILFFS_use_mkfs "Disable(0) or Enable(1) f_mkfs function. ZynqMP fsbl will set this to false" ON)
option(XILFFS_use_trim "Disable(0) or Enable(1) TRIM function. ZynqMP fsbl will set this to false" OFF)
option(XILFFS_enable_multi_partition "0:Single partition, 1:Enable multiple partition" OFF)
SET(XILFFS_num_logical_vol 35 CACHE STRING "Number of volumes (logical drives, from 1 to 175) to be used.")
SET(XILFFS_use_strfunc 0 CACHE STRING "Enables the string functions (valid values 0 to 2).")
SET_PROPERTY(CACHE XILFFS_use_strfunc PROPERTY STRINGS 0 1 2)
SET(XILFFS_set_fs_rpath	 0 CACHE STRING "Configures relative path feature (valid values 0 to 2).")
SET_PROPERTY(CACHE XILFFS_set_fs_rpath PROPERTY STRINGS 0 1 2)
option(XILFFS_word_access "Enables word access for misaligned memory access platform" ON)
option(XILFFS_use_chmod "Enables use of CHMOD functionality for changing attributes (valid only with read_only set to false)" OFF)
SET(XILFFS_max_sector_size 4096 CACHE STRING "Maximum Sector size(valid values are 4096, 8192, 16384, 32768)")

SET(XILFFS_ramfs_size 3145728 CACHE STRING "RAM FS size")
SET(XILFFS_ramfs_start_addr CACHE STRING "RAM FS start address")

if (${XILFFS_fs_interface} EQUAL 2)
	set(FILE_SYSTEM_INTERFACE_RAM " ")
        set(RAMFS_SIZE ${XILFFS_ramfs_size})
        if(${XILFFS_ramfs_start_addr})
		set(RAMFS_START_ADDR ${XILFFS_ramfs_start_addr})
	else()
		set(RAMFS_START_ADDR 0x10000000)
	endif()
endif()

if (${XILFFS_fs_interface} EQUAL 1)
	set(FILE_SYSTEM_INTERFACE_SD " ")
endif()

if (${XILFFS_fs_interface})
	if (${XILFFS_read_only})
		set(FILE_SYSTEM_READ_ONLY " ")
	endif()
	if (${XILFFS_enable_exfat})
		set(XILFFS_use_lfn 1)
		set(FILE_SYSTEM_FS_EXFAT " ")
	endif()
	if (${XILFFS_use_lfn} GREATER 0)
		set(FILE_SYSTEM_USE_LFN ${XILFFS_use_lfn})
	endif()
	if (${XILFFS_use_mkfs})
		set(FILE_SYSTEM_USE_MKFS " ")
	endif()
	if (${XILFFS_enable_multi_partition})
		set(FILE_SYSTEM_MULTI_PARTITION " ")
	endif()
	if (${XILFFS_use_trim})
		set(FILE_SYSTEM_USE_TRIM " ")
	endif()
	if (${XILFFS_use_chmod})
		if (${XILFFS_read_only})
			message("WARNING : Cannot Enable CHMOD in read only mode\n")
			message("WARNING : File System supports only up to 175 logical drives Setting back the num of vol to 175\n")
		else()
			set(FILE_SYSTEM_USE_CHMOD " ")
		endif()
	endif()

	if (${XILFFS_num_logical_vol})
		if (${XILFFS_num_logical_vol} GREATER 175)
			message("WARNING : File System supports only up to 175 logical drives Setting back the num of vol to 175\n")
			set(FILE_SYSTEM_NUM_LOGIC_VOL 175)
		else()
			set(FILE_SYSTEM_NUM_LOGIC_VOL ${XILFFS_num_logical_vol})
		endif()
	endif()

	if (${XILFFS_max_sector_size})
		set(FILE_SYSTEM_MAX_SECTOR_SIZE ${XILFFS_max_sector_size})
	else()
		set(FILE_SYSTEM_MAX_SECTOR_SIZE 4096)
	endif()

	if (${XILFFS_use_strfunc})
		set(FILE_SYSTEM_USE_STRFUNC ${XILFFS_use_strfunc})
	endif()
	if (${XILFFS_set_fs_rpath})
		set(FILE_SYSTEM_SET_FS_RPATH ${XILFFS_set_fs_rpath})
	endif()

	if((NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") AND
           (NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv") AND
           (NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") AND
	   (NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") AND
	   (NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze") AND
	   (${XILFFS_word_access}))
		set(FILE_SYSTEM_WORD_ACCESS " ")
	endif()

endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilffs_config.h.in ${CMAKE_BINARY_DIR}/include/xilffs_config.h)
