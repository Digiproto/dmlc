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

char* add_type_str(char* addr, char* str){
	printf("In %s, line = %d, addr : %s :  str: %s\n",
			__func__, __LINE__, addr, str);
	addr = (char*)gdml_realloc(addr, (strlen(str) + 2));
	addr = strcat(addr, str);
	addr = strcat(addr, " ");
	printf("In %s, line = %d, str: %s : addr: %s\n",
			__func__, __LINE__, str, addr);

	return addr;
}

void set_decl_type(decl_type_t* decl_type, type_t type) {
	assert(decl_type != NULL);

	switch(type) {
		case CHAR_TYPE:
			decl_type->char_type = 1;
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
}

decl_type_t*  parse_identifier(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(decl != NULL);

	if ((node->common.type) != IDENT_TYPE) {
		fprintf(stderr, "node is other type: %s\n", node->common.name);
		/* FIXME: handle the error */
		exit(-1);
	}

	printf("In %s, line = %d, find symbol: %s\n",
			__func__, __LINE__, node->ident.str);
	symbol_t symbol = symbol_find_notype(table, node->ident.str);
	if (symbol != NULL) {
		printf("In %s, line = %d, finded the symbol: %s\n", node->ident.str);
		/* FIXME: the symbol is struct or other */
		exit(-1);
	}
	else {
		decl->decl_str = add_type_str(decl->decl_str, node->ident.str);
		if (decl->var != NULL) {
			var_name_t* var = decl->var;
			while (var->next != NULL)  {
				var = var->next;
			}
			var_name_t* var_new = (var_name_t*)gdml_zmalloc(sizeof(var_name_t));
			var_new->var_name = node->ident.str;
			var->next = var_new;
			var->var_num += 1;
		}
		else {
			var_name_t* var = (var_name_t*)gdml_zmalloc(sizeof(var_name_t));
			var->var_name = node->ident.str;
			var->var_num += 1;
			decl->var = var;
		}
		//symbol_insert(table, node->ident.str, IDENT_TYPE, decl);
	}

	return NULL;
}

decl_t* parse_struct(tree_t* node, symtab_t table, decl_t* decl) {
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);
	exit(-1);

	return decl;
}

decl_t* parse_layout(tree_t* node, symtab_t table, decl_t* decl) {
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	exit(-1);
	return decl;
}

decl_t* parse_bitfields(tree_t* node, symtab_t table, decl_t* decl) {
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	exit(-1);
	return decl;
}

decl_t* parse_typeof(tree_t* node, symtab_t table, decl_t* decl) {
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	exit(-1);
	return decl;
}

