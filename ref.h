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
#include "object.h"
#include "gen_utility.h"

typedef struct ref_info {
	struct list_head list;
} ref_info_t;

struct tree_t;
typedef struct node_info {
	struct list_head entry;
	tree_t *node;
	tree_t *index;
}node_info_t;

typedef struct ref_ret {
        int is_obj;
        object_t *con;
        object_t *iface;
        const char *method;
		ref_info_t *ref;
		tree_t *index;
		symbol_t sym;
} ref_ret_t;

static inline void init_ref_ret(ref_ret_t *fr) {
		fr->is_obj = 0;
		fr->con = NULL;
		fr->iface = NULL;
		fr->method = NULL;
		fr->ref = NULL;
}

void ref_info_init(ref_info_t *info);
ref_info_t *new_ref_info(void);
node_info_t *new_node_info(tree_t *node, tree_t *index);
void add_node_info(ref_info_t *ref, node_info_t *node);
void ref_info_destroy(ref_info_t *ref);
void ref_info_print(ref_info_t *fi);
symbol_t get_ref_sym(tree_t *t, ref_ret_t *ret, symtab_t table);
void collect_ref_info(tree_t *expr, ref_info_t *fi);
void printf_ref(ref_ret_t *ref_ret);
#endif /* __REF_H__ */
