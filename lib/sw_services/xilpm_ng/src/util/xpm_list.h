/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_LIST_H__
#define __XPM_LIST_H__

#include "xpm_runtime_alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATE_LIST(T) \
/* Creates a node type of T - named "{T}Node" */ \
typedef struct T##Node { \
	T* Data; \
	struct T##Node *Next; \
} T##Node; \
\
/* Creates a list type of "{T}Node" */ \
typedef struct T##List { \
	T##Node *Root; \
} T##List; \
\
/* Creates a function to make a new list of type "{T}List", named "Make_{T}List" */ \
static inline T##List* Make_##T##List(void) { \
	T##List* list = (T##List *)XPm_AllocBytesOthers(sizeof(T##List)); \
	if (list != NULL) { \
		list->Root = NULL; \
	} \
	return list; \
}

#define LIST_PREPEND(List, Value) \
do { \
	typeof((List)->Root) NewNode = (typeof((List)->Root))XPm_AllocBytesOthers(sizeof(*(List)->Root)); \
	if (NULL == NewNode) { \
		PmErr("OOM: Data container to be added to %s\r\n", #List); \
		Status = XST_BUFFER_TOO_SMALL; \
		goto done; \
	} \
	NewNode->Data = (Value); \
	NewNode->Next = (List)->Root; \
	(List)->Root = NewNode; \
} while (0)

#define LIST_FOREACH(List, var) \
	if ((List) == NULL) { \
		PmErr("List is NULL\n"); \
	} else \
		for (typeof((List)->Root) var = (List)->Root; var != NULL; var = var->Next)

#define LIST_FIRST_DATA(List) \
	((List)->Root != NULL ? (List)->Root->Data : NULL)

#define LIST_LAST_DATA(List) \
({ \
	typeof((List)->Root) __node = (List)->Root; \
	typeof(__node->Data) __data = NULL; \
	if (__node != NULL) { \
		while (__node->Next != NULL) { \
			__node = __node->Next; \
		} \
		__data = __node->Data; \
	} \
	__data; \
})

#ifdef __cplusplus
}
#endif

#endif /*__XPM_LIST_H__ */
