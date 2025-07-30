// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_frmbufrd_linux.c
* @addtogroup v_frmbuf_rd Overview
*/
#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_frmbufrd.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/
// Structure to hold address and size for each UIO memory map
typedef struct {
    u32 addr;  // Physical address of the memory map
    u32 size;  // Size of the memory map in bytes
} XV_frmbufrd_uio_map;

// Structure to hold UIO device information and memory map details
typedef struct {
    int  uio_fd;                                 // File descriptor for /dev/uio device
    int  uio_num;                                // UIO device number
    char name[ MAX_UIO_NAME_SIZE ];              // Device name string
    char version[ MAX_UIO_NAME_SIZE ];           // Device version string
    XV_frmbufrd_uio_map maps[ MAX_UIO_MAPS ];    // Array of memory map info
} XV_frmbufrd_uio_info;

/***************** Variable Definitions **************************************/
static XV_frmbufrd_uio_info uio_info;

/************************** Function Implementation *************************/
/**
 * @brief Reads a single line from a file into a buffer.
 *
 * This function opens the specified file, reads the first line into the provided
 * buffer, and removes any trailing newline character. The buffer size is limited
 * to MAX_UIO_NAME_SIZE. Returns 0 on success, or a negative error code on failure.
 *
 * @param filename Path to the file to read from.
 * @param linebuf  Buffer to store the read line.
 *
 * @return 0 on success,
 *        -1 if the file cannot be opened,
 *        -2 if the line cannot be read.
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
 * @brief Reads the name of a UIO device and stores it in the provided info structure.
 *
 * This function constructs the path to the name file of the specified UIO device,
 * reads the name string from the file, and stores it in the `name` field of the
 * given `XV_frmbufrd_uio_info` structure.
 *
 * @param info Pointer to an XV_frmbufrd_uio_info structure containing the UIO device number
 *             and a buffer to store the name string.
 *
 * @return 0 on success, or a negative error code on failure.
 */
 static int uio_info_read_name(XV_frmbufrd_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/**
 * @brief Reads the version information of a UIO device and stores it in the provided info structure.
 *
 * This function constructs the path to the version file of the specified UIO device,
 * reads the version string from the file, and stores it in the `version` field of the
 * given `XV_frmbufrd_uio_info` structure.
 *
 * @param info Pointer to an XV_frmbufrd_uio_info structure containing the UIO device number
 *             and a buffer to store the version string.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_version(XV_frmbufrd_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/**
 * Reads the physical address of the specified memory map for a UIO device.
 *
 * This function constructs the path to the sysfs file containing the address
 * of the specified map (by index `n`) for the UIO device identified by
 * `info->uio_num`. It then opens the file, reads the address, and stores it
 * in `info->maps[n].addr`. If the file cannot be opened or the address cannot
 * be read, an error code is returned.
 *
 * @param info Pointer to the XV_frmbufrd_uio_info structure containing UIO device information.
 * @param n    Index of the map whose address is to be read.
 *
 * @return     0 on success,
 *            -1 if the address file cannot be opened,
 *            -2 if the address cannot be read from the file.
 */

static int uio_info_read_map_addr(XV_frmbufrd_uio_info* info, int n) {
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

static int uio_info_read_map_size(XV_frmbufrd_uio_info* info, int n) {
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
 * This function initializes the XV_frmbufrd instance for Linux UIO devices.
 *
 * This function scans the /sys/class/uio directory to find a UIO device that matches
 * the specified InstanceName. If found, it reads the device's information, opens the
 * corresponding /dev/uio device file, and memory-maps the control interface. The
 * function sets up the XV_frmbufrd instance for further use.
 *
 * @param InstancePtr   Pointer to the XV_frmbufrd instance to initialize.
 * @param InstanceName  Name of the UIO device instance to match.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the specified device is not found.
 *   - XST_OPEN_DEVICE_FAILED if the device file cannot be opened.
 *
 * @note
 *   - The function asserts that InstancePtr is not NULL.
 *   - The control interface is expected to be mapped to uioX/map0.
 *   - The function assumes that the UIO device and its maps are properly configured in sysfs.
 */
int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, const char* InstanceName) {
    XV_frmbufrd_uio_info *InfoPtr = &uio_info;
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
 * @brief Releases resources allocated for the XV_frmbufrd instance.
 *
 * This function unmaps the memory region associated with the frame buffer reader
 * and closes the corresponding UIO file descriptor. It should be called when the
 * XV_frmbufrd instance is no longer needed to free system resources.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance to be released.
 *
 * @return
 *   - XST_SUCCESS on successful release of resources.
 *
 * @note
 *   The function asserts that InstancePtr is not NULL and that the instance is ready.
 */
int XV_frmbufrd_Release(XV_frmbufrd *InstancePtr) {
    XV_frmbufrd_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
