
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
typedef enum
{
	/* dml */
	DML_TYPE = 1,
	/* device */
	DEVICE_TYPE,
	/* parameter */
	PARAMETER_TYPE,
	/* method */
	METHOD_TYPE,
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
	PARM_TYPE,
	/* constant string */
	CONST_STRING_TYPE,
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
	/* int data */
	INTEGER_TYPE,
	/* type identify : int8 int16 */
	TYPEIDENT_TYPE,
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
	/* defined */
	DEFINED_TYPE,
	/* undefined */
	UNDEFINED_TYPE,
	/* throw */
	THROW_TYPE,
	/* const */
	CONST_KEY_TYPE,
	/* extern */
	EXTERN_KEY_TYPE,
	/* struct */
	STRUCT_TYPE,
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
	/* typeof */
	TYPEOF_TYPE,
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
	LAYOUT_TYPE
} ident_type_t;

typedef struct obj_variable
{
	int type;
	char *name;
} objtype;
typedef struct expression
{
} expression_t;
#endif
