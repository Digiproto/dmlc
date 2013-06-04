/*
 * tree.h:
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

#ifndef __TREE_H__
#define __TREE_H__

#include "symbol-common.h"
//#include "decl.h"

/**
 * @brief : location of token in file
 */
struct location_s
{
	/* The name of the source file involved.  */
	const char *file;
	/* The line-location in the source file.  */
	int line;
};
typedef struct location_s location_t;

union tree_node;
typedef union tree_node tree_t;
typedef void (*print_node_t)(tree_t*, int);

struct indentifier {
	char* str;
	struct indentifier* next;
};

/**
 * @brief : the common part about tree node
 */
struct tree_common
{
	tree_t* child;
	tree_t* sibling;
	tree_t* chain;
	print_node_t print_node;
	void* attr;
	int type;
	const char* name;
};

#if 0
struct tree_int_cst
{
  struct tree_common common;
  /* A sub-struct is necessary here because the function `const_hash'
     wants to scan both words as a unit and taking the address of the
     sub-struct yields the properly inclusive bounded pointer.  */
  struct tree_int_cst_lowhi {
    unsigned int low;
    unsigned int high;
  } int_cst;
};
#endif


/**
 * @brief : tree node about const string
 */
struct tree_string
{
  struct tree_common common;
  int length;
  const char *pointer;
};


/**
 * @brief : tree node about object spec
 */
struct tree_object_spec {
	struct tree_common common;
	tree_t* desc;
	tree_t* block;
};

/**
 * @brief : tree node about bank
 */
struct tree_bank
{
	struct tree_common common;
	const char* name;
	tree_t* templates;				// templates that bank inherit
	tree_t* spec;					// the spec about bank
};

/**
 * @brief : tree node about dml,
 * the node about the version of dml
 */
struct tree_dml
{
	struct tree_common common;
	const char* version;
};

/**
 * @brief : tree node about device bitorder
 * big endian or little endian
 */
struct tree_bitorder
{
	struct tree_common common;
	const char* endian;
};

/**
 * @brief : tree node about device,
 * it includes the name of device
 */
struct tree_device
{
	struct tree_common common;
	const char* name;
};

/**
 * @brief : tree node about parameter,
 * includes identifier and spec about parameter
 */
struct tree_param {
	struct tree_common common;
	const char* name;
	tree_t* paramspec;
};

/**
 * @brief : tree node about paramspec,
 * the parameter is auto|const string|default|auto|expression
 */
struct tree_paramspec {
	struct tree_common common;
	int is_auto;
	int is_default;
	tree_t* string;
	tree_t* expr;
};

/**
 * @brief : tree node about register information
 */
struct tree_register {
	struct tree_common common;
	const char* name;
	tree_t* sizespec;			// register size
	tree_t* offset;				// offset about register start
	char** templates;			// templates that register inherits
	tree_t* spec;				// spec about register
	tree_t* array;				// the array about registers that are the same type
};

/**
 * @brief : tree node about field information
 */
struct tree_field {
	struct tree_common common;
	const char* name;
	tree_t* bitrange;		// the bit range about field
	tree_t* templates;		// templates that file inherits
	tree_t* spec;			// spec about field
};

/**
 * @brief : tree node about data decl
 */
struct tree_data {
	struct tree_common common;
	tree_t* cdecl;
};

/**
 * @brief : tree node about connect
 */
struct tree_connect {
	struct tree_common common;
	const char* name;
	tree_t* arraydef;			// the array about connect object
	tree_t* templates;			// templates that connect inherits
	tree_t* spec;				// spec about connect
};

/**
 * @brief : tree node about interface
 */
struct tree_interface {
	struct tree_common common;
	const char* name;
	tree_t* templates;			// templates that interface inherits
	tree_t* spec;				// spec about interface
};

/**
 * @brief : tree node about attribute
 */
