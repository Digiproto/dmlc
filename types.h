
/* Copyright (C) 
* 2012 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/

/**
* @file types.h
* @brief The type of variable
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2012-12-04
*/
#ifndef __TYPES_H__
#define __TYPES_H__
#include "types.h"

#define TYPELIST(fn) \
	fn(NO_TYPE) \
	/* dml */ \
	fn(DML_TYPE) \
	/*object type*/ \
	fn(OBJECT_TYPE) \
	/* device */ \
	fn(DEVICE_TYPE) \
	/* parameter */ \
	fn(PARAMETER_TYPE) \
	/* method */ \
	fn(METHOD_TYPE) \
	/* c function  */ \
	fn(FUNCTION_TYPE) \
	/* c function pointer type*/ \
	fn(FUNCTION_POINTER_TYPE) \
	fn(FUNCTION_POINTER_VAR) \
	/* bank */ \
	fn(BANK_TYPE) \
	/* register */ \
	fn(REGISTER_TYPE) \
	/* group */ \
	fn(GROUP_TYPE) \
	/* port */ \
	fn(PORT_TYPE) \
	/* import */ \
	fn(IMPORT_TYPE) \
	/* template */ \
	fn(TEMPLATE_TYPE) \
	/* bitorder */ \
	fn(BITORDER_TYPE) \
	/* loggroup */ \
	fn(LOGGROUP_TYPE) \
	/* header */ \
	fn(HEADER_TYPE) \
	/* foreach */ \
	fn(FOREACH_TYPE) \
	/* call  */ \
	fn(CALL_TYPE) \
	/* data */ \
	fn(DATA_TYPE) \
	/* implement */ \
	fn(IMPLEMENT_TYPE) \
	/* field */ \
	fn(FIELD_TYPE) \
	/* attribute */ \
	fn(ATTRIBUTE_TYPE) \
	/* connect */ \
	fn(CONNECT_TYPE) \
	/* interface */ \
	fn(INTERFACE_TYPE) \
	/* event */ \
	fn(EVENT_TYPE) \
	/* identifier */ \
	fn(IDENT_TYPE) \
	/* method param */ \
	fn(PARAM_TYPE) \
	/* constant string */ \
	fn(CONST_STRING_TYPE) \
	/* constant */ \
	fn(CONSTANT_TYPE) \
	/* array type [10] | [i in ...] */ \
	fn(ARRAY_TYPE) \
	/* ... */ \
	fn(ELLIPSIS_TYPE) \
	/* assign type */ \
	fn(ASSIGN_TYPE) \
	/* exprssion = exprssion */ \
	fn(EXPR_ASSIGN_TYPE) \
	/* exprssion with brackets */ \
	fn(EXPR_BRACK_TYPE) \
	/* unary operator */ \
	fn(UNARY_TYPE) \
	/* binary operator */ \
	fn(BINARY_TYPE) \
	/* ternary operator */ \
	fn(TERNARY_TYPE) \
	/* default */ \
	fn(DEFAULT_TYPE) \
	/* auto */ \
	fn(AUTO_TYPE) \
	/* $ */ \
	fn(QUOTE_TYPE) \
	/* dml keyword */ \
	fn(DML_KEYWORD_TYPE) \
	/* c keyword */ \
	fn(C_KEYWORD_TYPE) \
	/* ( decl_list )*/ \
	fn(CDECL_BRACK_TYPE) \
	/* ctypedecl */ \
	fn(CTYPEDECL_TYPE) \
	/* ctypedecl_ptr */ \
	fn(CTYPEDECL_PTR_TYPE) \
	/* ctypedecl_simple */ \
	fn(CTYPEDECL_SIMPLE_TYPE) \
	/* typeoparg */ \
	fn(TYPEOPARG_TYPE) \
	/* stars */ \
	fn(STARS_TYPE) \
	/* new */ \
	fn(NEW_TYPE) \
	/* bit slicing exprssion */ \
	fn(BIT_SLIC_EXPR_TYPE) \
	/* if... else ...*/ \
	fn(IF_ELSE_TYPE) \
	/* log */ \
	fn(LOG_TYPE) \
	/* . | ->*/ \
	fn(COMPONENT_TYPE) \
	/* undefined */ \
	fn(UNDEFINED_TYPE) \
	/* throw */ \
	fn(THROW_TYPE) \
	/* const */ \
	fn(CONST_TYPE) \
	/* extern */ \
	fn(EXTERN_KEY_TYPE) \
	/* method extern  */ \
	fn(METHOD_EXTERN_TYPE) \
	/* goto */ \
	fn(GOTO_TYPE) \
	/* label */ \
	fn(LABEL_TYPE) \
	/* break */ \
	fn(BREAK_TYPE) \
	/* continue */ \
	fn(CONTINUE_TYPE) \
	/* inline */ \
	fn(INLINE_TYPE) \
	/* do while */ \
	fn(DO_WHILE_TYPE) \
	/* for */ \
	fn(FOR_TYPE) \
	/* switch */ \
	fn(SWITCH_TYPE) \
	/* delete */ \
	fn(DELETE_TYPE) \
	/* try catch */ \
	fn(TRY_CATCH_TYPE) \
	/* local_keyword */ \
	fn(LOCAL_KEYWORD_TYPE) \
	/* after call */ \
	fn(AFTER_CALL_TYPE) \
	/* error */ \
	fn(ERROR_TYPE) \
	/* select */ \
	fn(SELECT_TYPE) \
	/* return */ \
	fn(RETURN_TYPE) \
	/* cast */ \
	fn(CAST_TYPE) \
	/* case */ \
	fn(CASE_TYPE) \
	/* assert */ \
	fn(ASSERT_TYPE) \
	/* sizeof */ \
	fn(SIZEOF_TYPE) \
	/* local */ \
	fn(LOCAL_TYPE) \
	/* sizeoftype */ \
	fn(SIZEOFTYPE_TYPE) \
	/* spec of boject */ \
	fn(SPEC_TYPE) \
	/* block  */ \
	fn(BLOCK_TYPE) \
	/* cdecl  */ \
	fn(CDECL_TYPE) \
	/* bitfields */ \
	fn(BITFIELDS_TYPE) \
	/* bitfields decl */ \
	fn(BITFIELDS_DECL_TYPE) \
	/* layout */ \
	fn(LAYOUT_TYPE) \
	/* typedef */ \
	fn(TYPEDEF_TYPE) \
	/* typeof */ \
	fn(TYPEOF_TYPE) \
	/* bool */ \
	fn(BOOL_TYPE) \
	/* const int data: 0, 1, 3 */ \
	fn(INTEGER_TYPE) \
	/* float data */ \
	fn(FLOAT_TYPE) \
	/* c keyword */ \
	fn(CHAR_TYPE) \
	fn(DOUBLE_TYPE) \
	fn(INT_TYPE) \
	fn(LONG_TYPE) \
	fn(SHORT_TYPE) \
	fn(SIGNED_TYPE) \
	fn(UNSIGNED_TYPE) \
	fn(VOID_TYPE) \
	fn(ENUM_TYPE) \
	fn(UNION_TYPE) \
	/* struct */ \
	fn(STRUCT_TYPE) \
	fn(SIZE_TYPE) \
	fn(CLASS_TYPE) \
	fn(NAMESPACE_TYPE) \
	fn(PRIVATE_TYPE) \
	fn(PROTECTED_TYPE) \
	fn(PUBLIC_TYPE) \
	fn(RESTRICT_TYPE) \
	fn(USING_TYPE) \
	fn(VIRTUAL_TYPE) \
	fn(VOLATILE_TYPE) \
	fn(FOOTER_TYPE) \
	fn(THIS_TYPE) \
	fn(TMP_TYPE) \
	fn(EXPR_STATEMENT) \
	/* dictionary */ \
	fn(DICTIONARY_TYPE) \
	fn(BITFIELDS_ELEM_TYPE) \
	fn(OBJECT_IF) \
	/* nil */ \
	fn(NIL_TYPE)

