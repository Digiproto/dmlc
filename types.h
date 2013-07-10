
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
typedef enum
{
	NO_TYPE = 1,
	/* dml */
	DML_TYPE,
	/* device */
	DEVICE_TYPE,
	/* parameter */
	PARAMETER_TYPE,
	/* method */
	METHOD_TYPE,
	/* c function  */
	FUNCTION_TYPE,
	/* c function pointer type*/
	FUNCTION_POINTER_TYPE,
	FUNCTION_POINTER_VAR,
	/* bank */
	BANK_TYPE,
	/* register */
	REGISTER_TYPE,
	/* group */
	GROUP_TYPE,
	/* port */
	PORT_TYPE,
	/* import */
	IMPORT_TYPE,
	/* template */
	TEMPLATE_TYPE,
	/* bitorder */
	BITORDER_TYPE,
	/* loggroup */
	LOGGROUP_TYPE,
	/* header */
	HEADER_TYPE,
	/* foreach */
	FOREACH_TYPE,
	/* call  */
	CALL_TYPE,
	/* data */
	DATA_TYPE,
	/* implement */
	IMPLEMENT_TYPE,
	/* field */
	FIELD_TYPE,
	/* attribute */
	ATTRIBUTE_TYPE,
	/* connect */
	CONNECT_TYPE,
	/* interface */
	INTERFACE_TYPE,
	/* event */
	EVENT_TYPE,
	/* identifier */
	IDENT_TYPE,
	/* method param */
	PARAM_TYPE,
	/* constant string */
	CONST_STRING_TYPE,
	/* constant */
	CONSTANT_TYPE,
	/* array type [10] | [i in ...] */
	ARRAY_TYPE,
	/* ... */
	ELLIPSIS_TYPE,
	/* assign type */
	ASSIGN_TYPE,
	/* exprssion = exprssion */
	EXPR_ASSIGN_TYPE,
	/* exprssion with brackets */
	EXPR_BRACK_TYPE,
	/* unary operator */
	UNARY_TYPE,
	/* binary operator */
	BINARY_TYPE,
	/* ternary operator */
	TERNARY_TYPE,
	/* default */
	DEFAULT_TYPE,
	/* auto */
	AUTO_TYPE,
	/* $ */
	QUOTE_TYPE,
	/* dml keyword */
	DML_KEYWORD_TYPE,
	/* c keyword */
	C_KEYWORD_TYPE,
	/* ( decl_list )*/
	CDECL_BRACK_TYPE,
	/* ctypedecl */
	CTYPEDECL_TYPE,
	/* ctypedecl_ptr */
	CTYPEDECL_PTR_TYPE,
	/* ctypedecl_simple */
	CTYPEDECL_SIMPLE_TYPE,
	/* typeoparg */
	TYPEOPARG_TYPE,
	/* stars */
	STARS_TYPE,
	/* new */
	NEW_TYPE,
	/* bit slicing exprssion */
	BIT_SLIC_EXPR_TYPE,
	/* if... else ...*/
	IF_ELSE_TYPE,
	/* log */
	LOG_TYPE,
	/* . | ->*/
	COMPONENT_TYPE,
	/* undefined */
	UNDEFINED_TYPE,
	/* throw */
	THROW_TYPE,
	/* const */
	CONST_TYPE,
	/* extern */
	EXTERN_KEY_TYPE,
	/* method extern  */
	METHOD_EXTERN_TYPE,
	/* goto */
	GOTO_TYPE,
	/* break */
	BREAK_TYPE,
	/* continue */
	CONTINUE_TYPE,
	/* inline */
	INLINE_TYPE,
	/* do while */
	DO_WHILE_TYPE,
	/* for */
	FOR_TYPE,
	/* switch */
	SWITCH_TYPE,
	/* delete */
	DELETE_TYPE,
	/* try catch */
	TRY_CATCH_TYPE,
	/* local_keyword */
	LOCAL_KEYWORD_TYPE,
	/* after call */
	AFTER_CALL_TYPE,
	/* error */
	ERROR_TYPE,
	/* select */
	SELECT_TYPE,
	/* return */
	RETURN_TYPE,
	/* cast */
	CAST_TYPE,
	/* case */
	CASE_TYPE,
	/* assert */
	ASSERT_TYPE,
	/* sizeof */
	SIZEOF_TYPE,
	/* local */
	LOCAL_TYPE,
	/* sizeoftype */
	SIZEOFTYPE_TYPE,
	/* spec of boject */
	SPEC_TYPE,
	/* block  */
	BLOCK_TYPE,
	/* cdecl  */
	CDECL_TYPE,
	/* bitfields */
	BITFIELDS_TYPE,
	/* bitfields decl */
	BITFIELDS_DECL_TYPE,
	/* layout */
	LAYOUT_TYPE,
	/* typedef */
	TYPEDEF_TYPE,
	/* typeof */
	TYPEOF_TYPE,
	/* bool */
	BOOL_TYPE,
	/* const int data: 0, 1, 3 */
	INTEGER_TYPE,
	/* float data */
	FLOAT_TYPE,
	/* c keyword */
	CHAR_TYPE,
	DOUBLE_TYPE,
	INT_TYPE,
	LONG_TYPE,
	SHORT_TYPE,
	SIGNED_TYPE,
	UNSIGNED_TYPE,
	VOID_TYPE,
	ENUM_TYPE,
	UNION_TYPE,
	/* struct */
	STRUCT_TYPE,
	SIZE_TYPE,
	CLASS_TYPE,
	NAMESPACE_TYPE,
	PRIVATE_TYPE,
	PROTECTED_TYPE,
	PUBLIC_TYPE,
	RESTRICT_TYPE,
	USING_TYPE,
	VIRTUAL_TYPE,
	VOLATILE_TYPE,
	FOOTER_TYPE,
	THIS_TYPE,
	/* object */
	OBJECT_TYPE,
	/* dictionary */
	DICTIONARY_TYPE,
	/* nil */
	NIL_TYPE,
	TYPE_END
} type_t;

typedef enum {
	TYPE_START = TYPE_END,
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

#endif
