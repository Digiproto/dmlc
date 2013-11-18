/*
 * decl.c:
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
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "symbol.h"
#include "tree.h"
#include "types.h"
#include "ast.h"
#include "decl.h"
#include "expression.h"
#include "info_output.h"
#include "object.h"

void* gdml_realloc(void* addr, int size) {
	if (addr == NULL) {
		addr = gdml_zmalloc(size);
	}
	else {
		addr = realloc(addr, size);
	}
	if (addr == NULL) {
		fprintf(stderr, "realloc failed: %s", strerror(errno));
		exit(-1);
	}

	return addr;
}

decl_t* create_cdecl_struct(void) {
	decl_t* decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
	decl_type_t* type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
	decl->type = type;

	return decl;
}

char* add_type_str(char* addr, const char* str){
	assert(str != NULL);
	int size = 0;

	if (addr != NULL) {
		size = strlen(addr);
	}
	size += strlen(str) + 2;

	addr = (char*)gdml_realloc(addr, size);
	addr = strcat(addr, str);
	addr = strcat(addr, " ");
	DEBUG_DECL("addr str : %s, len: %d\n", addr, (int)(strlen(str)));

	return addr;
}

void set_decl_type(decl_type_t* decl_type, type_t type) {
	assert(decl_type != NULL);

	switch(type) {
		case NO_TYPE:
		case UNDEFINED_TYPE:
			decl_type->no_type = 1;
			break;
		case BOOL_TYPE:
			decl_type->bool_type = 1;
			break;
		case CHAR_TYPE:
			decl_type->char_type = 1;
			break;
		case INTEGER_TYPE:
		case INT_TYPE:
			decl_type->int_type = 1;
			break;
		case DOUBLE_TYPE:
			decl_type->double_type = 1;
			break;
		case FLOAT_TYPE:
			decl_type->float_type = 1;
			break;
		case LONG_TYPE:
			decl_type->long_type = 1;
			break;
		case SHORT_TYPE:
			decl_type->short_type = 1;
			break;
		case SIGNED_TYPE:
			decl_type->signed_type = 1;
			break;
		case UNSIGNED_TYPE:
			decl_type->unsigned_type = 1;
			break;
		case CONST_STRING_TYPE:
			decl_type->string_type = 1;
			break;
		case POINTER_TYPE:
			decl_type->point_type = 1;
			break;
		case VECT_TYPE:
			decl_type->vect_type = 1;
			break;
		case ARRAY_TYPE:
			decl_type->array_type = 1;
			break;
		case STRUCT_TYPE:
			decl_type->struct_type = 1;
			break;
		case LAYOUT_TYPE:
			decl_type->layout_type = 1;
			break;
		case BITFIELDS_TYPE:
			decl_type->bitfields_type = 1;
			break;
		case TYPEOF_TYPE:
			decl_type->typeof_type = 1;
			break;
		case ENUM_TYPE:
			decl_type->enum_type = 1;
			break;
		case UNION_TYPE:
			decl_type->union_type = 1;
			break;
		case TYPEDEF_TYPE:
			decl_type->typedef_type = 1;
			break;
		default:
			printf("other type: %d\n", type);
			exit(-1);
			break;
	}

	return;
}

type_t get_decl_type(decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if (type->no_type)
		return NO_TYPE;
	if (type->point_type)
		return POINTER_TYPE;
	if (type->char_type)
		return CHAR_TYPE;
	if (type->int_type)
		return INT_TYPE;
	if (type->double_type)
		return DOUBLE_TYPE;
	if (type->float_type)
		return FLOAT_TYPE;
	if (type->long_type) {
		if (type->double_type) {
			return DOUBLE_TYPE;
		}
		return INT_TYPE;
	}
	if (type->short_type) {
		return INT_TYPE;
	}
	if (type->signed_type) {
		if ((type->char_type) || (type->float_type)
				|| (type->double_type) || (type->float_type)){
			/* do nothing */
		}
		else {
			return INT_TYPE;
		}
	}
	if (type->unsigned_type) {
		if ((type->char_type) || (type->float_type)
				|| (type->double_type) || (type->float_type)){
			/* do nothing */
		}
		else {
			return INT_TYPE;
		}
	}
	if (type->bool_type)
		return BOOL_TYPE;
	if (type->string_type)
		return CONST_STRING_TYPE;
	if (type->vect_type)
		return VECT_TYPE;
	if (type->struct_type)
		return STRUCT_TYPE;
	if (type->layout_type)
		return LAYOUT_TYPE;
	if (type->bitfields_type)
		return BITFIELDS_TYPE;
	if (type->typeof_type)
		return TYPEOF_TYPE;
	if (type->enum_type)
		return ENUM_TYPE;
	if (type->union_type)
		return UNION_TYPE;
	if (type->typedef_type)
		return TYPEDEF_TYPE;
	if (type->is_func_pointer)
		return FUNCTION_POINTER_VAR;

	return 0;
}

var_name_t* create_var_name(const char* name) {
	assert(name != NULL);

	var_name_t* var_new = (var_name_t*)gdml_zmalloc(sizeof(var_name_t));
	var_new->var_name = name;

	return var_new;
}

void get_typedef_info(symbol_t symbol, decl_t* decl) {
	assert(symbol != NULL);
	assert(decl != NULL);

	typedef_attr_t* attr = symbol->attr;
	decl_t* base_decl = attr->decl;
	int type = attr->base_type;
	switch (type) {
		case STRUCT_TYPE:
			decl->type->struct_type = 1;
			decl->type->struct_name = base_decl->type->struct_name;
			DEBUG_DECL("In %s, line = %d, struct_type.\n", __func__, __LINE__);
			break;
		case LAYOUT_TYPE:
			decl->type->layout_type = 1;
			decl->type->layout_name = base_decl->type->layout_name;
			DEBUG_DECL("In %s, line = %d, layout_type.\n", __func__, __LINE__);
			break;
		case BITFIELDS_TYPE:
			decl->type->bitfields_type = 1;
			decl->type->bitfields_name = base_decl->type->bitfields_name;
			DEBUG_DECL("In %s, line = %d, bitfields_type.\n", __func__, __LINE__);
			break;
		case TYPEDEF_TYPE:
			decl->type->typedef_type = 1;
			decl->type->typedef_name = base_decl->type->typedef_name;
			DEBUG_DECL("In %s, line = %d, typedef_type, base_type. name: %s\n",
					__func__, __LINE__ , decl->type->typedef_name);
			break;
		case TYPEOF_TYPE:
			/* TODO:  how to deal the typedef */
			DEBUG_DECL("In %s, line = %d, typeof_type.\n", __func__, __LINE__);
			break;
		default:
			set_decl_type(decl->type, type);
			break;
	}

	return;
}

int symbol_construct_type(symbol_t symbol, decl_t* decl) {
	assert(symbol != NULL);
	assert(decl != NULL);
	void* attr = NULL;
	int type = 0;

	switch(symbol->type) {
		case STRUCT_TYPE:
			decl->type->struct_type = 1;
			decl->type->struct_name = symbol->name;
			DEBUG_DECL("In %s, line = %d, struct_type : %s!\n",
					__func__, __LINE__, symbol->name);
			break;
		case LAYOUT_TYPE:
			decl->type->layout_type = 1;
			decl->type->layout_name = symbol->name;
			DEBUG_DECL("In %s, line = %d, layout_type!\n", __func__, __LINE__);
			exit(-1);
			break;
		case BITFIELDS_TYPE:
			decl->type->bitfields_type = 1;
			decl->type->bitfields_name = symbol->name;
			DEBUG_DECL("In %s, line = %d, bitfields_type!\n", __func__, __LINE__);
			exit(-1);
			break;
		case TYPEDEF_TYPE:
			get_typedef_info(symbol, decl);
			DEBUG_DECL("In %s, line = %d, typedef_type, base_type!\n", __func__, __LINE__ );
			break;
		case TYPEOF_TYPE:
			DEBUG_DECL("In %s, line = %d, typeof_type!\n", __func__, __LINE__);
			exit(-1);
			break;
		case ENUM_TYPE:
			DEBUG_DECL("In %s, line = %d, enum_type!\n", __func__, __LINE__);
			exit(-1);
			break;
		case UNION_TYPE:
			DEBUG_DECL("In %s, line = %d, union_type!\n", __func__, __LINE__);
			exit(-1);
			break;
		default:
			DEBUG_DECL("In %s, line = %d, symbol name: %s, type: %d\n",
					__func__, __LINE__, symbol->name, symbol->type);
			decl->is_defined = 1;
			decl->defined_name = symbol->name;
			break;
	}

	return 0;
}