struct tree_attribute {
	struct tree_common common;
	const char* name;
	tree_t* arraydef;		// array about the same attribute
	tree_t* templates;		// templates that attribute inherits
	tree_t* spec;			// spec about attribute
};

/**
 * @brief : tree node about event,
 * includes templates that inherited and
 * spec about event
 */
struct tree_event {
	struct tree_common common;
	const char* name;
	tree_t* templates;
	tree_t* spec;
};

/**
 * @brief : tree node about implement,
 * includes templates that inherited and
 * spec about implement
 */
struct tree_implement {
	struct tree_common common;
	const char* name;
	tree_t* templates;
	tree_t* spec;
};

/**
 * @brief : tree node about identifier
 * include the pointer and length about the identifier
 */
struct tree_ident {
	struct tree_common common;
	const char* str;
	int len;
	type_t type;
};

/**
 * @brief : tree node about method
 */
struct tree_method {
	struct tree_common common;
	const char* name;
	int is_extern;				// method is extern?
	int is_default;				// default method
	tree_t* params;				// method parameters
	tree_t* block;				// the body of method
};

/**
 * @brief : tree node about parameters
 */
struct tree_params {
	struct tree_common common;
	int have_in_param;			// have incoming parameters
	int have_ret_param;			// have returning parameters
	int in_argc;				// number of incoming parameters
	int ret_argc;				// number of returning parameters
	tree_t* in_params;			// the list of incoming parameters
	tree_t* ret_params;			// the list of returning parameters
};

/**
 * @brief : tree node about template
 * including the name and spec about template
 */
struct tree_template {
	struct tree_common common;
	const char* name;
	tree_t* spec;
};

/**
 * @brief : tree node about loggroup
 * only need name about it.
 */
struct tree_loggroup {
	struct tree_common common;
	const char* name;
};

/**
 * @brief : tree node about assign
 */
struct tree_assign {
	struct tree_common common;
	int is_constant;			// the constant of dml
	int is_const;				// the const of c
	int is_static;				// static assignment
	int is_local;				// local assignment
	tree_t* decl;				// the decl about assignment like int, char struct
	tree_t* expr;				// the left assignment element
	tree_t* expr_right;			// the right assignment element
};

/**
 * @brief : tree node about cdecl,like int a;
 */
struct tree_cdecl {
	struct tree_common common;
	int is_data;			// the decl is data?
	int is_extern;			// the decl is extern?
	int is_typedef;			// the decl is typedef?
	int is_const;			// the decl is const?
	int is_point;			// the decl is pointer(*)?
	int is_point_access;	// the decl is point access(->)?
	int is_vect;			// the decl is vect?
	tree_t* basetype;			// the basetype about cdecl, like int, char
	tree_t* decl;				// the body about decl
};

/**
 * @brief : the cdecl with brackets
 */
struct tree_cdecl_brack {
	struct tree_common common;
	int is_list;
	tree_t* cdecl;				// the cdecl out brackets
	tree_t* decl_list;			// the cdecl in brackets
};

/**
 * @brief : tree node about head
 */
struct tree_head {
	struct tree_common common;
	const char* str;
	tree_t* head;
};

/**
 * @brief : tree node about block
 * the block is defined about {}
 */
struct tree_block {
	struct tree_common common;
	tree_t* statement;			// the content in block({})
	symtab_t table;
};

#if 0
struct tree_param_cdecl {
	struct tree_common common;
	tree_t* decl;
};
#endif

/**
 * @brief : tree node about dml object array
 */
struct tree_array {
	struct tree_common common;
	int is_fix;					// the array is fix or range
	tree_t* ident;				// the identifier of range
	tree_t* expr;				// the expression about array
	tree_t* expr_end;			// the end expression about array
	tree_t* decl;				// the cdecl about array,like int register;
};

/**
 * @brief : tree node about bit slicing expression
 */
struct tree_bit_slic {
	struct tree_common common;
	tree_t* expr;				// the expression about bit
	tree_t* bit;				// the start expr about bit
	tree_t* bit_end;			// the end expr about bit
	tree_t* endian;				// the bitorder about bit
};

