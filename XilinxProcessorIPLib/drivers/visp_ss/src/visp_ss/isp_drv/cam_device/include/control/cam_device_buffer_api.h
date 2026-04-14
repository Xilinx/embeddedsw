// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#ifndef CAMDEV_BUFFER_API_H
#define CAMDEV_BUFFER_API_H

//#include "media_buffer_pool.h"

#include "types.h"
#include "return_codes.h"
#include "oslayer.h"
#include "cam_device_api.h"
#include "buf_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 01_buf_mgmt_ctrl VsCamBufMgmt E10C01 BufMgmt_Ctrl Definitions
 * @{
 * For the control flow of E10 VsCamBufMgmt, its oeprations of external modules
 * on the buffer queue can be summarized into two types: buffer producer and
 * bufferconsumer.
 * Buffer Producer: Write image data to the memory pointed to by the idle media
 * buffer and deliver the filled media buffer to the full buffer queue.
 * Buffer Consumer: Read image data to the memory pointed to by the filled media
 * buffer and return the used media buffer to the empty buffer queue.
 */

/******************************************************************************/
/**
 * @brief   Timeout for buffer waiting in RDMA mode. Unit: msec.
 *
 *****************************************************************************/
#define CAMDEV_BUFFER_RDMA_WAIT_TIME_OUT 4000

/******************************************************************************/
/**
 * @brief   The buffer chain index enumeration of each output path.
 *
 *****************************************************************************/
typedef enum CamDeviceBufChainId_e {
    CAMDEV_BUFCHAIN_MP        = 0,               /**< ISP output main path buffer chain index*/
    CAMDEV_BUFCHAIN_SP1       = 1,               /**< ISP output self1 path buffer chain index*/
    CAMDEV_BUFCHAIN_SP2       = 2,               /**< ISP output self2 path buffer chain index*/
    CAMDEV_BUFCHAIN_RAW       = 3,               /**< ISP output RAW path buffer chain index*/
    CAMDEV_BUFCHAIN_HDR_RAW   = 4,               /**< ISP output retiming HDR RAW path buffer chain index*/
    CAMDEV_BUFCHAIN_METADATA  = 5,               /**< Warning: CAMDEV_BUFCHAIN_METADATA will be deleted in next release, don't use this index */
    CAMDEV_BUFCHAIN_RDMA      = 6,               /**< ISP input read DMA path buffer chain index*/
    CAMDEV_BUFCHAIN_RETIMING  = 7,               /**< ISP input retiming DMA path buffer chain index*/
    CAMDEV_BUFCHAIN_MAX                          /**< Maximum path buffer chain index */
} CamDeviceBufChainId_t;

/******************************************************************************/
/**
 * @brief   The buffer mode enumeration which can be configured by user application.
 *
 *****************************************************************************/
typedef enum CamDeviceBufMode_e {
    CAMDEV_BUFMODE_INVALID     = 0,          /**< Invalid buffer mode */
    CAMDEV_BUFMODE_USERPTR,                  /**< The buffer allocated from user application*/
    CAMDEV_BUFMODE_RESMEM,                   /**< The buffer allocated from internal reserved memory */
    CAMDEV_BUFMODE_MAX                       /**< Maximum buffer mode */
} CamDeviceBufMode_t;

/******************************************************************************/
/**
 * @brief   The buffer queue blocking type enumeration when user read/write buffer queue.
 *
 *****************************************************************************/
typedef enum CamDeviceBufQueBlockType_e {
    CAMDEV_BUFQUE_NONBLOCK_TYPE     = 0,     /**< Non-blocking type */
    CAMDEV_BUFQUE_TIMEOUT_TYPE,            /**< Time-blocking type*/
    CAMDEV_BUFQUE_BLOCK_TYPE,                 /**< Blocking type */
    CAMDEV_BUFQUE_BLOCK_TYPE_MAX                /**< Blocking type max */
} CamDeviceBufQueBlockType_t;


/******************************************************************************/
/**
 * @brief   The buffer pool configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBufPoolConfig_s {
    CamDeviceBufMode_t       bufMode;         /**< Buffer memory mode */
    uint32_t                 bufNum;          /**< The number of buffer pool */
    uint32_t                 bufSize;         /**< The buffer size*/
    uint32_t                *pBaseAddrList;   /**< The physical base address list. If the bufMode is set to USERPTR,
                                            this member must be set. The base address should be aligned with 1024. */
    bool_t                   is_mapped;       /*new member: whether to map ISP platform virtual address.*/
    void                   **pIplAddrList;
}CamDeviceBufPoolConfig_t;