void create_var_list(decl_t* decl, const char* var_name) {
	assert(decl != NULL);
	assert(var_name != NULL);

	if (decl->var == NULL) {
		decl->var = create_var_name(var_name);
		decl->var->var_num += 1;
		DEBUG_DECL("decl->var->name 2: %s : %s\n", decl->var->var_name, var_name);
	}
	else {
		var_name_t* var = decl->var;
		DEBUG_DECL("decl->var->name1: %s: %s\n", var->var_name, decl->var->var_name);
		while (var->next != NULL)  {
			var = var->next;
		}
		var->next = create_var_name(var_name);
		decl->var->var_num += 1;
	}

	return;
}

void parse_identifier(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(decl != NULL);

	if ((node->common.type) != IDENT_TYPE) {
		fprintf(stderr, "node is other type: %s\n", node->common.name);
		/* FIXME: handle the error */
		exit(-1);
	}

	DEBUG_DECL("In %s, line = %d, find symbol: %s\n",
			__func__, __LINE__, node->ident.str);
	symbol_t symbol = symbol_find_notype(table, node->ident.str);
	if (symbol != NULL) {
		DEBUG_DECL("In %s, line = %d, finded the symbol: %s, type: %d : %d\n",
				__func__, __LINE__, node->ident.str, node->common.type, symbol->type);
		symbol_construct_type(symbol, decl);
		decl->decl_str = add_type_str(decl->decl_str, node->ident.str);
	}
	else {
		decl->decl_str = add_type_str(decl->decl_str, node->ident.str);
		pre_parse_symbol_t* symbol = pre_symbol_find(node->ident.str);
		if (symbol != NULL) {
			decl->type->pre_dml_name = symbol->name;
			decl->type->pre_dml_type = symbol->type;
			DEBUG_DECL("In %s, line = %d, symbol name: %s, type: %d\n",
					__func__, __LINE__, symbol->name, symbol->type);
			switch(symbol->type) {
				case STRUCT_TYPE:
					decl->type->struct_type = 1;
					decl->type->struct_name = symbol->name;
					break;
				case TYPEDEF_TYPE:
					decl->type->typedef_type = 1;
					decl->type->typedef_name = symbol->name;
					break;
				case FUNCTION_POINTER_TYPE:
					decl->type->is_func_pointer = 1;
					decl->type->func_point_name = symbol->name;
					break;
				default:
					decl->type->is_constant = 1;
					break;
			}
		}
		else {
			create_var_list(decl, node->ident.str);
		}
	}

	return ;
}

int parse_bool(decl_t* decl) {
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->void_type) ||
			(type->bool_type) || (type->struct_type) || (type->layout_type) ||
			(type->bitfields_type) || (type->typeof_type) ||
			(type->enum_type) || (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "bool");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if ((type->long_type) || (type->short_type)) {
		fprintf(stderr, "error: both ‘%s’ and ‘bool’ in declaration specifiers\n",
				(type->long_type) ? "long" : "short");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->char_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "bool");
	}

	return 0;
}

int parse_char(decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->void_type) ||
			(type->bool_type) || (type->struct_type) || (type->layout_type) ||
			(type->bitfields_type) || (type->typeof_type) ||
			(type->enum_type) || (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "char");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if ((type->long_type) || (type->short_type)) {
		fprintf(stderr, "error: both ‘%s’ and ‘char’ in declaration specifiers\n",
				(type->long_type) ? "long" : "short");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->char_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "char");
	}

	return 0;
};

int parse_double(decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->short_type) || (type->void_type) ||
			(type->bool_type) || (type->struct_type) || (type->layout_type) ||
			(type->bitfields_type) || (type->typeof_type) || (type->enum_type) ||
			(type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "double");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	if ((type->long_type) > 1) {
		int i = 0;
		fprintf(stderr, "error: ");
		while (i < (type->long_type)) {
			fprintf(stderr, "long ");
			i++;
		}
		fprintf(stderr, "and ‘double’ in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->double_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "double");
	}

	return 0;
}

int parse_float(decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->short_type) || (type->void_type) ||
			(type->bool_type) || (type->struct_type) || (type->layout_type) ||
			(type->bitfields_type) || (type->typeof_type) || (type->enum_type) ||
			(type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "float");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if (type->long_type) {
		fprintf(stderr, "error: both ‘long’ and ‘float’ in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->float_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "float");
	}

	return 0;
}

int parse_int(tree_t* node, decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->void_type) || (type->bool_type) ||
			(type->struct_type) || (type->layout_type) || (type->bitfields_type) ||
			(type->typeof_type) || (type->enum_type) || (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "int");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if ((type->long_type) > 2) {
		int i = 0;
		fprintf(stderr, "error: ");
		while (i < (type->long_type)) {
			fprintf(stderr, "long ");
			i++;
		}
		fprintf(stderr, "is too long for compile\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->int_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, node->ident.str);
		DEBUG_DECL("In %s, line = %d, decl str: %s\n",
				__func__, __LINE__, decl->decl_str);
	}

	return 0;
}

