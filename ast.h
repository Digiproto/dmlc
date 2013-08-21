#ifndef __AST_H__
#define __AST_H__

#include "tree.h"
#include "symbol.h"
#include "debug_color.h"

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

tree_t* get_ast (const char *filename);
void* gdml_zmalloc(int size);
char** get_templates(tree_t* head);
int get_list_num (tree_t* root);
symtab_t get_obj_block_table(tree_t* spec);
arraydef_attr_t* get_arraydef(tree_t** node, symtab_t table);
//tree_t* create_node (char *name, int type, int size);
tree_t* create_node (char *name, int type, int size, YYLTYPE* location);
void add_child (tree_t* parent, tree_t* child);
tree_t* create_node_list (tree_t* root, tree_t* new_node);
char* get_obj_desc(tree_t* spec);
int get_size(tree_t** node, symtab_t table);
int get_offset(tree_t** node, symtab_t table);
method_params_t* get_method_params(tree_t* node, symtab_t table);
void get_object_template_table(symtab_t table, tree_t* node);
tree_t* c_keyword_node (const char* name);
tree_t* dml_keyword_node(const char* name, YYLTYPE* location);
long long strtoi (char *str);
void print_ast (tree_t* root);
int get_param_num(tree_t* node);
int get_list_num (tree_t* root);
void parse_undef_node(symtab_t table);
int charge_standard_parameter(symtab_t table, parameter_attr_t* attr);

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