/**
 * @brief : tree node about if ... else ...
 */
struct tree_if_else {
	struct tree_common common;
	tree_t* cond;				// the condition about if
	tree_t* if_block;			// the block about if
	tree_t* else_block;			// the block about else
};

/**
 * @brief : tree node about do..while... or while...
 */
struct tree_do_while {
	struct tree_common common;
	int have_do;			// is do...while?
	tree_t* cond;			// condition about while
	tree_t* block;			// the body about while
};

/**
 * @brief : tree node about for(.., ..,..){};
 */
struct tree_for {
	struct tree_common common;
	tree_t* init;				// init value
	tree_t* cond;				// condition about for
	tree_t* update;				// update value when body run once
	tree_t* block;				// block about for
};

/**
 * @brief : tree node about switch
 */
struct tree_switch {
	struct tree_common common;
	tree_t* cond;				// condition aoubt switch
	tree_t* block;				// block about switch
};

/**
 * @brief : tree node about delete
 * only including the expression about delete
 */
struct tree_delete {
	struct tree_common common;
	tree_t* expr;
};

/**
 * @brief : tree node about try...catch...
 */
struct tree_try_catch {
	struct tree_common common;
	tree_t* try_block;			// block about try
	tree_t* catch_block;		// block about catch
};

/**
 * @brief : tree node about after...call...
 */
struct tree_after_call {
	struct tree_common common;
	tree_t* cond;				// condition about after
	tree_t* call_expr;			// the expression about call
};

/**
 * @brief : tree node about call or inline
 */
struct tree_call_inline {
	struct tree_common common;
	tree_t* expr;				//  expression about call or inline
	tree_t* ret_args;			// the return args
};

/**
 * @brief : tree node about assert
 * only including the assert expression
 */
struct tree_assert {
	struct tree_common common;
	tree_t* expr;
};

/**
 * @brief : tree node about log statement
 */
struct tree_log {
	struct tree_common common;
	const char* log_type;				// log type: error,info,etc.
	tree_t* level;				// log level:from 1 to 4
	tree_t* group;				// the group the log belonged
	const char* format;				// the format aobut log information
	int argc;					// number about information
	tree_t* args;				// log information
};

/**
 * @brief : tree node about select statement
 */
struct tree_select {
	struct tree_common common;
	tree_t* ident;				// the indentifier about select
	tree_t* in_expr;			// expression about in
	tree_t* cond;				// condition expression about where
	tree_t* where_block;		// block about where
	tree_t* else_block;			// block about else
};

/**
 * @brief : tree node about foreach statement
 */
struct tree_foreach {
	struct tree_common common;
	tree_t* ident;				// indentifier about foreach
	tree_t* in_expr;			// expression about in
	tree_t* block;				// block about block
};

/**
 * @brief : tree node about case statement
 */
struct tree_case {
	struct tree_common common;
	tree_t* expr;				// expr about case
	tree_t* block;				// block about case
};

/**
 * @brief : tree node about default
 * only including the block
 */
struct tree_default {
	struct tree_common common;
	tree_t* block;
};

/**
 * @brief : tree node about goto statement
 * only including the label to goto
 */
struct tree_goto {
	struct tree_common common;
	tree_t* label;
};

/**
 * @brief : tree node about error
 * only including the string about error
 */
struct tree_error {
	struct tree_common common;
	const char* str;
};

/**
 * @brief : tree node about struct
 * only including the block
 */
struct tree_struct {
	struct tree_common common;
	tree_t* ident;
	tree_t* block;
};

/**
 * @brief : tree node about layout
 * including the name and block
 */
struct tree_layout {
	struct tree_common common;
	const char* name;
	tree_t* block;
};

/**
 * @brief : tree node about typeof
 * only including the expression about typeof
 */
struct tree_typeof {
	struct tree_common common;
	tree_t* expr;
};