int parse_long(decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->float_type) || (type->void_type) || (type->bool_type) ||
			(type->struct_type) || (type->layout_type) || (type->bitfields_type) ||
			(type->typeof_type) || (type->enum_type) || (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "long");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if ((type->int_type) && ((type->long_type) > 1)) {
		int i = 0;
		fprintf(stderr, "error: ");
		while (i < (type->long_type)) {
			fprintf(stderr, "long ");
			i++;
		}
		fprintf(stderr, "long is too long for compile\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else if ((type->double_type) && ((type->long_type) > 0)) {
		int i = 0;
		fprintf(stderr, "error: ");
		while (i < (type->long_type)) {
			fprintf(stderr, "long ");
			i++;
		}
		fprintf(stderr, "long and ‘double’ in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->long_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "long");
	}
	return 0;
}

int parse_short(decl_t* decl) {
	assert(decl != NULL);

	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->short_type) || (type->void_type) || (type->bool_type) ||
			(type->struct_type) || (type->layout_type) || (type->bitfields_type) ||
			(type->typeof_type) || (type->enum_type) || (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "short");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if (type->long_type) {
		fprintf(stderr, "error: both ‘long’ and ‘short’ in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->short_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "short");
	}

	return 0;
}

int parse_signed_unsigned(decl_t* decl, int is_signed) {
	assert(decl != NULL);

	decl_type_t* type = decl->type;
	if ((type->struct_type) || (type->layout_type) || (type->bitfields_type) ||
			(type->typeof_type) || (type->enum_type) || (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "unsigned");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if (type->signed_type) {
		fprintf(stderr, "error: duplicate ‘unsigned’\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else if (type->unsigned_type) {
		fprintf(stderr, "error: both ‘signed’ and ‘unsigned’ in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		if (is_signed == 1) {
			type->signed_type = 1;
			decl->decl_str = add_type_str(decl->decl_str, "signed");
		}
		else {
			type->unsigned_type = 1;
			decl->decl_str = add_type_str(decl->decl_str, "unsigned");
		}
	}

	return 0;
}

int parse_void(decl_t* decl) {
	assert(decl != NULL);

	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->short_type) || (type->long_type) ||
			(type->unsigned_type) || (type->signed_type) || (type->bool_type)
			|| (type->vect_type) || (type->struct_type) || (type->layout_type)
			||(type->bitfields_type) || (type->typeof_type) || (type->enum_type)
			|| (type->union_type)) {
		if (decl->type->typedef_type) {
			create_var_list(decl, "void");
		}
		else {
			fprintf(stderr, "error: two or more data types in declaration specifiers\n");
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if (type->void_type) {
		fprintf(stderr, "error: duplicate ‘void’\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		type->void_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "void");
	}

	return 0;
}

int parse_const(decl_t* decl) {
	assert(decl != NULL);
	decl->type->type_const += 1;
	decl->decl_str = add_type_str(decl->decl_str, "const");

	return 0;
}

decl_t* parse_c_keyword(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(decl != NULL);
	DEBUG_DECL("In %s, line = %d, node->type: %s\n",
			__func__, __LINE__, node->common.name);

	switch(node->ident.type) {
		case BOOL_TYPE:
			parse_bool(decl);
			break;
		case CHAR_TYPE:
			parse_char(decl);
			break;
		case DOUBLE_TYPE:
			parse_double(decl);
			break;
		case FLOAT_TYPE:
			parse_float(decl);
			break;
		case INT_TYPE:
			parse_int(node, decl);
			break;
		case LONG_TYPE:
			parse_long(decl);
			break;
		case SHORT_TYPE:
			parse_short(decl);
			break;
		case SIGNED_TYPE:
			parse_signed_unsigned(decl, 1);
			break;
		case UNSIGNED_TYPE:
			parse_signed_unsigned(decl, 0);
			break;
		case CONST_TYPE:
			parse_const(decl);
			break;
		case VOID_TYPE:
			parse_void(decl);
			break;
		default:
			fprintf(stderr, "Other c keyword: %s\n", node->ident.str);
			/* FIXME: handle the error */
			exit(-1);
	}

	return decl;
}

decl_t* parse_dml_keyword(tree_t* node, symtab_t table, decl_t* decl) {
	DEBUG_DECL("IN %s, line = %d, node type: %s : %s\n",
			__func__, __LINE__, node->common.name, node->ident.str);
	node->common.type = IDENT_TYPE;
	parse_identifier(node, table, decl);

	return decl;
}

void parse_basetype(tree_t* node, symtab_t table, decl_t* decl) {
	 assert(node != NULL);
	 assert(table != NULL);
	 assert(decl != NULL);
	DEBUG_DECL("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	switch (node->common.type) {
		case STRUCT_TYPE:
			//parse_struct(node, table, decl);
			break;
		case LAYOUT_TYPE:
			//parse_layout(node, table, decl);
			break;
		case BITFIELDS_TYPE:
			//parse_bitfields(node, table, decl);
			break;
		case TYPEOF_TYPE:
			//parse_typeof(node, table, decl);
			break;
		default:
			//parse_typeident(node, table, decl);
			break;
	}

	return ;
}

static cdecl_t* parse_cdecl(tree_t* node, symtab_t table);
params_t* get_param_decl(tree_t* node, symtab_t table) {
	assert(node != NULL);
	if (node->common.type != CDECL_TYPE) {
		error("params type is : %s\n", node->common.name);
	}
	params_t* param = (params_t*)gdml_zmalloc(sizeof(params_t));
	table->pass_num = 0;
	cdecl_t* decl = parse_cdecl(node, table);
	table->pass_num = 1;
	if (decl->common.no_decalare) {
		param->var_name = decl->var_name;
		//param->is_notype = 1;
		decl->common.categ = NO_TYPE;
		param->decl = decl;
	}
	else {
		param->var_name = decl->var_name;
		param->decl = decl;
	}

	DEBUG_DECL("IN %s, line = %d, va_name: %s, type: %d, INT_T: %d, TYPEDEF_T: %d\n",
			__func__, __LINE__,  param->var_name, decl->common.categ, INT_T, TYPEDEF_T);

	return param;
}

int parse_cdecl2_modify(tree_t* node, symtab_t table, decl_t* decl) {
	if (node == NULL) {
		return 0;
	}

	if (node->cdecl.is_const) {
		tree_t* decl2 = node->cdecl.decl;
		if (decl2->cdecl.is_point) {
			decl->type->addr_const += 1;
		}
		else {
			decl->type->type_const += 1;
		}
		decl->decl_str = add_type_str(decl->decl_str, "const");
		//parse_cdecl2(decl2, table, decl);
	}

	if (node->cdecl.is_point) {
		decl->type->point_type += 1;
		decl->decl_str = add_type_str(decl->decl_str, "*");
		//parse_cdecl2(node->cdecl.decl, table, decl);
	}

	if (node->cdecl.is_vect) {
		decl->type->vect_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "vect");
		//parse_cdecl2(node->cdecl.decl, table, decl);
	}

	return 0;
}

expression_t* parse_array_expression(tree_t* node, symtab_t table) {
	assert(node != NULL);
	assert(table != NULL);
	expression_t* expr = NULL;
	DEBUG_DECL("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	while (node != NULL) {
		if (expr == NULL) {
			expr = parse_expression(&node, table);
		}
		else {
			expr->next = parse_expression(&node, table);
		}
		node = node->common.sibling;
	}

	return expr;
}

int parse_array(tree_t* node, symtab_t table, decl_t* decl) {
	assert((node != NULL) || (table != NULL) || (decl != NULL));
	DEBUG_DECL("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	array_decl_t* array_decl = (array_decl_t*)gdml_zmalloc(sizeof(array_decl_t));

	if (node->array.decl) {
		//parse_c_array(node, table, array_decl);
	}

	decl->type->array_type += 1;
	decl->decl_str = add_type_str(decl->decl_str, "[");

	array_decl->is_fix = node->array.is_fix;
	array_decl->decl= decl;

	if (node->array.ident) {
		tree_t* ident = node->array.ident;
		if ((ident->common.type) != IDENT_TYPE) {
			fprintf(stderr, "the identifier type is: %s\n", ident->common.name);
			/* FIXME: handle the error */
			exit(-1);
		}
		DEBUG_DECL("array ident: %s\n", ident->ident.str);
		if (symbol_insert(table, ident->ident.str, IDENT_TYPE, NULL) == -1) {
			fprintf(stderr, "redfined: %s\n", ident->ident.str);
			/* FIXME: handle  the error */
			exit(-1);
		}
	}

	if (node->array.expr) {
		array_decl->expr = parse_array_expression(node->array.expr, table);
	}
	if (node->array.expr_end) {
		array_decl->expr_end = parse_array_expression(node->array.expr_end, table);
	}

	if ((decl->type->array_decl) == NULL) {
		decl->type->array_decl = array_decl;
	}
	else {
		array_decl_t* array = decl->type->array_decl;
		while (array->next != NULL) {
			array = array->next;
		}
		array->next = array_decl;
	}
	decl->decl_str = add_type_str(decl->decl_str, "]");

	return 0;
}

int parse_cdecl_list(tree_t* node, symtab_t table, decl_t* decl) {
	if (node == NULL) {
		return 0;
	}
	DEBUG_DECL("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	if (node->common.type == ELLIPSIS_TYPE) {
		if (node->common.sibling) {
			parse_cdecl_list(node->common.sibling, table, decl);
		}
		decl->decl_str = add_type_str(decl->decl_str, "...");
	}
	else {
		if (node->common.sibling) {
			parse_cdecl_list(node->common.sibling, table, decl);
		}
		//parse_cdecl(node, table, decl);
	}

	return 0;
}

func_param_t* parse_func_param(tree_t* node, symtab_t table) {
	assert(node != NULL);
	assert(table != NULL);

	DEBUG_DECL("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	func_param_t* new_param = (func_param_t*)gdml_zmalloc(sizeof(func_param_t));

	decl_t* decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
	decl_type_t* type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
	decl->type = type;

	if (node->common.type == ELLIPSIS_TYPE) {
		new_param->is_ellipsis = 1;
		free(type);
		free(decl);
		DEBUG_DECL("params decl_str: %s\n", "...");
	}
	else {
		//parse_cdecl(node, table, decl);
		new_param->decl = decl;
		DEBUG_DECL("params decl_str: %s\n", new_param->decl->decl_str);
	}


	return new_param;
}

void print_param_list(func_param_t* param) {
	assert(param);

	func_param_t* tmp = param;
	int num = 0;

	while (tmp != NULL) {
		DEBUG_DECL("param[%d] : ", ++num);
		if (tmp->is_ellipsis) {
			DEBUG_DECL("%s\n", "...");
		}
		else {
			DEBUG_DECL("%s\n", tmp->decl->decl_str);
		}
		tmp = tmp->next;
	}

	return;
}

func_param_t* find_param_tail(func_param_t* root) {
	assert(root != NULL);
	func_param_t* tmp = root;

	while(tmp->next) {
		tmp = tmp->next;
	}

	return tmp;
}

func_param_t* create_param_list(func_param_t* root, func_param_t* new) {
	assert(new != NULL);

	func_param_t* tail = NULL;

	if (root == NULL) {
		root = new;
	}
	else {
		tail = find_param_tail(root);
		tail->next = new;
	}

	return root;
}

func_param_t* parse_function_params(tree_t* node, symtab_t table) {
	assert(table != NULL);

	if (node == NULL) {
		return NULL;
	}

	DEBUG_DECL("IN %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	tree_t* param_node = node;
	func_param_t* param = NULL;
	func_param_t* new_param = NULL;

	while (param_node) {
		new_param = parse_func_param(param_node, table);
		param = create_param_list(param, new_param);
		param_node = param_node->common.sibling;
	}
	print_param_list(param);

	return param;
}

decl_t* change_decl(decl_t* decl_dest, decl_t* decl_src) {
	assert(decl_dest != NULL);
	assert(decl_src != NULL);

	decl_dest->decl_str = decl_src->decl_str;
	decl_dest->is_defined = decl_src->is_defined;
	decl_dest->defined_name = decl_src->defined_name;
	decl_dest->type = decl_src->type;
	decl_dest->value = decl_src->value;
	decl_dest->var = decl_src->var;

	return decl_dest;
}

function_t* get_func_infor(const char* func_name, tree_t* func_params, symtab_t table, decl_t* decl, int is_pointer) {
	assert(func_name != NULL);
	assert(table != NULL);
	assert(decl != NULL);

	decl_t* ret_decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
	ret_decl = change_decl(ret_decl, decl);

	function_t* func = (function_t*)gdml_zmalloc(sizeof(function_t));
	func->func_name = func_name;
	if(strcmp(func->func_name, "this") == 0) {
		fprintf(stderr, "synax error at 'this'\n");
		/* TODO: handle the error */
		exit(-1);
	}
	func->ret_decl = ret_decl;
	DEBUG_DECL("ret param: %s\n", decl->decl_str);
	if (func_params) {
		func->argc = get_param_num(func_params);
		func->param = parse_function_params(func_params, table);
		if (func->argc == 1) {
				if ((func->param->is_ellipsis) ||
				(strcmp(func->param->decl->decl_str, "void ") == 0)) {
				func->argc = 0;
			}
		}
		if ((is_pointer == 1) && ((func->argc > 0) && (func->param->is_ellipsis == 0))) {
			if (strncmp(func->param->decl->decl_str, "conf_object_t * obj", strlen("conf_object_t * obj")) == 0) {
				func->argc -= 1;
				func->param = func->param->next;
			}
		}
	}

	return func;
}

int parse_c_function(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(decl != NULL);

	char* func_name = NULL;
	tree_t* func_node = node->cdecl_brack.cdecl;
	tree_t* func_params = node->cdecl_brack.decl_list;

	if ((func_node->common.type == IDENT_TYPE)
			|| (func_node->common.type == DML_KEYWORD_TYPE)) {
		decl_t* new_decl = create_cdecl_struct();
		function_t* func = get_func_infor((char*)(func_node->ident.str), func_params, table, decl, 0);
		new_decl->type->is_func = 1;
		new_decl->type->func_decl = func;
		decl = change_decl(decl, new_decl);
	}
	else if (func_node->common.type == CDECL_BRACK_TYPE) {
		tree_t* cdecl_node = func_node->cdecl_brack.cdecl;
		tree_t* list_node = func_node->cdecl_brack.decl_list;
		if ( cdecl_node && list_node) {
			/* FIXME: handle the error */
			fprintf(stderr, "cdecl name collision on '%s', node type: %s(%d)\n",
				cdecl_node->ident.str, cdecl_node->common.name, cdecl_node->common.type);
			exit(-1);
		}
		else if (list_node) {
			/* void(*func)(type param1, type param2)*/
			/* (int)(a) */
			decl_t* new_decl = create_cdecl_struct();
			//parse_cdecl2(list_node, table, new_decl);
			if ((new_decl->decl_str)[0] == '*') {
				char* name = NULL;
				if (new_decl->var) {
					name  = (char*)(new_decl->var->var_name);
				}
				else {
					name = new_decl->type->pre_dml_name;
				}
				function_t* func = get_func_infor(name, func_params, table, decl, 1);

				new_decl->type->is_func_pointer = 1;
				new_decl->type->func_decl = func;
				decl = change_decl(decl, new_decl);
			}/* function pointer */
			else {
				decl->type = new_decl->type ;
				/* TODO: fix the cast type*/
				fprintf(stderr, "cast type!\n");
				exit(-1);
			} /* type cast */
		}
	}
	else {
		fprintf(stderr, "name collision on '%s', node type: %s(%d)\n",
				func_node->ident.str, node->common.name, node->common.type);
		/* TODO: handle the error */
		exit(-1);
	}
	tree_t* list = node->cdecl_brack.decl_list;

	return 0;
}

int parse_decl_brack(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	DEBUG_DECL("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	/*
	 * Grammar:
	 *		cdecl3 '(' cdecl_list ')'
	 *		| '(' cdecl2 ')'
	 */
	/* extern function grammar : cdecl3 '(' cdecl_list ')' */
	if ((node->cdecl_brack.cdecl) && (node->cdecl_brack.is_list)) {
		DEBUG_DECL("extern c function : decl_str: %s\n", decl->decl_str);
		parse_c_function(node, table, decl);
	}
	else if (node->cdecl_brack.decl_list) {
		//parse_cdecl2(node->cdecl_brack.decl_list, table, decl);
		fprintf(stderr, "not extern c function\n");
		exit(-1);
	}
	else {
		fprintf(stderr, "other cdecl3!\n");
		/* TODO: handle the error */
		exit(-1);
	}

	return 0;
}

int parse_typeident(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(decl != NULL);
	switch(node->common.type) {
		case C_KEYWORD_TYPE:
			parse_c_keyword(node, table, decl);
			break;
		case DML_KEYWORD_TYPE:
			parse_dml_keyword(node, table, decl);
			break;
		case IDENT_TYPE:
			parse_identifier(node, table, decl);
			break;
		default:
			fprintf(stderr, "Other typeident: %s\n", node->common.name);
			/* FIXME: handle the error */
			exit(-1);
			break;
	}

	return 0;
}

int parse_base_aggregate(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(decl != NULL);

	tree_t* base_node = node->cdecl.basetype;
	if (node->cdecl.decl) {
		//parse_cdecl2(node->cdecl.decl, table, decl);
	}

	switch (base_node->common.type) {
		case STRUCT_TYPE:
			//parse_struct(base_node, table, decl);
			break;
		case LAYOUT_TYPE:
			//parse_layout(base_node, table, decl);
			break;
		case BITFIELDS_TYPE:
			//parse_bitfields(base_node, table, decl);
			break;
		case TYPEOF_TYPE:
			//parse_typeof(base_node, table, decl);
			break;
		default:
			fprintf(stderr, "wrong dml struct type(%d)\n", base_node->common.type);
			/* TODO: handle the error */
			exit(-1);
	}

	return 0;
}

int charge_integer_expr(symtab_t table, decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->int_type) || (type->double_type) ||
			(type->float_type) || (type->short_type) || (type->long_type) ||
			(type->unsigned_type) || (type->signed_type) || (type->bool_type)
			|| (type->no_type)) {
		return 0;
	}
	else if (type->point_type) {
		fprintf(stderr, "warning: assignment makes pointer from integer without a cast\n");
		/* FIXME: handle the error */
		exit(-1);
		return 1;
	}
	else if (type->typedef_type) {
		symbol_t symbol = symbol_find(table, type->typedef_name, TYPEDEF_TYPE);
		typedef_attr_t* attr = symbol->attr;
		//int typedef_type = attr->base_type;
		return charge_integer_expr(table, attr->decl);
	}
	else {
		fprintf(stderr, "Line: %d, error: invalid initializer\n", __LINE__);
		/* FIXME: handle the error */
		exit(-1);
		return -1;
	}

	return 0;
}

int charge_float_expr(decl_type_t* type) {
	assert(type != NULL);
	if ((type->char_type) || (type->int_type) || (type->double_type) ||
			(type->float_type) || (type->short_type) || (type->long_type) ||
			(type->unsigned_type) || (type->signed_type) || (type->no_type)
			|| (type->bool_type)) {
		return 0;
	}
	else {
		fprintf(stderr, "Line : %d, error: invalid initializer\n", __LINE__);
		/* FIXME: handle the error */
		exit(-1);
		return -1;
	}

	return 0;
}

int charge_pointer_expr(decl_type_t* type) {
	assert(type != NULL);

	if (type->point_type) {
		return 0;
	}
	else {
		fprintf(stderr, "warning: initialization from pointer without a cast\n");
		/* FIXME: handle the warning */
		return -1;
	}

	return 0;
}

int charge_decl_expr_type(symtab_t table, decl_t* decl, expression_t* expr) {
	assert(decl != NULL);
	assert(expr != NULL);

	int ret = 0;

	switch (expr->final_type) {
		case INTEGER_TYPE:
		case INT_TYPE:
			ret = charge_integer_expr(table, decl);
			break;
		case FLOAT_TYPE:
			ret = charge_float_expr(decl->type);
			break;
		case BOOL_TYPE:
			ret = charge_integer_expr(table, decl);
			break;
		case NO_TYPE:
			//ret = charge_notype_expr(decl->type);
			break;
		case POINTER_TYPE:
			ret = charge_pointer_expr(decl->type);
			break;
		default:
			fprintf(stderr, "other expression type: %d\n", expr->final_type);
			/* FIXME: handle the error */
			exit(-1);
			ret = -1;
			break;
	}

	return ret;
}

void insert_ident_decl(symtab_t table, decl_t* decl) {
	assert(table != NULL);
	assert(decl != NULL);

	int i = 0;
	char* name = NULL;

	if (decl->type->func_decl) {
		int type = 0;
		type = (decl->type->is_func) ? FUNCTION_TYPE : FUNCTION_POINTER_TYPE;
		function_t* func = (function_t*)(decl->type->func_decl);
		symbol_insert(table, func->func_name, type, func);
	}
	else {
		int type = get_decl_type(decl);
		var_name_t* var = decl->var;
		//printf("\nLine: %d name: %s, type: %d\n", __LINE__, var->var_name, type);
		while (var) {
			name = (char*)(var->var_name);
			symbol_insert(table, name, type, decl);
			var = var->next;
		}
	}

	return;
}

void parse_data_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* decl = parse_cdecl(node->cdecl.decl, table);

	if (symbol_defined(table, decl->var_name))
		error("name collision on '%s'\n", decl->var_name);
	symbol_insert(table, decl->var_name, DATA_TYPE, decl);

	return;
}

void parse_local_decl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* decl = parse_cdecl(node->local_tree.cdecl, table);

	if (node->local_tree.is_static) {
		decl->common.is_static = 1;
	}

	if (node->local_tree.local_keyword) {
		tree_t* keyword = node->local_tree.local_keyword;
		char* keyword_name = (char*)(keyword->local_keyword.name);
		if (strcmp(keyword_name, "auto") == 0) {
			decl->common.is_auto = 1;
		}
		else if (strcmp(keyword_name, "local") == 0) {
			decl->common.is_local = 1;
		}
		else {
			fprintf(stderr, "Other local keyword: %s\n", keyword_name);
			/* FIXME: handle the error */
			exit(-1);
		}
	}

	if (node->local_tree.expr) {
		expr_t* expr = check_expr(node->local_tree.expr, table);
		cdecl_t* type = expr->type;
		if (!both_scalar_type(decl, expr->type) &&
				!(both_array_type(decl, expr->type)) &&
				!(is_same_type(decl, expr->type)) &&
				!(is_no_type(decl)) && !is_parameter_type(decl)) {
			error("incompatible types when initializing\n");
		}
	}

	symbol_insert(table, decl->var_name, decl->common.categ, decl);

	return;
}

int parse_stars(tree_t* node, symtab_t table, decl_t* decl) {
	assert(table != NULL);
	assert(decl != NULL);

	if (node == NULL) {
		return 0;
	}

	decl->type->point_type += 1;
	add_type_str(decl->decl_str, "*");

	if (node->stars.is_const) {
		decl->type->addr_const += 1;
		add_type_str(decl->decl_str, "const");
	}

	parse_stars(node->stars.stars, table, decl);

	return 0;
}

extern int parse_ctypedecl_ptr(tree_t* node, symtab_t table, decl_t* decl);
int parse_ctypedecl_simple(tree_t* node, symtab_t table, decl_t* decl) {
	assert(table);
	assert(decl);
	if (node == NULL) {
		return 0;
	}

	add_type_str(decl->decl_str, "(");

	parse_ctypedecl_ptr(node->cdecl_brack.decl_list, table, decl);

	add_type_str(decl->decl_str, ")");

	return 0;
}

int parse_ctypedecl_ptr(tree_t* node, symtab_t table, decl_t* decl) {
	assert(table != NULL);
	assert(decl != NULL);

	if (node == NULL) {
		return 0;
	}

	parse_stars(node, table, decl);

	parse_ctypedecl_simple(node->ctypedecl_ptr.array, table, decl);

	return 0;
}

decl_t* parse_ctypedecl(tree_t* node, symtab_t table) {
	assert(node != NULL);
	assert(table != NULL);

	decl_t* decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
	decl_type_t* type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
	decl->type = type;

	if (node->ctypedecl.const_opt) {
		type->type_const += 1;
		decl->decl_str = add_type_str(decl->decl_str, "const");
	}

	if (node->ctypedecl.basetype) {
		parse_basetype(node->ctypedecl.basetype, table, decl);
	}

	if (node->ctypedecl.ctypedecl_ptr) {
		parse_ctypedecl_ptr(node->ctypedecl.ctypedecl_ptr, table, decl);
	}

	return decl;
}

int parse_base_aggregate_block(tree_t* node, symtab_t table) {
	assert(table != NULL);
	assert(node != NULL);

	decl_t* decl = NULL;
	decl_type_t* type = NULL;
	tree_t* tmp_node = node;

	while (tmp_node != NULL) {
		decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
		type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
		decl->type = type;

		//parse_cdecl(tmp_node, table, decl);
		insert_ident_decl(table, decl);

		tmp_node = tmp_node->common.sibling;
	}

	return 0;
}

int parse_struct_decls(tree_t* node, symtab_t table) {
	assert(table != NULL);

	if (node != NULL) {
		parse_base_aggregate_block(node, table);
	}

	return 0;
}

int parse_layout_decls(tree_t* node, symtab_t table) {
	assert(table != NULL);

	if (node != NULL) {
		parse_base_aggregate_block(node, table);
	}

	return 0;
}

int parse_bitfields_decls(tree_t* node, symtab_t table) {
	assert(table != NULL);
	if (node == NULL) {
		return 0;
	}

	decl_t* decl = NULL;
	decl_type_t* type = NULL;
	bitfield_decl_attr_t* attr = NULL;
	int var_type = 0;
	char* var_name = NULL;

	while (node != NULL) {
		decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
		type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
		attr = (bitfield_decl_attr_t*)gdml_zmalloc(sizeof(bitfield_decl_attr_t));

		//parse_cdecl(node->bitfields_dec.decl, table, decl);
		var_type = get_decl_type(decl);
		var_name = (char*)(decl->var->var_name);

		attr->decl = decl;
		attr->var_name = (char*)(decl->var->var_name);
		attr->start_expr = parse_expression(&(node->bitfields_dec.start), table);
		attr->end_expr = parse_expression(&(node->bitfields_dec.end), table);

		symbol_insert(table, var_name, var_type, attr);

		node = node->common.sibling;
	}

	return 0;
}

static element_t* add_element(element_t* head, element_t* new) {
	assert(new != NULL);
	if (head == NULL) {
		head = new;
	}
	else {
		element_t* tmp = head;
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new;
	}
	return head;
}

static cdecl_t* parse_cdecl(tree_t* node, symtab_t table);
void parse_undef_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* decl = parse_cdecl(node, table);
	if (decl->common.no_decalare == 0) {
		symbol_insert(table, decl->var_name, decl->common.categ, decl);
	}

	return;
}

static cdecl_t* parse_struct(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = STRUCT_T;
	type->struc.table = node->struct_tree.table;

	tree_t* elem_node = node->struct_tree.block;
	cdecl_t* decl = NULL;
	element_t* elem = NULL;
	symtab_t struct_table = type->struc.table;
	while(elem_node) {
		elem = (element_t*)gdml_zmalloc(sizeof(element_t));
		decl = parse_cdecl(elem_node, struct_table);
		if (decl->common.no_decalare == 1) {
			elem_node->common.parse = parse_undef_cdecl;
			undef_var_insert(struct_table, elem_node);
			free(elem); free(decl);
			elem = NULL; decl = NULL;
		}
		else {
			elem->decl = decl;
			type->struc.elem = add_element(type->struc.elem, elem);
		}
		elem_node = elem_node->common.sibling;
	}

	return type;
}

static cdecl_t* parse_layout(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	const char* bitorder = node->layout.desc;
	if (strcmp(bitorder, "\"little-endian\"") && (strcmp(bitorder, "\"big-endian\""))) {
		error("%s not one of \"big-endian\" or \"little-endian\"\n", bitorder);
		return NULL;
	}

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = LAYOUT_T;
	type->layout.table = node->layout.table;
	type->layout.bitorder = strdup(bitorder);

	tree_t* elem_node = node->layout.block;
	cdecl_t* decl = NULL;
	element_t* elem = NULL;
	symtab_t layout_table = type->layout.table;
	/* parse the element about layout */
	while (elem_node) {
		elem = (element_t*)gdml_zmalloc(sizeof(element_t));
		decl = parse_cdecl(elem_node, layout_table);
		if (decl->common.no_decalare == 1) {
			elem_node->common.parse = parse_undef_cdecl;
			undef_var_insert(layout_table, elem_node);
			free(elem); free(decl);
			elem = NULL; decl = NULL;
		}
		else {
			elem->decl = decl;
			type->layout.elem = add_element(type->layout.elem, elem);
		}
		elem_node = elem_node->common.sibling;
		printf("In %s, line = %d, parse the layout elements\n", __func__, __LINE__);
		exit(-1);
	}

	return type;
}

static void parse_bit_expr(tree_t* node, symtab_t table, bit_element_t* elem) {
	assert(node != NULL); assert(table != NULL); assert(elem != NULL);
	cdecl_t* decl = elem->decl;
	/* TODO: calculate the expression */
	/* In simic the bit style: a @ [end : start]*/
	//elem->start = ;
	//elem->end = ;
	elem->size = elem->end - elem->start;
	if (decl->common.size != elem->size) {
		error("illegal bitfields definition: field '%s' has wrong size\n", decl->var_name);
		free(decl); free(elem);
		decl = NULL; elem = NULL;
	}

	return;
}

void parse_undef_bit_elem(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	bit_element_t* elem = (bit_element_t*)gdml_zmalloc(sizeof(bit_element_t));
	cdecl_t* decl = parse_cdecl(node->bitfields_dec.decl, table);
	if (decl->common.no_decalare == 0) {
		if (decl->common.categ != INT_T) {
			error("illegal bitfields definition: non-integer field");
			free(decl); decl = NULL;
		}
		else {
			elem->decl = decl;
			parse_bit_expr(node, table, elem);
			symbol_insert(table, elem->decl->var_name, elem->decl->common.categ, elem);
		}
	}

	return;
}

static bit_element_t* parse_bit_element(tree_t* node, symtab_t table) {
	assert(table != NULL);
	bit_element_t* elem = (bit_element_t*)gdml_zmalloc(sizeof(bit_element_t));
	cdecl_t* decl = parse_cdecl(node->bitfields_dec.decl, table);
	if (decl->common.no_decalare == 1) {
		node->common.parse = parse_undef_bit_elem;
		undef_var_insert(table, node);
		free(decl); free(elem);
		decl = NULL; elem = NULL;
	}
	else if (decl->common.categ != INT_T) {
		free(decl); free(elem);
		decl = NULL; elem = NULL;
		error("illegal bitfields definition: non-integer field");
	}
	else {
		elem->decl = decl;
		parse_bit_expr(node, table, elem);
	}
	return elem;
}

static bit_element_t* add_bit_element(bit_element_t* head, bit_element_t* new) {
	assert(new != NULL);
	if (head == NULL) {
		head = new;
	} else {
		bit_element_t* tmp = head;
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new;
	}

	return head;
}

static cdecl_t* parse_bitfields(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	int size = node->bitfields.size;
	if (size > 64) {
		error("illegal bitfields definition: bitfields width is > 64\n");
		return NULL;
	}

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = BITFIELDS_T;
	type->bitfields.size = size;
	type->bitfields.table = node->bitfields.table;

	tree_t* elem_node = node->bitfields.block;
	bit_element_t* elem = NULL;
	symtab_t bitfield_table = type->bitfields.table;
	/* TODO: parse the elements about bitfields */
	while (elem_node) {
		elem = parse_bit_element(elem_node, bitfield_table);
		if (elem != NULL) {
			type->bitfields.elem = add_bit_element(type->bitfields.elem, elem);
		}
		elem_node = elem_node->common.sibling;
		printf("IN %s, line = %d, parse the bitfields elements\n", __func__, __LINE__);
		exit(-1);
	}

	return type;
}

static cdecl_t* parse_typeof(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* type = get_typeof_type(node, table);

	return type;
}

static int get_int_size(const char* int_str) {
	assert(int_str);
	int size = 0;
	if (strcmp(int_str, "int") == 0) {
		size = sizeof(int) * 8;
	}
	else if (strcmp(int_str, "uint") == 0) {
		error("unknown type: 'uint'\n");
	}
	else {
		const char* data = NULL;
		if (strstr(int_str, "uint"))
			data = &int_str[4];
		else
			data = &int_str[3];
		size = atoi(data);
		if (size > 64)
			error("unknown type: '%s'\n", int_str);
	}

	return size;
}

static cdecl_t* parse_type_ident(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	//printf("IN %s, line = %d, c type: %s\n", __func__, __LINE__, node->ident.str);
	if (node->common.type == C_KEYWORD_TYPE) {
		switch(node->ident.type) {
			case BOOL_TYPE:
				type->common.categ = BOOL_T;
				type->common.size = sizeof(char) * 8;
				break;
			case CHAR_TYPE:
				type->common.categ = CHAR_T;
				type->common.size = sizeof(char) * 8;
				break;
			case DOUBLE_TYPE:
				type->common.categ = DOUBLE_T;
				type->common.size = sizeof(double) * 8;
				break;
			case FLOAT_TYPE:
				type->common.categ = FLOAT_T;
				type->common.size = sizeof(float) * 8;
				break;
			case INT_TYPE:
				type->common.categ = INT_T;
				type->common.size = get_int_size(node->ident.str);
				break;
			case LONG_TYPE:
				type->common.categ = LONG_T;
				type->common.size = sizeof(long) * 8;
				break;
			case SHORT_TYPE:
				type->common.categ = SHORT_T;
				type->common.size = sizeof(short) * 8;
				break;
			case VOID_TYPE:
				type->common.categ = VOID_T;
				break;
			default:
				error("unknown type: '%s'\n", node->ident.str);
				break;
		}
	}
	else {
		symbol_t symbol = symbol_find_notype(table, node->ident.str);
		if (symbol == NULL) {
			symtab_t root_table = get_root_table();
			symbol = symbol_find_notype(root_table, node->ident.str);
		}
		/* In simics, some method parameter can not have type*/
		if (symbol) {
			if ((symbol->type == TYPEDEF_T) || (symbol->type == STRUCT_T)) {
				memcpy(type, symbol->attr, sizeof(cdecl_t));
				type->var_name = NULL;
			} else if (symbol->type == STRUCT_TYPE) {
				/* some struct defined like this:
					 struct abc {
						struct abc ab;
					}
				but in the "struct abc ab", the "struct abc" do not have
				type block, so should mark 'struct abc ab" undefined, and
				for the second time to parse it*/
				struct_attr_t* attr = symbol->attr;
				if (attr->decl) {
					memcpy(type, ((struct_attr_t*)(symbol->attr))->decl, sizeof(cdecl_t));
					type->var_name = NULL;
				} else {
					type->var_name = node->ident.str;
					type->common.no_decalare = 1;
				}
			} else if (symbol) {
				type->var_name = node->ident.str;
				type->common.categ = NO_TYPE;
			} else {
				error("unknown type : %s, type: %d, ATTRIBUTE: %d\n", symbol->name, symbol->type, ATTRIBUTE_TYPE);
			}
		}
		else {
			if (table->pass_num == 0) {
				type->var_name = node->ident.str;
				type->common.no_decalare = 1;
				DEBUG_DECL("unknown symbol: %s\n", type->var_name);
			}
			else {
				error("'%s' isn't declared (first use)\n", node->ident.str);
			}
		}
	}

	return type;
}

static cdecl_t* parse_base_type(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* type = NULL;
	switch(node->common.type) {
		case STRUCT_TYPE:
			type = parse_struct(node, table);
			break;
		case LAYOUT_TYPE:
			type = parse_layout(node, table);
			break;
		case BITFIELDS_TYPE:
			type = parse_bitfields(node, table);
			break;
		case TYPEOF_TYPE:
			type = parse_typeof(node, table);
			break;
		default:
			type = parse_type_ident(node, table);
			break;

	}
	return type;
}

static void parse_cdecl2(tree_t* node, symtab_t table, cdecl_t* type);
static void parse_point(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);

	int qual = type->common.qual;
	type->common.qual = 0;
	/* in the declare about c function, it's parameter
	 * can not have name, only have it's type */
	if (node->cdecl.decl)
		parse_cdecl2(node->cdecl.decl, table, type);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	drv->ctor = POINTER_TO;
	drv->qual = type->common.qual;
	drv->next = type->common.drv;
	type->common.drv = drv;
	type->common.qual = qual;

	return;
}

static void parse_cdecl3(tree_t* node, symtab_t table, cdecl_t* type);
static void parse_c_array(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	parse_cdecl3(node->array.decl, table, type);
	/* TODO: calculate the array expression*/
	//printf("\npay attention: In %s, line = %d, calculate the expression value\n\n", __func__, __LINE__);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	drv->ctor = ARRAY_OF;
	drv->len = check_expr(node->array.expr, table);
	drv->next = type->common.drv;
	type->common.drv = drv;

	return;
}

static void free_param(void** data, int i) {
	assert(data != NULL);
	while (i) {
		free(data[--i]);
	}
	return;
}

static signature_t* parse_function_param(tree_t* node, symtab_t table) {
	assert(table != NULL);
	if (node == NULL) return NULL;

	signature_t* sig = (signature_t*)gdml_zmalloc(sizeof(signature_t));
	int param_num = get_param_num(node);
	vector_t* vect = (vector_t*)gdml_zmalloc(sizeof(vector_t));
	vect->data = (void**)gdml_zmalloc(param_num * sizeof(void*));
	vect->len = param_num;
	sig->params = vect;

	tree_t* tmp = node;
	param_t* param = NULL;
	cdecl_t* ty = NULL;
	int i = 0;
	while (tmp) {
		param = (param_t*)gdml_zmalloc(sizeof(param_t));
		if (tmp->common.type == ELLIPSIS_TYPE) {
			sig->has_ellipse = 1;
			param->is_ellipsis = 1;
			break;
		}
		tree_t* basetype = tmp->cdecl.basetype;
		int type = basetype->common.type;
		/* in function parameter, can not be allowed to defined new record type */
		if((type == STRUCT_TYPE || type == LAYOUT_TYPE || type == BITFIELDS_TYPE)) {
			free(param);
			error("the function parameter type define '%s'\n", node->common.name);
			break;
		}
		ty = parse_cdecl(tmp, table);
		param->id = ty->var_name;
		param->ty = ty;
		vect->data[i++] = param;
		if (ty->common.no_decalare) {
			sig->has_no_decalare = 1;
			free_param(vect->data, i);
			free(vect->data); free(vect);
			break;
		}
		if((ty->common.categ == VOID_T) && (tmp->cdecl.decl == NULL)) {
			if (vect->len == 1) {
				free_param(vect->data, i);
				vect->len = 0;
			} else {
				free_param(vect->data, i);
				error("void’ must be the only parameter\n");
			}
		}
		tmp = tmp->common.sibling;
	}
	return sig;
}

static void parse_function(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	parse_cdecl3(node->cdecl_brack.cdecl, table, type);
	signature_t* sig = parse_function_param(node->cdecl_brack.decl_list, table);
	if (sig && (sig->has_no_decalare)) {
		type->common.no_decalare = 1;
		free(sig);
		return;
	}
	DEBUG_DECL("function name: %s\n", type->var_name);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	drv->ctor = FUNCTION_RETURN;
	drv->sig = sig;
	drv->next = type->common.drv;
	type->common.drv = drv;

	return;
}

static int is_c_keyword(tree_t* node) {
	assert(node != NULL);
	if (node->common.type == C_KEYWORD_TYPE)
		return 1;
	else
		return 0;
}

static void parse_ident(tree_t* node, symtab_t table, cdecl_t* decl) {
	assert(node != NULL); assert(table != NULL); assert(decl != NULL);
	if (is_c_keyword(node) && (decl->common.is_typedef == 0)) {
		error("c keyword can not be the identifier name\n");
	}
	else {
		decl->var_name = node->ident.str;
		//printf("IN %s, line = %d, var name: %s\n", __func__, __LINE__, decl->var_name);
	}

	return;
}

static void parse_cdecl3(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(table != NULL); assert(type != NULL);
	if (node == NULL) return;

	if (node->common.type == ARRAY_TYPE) {
		parse_c_array(node, table, type);
	}
	else if (node->common.type == CDECL_BRACK_TYPE) {
		if (node->cdecl_brack.cdecl) {
			parse_function(node, table, type);
		} else {
			parse_cdecl2(node->cdecl_brack.decl_list, table, type);
		}
	}
	else {
		parse_ident(node, table, type);
	}

	return;
}

static void parse_cdecl2(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(table != NULL); assert(type != NULL);
	if (node == NULL) return;
	if (node->common.type != CDECL_TYPE)
		parse_cdecl3(node, table, type);

	if (node->cdecl.is_const == 1) {
		type->common.qual |= CONST_QUAL;
		parse_cdecl2(node->cdecl.decl, table, type);
	}
	if (node->cdecl.is_vect == 1) {
		type->common.qual |= VECT_QUAL;
		parse_cdecl2(node->cdecl.decl, table, type);
	}
	if (node->cdecl.is_point == 1) {
		parse_point(node, table, type);
	}

	return;
}

static cdecl_t* qualify(int qual, cdecl_t* type) {
	assert(type != NULL);
	if ((qual == 0) || (qual == type->common.qual))
		return type;

	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	*ty = *type;
	ty->common.qual |= qual;
	if (ty->common.qual != 0) {
		ty->common.bty = ty->common.bty;
	} else {
		ty->common.bty = ty;
	}

	return ty;
}

cdecl_t* pointer_to(cdecl_t* type) {
	assert(type != NULL);
	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	ty->common.categ = POINTER_T;
	ty->common.qual = 0;
	ty->common.size = sizeof(void*);
	ty->common.bty = type;
	ty->var_name = type->var_name;

	return ty;
}

cdecl_t* array_of(cdecl_t* type) {
	assert(type != NULL);
	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	ty->common.categ = ARRAY_T;
	ty->common.qual = 0;
	//ty->common.size = len * (ty->common.size);
	ty->common.bty = type;
	ty->var_name = type->var_name;

	return ty;
}

static cdecl_t* function_return(cdecl_t* type, signature_t* sig) {
	assert(type != NULL);
	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	ty->common.categ = FUNCTION_T;
	ty->common.qual = 0;
	ty->common.size = sizeof(void*);
	ty->function.sig = sig;
	ty->common.bty = type;
	ty->var_name = type->var_name;

	return ty;
}

static void free_drv(type_deriv_list_t* head) {
	if (head == NULL) return;

	type_deriv_list_t* tmp = head;
	type_deriv_list_t* drv = head;
	while (drv) {
		tmp = drv->next;
		free(drv);
		drv = tmp;
	}
	return;
}

static cdecl_t* derive_type(cdecl_t* type) {
	assert(type != NULL);
	cdecl_t* ty = type;
	type_deriv_list_t* drv = type->common.drv;
	if (type->common.no_decalare == 1) {
		free_drv(type->common.drv);
		return ty;
	}
	while (drv != NULL) {
		if (drv->ctor == POINTER_TO) {
			cdecl_t* tmp = pointer_to(ty);
			ty = qualify(drv->qual, tmp);
		}
		else if (drv->ctor == ARRAY_OF) {
			if (type->common.categ == FUNCTION_T)
				return NULL;
			ty = array_of(ty);
		}
		else {
			if (ty->common.categ == ARRAY_T || ty->common.categ == FUNCTION_T)
				return NULL;
			ty = function_return(ty, drv->sig);
		}
		drv = drv->next;
	}
	return ty;
}

static cdecl_t* parse_base(tree_t* node, symtab_t table, int is_const) {
	assert(node != NULL); assert(table != NULL);
	/* parse the decal base type*/
	tree_t* base = node;
	cdecl_t* type = parse_base_type(base, table);
	if (is_const)
		type->common.qual |= CONST_QUAL;

	return type;
}

static cdecl_t* parse_decl(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(table != NULL); assert(type != NULL);
	if (node == NULL)
		return type;
	cdecl_t* ty = type;
	parse_cdecl2(node, table, ty);
	ty = derive_type(ty);
	if (ty == NULL)
		error("illegal type\n");

	return ty;
}

static cdecl_t* parse_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* parse the decal base type*/
	tree_t* base = node->cdecl.basetype;
	cdecl_t* type = parse_base(base, table, node->cdecl.is_const);

	/* parse the indentifier, but the simics, there can be some
	 * extern c functions with parameters that no have name, but
	 * only have type; and the method about simics, that can only
	 * have name, not have type */
	tree_t* decl = node->cdecl.decl;
	type = parse_decl(decl, table, type);

	return type;
}

