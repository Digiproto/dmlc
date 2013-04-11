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
typedef enum{
	DML_TYPE = 1,
	DEVICE_TYPE,
	PARAMETER_TYPE,
	METHOD_TYPE,
	BANK_TYPE,
	REGISTER_TYPE,
	IMPORT_TYPE,
	TEMPLATE_TYPE,
	IS_TYPE,
	BITORDER_TYPE,
	LOGGROUP_TYPE,
	TYPEDEF_TYPE,
	CONSTANT_TYPE,
	HEADER_TYPE,
	FOREACH_TYPE,
	CALL_TYPE,
	EXPRESSION_TYPE,
	DATA_TYPE,
	EXTERN_TYPE,
	IMPLEMENT_TYPE,
	FIELD_TYPE,
	AFTER_TYPE,
	ATTRIBUTE_TYPE,
	CONNECT_TYPE,
	INTERFACE_TYPE,
	IDENTIFER_TYPE,
	/* constant string */
	CONST_STRING_TYPE,
	/* variable string */
	VAR_STRING_TYPE,
	/* indentifier type */
	INDENTIFIER_TYPE,
	/* the array with a fix length eg: arr[10] */
	ARRAY_FIX_TYPE,
	/* array like [i int 0..15]*/
	ARRAY_RANGE_TYPE,
	/* array minist index */
	ARRAY_START_TYPE,
	/* array max index */
	ARRAY_END_TYPE,
	/* ; */
	SEMICOLON_TYPE,
	/* ... */
	ELLIPSIS_TYPE,
	/* assign type */
	ASSIGN_TYPE,
	/* default */
	DEFAULT_TYPE,
	/* auto */
	AUTO_TYPE,
	/* istemplate */
	ISTEMPLATE_TYPE,
	/* == */
	EQ_OP_TYPE,
	/* $ */
	QUOTE_TYPE,
	/* - */
	MINUS_TYPE,
	/* ( */
	PRE_BRACKETS_TYPE,
	/* ) */
	AFTER_BRACKETS_TYPE,
	/* { */
	PRE_BRACES_TYPE,
	/* } */
	AFTER_BRACES_TYPE,
	/* [ */
	PRE_MID_BRACES_TYPE,
	/* ] */
	AFTER_MID_BRACES_TYPE,
	/* ? */
	QUEST_MARK_TYPE,
	/* : */
	COLON_TYPE,
	/* , */
	COMMA_TYPE,
	/* @ */
	REG_OFFSET_TYPE,
	/* basic type */
	BASETYPE_TYPE,
	/* -> */
	METHOD_RETURN_TYPE,
	/* int data */
	INTEGER_TYPE,
	/* type identify : int8 int16 */
	TYPEIDENT_TYPE,
	/* keyword type : static */
	KEY_WORD_TYPE,
	/* IF_TYPE */
	IF_TYPE,
	/* else */
	ELSE_TYPE,
	/* > */
	GREATER_TYPE,
	/* || */
	OR_OP_TYPE,
	/* && */
	AND_OP_TYPE,
	/* != */
	EN_OP_TYPE,
	/* ! */
	EXCLAM_TYPE,
	/* log */
	LOG_KYE_TYPE,
	LOG_TYPE,
	/* | */
	OR_TYPE,
	/* & */
	AND_TYPE,
	/* ~ */
	NON_TYPE,
	/* |= */
	OR_ASSIGN_TYPE,
	/* . */
	PERIOD_TYPE,
	/* * */
	POINT_TYPE,
	/* & */
	ADDRESS_TYPE,
	/* + */
	ADD_TYPE,
	/* * */
	MUL_TYPE,
	/* / */
	DIV_TYPE,
	/* % */
	REMAIN_TYPE,
	/* - */
	NEGAT_TYPE,
	/* + */
	POSIT_TYPE,
	/* < */
	LESS_TYPE,
	/* <= */
	LE_OP_TYPE,
	/* >= */
	GE_OP_TYPE,
	/* << */
	LEFT_OP_TYPE,
	/* << */
	RIGHT_OP_TYPE,
	/* &= */
	AND_ASSIGN_TYPE,
	/* defined */
	DEFINED_TYPE,
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
	/* inline */
	INLINE_TYPE,
	/* try */
	TRY_TYPE,
	/* catch */
	CATCH_TYPE,
	/* local_keyword */
	LOCAL_KEYWORD_TYPE,
	/* error */
	ERROR_TYPE,
	/* typeof */
	TYPEOF_TYPE,
	/* return */
	RETURN_TYPE,
	/* cast */
	CAST_TYPE,
	/* assert */
	ASSERT_TYPE
}ident_type_t;

typedef struct obj_variable{
	int type;
	char* name;
}objtype;
typedef struct expression{
}expression_t;
#endif
