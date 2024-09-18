/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_resourcemanager.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the hardware resource manager of ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  02/08/24 Initial release
 *       ma   05/14/24 Modify resource manager functionality to check resources availability based
 *                     on resources mask and allocate resources
 *       ma   06/04/24 Check if random bytes are available or not for TRNG GetRandomBytes command
 *       ma   07/08/24 Add task based approach at queue level
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_trnghandler.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

#define XASUFW_MAX_RESOURCES				XASUFW_INVALID          /**< Maximum hardware resources */

/************************************** Type Definitions *****************************************/
/**
 * @brief Enumeration of hardware resources state
 */
typedef enum {
	XASUFW_RESOURCE_IS_FREE,	/**< Resource is not blocked */
	XASUFW_RESOURCE_IS_IDLE,	/**< Resource is blocked by an application, but idle in state */
	XASUFW_RESOURCE_IS_BUSY		/**< Resource is busy in performing operation */
} XAsufw_ResourceState;

/**
 * @brief Enumeration of hardware resources manager
 */
typedef struct {
	XAsufw_ResourceState State;	/**< State of the resource */
	u32 AllocatedResources;		/**< Each bit represents the allocated resources info */
	u32 BlockedId;				/**< ID of the requester which has blocked the resource */
} XAsufw_ResourceManager;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_IsResourceAvailable(XAsufw_Resource Resource, u32 RequesterId);

/************************************ Variable Definitions ***************************************/
static XAsufw_ResourceManager ResourceManager[XASUFW_MAX_RESOURCES];

/*************************************************************************************************/
/**
 * @brief	Initializes the hardware resource manager and prepares it for resource allocation.
 *
 *************************************************************************************************/
void XAsufw_ResourceInit(void)
{
	u32 Index;

	for (Index = 0U; Index < (u32)XASUFW_MAX_RESOURCES; Index++) {
		ResourceManager[Index].State = XASUFW_RESOURCE_IS_FREE;
		ResourceManager[Index].AllocatedResources = 0x0U;
		ResourceManager[Index].BlockedId = 0x0U;
	}
}

/*************************************************************************************************/
/**
 * @brief	Checks the availability of DMA and AES resources and allocates the resources.
 *
 * @param	RequesterId	The unique ID of the requester.
 *
 * @return
 *	-	XASUFW_SUCCESS on successful resources allocation.
 *	- 	Error code on failure of resources allocation.
 *
 *************************************************************************************************/