#define type_produce(x) x,
typedef enum
{
	type_produce(START_TYPE)
	TYPELIST(type_produce)
	type_produce(TYPE_END)
} type_t;

#define name_produce(x) \
	[(x)] = #x,
static const char *type_name[] = {
	name_produce(START_TYPE)
	TYPELIST(name_produce)
	name_produce(TYPE_END)
};

#define DATA_TYPELIST(fn) \
	fn(BOOL_T) \
	fn(CHAR_T) \
	fn(SHORT_T) \
	fn(INT_T) \
	fn(UINT_T) \
	fn(BITFIELDS_T) \
	fn(LONG_T) \
	fn(FLOAT_T) \
	fn(DOUBLE_T) \
	fn(STRING_T) \
	fn(POINTER_T) \
	fn(VOID_T) \
	fn(ARRAY_T) \
	fn(STRUCT_T) \
	fn(LAYOUT_T) \
	fn(TYPEDEF_T) \
	fn(FUNCTION_T) \
	fn(INTERFACE_T)

enum data_type {
	DATA_TYPE_START = TYPE_END + 1,
	DATA_TYPELIST(type_produce)
	DATA_TYPE_END,
}data_type_t;

#define data_name_produce(x) \
	[(x) - DATA_TYPE_START] = #x,
