/*
 * decl_func.h:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
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

#ifndef __DECL_FUNC_H__
#define __DECL_FUNC_H__
#include "tree.h"
#include "symbol.h"
#include "decl.h"
#include "types.h"

 params_t* get_param_decl(tree_t* node, symtab_t table);
 //void set_decl_type(decl_type_t* decl_type, type_t type);
 void parse_local_decl(tree_t* node, symtab_t table);

#endif /* __DECL_FUNC_H__ */