/**
 * @brief : tree node about bitfields
 * including the name and block
 */
struct tree_bitfields {
	struct tree_common common;
	const char* name;
	tree_t* block;
};

/**
 * @brief : tree node about bitfields describution
 */
struct tree_bitfields_dec {
	struct tree_common common;
	tree_t* decl;				// the decl about bitfields
	tree_t* start;				// bitfields start
	tree_t* end;				// bitfields end
};

/**
 * @brief : the expression assignment
 * like: expr = expr
 */
struct tree_expr_assign {
	struct tree_common common;
	const char* assign_symbol;		// the assign symbol like: = /= %= ...
	operator_type_t type;
	tree_t* left;				// the left expression about assignment
	tree_t* right;				// the right expression about assignment
};

/**
 * @brief :tree node about new
 * including the type and count
 */
struct tree_new {
	struct tree_common common;
	tree_t* type;
	tree_t* count;
};

/**
 * @brief : tree node about unary
 * including the operator and expression
 */
struct tree_unary {
	struct tree_common common;
	const char* operat;
	operator_type_t type;
	tree_t* expr;
};

/**
 * @brief : tree node about binary
 */
struct tree_binary {
	struct tree_common common;
	const char* operat;			// the operator about binary like: / &= += etc.
	operator_type_t type;
	type_t final_type;
	tree_t* left;				// the left expression about binary
	tree_t* right;				// the right expression about binary
};

/**
 * @brief : tree node about ternary
 */
struct tree_ternary {
	struct tree_common common;
	tree_t* cond;				// condition about ternary
	tree_t* expr_true;			// the expression about condition true
	tree_t* expr_false;			// expression about condition false
};

/**
 * @brief :tree node about cast
 * including expression and type
 */
struct tree_cast {
	struct tree_common common;
	tree_t* expr;
	tree_t* ctype;
	void* decl;
};

/**
 * @brief : tree node about sizeof
 * only including the expression
 */
struct tree_sizeof {
	struct tree_common common;
	tree_t* expr;
};

/**
 * @brief : tree node about sizeoftype
 * only including the type like tyedef type
 */
struct tree_sizeoftype {
	struct tree_common common;
	tree_t* typeoparg;
};

/**
 * @brief : tree node about type decl
 * the decl have two class: one is only decl
 * and another decl with brackets
 */
struct tree_typeoparg {
	struct tree_common common;
	tree_t* ctypedecl;
	tree_t* ctypedecl_brack;
};

/**
 * @brief : tree node about c type decl
 */
struct tree_ctypedecl {
	struct tree_common common;
	tree_t* const_opt;			// const operator
	tree_t* basetype;			// the basetype about decl
	tree_t* ctypedecl_ptr;		// c type decl body
};

/**
 * @brief : tree node about c type decl body
 */
struct tree_ctypedecl_ptr {
	struct tree_common common;
	tree_t* stars;				// the stars(*) about decl
	tree_t* array;				// the array about decl
};

/**
 * @brief : tree node about stars
 */
struct tree_stars {
	struct tree_common common;
	int is_const;				// is const?
	tree_t* stars;				// another stars
};

/**
 * @brief : tree node about expression with brackets
 */
struct tree_expr_brack {
	struct tree_common common;
	tree_t* expr;				// the expression out brackets
	tree_t* expr_in_brack;		// expression in brackets
};

/**
 * @brief : tree node about int constant
 */
struct tree_int_cst {
	struct tree_common common;
	const char* int_str;				// the int value formed with string
	long long value;					// the int value that tanslate from string,
										// attention: some parameter's value is int64
	int out_64bit;						// pay attention: some parameter's value is out of int64
										// we should store it with string;
};

/**
 * @brief : tree node about float constant
 */
struct tree_float_cst {
	struct tree_common common;
	const char* float_str;				// the int value formed with string
	double value;					// the int value that tanslate from string,
};

/**
 * @brief : tree node about quote($)
 * only including the indentifier
 */
