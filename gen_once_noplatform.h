/*
 * gen_common.h: 
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

#ifndef __GEN_NOPLATFORM_H__
#define __GEN_NOPLATFORM_H__

#include "object.h"
#include "gen_utility.h"

void gen_code_once_noplatform(device_t *dev, FILE *f);
typedef struct register_array_list {
	int size;
	int base;
	int len;
	int interval;
	int arg; 
	int list_count;
	struct list_head list;
} reg_array_t;

typedef struct reg_element {
	struct list_head entry;
	struct list_head case_entry;
	object_t *obj;
} reg_item_t;

typedef struct case_sort {
	struct list_head entry;
	struct list_head sublist;
	int val;
} case_sort_t;

typedef struct case_array_list {
	int list_count;
	struct list_head list;
} case_array_t;

#define LIST_SZ 128
#endif /* __GEN_COMMON_H__ */
