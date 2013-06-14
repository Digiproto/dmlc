%{
#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "symbol.h"
#include "stack.h"
#include "ast.h"
#include "Parser.h"
#define YYDEBUG 1
const char* dir = "/opt/virtutech/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0/";

//#define PARSE_DEBUG
#ifdef PARSE_DEBUG
#define DBG debug_black
//#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

typedef struct YYLTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} YYLTYPE;
#include "Lexer.h"

extern int  yylex(YYSTYPE *yylval_param, yyscan_t yyscanner);
void yyerror(yyscan_t* scanner, void* v, char *s);
extern char* builtin_filename;
symtab_t root_table;
stack_t* table_stack;
int object_spec_type = -1;
static symtab_t current_table = NULL;
static symtab_t prefix_table = NULL;
static long int current_table_num = 0;
tree_t* current_object_node = NULL;
%}

%output  "Parser.c"
%defines "Parser.h"

%define api.pure

%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { tree_t** root_ptr }

%union  {
	int ival;
	char* sval;
	node_t* nodeval;
	int itype;
	tree_t* tree_type;
	//enum tree_code code;
	location_t location;
}

%token DML IDENTIFIER INTEGER_LITERAL FLOAT_LITERAL STRING_LITERAL SIZEOF
%token INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME REG_OFFSET

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token METHOD_RETURN RANGE_SIGN

%token PARAMETER BANK FIELD DATA CONNECT INTERFACE ATTRIBUTE EVENT GROUP
%token IN TEMPLATE HEADER FOOTER LOGGROUP IMPORT SIZE LAYOUT BITFIELDS
%token USING PORT PUBLIC PROTECTED PRIVATE STRICT THIS SELECT IS IMPLEMENT VECT WHERE
%token DEVICE DEFINED AFTER ASSERT BITORDER CATCH TRY THROW CLASS LOG METHOD
%token CALL CAST CONSTANT ERROR FOREACH INLINE LOCAL NAMESPACE 
%token RESTRICT SIZEOFTYPE TYPEOF UNDEFINED VIRTUAL NEW DELETE

%start begin_unit

%type	<tree_type> DEVICE
%type	<tree_type> IMPORT
%type	<tree_type> IDENTIFIER
%type	<tree_type> CONSTANT
%type	<tree_type> SIGNED
%type	<tree_type> HEADER
%type	<tree_type> objident
%type	<tree_type> maybe_objident
%type	<tree_type> ident
%type	<sval> STRING_LITERAL
%type	<sval> INTEGER_LITERAL
%type	<sval> FLOAT_LITERAL
%type	<sval> INT
%type	<tree_type> typeident
%type	<tree_type> local_keyword
%type	<tree_type> const_opt
%type	<tree_type> dml
%type	<tree_type> syntax_modifiers
%type	<tree_type> device_statements
%type	<tree_type> device_statement
%type	<tree_type> statement
%type	<tree_type> if_statment
%type	<tree_type> syntax_modifier
%type	<tree_type> toplevel
%type	<tree_type> import
%type	<tree_type> parameter
%type	<tree_type> offsetspec
%type	<tree_type> object_desc
%type	<tree_type> object_statement
%type	<tree_type> method
%type	<tree_type> sizespec
%type	<tree_type> object
%type	<tree_type> object_spec
%type	<tree_type> object_statements
%type	<tree_type> istemplate_stmt
%type	<tree_type> istemplate
%type	<tree_type> compound_statement
%type	<tree_type> objident_list
%type	<tree_type> arraydef
%type	<tree_type> bitrange
%type	<tree_type> paramspec
%type	<tree_type> basetype
%type	<tree_type> expression
%type	<tree_type> comma_expression
%type	<tree_type> comma_expression_opt
%type	<tree_type> expression_list
%type	<tree_type> method_params
%type	<tree_type> cdecl_or_ident_list
%type	<tree_type> cdecl_or_ident_list2
%type	<tree_type> cdecl
%type	<tree_type> cdecl2
%type	<tree_type> cdecl3
%type	<tree_type> cdecl_list
%type	<tree_type> cdecl_list2
%type	<tree_type> local
%type	<tree_type> typeof
%type	<tree_type> cdecl_or_ident
%type	<tree_type> statement_list
%type	<tree_type> log_args
%type	<tree_type> endianflag
%type	<tree_type> struct
%type	<tree_type> stars
%type	<tree_type> struct_decls
%type	<tree_type> returnargs
%type	<tree_type> ctypedecl_ptr
%type	<tree_type> ctypedecl
%type	<tree_type> ctypedecl_array
%type	<tree_type> ctypedecl_simple
%type	<tree_type> object_if
%type	<tree_type> object_if_statement
%type	<tree_type> layout
%type	<tree_type> layout_decls
%type	<tree_type> bitfields
%type	<tree_type> bitfields_decls
%type	<tree_type> typeoparg
%type	<tree_type> expression_opt

/* Add precedence rules to solve dangling else s/r conflict */
%nonassoc IF
%nonassoc ELSE

%right '=' RIGHT_ASSIGN LEFT_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%right '?' ':'
%left OR_OP
%left AND_OP
%left '|'
%left '^'
%left '&'
%left EQ_OP NE_OP
%left '>' '<' GE_OP LE_OP
%left LEFT_OP RIGHT_OP
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' '#' '$' DEFINED INC_OP DEC_OP
%left HYPERUNARY
%left POINTSAT '.' '(' '['

%%
begin_unit
	: DML FLOAT_LITERAL';' dml {
		dml_attr_t* attr = (dml_attr_t*)gdml_zmalloc(sizeof(dml_attr_t));
		attr->version = $2;
		//symbol_insert("dml", DML_TYPE, attr);

		if(*root_ptr != NULL) {
			/* something wrong */
			printf("root of ast already exists\n");
			exit(-1);
		}
		*root_ptr = (tree_t*)create_node("dml", DML_TYPE, sizeof(struct tree_dml));
		printf("root_ptr: 0x%x, *root_ptr: 0x%x\n", root_ptr, *root_ptr);
		(*root_ptr)->dml.version = $2;
		(*root_ptr)->dml.common.print_node = print_dml;
		if($4 != NULL)
			add_child(*root_ptr, $4);
	}
	;

dml
	: DEVICE objident ';' {
		root_table = symtab_create(DEVICE_TYPE);
		current_table = root_table;
		current_table->table_num = ++current_table_num;
		table_stack = initstack();
		push(table_stack, current_table);
		printf("current_table_num: %d\n", current_table_num);
		device_attr_t* attr = (device_attr_t*)malloc(sizeof(device_attr_t));
		attr->name = $2->ident.str;
		if (symbol_insert(root_table, $2->ident.str, DEVICE_TYPE, attr) == -1) {
			fprintf(stderr, "redefined\n");
		}
		/* FIXME: have any other device */

		tree_t* node = (tree_t*)create_node("device", DEVICE_TYPE, sizeof(struct tree_device));
		node->device.name = $2->ident.str;
		node->device.common.print_node = print_device;
		attr->common.node = node;
		node->common.attr = attr;

		$<tree_type>$ = node;
	}
	syntax_modifiers device_statements {
		tree_t* import_ast = NULL;
		if(builtin_filename != NULL) {
			import_ast = (tree_t*)get_ast(builtin_filename);
			printf("builtin_filename: %s\n", builtin_filename);
			if(import_ast->common.child != NULL) {
				create_node_list($<tree_type>4, import_ast->common.child);
				//create_node_list(node, import_ast);
			}
		}

		if($5 != NULL)	{
			printf("\ndevice list: name: %s\n\n", $5->common.name);
			create_node_list($<tree_type>4, $5);
		}
		if($6 != NULL) {
			printf("\ndevice list: name: %s\n\n", $6->common.name);
			create_node_list($<tree_type>4, $6);
		}

		DBG("Device type is %s\n", $2->ident.str);
		$$ = $<tree_type>4;
	}
	| syntax_modifiers device_statements {
		if($1 == NULL && $2 != NULL) {
			$$ = $2;
		}
		else if($1 != NULL && $2 == NULL) {
			$$ = $1;
		}
		else if($1 != NULL && $2 != NULL){
			create_node_list($1, $2);
			$$ = $1;
		}
		else {
			printf("Line = %d, maybe something Wrong\n", __LINE__);
		}
	}
	;

syntax_modifiers
	: {
		$$ = NULL;
	}
	| syntax_modifiers syntax_modifier {
		DBG("In syntax_modifiers\n");
		if($1 == NULL && $2 != NULL) {
			$$ = $2;
		}
		else if($1 != NULL && $2 != NULL) {
			$$ = (tree_t*)create_node_list($1, $2);
		}
		else {
			$$ = NULL;
		}
	}
	;

syntax_modifier
	: BITORDER ident ';' {
		DBG("In BITORDER: %s\n", $2->ident.str);
		bitorder_attr_t*  attr = (bitorder_attr_t*)gdml_zmalloc(sizeof(bitorder_attr_t));
		attr->endian = $2->ident.str;
		attr->common.table_num = current_table->table_num;
		if (symbol_insert(current_table, $2->ident.str, BITORDER_TYPE, attr) == -1) {
			fprintf(stderr, "redefined\n");
		}

		tree_t* node = (tree_t*)create_node("bitorder", BITORDER_TYPE, sizeof(struct tree_bitorder));
		node->bitorder.endian = $2->ident.str;
		node->common.print_node = print_bitorder;
		node->common.attr = attr;
		attr->common.node = node;
		$$ = node;
	}
	;

device_statements
	: {
		$$ = NULL;
	} 
	| device_statements device_statement {
		DBG("In device_statements\n");
		if($1 == NULL && $2 != NULL) {
			$$ = $2;
		}
		else if($1 != NULL && $2 != NULL){
			DBG("create_node_list, $1=0x%x, $2=0x%x. | device_statements device_statement\n",
					(unsigned)$1, (unsigned)$2);
			$$ = (tree_t*)create_node_list($1, $2);
		}
		else {
			printf("maybe something wrong in device_statements\n");
		}
	}
	;

device_statement
	: object_statement {
		DBG("object_statement In device_statement\n");
		$$ = $1;
	}
	| toplevel {
		DBG("toplevel In device_statement\n");
		$$ = $1;
	}
	| import {
		DBG("import In device_statement\n");
		/* FIXME: there maybe some problems */
		$$ = $1;
	}
	;

