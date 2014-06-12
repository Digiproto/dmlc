/*
 * type_info.h: 
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

#ifndef __TYPE_INFO_H__
#define __TYPE_INFO_H__
#include "stdio.h"
#include "tree.h"

enum type_info {
	TYPE_ZERO,
	TYPE_INDEX = 1 << 1,
	TYPE_LAYOUT = 1 << 2,
	TYPE_SINGLEBIT = 1 << 3,
	TYPE_BITFIELD  = 1 << 4
};


typedef struct layout_info {
    tree_t *expr;
    int offset;
    const char *endian;
} layout_info;

typedef struct bitfield_info {
    tree_t *expr;
	tree_t *start;
	tree_t *end;
} bt_info;

typedef struct {
	enum type_info kind;
	bt_info bt;
	layout_info layout;
} type_info_t;

#endif /* __TYPE_INFO_H__ */