struct tree_quote {
	struct tree_common common;
	tree_t* ident;
};

/**
 * @brief : tree node about component(.)
 */
struct tree_component {
	struct tree_common common;
	const char* comp;					// the component(.)
	operator_type_t type;
	tree_t* expr;				// expression about component
	tree_t* ident;				// indentifier about component
};

/**
 * @brief : tree node about local
 */
struct tree_local {
	struct tree_common common;
	int is_static;				// is static?
	tree_t* local_keyword;		// local keyword(local|auto)
	tree_t* cdecl;				// the cdecl about local
	tree_t* expr;				// expression about local
};

/**
 * @brief : tree node about local keyword
 * only including the keyword: local or auto
 */
struct tree_local_keyword {
	struct tree_common common;
	const char* name;
};

/**
 * @brief : tree node about group
 */
struct tree_group {
	struct tree_common common;
	const char* name;					// name about identifier
	int is_array;				// the group is array
	tree_t* templates;			// templates that inherits
	tree_t* spec;				// the spec about group
	tree_t* array;				// the array that about group
};

/**
 * @brief : tree node about port
 */
struct tree_port {
	struct tree_common common;
	const char* name;					// port name
	tree_t* templates;			// templates that inherited
	tree_t* spec;				// the spec about port
};

/* Define the overall contents of a tree node.
   It may be any of the structures declared above
   for various types of node.  */
union tree_node
{
	struct tree_common  common;
	struct tree_dml dml;
	struct tree_device device;
	struct tree_bitorder bitorder;
	struct tree_bank bank;
	struct tree_object_spec spec;
	struct tree_param param;
	struct tree_paramspec paramspec;
	struct tree_field  field;
	struct tree_data data;
	struct tree_connect connect;
	struct tree_interface interface;
	struct tree_attribute attribute;
	struct tree_event event;
	struct tree_implement implement;
	struct tree_method method;
	struct tree_params params;
	struct tree_template temp;
	struct tree_loggroup loggroup;
	struct tree_assign assign;
	struct tree_expr_assign expr_assign;
	struct tree_cdecl cdecl;
	struct tree_cdecl_brack cdecl_brack;
	struct tree_head head;
	struct tree_block block;
	struct tree_array array;
	struct tree_if_else if_else;
	struct tree_do_while do_while;
	struct tree_for for_tree;
	struct tree_switch switch_tree;
	struct tree_delete delete_tree;
	struct tree_try_catch try_catch;
	struct tree_after_call after_call;
	struct tree_call_inline call_inline;
	struct tree_assert assert_tree;
	struct tree_log log;
	struct tree_select select;
	struct tree_foreach foreach;
	struct tree_default default_tree;
	struct tree_goto goto_tree;
	struct tree_error error;
	struct tree_struct struct_tree;
	struct tree_typeof typeof_tree;
	struct tree_layout layout;
	struct tree_bitfields bitfields;
	struct tree_bitfields_dec bitfields_dec;
	struct tree_unary unary;
	struct tree_binary binary;
	struct tree_ternary ternary;
	struct tree_cast cast;
	struct tree_sizeof sizeof_tree;
	struct tree_sizeoftype sizeoftype;
	struct tree_expr_brack expr_brack;
	struct tree_case case_tree;
	struct tree_int_cst  int_cst;
	struct tree_float_cst float_cst;
	struct tree_string string;
	struct tree_register reg;
	struct tree_quote quote;
	struct tree_ident ident;
	struct tree_stars stars;
	struct tree_component component;
	struct tree_typeoparg typeoparg;
	struct tree_ctypedecl ctypedecl;
	struct tree_ctypedecl_ptr ctypedecl_ptr;
	struct tree_new new_tree;
	struct tree_local local_tree;
	struct tree_bit_slic bit_slic;
	struct tree_local_keyword local_keyword;
	struct tree_group group;
	struct tree_port port;
 };

#endif /* __TREE_H__ */