void parse_extern_cdecl_or_ident(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* decl = parse_cdecl(node->cdecl.decl, table);
	decl->common.is_extern = 1;
	/* as some type and identifier can be used before declares
	 * so when the first pass times, insert the undefined node
	 * into table so as the second passing time the check them*/
	if (table->pass_num == 0) {
		if (decl->common.no_decalare == 1) {
			tree_t* tmp = node->cdecl.decl;
			/* in simics there can like "extern VNULL "*/
			if ((decl->var_name) &&
					((tmp->cdecl.basetype) && (tmp->cdecl.decl == NULL))) {
				if (symbol_insert(table, decl->var_name, decl->common.categ, decl)) {
					//wa("duplicate '%s'\n", decl->var_name);
					return;
				}
			}
			else {
				free(decl);
				undef_var_insert(table, node);
			}
		}
		else {
			if (decl->var_name) {
				symbol_insert(table, decl->var_name, decl->common.categ, decl);
			}
			else {
				error("expected identifier about extern\n");
			}
		}
	}
	else {
		if ((decl->var_name) && (decl->common.no_decalare == 0)) {
				int rt = symbol_insert(table, decl->var_name, decl->common.categ, decl);
				if (rt) {
					//error("duplicate '%s'\n", decl->var_name);
					return;
				}
		}
	}

	return;
}

