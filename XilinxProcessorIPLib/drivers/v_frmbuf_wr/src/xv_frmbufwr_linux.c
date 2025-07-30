// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * *
* @file xv_frmbufwr_linux.c
* @addtogroup v_frmbuf_wr Overview
*
**/
#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_frmbufwr.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/

// Structure to hold address and size information for each UIO memory map
typedef struct {
    u32 addr;
    u32 size;
} XV_frmbufwr_uio_map;

// Structure to hold UIO device information including file descriptor, device number, name, version, and memory maps
typedef struct {
    int  uio_fd;
    int  uio_num;
    char name[ MAX_UIO_NAME_SIZE ];
    char version[ MAX_UIO_NAME_SIZE ];
    XV_frmbufwr_uio_map maps[ MAX_UIO_MAPS ];
} XV_frmbufwr_uio_info;

/***************** Variable Definitions **************************************/
static XV_frmbufwr_uio_info uio_info;

/************************** Function Implementation *************************/
/**
 * @brief Reads a single line from a file into a buffer.
 *
 * This function opens the specified file, reads one line (up to MAX_UIO_NAME_SIZE characters)
 * into the provided buffer, and removes the trailing newline character if present.
 *
 * @param filename The path to the file to read from.
 * @param linebuf Pointer to the buffer where the line will be stored.
 *                The buffer must be at least MAX_UIO_NAME_SIZE bytes long.
 * @return 0 on success,
 *         -1 if the file could not be opened,
 *         -2 if no line could be read from the file.
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
 * Reads the name of the UIO device corresponding to the given info structure.
 *
 * This function constructs the path to the 'name' file for the specified UIO device
 * (using the uio_num field of the info structure), reads the device name from the file,
 * and stores it in the info->name field.
 *
 * @param info Pointer to an XV_frmbufwr_uio_info structure containing the UIO device number.
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_name(XV_frmbufwr_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/**
 * Reads the version of the UIO device corresponding to the given info structure.
 *
 * This function constructs the path to the 'version' file for the specified UIO device
 * (using the uio_num field of the info structure), reads the device version from the file,
 * and stores it in the info->version field.
 *
 * @param info Pointer to an XV_frmbufwr_uio_info structure containing the UIO device number.
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_version(XV_frmbufwr_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/**
 * Reads the physical address of the specified memory map for a UIO device.
 *
 * This function constructs the path to the sysfs file that contains the address
 * of the specified map (by index `n`) for the UIO device identified by `info->uio_num`.
 * It then opens the file, reads the address, and stores it in `info->maps[n].addr`.
 *
 * @param info Pointer to the XV_frmbufwr_uio_info structure containing UIO device information.
 * @param n    Index of the map whose address is to be read.
 * @return     0 on success,
 *            -1 if the address file could not be opened,
 *            -2 if the address could not be read from the file.
 */
static int uio_info_read_map_addr(XV_frmbufwr_uio_info* info, int n) {
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
 * Reads the size of the specified UIO memory map from sysfs and stores it in the info structure.
 *
 * @param info Pointer to the XV_frmbufwr_uio_info structure containing UIO device information.
 * @param n    Index of the memory map to read the size for.
 * @return     0 on success,
 *            -1 if the size file could not be opened,
 *            -2 if reading the size from the file failed.
 *
 * The function constructs the path to the sysfs file representing the size of the
 * specified UIO memory map, opens the file, reads the size (in hexadecimal format),
 * and stores it in info->maps[n].size. It handles errors related to file access and parsing.
 */
static int uio_info_read_map_size(XV_frmbufwr_uio_info* info, int n) {
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
 * XV_frmbufwr_Initialize - Initialize the XV_frmbufwr instance for Linux UIO.
 *
 * This function scans the /sys/class/uio directory to find a UIO device
 * matching the given InstanceName. If found, it reads the device's UIO
 * information, opens the corresponding /dev/uio device, and maps the
 * control interface into user space. The function sets up the instance
 * structure and marks it as ready.
 *
 * @param InstancePtr   Pointer to the XV_frmbufwr instance to initialize.
 * @param InstanceName  Name of the UIO device instance to match.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the device with the given name is not found.
 *   - XST_OPEN_DEVICE_FAILED if the UIO device file cannot be opened.
 *
 * Note:
 *   - The function asserts that InstancePtr is not NULL.
 *   - The control interface is expected to be mapped to uioX/map0.
 *   - The function assumes that supporting helper functions such as
 *     line_from_file, uio_info_read_name, uio_info_read_version,
 *     uio_info_read_map_addr, and uio_info_read_map_size are implemented.
 */
int XV_frmbufwr_Initialize(XV_frmbufwr *InstancePtr, const char* InstanceName) {
    XV_frmbufwr_uio_info *InfoPtr = &uio_info;
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
 * Releases resources associated with the XV_frmbufwr instance.
 *
 * This function unmaps the memory region mapped for the frame buffer writer
 * and closes the associated UIO file descriptor. It should be called when
 * the XV_frmbufwr instance is no longer needed to free system resources.
 *
 * @param  InstancePtr  Pointer to the XV_frmbufwr instance to be released.
 *                      Must not be NULL and must be initialized.
 *
 * @return XST_SUCCESS on successful release of resources.
 */
int XV_frmbufwr_Release(XV_frmbufwr *InstancePtr) {
    XV_frmbufwr_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