/******************************************************************************/
/**
 * @brief   The buffer queue operation configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBufQueOpConfig_s {

    CamDeviceBufQueBlockType_t    blockType;     /**< The buffer queue block type */
    uint32_t                      waitTime;      /**< When the blockType is time-blocking, should set the wait time. unit:msec*/
}CamDeviceBufQueOpConfig_t;


/******************************************************************************/
/**
 * @brief   The buffer chain configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBufChainConfig_s {
    uint32_t             skipInterval;         /**< reserved */
    uint32_t             bufQueLength;         /**< The empty and full buffer queue Length*/
    CamDeviceBufQueOpConfig_t  emptyQueOp;
    CamDeviceBufQueOpConfig_t  fullQueOp;
}CamDeviceBufChainConfig_t;

/*****************************************************************************/
/**
 * @brief   This function initializes buffer chain.
 * @startuml VsiCamDeviceInitBufChain
 * !include E10_External/VsiCamDeviceInitBufChain.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer chain index.
 * @param[in]       pConfig     Buffer chain configuration.
 * @details This function calls: CamDeviceCreateBufChain
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceInitBufChain
(
    CamDeviceHandle_t            hCamDevice,
    CamDeviceBufChainId_t        bufId,
    CamDeviceBufChainConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function deinitializes buffer chain.
 * @startuml VsiCamDeviceDeInitBufChain
 * !include E10_External/VsiCamDeviceDeInitBufChain.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer chain index.
 * @details This function calls: CamDeviceDestoryBufChain
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceDeInitBufChain
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
);

/*****************************************************************************/
/**
 * @brief   This function creates buffer pool.
 * @startuml VsiCamDeviceCreateBufPool
 * !include E10_External/VsiCamDeviceCreateBufPool.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer chain index.
 * @param[inout]    pConfig     The pointer of buffer pool configuration.
 * @details This function calls: CamDeviceCreateBufPool
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCreateBufPool
(
    CamDeviceHandle_t           hCamDevice,
    CamDeviceBufChainId_t       bufId,
    CamDeviceBufPoolConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function destroys buffer pool.
 * @startuml VsiCamDeviceDestroyBufPool
 * !include E10_External/VsiCamDeviceDestroyBufPool.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer chain index.
 * @details This function calls: CamDeviceDestoryBufPool
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceDestroyBufPool
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
);

/*****************************************************************************/
/**
 * @brief   This function sets up buffer management.
 * @startuml VsiCamDeviceSetupBufMgmt
 * !include E10_External/VsiCamDeviceSetupBufMgmt.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer channel index.
 * @details This function calls: CamDeviceSetupBufMgmt
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetupBufMgmt
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
);

/*****************************************************************************/
/**
 * @brief   This function releases buffer management.
 * @startuml VsiCamDeviceReleaseBufMgmt
 * !include E10_External/VsiCamDeviceReleaseBufMgmt.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer channel index.
 * @details This function calls: CamDeviceReleaseBufMgmt
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceReleaseBufMgmt
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
);

/*****************************************************************************/
/**
 * @brief   This function dequeues a buffer from full buffer queue.
 * @startuml VsiCamDeviceDeQueBuffer
 * !include E10_External/VsiCamDeviceDeQueBuffer.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer channel index.
 * @param[inout]    pMediaBuf   The pointer of a media buffer
 * @details This function calls: CamDeviceDQbuffer
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_TIME_OUT        Operation failed due to time out
 *
 *****************************************************************************/
RESULT VsiCamDeviceDeQueBuffer
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceBufChainId_t     bufId,
    MediaBuffer_t           **pMediaBuf
);

/*****************************************************************************/
/**
 * @brief   This function returns a buffer to empty buffer queue.
 * @startuml VsiCamDeviceEnQueBuffer
 * !include E10_External/VsiCamDeviceEnQueBuffer.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       bufId       Buffer channel index.
 * @param[inout]    pMediaBuf   The pointer of a media buffer
 * @details This function calls: CamDeviceQbuffer
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_TIME_OUT        Operation failed due to time out
 *
 *****************************************************************************/
RESULT VsiCamDeviceEnQueBuffer
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId,
    MediaBuffer_t          *pMediaBuf
);

/*****************************************************************************/
/**
 * @brief   This function gets the size of buffer.
 * @startuml VsiCamDeviceGetBufferSize
 * !include E10_External/VsiCamDeviceGetBufferSize.plantuml
 * @enduml
 * @param[in]       hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       bufId      Buffer chain index.
 * @param[inout]    pBufSize  Buffer size pointer.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetBufferSize
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceBufChainId_t    bufId,
    uint32_t                *pBufSize
);

/** @} 01_buf_mgmt_ctrl */

#ifdef __cplusplus
}
#endif

#endif  // CAMDEV_BUFFER_API_H