int record_type(cdecl_t* type) {
	assert(type != NULL);
	int is_record_type = 0;
	switch(type->common.categ) {
		case STRUCT_T:
		case LAYOUT_T:
		case BITFIELDS_T:
			is_record_type = 1;
			break;
		default:
			is_record_type = 0;
			break;
	}
	return is_record_type;
}

static void insert_record_elems(cdecl_t* type) {
	assert(type != NULL);
	if (type->common.categ == STRUCT_T) {
		element_t* elem = type->struc.elem;
		symtab_t table = type->struc.table;
		while(elem) {
			int categ = elem->decl->common.categ;
			symbol_insert(table, elem->decl->var_name, categ, elem->decl);
			if (record_type(elem->decl)) {
				insert_record_elems(elem->decl);
			}
			elem = elem->next;
		}
	}
	else if (type->common.categ == LAYOUT_T) {
		element_t* elem = type->layout.elem;
		symtab_t table = type->layout.table;
		while(elem) {
			int categ = elem->decl->common.categ;
			symbol_insert(table, elem->decl->var_name, categ, elem->decl);
			if (record_type(elem->decl))
				insert_record_elems(elem->decl);
			elem = elem->next;
		}
	}
	else {
		bit_element_t* elem = type->bitfields.elem;
		symtab_t table = type->bitfields.table;
		while(elem) {
			int catag = elem->decl->common.categ;
			symbol_insert(table, elem->decl->var_name, catag, elem);
			if (record_type(elem->decl))
				insert_record_elems(elem->decl);
			elem = elem->next;
		}
	}

	return;
}

