/*
 * obj_ref.h: 
 *
 * Copyright (C) 2014 alloc <alloc.young@gmail.com>
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

#ifndef __OBJ_REF_H__
#define __OBJ_REF_H__  
#include "ref.h"
typedef struct obj_context {
	object_t *obj;
	tree_t *ref;
	ref_ret_t ret;
} obj_ref_t;

typedef struct context_table {
        symtab_t current;
        symtab_t saved;
        symtab_t method_parent;
		obj_ref_t *obj;
} context_t;

#endif /* __OBJ_REF_H__ */