s32 XAsufw_AllocateAesResources(u32 RequesterId)
{
	s32 Status = XASUFW_FAILURE;
	XAsufw_Dma *AsuDmaPtr = NULL;

	Status = XAsufw_IsResourceAvailable(XASUFW_AES, RequesterId);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* AES operation is dependent on DMA, so AES is allocated only when DMA is available */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_AES, RequesterId);
	if (AsuDmaPtr != NULL) {
		XAsufw_AllocateResource(XASUFW_AES, RequesterId);
	} else {
		Status = XASUFW_FAILURE;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Release a hardware resource(s).
 *
 * @param	RequesterId	The unique ID of the requester.
 *
 * @return
 * 	-	XASUFW_SUCCESS on successful resource(s) release.
 * 	-	Error code on invalid resource.
 *
 *************************************************************************************************/
s32 XAsufw_ReleaseAesResources(u32 RequesterId)
{
	s32 Status = XASUFW_FAILURE;

	Status = XAsufw_ReleaseResource(XASUFW_AES, RequesterId);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Release a hardware resource(s).
 *
 * @param	Resource	The hardware resource to be released.
 * @param	RequesterId	The unique ID of the requester.
 *
 * @return
 * 	-	XASUFW_SUCCESS on successful resource(s) release.
 * 	-	Error code on invalid resource.
 *
 *************************************************************************************************/
s32 XAsufw_ReleaseResource(XAsufw_Resource Resource, u32 RequesterId)
{
	s32 Status = XASUFW_FAILURE;
	u32 Index = 0x0U;
	u32 AllocatedResources = ResourceManager[Resource].AllocatedResources;

	if ((ResourceManager[Resource].BlockedId != RequesterId) ||
	    (ResourceManager[Resource].State == XASUFW_RESOURCE_IS_BUSY)) {
		Status = XASUFW_RESOURCE_RELEASE_NOT_ALLOWED;
		XFIH_GOTO(END);
	}

	/* Release the requested resource */
	ResourceManager[Resource].State = XASUFW_RESOURCE_IS_FREE;
	ResourceManager[Resource].BlockedId = 0x0U;
	while (AllocatedResources != 0x0U) {
		if ((AllocatedResources & 0x1U) != 0x0U) {
			ResourceManager[Index].AllocatedResources &= ~(1U << (u32)Resource);
		}
		Index++;
		AllocatedResources = AllocatedResources >> 1U;
	}
	ResourceManager[Resource].AllocatedResources = 0x0U;

	if (AllocatedResources == 0x0U) {
		Status = XASUFW_SUCCESS;
	}
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Allocates the resources
 *
 * @param	Resource	The hardware resource to allocate.
 * @param	RequesterId	The unique ID of the requester.
 *
 *************************************************************************************************/
void XAsufw_AllocateResource(XAsufw_Resource Resource, u32 RequesterId)
{

	ResourceManager[Resource].BlockedId = RequesterId;
	ResourceManager[Resource].State = XASUFW_RESOURCE_IS_IDLE;

}

/*************************************************************************************************/
/**
 * @brief	Checks the availability of resource.
 *
 * @param	Resource	The hardware resource to allocate.
 * @param	RequesterId	The unique ID of the requester.
 *
 * @return
 *	-	XASUFW_SUCCESS on successful resource allocation.
 *	- 	Error code upon resource unavailability
 *
 *************************************************************************************************/
static s32 XAsufw_IsResourceAvailable(XAsufw_Resource Resource, u32 RequesterId)
{
	s32 Status = XASUFW_FAILURE;

	/* Check resource availability */
	if ((ResourceManager[Resource].State == XASUFW_RESOURCE_IS_BUSY) ||
	    ((ResourceManager[Resource].State == XASUFW_RESOURCE_IS_IDLE) &&
	     (ResourceManager[Resource].BlockedId != RequesterId))) {
		Status = XASUFW_RESOURCE_UNAVAILABLE;
	}
	else {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if all the required resources to execute the command are available
 *  or not.
 *
 * @param	Resource	OR of all the hardware resources required for the command.
 * @param	RequesterId	The unique ID of the requester.
 *
 * @return
 *	-	XASUFW_SUCCESS if all the required resources are available.
 *	- 	Error code upon resource unavailability
 *
 *************************************************************************************************/
s32 XAsufw_CheckResourceAvailability(XAsufw_ResourcesRequired Resources, u32 RequesterId)
{
	s32 Status = XASUFW_FAILURE;
	XAsufw_ResourcesRequired ReqResources = Resources;
	XAsufw_Resource Resource = XASUFW_INVALID;
	XAsufw_ResourcesRequired TempResource;
	u32 Loop;

	if (Resources == 0U) {
		Status = XASUFW_SUCCESS;
		XFIH_GOTO(END);
	}

	for (Loop = 0U; (Loop < XASUFW_INVALID), (ReqResources != 0U); ++Loop) {
		TempResource = ReqResources & (1 << Loop);
		switch (TempResource) {
			case XASUFW_DMA_RESOURCE_MASK:
				if (XAsufw_IsResourceAvailable(XASUFW_DMA0, RequesterId) == XASUFW_SUCCESS) {
					Resource = XASUFW_DMA0;
				} else if (XAsufw_IsResourceAvailable(XASUFW_DMA1, RequesterId) == XASUFW_SUCCESS) {
					Resource = XASUFW_DMA1;
				} else {
					Status = XASUFW_RESOURCE_UNAVAILABLE;
				}
				break;
			case XASUFW_AES_RESOURCE_MASK:
				Resource = XASUFW_AES;
				break;
			case XASUFW_SHA2_RESOURCE_MASK:
				Resource = XASUFW_SHA2;
				break;
			case XASUFW_SHA3_RESOURCE_MASK:
				Resource = XASUFW_SHA3;
				break;
			case XASUFW_PLI_RESOURCE_MASK:
				Resource = XASUFW_PLI;
				break;
			case XASUFW_TRNG_RESOURCE_MASK:
				Resource = XASUFW_TRNG;
				break;
			case XASUFW_TRNG_RANDOM_BYTES_MASK:
				Resource = XASUFW_TRNG;
				if (XAsufw_TrngIsRandomNumAvailable() != XASUFW_SUCCESS) {
					Status = XASUFW_RESOURCE_UNAVAILABLE;
				}
				break;
			case XASUFW_ECC_RESOURCE_MASK:
				Resource = XASUFW_ECC;
				break;
			case XASUFW_RSA_RESOURCE_MASK:
				Resource = XASUFW_RSA;
				break;
			case XASUFW_HMAC_RESOURCE_MASK:
				Resource = XASUFW_HMAC;
				break;
			default:
				Status = XASUFW_RESOURCE_INVALID;
				break;
		}

		if (Status == XASUFW_RESOURCE_UNAVAILABLE) {
			XFIH_GOTO(END);
		}

		if (TempResource != 0x0U) {
			Status = XAsufw_IsResourceAvailable(Resource, RequesterId);
			if (Status != XASUFW_SUCCESS) {
				XFIH_GOTO(END);
			}
		}
		ReqResources = ReqResources & (~TempResource);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Allocates the available DMA for the requested hardware resource.
 *
 * @param	Resource	The hardware resource to allocate.
 * @param	RequesterId	The unique ID of the requester.
 * @param	AsuDmaPtr	The DMA pointer to be returned to the caller.
 *
 * @return
 *	-	XASUFW_SUCCESS on successful DMA allocation.
 *	-	Error code on resource unavailability.
 *
 *************************************************************************************************/
XAsufw_Dma *XAsufw_AllocateDmaResource(XAsufw_Resource Resource, u32 RequesterId)
{
	XAsufw_Resource DmaAllocate = XASUFW_INVALID;
	u32 DmaDeviceId;
	XAsufw_Dma *AsuDmaPtr = NULL;

	if (XAsufw_IsResourceAvailable(XASUFW_DMA0, RequesterId) == XASUFW_SUCCESS) {
		DmaAllocate = XASUFW_DMA0;
		DmaDeviceId = ASUDMA_0_DEVICE_ID;
	} else if (XAsufw_IsResourceAvailable(XASUFW_DMA1, RequesterId) == XASUFW_SUCCESS) {
		DmaAllocate = XASUFW_DMA1;
		DmaDeviceId = ASUDMA_1_DEVICE_ID;
	} else {
		XFIH_GOTO(END);
	}

	if (DmaAllocate != XASUFW_INVALID) {
		AsuDmaPtr = XAsufw_GetDmaInstance(DmaDeviceId);
		XAsufw_AllocateResource(DmaAllocate, RequesterId);
		ResourceManager[Resource].AllocatedResources |= (1U << (u32)DmaAllocate);
		ResourceManager[DmaAllocate].AllocatedResources |= (1U << (u32)Resource);
	}

END:
	return AsuDmaPtr;
}

/** @} */