void parse_typedef_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	//cdecl_t* type = parse_cdecl(node->cdecl.decl, table);
	tree_t* decl = node->cdecl.decl;
	cdecl_t* type =  parse_base(decl->cdecl.basetype, table, decl->cdecl.is_const);
	type->common.is_typedef = 1;
	type = parse_decl(decl->cdecl.decl, table, type);
	if (type == NULL)
		return;

	const char* name = type->var_name;
	/* insert the elements that in struct layout, bitfield
	 * into it's table */
	if (record_type(type)) {
		insert_record_elems(type);
	}

	if (table->pass_num == 0) {
		if (type->common.no_decalare == 1) {
			undef_var_insert(table, node);
			free(type);
		}
		else {
			if (type->var_name) {
				type->typedef_name = strdup(type->var_name);
				symbol_insert(table, type->var_name, TYPEDEF_T, type);
			} else {
				error("useless type name in empty declaration\n");
			}
		}
	}
	else {
		if ((type->var_name) && (type->common.no_decalare == 0)) {
			type->typedef_name = strdup(type->var_name);
			if (symbol_insert(table, type->var_name, TYPEDEF_T, type))
				error("duplicate defined '%s'\n", type->var_name);
		}
	}

	return;
}

void parse_top_struct_cdecl(tree_t* node, symtab_t table, void* attr) {
	assert(node != NULL); assert(table != NULL); assert(attr != NULL);

	cdecl_t* decl = parse_struct(node, table);
	insert_record_elems(decl);
	((struct_attr_t*)attr)->decl = decl;

	return;
}