static const char *data_type_name[] = {
	data_name_produce(DATA_TYPE_START)
	DATA_TYPELIST(data_name_produce)
	data_name_produce(DATA_TYPE_END)
};

#define TYPENAME(X) \
	( \
		((X) < START_TYPE || (X) > TYPE_END) ? \
			( \
				((X) >= DATA_TYPE_START && (X) <= DATA_TYPE_END) ? \
					data_type_name[(X)] \
						: \
					"Unknown_TYPE" \
			) \
		: \
		type_name[(X)] \
	)

typedef enum {
	TYPE_START = DATA_TYPE_END,
	/*-----expr assign type----*/
	/* += */
	ADD_ASSIGN_TYPE,
	/* -= */
	SUB_ASSIGN_TYPE,
	/* *= */
	MUL_ASSIGN_TYPE,
	/* /= */
	DIV_ASSIGN_TYPE,
	/* %= */
	MOD_ASSIGN_TYPE,
	/* |= */
	OR_ASSIGN_TYPE,
	/* &= */
	AND_ASSIGN_TYPE,
	/* ^= */
	XOR_ASSIGN_TYPE,
	/* <<= */
	LEFT_ASSIGN_TYPE,
	/* >>= */
	RIGHT_ASSIGN_TYPE,

	/*-----binary operat------ */
	/* + */
	ADD_TYPE,
	/* - */
	SUB_TYPE,
	/* * */
	MUL_TYPE,
	/* / */
	DIV_TYPE,
	/* % */
	MOD_TYPE,
	/* << */
	LEFT_OP_TYPE,
	/* >> */
	RIGHT_OP_TYPE,
	/* == */
	EQ_OP_TYPE,
	/* != */
	NE_OP_TYPE,
	/* < */
	LESS_TYPE,
	/* > */
	GREAT_TYPE,
	/* <= */
	LESS_EQ_TYPE,
	/* >= */
	GREAT_EQ_TYPE,
	/* || */
	OR_OP_TYPE,
	/* && */
	AND_OP_TYPE,
	/* | */
	OR_TYPE,
	/* ^ */
	XOR_TYPE,
	/* & */
	AND_TYPE,

	/* unary operator */
	/* - */
	NEGATIVE_TYPE,
	/* + */
	CONVERT_TYPE,
	/* ! */
	NON_OP_TYPE,
	/* ~ */
	BIT_NON_TYPE,
	/* & */
	ADDR_TYPE,
	/* * */
	POINTER_TYPE,
	/* defined */
	DEFINED_TYPE,
	/* vect */
	VECT_TYPE,
	/* # */
	EXPR_TO_STR_TYPE,
	/* ++A */
	PRE_INC_OP_TYPE,
	/* --A */
	PRE_DEC_OP_TYPE,
	/* A++ */
	AFT_INC_OP_TYPE,
	/* A-- */
	AFT_DEC_OP_TYPE,

	/*------component_type-----*/
	/* . */
	COMPONENT_DOT_TYPE,
	/* -> */
	COMPONENT_POINTER_TYPE,
}operator_type_t;

static const char *sym_type2str[] = {
	[NO_TYPE] = "none",
	[DML_TYPE] = "dml",
	[OBJECT_TYPE] = "object",
	[DEVICE_TYPE] = "device",
	[PARAMETER_TYPE] = "parameter",
	[METHOD_TYPE]    = "method",
	[BANK_TYPE]      = "bank",
	[REGISTER_TYPE]  = "register",
	[GROUP_TYPE]     = "group",
	[PORT_TYPE]      = "port",
	[IMPORT_TYPE]    = "import",
	[FOREACH_TYPE]   = "foreach",
	[TEMPLATE_TYPE]  = "template",
	[BITORDER_TYPE]  = "bitorder",
	[IDENT_TYPE]     = "ident",
	[DML_KEYWORD_TYPE] = "dml keyword",
	[C_KEYWORD_TYPE]  = "c keyword",
	[IF_ELSE_TYPE]    = "if...esle...",
	[THIS_TYPE]      = "this",
};
#endif