object
	: BANK maybe_objident istemplate {
		if ($2 == NULL) {
			fprintf(stderr, "There must be objident.\n");
			exit(-1);
		}
		DBG("BANK is %s\n", $2->ident.str);

		bank_attr_t* attr = (bank_attr_t*)gdml_zmalloc(sizeof(bank_attr_t));
		attr->name = $2->ident.str;
		attr->common.table_num = current_table->table_num;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);

		tree_t* node = (tree_t*)create_node("bank", BANK_TYPE, sizeof(struct tree_bank));
		node->bank.name = $2->ident.str;
		node->bank.templates = $3;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;

		symbol_insert(current_table, $2->ident.str, BANK_TYPE, attr);
		object_spec_type = BANK_TYPE;

		$<tree_type>$ = node;
	}
	object_spec {
		$<tree_type>4->bank.spec = $5;
		$<tree_type>4->common.print_node = print_bank;
		bank_attr_t* attr = (bank_attr_t*)($<tree_type>4->common.attr);
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		object_spec_type = -1;
		$$ = $<tree_type>4;
	}
	| REGISTER objident sizespec offsetspec istemplate {
		DBG("register is %s\n", $2->ident.str);
		register_attr_t* attr = (register_attr_t*)gdml_zmalloc(sizeof(register_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 0;
		attr->size = get_size($3);
		attr->offset = get_offset($4);
		attr->templates = get_templates($3);
		attr->templates_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, REGISTER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("register", REGISTER_TYPE, sizeof(struct tree_register));
		node->reg.name = $2->ident.str;
		node->reg.sizespec = $3;
		node->reg.offset = $4;
		node->reg.templates = $5;
		node->common.print_node = print_register;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		object_spec_type = REGISTER_TYPE;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>6;
		register_attr_t* attr = (register_attr_t*)(node->common.attr);
		attr->desc = get_obj_desc($7);
		attr->table = get_obj_block_table($7);
		node->reg.spec = $7;
		object_spec_type = -1;
		$$ = node;
	}
	| REGISTER objident '[' arraydef ']' sizespec offsetspec istemplate {
		DBG("Register is %s\n", $2->ident.str);
		register_attr_t* attr = (register_attr_t*)gdml_zmalloc(sizeof(register_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 1;
		attr->size = get_size($6);
		attr->offset = get_offset($7);
		attr->arraydef = get_arraydef($4);
		attr->templates = get_templates($8);
		attr->templates_num = get_list_num($8);
		symbol_insert(current_table, $2->ident.str, REGISTER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("register", REGISTER_TYPE, sizeof(struct tree_register));
		node->reg.name = $2->ident.str;
		node->reg.array = $4;
		node->reg.sizespec = $6;
		node->reg.offset = $7;
		node->reg.templates = $8;
		node->common.print_node = print_register;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		object_spec_type = REGISTER_TYPE;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>9;
		register_attr_t* attr = (register_attr_t*)(node->common.attr);
		node->reg.spec = $10;
		attr->desc = get_obj_desc($10);
		attr->table = get_obj_block_table($10);
		object_spec_type = -1;
		$$ = node;
	}
	| FIELD objident bitrange istemplate {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		field_attr_t* attr = (field_attr_t*)gdml_zmalloc(sizeof(field_attr_t));
		attr->name = $2->ident.str;
		attr->is_range = 1;
		attr->templates = get_templates($4);
		attr->template_num = get_list_num($4);
		/* FIXME: should parsing bitrange */
		attr->bitrange = $3->common.attr;
		symbol_insert(current_table, $2->ident.str, FIELD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("field", FIELD_TYPE, sizeof(struct tree_field));
		node->field.name = $2->ident.str;
		node->field.bitrange = $3;
		node->field.templates = $4;
		node->common.print_node = print_field;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;

		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>5;
		field_attr_t* attr = (field_attr_t*)(node->common.attr);
		node->field.spec = $6;
		attr->desc = get_obj_desc($6);
		attr->table = get_obj_block_table($6);
		$$ = node;
	}
	| FIELD objident istemplate {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		field_attr_t* attr = (field_attr_t*)gdml_zmalloc(sizeof(field_attr_t));
		attr->name = $2->ident.str;
		attr->is_range = 0;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, FIELD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("field", FIELD_TYPE, sizeof(struct tree_field));
		node->field.name = $2->ident.str;
		node->field.bitrange = NULL;
		node->field.templates = $3;
		node->common.print_node = print_field;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		field_attr_t* attr = (field_attr_t*)(node->common.attr);
		node->field.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| DATA cdecl ';' {
		/* TODO: should find the identifier*/
		#if 0
		node_t* node = find_node($2, IDENT_TYPE);
		data_attr_t* attr = malloc(sizeof(data_attr_t));
		memset(attr, 0, sizeof(data_attr_t));
		attr->name = strdup(node->name);
		symbol_insert(node->name, DATA_TYPE, attr);
		#endif

		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_data = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| CONNECT objident istemplate {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		connect_attr_t* attr = (connect_attr_t*)gdml_zmalloc(sizeof(connect_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 0;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, CONNECT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("connect", CONNECT_TYPE, sizeof(struct tree_connect));
		node->connect.name = $2->ident.str;
		node->connect.templates = $3;
		node->common.print_node = print_connect;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		DBG("CONNECT_TYPE: %s\n", $2->ident.str);
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		connect_attr_t* attr = (connect_attr_t*)(node->common.attr);
		node->connect.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| INTERFACE objident istemplate {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		interface_attr_t* attr = (interface_attr_t*)gdml_zmalloc(sizeof(interface_attr_t));
		attr->name = $2->ident.str;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, INTERFACE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("interface", INTERFACE_TYPE, sizeof(struct tree_interface));
		node->interface.name = $2->ident.str;
		node->interface.templates = $3;
		node->common.print_node = print_interface;
		node->common.attr =attr;
		current_object_node = node;

		attr->common.node = node;
		DBG("Interface_type: %s\n", $2->ident.str);
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		interface_attr_t* attr = (interface_attr_t*)(node->common.attr);
		node->interface.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| ATTRIBUTE objident istemplate {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		attribute_attr_t* attr = (attribute_attr_t*)gdml_zmalloc(sizeof(attribute_attr_t));
		attr->name = $2->ident.str;
		attr->arraydef = 0;
		attr->templates =  get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, ATTRIBUTE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("attribute", ATTRIBUTE_TYPE, sizeof(struct tree_attribute));
		node->attribute.name = $2->ident.str;
		node->attribute.templates = $3;
		node->common.print_node = print_attribute;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		DBG("Attribute: %s\n", $2->ident.str);
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		attribute_attr_t* attr = (attribute_attr_t*)(node->common.attr);
		node->attribute.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| EVENT objident istemplate {
		event_attr_t* attr = (event_attr_t*)gdml_zmalloc(sizeof(event_attr_t));
		attr->name = $2->ident.str;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, EVENT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("event", EVENT_TYPE, sizeof(struct tree_event));
		node->event.name = $2->ident.str;
		node->event.templates = $3;
		node->common.print_node = print_event;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		event_attr_t* attr = (event_attr_t*)(node->common.attr);
		node->event.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| GROUP objident istemplate {
		group_attr_t* attr = (group_attr_t*)gdml_zmalloc(sizeof(group_attr_t));
		attr->name = $2->ident.str;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, GROUP_TYPE, attr);

		tree_t* node = (tree_t*)create_node("group", GROUP_TYPE, sizeof(struct tree_group));
		node->group.name = $2->ident.str;
		node->group.templates = $3;
		node->common.print_node = print_group;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		group_attr_t* attr = (group_attr_t*)(node->common.attr);
		node->group.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| PORT objident istemplate {
		port_attr_t* attr = (port_attr_t*)gdml_zmalloc(sizeof(port_attr_t));
		attr->name = $2->ident.str;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, IMPLEMENT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("port", PORT_TYPE, sizeof(struct tree_port));
		node->port.name = $2->ident.str;
		node->port.templates = $3;
		node->common.print_node = print_port;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		port_attr_t* attr = (port_attr_t*)(node->common.attr);
		node->port.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| IMPLEMENT objident istemplate {
		implement_attr_t* attr = (implement_attr_t*)gdml_zmalloc(sizeof(implement_attr_t));
		attr->name = $2->ident.str;
		attr->templates = get_templates($3);
		attr->template_num = get_list_num($3);
		symbol_insert(current_table, $2->ident.str, IMPLEMENT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("implement", IMPLEMENT_TYPE, sizeof(struct tree_implement));
		node->implement.name = $2->ident.str;
		node->implement.templates = $3;
		node->common.print_node = print_implement;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		DBG("objident: %s\n", $2->ident.str);
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>4;
		implement_attr_t* attr = (implement_attr_t*)(node->common.attr);
		node->implement.spec = $5;
		attr->desc = get_obj_desc($5);
		attr->table = get_obj_block_table($5);
		$$ = node;
	}
	| ATTRIBUTE objident '[' arraydef ']' istemplate {
		attribute_attr_t* attr = (attribute_attr_t*)gdml_zmalloc(sizeof(attribute_attr_t));
		attr->name = $2->ident.str;
		attr->arraydef = 1;
		attr->arraydef = get_arraydef($4);
		attr->templates =  get_templates($6);
		attr->template_num = get_list_num($6);
		symbol_insert(current_table, $2->ident.str, ATTRIBUTE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("attribute", ATTRIBUTE_TYPE, sizeof(struct tree_attribute));
		node->attribute.name = $2->ident.str;
		node->attribute.arraydef = $4;
		node->attribute.templates = $6;
		node->common.print_node = print_attribute;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;

		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>7;
		attribute_attr_t* attr = (attribute_attr_t*)(node->common.attr);
		node->attribute.spec = $8;
		attr->desc = get_obj_desc($8);
		attr->table = get_obj_block_table($8);
		$$ = node;
	}
	| GROUP objident '[' arraydef ']' istemplate {
		group_attr_t* attr = (group_attr_t*)gdml_zmalloc(sizeof(group_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 1;
		attr->arraydef = get_arraydef($4);
		attr->templates = get_templates($6);
		attr->template_num = get_list_num($6);

		tree_t* node = (tree_t*)create_node("group", GROUP_TYPE, sizeof(struct tree_group));
		node->group.name = $2->ident.str;
		node->group.is_array = 1;
		node->group.array = $4;
		node->group.templates = $6;
		node->common.print_node = print_group;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>7;
		group_attr_t* attr = (group_attr_t*)(node->common.attr);
		node->group.spec = $8;
		attr->desc = get_obj_desc($8);
		attr->table = get_obj_block_table($8);
		$$ = node;
	}
	| CONNECT objident '[' arraydef ']' istemplate {
		connect_attr_t* attr = (connect_attr_t*)gdml_zmalloc(sizeof(connect_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 1;
		attr->arraydef = get_arraydef($4);
		attr->templates = get_templates($6);
		attr->template_num = get_list_num($6);
		symbol_insert(current_table, $2->ident.str, CONNECT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("connect", CONNECT_TYPE, sizeof(struct tree_connect));
		node->connect.name = $2->ident.str;
		node->connect.arraydef = $4;
		node->connect.templates = $6;
		node->common.print_node = print_connect;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;

		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>7;
		connect_attr_t* attr = (connect_attr_t*)(node->common.attr);
		node->connect.spec = $8;
		attr->desc = get_obj_desc($8);
		attr->table = get_obj_block_table($8);
		$$ = node;
	}
	;

method
	: METHOD objident method_params {
		method_attr_t* attr = (method_attr_t*)gdml_zmalloc(sizeof(method_attr_t));
		attr->name = $2->ident.str;
		attr->is_extern = 0;
		attr->is_default = 0;
		attr->method_params = get_method_params($3, current_table);
		symbol_insert(current_table, $2->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $2->ident.str;
		node->method.is_extern = 0;
		node->method.is_default = 0;
		node->method.params = $3;
		node->common.print_node = print_method;
		node->common.attr = attr;

		attr->common.node = node;
		DBG("method is %s\n", $2->ident.str);

		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		attr->table = current_table;

		/* insert the parameters of method into table */
		params_insert_table(current_table, attr->method_params);
		$<tree_type>$ = node;
	}
	compound_statement {
		tree_t* node = $<tree_type>4;
		node->method.block = $5;
		//current_table = prefix_table;
		current_table = pop(table_stack);
		$$ = node;
	}
	| METHOD objident method_params DEFAULT {
		method_attr_t* attr = (method_attr_t*)gdml_zmalloc(sizeof(method_attr_t));
		attr->name = $2->ident.str;
		attr->is_extern = 0;
		attr->is_default = 1;
		attr->method_params = get_method_params($3, current_table);
		symbol_insert(current_table, $2->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $2->ident.str;
		node->method.is_extern = 0;
		node->method.is_default = 1;
		node->method.params = $3;
		node->common.print_node = print_method;
		node->common.attr = attr;

		attr->common.node = node;
		DBG("method is %s\n", $2->ident.str);

		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		attr->table = current_table;
		/* insert the parameters of method into table */
		params_insert_table(current_table, attr->method_params);
		$<tree_type>$ = node;
	}
	compound_statement {
		tree_t* node = $<tree_type>5;
		node->method.block = $6;
		current_table = pop(table_stack);
		$$ = node;
	}
	| METHOD EXTERN objident method_params {
		method_attr_t* attr = (method_attr_t*)gdml_zmalloc(sizeof(method_attr_t));
		attr->name = $3->ident.str;
		attr->is_extern = 1;
		attr->is_default = 0;
		attr->method_params = get_method_params($4, current_table);
		symbol_insert(current_table, $3->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $3->ident.str;
		node->method.is_extern = 1;
		node->method.is_default = 0;
		node->method.params = $4;
		node->common.print_node = print_method;
		node->common.attr = attr;

		attr->common.node = node;
		DBG("method extern is %s\n", $3->ident.str);

		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		attr->table = current_table;
        /* insert the parameters of method into table */
        params_insert_table(current_table, attr->method_params);
		$<tree_type>$ = node;
	}
	compound_statement {
		tree_t* node = $<tree_type>5;
		node->method.block = $6;
		//current_table = prefix_table;
		current_table = pop(table_stack);
		$$ = node;
	}
	| METHOD EXTERN objident method_params DEFAULT {
		method_attr_t* attr = (method_attr_t*)gdml_zmalloc(sizeof(method_attr_t));
		attr->name = $3->ident.str;
		attr->is_extern = 1;
		attr->is_default = 1;
		attr->method_params = get_method_params($4, current_table);
		symbol_insert(current_table, $3->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $3->ident.str;
		node->method.is_extern = 1;
		node->method.is_default = 1;
		node->method.params = $4;
		node->common.print_node = print_method;
		node->common.attr = attr;

		attr->common.node = node;
		DBG("method extern is %s\n", $3->ident.str);

		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		attr->table = current_table;
        /* insert the parameters of method into table */
        params_insert_table(current_table, attr->method_params);
        $<tree_type>$ = node;
	}
	compound_statement {
        tree_t* node = $<tree_type>6;
        node->method.block = $7;
        //current_table = prefix_table;
        current_table = pop(table_stack);
		$$ = node;
	}
	;

arraydef
	: expression {
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 1;
		node->array.expr = $1;
		node->common.print_node = print_arraydef;
		$$ = node;
	}
	| ident IN expression RANGE_SIGN expression {
		if ($1 == NULL) {
			fprintf(stderr, "There must be identifier!\n");
			exit(-1);
		}
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 0;
		node->array.ident = $1;
		node->array.expr = $3;
		node->array.expr_end = $5;
		node->common.print_node = print_arraydef;
		$$ = node;
	}
	;

toplevel
	: TEMPLATE objident {
		DBG("in TEMPLATE %s\n", $2->ident.str);
		template_attr_t* attr = (template_attr_t*)gdml_zmalloc(sizeof(template_attr_t));
		attr->name = $2->ident.str;
		if (symbol_insert(current_table, $2->ident.str, TEMPLATE_TYPE, attr) == -1) {
			fprintf(stderr, "redefined\n");
			/* FIXME: handle the error */
			exit(-1);
		}

		tree_t* node = (tree_t*)create_node("template", TEMPLATE_TYPE, sizeof(struct tree_template));
		node->temp.name = $2->ident.str;
		node->common.print_node = print_template;
		node->common.attr = attr;
		current_object_node = node;

		attr->common.node = node;
		object_spec_type = TEMPLATE_TYPE;
		$<tree_type>$ = node;
	}
	object_spec {
		tree_t* node = $<tree_type>3;
		template_attr_t* attr = (template_attr_t*)(node->common.attr);
		node->temp.spec = $4;
		attr->desc = get_obj_desc($4);
		attr->table = get_obj_block_table($4);
		object_spec_type = -1;
		$$ = node;
	}
	| LOGGROUP ident ';' {
		DBG("in LOGGROUP %s\n", $2->ident.str);
		if (current_table->table_num == 1) {
			symbol_insert(current_table, $2->ident.str, LOGGROUP_TYPE, NULL);
		}
		else {
			fprintf(stderr, "loggroup %s should the topest level\n", $2->ident.str);
		}

		tree_t* node = (tree_t*)create_node("loggroup", LOGGROUP_TYPE, sizeof(struct tree_loggroup));
		node->loggroup.name = $2->ident.str;
		node->common.print_node = print_loggroup;
		$$ = node;
	}
	| CONSTANT ident '=' expression ';' {
		if ((current_table->table_num) != 1) {
			fprintf(stderr, "constant %s should the topest level\n", $2->ident.str);
			exit(-1);
		}
		constant_attr_t* attr = (constant_attr_t*)gdml_zmalloc(sizeof(constant_attr_t));
		attr->name = $2->ident.str;
		attr->common.table_num = current_table->table_num;
		attr->value = parse_expression($4, current_table);
		/*FIXME: should calulate the expression value */
		symbol_insert(current_table, $2->ident.str, CONSTANT_TYPE, attr);
		printf("constant type: %d\n", attr->value->final_type);

		tree_t* node = (tree_t*)create_node("assign", ASSIGN_TYPE, sizeof(struct tree_assign));
		node->assign.is_constant = 1;
		node->assign.decl = $2;
		node->assign.expr = $4;
		node->common.print_node = print_assign;
		//node->common.attr = attr;

		attr->common.node = node;
		$$ = node;
	}
	| EXTERN cdecl_or_ident ';' {
		/* TODO: we should find the name of extern */
		//symbol_t* symbol = symbol_find()
		DBG("\nPay attention: we should find the name of extern\n\n");
		#if 0
		cdecl_attr_t* attr = (cdecl_attr_t*)gdml_zmalloc(sizeof(cdecl_attr_t));
		attr->is_extern = 1;
		#endif
		parse_extern_cdecl_or_ident($2, current_table);

		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_extern = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| TYPEDEF cdecl ';' {
		/* FIXME: we should find the name cdecl */
		DBG("\nPay attention: we should find the name of typedef cdecl\n\n");
		//$$ = create_node("UNIMP", TYPEDEF_TYPE);
		#if 0
		cdecl_attr_t* attr = (cdecl_attr_t*)gdml_zmalloc(sizeof(cdecl_attr_t));
		attr->is_typedef = 1;
		#endif

		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_typedef = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| EXTERN TYPEDEF cdecl ';' {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_extern = 1;
		node->cdecl.is_typedef = 1;
		node->cdecl.decl = $3;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| STRUCT ident '{' {
		struct_attr_t* attr = (struct_attr_t*)gdml_zmalloc(sizeof(struct_attr_t));
		attr->name = $2->ident.str;

		current_table = change_table(current_table, table_stack, &current_table_num, STRUCT_TYPE);
		attr->table = current_table;

		tree_t* node = (tree_t*)create_node("struct", STRUCT_TYPE, sizeof(struct tree_struct));
		node->struct_tree.ident = $2->ident.str;
		node->common.print_node = print_struct;
		node->common.attr = attr;
		$<tree_type>$ = node;
	}
	struct_decls '}' {
		tree_t* node = $<tree_type>4;
		node->struct_tree.block = $5;
		current_table = pop(table_stack);
		$$ = node;
	}
	| HEADER {
		/* FIXME: the header include much content */
		tree_t* node = (tree_t*)create_node("header", HEADER_TYPE, sizeof(struct tree_head));
		node->head.str = $1;
		node->common.print_node = print_header;
		/* TODO: should analyze the content of head */
		//node->head.head = $1;
		$$ = node;
	}
	;

istemplate_stmt
	: IS objident ';' {
		DBG("In IS statement\n");
		#if 0
		symbol_t* symbol = symbol_find($2, TEMPLATE_TYPE);	
		if(symbol == NULL){
			fprintf(stderr, "Pay attention: No such template %s in IS statement\n", $2);
		}
		#endif
		add_template_to_table(current_table, $2->ident.str);
		print_templates(current_table);
		$$ = $2;
	}
	;

import
	: IMPORT STRING_LITERAL ';' {
		DBG("import file is %s\n", $2);
		printf("import file is %s\n", $2);
		char fullname[1024];
		int dirlen = strlen(dir);
		int filelen = strlen($2);

		/* FIXME, should check if the same filename is imported already. */
		//symbol_insert($2, IMPORT_TYPE, NULL);
		assert((dirlen + filelen) < 1024);
		strncpy(&fullname[0], dir, dirlen);
		if(*($2) == '"') {
			$2 ++;
		}
		else {
		/* something wrong */
			fprintf(stderr, "the import file format is wrong\n");
		}
		if(*($2 + strlen($2) - 1) == '"') {
			strncpy(&fullname[dirlen], $2, filelen - 1);
		}
		else {
			/* something wrong */
			fprintf(stderr, "the import file format is wrong\n");
		}
		fullname[dirlen + filelen - 2] = '\0';
		DBG("In IMPORT, fullname=%s, dirlen=%d, filelen=%d\n", fullname, dirlen, filelen);

		FILE *file = fopen(fullname, "r");
		if (file == NULL) {
			printf("Can't open imported file %s.\n", fullname);
			exit(EXIT_FAILURE);
		}

		yyscan_t scanner;
		tree_t* root = (tree_t*)create_node($2, IMPORT_TYPE, sizeof(struct tree_import));
		root->import.file_name = strdup(fullname);
		DBG("Begin parse the import file %s\n", fullname);
		tree_t* ast = NULL;
		yylex_init(&scanner);
		yyrestart(file, scanner);
		yyparse(scanner, &ast);
		yylex_destroy(scanner);
		fclose(file);
		DBG("End of parse the import file %s\n", fullname);

		$$ = ast->common.child;
		//$$ = ast;
	}
	;

object_desc
	: STRING_LITERAL {
		tree_t* node = (tree_t*)create_node("const_string", CONST_STRING_TYPE, sizeof(struct tree_string));
		node->string.pointer = $1;
		node->string.length = strlen($1);
		node->common.print_node = print_object_desc;
		$$ = node;
	}
	| {
		$$ = NULL;
	}
	;

object_spec
	: object_desc ';' {
		if ($1 == NULL) {
			$$ = NULL;
		}
		else {
			tree_t* node = (tree_t*)create_node("object_spec", SPEC_TYPE, sizeof(struct tree_object_spec));
			node->spec.desc = $1;
			node->spec.block = NULL;
			node->common.print_node = print_object_spec;
			$$ = node;
		}
	}
	| object_desc '{' {
		DBG("object_statements for object_spec\n");
		tree_t* node = (tree_t*)create_node("object_spec", SPEC_TYPE, sizeof(struct tree_object_spec));
		node->spec.desc = $1;

		tree_t* block = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block));
	symtab_t table = NULL;
		if (object_spec_type == TEMPLATE_TYPE) {
			table = symtab_create(TEMPLATE_TYPE);
		}
		else {
			table = symtab_create(SPEC_TYPE);
		}
		table->table_num = ++current_table_num;
		block->block.table = symtab_insert_child(current_table, table);
		//prefix_table = current_table;
		push(table_stack, current_table);
		current_table = block->block.table;

		node->spec.block = block;
		node->common.print_node = print_object_spec;

		/* get the object templates */
		get_object_template_table(current_table, current_object_node);
		$<tree_type>$ = node;
	}
	object_statements '}' {
		tree_t* node = $<tree_type>3;
		tree_t* block = node->spec.block;
		block->block.statement = $4;
		block->common.print_node = print_obj_block;
		if ($4 == NULL) {
			stack_node_t* node = get_top(table_stack);
			symtab_free(node->data, current_table_num);
			block->block.table = NULL;
			current_table_num--;
		}
		current_table = pop(table_stack);
		$$ = node;
	}
	;

object_statements
	: object_statements object_statement {
		DBG("In object_statements\n");
		if($1 == NULL && $2 != NULL) {
			$$ = $2;
		}
		else if($1 != NULL && $2 != NULL) {
			$$ = (tree_t*)create_node_list($1, $2);
		}
		else {
			fprintf(stderr, "maybe something wrong in object_statements\n");
		}
	}
	| {
		$$ = NULL;
	}
	;

object_statement
	: object {
		DBG("in object for object_statement\n");
		$$ = $1;
	}
	| parameter {
		$$ = $1;
	}
	| method {
		DBG("method \n");
		$$ = $1;
	}
	| istemplate_stmt {
		DBG("istemplate_stmt in object_statement\n");
		$$ = $1;
	}
	| object_if {
		DBG("object_if in object_statement \n");
		$$ = $1;
	}
	;

object_if_statement
	: IF '(' expression ')' '{' {
		tree_t* node =  (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		parse_expression($3, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);
		$<tree_type>$ = node;
	}
	object_statements '}' {
		tree_t* node = $<tree_type>6;
		node->if_else.if_block = $7;
		node->common.print_node = print_if_else;

		current_table = pop(table_stack);
		$$ = node;
	}

object_if
	: object_if_statement {
		$$ = $1;
	}
	| object_if_statement ELSE '{' {
		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);
		$<tree_type>$ = NULL;
	}
	object_statements '}' {
		tree_t* node = $1;
		node->if_else.else_block = $5;

		current_table = pop(table_stack);

		$$ = node;
	}
	| object_if_statement ELSE {
		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);
		$<tree_type>$ = NULL;
	}
	object_if {
		tree_t* node = $1;
		node->if_else.else_block = $4;

		current_table = pop(table_stack);

		$$ = node;
	}
	;

parameter
	: PARAMETER objident paramspec {
		if ($2 == NULL) {
			fprintf(stderr, "parameter should ojbidentifier!\n");
			exit(-1);
		}
		parameter_attr_t* attr = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));
		attr->name = $2->ident.str;
		attr->spec = get_paramspec($3, current_table);
		symbol_insert(current_table, $2->ident.str, PARAMETER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("parameter", PARAMETER_TYPE, sizeof(struct tree_param));
		node->param.name = $2->ident.str;
		node->param.paramspec = $3;
		node->common.print_node = print_parameter;
		DBG("parameter name is %s\n", $2->ident.str);
		$$ = node;
	}
	;

paramspec
	: ';' {
		$$ = NULL;
	}
	| '=' expression ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.expr = $2;
		node->common.print_node = print_paramspec;
		$$ = node;
	}
	| '=' STRING_LITERAL ';' {
		tree_t* str = (tree_t*)create_node("const_string", CONST_STRING_TYPE, sizeof(struct tree_string));
		str->string.pointer = $2;
		str->string.length = strlen($2);
		str->common.print_node = print_string;

		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.string = str;
		node->common.print_node = print_paramspec;
		DBG("paramspec: %s\n", $2);
		$$ = node;
	}
	| DEFAULT expression ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.is_default = 1;
		node->paramspec.expr = $2;
		node->common.print_node = print_paramspec;
		$$ = node;
	}
	| AUTO ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.is_auto = 1;
		node->common.print_node = print_paramspec;
		$$ = node;
	}
	;

method_params
	:  {
		$$ = NULL;
	}
	| '(' cdecl_or_ident_list ')' {
		tree_t* node = (tree_t*)create_node("method_params", PARAM_TYPE, sizeof(struct tree_params));
		node->params.have_in_param = 1;
		/* TODO: maybe we should get the pararmeters' type */
		//node->params.in_argc = get_node_num($2);
		node->params.in_params = $2;
		node->params.have_ret_param = 0;
		node->params.ret_params = NULL;
		node->common.print_node = print_method_params;
		$$ = node;
	}
	| METHOD_RETURN '(' cdecl_or_ident_list ')' {
		tree_t* node = (tree_t*)create_node("method_params", PARAM_TYPE, sizeof(struct tree_params));
		node->params.in_params = NULL;
		node->params.have_in_param = 0;
		node->params.have_ret_param = 1;
		//node->params.ret_argc = get_node_num($3);
		/* TODO: maybe we should get the pararmeters' type */
		node->params.ret_params = $3;
		node->common.print_node = print_method_params;
		$$ = node;
	}
	| '(' cdecl_or_ident_list ')' METHOD_RETURN '(' cdecl_or_ident_list ')' {
		tree_t* node = (tree_t*)create_node("method_params", PARAM_TYPE, sizeof(struct tree_params));
		node->params.have_in_param = 1;
		//node->params.in_argc = get_node_num($2);
		/* TODO: maybe we should get the pararmeters' type */
		node->params.in_params = $2;
		node->params.have_ret_param = 1;
		//node->params.ret_argc = get_node_num($6);
		/* TODO: maybe we should get the pararmeters' type */
		node->params.ret_params = $6;
		node->common.print_node = print_method_params;
		DBG("with METHOD_RETURN in method_params\n");
		$$ = node;
	}
	;

returnargs
	: {
		$$ = NULL;
	}
	| METHOD_RETURN '(' expression_list ')' {
		$$ = $3;
	}
	;

istemplate
	: IS '(' objident_list ')' {
		$$ = $3;
	}
	| {
		$$ = NULL;
	}
	;

sizespec
	: SIZE expression {
		$$ = $2;
	}
	| {
		/* FIXME: this should make special handle */
		DBG("\nPay attention: the default value should read template!\n\n");
		$$ = NULL;
	}
	;

offsetspec
	: REG_OFFSET expression {
		$$ = $2;
	}
	| {
		/* FIXME: this should make special handle */
		DBG("\nPay attention: the default value should read template2!\n");
		$$ = NULL;
	}
	;

bitrange
	: '[' expression ']' {
		/* should calulate the expression value */
		bitrange_attr_t* attr = (bitrange_attr_t*)gdml_zmalloc(sizeof(bitrange_attr_t));
		attr->is_fix = 1;
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 1;
		node->array.expr = $2;
		node->common.print_node = print_array;
		node->common.attr = attr;

		attr->common.node = node;
		$$ = node;
	}
	| '[' expression ':' expression ']' {
		/* should calulate the expression value */
		bitrange_attr_t* attr = (bitrange_attr_t*)gdml_zmalloc(sizeof(bitrange_attr_t));
		attr->is_fix = 0;
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 0;
		node->array.expr = $2;
		node->array.expr_end = $4;
		node->common.print_node = print_array;
		$$ = node;
	}
	;

cdecl_or_ident
	: cdecl {
		$$ = $1;
	}
	;

cdecl
	: basetype cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl))	;
		node->cdecl.basetype = $1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| CONST basetype cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl))	;
		node->cdecl.is_const = 1;
		node->cdecl.basetype = $2;
		node->cdecl.decl = $3;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	;

basetype
	: typeident {
		DBG("typeident: %s\n", $1->common.name);
		$$ = $1;
	}
	| struct {
		DBG("struct\n");
		$$ = $1;
	}
	| layout {
		$$ = $1;
	}
	| bitfields {
		$$ = $1;
	}
	| typeof {
		$$ = $1;
	}
	;

cdecl2
	: cdecl3 {
		$$ = $1;
	}
	| CONST cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_const = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| '*' cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_point = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| VECT cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.is_vect = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	;

cdecl3
	: typeident {
		DBG("typeident: %s\n", $1->common.name);
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	| cdecl3 '[' expression ']' {
		tree_t* node = (tree_t*)create_node("array_cdecl", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 1;
		node->array.expr = $3;
		node->array.decl = $1;
		node->common.print_node = print_array;
		$$ = node;
	}
	| cdecl3 '(' cdecl_list ')' {
		tree_t* node = (tree_t*)create_node("cdecl_brack", CDECL_BRACK_TYPE, sizeof(struct tree_cdecl_brack));
		node->cdecl_brack.is_list = 1;
		node->cdecl_brack.cdecl = $1;
		node->cdecl_brack.decl_list = $3;
		node->common.print_node = print_cdecl_brak;
		$$ = node;
	}
	| '(' cdecl2 ')' {
		tree_t* node = (tree_t*)create_node("cdecl_brack", CDECL_BRACK_TYPE, sizeof(struct tree_cdecl_brack));
		node->cdecl_brack.cdecl = NULL;
		node->cdecl_brack.decl_list = $2;
		node->common.print_node = print_cdecl_brak;
		$$ = node;
	}
	;

cdecl_list
	: {
		$$ = NULL;
	}
	| cdecl_list2 {
		$$ = $1;
	}
	;

cdecl_list2
	: cdecl {
		$$ = $1;
	}
	| ELLIPSIS {
		tree_t* node = (tree_t*)create_node("...", ELLIPSIS_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_ellipsis;
		$$ = node;
	}
	| cdecl_list2 ',' cdecl {
		if ($1 == NULL) {
			$$ = $3;
		}
		else {
			create_node_list($1, $3);
		}
		$$ = $1;
	}
	| cdecl_list2 ',' ELLIPSIS {
		tree_t* node = (tree_t*)create_node("...", ELLIPSIS_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_ellipsis;
		if ($1 == NULL) {
			$$ = node;
		}
		else {
			create_node_list($1, node);
		}
		$$ = $1;
	}
	;

cdecl_or_ident_list
	: {
		$$ = NULL;
	}
	| cdecl_or_ident_list2 {
		$$ = $1;
	}
	;

cdecl_or_ident_list2
	: cdecl_or_ident {
		$$ = $1;
	}
	| cdecl_or_ident_list2 ',' cdecl_or_ident {
		if (($1 == NULL) && ($3 == NULL)) {
			fprintf(stderr, "The cdecl maybe wrong!\n");
			$$ = NULL;
		}
		else if (($1 == NULL) && ($3 != NULL)) {
			$$ = $3;
		}
		else if (($1 != NULL) && ($3 == NULL)) {
			fprintf(stderr, "There cdecl maybe wrong!\n");
			$$ = $1;
		}
		else {
			create_node_list($1, $3);
			$$ = $1;
		}
	}
	;

typeof
	: TYPEOF expression {
		tree_t* node = (tree_t*)create_node("typeof", TYPEOF_TYPE, sizeof(struct tree_struct));
		node->typeof_tree.expr = $2;
		node->common.print_node = print_typeof;
		$$ = node;
	}
	;

struct
	: STRUCT '{' struct_decls '}' {
		tree_t* node = (tree_t*)create_node("struct", STRUCT_TYPE, sizeof(struct tree_struct));
		node->struct_tree.block = $3;
		node->common.print_node = print_struct;
		$$ = node;
	}
	;

struct_decls
	: struct_decls cdecl ';' {
		if ($1 == NULL) {
			$$ = $2;
		}
		else {
			create_node_list($1, $2);
		}
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	;

layout
	:LAYOUT STRING_LITERAL '{' {
		layout_attr_t* attr = (layout_attr_t*)gdml_zmalloc(sizeof(layout_attr_t));
		attr->str = $2;
		symbol_insert(current_table, $2, LAYOUT_TYPE, attr);

		current_table = change_table(current_table, table_stack, &current_table_num, LAYOUT_TYPE);
		attr->table  = current_table;

		tree_t* node = (tree_t*)create_node("layout", LAYOUT_TYPE, sizeof(struct tree_layout));
		node->layout.name = $2;
		node->common.print_node = print_layout;
		node->common.attr = attr;

		attr->common.node = node;

		$<tree_type>$ = node;
	}
	layout_decls '}' {
		tree_t* node = $<tree_type>4;
		node->layout.block = $5;
		current_table = pop(table_stack);
		$$ = node;
	}
	;

layout_decls
	:layout_decls cdecl ';' {
		create_node_list($1, $2);
		$$ = $1;
	}
	|  {
		$$ = NULL;
	}
	;

bitfields
	: BITFIELDS INTEGER_LITERAL '{' {
		bitfield_attr_t* attr = (bitfield_attr_t*)gdml_zmalloc(sizeof(bitfield_attr_t));
		attr->name = $2;
		symbol_insert(current_table, $2, BITFIELDS_TYPE, attr);

		current_table = change_table(current_table, table_stack, &current_table_num, BITFIELDS_TYPE);
		attr->table = current_table;

		tree_t* node = (tree_t*)create_node("bitfields", BITFIELDS_TYPE, sizeof(struct tree_bitfields));
		node->bitfields.name = $2;
		node->common.print_node = print_bitfields;
		node->common.attr = attr;

		attr->common.node = node;
		$<tree_type>$ = node;
	}
	bitfields_decls '}' {
		tree_t* node = $<tree_type>4;
		node->bitfields.block = $5;
		current_table = pop(table_stack);
		$$ = node;
	}
	;

bitfields_decls
	: bitfields_decls cdecl '@' '[' expression ':' expression ']' ';' {
		tree_t* node = (tree_t*)create_node("bitfields_decls", BITFIELDS_DECL_TYPE, sizeof(struct tree_bitfields_dec));
		node->bitfields_dec.decl = $2;
		node->bitfields_dec.start = $5;
		node->bitfields_dec.end = $7;
		node->common.print_node = print_bitfields_decls;
		if ($1 != NULL) {
			create_node_list($1, node);
			$$ = $1;
		}
		else {
			$$ = node;
		}
	}
	| {
		$$ = NULL;
	}
	;

ctypedecl
	: const_opt basetype ctypedecl_ptr {
		tree_t* node = (tree_t*)create_node("ctypedecl", CTYPEDECL_TYPE, sizeof(struct tree_ctypedecl));
		printf("line: %d, name: %s\n", __LINE__, node->common.name);
		node->ctypedecl.const_opt = $1;
		node->ctypedecl.basetype = $2;
		node->ctypedecl.ctypedecl_ptr = $3;
		node->common.print_node = print_ctypedecl;
		$$ = node;
	}
	;

ctypedecl_ptr
	: stars ctypedecl_array {
		tree_t* node = (tree_t*)create_node("ctypedecl_ptr", CTYPEDECL_PTR_TYPE, sizeof(struct tree_ctypedecl_ptr));
		node->ctypedecl_ptr.stars = $1;
		node->ctypedecl_ptr.array = $2;
		node->common.print_node = print_ctypedecl_ptr;
		$$ = node;
	}
	;

stars
	:  {
		$$ = NULL;
	}
	| '*' CONST stars {
		tree_t* node = (tree_t*)create_node("stars", STARS_TYPE, sizeof(struct tree_stars));
		node->stars.is_const = 1;
		node->stars.stars = $3;
		node->common.print_node = print_stars;
		$$ = node;
	}
	| '*' stars {
		tree_t* node = (tree_t*)create_node("stars", STARS_TYPE, sizeof(struct tree_stars));
		node->stars.is_const = 0;
		node->stars.stars = $2;
		node->common.print_node = print_stars;
		$$ = node;
	}
	;

ctypedecl_array
	: ctypedecl_simple {
		$$ = $1;
	}
	;

ctypedecl_simple
	: '(' ctypedecl_ptr ')' {
		tree_t* node = (tree_t*)create_node("ctypedecl_simple", CTYPEDECL_SIMPLE_TYPE, sizeof(struct tree_cdecl_brack));
		node->cdecl_brack.decl_list = $2;
		node->common.print_node = print_ctypedecl_simple;
		$$ = node;
	}
	| {
		$$ = NULL;
	}
	;

const_opt
	: CONST  {
		tree_t* node  = (tree_t*)c_keyword_node("const");
		node->ident.type = CONST_TYPE;
		$$ = node;
	}
	|  {
		$$ = NULL;
	}
	;

typeident
	: ident {
		$$ = $1;
	}
	| CHAR {
		tree_t* node = (tree_t*)c_keyword_node("char");
		node->ident.type = CHAR_TYPE;
		$$ = node;
	}
	| DOUBLE {
		tree_t* node = (tree_t*)c_keyword_node("double");
		node->ident.type = DOUBLE_TYPE;
		$$ = node;
	}
	| FLOAT {
		tree_t* node = (tree_t*)c_keyword_node("float");
		node->ident.type = FLOAT_TYPE;
		$$ = node;
	}
	| INT {
		tree_t* node = (tree_t*)c_keyword_node($1);
		node->ident.type = INT_TYPE;
		$$ = node;
	}
	| LONG {
		tree_t* node = (tree_t*)c_keyword_node("long");
		node->ident.type = LONG_TYPE;
		$$ = node;
	}
	| SHORT  {
		tree_t* node = (tree_t*)c_keyword_node("short");
		node->ident.type = SHORT_TYPE;
		$$ = node;
	}
	| SIGNED {
		tree_t* node = (tree_t*)c_keyword_node("signed");
		node->ident.type = SIGNED_TYPE;
		$$ = node;
	}
	| UNSIGNED {
		tree_t* node = (tree_t*)c_keyword_node("unsigned");
		node->ident.type = UNSIGNED_TYPE;
		$$ = node;
	}
	| VOID {
		tree_t* node = (tree_t*)c_keyword_node("void");
		node->ident.type = VOID_TYPE;
		$$ = node;
	}
	;

comma_expression
	: expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = $1;
	}
	| comma_expression ',' expression {
		debug_proc("Line : %d\n", __LINE__);
		create_node_list($1, $3);
		$$ = $1;
	}
	;

expression
	: expression '=' expression {
		tree_t* node = (tree_t*)create_node("assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("=");
		node->expr_assign.type = EXPR_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_expr_assign;
		$$ = node;
	}
	| expression ADD_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("add_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("+=");
		node->expr_assign.type = ADD_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression SUB_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("sub_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("-=");
		node->expr_assign.type = SUB_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression MUL_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("mul_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("*=");
		node->expr_assign.type = MUL_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression DIV_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("div_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("/=");
		node->expr_assign.type = DIV_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression MOD_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("mod_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("%=");
		node->expr_assign.type = MOD_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression OR_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("or_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("|=");
		node->expr_assign.type = OR_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression AND_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("and_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("&=");
		node->expr_assign.type = AND_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression XOR_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("xor_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("^=");
		node->expr_assign.type = XOR_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression LEFT_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("left_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("<<=");
		node->expr_assign.type = LEFT_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression RIGHT_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("right_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup(">>=");
		node->expr_assign.type = RIGHT_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '?' expression ':' expression {
		tree_t* node = (tree_t*)create_node("ternary", TERNARY_TYPE, sizeof(struct tree_ternary));
		node->ternary.cond = $1;
		node->ternary.expr_true = $3;
		node->ternary.expr_false = $5;
		node->common.print_node = print_ternary;
		$$ = node;
	}
	| expression '+' expression {
		tree_t* node = (tree_t*)create_node("add", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("+");
		node->binary.type = ADD_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '-' expression {
		tree_t* node = (tree_t*)create_node("sub", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("-");
		node->binary.type = SUB_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '*' expression {
		tree_t* node = (tree_t*)create_node("mul", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("*");
		node->binary.type = MUL_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '/' expression {
		tree_t* node = (tree_t*)create_node("div", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("/");
		node->binary.type = DIV_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '%' expression {
		tree_t* node = (tree_t*)create_node("mod", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("%");
		node->binary.type = MOD_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression LEFT_OP expression {
		tree_t* node = (tree_t*)create_node("left_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("<<");
		node->binary.type = LEFT_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression RIGHT_OP expression {
		tree_t* node = (tree_t*)create_node("right_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup(">>");
		node->binary.type = RIGHT_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression EQ_OP expression {
		tree_t* node = (tree_t*)create_node("eq_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("==");
		node->binary.type = EQ_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression NE_OP expression {
		tree_t* node = (tree_t*)create_node("ne_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("!=");
		node->binary.type = NE_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '<' expression {
		tree_t* node = (tree_t*)create_node("less", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("<");
		node->binary.type = LESS_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '>' expression {
		tree_t* node = (tree_t*)create_node("great", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup(">");
		node->binary.type = GREAT_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression LE_OP expression {
		tree_t* node = (tree_t*)create_node("less_eq", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("<=");
		node->binary.type = LESS_EQ_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression GE_OP expression {
		tree_t* node = (tree_t*)create_node("great_eq", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup(">=");
		node->binary.type = GREAT_EQ_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression OR_OP expression {
		tree_t* node = (tree_t*)create_node("or_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("||");
		node->binary.type = OR_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression AND_OP expression {
		tree_t* node = (tree_t*)create_node("and_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("&&");
		node->binary.type = AND_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '|' expression {
		tree_t* node = (tree_t*)create_node("or", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("|");
		node->binary.type = OR_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '^' expression {
		tree_t* node = (tree_t*)create_node("xor", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("^");
		node->binary.type = XOR_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '&' expression {
		tree_t* node = (tree_t*)create_node("and", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("&");
		node->binary.type = AND_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| CAST '(' expression ',' ctypedecl ')' {
		tree_t* node = (tree_t*)create_node("cast", CAST_TYPE, sizeof(struct tree_cast));
		node->cast.expr = $3;
		node->cast.ctype = $5;
		node->common.print_node = print_cast;
		$$ = node;
	}
	| SIZEOF expression {
		tree_t* node = (tree_t*)create_node("sizeof", SIZEOF_TYPE, sizeof(struct tree_sizeof));
		node->sizeof_tree.expr = $2;
		node->common.print_node = print_sizeof;
		$$ = node;
	}
	| '-' expression {
		tree_t* node = (tree_t*)create_node("negative", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("-");
		node->unary.type = NEGATIVE_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '+' expression {
		tree_t* node = (tree_t*)create_node("convert", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("+");
		node->unary.type = CONVERT_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '!' expression {
		tree_t* node = (tree_t*)create_node("non_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("!");
		node->unary.type = NON_OP_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '~' expression {
		tree_t* node = (tree_t*)create_node("bit_non", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("~");
		node->unary.type = BIT_NON_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '&' expression {
		tree_t* node = (tree_t*)create_node("addr", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("&");
		node->unary.type = ADDR_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '*' expression {
		tree_t* node = (tree_t*)create_node("pointer", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("*");
		node->unary.type = POINTER_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| DEFINED expression {
		tree_t* node = (tree_t*)create_node("defined", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("defined");
		node->unary.type = DEFINED_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '#' expression {
		tree_t* node = (tree_t*)create_node("translates", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("#");
		node->unary.type = EXPR_TO_STR_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| INC_OP expression {
		tree_t* node = (tree_t*)create_node("pre_inc_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("pre_inc");
		node->unary.type = PRE_INC_OP_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| DEC_OP expression {
		tree_t* node = (tree_t*)create_node("pre_dec_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("pre_dec");
		node->unary.type = PRE_DEC_OP_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| expression INC_OP {
		tree_t* node = (tree_t*)create_node("aft_inc_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("aft_inc");
		node->unary.type = AFT_INC_OP_TYPE;
		node->unary.expr = $1;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| expression DEC_OP {
		tree_t* node = (tree_t*)create_node("aft_dec_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("aft_dec");
		node->unary.type = AFT_DEC_OP_TYPE;
		node->unary.expr = $1;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| expression '(' expression_list ')' {
		tree_t* node  = (tree_t*)create_node("expr_brack", EXPR_BRACK_TYPE, sizeof(struct tree_expr_brack));
		node->expr_brack.expr = $1;
		node->expr_brack.expr_in_brack = $3;
		node->common.print_node = print_expr_brack;
		$$ = node;
	}
	| INTEGER_LITERAL {
		DBG("Integer_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst));
		node->int_cst.int_str = $1;
		if (strlen($1) <= 10) {
			node->int_cst.value = strtoi($1);
		}
		else {
			node->int_cst.out_64bit = 1;
		}
		node->common.print_node = print_interger;
		$$= node;
	}
	| FLOAT_LITERAL {
		DBG("Float_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst));
		node->float_cst.float_str = $1;
		node->float_cst.value = atof($1);
		node->common.print_node = print_float_literal;
		$$= node;
	}
	| STRING_LITERAL {
		DBG("String_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("string_literal", CONST_STRING_TYPE, sizeof(struct tree_string));
		node->string.length = strlen($1);
		node->string.pointer = $1;
		node->common.print_node = print_string;
		$$=node;
	}
	| UNDEFINED {
		tree_t* node = (tree_t*)create_node("undefined", UNDEFINED_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_undefined;
		$$ = node;
	}
	| '$' objident {
		DBG("In $objident: %s\n", $2->ident.str);
		tree_t* node = (tree_t*)create_node("quote", QUOTE_TYPE, sizeof(struct tree_quote));
		node->quote.ident = $2;
		node->common.print_node = print_quote;
		$$ = node;
	}
	| ident {
		DBG("ident: %s\n", $1->ident.str);
		$$ = $1;
	}
	| expression '.' objident {
		tree_t* node = (tree_t*)create_node("dot", COMPONENT_TYPE, sizeof(struct tree_component));
		node->component.comp = strdup(".");
		node->component.type = COMPONENT_DOT_TYPE;
		node->component.expr = $1;
		node->component.ident = $3;
		node->common.print_node = print_component;
		$$ = node;
	}
	| expression METHOD_RETURN objident {
		tree_t* node = (tree_t*)create_node("pointer", COMPONENT_TYPE, sizeof(struct tree_component));
		node->component.comp = strdup("->");
		node->component.type = COMPONENT_POINTER_TYPE;
		node->component.expr = $1;
		node->component.ident = $3;
		node->common.print_node = print_component;
		$$ = node;
	}
	| SIZEOFTYPE typeoparg {
		tree_t* node = (tree_t*)create_node("sizeoftype", SIZEOFTYPE_TYPE, sizeof(struct tree_sizeoftype));
		node->sizeoftype.typeoparg = $2;
		node->common.print_node = print_sizeoftype;
		$$ = node;
	}
	;

typeoparg
	: ctypedecl {
		tree_t* node = (tree_t*)create_node("typeoparg", TYPEOPARG_TYPE, sizeof(struct tree_typeoparg));
		node->typeoparg.ctypedecl = $1;
		node->common.print_node = print_typeoparg;
		$$ = node;
	}
	| '(' ctypedecl ')' {
		tree_t* node = (tree_t*)create_node("typeoparg_brack", TYPEOPARG_TYPE, sizeof(struct tree_typeoparg));
		node->typeoparg.ctypedecl_brack = $2;
		node->common.print_node = print_typeoparg;
		$$ = node;
	}
	;

expression
	: NEW ctypedecl {
		tree_t* node = (tree_t*)create_node("new", NEW_TYPE, sizeof(struct tree_new));
		node->new_tree.type = $2;
		node->common.print_node = print_new;
		$$ = node;
	}
	| NEW ctypedecl '[' expression ']' {
		tree_t* node = (tree_t*)create_node("new", NEW_TYPE, sizeof(struct tree_new));
		node->new_tree.type = $2;
		node->new_tree.count = $4;
		node->common.print_node = print_new;
		$$ = node;
	}
	| '(' expression ')' {
		tree_t* node = (tree_t*)create_node("expr_brack", EXPR_BRACK_TYPE, sizeof(struct tree_expr_brack));
		 node->expr_brack.expr = NULL;
		 node->expr_brack.expr_in_brack = $2;
		 node->common.print_node = print_expr_brack;
		 $$ = node;

	}
	| '[' expression_list ']' {
		debug_proc("Line : %d\n", __LINE__);
        tree_t* node = (tree_t*)create_node("expr_array", ARRAY_TYPE, sizeof(struct tree_array));
        node->array.expr = $2;
        node->common.print_node = print_array;
        $$ = node;
    }
	| expression '[' expression endianflag ']' {
		tree_t* node = (tree_t*)create_node("bit_slicing_expr", BIT_SLIC_EXPR_TYPE, sizeof(struct tree_bit_slic));
		node->bit_slic.expr = $1;
		node->bit_slic.bit = $3;
		node->bit_slic.endian = $4;
		node->common.print_node = print_bit_slic;
		$$ = node;
	}
	| expression '[' expression ':' expression endianflag ']' {
		tree_t* node = (tree_t*)create_node("bit_slicing_expr", BIT_SLIC_EXPR_TYPE, sizeof(struct tree_bit_slic));
		node->bit_slic.expr = $1;
		node->bit_slic.bit = $3;
		node->bit_slic.bit_end = $5;
		node->bit_slic.endian = $6;
		node->common.print_node = print_bit_slic;
		$$ = node;
	}
	;

endianflag
	: ',' IDENTIFIER {
		tree_t* node = (tree_t*)create_node("endianflag", IDENT_TYPE, sizeof(struct tree_ident));
		node->ident.str = $2;
		node->ident.len = strlen($2);
		node->common.print_node = print_ident;
		$$ = node;
	}
	| {
		$$ = NULL;
	}
	;

expression_opt
	: expression {
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	;

comma_expression_opt
	: comma_expression {
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	;

expression_list
	: {
		$$ = NULL;
	}
	| expression {
		$$ = $1;
	}
	| expression ',' expression_list {
		if ($3 != NULL) {
			create_node_list($1, $3);
		}
		$$ = $1;
	}
	;

if_statment
	: IF '(' expression ')' {
		tree_t* node = (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		parse_expression($3, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>5;
		node->if_else.if_block = $6;
		node->common.print_node = print_if_else;
		current_table = pop(table_stack);
		$$ = node;
	}

statement
	: compound_statement {
		$$ = $1;
	}
	| local {
		$$ = $1;
		parse_local_decl($1, current_table);
	}
	| ';' {
		$$ = NULL;
	}
	| expression ';'{
		DBG("expression in statement\n");
		parse_expression($1, current_table);
		$$ =  $1;
	}
	| if_statment
	| if_statment ELSE {
		tree_t* node = $1;

		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>1;
		node->if_else.else_block = $4;
		current_table = pop(table_stack);
		$$ = node;
	}
	| WHILE '(' expression ')' {
		tree_t* node = (tree_t*)create_node("do_while", DO_WHILE_TYPE, sizeof(struct tree_do_while));
		node->do_while.cond = $3;
		parse_expression($3, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, DO_WHILE_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>5;
		node->do_while.block = $6;
		node->common.print_node = print_while;
		current_table = pop(table_stack);
		$$ = node;
	}
	| DO {
		tree_t* node = (tree_t*)create_node("do_while", DO_WHILE_TYPE, sizeof(struct tree_do_while));
		node->do_while.have_do = 1;

		current_table = change_table(current_table, table_stack, &current_table_num, DO_WHILE_TYPE);

		$<tree_type>$ = node;
	}
	statement WHILE '(' expression ')' ';' {
		tree_t* node = $<tree_type>2;
		node->do_while.cond = $6;
		node->do_while.block = $3;
		node->common.print_node = print_do_while;
		parse_expression($6, current_table);

		current_table = pop(table_stack);

		$$ = node;
	}
	| FOR '(' comma_expression_opt ';' expression_opt ';' comma_expression_opt ')' {
		tree_t* node = (tree_t*)create_node("for", FOR_TYPE, sizeof(struct tree_for));
		node->for_tree.init = $3;
		node->for_tree.cond = $5;
		node->for_tree.update = $7;
		node->common.print_node = print_for;
		parse_comma_expression($3, current_table);
		parse_expression($5, current_table);
		parse_comma_expression($7, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, FOR_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>9;
		node->for_tree.block = $10;

		current_table = pop(table_stack);

		$$ = node;
	}
	| SWITCH '(' expression ')' {
		tree_t* node = (tree_t*)create_node("switch", SWITCH_TYPE, sizeof(struct tree_switch));
		node->switch_tree.cond = $3;
		parse_expression($3, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, SWITCH_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>5;
		node->switch_tree.block = $6;
		node->common.print_node = print_switch;

		current_table = pop(table_stack);
		$$ = node;
	}
	| DELETE expression ';' {
		tree_t* node = (tree_t*)create_node("delete", DELETE_TYPE, sizeof(struct tree_delete));
		node->delete_tree.expr = $2;
		node->common.print_node = print_delete;
		parse_expression($2, current_table);
		$$ = node;
	}
	| TRY {
		try_catch_attr_t* attr = (try_catch_attr_t*)gdml_zmalloc(sizeof(try_catch_attr_t));

		tree_t* node = (tree_t*)create_node("try_catch", TRY_CATCH_TYPE, sizeof(struct tree_try_catch));
		node->common.attr = attr;

		current_table = change_table(current_table, table_stack, &current_table_num, TRY_CATCH_TYPE);
		attr->try_table = current_table;

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>2;
		node->try_catch.try_block = $3;
		current_table = pop(table_stack);
		$<tree_type>$ = node;
	}
	CATCH {
		tree_t* node = $<tree_type>4;
		try_catch_attr_t* attr = node->common.attr;

		current_table = change_table(current_table, table_stack, &current_table_num, TRY_CATCH_TYPE);
		attr->catch_table = current_table;

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>6;
		node->try_catch.catch_block = $7;
		node->common.print_node = print_try_catch;
		current_table = pop(table_stack);
		DBG(" try catch in statement\n");
		$$ = node;
	}
	| AFTER '(' expression ')' CALL expression ';' {
		tree_t* node = (tree_t*)create_node("after_call", AFTER_CALL_TYPE, sizeof(struct tree_after_call));
		node->after_call.cond = $3;
		node->after_call.call_expr = $6;
		node->common.print_node = print_after_call;
		parse_expression($3, current_table);
		parse_expression($6, current_table);
		DBG("AFTER CALL statement\n");
		$$ = node;
	}
	| CALL expression returnargs ';' {
		tree_t* node = (tree_t*)create_node("call", CALL_TYPE, sizeof(struct tree_call_inline));
		node->call_inline.expr = $2;
		node->call_inline.ret_args = $3;
		node->common.print_node = print_call_inline;
		parse_expression($2, current_table);
		DBG("CALL statement\n");
		$$ = node;
	}
	| INLINE expression returnargs ';' {
		tree_t* node = (tree_t*)create_node("inline", INLINE_TYPE, sizeof(struct tree_call_inline));
		node->call_inline.expr = $2;
		node->call_inline.ret_args = $3;
		node->common.print_node = print_call_inline;
		parse_expression($2, current_table);
		DBG("inline statement\n");
		$$ = node;
	}
	| ASSERT expression ';' {
		/* TODO: we should find the identifier from symbol table */
		tree_t* node = (tree_t*)create_node("assert", ASSERT_TYPE, sizeof(struct tree_assert));
		node->assert_tree.expr = $2;
		node->common.print_node = print_assert;
		parse_expression($2, current_table);
		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ',' expression ':' STRING_LITERAL ',' log_args ';' {
		DBG("In LOG statement: %s\n", $8);
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log));
		node->log.log_type = $2;
		node->log.level = $4;
		node->log.group = $6;
		node->log.format = $8;
		node->log.args = $10;

		parse_expression($4, current_table);
		parse_expression($6, current_table);
		parse_log_args($10, current_table);

		node->log.log_info = parse_log(node);
		node->common.print_node = print_log;

		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ':' STRING_LITERAL log_args ';' {
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log));
		node->log.log_type = $2;
		node->log.level = $4;
		node->log.format = $6;
		node->log.args = $7;

		parse_expression($4, current_table);
		parse_log_args($7, current_table);

		node->log.log_info = parse_log(node);
		node->common.print_node = print_log;

		$$ = node;
	}
	| LOG STRING_LITERAL ':' STRING_LITERAL log_args ';' {
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log));
		node->log.log_type = $2;
		node->log.format = $4;
		node->log.args = $5;

		parse_log_args($5, current_table);

		node->log.log_info = parse_log(node);
		node->common.print_node = print_log;

		$$ = node;
	}
	| SELECT ident IN '(' expression ')' WHERE '(' expression ')' {
		tree_t* node = (tree_t*)create_node("select", SELECT_TYPE, sizeof(struct tree_select));
		node->select.ident = $2;
		node->select.in_expr = $5;
		node->select.cond = $9;
		node->common.print_node = print_select;
		parse_expression($5, current_table);
		parse_expression($9, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, SELECT_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>11;
		node->select.where_block = $12;
		current_table = pop(table_stack);
		$<tree_type>$ = node;
	}
	ELSE {
		current_table = change_table(current_table, table_stack, &current_table_num, SELECT_TYPE);
		$<tree_type>$ = NULL;
	}
	statement {
		tree_t* node = $<tree_type>11;
		node->select.else_block = $16;
		current_table = pop(table_stack);
		$$ = node;
	}
	| FOREACH ident IN '(' expression ')' {
		foreach_attr_t* attr = (foreach_attr_t*)gdml_zmalloc(sizeof(foreach_attr_t));
		attr->ident = $2->ident.str;

		current_table = change_table(current_table, table_stack, &current_table_num, FOREACH_TYPE);
		attr->table = current_table;

		symbol_insert(current_table, $2->ident.str, FOREACH_TYPE, attr);

		tree_t* node = (tree_t*)create_node("foreach", FOREACH_TYPE, sizeof(struct tree_foreach));
		node->foreach.ident = $2;
		node->foreach.in_expr = $5;
		node->common.print_node = print_foreach;
		node->common.attr = attr;

		attr->common.node = node;
		DBG("FOREACH in statement\n");
		parse_expression($5, current_table);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>7;
		node->foreach.block = $8;
		current_table = pop(table_stack);
		$$ = node;
	}
	| ident ':' statement {
		$$ = NULL;
	}
	| CASE expression ':' {
		tree_t* node = (tree_t*)create_node("case", CASE_TYPE, sizeof(struct tree_case));
		/* TODO: charge the break */
		node->case_tree.expr = $2;
		parse_expression($2, current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, CASE_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>4;
		node->case_tree.block = $5;
		current_table = pop(table_stack);
		$$ = node;
	}
	| DEFAULT ':' {
		tree_t* node = (tree_t*)create_node("default", DEFAULT_TYPE, sizeof(struct tree_default));

		current_table = change_table(current_table, table_stack, &current_table_num, DEFAULT_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>3;
		node->default_tree.block = $4;
		node->common.print_node = print_default;
		current_table = pop(table_stack);
		$$ = node;
	}
	| GOTO ident ';' {
		tree_t* node = (tree_t*)create_node("goto", GOTO_TYPE, sizeof(struct tree_goto));
		node->goto_tree.label = $2;
		node->common.print_node = print_goto;
		$$ = node;
	}
	| BREAK ';' {
		tree_t* node = (tree_t*)create_node("break", BREAK_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_break;
		$$ = node;
	}
	| CONTINUE ';' {
		tree_t* node = (tree_t*)create_node("continue", CONTINUE_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_continue;
		$$ = node;
	}
	| THROW ';' {
		tree_t* node = (tree_t*)create_node("throw", THROW_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_throw;
		$$ = node;
	}
	| RETURN ';' {
		tree_t* node = (tree_t*)create_node("return", RETURN_TYPE, sizeof(struct tree_common));
		node->common.print_node = print_return;
		$$ = node;
	}
	| ERROR ';' {
		tree_t* node = (tree_t*)create_node("error", ERROR_TYPE, sizeof(struct tree_error));
		node->error.str = NULL;
		node->common.print_node = print_error;
		$$ = node;
	}
	| ERROR STRING_LITERAL ';' {
		tree_t* node = (tree_t*)create_node("error", ERROR_TYPE, sizeof(struct tree_error));
		node->error.str = $2;
		node->common.print_node = print_error;
		$$ = node;
	}
	;

log_args
	: {
		$$ = NULL;
	}
	| log_args ',' expression {
		if ($1 == NULL) {
			$$ = $3;
		}
		else {
			create_node_list($1, $3);
			$$ = $1;
		}
	}
	| expression {
		$$ = $1;
	}
	;

compound_statement
	: '{' statement_list '}' {
		tree_t* node = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block));
		node->block.statement = $2;
		$$ = node;
	}
	| '{' '}' {
		tree_t* node = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block));
		node->block.statement = NULL;
		$$ = NULL;
	}
	;

statement_list
	: statement {
		$$ = $1;
	}
	| statement_list statement {
		if (($1 == NULL)) {
			$$ = $2;
		}
		else {
			create_node_list($1, $2);
			$$ = $1;
		}
	}
	;

local_keyword
	: LOCAL {
		tree_t* node = (tree_t*)create_node("local_keyword", LOCAL_KEYWORD_TYPE, sizeof(struct tree_local_keyword));
		node->local_keyword.name = strdup("local");
		node->common.print_node = print_local_keyword;
		$$ = node;
	}
	| AUTO {
		tree_t* node = (tree_t*)create_node("local_keyword", LOCAL_KEYWORD_TYPE, sizeof(struct tree_local_keyword));
		node->local_keyword.name = strdup("auto");
		node->common.print_node = print_local_keyword;
		$$ = node;
	}
	;

local
	: local_keyword cdecl ';' {
		tree_t* node = (tree_t*)create_node("local decl", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.local_keyword = $1;
		node->local_tree.cdecl = $2;
		node->common.print_node = print_local;
		$$ = node;
	}
	| STATIC cdecl ';' {
		DBG("In STATIC \n");
		tree_t* node = (tree_t*)create_node("local static", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.is_static = 1;
		node->local_tree.cdecl = $2;
		node->common.print_node = print_local;
		$$ = node;
	}
	| local_keyword cdecl '=' expression ';' {
		tree_t* node = (tree_t*)create_node("local assign", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.local_keyword = $1;
		node->local_tree.cdecl = $2;
		node->local_tree.expr = $4;
		node->common.print_node = print_local;
		$$ = node;
	}
	| STATIC cdecl '=' expression ';' {
		tree_t* node = (tree_t*)create_node("local static assign", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.is_static = 1;
		node->local_tree.cdecl = $2;
		node->local_tree.expr = $4;
		node->common.print_node = print_local;
		$$ = node;
	}
	;

objident_list
	: objident {
		$$ = $1;
	}
	| objident_list ',' objident {
		create_node_list($1, $3);
		$$ = $1;
	}
	;

maybe_objident
	: objident {
		$$ = $1;
	}
	|  {
		$$ = NULL;
	}
	;

objident
	:ident {
		DBG("ident: %s\n", $1->ident.str);
		$$ = $1;
	}
	| THIS {
		tree_t* node = (tree_t*)dml_keyword_node("this");
		node->ident.type = THIS_TYPE;
		$$ = node;
	}
	| REGISTER {
		tree_t* node = (tree_t*)dml_keyword_node("register");
		node->ident.type = REGISTER_TYPE;
		$$ = node;
	}
	| SIGNED {
		tree_t* node = (tree_t*)c_keyword_node("signed");
		node->ident.type = SIGNED_TYPE;
		$$ = node;
	}
	| UNSIGNED {
		tree_t* node = (tree_t*)c_keyword_node("unsigned");
		node->ident.type = UNSIGNED_TYPE;
		$$ = node;
	}
	;

ident
	: IDENTIFIER {
		tree_t* ident = (tree_t*)create_node("identifier", IDENT_TYPE, sizeof(struct tree_ident));
		ident->ident.str = $1;
		ident->ident.len = strlen($1);
		ident->common.print_node = print_ident;
		$$ = ident;
	}
	| ATTRIBUTE {
		tree_t* node = (tree_t*)dml_keyword_node("attribute");
		node->ident.type = ATTRIBUTE_TYPE;
		$$ = node;
	}
	| BANK {
		tree_t* node = (tree_t*)dml_keyword_node("bank");
		node->ident.type = BANK_TYPE;
		$$ = node;
	}
	| BITORDER {
		tree_t* node = (tree_t*)dml_keyword_node("bitorder");
		node->ident.type = BITORDER_TYPE;
		$$ = node;
	}
	| CONNECT {
		tree_t* node = (tree_t*)dml_keyword_node("connect");
		node->ident.type = CONNECT_TYPE;
		$$ = node;
	}
	| CONSTANT {
		tree_t* node = (tree_t*)dml_keyword_node("constant");
		node->ident.type = CONSTANT_TYPE;
		$$ = node;
	}
	| DATA {
		tree_t* node = (tree_t*)dml_keyword_node("data");
		node->ident.type = DATA_TYPE;
		$$ = node;
	}
	| DEVICE {
		tree_t* node = (tree_t*)dml_keyword_node("device");
		node->ident.type = DEVICE_TYPE;
		$$ = node;
	}
	| EVENT {
		tree_t* node = (tree_t*)dml_keyword_node("event");
		node->ident.type = EVENT_TYPE;
		$$ = node;
	}
	| FIELD {
		tree_t* node = (tree_t*)dml_keyword_node("field");
		node->ident.type = FIELD_TYPE;
		$$ = node;
	}
	| FOOTER {
		tree_t* node = (tree_t*)dml_keyword_node("footer");
		node->ident.type = FOOTER_TYPE;
		$$ = node;
	}
	| GROUP {
		tree_t* node = (tree_t*)dml_keyword_node("group");
		node->ident.type = GROUP_TYPE;
		$$ = node;
	}
	| IMPLEMENT {
		tree_t* node = (tree_t*)dml_keyword_node("implement");
		node->ident.type = IMPLEMENT_TYPE;
		$$ = node;
	}
	| IMPORT {
		tree_t* node = (tree_t*)dml_keyword_node("import");
		node->ident.type = IMPORT_TYPE;
		$$ = node;
	}
	| INTERFACE {
		tree_t* node = (tree_t*)dml_keyword_node("interface");
		node->ident.type = INTERFACE_TYPE;
		$$ = node;
	}
	| LOGGROUP {
		tree_t* node = (tree_t*)dml_keyword_node("loggroup");
		node->ident.type = LOGGROUP_TYPE;
		$$ = node;
	}
	| METHOD {
		tree_t* node = (tree_t*)dml_keyword_node("method");
		node->ident.type = METHOD_TYPE;
		$$ = node;
	}
	| PORT {
		tree_t* node = (tree_t*)dml_keyword_node("port");
		node->ident.type = PORT_TYPE;
		$$ = node;
	}
	| SIZE {
		tree_t* node = (tree_t*)dml_keyword_node("size");
		node->ident.type = SIZE_TYPE;
		$$ = node;
	}
	| CLASS {
		tree_t* node = (tree_t*)dml_keyword_node("class");
		node->ident.type = CLASS_TYPE;
		$$ = node;
	}
	| ENUM {
		tree_t* node = (tree_t*)dml_keyword_node("enum");
		node->ident.type = ENUM_TYPE;
		$$ = node;
	}
	| NAMESPACE {
		tree_t* node = (tree_t*)dml_keyword_node("namespace");
		node->ident.type = NAMESPACE_TYPE;
		$$ = node;
	}
	| PRIVATE {
		tree_t* node = (tree_t*)dml_keyword_node("private");
		node->ident.type = PRIVATE_TYPE;
		$$ = node;
	}
	| PROTECTED {
		tree_t* node = (tree_t*)dml_keyword_node("protected");
		node->ident.type = PROTECTED_TYPE;
		$$ = node;
	}
	| PUBLIC {
		tree_t* node = (tree_t*)dml_keyword_node("public");
		node->ident.type = PUBLIC_TYPE;
		$$ = node;
	}
	| RESTRICT {
		tree_t* node = (tree_t*)dml_keyword_node("register");
		node->ident.type = RESTRICT_TYPE;
		$$ = node;
	}
	| UNION {
		tree_t* node = (tree_t*)dml_keyword_node("union");
		node->ident.type = UNION_TYPE;
		$$ = node;
	}
	| USING {
		tree_t* node = (tree_t*)dml_keyword_node("using");
		node->ident.type = USING_TYPE;
		$$ = node;
	}
	| VIRTUAL {
		tree_t* node = (tree_t*)dml_keyword_node("virtual");
		node->ident.type = VIRTUAL_TYPE;
		$$ = node;
	}
	| VOLATILE {
		tree_t* node  = (tree_t*)dml_keyword_node("volatile");
		node->ident.type = VOLATILE_TYPE;
		$$ = node;
	}
	;

%%
#include <stdio.h>
#include <stdlib.h>

extern int column;
int lineno  = 1;    /* number of current source line */

void yyerror(yyscan_t* scanner, void* v, char *s) {
	fflush(stdout);
	fprintf(stderr,"Syntax error on line #%d, column #%d: %s\n", lineno, column, s);
	exit(1);
}
