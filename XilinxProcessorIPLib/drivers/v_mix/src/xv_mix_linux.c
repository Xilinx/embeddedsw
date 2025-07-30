// ==============================================================
// Copyright (c) 2015 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_mix_linux.c
* @addtogroup v_mix Overview
*/

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_mix.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/
// Structure to hold address and size information for each UIO memory map
typedef struct {
    u32 addr; // Base address of the memory map
    u32 size; // Size of the memory map
} XV_mix_uio_map;

// Structure to hold UIO device information, including file descriptor, device number,
// name, version, and memory map details for each region.
typedef struct {
    int  uio_fd;                                 // File descriptor for the opened UIO device
    int  uio_num;                                // UIO device number (e.g., for /dev/uioX)
    char name[ MAX_UIO_NAME_SIZE ];              // Device name string
    char version[ MAX_UIO_NAME_SIZE ];           // Device version string
    XV_mix_uio_map maps[ MAX_UIO_MAPS ];         // Array of memory map info structures
} XV_mix_uio_info;

/***************** Variable Definitions **************************************/
static XV_mix_uio_info uio_info;

/************************** Function Implementation *************************/
/**
 * @brief Reads the first line from a file into a buffer.
 *
 * This function opens the specified file, reads the first line into the provided
 * buffer, and removes the trailing newline character if present. The buffer size
 * is limited by MAX_UIO_NAME_SIZE.
 *
 * @param filename Path to the file to read from.
 * @param linebuf Pointer to the buffer where the line will be stored.
 * @return 0 on success,
 *         -1 if the file could not be opened,
 *         -2 if reading from the file failed.
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
 * @brief Reads the name of the UIO device and stores it in the provided info structure.
 *
 * This function constructs the path to the UIO device's name file in the sysfs
 * (e.g., "/sys/class/uio/uioX/name" where X is the UIO device number), reads
 * the device name from the file, and stores it in the 'name' field of the
 * provided XV_mix_uio_info structure.
 *
 * @param info Pointer to an XV_mix_uio_info structure containing the UIO device number.
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_name(XV_mix_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/**
 * @brief Reads the version of the UIO device and stores it in the provided info structure.
 *
 * This function constructs the path to the UIO device's version file in the sysfs
 * (e.g., "/sys/class/uio/uioX/version" where X is the UIO device number), reads
 * the device version from the file, and stores it in the 'version' field of the
 * provided XV_mix_uio_info structure.
 *
 * @param info Pointer to an XV_mix_uio_info structure containing the UIO device number.
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_version(XV_mix_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/**
 * Reads the physical address of the specified UIO map from sysfs and stores it in the info structure.
 *
 * @param info Pointer to the XV_mix_uio_info structure where the map address will be stored.
 * @param n    Index of the map to read the address for.
 * @return     0 on success,
 *            -1 if the address file could not be opened,
 *            -2 if the address could not be read from the file.
 *
 * This function constructs the sysfs path for the UIO map address, opens the file,
 * reads the address, and stores it in info->maps[n].addr. If any step fails, an error code is returned.
 */
static int uio_info_read_map_addr(XV_mix_uio_info* info, int n) {
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
 * @brief Reads the size of a specific memory map for a UIO device.
 *
 * This function constructs the path to the sysfs file that contains the size
 * of the specified map for the given UIO device, opens the file, and reads
 * the size value into the corresponding entry in the info structure.
 *
 * @param info Pointer to the XV_mix_uio_info structure containing UIO device information.
 * @param n    Index of the map whose size is to be read.
 *
 * @return 0 on success,
 *         -1 if the sysfs file cannot be opened,
 *         -2 if reading the size from the file fails.
 */
static int uio_info_read_map_size(XV_mix_uio_info* info, int n) {
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
 * XV_mix_Initialize - Initialize the XV_mix instance for use in Linux.
 *
 * This function scans the /sys/class/uio directory to find a UIO device
 * matching the given InstanceName. If found, it reads the UIO device's
 * information, opens the corresponding device file, and memory-maps the
 * control interface. The function sets up the XV_mix instance structure
 * for further use.
 *
 * @param InstancePtr   Pointer to the XV_mix instance to initialize.
 * @param InstanceName  Name of the UIO device instance to search for.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the specified device is not found.
 *   - XST_OPEN_DEVICE_FAILED if the device file cannot be opened.
 *
 * Note:
 *   - The function asserts that InstancePtr is not NULL.
 *   - The control interface is expected to be mapped to uioX/map0.
 *   - The function assumes that the UIO device exposes the required
 *     sysfs attributes and device file.
 */
int XV_mix_Initialize(XV_mix *InstancePtr, const char* InstanceName) {
	XV_mix_uio_info *InfoPtr = &uio_info;
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

    // NOTE: slave interface 'Ctrl' should be mapped to uioX/map0
    InstancePtr->Config.BaseAddress = (u32)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Config.BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/**
 * Releases resources allocated for the XV_mix instance.
 * @param InstancePtr Pointer to the XV_mix instance to be released.
 *
 * This function unmaps the memory region associated with the XV_mix hardware
 * instance and closes the corresponding UIO file descriptor. It asserts that
 * the provided instance pointer is valid and that the instance is ready before
 * performing the release operations.
 *
 * @return XST_SUCCESS on successful release of resources.
 */
int XV_mix_Release(XV_mix *InstancePtr) {
	XV_mix_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
