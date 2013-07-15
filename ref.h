/*
 * ref.h: 
 *
 * Copyright (C) 2013 alloc <alloc.young@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __REF_H__ 
#define __REF_H__  
#include <stdlib.h>
#include "list.h"
#include "tree.h"
#include "ast.h"

typedef struct ref_info {
	struct list_head list;
} ref_info_t;

struct tree_t;
typedef struct node_info {
	struct list_head entry;
	tree_t *node;
}node_info_t;

void ref_info_init(ref_info_t *info);
node_info_t *new_node_info(tree_t *node);
void add_node_info(ref_info_t *ref, node_info_t *node);
void ref_info_destroy(ref_info_t *ref);
void ref_info_print(ref_info_t *fi);

#endif /* __REF_H__ */
