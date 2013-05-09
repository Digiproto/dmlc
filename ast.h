#ifndef __AST_H__
#define __AST_H__

#include "tree.h"

typedef struct node
{
	int type;
	char *name;
	struct node *sibling;
	struct node *child;
} node_t;

typedef struct level
{
	int depth;

} level_t;

void print_dml(tree_t* node, int pos);
void print_device(tree_t* node, int pos);
void print_bitorder(tree_t* node, int pos);
void print_parameter(tree_t* node, int pos);
void print_paramspec(tree_t* node, int pos);
void print_string(tree_t* node, int pos);
void print_bank(tree_t* node, int pos);
void print_template(tree_t* node, int pos);
void print_method (tree_t* node, int pos);
void print_object_spec(tree_t* node, int pos);
void print_object_desc (tree_t* node, int pos);
void print_obj_block(tree_t* node, int pos);
void print_interger(tree_t* node, int pos);
void print_expr_brack(tree_t* node, int pos);
void print_quote(tree_t* node, int pos);
void print_eq_op(tree_t* node, int pos);
void print_ternary(tree_t* node, int pos);
void print_method_params(tree_t* node, int pos);
void print_cdecl(tree_t* node, int pos);
void print_ident(tree_t* node, int pos);
void print_loggroup(tree_t* node, int pos);
void print_cdecl_brak(tree_t* node, int pos);
void print_ellipsis(tree_t* node, int pos);
void print_assign(tree_t* node, int pos);
void print_header(tree_t* node, int pos);
void print_struct(tree_t* node, int pos);
void print_undefined(tree_t* node, int pos);
void print_dot(tree_t* node, int pos);
void print_foreach(tree_t* node, int pos);
void print_call_inline(tree_t* node, int pos);
void print_non_op(tree_t* node, int pos);
void print_defined(tree_t* node, int pos);
void print_if_else(tree_t* node, int pos);
void print_expr_assign(tree_t* node, int pos);
void print_throw(tree_t* node, int pos);
void print_unary(tree_t* node, int pos);
void print_binary(tree_t* node, int pos);
void print_try_catch(tree_t* node, int pos);
void print_log(tree_t* node, int pos);
void print_local_keyword(tree_t* node, int pos);
void print_local(tree_t* node, int pos);
void print_error(tree_t* node, int pos);
void print_return(tree_t* node, int pos);
void print_break(tree_t* node, int pos);
void print_continue(tree_t* node, int pos);
void print_component(tree_t* node, int pos);
void print_bit_slic(tree_t* noe, int pos);
void print_typeof(tree_t* node, int pos);
void print_assert(tree_t* node, int pos);
void print_float_literal(tree_t* node, int pos);
void print_cast(tree_t* node, int pos);
void print_ctypedecl(tree_t* node, int pos);
void print_ctypedecl_ptr(tree_t* node, int pos);
void print_ctypedecl_simple(tree_t* node, int pos);
void print_stars(tree_t* node, int pos);
void print_register(tree_t* node, int pos);
void print_arraydef(tree_t* node, int pos);
void print_port(tree_t* node, int pos);
void print_implement(tree_t* node, int pos);
void print_connect(tree_t* node, int pos);
void print_interface(tree_t* node, int pos);
void print_attribute(tree_t* node, int pos);
void print_event(tree_t* node, int pos);
void print_group(tree_t* node, int pos);
void print_port(tree_t* node, int pos);
void print_select(tree_t* node, int pos);
void print_field(tree_t* node, int pos);
void print_array(tree_t* node, int pos);
void print_after_call(tree_t* node, int pos);
void print_while(tree_t* node, int pos);
void print_do_while(tree_t* node, int pos);

#endif
