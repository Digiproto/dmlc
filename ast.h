#ifndef __AST_H__
#define __AST_H__

#include "tree.h"
#include "symbol.h"
#include "debug_color.h"
#include "symbol-common.h"

tree_t* get_ast (const char *filename);
void* gdml_zmalloc(int size);
char** get_templates(char** templates, tree_t* head, int num);
int get_list_num (tree_t* root);
symtab_t get_obj_block_table(tree_t* spec);
arraydef_attr_t* get_arraydef(tree_t* node, symtab_t table);
tree_t* create_node (const char *name, int type, int size, YYLTYPE* location);
tree_t *create_string_node(const char *ptr);
void add_child (tree_t* parent, tree_t* child);
tree_t* create_node_list (tree_t* root, tree_t* new_node);
obj_spec_t* get_obj_spec(obj_spec_t* obj_spec, tree_t* node);
char* get_obj_desc(obj_spec_t* spec);
int get_size(tree_t* node, symtab_t table);
int get_offset(tree_t* node, symtab_t table);
method_params_t* get_method_params(tree_t* node, symtab_t table);
void get_object_template_table(symtab_t table, tree_t* node);
tree_t* c_keyword_node (const char* name, YYLTYPE* location);
tree_t* dml_keyword_node(const char* name, YYLTYPE* location);
long long strtoi (char *str);
void print_ast (tree_t* root);
int get_param_num(tree_t* node);
int get_list_num (tree_t* root);
void parse_undef_node(symtab_t table);
int charge_standard_parameter(symtab_t table, parameter_attr_t* attr);
object_attr_t* create_object(symtab_t table, const char* node_name, const char* symbol_name, int type, int node_size, int attr_size, YYLTYPE* location);
tree_t* create_template_list(tree_t* head, tree_t* templates);
tree_t* get_obj_default_param(tree_t* head, tree_t* new, const char* name);

void parse_object(tree_t *node, symtab_t table);
void parse_device(tree_t* node, symtab_t table);
void parse_bitorder(tree_t* node, symtab_t table);
void parse_parameter(tree_t* node, symtab_t table);
void parse_obj_if_else(tree_t* node, symtab_t table);
void parse_method(tree_t* node, symtab_t table);
void parse_method_block(tree_t* node);
void parse_loggroup(tree_t* node, symtab_t table);
void parse_constant(tree_t* node, symtab_t table);
void parse_extern_decl(tree_t* node, symtab_t table);
void parse_typedef(tree_t* node, symtab_t table);
void parse_struct_top(tree_t* node, symtab_t table);
void parse_data(tree_t* node, symtab_t table);
void parse_bank(tree_t* node, symtab_t table);
void parse_register(tree_t* node, symtab_t table);
void parse_register_attr(tree_t* node, symtab_t table);
void parse_field(tree_t* node, symtab_t table);
void parse_field_attr(tree_t* node, symtab_t table);
void parse_connect(tree_t* node, symtab_t table);
void parse_connect_attr(tree_t* node, symtab_t table);
void parse_interface(tree_t* node, symtab_t table);
void parse_attribute(tree_t* node, symtab_t table);
void parse_attribute_attr(tree_t* node, symtab_t table);
void parse_event(tree_t* node, symtab_t table);
void parse_group(tree_t* node, symtab_t table);
void parse_group_attr(tree_t* node, symtab_t table);
void parse_port(tree_t* node, symtab_t table);
void parse_port_attr(tree_t* node, symtab_t table);
void parse_implement(tree_t* node, symtab_t table);
void parse_expr(tree_t* node, symtab_t table);
void parse_if_else(tree_t* node, symtab_t table);
void parse_local(tree_t* node, symtab_t table);
void parse_do_while(tree_t* node, symtab_t table);
void parse_for(tree_t* node, symtab_t table);
void parse_switch(tree_t* node, symtab_t table);
void parse_delete(tree_t* node, symtab_t table);
void parse_try_catch(tree_t* node, symtab_t table);
void parse_after_call(tree_t* node, symtab_t table);
void parse_call(tree_t* node, symtab_t table);
void parse_inline(tree_t* node, symtab_t table);
void parse_assert(tree_t* node, symtab_t table);
void parse_log(tree_t* node, symtab_t table);
void parse_select(tree_t* node, symtab_t table);
void parse_foreach(tree_t* node, symtab_t table);
void parse_label(tree_t* node, symtab_t table);
void parse_case(tree_t* node, symtab_t table);
void parse_default(tree_t* node, symtab_t table);
void parse_goto(tree_t* node, symtab_t table);
void parse_undef_templates(symtab_t table);
void check_template_parsed(const char* name);
void parse_undef_template(const char* name);
void parse_unparsed_obj(tree_t* node, symtab_t table);

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
void print_footer(tree_t* node, int pos);
void print_struct(tree_t* node, int pos);
void print_undefined(tree_t* node, int pos);
void print_dot(tree_t* node, int pos);
void print_foreach(tree_t* node, int pos);
void print_label(tree_t* node, int pos);
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
void print_layout(tree_t* node, int pos);
void print_sizeof(tree_t* node, int pos);
void print_sizeoftype (tree_t* node, int pos);
void print_typeoparg(tree_t* node, int pos);
void print_for(tree_t* node, int pos);
void print_switch(tree_t* node, int pos);
void print_delete(tree_t* node, int pos);
void print_default(tree_t* node, int pos);
void print_new(tree_t* node, int pos);
void print_bitfields_decls(tree_t* node, int pos);
void print_bitfields(tree_t* node, int pos);
void print_goto(tree_t* node, int pos);
void print_case(tree_t* node, int pos);
void print_undef_templates(symtab_t table);

#define insert_no_defined_expr(table, node, expr) \
	do { \
		if (expr->no_defined) { \
			undef_var_insert(table, node); \
			return; \
		} \
	} while(0)

//#define DEBUG_AST_TREE

#ifdef DEBUG_AST_TREE
#define DEBUG_ADD_TEMPLATE debug_brown
#define DEBUG_TEMPLATES debug_cyan
#define DEBUG_AST printf
#else
#define DEBUG_ADD_TEMPLATE
#define DEBUG_TEMPLATES
#define DEBUG_AST
#endif


#endif
