// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
/**
 * @file xv_warp_filter_linux.c
 * @addtogroup v_warp_filter Overview
 */

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_warp_filter.h"

/************************** Constant Definitions *****************************/
/** Maximum path size for UIO device paths */
#define MAX_UIO_PATH_SIZE       256
/** Maximum name size for UIO device names */
#define MAX_UIO_NAME_SIZE       64
/** Maximum number of memory maps per UIO device */
#define MAX_UIO_MAPS            5
/** Invalid address value for UIO mapping */
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions *******************************/
/**
 * @brief UIO memory map structure.
 *
 * This structure stores information about a single memory mapping
 * for a UIO device.
 */
typedef struct {
    u64 addr;  /**< Physical address of the memory map */
    u32 size;  /**< Size of the memory map in bytes */
} XV_warp_filter_uio_map;

/**
 * @brief UIO device information structure.
 *
 * This structure contains all the information needed to access a UIO device,
 * including file descriptor, device number, name, and memory mappings.
 */
typedef struct {
    int  uio_fd;                         /**< File descriptor for the UIO device */
    int  uio_num;                        /**< UIO device number */
    char name[MAX_UIO_NAME_SIZE];        /**< Device name */
    char version[MAX_UIO_NAME_SIZE];     /**< Device version string */
    XV_warp_filter_uio_map maps[MAX_UIO_MAPS];  /**< Array of memory maps */
} XV_warp_filter_uio_info;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int line_from_file(char* filename, char* linebuf);
static int uio_info_read_name(XV_warp_filter_uio_info* info);
static int uio_info_read_version(XV_warp_filter_uio_info* info);
static int uio_info_read_map_addr(XV_warp_filter_uio_info* info, int n);
static int uio_info_read_map_size(XV_warp_filter_uio_info* info, int n);

/************************** Variable Definitions *****************************/
/** Global UIO device information */
static XV_warp_filter_uio_info uio_info;

/************************** Function Definitions *****************************/
/**
 * @brief Reads a single line from a file.
 *
 * This function opens a text file, reads the first line, removes any newline
 * character, and stores the result in the provided buffer.
 *
 * @param filename Path to the file to read.
 * @param linebuf Buffer to store the line read from the file.
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails.
 */
static int line_from_file(char* filename, char* linebuf) {
    char* s;
    int i;
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;
    s = fgets(linebuf, MAX_UIO_NAME_SIZE, fp);
    fclose(fp);
    if (!s) return -2;
    for (i=0; (*s)&&(i<MAX_UIO_NAME_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return 0;
}

/**
 * @brief Reads the UIO device name from sysfs.
 *
 * This function reads the device name from /sys/class/uio/uioX/name
 * and stores it in the info structure.
 *
 * @param info Pointer to the UIO device information structure.
 *
 * @return 0 on success, negative value on failure.
 */
static int uio_info_read_name(XV_warp_filter_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/**
 * @brief Reads the UIO device version from sysfs.
 *
 * This function reads the device version string from /sys/class/uio/uioX/version
 * and stores it in the info structure.
 *
 * @param info Pointer to the UIO device information structure.
 *
 * @return 0 on success, negative value on failure.
 */
static int uio_info_read_version(XV_warp_filter_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/**
 * @brief Reads the physical address of a UIO memory map from sysfs.
 *
 * This function reads the physical memory address for a specific map
 * from /sys/class/uio/uioX/maps/mapN/addr.
 *
 * @param info Pointer to the UIO device information structure.
 * @param n Index of the memory map to read (0 to MAX_UIO_MAPS-1).
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails.
 */
static int uio_info_read_map_addr(XV_warp_filter_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    info->maps[n].addr = UIO_INVALID_ADDR;
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/addr", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].addr);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

/**
 * @brief Reads the size of a UIO memory map from sysfs.
 *
 * This function reads the size of a specific memory map
 * from /sys/class/uio/uioX/maps/mapN/size.
 *
 * @param info Pointer to the UIO device information structure.
 * @param n Index of the memory map to read (0 to MAX_UIO_MAPS-1).
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails.
 */
static int uio_info_read_map_size(XV_warp_filter_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/size", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].size);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

/**
 * @brief Initializes the Warp Filter driver on Linux using UIO.
 *
 * This function searches for the UIO device by name, opens the device,
 * reads device information from sysfs, and maps the control register space
 * into user space memory.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param InstanceName Name of the UIO device to search for.
 *
 * @return XST_SUCCESS on successful initialization.
 * @return XST_DEVICE_NOT_FOUND if the device cannot be found.
 * @return XST_OPEN_DEVICE_FAILED if the device cannot be opened.
 *
 * @note The control interface (map0) is mapped to user space.
 * @note InstancePtr must not be NULL.
 */
int XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, const char* InstanceName) {
	XV_warp_filter_uio_info *InfoPtr = &uio_info;
	struct dirent **namelist;
    int i, n;
    char* s;
    char file[ MAX_UIO_PATH_SIZE ];
    char name[ MAX_UIO_NAME_SIZE ];
    int flag = 0;

    assert(InstancePtr != NULL);

    n = scandir("/sys/class/uio", &namelist, 0, alphasort);
    if (n < 0)  return XST_DEVICE_NOT_FOUND;
    for (i = 0;  i < n; i++) {
	strcpy(file, "/sys/class/uio/");
	strcat(file, namelist[i]->d_name);
	strcat(file, "/name");
        if ((line_from_file(file, name) == 0) && (strcmp(name, InstanceName) == 0)) {
            flag = 1;
            s = namelist[i]->d_name;
            s += 3; // "uio"
            InfoPtr->uio_num = atoi(s);
            break;
        }
    }
    if (flag == 0)  return XST_DEVICE_NOT_FOUND;

    uio_info_read_name(InfoPtr);
    uio_info_read_version(InfoPtr);
    for (n = 0; n < MAX_UIO_MAPS; ++n) {
        uio_info_read_map_addr(InfoPtr, n);
        uio_info_read_map_size(InfoPtr, n);
    }

    sprintf(file, "/dev/uio%d", InfoPtr->uio_num);
    if ((InfoPtr->uio_fd = open(file, O_RDWR)) < 0) {
        return XST_OPEN_DEVICE_FAILED;
    }

    // NOTE: slave interface 'Control' should be mapped to uioX/map0
    InstancePtr->Control_BaseAddress = (u64)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Control_BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/**
 * @brief Releases resources allocated by the Warp Filter driver.
 *
 * This function unmaps the memory-mapped control register space
 * and closes the UIO device file descriptor.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance to release.
 *
 * @return XST_SUCCESS on successful release.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 */
int XV_warp_filter_Release(XV_warp_filter *InstancePtr) {
	XV_warp_filter_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Control_BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
