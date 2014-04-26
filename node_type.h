/*
 * node_type.h: 
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

#ifndef __NODE_TYPE_H__
#define __NODE_TYPE_H__ 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "symbol.h"
#include "type_info.h"
#include "ast.h"
#include "info_output.h"

void check_expr_type(tree_t *node, symtab_t table, type_info_t **info);


#ifdef NODE_TRACE
#define NODE_TRACE(fmt, ...) do { \
			fprintf(stderr, fmt, ## __VA_ARGS__); \
		} while(0)
#else 
#define NODE_TRACE(fmt, ...) do {}while(0)
#endif
#endif /* __NODE_TYPE_H__ */
