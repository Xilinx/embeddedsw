/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_resourcemanager.c
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       ma   01/15/25 Added KDF to the resources list
 *       yog  02/25/25 Added ECIES to the resources list
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_trnghandler.h"
#include "xasufw_ecchandler.h"

/************************************ Constant Definitions ***************************************/

#define XASUFW_MAX_RESOURCES		(u32)XASUFW_INVALID          /**< Maximum hardware resources */

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
	u32 OwnerId;				/**< ID of the request which has blocked the resource */
} XAsufw_ResourceManager;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_IsResourceAvailable(XAsufw_Resource Resource, u32 ReqId);

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

	for (Index = 0U; Index < XASUFW_MAX_RESOURCES; Index++) {
		ResourceManager[Index].State = XASUFW_RESOURCE_IS_FREE;
		ResourceManager[Index].AllocatedResources = 0x0U;
		ResourceManager[Index].OwnerId = 0x0U;
	}
}

/*************************************************************************************************/
/**
 * @brief	This function releases requested hardware resource(s).
 *
 * @param	Resource	The hardware resource to be released.
 * @param	ReqId		The unique ID of the request.
 *
 * @return
 * 	-	XASUFW_SUCCESS on successful resource(s) release.
 * 	-	Error code on invalid resource.
 *
 *************************************************************************************************/