static void parse_star(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	if (node->stars.is_const)
		drv->qual |= CONST_QUAL;
	drv->ctor = POINTER_TO;
	drv->next = type->common.drv;
	type->common.drv = drv;

	if (node->stars.stars) {
		parse_star(node->stars.stars, table, type);
	}

	return;
}

static void parse_ctype_decl_prt(tree_t* node, symtab_t table, cdecl_t* type);
static void parse_ctypedecl_arr(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	if (node->cdecl_brack.decl_list) {
		parse_ctype_decl_prt(node->cdecl_brack.decl_list, table, type);
	}

	return;
}

static void parse_ctype_decl_prt(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	if (node->ctypedecl_ptr.stars) {
		parse_star(node->ctypedecl_ptr.stars, table, type);
	}

	if (node->ctypedecl_ptr.array) {
		parse_ctypedecl_arr(node->ctypedecl_ptr.array, table, type);
	}

	return;
}

cdecl_t* parse_ctype_decl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* type = parse_base(node->ctypedecl.basetype, table, 0);
	if (node->ctypedecl.const_opt)
		type->common.qual |= CONST_QUAL;

	if (node->ctypedecl.ctypedecl_ptr) {
		parse_ctype_decl_prt(node->ctypedecl.ctypedecl_ptr, table, type);
	}

	type = derive_type(type);
	if (type == NULL)
		error("illegal type\n");

	return type;
}

cdecl_t* parse_typeoparg(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	tree_t* tmp = node->typeoparg.ctypedecl;
	tree_t* type_node = tmp ? tmp : node->typeoparg.ctypedecl_brack;
	cdecl_t* type = parse_ctype_decl(type_node, table);

	return type;
}

