/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * @file	list.h
 * @brief	List primitives for libmetal.
 */

#ifndef __METAL_LIST__H__
#define __METAL_LIST__H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup list List Primitives
 *  @{ */

struct metal_list {
	struct metal_list *next, *prev;
};

#define METAL_DECLARE_LIST(name)			\
	struct metal_list name = { .next = &name, .prev = &name }

static inline void metal_list_init(struct metal_list *list)
{
	list->next = list->prev = list;
}

static inline void metal_list_add_before(struct metal_list *node,
					 struct metal_list *new_node)
{
	new_node->prev = node->prev;
	new_node->next = node;
	new_node->next->prev = new_node;
	new_node->prev->next = new_node;
}

static inline void metal_list_add_after(struct metal_list *node,
					struct metal_list *new_node)
{
	new_node->prev = node;
	new_node->next = node->next;
	new_node->next->prev = new_node;
	new_node->prev->next = new_node;
}

static inline void metal_list_add_head(struct metal_list *list,
				       struct metal_list *node)
{
	metal_list_add_after(list, node);
}

static inline void metal_list_add_tail(struct metal_list *list,
				       struct metal_list *node)
{
	metal_list_add_before(list, node);
}

static inline int metal_list_is_empty(struct metal_list *list)
{
	return list->next == list;
}

static inline void metal_list_del(struct metal_list *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	node->next = node->prev = node;
}

static inline struct metal_list *metal_list_first(struct metal_list *list)
{
	return metal_list_is_empty(list) ? NULL : list->next;
}

#define metal_list_for_each(list, node)		\
	for ((node) = (list)->next;		\
	     (node) != (list);			\
	     (node) = (node)->next)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LIST__H__ */
