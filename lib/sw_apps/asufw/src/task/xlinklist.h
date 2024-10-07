/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xlinklist.h
 *
 * This file contains the code for linked list support.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XLINKLIST_H
#define XLINKLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/** Type definition for XLinkList double linked list structure */
typedef struct XLinkList XLinkList;

/** Structure for double linked list */
struct XLinkList {
	XLinkList *Next;    /**< Pointer to the next list element */
	XLinkList *Prev;    /**< Pointer to the previous list element */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the given XLinkList structure.
 *
 * @param	List    Linked list
 *
 *************************************************************************************************/
static inline void XLinkList_Init(XLinkList *List)
{
	List->Prev = List;
	List->Next = List;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if the list is empty or not.
 *
 * @param	List    Linked list
 *
 * @return
 * 			- Returns TRUE if given list is empty. Otherwise, returns FALSE.
 *
 *************************************************************************************************/
static inline u8 XLinkList_IsEmpty(const XLinkList *List)
{
	return (u8)((List->Next == List) || (List->Next == NULL));
}

/*************************************************************************************************/
/**
 * @brief	This function removes given item from the list.
 *
 * @param   List    Linked list
 *
 *************************************************************************************************/
static inline void XLinkList_RemoveItem(XLinkList *List)
{
	List->Next->Prev = List->Prev;
	List->Prev->Next = List->Next;
	List->Prev = List;
	List->Next = List->Prev;
}

/*************************************************************************************************/
/**
 * @brief	This function adds the given item to the start of the given linked list.
 *
 * @param   Item    Item to be added to the starting of the list
 * @param   List    Linked list
 *
 *************************************************************************************************/
static inline void XLinkList_AddItemFirst(XLinkList *Item, XLinkList *List)
{
	Item->Next = List->Next;
	Item->Prev = List;
	List->Next->Prev = Item;
	List->Next = Item;
}

/*************************************************************************************************/
/**
 * @brief	This function adds the item to the end of the given linked list.
 *
 * @param   Item    Item to be added to the ending of the list
 * @param   List    Linked list
 *
 *************************************************************************************************/
static inline void XLinkList_AddItemLast(XLinkList *Item, XLinkList *List)
{
	Item->Next = List;
	Item->Prev = List->Prev;
	List->Prev->Next = Item;
	List->Prev = Item;
}

/*************************************************************************************************/
/**
 * @brief   This function concatinates given two linked lists.
 *
 * @param   List1   First linked list to be concatinated with second list
 * @param   List2   Second linked list to be concatinated at the end of first list
 *
 *************************************************************************************************/
static inline void XLinkList_Concat(XLinkList *List1, const XLinkList *List2)
{
	if (XLinkList_IsEmpty(List2) != (u8)TRUE) {
		List1->Prev->Next = List2->Next;
		List2->Next->Prev = List1->Prev;
		List1->Prev = List2->Prev;
		List2->Prev->Next = List1;
	}
}

/*************************************************************************************************/
/**
 * @brief	Helper macro to traverse the linked list.
 *
 * @param   Item    Pointer to hold the item in linked list
 * @param	List    Linked list to be traversed
 *
 * @return
 *          - Pointer to item in the linked list being traversed.
 *
 *************************************************************************************************/
#define XLinkList_ForEach(Item, List)   \
	for ((Item) = (List)->Next;         \
	     (Item) != (List);               \
	     (Item) = (Item)->Next)

/*************************************************************************************************/
/**
 * @brief	Get the structure address from the structure element
 *
 * @param   Item    Linked list of the task structure
 * @param	Type    Structure type from which the Item's address need to be derived
 * @param   Member  Structure member which corresponds to Item
 *
 * @return
 *          - Returns the base address of structure containing given structure element
 *
 *************************************************************************************************/
#define XLinkList_ContainerOf(Item, Type, Member)    \
	((Type *)((char *)(Item) - offsetof(Type, Member)))

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLINKLIST_H */