s32 XAsufw_ReleaseResource(XAsufw_Resource Resource, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 Index = 0x0U;
	u32 AllocatedResources = ResourceManager[Resource].AllocatedResources;

	if (ResourceManager[Resource].OwnerId != ReqId) {
		Status = XASUFW_RESOURCE_RELEASE_NOT_ALLOWED;
		goto END;
	}

	/** Release the requested resource. */
	ResourceManager[Resource].State = XASUFW_RESOURCE_IS_FREE;
	ResourceManager[Resource].OwnerId = 0x0U;

	/** Release all the allocated resources of main resource. */
	while (AllocatedResources != 0x0U) {
		if ((AllocatedResources & 0x1U) != 0x0U) {
			ResourceManager[Index].AllocatedResources &= ~(1U << (u32)Resource);
			/** If there are allocated resources for the dependency resource, make it free. */
			if (ResourceManager[Index].AllocatedResources == 0x0U) {
				ResourceManager[Index].State = XASUFW_RESOURCE_IS_FREE;
				ResourceManager[Index].OwnerId = 0x0U;
			}
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
 * @brief	This function allocates the requested resource.
 *
 * @param	Resource		The hardware resource to allocate.
 * @param	MainResource	The main resource for which the above resource is allocated.
 * @param	ReqId			The unique ID of the request.
 *
 *************************************************************************************************/
void XAsufw_AllocateResource(XAsufw_Resource Resource, XAsufw_Resource MainResource, u32 ReqId)
{
	ResourceManager[Resource].OwnerId = ReqId;
	ResourceManager[Resource].State = XASUFW_RESOURCE_IS_BUSY;
	ResourceManager[MainResource].AllocatedResources |= ((u32)1U << (u32)Resource);
}

/*************************************************************************************************/
/**
 * @brief	This function idles the given resource.
 *
 * @param	Resource	The hardware resource to allocate.
 *
 *************************************************************************************************/
void XAsufw_IdleResource(XAsufw_Resource Resource)
{
	u32 AllocatedResources = ResourceManager[Resource].AllocatedResources;
	u32 Index = 0x0U;

	/** Change the main resource and allocated resources state to IDLE */
	ResourceManager[Resource].State = XASUFW_RESOURCE_IS_IDLE;
	while (AllocatedResources != 0x0U) {
		if ((AllocatedResources & 0x1U) != 0x0U) {
			ResourceManager[Index].State = XASUFW_RESOURCE_IS_IDLE;
		}
		Index++;
		AllocatedResources = AllocatedResources >> 1U;
	}
}

/*************************************************************************************************/
/**
 * @brief	This function checks the availability of resource.
 *
 * @param	Resource	The hardware resource to allocate.
 * @param	ReqId		The unique ID of the request.
 *
 * @return
 * 	-	XASUFW_SUCCESS, upon resource availability.
 * 	- 	XASUFW_RESOURCE_UNAVAILABLE, upon resource unavailability
 *
 *************************************************************************************************/
static s32 XAsufw_IsResourceAvailable(XAsufw_Resource Resource, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	/* Check resource availability */
	if ((ResourceManager[Resource].State == XASUFW_RESOURCE_IS_BUSY) ||
		((ResourceManager[Resource].State == XASUFW_RESOURCE_IS_IDLE) &&
			(ResourceManager[Resource].OwnerId != ReqId))) {
		Status = XASUFW_RESOURCE_UNAVAILABLE;
	}
	else {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if all the required resources to execute the command are
 * 		available or not.
 *
 * @param	Resources	OR of all the hardware resources required for the command.
 * @param	ReqId		The unique ID of the request.
 * @param	ReqBuf		Pointer to the request buffer.
 *
 * @return
 * 	- XASUFW_SUCCESS if all the required resources are available.
 * 	- XASUFW_RESOURCE_UNAVAILABLE, if any resource is not available.
 * 	- XASUFW_RESOURCE_INVALID, if any resource is invalid.
 *
 *************************************************************************************************/
s32 XAsufw_CheckResourceAvailability(XAsufw_ResourcesRequired Resources, u32 ReqId,
		const XAsu_ReqBuf *ReqBuf)
{
	s32 Status = XASUFW_FAILURE;
	XAsufw_ResourcesRequired ReqResources = Resources;
	XAsufw_Resource Resource = XASUFW_INVALID;
	XAsufw_ResourcesRequired TempResource;
	u32 Loop;

	if (Resources == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	for (Loop = 0U; ((Loop < XASUFW_INVALID) && (ReqResources != 0U)); ++Loop) {
		TempResource = ReqResources & (1U << Loop);
		switch (TempResource) {
			case XASUFW_DMA_RESOURCE_MASK:
				if (XAsufw_IsResourceAvailable(XASUFW_DMA0, ReqId) == XASUFW_SUCCESS) {
					Resource = XASUFW_DMA0;
				} else if (XAsufw_IsResourceAvailable(XASUFW_DMA1, ReqId) == XASUFW_SUCCESS) {
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
				Resource = XAsufw_GetEccMaskResourceId(ReqBuf);
				break;
			case XASUFW_RSA_RESOURCE_MASK:
				Resource = XASUFW_RSA;
				break;
			case XASUFW_HMAC_RESOURCE_MASK:
				Resource = XASUFW_HMAC;
				break;
			case XASUFW_KDF_RESOURCE_MASK:
				Resource = XASUFW_KDF;
				break;
			case XASUFW_ECIES_RESOURCE_MASK:
				Resource = XASUFW_ECIES;
				break;
			case XASUFW_KEYWRAP_RESOURCE_MASK:
				Resource = XASUFW_KEYWRAP;
				break;
			default:
				Status = XASUFW_RESOURCE_INVALID;
				break;
		}

		if (Status == XASUFW_RESOURCE_UNAVAILABLE) {
			goto END;
		}

		if (TempResource != 0x0U) {
			Status = XAsufw_IsResourceAvailable(Resource, ReqId);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
		}
		ReqResources = ReqResources & (~TempResource);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates the available DMA for the requested hardware resource.
 *
 * @param	Resource	The hardware resource to allocate.
 * @param	ReqId		The unique ID of the request.
 *
 * @return
 *	-	Pointer to the DMA instance.
 *	-	NULL, upon unavailability.
 *
 *************************************************************************************************/
XAsufw_Dma *XAsufw_AllocateDmaResource(XAsufw_Resource Resource, u32 ReqId)
{
	XAsufw_Resource DmaAllocate = XASUFW_INVALID;
	u32 DmaDeviceId;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check for availability of DMA. */
	if (XAsufw_IsResourceAvailable(XASUFW_DMA0, ReqId) == XASUFW_SUCCESS) {
		DmaAllocate = XASUFW_DMA0;
		DmaDeviceId = ASUDMA_0_DEVICE_ID;
	} else if (XAsufw_IsResourceAvailable(XASUFW_DMA1, ReqId) == XASUFW_SUCCESS) {
		DmaAllocate = XASUFW_DMA1;
		DmaDeviceId = ASUDMA_1_DEVICE_ID;
	} else {
		goto END;
	}

	/** Allocate DMA to the resource if DMA is available. */
	if (DmaAllocate != XASUFW_INVALID) {
		AsuDmaPtr = XAsufw_GetDmaInstance(DmaDeviceId);
		XAsufw_AllocateResource(DmaAllocate, Resource, ReqId);
		ResourceManager[Resource].AllocatedResources |= (1U << (u32)DmaAllocate);
		ResourceManager[DmaAllocate].AllocatedResources |= (1U << (u32)Resource);
	}

END:
	return AsuDmaPtr;
}

/*************************************************************************************************/
/**
 * @brief	This function releases the allocated DMA resource.
 *
 * @param	AsuDmaPtr	Pointer to the DMA instance.
 * @param	ReqId		The unique ID of the request.
 *
 * @return
 *	-	XASUFW_SUCCESS on successful DMA resource release.
 * 	-	Error code on invalid resource.
 *
 *************************************************************************************************/
s32 XAsufw_ReleaseDmaResource(XAsufw_Dma *AsuDmaPtr, u32 ReqId)
{
	s32 Status = XST_FAILURE;
	XAsufw_Resource DmaResource = XASUFW_INVALID;

	if (AsuDmaPtr == NULL) {
		goto END;
	}

	if (AsuDmaPtr->AsuDma.Config.BaseAddress == XPAR_XCSUDMA_0_BASEADDR) {
		DmaResource = XASUFW_DMA0;
	} else {
		DmaResource = XASUFW_DMA1;
	}

	Status = XAsufw_ReleaseResource(DmaResource, ReqId);

END:
	return Status;
}
/** @} */
