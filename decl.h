/*
 * decl.h:
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

#ifndef __DECL_H__
#define __DECL_H__
//#include "symbol.h"

typedef struct array_decl array_decl_t;

typedef struct type {
	unsigned char_type : 1;
	unsigned int_type  : 1;
	unsigned double_type : 1;
	unsigned float_type : 1;

	unsigned short_type : 1;
	/* long,  long int, long doulbe, long long */
	unsigned long_type : 2;
	unsigned void_type : 1;


	unsigned unsigned_type : 1;
	unsigned signed_type : 1;
	unsigned bool_type : 1;
	unsigned string_type : 1;

	unsigned no_type : 1;
	unsigned vect_type : 1;
	unsigned data_type : 1;

	unsigned type_const : 3;
	unsigned addr_const : 3;
	unsigned array_type : 2;
	unsigned point_type : 4;

	unsigned struct_type : 1;
	unsigned layout_type : 1;
	unsigned bitfields_type : 1;
	unsigned typeof_type : 1;

	unsigned enum_type : 1;
	unsigned union_type : 1;
	unsigned typedef_type : 1;

	unsigned is_constant: 1;

	unsigned is_extern : 1;
	unsigned is_local : 1;
	unsigned is_auto : 1;
	unsigned is_static : 1;

	//unsigned is_const : 4;

	char* struct_name;
	char* layout_name;
	char* bitfields_name;
	char* typeof_name;
	char* enum_name;
	char* union_name;
	char* typedef_name;
	array_decl_t* array_decl;
}decl_type_t;

struct decl_common {
	unsigned value_const : 1;
	unsigned is_unsigned : 1;
	unsigned is_signed : 1;
	unsigned addr_const : 1;

	unsigned is_data : 1;
	unsigned is_constant: 1;
	unsigned is_extern : 1;
	unsigned is_local : 1;

	unsigned is_auto : 1;
	unsigned is_static : 1;
	unsigned point_num: 22;

	char* decl_str;
	type_t type;
};

struct decl_int {
	struct decl_common common;
	/* int, int1..64, uint1..64
	 * short (int), long (int), long long (int)
	 * unsigned, signed
	 * */
	char* int_name;
};

struct decl_char {
	struct decl_common common;
};

struct decl_float {
	struct decl_common common;
	/* float, double, long double */
	char* float_name;
};

struct decl_enum {
	struct decl_common common;
};

typedef struct variable_name {
	char* var_name;
	struct variable_name* next;
	int var_num;
}var_name_t;

typedef struct decl {
	char* decl_str;
	decl_type_t* type;
	expression_t* value;
	var_name_t* var;
}decl_t;

struct array_decl {
	decl_t* decl;
	int is_fix;
	char* ident;
	expression_t* expr;
	expression_t* expr_end;
	struct array_decl* next;
};

#endif /* __DECL_H__ */