int parse_char(decl_t* decl) {
	assert(decl);
	decl_type_t* type = decl->type;
	if ((type->char_type) || (type->double_type) || (type->float_type) ||
			(type->int_type) || (type->void_type) ||
			(type->bool_type) || (type->struct_type) || (type->layout_type) ||
			(type->bitfields_type) || (type->typeof_type) ||
			(type->enum_type) || (type->union_type)) {
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
		printf("In %s, line = %d, decl str: %s\n",
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
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
			|| (type->union_type) || (type->typedef_type)) {
		fprintf(stderr, "error: two or more data types in declaration specifiers\n");
		/* FIXME: handle the error */
		exit(-1);
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
	printf("In %s, line = %d, node->type: %s\n",
			__func__, __LINE__, node->common.name);

	switch(node->ident.type) {
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
	printf("IN %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);
	exit(-1);

	return decl;
}

decl_t* parse_basetype(tree_t* node, symtab_t table, decl_t* decl) {
	 assert(node != NULL);
	 assert(table != NULL);
	 assert(decl != NULL);
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	switch (node->common.type) {
		case STRUCT_TYPE:
			printf("struct defined !\n");
			parse_struct(node, table, decl);
			break;
		case LAYOUT_TYPE:
			printf("layout defined!\n");
			parse_layout(node, table, decl);
			break;
		case BITFIELDS_TYPE:
			printf("bitfields defined!\n");
			parse_bitfields(node, table, decl);
			break;
		case TYPEOF_TYPE:
			printf("typeof defined!\n");
			 parse_typeof(node, table, decl);
			break;
		default:
			parse_typeident(node, table, decl);
			break;
	}

	return decl;
}

params_t* get_param_decl(tree_t* node, symtab_t table) {
	assert(node != NULL);
	if (node->common.type != CDECL_TYPE) {
		printf("params type is : %s\n", node->common.name);
		exit(-1);
	}
	params_t* param = (params_t*)gdml_zmalloc(sizeof(params_t));
	decl_t* decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
	decl_type_t* type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
	decl->type = type;
	param->decl = decl;

	parse_cdecl(node, table, decl);
	if ((decl->var->var_num) > 1) {
		fprintf(stderr, "the method parameter some problem!\n");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		param->var_name = decl->var->var_name;
		if ((node->cdecl.decl) == NULL) {
			param->is_notype = 1;
		}
	}

	printf("IN %s, line = %d, decl_str: %s, va_name: %s\n",
			__func__, __LINE__, decl->decl_str, decl->var->var_name);

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
		parse_cdecl2(decl2, table, decl);
	}

	if (node->cdecl.is_point) {
		decl->type->point_type += 1;
		decl->decl_str = add_type_str(decl->decl_str, "*");
		parse_cdecl2(node->cdecl.decl, table, decl);
	}

	if (node->cdecl.is_vect) {
		decl->type->vect_type = 1;
		decl->decl_str = add_type_str(decl->decl_str, "vect");
		parse_cdecl2(node->cdecl.decl, table, decl);
	}

	return 0;
}

expression_t* parse_array_expression(tree_t* node, symtab_t table) {
	assert(node != NULL);
	assert(table != NULL);
	expression_t* expr = NULL;
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	while (node != NULL) {
		if (expr == NULL) {
			expr = parse_expression(node, table);
		}
		else {
			expr->next = parse_expression(node, table);
		}
		node = node->common.sibling;
	}

	return expr;
}

int parse_array(tree_t* node, symtab_t table, decl_t* decl) {
	assert((node != NULL) || (table != NULL) || (decl != NULL));
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	array_decl_t* array_decl = (array_decl_t*)gdml_zmalloc(sizeof(array_decl_t));

	if (node->array.decl) {
		parse_cdecl3(node->array.decl, table, decl);
		/* FIXME should get array identifier */
	}

	decl->type->array_type += 1;
	decl->decl_str = add_type_str(decl->decl_str, "[");

	array_decl->is_fix = node->array.is_fix;

	if (node->array.ident) {
		tree_t* ident = node->array.ident;
		if ((ident->common.type) != IDENT_TYPE) {
			fprintf(stderr, "the identifier type is: %s\n", ident->common.name);
			/* FIXME: handle the error */
			exit(-1);
		}
		printf("array ident: %s\n", ident->ident.str);
		if (symbol_insert(table, ident->ident.str, IDENT_TYPE, NULL) == -1) {
			fprintf(stderr, "redfined: %s\n", ident->ident.str);
			/* FIXME: handle  the error */
			exit(-1);
		}
	}

	array_decl->expr = parse_array_expression(node->array.expr, table);
	array_decl->expr_end = parse_array_expression(node->array.expr_end, table);

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
	printf("In %s, line = %d, node type: %s\n",
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
		parse_cdecl(node, table, decl);
	}

	return 0;
}

int parse_decl_brack(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	/*
	 * Grammar:
	 *		cdecl3 '(' cdecl_list ')'
	 *		| '(' cdecl2 ')'
	 */
	parse_cdecl3(node->cdecl_brack.cdecl, table, decl);

	decl->decl_str = add_type_str(decl->decl_str, "( ");

	if (node->cdecl_brack.is_list) {
		parse_cdecl_list((node->cdecl_brack.decl_list), table, decl);
	}
	else {
		parse_cdecl2(node->cdecl_brack.decl_list, table, decl);
	}

	decl->decl_str = add_type_str(decl->decl_str, ") ");

	return 0;
}

int parse_typeident(tree_t* node, symtab_t table, decl_t* decl) {
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

int parse_cdecl3(tree_t* node, symtab_t table, decl_t* decl) {
	if (node == NULL) {
		return 0;
	}

	assert(table != NULL);
	assert(decl != NULL);

	switch (node->common.type) {
		case ARRAY_TYPE:
			parse_array(node, table, decl);
			break;
		case CDECL_BRACK_TYPE:
			parse_decl_brack(node, table, decl);
			break;
		default:
			parse_typeident(node, table, decl);
			break;
	}
	return 0;
}

int parse_cdecl2(tree_t* node, symtab_t table, decl_t* decl) {
	if (node == NULL) {
		return 0;
	}

	switch(node->common.type) {
		case CDECL_TYPE:
			parse_cdecl2_modify(node, table, decl);
			break;
		default:
			parse_cdecl3(node, table, decl);
			break;
	}

	return 0;
}

int parse_cdecl(tree_t* node, symtab_t table, decl_t* decl) {
	assert(node != NULL);
	assert(table != NULL);
	assert(table != NULL);
	printf("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, node->common.name);

	decl_type_t* type = decl->type;

	if (node->cdecl.is_const) {
		decl->type->type_const += 1;
		decl->decl_str = add_type_str(decl->decl_str, "const ");
		printf("In %s, line = %d, decl_str: %s\n",
				__func__, __LINE__, decl->decl_str);
	}

	if (node->cdecl.basetype) {
		parse_basetype(node->cdecl.basetype, table, decl);
	}
	printf("In %s, line = %d, decl_str: %s\n",
			__func__, __LINE__, decl->decl_str);

	if (node->cdecl.decl) {
		parse_cdecl2(node->cdecl.decl, table, decl);
	}

	return 0;
}

int charge_integer_expr(decl_type_t* type) {
	assert(type != NULL);
	if ((type->char_type) || (type->int_type) || (type->double_type) ||
			(type->float_type) || (type->short_type) || (type->long_type) ||
			(type->unsigned_type) || (type->signed_type)) {
		return 0;
	}
	else if (type->point_type) {
		fprintf(stderr, "warning: assignment makes pointer from integer without a cast\n");
		/* FIXME: handle the error */
		exit(-1);
		return 1;
	}
	else {
		fprintf(stderr, "error: invalid initializer\n");
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
			(type->unsigned_type) || (type->signed_type)) {
		return 0;
	}
	else {
		fprintf(stderr, "error: invalid initializer\n");
		/* FIXME: handle the error */
		exit(-1);
		return -1;
	}

	return 0;
}

int charge_decl_expr_type(decl_t* decl, expression_t* expr) {
	assert(decl != NULL);
	assert(expr != NULL);

	int ret = 0;

	switch (expr->final_type) {
		case INTEGER_TYPE:
		case INT_TYPE:
			ret = charge_integer_expr(decl->type);
			break;
		case FLOAT_TYPE:
			ret = charge_float_expr(decl->type);
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
	int type = get_decl_type(decl);

	char* name = NULL;
	var_name_t* var = decl->var;
	while (var) {
		symbol_insert(table, var->var_name, type, decl);
		var = var->next;
	}

	return;
}

void parse_local_decl(tree_t* node, symtab_t table) {
	assert(node != NULL);
	assert(table != NULL);

	decl_t* decl = (decl_t*)gdml_zmalloc(sizeof(decl_t));
	decl_type_t* type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
	decl->type = type;

	if (node->local_tree.is_static) {
		type->is_static = 1;
		decl->decl_str = malloc(sizeof("static "));
		sprintf(decl->decl_str, "%s", "static ");
	}

	if (node->local_tree.local_keyword) {
		tree_t* keyword = node->local_tree.local_keyword;
		char* keyword_name = keyword->local_keyword.name;
		if (strcmp(keyword_name, "auto") == 0) {
			type->is_auto = 1;
			//decl->decl_str = malloc(sizeof("auto "));
			//sprintf(decl->decl_str, "%s", "auto ");
			decl->decl_str = add_type_str(decl->decl_str, "auto ");
		}
		else if (strcmp(keyword_name, "local") == 0) {
			type->is_local = 1;
			//decl->decl_str = malloc(sizeof("local "));
			//sprintf(decl->decl_str, "%s", "local ");
			decl->decl_str = add_type_str(decl->decl_str, "local ");
		}
		else {
			fprintf(stderr, "Other local keyword: %s\n", keyword_name);
			/* FIXME: handle the error */
			exit(-1);
		}
	}

	if ((node->local_tree.cdecl) == NULL) {
		fprintf(stderr, "local decal need decl!\n");
		/* FIXME: handle the error */
		exit(-1);
	}

	parse_cdecl(node->local_tree.cdecl, table, decl);

	printf("In %s, line = %d, decl_str: %s, var_name: %s\n",
			__func__, __LINE__, decl->decl_str, decl->var->var_name);

	if (node->local_tree.expr) {
		expression_t* expr = parse_expression(node->local_tree.expr, table);
		if (charge_decl_expr_type(decl, expr) != 0) {
			fprintf(stderr, "error: invalid initializer\n");
			return;
		}
		decl->value =  expr;
		printf("In %s, line = %d, value final type: %d : %d\n",
				__func__, __LINE__, decl->value->final_type, INTEGER_TYPE);
	}

	insert_ident_decl(table, decl);

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

