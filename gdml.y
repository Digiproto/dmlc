%{
#include <stdio.h>
#include <assert.h>
#include "dmlc.h"
#include "types.h"
#include "symbol.h"
#include "stack.h"
#include "ast.h"
//#include "Parser.h"
#include "gen_common.h"
#include "gen_expression.h"
#include "info_output.h"
#define YYDEBUG 1
//const char* dir = "/opt/virtutech/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0/";

//#define PARSE_DEBUG
#ifdef PARSE_DEBUG
//#define DBG debug_black
//#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#define DBG(fmt, ...) do { \
		fprintf(stderr, \
				"[Debug]file: %s\n" \
				"       (line: %d, column: %d) to (line: %d, column: %d)\n" \
				"       info: ", \
				yylloc.file->name, \
				yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column); \
		fprintf(stderr, fmt, ## __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif
%}

%output  "Parser.c"
%defines "Parser.h"

%code requires {
#include "import.h"

struct file_stack* filestack_top;

#define YYLTYPE_IS_DECLARED 1

#define YYLLOC_DEFAULT(Current, Rhs, N)                                \
	do{                                                                \
		if (N)                                                         \
		{                                                              \
			(Current).first_line = YYRHSLOC (Rhs, 1).first_line;       \
			(Current).first_column = YYRHSLOC (Rhs, 1).first_column;   \
			(Current).last_line = YYRHSLOC (Rhs, N).last_line;         \
			(Current).last_column = YYRHSLOC (Rhs, N).last_column;     \
			(Current).file = YYRHSLOC (Rhs, 1).file;                   \
		}else{                                                         \
			(Current).first_line = (Current).last_line =               \
					YYRHSLOC (Rhs, 0).last_line;                       \
			(Current).first_column = (Current).last_column =           \
					YYRHSLOC (Rhs, 0).last_column;                     \
			(Current).file = YYRHSLOC (Rhs, 0).file;                   \
		}                                                              \
	}while(0)
}

%define api.pure

%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { tree_t** root_ptr }
%locations

%union  {
	int ival;
	char* sval;
	node_t* nodeval;
	int itype;
	tree_t* tree_type;
	//enum tree_code code;
	location_t location;
}

%{
#include "Lexer.h"
//extern int  yylex(YYSTYPE *yylval_param, yyscan_t yyscanner);
extern tree_t* parse_auto_api(void);
void yyerror(YYLTYPE* location, yyscan_t* scanner, tree_t** root_ptr, char *s);
extern tree_t* parse_file(const char* name);
extern int charge_standard_parameter(symtab_t table, parameter_attr_t* attr);
extern void insert_auto_defaut_param(symtab_t table);
extern void insert_array_index(object_attr_t* attr, symtab_t table);
extern char* builtin_filename;
extern symtab_t root_table;
stack_t* table_stack;
int object_spec_type = -1;
object_attr_t* object_comm_attr = NULL;
static symtab_t current_table = NULL;
static symtab_t prefix_table = NULL;
static long int current_table_num = 0;
tree_t* current_object_node = NULL;
%}

%token DML IDENTIFIER INTEGER_LITERAL FLOAT_LITERAL STRING_LITERAL SIZEOF
%token INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME REG_OFFSET

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token METHOD_RETURN RANGE_SIGN

%token PARAMETER BANK FIELD DATA CONNECT INTERFACE ATTRIBUTE EVENT GROUP
%token IN TEMPLATE HEADER FOOTER BODY LOGGROUP IMPORT SIZE LAYOUT BITFIELDS
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
%type	<tree_type> BODY
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
%type	<tree_type> if_statement
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
//			printf("root of ast already exists\n");
			PERROR("(inner) root of ast already exists", @$);
		}
		*root_ptr = (tree_t*)create_node("dml", DML_TYPE, sizeof(struct tree_dml), &@$);
		//printf("root_ptr: 0x%x, *root_ptr: 0x%x\n", root_ptr, *root_ptr);
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
		set_root_table(root_table);
		table_stack = initstack();
		push(table_stack, current_table);
#ifndef RELEASE
		printf("current_table_num: %ld\n", current_table_num);
#endif
		device_attr_t* attr = (device_attr_t*)malloc(sizeof(device_attr_t));
		attr->name = $2->ident.str;
		if (symbol_insert(root_table, $2->ident.str, DEVICE_TYPE, attr) == -1) {
//			fprintf(stderr, "Line = %d, redefined, device %s\n", __LINE__, $2->ident.str);
			PWARN("redefined device \"%s\"", @2, $2->ident.str);
		}

		tree_t* node = (tree_t*)create_node("device", DEVICE_TYPE, sizeof(struct tree_device), &@$);
		node->device.name = $2->ident.str;
		node->device.common.print_node = print_device;
		attr->common.node = node;
		node->common.attr = attr;

		/* parase the pre-import file */
		int i = 0;
		tree_t* import_ast = NULL;
		while(import_file_list[i] != NULL) {
#ifndef RELEASE
			printf("start parse file: %s\n", import_file_list[i]);
#endif
			import_ast = (tree_t*)get_ast(import_file_list[i]);
			if(import_ast->common.child != NULL) {
				create_node_list(node, import_ast->common.child);
			}
#ifndef RELEASE
			printf("finished parse file: %s\n", import_file_list[i]);
#endif
			i++;
		}

		$<tree_type>$ = node;
	}
	syntax_modifiers device_statements {
		if($5 != NULL)	{
#ifndef RELEASE
			printf("\ndevice list: name: %s\n\n", $5->common.name);
#endif
			create_node_list($<tree_type>4, $5);
		}
		if($6 != NULL) {
#ifndef RELEASE
			printf("\ndevice list: name: %s\n\n", $6->common.name);
#endif
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
//			printf("Line = %d, maybe something Wrong\n", __LINE__);
			PWARN("can't find syntax_modifiers and device_statements", @$);
			$$ = NULL;
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
		tree_t* node = (tree_t*)create_node("bitorder", BITORDER_TYPE, sizeof(struct tree_bitorder), &@$);
		node->bitorder.endian = $2->ident.str;
		node->common.print_node = print_bitorder;
		node->common.parse = parse_bitorder;
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
//			printf("maybe something wrong in device_statements\n");
			PWARN("can't find device_statements and device_statements", @$);
		}
	}
	;

device_statement
	: object_statement {
		$$ = $1;
	}
	| toplevel {
		$$ = $1;
	}
	| import {
		/* FIXME: there maybe some problems */
		$$ = $1;
	}
	;

object
	: BANK maybe_objident istemplate {
        object_attr_t* attr = create_object(current_table, "bank", $2->ident.str, BANK_TYPE,
                                            sizeof(struct tree_bank), sizeof(bank_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        } else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->bank.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                symbol_insert(current_table, $2->ident.str, BANK_TYPE, attr);
            }
            node->bank.templates = create_template_list(node->bank.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->bank.templates);
            attr->common.templates = get_templates(attr->common.templates, node->bank.templates, attr->common.templates_num);

            object_spec_type = BANK_TYPE;
            object_comm_attr = attr;

            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->bank.spec = get_obj_spec(node->bank.spec, $5);
        node->common.print_node = print_bank;

        bank_attr_t* attr = (bank_attr_t*)($<tree_type>4->common.attr);
        attr->common.desc = get_obj_desc(node->bank.spec);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = $<tree_type>4;
	}
	| REGISTER objident sizespec offsetspec istemplate {
        DBG("register is %s\n", $2->ident.str);
        object_attr_t* attr = create_object(current_table, "register", $2->ident.str, REGISTER_TYPE,
                                            sizeof(struct tree_register), sizeof(register_attr_t), &@$);
        if (attr == NULL) {
            $<tree_type>$ = NULL;
        } else {
            tree_t* node = attr->common.node;
            /* the register is defined first time*/
            if (attr->common.is_defined == 0) {
                node->reg.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->reg.is_array = 0;
                symbol_insert(current_table, $2->ident.str, REGISTER_TYPE, attr);
            }

            node->reg.sizespec = get_obj_default_param(node->reg.sizespec, $3, "size");
            node->reg.offset = get_obj_default_param(node->reg.offset, $4, "offset");
            node->reg.templates = create_template_list(node->reg.templates, $5);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->reg.templates);
            attr->common.templates = get_templates(attr->common.templates, node->reg.templates, attr->common.templates_num);

            object_spec_type = REGISTER_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>6;
        node->reg.spec = get_obj_spec(node->reg.spec, $7);
        node->common.print_node = print_register;

        register_attr_t* attr = (register_attr_t*)(node->common.attr);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| REGISTER objident '[' arraydef ']' sizespec offsetspec istemplate {
		object_attr_t* attr = create_object(current_table, "register", $2->ident.str, REGISTER_TYPE,
                                            sizeof(struct tree_register), sizeof(register_attr_t), &@$);
        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->reg.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->reg.is_array = 1;
                symbol_insert(current_table, $2->ident.str, REGISTER_TYPE, attr);
            }
            node->reg.array = get_obj_default_param(node->reg.array, $4, "array");
            node->reg.sizespec = get_obj_default_param(node->reg.sizespec, $6, "size");
            node->reg.offset = get_obj_default_param(node->reg.offset, $7, "offset");
            node->reg.templates = create_template_list(node->reg.templates, $8);
            current_object_node = node;
            attr->common.templates_num = get_list_num(node->reg.templates);
            attr->common.templates = get_templates(attr->common.templates, node->reg.templates, attr->common.templates_num);


            object_spec_type = REGISTER_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>9;
        node->reg.spec = get_obj_spec(node->reg.spec, $10);
        node->common.print_node = print_register;
        register_attr_t* attr = (register_attr_t*)(node->common.attr);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| FIELD objident bitrange istemplate {
        object_attr_t* attr = create_object(current_table, "field", $2->ident.str, FIELD_TYPE,
                                            sizeof(struct tree_field), sizeof(field_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->field.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->field.is_range = 1;
                symbol_insert(current_table, $2->ident.str, FIELD_TYPE, attr);
            }
            node->field.bitrange = get_obj_default_param(node->field.bitrange, $3, "bitrange");
            node->field.templates = create_template_list(node->field.templates, $4);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->field.templates);
            attr->common.templates = get_templates(attr->common.templates, node->field.templates, attr->common.templates_num);

            object_spec_type = FIELD_TYPE;
            object_comm_attr = attr;

            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>5;
        node->field.spec = get_obj_spec(node->field.spec, $6);
        node->common.print_node = print_field;

        field_attr_t* attr = (field_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->field.spec);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| FIELD objident istemplate {
        object_attr_t* attr = create_object(current_table, "field", $2->ident.str, FIELD_TYPE,
                                            sizeof(struct tree_field), sizeof(field_attr_t), &@$);
        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->field.name = $2->ident.str;
                node->field.bitrange = NULL;

                attr->common.name = $2->ident.str;
                attr->field.is_range = 0;
                symbol_insert(current_table, $2->ident.str, FIELD_TYPE, attr);
            }
            node->field.templates = create_template_list(node->field.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->field.templates);
            attr->common.templates = get_templates(attr->common.templates, node->field.templates, attr->common.templates_num);
            attr->common.obj_type = FIELD_TYPE;

            object_spec_type = FIELD_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->field.spec = get_obj_spec(node->field.spec, $5);
        node->common.print_node = print_field;

        field_attr_t* attr = (field_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->field.spec);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| DATA cdecl ';' {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_data = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		node->common.parse = parse_data;
		$$ = node;
	}
	| CONNECT objident istemplate {
        object_attr_t* attr = create_object(current_table, "connect", $2->ident.str, CONNECT_TYPE,
                                            sizeof(struct tree_connect), sizeof(connect_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->connect.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->connect.is_array = 0;
                symbol_insert(current_table, $2->ident.str, CONNECT_TYPE, attr);
            }
            node->connect.templates = create_template_list(node->connect.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->connect.templates);
            attr->common.templates = get_templates(attr->common.templates, node->connect.templates, attr->common.templates_num);

            object_spec_type = CONNECT_TYPE;
            object_comm_attr = attr;
            DBG("CONNECT_TYPE: %s\n", $2->ident.str);
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->connect.spec = get_obj_spec(node->connect.spec, $5);
        node->common.print_node = print_connect;

        connect_attr_t* attr = (connect_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->connect.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| INTERFACE objident istemplate {
        object_attr_t* attr = create_object(current_table, "interface", $2->ident.str, INTERFACE_TYPE,
                                            sizeof(struct tree_interface), sizeof(interface_attr_t), &@$);
        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->interface.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                symbol_insert(current_table, $2->ident.str, INTERFACE_TYPE, attr);
            }
            node->interface.templates = create_template_list(node->interface.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->interface.templates);
            attr->common.templates = get_templates(attr->common.templates, node->interface.templates, attr->common.templates_num);

            object_spec_type = INTERFACE_TYPE;
            object_comm_attr = attr;
            DBG("Interface_type: %s\n", $2->ident.str);
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->interface.spec = get_obj_spec(node->interface.spec, $5);
        node->common.print_node = print_interface;

        interface_attr_t* attr = (interface_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->interface.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| ATTRIBUTE objident istemplate {
        object_attr_t* attr = create_object(current_table, "attribute", $2->ident.str, ATTRIBUTE_TYPE,
                                            sizeof(struct tree_attribute), sizeof(attribute_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->attribute.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->attribute.arraydef = 0;
                symbol_insert(current_table, $2->ident.str, ATTRIBUTE_TYPE, attr);
            }
            node->attribute.templates = create_template_list(node->attribute.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->attribute.templates);
            attr->common.templates =  get_templates(attr->common.templates, node->attribute.templates, attr->common.templates_num);

            object_spec_type = ATTRIBUTE_TYPE;
            object_comm_attr = attr;
            DBG("Attribute: %s\n", $2->ident.str);
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->attribute.spec = get_obj_spec(node->attribute.spec, $5);
        node->common.print_node = print_attribute;

        attribute_attr_t* attr = (attribute_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->attribute.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| EVENT objident istemplate {
        object_attr_t* attr = create_object(current_table, "event", $2->ident.str, EVENT_TYPE,
                                                sizeof(struct tree_event), sizeof(event_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->event.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                symbol_insert(current_table, $2->ident.str, EVENT_TYPE, attr);
            }
            node->event.templates = create_template_list(node->event.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->event.templates);
            attr->common.templates = get_templates(attr->common.templates, node->event.templates, attr->common.templates_num);

            object_spec_type = EVENT_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->event.spec = get_obj_spec(node->event.spec, $5);
        node->common.print_node = print_event;

        event_attr_t* attr = (event_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->event.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        $$ = node;
	}
	| GROUP objident istemplate {
        object_attr_t* attr = create_object(current_table, "group", $2->ident.str, GROUP_TYPE,
                                            sizeof(struct tree_group), sizeof(group_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->group.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                symbol_insert(current_table, $2->ident.str, GROUP_TYPE, attr);
            }
            node->group.templates = create_template_list(node->group.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->group.templates);
            attr->common.templates = get_templates(attr->common.templates, node->group.templates, attr->common.templates_num);

            object_spec_type = GROUP_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->group.spec = get_obj_spec(node->group.spec, $5);
        node->common.print_node = print_group;

        group_attr_t* attr = (group_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->group.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| PORT objident istemplate {
        object_attr_t* attr = create_object(current_table, "port", $2->ident.str, PORT_TYPE,
                                            sizeof(struct tree_port), sizeof(port_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->port.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                symbol_insert(current_table, $2->ident.str, IMPLEMENT_TYPE, attr);
            }
            node->port.templates = create_template_list(node->port.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->port.templates);
            attr->common.templates = get_templates(attr->common.templates, node->port.templates, attr->common.templates_num);

            object_spec_type = PORT_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
       tree_t* node = $<tree_type>4;
        node->port.spec = get_obj_spec(node->port.spec, $5);
        node->common.print_node = print_port;

        port_attr_t* attr = (port_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->port.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| IMPLEMENT objident istemplate {
        object_attr_t* attr = create_object(current_table, "implement", $2->ident.str, IMPLEMENT_TYPE,
                                            sizeof(struct tree_implement), sizeof(implement_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->implement.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                symbol_insert(current_table, $2->ident.str, IMPLEMENT_TYPE, attr);
            }
            node->implement.templates = create_template_list(node->implement.templates, $3);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->implement.templates);
            attr->common.templates = get_templates(attr->common.templates, node->implement.templates, attr->common.templates_num);

            object_spec_type = IMPLEMENT_TYPE;
            object_comm_attr = attr;
            DBG("objident: %s\n", $2->ident.str);
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>4;
        node->implement.spec = get_obj_spec(node->implement.spec, $5);
        node->common.print_node = print_implement;

        implement_attr_t* attr = (implement_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->implement.spec);
        //attr->common.table = get_obj_block_table($5);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| ATTRIBUTE objident '[' arraydef ']' istemplate {
        object_attr_t* attr = create_object(current_table, "attribute", $2->ident.str, ATTRIBUTE_TYPE,
                                            sizeof(struct tree_attribute), sizeof(attribute_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined) {
                node->attribute.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->attribute.is_array = 1;
                symbol_insert(current_table, $2->ident.str, ATTRIBUTE_TYPE, attr);
            }
            node->attribute.arraydef = get_obj_default_param(node->attribute.arraydef, $4, "array");
            node->attribute.templates = create_template_list(node->attribute.templates, $6);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->attribute.templates);
            attr->common.templates = get_templates(attr->common.templates, node->attribute.templates, attr->common.templates_num);

            object_spec_type = ATTRIBUTE_TYPE;
            object_comm_attr = attr;

            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>7;
        node->attribute.spec = get_obj_spec(node->attribute.spec, $8);
        node->common.print_node = print_attribute;

        attribute_attr_t* attr = (attribute_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->attribute.spec);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| GROUP objident '[' arraydef ']' istemplate {
        object_attr_t* attr = create_object(current_table, "group", $2->ident.str, GROUP_TYPE,
                                            sizeof(struct tree_group), sizeof(group_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->group.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->group.is_array = 1;
            }
            node->group.array = get_obj_default_param(node->group.array, $4, "array");
            node->group.templates = create_template_list(node->group.templates, $6);
            current_object_node = node;

            //attr->group.arraydef = get_arraydef(&($4), current_table);
            attr->common.templates_num = get_list_num(node->group.templates);
            attr->common.templates = get_templates(attr->common.templates, node->group.templates, attr->common.templates_num);

            object_spec_type = GROUP_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>7;
        node->group.spec = get_obj_spec(node->group.spec, $8);
        node->common.print_node = print_group;

        group_attr_t* attr = (group_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->group.spec);
        //attr->common.table = get_obj_block_table($8);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| PORT objident '[' arraydef ']' istemplate {
        object_attr_t* attr = create_object(current_table, "port", $2->ident.str, PORT_TYPE,
                                            sizeof(struct tree_port), sizeof(port_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->port.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->port.is_array = 1;
                symbol_insert(current_table, $2->ident.str, IMPLEMENT_TYPE, attr);
            }
            node->port.array = get_obj_default_param(node->port.array, $4, "array");
            node->port.templates = create_template_list(node->port.templates, $6);
            current_object_node = node;

            attr->common.templates_num = get_list_num(node->port.templates);
            attr->common.templates = get_templates(attr->common.templates, node->port.templates, attr->common.templates_num);
            //attr->port.arraydef = get_arraydef(&($4), current_table);

            object_spec_type = PORT_TYPE;
            object_comm_attr = attr;
            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>7;
        node->port.spec = get_obj_spec(node->port.spec, $8);
        node->common.print_node = print_port;

        port_attr_t* attr = (port_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->port.spec);
        //attr->common.table = get_obj_block_table($8);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	| CONNECT objident '[' arraydef ']' istemplate {
       object_attr_t* attr = create_object(current_table, "connect", $2->ident.str, CONNECT_TYPE,
                                            sizeof(struct tree_connect), sizeof(connect_attr_t), &@$);

        if (attr == NULL) {
            $<tree_type>$ = NULL;
        }
        else {
            tree_t* node = attr->common.node;
            if (attr->common.is_defined == 0) {
                node->connect.name = $2->ident.str;

                attr->common.name = $2->ident.str;
                attr->connect.is_array = 1;
                symbol_insert(current_table, $2->ident.str, CONNECT_TYPE, attr);
            }
            node->connect.arraydef = get_obj_default_param(node->connect.arraydef, $4, "array");
            node->connect.templates = create_template_list(node->connect.templates, $6);
            current_object_node = node;

            //attr->connect.arraydef = get_arraydef(&($4), current_table);
            attr->common.templates_num = get_list_num(node->connect.templates);
            attr->common.templates = get_templates(attr->common.templates, node->connect.templates, attr->common.templates_num);

            object_spec_type = CONNECT_TYPE;
            object_comm_attr = attr;

            $<tree_type>$ = node;
        }
	}
	object_spec {
        tree_t* node = $<tree_type>7;
        node->connect.spec = get_obj_spec(node->connect.spec, $8);
        node->common.print_node = print_connect;

        connect_attr_t* attr = (connect_attr_t*)(node->common.attr);
        attr->common.desc = get_obj_desc(node->connect.spec);
        //attr->common.table = get_obj_block_table($8);
        object_spec_type = -1;
        object_comm_attr = NULL;
        $$ = node;
	}
	;

method
	: METHOD objident method_params {
		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method), &@$);
		node->method.name = $2->ident.str;
		node->method.is_extern = 0;
		node->method.is_default = 0;
		node->method.params = $3;
		node->common.print_node = print_method;
		node->common.parse = parse_method;
		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		$<tree_type>$ = node;
	}
	compound_statement {
		tree_t* node = $<tree_type>4;
		node->method.block = $5;
		current_table = pop(table_stack);
		$$ = node;
	}
	| METHOD objident method_params DEFAULT {
		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method), &@$);
		node->method.name = $2->ident.str;
		node->method.is_extern = 0;
		node->method.is_default = 1;
		node->method.params = $3;
		node->common.print_node = print_method;
		node->common.parse = parse_method;
		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		$<tree_type>$ = node;
	}
	compound_statement {
		tree_t* node = $<tree_type>5;
		node->method.block = $6;
		current_table = pop(table_stack);
		$$ = node;
	}
	| METHOD EXTERN objident method_params {
		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method), &@$);
		node->method.name = $3->ident.str;
		node->method.is_extern = 1;
		node->method.is_default = 0;
		node->method.params = $4;
		node->common.print_node = print_method;
		node->common.parse = parse_method;
		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
		$<tree_type>$ = node;
	}
	compound_statement {
		tree_t* node = $<tree_type>5;
		node->method.block = $6;
		current_table = pop(table_stack);
		$$ = node;
	}
	| METHOD EXTERN objident method_params DEFAULT {
		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method), &@$);
		node->method.name = $3->ident.str;
		node->method.is_extern = 1;
		node->method.is_default = 1;
		node->method.params = $4;
		node->common.print_node = print_method;
		node->common.parse = parse_method;
		current_table = change_table(current_table, table_stack, &current_table_num, METHOD_TYPE);
        $<tree_type>$ = node;
	}
	compound_statement {
        tree_t* node = $<tree_type>6;
        node->method.block = $7;
        current_table = pop(table_stack);
		$$ = node;
	}
	;

arraydef
	: expression {
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array), &@$);
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
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array), &@$);
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
		template_attr_t* attr = (template_attr_t*)gdml_zmalloc(sizeof(template_attr_t));
		attr->name = $2->ident.str;
		if (symbol_defined(current_table, $2->ident.str))
			error("name collision on '%s'\n", $2->ident.str);
		symbol_insert(current_table, $2->ident.str, TEMPLATE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("template", TEMPLATE_TYPE, sizeof(struct tree_template), &@$);
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
        node->temp.spec = get_obj_spec(node->temp.spec, $4);
        node->common.print_node = print_template;

        template_attr_t* attr = (template_attr_t*)(node->common.attr);
        attr->desc = get_obj_desc(node->temp.spec);
        attr->table = get_obj_block_table($4);
        object_spec_type = -1;
        $$ = node;
	}
	| LOGGROUP ident ';' {
		tree_t* node = (tree_t*)create_node("loggroup", LOGGROUP_TYPE, sizeof(struct tree_loggroup), &@$);
		node->loggroup.name = $2->ident.str;
		node->common.print_node = print_loggroup;
		node->common.parse = parse_loggroup;
		$$ = node;
	}
	| CONSTANT ident '=' expression ';' {
		tree_t* node = (tree_t*)create_node("assign", ASSIGN_TYPE, sizeof(struct tree_assign), &@$);
		node->assign.is_constant = 1;
		node->assign.decl = $2;
		node->assign.expr = $4;
		node->common.print_node = print_assign;
		node->common.parse = parse_constant;
		$$ = node;
	}
	| EXTERN cdecl_or_ident ';' {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_extern = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		node->common.parse = parse_extern_decl;
		$$ = node;
	}
	| TYPEDEF cdecl ';' {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_typedef = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		node->common.parse = parse_typedef;

		$$ = node;
	}
	| EXTERN TYPEDEF cdecl ';' {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_extern = 1;
		node->cdecl.is_typedef = 1;
		node->cdecl.decl = $3;
		node->common.print_node = print_cdecl;
		node->common.parse = parse_typedef;

		$$ = node;
	}
	| STRUCT ident '{' {
		current_table = change_table(current_table, table_stack, &current_table_num, STRUCT_TYPE);
		tree_t* node = (tree_t*)create_node("struct", STRUCT_TYPE, sizeof(struct tree_struct), &@$);
		node->struct_tree.name = $2->ident.str;
		node->struct_tree.table = current_table;
		node->common.print_node = print_struct;
		node->common.parse = parse_struct_top;
		$<tree_type>$ = node;
	}
	struct_decls '}' {
		tree_t* node = $<tree_type>4;
		node->struct_tree.block = $5;
		current_table = pop(table_stack);
		$$ = node;
	}
	| HEADER BODY {
		/* FIXME: the header include much content */
		tree_t* node = (tree_t*)create_node("header", HEADER_TYPE, sizeof(struct tree_head), &@$);
		node->head.str = (char*)($2);
		node->common.print_node = print_header;
		/* TODO: should analyze the content of head */
		//node->head.head = $1;
		$$ = node;
	}
	| FOOTER BODY {
		/* FIXME: the header include much content */
		tree_t* node = (tree_t*)create_node("footer", FOOTER_TYPE, sizeof(struct tree_foot), &@$);
		node->head.str = (char*)($2);
		node->common.print_node = print_footer;
		/* TODO: should analyze the content of foot */
		//node->head.head = $1;
		$$ = node;
	}
	;

istemplate_stmt
	: IS objident ';' {
		DBG("In IS statement\n");
		add_template_to_table(current_table, $2->ident.str);
		print_templates(current_table);
		$$ = $2;
	}
	;

import
	: IMPORT STRING_LITERAL ';' {
		DBG("import file is %s\n", $2);

		/* remove \" character. */
		char filename[DIR_MAX_LEN];
		int rt = snprintf(filename, DIR_MAX_LEN, "%s", ($2 + 1));
		if(rt >= DIR_MAX_LEN) {
			PERROR("import file name too long (>= %d).", @2, DIR_MAX_LEN);
		}
		filename[rt - 1] = '\0';

		/* I replace with the parse_file function.
		 * Because import file from multiple directory according to priority.
		 * In other hand linking directory and file name is different on windows or Linux. */
		tree_t* ast = parse_file(filename);

		import_attr_t* attr = (import_attr_t*)gdml_zmalloc(sizeof(import_attr_t));
		attr->file = $2;
		attr->table = current_table;
		attr->common.node = ast->common.child;
		attr->common.table_num = current_table->table_num;
		//printf("file: %s, table_num: %d\n", attr->file, attr->common.table_num);
		/* FIXME, should check if the same filename is imported already. */
		symbol_insert(current_table, $2, IMPORT_TYPE, attr);

		$$ = ast->common.child;
	}
	;

object_desc
	: STRING_LITERAL {
		tree_t* node = (tree_t*)create_node("const_string", CONST_STRING_TYPE, sizeof(struct tree_string), &@$);
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
			tree_t* node = (tree_t*)create_node("object_spec", SPEC_TYPE, sizeof(struct tree_object_spec), &@$);
			node->spec.desc = $1;
			node->spec.block = NULL;
			node->common.print_node = print_object_spec;
			$$ = node;
		}
	}
	| object_desc '{' {
		DBG("object_statements for object_spec\n");
		tree_t* node = (tree_t*)create_node("object_spec", SPEC_TYPE, sizeof(struct tree_object_spec), &@$);
		node->spec.desc = $1;

		tree_t* block = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block), &@$);
		symtab_t table = NULL;
		/* only template and object have the object_spec, and object has it own object_comm_attr
			the template do not have object_comm_attr */
		if (object_comm_attr == NULL) {
			table = symtab_create(TEMPLATE_TYPE);
			table->no_check = 1;
			table->table_num = ++current_table_num;
			block->block.table = symtab_insert_child(current_table, table);
		}
		else if (object_comm_attr->common.obj_type == IMPLEMENT_TYPE) {
			table = symtab_create(IMPLEMENT_TYPE);
			table->no_check = 1;
			table->table_num = ++current_table_num;
			block->block.table = symtab_insert_child(current_table, table);
		}
		else if (object_comm_attr->common.table){
			table = object_comm_attr->common.table;
			block->block.table = table;
		}
		else {
			table = symtab_create(object_comm_attr->common.obj_type);
			table->table_num = ++current_table_num;
			block->block.table = symtab_insert_child(current_table, table);
		}
		push(table_stack, current_table);
		current_table = block->block.table;
		if ((object_comm_attr) && (object_comm_attr->common.table == NULL)) {
			object_comm_attr->common.table = current_table;;
			//insert_array_index(object_comm_attr, current_table);
			//insert_auto_defaut_param(current_table);
		}

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
		tree_t* node =  (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else), &@$);
		node->if_else.cond = $3;
		//parse_expression(&($3), current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);
		$<tree_type>$ = node;
	}
	object_statements '}' {
		tree_t* node = $<tree_type>6;
		node->if_else.if_block = $7;
		node->if_else.if_table = current_table;
		node->common.print_node = print_if_else;
		node->common.parse = parse_obj_if_else;

		current_table = pop(table_stack);
		$$ = node;
	}
	;

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
		node->if_else.else_table = current_table;
		current_table = pop(table_stack);

		$$ = node;
	}
	| object_if_statement ELSE object_if {
		tree_t* node = $1;
		node->if_else.else_if = $3;

		$$ = node;
	}
	;

parameter
	: PARAMETER objident paramspec {
		#if 0
		//attr->spec = get_paramspec($3, current_table);
		if (charge_standard_parameter(current_table, attr) == 0) {
			if ((attr->spec == NULL) && ((current_table->no_check) == 0)) {
				fprintf(stderr, "error: no assignment to parameter '%s'\n", attr->name);
				free(attr);
				/* TODO: handle the error*/
				exit(-1);
			}
           else if (symbol_defined(current_table, $2->ident.str)) {
               fprintf(stderr, "error: duplicate assignment to parameter '%s'\n", $2->ident.str);
				if (attr->spec)
					free(attr->spec);
               free(attr);
               /* TODO: handle the error */
               exit(-1);
               $<tree_type>$ = NULL;
			}
			else {
				symbol_insert(current_table, $2->ident.str, PARAMETER_TYPE, attr);
			}
		}
		#endif

		tree_t* node = (tree_t*)create_node("parameter", PARAMETER_TYPE, sizeof(struct tree_param), &@$);
		node->param.name = $2->ident.str;
		node->param.paramspec = $3;
		node->common.print_node = print_parameter;
		node->common.parse = parse_parameter;
		DBG("parameter name is %s\n", $2->ident.str);
		$$ = node;
	}
	;

paramspec
	: ';' {
		$$ = NULL;
	}
	| '=' expression ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec), &@$);
		node->paramspec.expr = $2;
		node->common.print_node = print_paramspec;
		$$ = node;
	}
	| '=' STRING_LITERAL ';' {
		tree_t* str = (tree_t*)create_node("const_string", CONST_STRING_TYPE, sizeof(struct tree_string), &@$);
		str->string.pointer = $2;
		str->string.length = strlen($2);
		str->common.print_node = print_string;

		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec), &@$);
		node->paramspec.string = str;
		node->common.print_node = print_paramspec;
		DBG("paramspec: %s\n", $2);
		$$ = node;
	}
	| DEFAULT expression ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec), &@$);
		node->paramspec.is_default = 1;
		node->paramspec.expr = $2;
		node->common.print_node = print_paramspec;
		$$ = node;
	}
	| AUTO ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec), &@$);
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
		tree_t* node = (tree_t*)create_node("method_params", PARAM_TYPE, sizeof(struct tree_params), &@$);
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
		tree_t* node = (tree_t*)create_node("method_params", PARAM_TYPE, sizeof(struct tree_params), &@$);
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
		tree_t* node = (tree_t*)create_node("method_params", PARAM_TYPE, sizeof(struct tree_params), &@$);
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
		//attr->expr = parse_expression(&($2), current_table);
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array), &@$);
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
		//attr->expr = parse_expression(&($2), current_table);
		//attr->expr_end = parse_expression(&($4), current_table);
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array), &@$);
		node->array.is_fix = 0;
		node->array.expr = $2;
		node->array.expr_end = $4;
		node->common.print_node = print_array;
		node->common.attr = attr;
		attr->common.node = node;
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
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.basetype = $1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		node->common.translate = translate_cdecl;
		$$ = node;
	}
	| CONST basetype cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_const = 1;
		node->cdecl.basetype = $2;
		node->cdecl.decl = $3;
		node->common.print_node = print_cdecl;
		node->common.translate = translate_cdecl2;
		$$ = node;
	}
	;

basetype
	: typeident {
		DBG("Line = %d typeident: %s : %s\n", __LINE__, $1->common.name, $1->ident.str);
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
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_const = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	| '*' cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_point = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		node->common.translate = translate_cdecl2;
		$$ = node;
	}
	| VECT cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl), &@$);
		node->cdecl.is_vect = 1;
		node->cdecl.decl = $2;
		node->common.print_node = print_cdecl;
		$$ = node;
	}
	;

cdecl3
	: typeident {
		DBG("Line = %d typeident: %s : %s\n", __LINE__, $1->common.name, $1->ident.str);
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	| cdecl3 '[' expression ']' {
		tree_t* node = (tree_t*)create_node("array_cdecl", ARRAY_TYPE, sizeof(struct tree_array), &@$);
		node->array.is_fix = 1;
		node->array.expr = $3;
		node->array.decl = $1;
		node->common.print_node = print_array;
		node->common.translate = translate_cdecl3_array;
		$$ = node;
	}
	| cdecl3 '(' cdecl_list ')' {
		tree_t* node = (tree_t*)create_node("cdecl_brack", CDECL_BRACK_TYPE, sizeof(struct tree_cdecl_brack), &@$);
		node->cdecl_brack.is_list = 1;
		node->cdecl_brack.cdecl = $1;
		node->cdecl_brack.decl_list = $3;
		node->common.print_node = print_cdecl_brak;
		$$ = node;
	}
	| '(' cdecl2 ')' {
		tree_t* node = (tree_t*)create_node("cdecl_brack", CDECL_BRACK_TYPE, sizeof(struct tree_cdecl_brack), &@$);
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
		tree_t* node = (tree_t*)create_node("...", ELLIPSIS_TYPE, sizeof(struct tree_common), &@$);
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
		tree_t* node = (tree_t*)create_node("...", ELLIPSIS_TYPE, sizeof(struct tree_common), &@$);
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
		tree_t* node = (tree_t*)create_node("typeof", TYPEOF_TYPE, sizeof(struct tree_typeof), &@$);
		node->typeof_tree.expr = $2;
		node->common.print_node = print_typeof;
		node->common.translate = translate_typeof;
		$$ = node;
	}
	;

struct
	: STRUCT '{' {
		tree_t* node = (tree_t*)create_node("struct", STRUCT_TYPE, sizeof(struct tree_struct), &@$);
		current_table = change_table(current_table, table_stack, &current_table_num, STRUCT_TYPE);
		node->common.print_node = print_struct;
		node->struct_tree.table = current_table;
		$<tree_type>$ = node;
	}
	struct_decls '}' {
		tree_t* node = $<tree_type>3;
		node->struct_tree.block = $4;

		//parse_struct_decls($4, current_table);
		current_table = pop(table_stack);

		$$ = node;
	}
	;

struct_decls
	: struct_decls cdecl ';' {
		if ($1 != NULL) {
			create_node_list($1, $2);
			$$ = $1;
		}
		else {
			$$ = $2;
		}
	}
	| {
		$$ = NULL;
	}
	;

layout
	:LAYOUT STRING_LITERAL '{' {
	#if 0
		layout_attr_t* attr = (layout_attr_t*)gdml_zmalloc(sizeof(layout_attr_t));
		attr->str = $2;
		symbol_insert(current_table, $2, LAYOUT_TYPE, attr);
		attr->table  = current_table;
		attr->common.node = node;
	#endif

		current_table = change_table(current_table, table_stack, &current_table_num, LAYOUT_TYPE);

		tree_t* node = (tree_t*)create_node("layout", LAYOUT_TYPE, sizeof(struct tree_layout), &@$);
		node->layout.desc = $2;
		node->layout.table = current_table;
		node->common.print_node = print_layout;
		//node->common.attr = attr;

		$<tree_type>$ = node;
	}
	layout_decls '}' {
		tree_t* node = $<tree_type>4;
		node->layout.block = $5;

		//parse_layout_decls($5, current_table);
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
	#if 0
		bitfield_attr_t* attr = (bitfield_attr_t*)gdml_zmalloc(sizeof(bitfield_attr_t));
		attr->name = $2;
		symbol_insert(current_table, $2, BITFIELDS_TYPE, attr);
		attr->table = current_table;
		attr->common.node = node;
	#endif

		current_table = change_table(current_table, table_stack, &current_table_num, BITFIELDS_TYPE);

		tree_t* node = (tree_t*)create_node("bitfields", BITFIELDS_TYPE, sizeof(struct tree_bitfields), &@$);
		node->bitfields.size_str = $2;
		node->bitfields.size = strtoi($2);
		node->bitfields.table = current_table;
		node->common.print_node = print_bitfields;
		//node->common.attr = attr;

		$<tree_type>$ = node;
	}
	bitfields_decls '}' {
		tree_t* node = $<tree_type>4;
		node->bitfields.block = $5;
		//parse_bitfields_decls($5, current_table);
		current_table = pop(table_stack);
		$$ = node;
	}
	;

bitfields_decls
	: bitfields_decls cdecl '@' '[' expression ':' expression ']' ';' {
		tree_t* node = (tree_t*)create_node("bitfields_decls", BITFIELDS_DECL_TYPE, sizeof(struct tree_bitfields_dec), &@$);
		node->bitfields_dec.decl = $2;
		node->bitfields_dec.start = $7;
		node->bitfields_dec.end = $5;
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
		tree_t* node = (tree_t*)create_node("ctypedecl", CTYPEDECL_TYPE, sizeof(struct tree_ctypedecl), &@$);
		node->ctypedecl.const_opt = $1;
		node->ctypedecl.basetype = $2;
		node->ctypedecl.ctypedecl_ptr = $3;
		node->common.print_node = print_ctypedecl;
		$$ = node;
	}
	;

ctypedecl_ptr
	: stars ctypedecl_array {
		tree_t* node = (tree_t*)create_node("ctypedecl_ptr", CTYPEDECL_PTR_TYPE, sizeof(struct tree_ctypedecl_ptr), &@$);
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
		tree_t* node = (tree_t*)create_node("stars", STARS_TYPE, sizeof(struct tree_stars), &@$);
		node->stars.is_const = 1;
		node->stars.stars = $3;
		node->common.print_node = print_stars;
		$$ = node;
	}
	| '*' stars {
		tree_t* node = (tree_t*)create_node("stars", STARS_TYPE, sizeof(struct tree_stars), &@$);
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
		tree_t* node = (tree_t*)create_node("ctypedecl_simple", CTYPEDECL_SIMPLE_TYPE, sizeof(struct tree_cdecl_brack), &@$);
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
		tree_t* node  = (tree_t*)c_keyword_node("const", &@$);
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
	| BOOL {
		tree_t* node = (tree_t*)c_keyword_node("bool", &@$);
		node->ident.type = BOOL_TYPE;
		node->common.translate = translate_c_keyword;
		$$ = node;
	}
	| CHAR {
		tree_t* node = (tree_t*)c_keyword_node("char", &@$);
		node->ident.type = CHAR_TYPE;
		$$ = node;
	}
	| DOUBLE {
		tree_t* node = (tree_t*)c_keyword_node("double", &@$);
		node->ident.type = DOUBLE_TYPE;
		$$ = node;
	}
	| FLOAT {
		tree_t* node = (tree_t*)c_keyword_node("float", &@$);
		node->ident.type = FLOAT_TYPE;
		$$ = node;
	}
	| INT {
		tree_t* node = (tree_t*)c_keyword_node($1, &@$);
		node->ident.type = INT_TYPE;
		node->common.translate = translate_c_keyword;
		$$ = node;
	}
	| LONG {
		tree_t* node = (tree_t*)c_keyword_node("long", &@$);
		node->ident.type = LONG_TYPE;
		$$ = node;
	}
	| SHORT  {
		tree_t* node = (tree_t*)c_keyword_node("short", &@$);
		node->ident.type = SHORT_TYPE;
		$$ = node;
	}
	| SIGNED {
		tree_t* node = (tree_t*)c_keyword_node("signed", &@$);
		node->ident.type = SIGNED_TYPE;
		$$ = node;
	}
	| UNSIGNED {
		tree_t* node = (tree_t*)c_keyword_node("unsigned", &@$);
		node->ident.type = UNSIGNED_TYPE;
		$$ = node;
	}
	| VOID {
		tree_t* node = (tree_t*)c_keyword_node("void", &@$);
		node->ident.type = VOID_TYPE;
		$$ = node;
	}
	;

comma_expression
	: expression {
		$$ = $1;
	}
	| comma_expression ',' expression {
		create_node_list($1, $3);
		$$ = $1;
	}
	;

expression
	: expression '=' expression {
		tree_t* node = (tree_t*)create_node("assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("=");
		node->expr_assign.type = EXPR_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_expr_assign;
		node->common.translate = translate_assign;
		$$ = node;
	}
	| expression ADD_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("add_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("+=");
		node->expr_assign.type = ADD_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		node->common.translate  = translate_assign;
		$$ = node;
	}
	| expression SUB_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("sub_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("-=");
		node->expr_assign.type = SUB_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_assign;
		$$ = node;
	}
	| expression MUL_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("mul_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("*=");
		node->expr_assign.type = MUL_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression DIV_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("div_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("/=");
		node->expr_assign.type = DIV_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression MOD_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("mod_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("%=");
		node->expr_assign.type = MOD_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression OR_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("or_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("|=");
		node->expr_assign.type = OR_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_assign;
		$$ = node;
	}
	| expression AND_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("and_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("&=");
		node->expr_assign.type = AND_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_assign;
		$$ = node;
	}
	| expression XOR_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("xor_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("^=");
		node->expr_assign.type = XOR_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression LEFT_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("left_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup("<<=");
		node->expr_assign.type = LEFT_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression RIGHT_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("right_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign), &@$);
		node->expr_assign.assign_symbol = strdup(">>=");
		node->expr_assign.type = RIGHT_ASSIGN_TYPE;
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '?' expression ':' expression {
		tree_t* node = (tree_t*)create_node("ternary", TERNARY_TYPE, sizeof(struct tree_ternary), &@$);
		node->ternary.cond = $1;
		node->ternary.expr_true = $3;
		node->ternary.expr_false = $5;
		node->common.print_node = print_ternary;
		$$ = node;
	}
	| expression '+' expression {
		tree_t* node = (tree_t*)create_node("add", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("+");
		node->binary.type = ADD_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression '-' expression {
		tree_t* node = (tree_t*)create_node("sub", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("-");
		node->binary.type = SUB_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression '*' expression {
		tree_t* node = (tree_t*)create_node("mul", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("*");
		node->binary.type = MUL_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '/' expression {
		tree_t* node = (tree_t*)create_node("div", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("/");
		node->binary.type = DIV_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '%' expression {
		tree_t* node = (tree_t*)create_node("mod", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("%");
		node->binary.type = MOD_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression LEFT_OP expression {
		tree_t* node = (tree_t*)create_node("left_op", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("<<");
		node->binary.type = LEFT_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression RIGHT_OP expression {
		tree_t* node = (tree_t*)create_node("right_op", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup(">>");
		node->binary.type = RIGHT_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression EQ_OP expression {
		tree_t* node = (tree_t*)create_node("eq_op", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("==");
		node->binary.type = EQ_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate  = translate_binop_expr;
		$$ = node;
	}
	| expression NE_OP expression {
		tree_t* node = (tree_t*)create_node("ne_op", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("!=");
		node->binary.type = NE_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression '<' expression {
		tree_t* node = (tree_t*)create_node("less", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("<");
		node->binary.type = LESS_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression '>' expression {
		tree_t* node = (tree_t*)create_node("great", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup(">");
		node->binary.type = GREAT_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression LE_OP expression {
		tree_t* node = (tree_t*)create_node("less_eq", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("<=");
		node->binary.type = LESS_EQ_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression GE_OP expression {
		tree_t* node = (tree_t*)create_node("great_eq", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup(">=");
		node->binary.type = GREAT_EQ_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression OR_OP expression {
		tree_t* node = (tree_t*)create_node("or_op", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("||");
		node->binary.type = OR_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression AND_OP expression {
		tree_t* node = (tree_t*)create_node("and_op", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("&&");
		node->binary.type = AND_OP_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression '|' expression {
		tree_t* node = (tree_t*)create_node("or", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("|");
		node->binary.type = OR_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| expression '^' expression {
		tree_t* node = (tree_t*)create_node("xor", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("^");
		node->binary.type = XOR_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		$$ = node;
	}
	| expression '&' expression {
		tree_t* node = (tree_t*)create_node("and", BINARY_TYPE, sizeof(struct tree_binary), &@$);
		node->binary.operat = strdup("&");
		node->binary.type = AND_TYPE;
		node->binary.left = $1;
		node->binary.right = $3;
		node->common.print_node = print_binary;
		node->common.translate = translate_binop_expr;
		$$ = node;
	}
	| CAST '(' expression ',' ctypedecl ')' {
		tree_t* node = (tree_t*)create_node("cast", CAST_TYPE, sizeof(struct tree_cast), &@$);
		node->cast.expr = $3;
		node->cast.ctype = $5;
		node->common.print_node = print_cast;
		$$ = node;
	}
	| SIZEOF expression {
		tree_t* node = (tree_t*)create_node("sizeof", SIZEOF_TYPE, sizeof(struct tree_sizeof), &@$);
		node->sizeof_tree.expr = $2;
		node->common.print_node = print_sizeof;
		$$ = node;
	}
	| '-' expression {
		tree_t* node = (tree_t*)create_node("negative", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("-");
		node->unary.type = NEGATIVE_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		node->common.translate = translate_unary_expr;
		$$ = node;
	}
	| '+' expression {
		tree_t* node = (tree_t*)create_node("convert", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("+");
		node->unary.type = CONVERT_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		node->common.translate = translate_unary_expr;
		$$ = node;
	}
	| '!' expression {
		tree_t* node = (tree_t*)create_node("non_op", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("!");
		node->unary.type = NON_OP_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '~' expression {
		tree_t* node = (tree_t*)create_node("bit_non", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("~");
		node->unary.type = BIT_NON_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		DBG("~~~~~~~~ %s\n", node->unary.expr->int_cst.int_str);
		node->common.translate = translate_unary_expr; 
		$$ = node;
	}
	| '&' expression {
		tree_t* node = (tree_t*)create_node("addr", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("&");
		node->unary.type = ADDR_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		node->common.translate = translate_unary_expr;
		$$ = node;
	}
	| '*' expression {
		tree_t* node = (tree_t*)create_node("pointer", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("*");
		node->unary.type = POINTER_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| DEFINED expression {
		tree_t* node = (tree_t*)create_node("defined", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("defined");
		node->unary.type = DEFINED_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| '#' expression {
		tree_t* node = (tree_t*)create_node("translates", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("#");
		node->unary.type = EXPR_TO_STR_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		$$ = node;
	}
	| INC_OP expression {
		tree_t* node = (tree_t*)create_node("pre_inc_op", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("pre_inc");
		node->unary.type = PRE_INC_OP_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		node->common.translate = translate_pre_expr;
		$$ = node;
	}
	| DEC_OP expression {
		tree_t* node = (tree_t*)create_node("pre_dec_op", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("pre_dec");
		node->unary.type = PRE_DEC_OP_TYPE;
		node->unary.expr = $2;
		node->common.print_node = print_unary;
		node->common.translate = translate_pre_expr;
		$$ = node;
	}
	| expression INC_OP {
		tree_t* node = (tree_t*)create_node("aft_inc_op", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("aft_inc");
		node->unary.type = AFT_INC_OP_TYPE;
		node->unary.expr = $1;
		node->common.print_node = print_unary;
		node->common.translate = translate_post_expr;
		$$ = node;
	}
	| expression DEC_OP {
		tree_t* node = (tree_t*)create_node("aft_dec_op", UNARY_TYPE, sizeof(struct tree_unary), &@$);
		node->unary.operat = strdup("aft_dec");
		node->unary.type = AFT_DEC_OP_TYPE;
		node->unary.expr = $1;
		node->common.print_node = print_unary;
		node->common.translate = translate_post_expr;
		$$ = node;
	}
	| expression '(' expression_list ')' {
		tree_t* node  = (tree_t*)create_node("expr_brack", EXPR_BRACK_TYPE, sizeof(struct tree_expr_brack), &@$);
		node->expr_brack.expr = $1;
		node->expr_brack.expr_in_brack = $3;
		node->common.print_node = print_expr_brack;
		node->common.translate = translate_expr_brack_direct;
		$$ = node;
	}
	| INTEGER_LITERAL {
		DBG("Integer_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst), &@$);
		node->int_cst.int_str = $1;
		if (strlen($1) <= 10) {
			node->int_cst.value = strtoi($1);
		}
		else {
			node->int_cst.out_64bit = 1;
		}
		node->common.print_node = print_interger;
		node->common.translate = translate_integer_literal;
		$$= node;
	}
	| FLOAT_LITERAL {
		tree_t* node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst), &@$);
		node->float_cst.float_str = $1;
		node->float_cst.value = atof($1);
		node->common.print_node = print_float_literal;
		node->common.translate = translate_float;
		$$= node;
	}
	| STRING_LITERAL {
		DBG("String_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("string_literal", CONST_STRING_TYPE, sizeof(struct tree_string), &@$);
		node->string.length = strlen($1);
		node->string.pointer = $1;
		node->common.print_node = print_string;
		$$=node;
	}
	| UNDEFINED {
		tree_t* node = (tree_t*)create_node("undefined", UNDEFINED_TYPE, sizeof(struct tree_common), &@$);
		node->common.print_node = print_undefined;
		$$ = node;
	}
	| '$' objident {
		DBG("In $objident: %s\n", $2->ident.str);
		tree_t* node = (tree_t*)create_node("quote", QUOTE_TYPE, sizeof(struct tree_quote), &@$);
		node->quote.ident = $2;
		node->common.print_node = print_quote;
		node->common.translate = translate_quote;
		$$ = node;
	}
	| ident {
		DBG("ident: %s\n", $1->ident.str);
		$$ = $1;
	}
	| expression '.' objident {
		tree_t* node = (tree_t*)create_node("dot", COMPONENT_TYPE, sizeof(struct tree_component), &@$);
		node->component.comp = strdup(".");
		node->component.type = COMPONENT_DOT_TYPE;
		node->component.expr = $1;
		node->component.ident = $3;
		node->common.print_node = print_component;
		node->common.translate = translate_ref_expr;
		$$ = node;
	}
	| expression METHOD_RETURN objident {
		tree_t* node = (tree_t*)create_node("pointer", COMPONENT_TYPE, sizeof(struct tree_component), &@$);
		node->component.comp = strdup("->");
		node->component.type = COMPONENT_POINTER_TYPE;
		node->component.expr = $1;
		node->component.ident = $3;
		node->common.print_node = print_component;
		$$ = node;
	}
	| SIZEOFTYPE typeoparg {
		tree_t* node = (tree_t*)create_node("sizeoftype", SIZEOFTYPE_TYPE, sizeof(struct tree_sizeoftype), &@$);
		node->sizeoftype.typeoparg = $2;
		node->common.print_node = print_sizeoftype;
		$$ = node;
	}
	;

typeoparg
	: ctypedecl {
		tree_t* node = (tree_t*)create_node("typeoparg", TYPEOPARG_TYPE, sizeof(struct tree_typeoparg), &@$);
		node->typeoparg.ctypedecl = $1;
		node->common.print_node = print_typeoparg;
		$$ = node;
	}
	| '(' ctypedecl ')' {
		tree_t* node = (tree_t*)create_node("typeoparg_brack", TYPEOPARG_TYPE, sizeof(struct tree_typeoparg), &@$);
		node->typeoparg.ctypedecl_brack = $2;
		node->common.print_node = print_typeoparg;
		$$ = node;
	}
	;

expression
	: NEW ctypedecl {
		tree_t* node = (tree_t*)create_node("new", NEW_TYPE, sizeof(struct tree_new), &@$);
		node->new_tree.type = $2;
		node->common.print_node = print_new;
		$$ = node;
	}
	| NEW ctypedecl '[' expression ']' {
		tree_t* node = (tree_t*)create_node("new", NEW_TYPE, sizeof(struct tree_new), &@$);
		node->new_tree.type = $2;
		node->new_tree.count = $4;
		node->common.print_node = print_new;
		$$ = node;
	}
	| '(' expression ')' {
		tree_t* node = (tree_t*)create_node("expr_brack", EXPR_BRACK_TYPE, sizeof(struct tree_expr_brack), &@$);
		 node->expr_brack.expr = NULL;
		 node->expr_brack.expr_in_brack = $2;
		 node->common.print_node = print_expr_brack;
		 node->common.translate = translate_brack_expr;
		 $$ = node;

	}
	| '[' expression_list ']' {
		debug_proc("Line : %d\n", __LINE__);
        tree_t* node = (tree_t*)create_node("expr_array", ARRAY_TYPE, sizeof(struct tree_array), &@$);
        node->array.expr = $2;
        node->common.print_node = print_array;
        $$ = node;
    }
	| expression '[' expression endianflag ']' {
		tree_t* node = (tree_t*)create_node("bit_slicing_expr", BIT_SLIC_EXPR_TYPE, sizeof(struct tree_bit_slic), &@$);
		node->bit_slic.expr = $1;
		node->bit_slic.bit = $3;
		node->bit_slic.endian = $4;
		node->common.print_node = print_bit_slic;
		node->common.translate = translate_bit_slice2;
		$$ = node;
	}
	| expression '[' expression ':' expression endianflag ']' {
		tree_t* node = (tree_t*)create_node("bit_slicing_expr", BIT_SLIC_EXPR_TYPE, sizeof(struct tree_bit_slic), &@$);
		node->bit_slic.expr = $1;
		node->bit_slic.bit = $3;
		node->bit_slic.bit_end = $5;
		node->bit_slic.endian = $6;
		node->common.print_node = print_bit_slic;
		node->common.translate = translate_bit_slic;
		$$ = node;
	}
	;

endianflag
	: ',' IDENTIFIER {
		tree_t* node = (tree_t*)create_node("endianflag", IDENT_TYPE, sizeof(struct tree_ident), &@$);
		node->ident.str = (char*)($2);
		node->ident.len = strlen(((char*)($2)));
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

if_statement
	: IF '(' expression ')' {
		tree_t* node = (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else), &@$);
		node->if_else.cond = $3;
		//parse_expression(&($3), current_table);

		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>5;
		node->if_else.if_block = $6;
		node->if_else.if_table = current_table;
		node->common.print_node = print_if_else;
		node->common.parse = parse_if_else;
		node->common.translate = translate_if_else;
		current_table = pop(table_stack);
		$$ = node;
	}

statement
	: compound_statement {
		$$ = $1;
	}
	| local {
		tree_t* node = $1;
		node->common.parse = parse_local;
		$$ = node;
	}
	| ';' {
		$$ = NULL;
	}
	| expression ';'{
		DBG("expression in statement\n");
		tree_t* node = $1;
		node->common.parse = parse_expr;
		$$ =  $1;
	}
	| if_statement {
		$$ = $1;
	}
	| if_statement ELSE {
		tree_t* node = $1;
		current_table = change_table(current_table, table_stack, &current_table_num, IF_ELSE_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>1;
		if ($4->common.type == IF_ELSE_TYPE) {
			node->if_else.else_if = $4;
		}
		else {
			node->if_else.else_block = $4;
			node->if_else.else_table = current_table;
		}
		current_table = pop(table_stack);
		$$ = node;
	}
	| WHILE '(' expression ')' {
		tree_t* node = (tree_t*)create_node("do_while", DO_WHILE_TYPE, sizeof(struct tree_do_while), &@$);
		node->do_while.cond = $3;

		current_table = change_table(current_table, table_stack, &current_table_num, DO_WHILE_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>5;
		node->do_while.block = $6;
		node->do_while.table = current_table;
		node->common.print_node = print_while;
		node->common.parse = parse_do_while;
		node->common.translate = translate_while;
		current_table = pop(table_stack);
		$$ = node;
	}
	| DO {
		tree_t* node = (tree_t*)create_node("do_while", DO_WHILE_TYPE, sizeof(struct tree_do_while), &@$);
		node->do_while.have_do = 1;
		current_table = change_table(current_table, table_stack, &current_table_num, DO_WHILE_TYPE);
		$<tree_type>$ = node;
	}
	statement WHILE '(' expression ')' ';' {
		tree_t* node = $<tree_type>2;
		node->do_while.cond = $6;
		node->do_while.block = $3;
		node->do_while.table = current_table;
		node->common.print_node = print_do_while;
		node->common.parse = parse_do_while;
		current_table = pop(table_stack);
		$$ = node;
	}
	| FOR '(' comma_expression_opt ';' expression_opt ';' comma_expression_opt ')' {
		tree_t* node = (tree_t*)create_node("for", FOR_TYPE, sizeof(struct tree_for), &@$);
		node->for_tree.init = $3;
		node->for_tree.cond = $5;
		node->for_tree.update = $7;
		node->common.print_node = print_for;
		node->common.parse = parse_for;
		current_table = change_table(current_table, table_stack, &current_table_num, FOR_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>9;
		node->for_tree.block = $10;
		node->for_tree.table = current_table;
		current_table = pop(table_stack);
		$$ = node;
	}
	| SWITCH '(' expression ')' {
		tree_t* node = (tree_t*)create_node("switch", SWITCH_TYPE, sizeof(struct tree_switch), &@$);
		node->switch_tree.cond = $3;
		current_table = change_table(current_table, table_stack, &current_table_num, SWITCH_TYPE);
		node->common.translate = translate_switch;
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>5;
		node->switch_tree.block = $6;
		node->switch_tree.table = current_table;
		node->common.parse = parse_switch;
		node->common.print_node = print_switch;
		current_table = pop(table_stack);
		$$ = node;
	}
	| DELETE expression ';' {
		tree_t* node = (tree_t*)create_node("delete", DELETE_TYPE, sizeof(struct tree_delete), &@$);
		node->delete_tree.expr = $2;
		node->common.print_node = print_delete;
		node->common.parse = parse_delete;
		$$ = node;
	}
	| TRY {
		tree_t* node = (tree_t*)create_node("try_catch", TRY_CATCH_TYPE, sizeof(struct tree_try_catch), &@$);
		node->common.parse = parse_try_catch;
		current_table = change_table(current_table, table_stack, &current_table_num, TRY_CATCH_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>2;
		node->try_catch.try_block = $3;
		node->try_catch.try_table = current_table;
		current_table = pop(table_stack);
		$<tree_type>$ = node;
	}
	CATCH {
		tree_t* node = $<tree_type>4;
		current_table = change_table(current_table, table_stack, &current_table_num, TRY_CATCH_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>6;
		node->try_catch.catch_block = $7;
		node->try_catch.catch_table = current_table;
		node->common.print_node = print_try_catch;
		current_table = pop(table_stack);
		DBG(" try catch in statement\n");
		$$ = node;
	}
	| AFTER '(' expression ')' CALL expression ';' {
		tree_t* node = (tree_t*)create_node("after_call", AFTER_CALL_TYPE, sizeof(struct tree_after_call), &@$);
		node->after_call.cond = $3;
		node->after_call.call_expr = $6;
		node->common.print_node = print_after_call;
		node->common.parse = parse_after_call;
		node->common.translate = translate_after_call;

		$$ = node;
	}
	| CALL expression returnargs ';' {
		tree_t* node = (tree_t*)create_node("call", CALL_TYPE, sizeof(struct tree_call_inline), &@$);
		node->call_inline.expr = $2;
		node->call_inline.ret_args = $3;
		node->common.print_node = print_call_inline;
		node->common.translate = translate_call;
		//parse_expression(&($2), current_table);
		DBG("CALL statement\n");
		$$ = node;
	}
	| INLINE expression returnargs ';' {
		tree_t* node = (tree_t*)create_node("inline", INLINE_TYPE, sizeof(struct tree_call_inline), &@$);
		node->call_inline.expr = $2;
		node->call_inline.ret_args = $3;
		node->common.print_node = print_call_inline;
		node->common.translate  = translate_inline;
		DBG("inline statement\n");
		$$ = node;
	}
	| ASSERT expression ';' {
		tree_t* node = (tree_t*)create_node("assert", ASSERT_TYPE, sizeof(struct tree_assert), &@$);
		node->assert_tree.expr = $2;
		node->common.print_node = print_assert;
		node->common.parse = parse_assert;
		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ',' expression ':' STRING_LITERAL ',' log_args ';' {
		DBG("In LOG statement: %s\n", $8);
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log), &@$);
		node->log.log_type = $2;
		node->log.level = $4;
		node->log.group = $6;
		node->log.format = $8;
		node->log.args = $10;
		node->common.print_node = print_log;
		node->common.parse = parse_log;
		node->common.translate = translate_log;
		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ':' STRING_LITERAL log_args ';' {
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log), &@$);
		node->log.log_type = $2;
		node->log.level = $4;
		node->log.format = $6;
		node->log.args = $7;
		node->common.print_node = print_log;
		node->common.parse = parse_log;
		node->common.translate = translate_log;
		$$ = node;
	}
	| LOG STRING_LITERAL ':' STRING_LITERAL log_args ';' {
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log), &@$);
		node->log.log_type = $2;
		node->log.format = $4;
		node->log.args = $5;
		node->common.print_node = print_log;
		node->common.parse = parse_log;
		node->common.translate = translate_log;
		$$ = node;
	}
	| SELECT ident IN '(' expression ')' WHERE '(' expression ')' {
		tree_t* node = (tree_t*)create_node("select", SELECT_TYPE, sizeof(struct tree_select), &@$);
		node->select.ident = $2;
		node->select.in_expr = $5;
		node->select.cond = $9;
		node->common.print_node = print_select;
		node->common.parse = parse_select;
		current_table = change_table(current_table, table_stack, &current_table_num, SELECT_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>11;
		node->select.where_block = $12;
		node->select.where_table = current_table;
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
		node->select.else_table = current_table;
		current_table = pop(table_stack);
		$$ = node;
	}
	| FOREACH ident IN '(' expression ')' {
		tree_t* node = (tree_t*)create_node("foreach", FOREACH_TYPE, sizeof(struct tree_foreach), &@$);
		node->foreach.ident = $2;
		node->foreach.in_expr = $5;
		node->common.print_node = print_foreach;
		node->common.parse = parse_foreach;
		node->common.translate = translate_foreach;
		current_table = change_table(current_table, table_stack, &current_table_num, FOREACH_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>7;
		node->foreach.block = $8;
		node->foreach.table = current_table;
		current_table = pop(table_stack);
		$$ = node;
	}
	| ident ':'  {
		tree_t* node = (tree_t*)create_node("label", LABEL_TYPE, sizeof(struct tree_label), &@$);
		node->label.ident = $1;
		current_table = change_table(current_table, table_stack, &current_table_num, LABEL_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>3;
		node->label.block = $4;
		node->common.print_node = print_label;
		node->common.parse = parse_label;
		node->label.table = current_table;
		current_table = pop(table_stack);
		$$ = node;
	}
	| CASE expression ':' {
		tree_t* node = (tree_t*)create_node("case", CASE_TYPE, sizeof(struct tree_case), &@$);
		/* TODO: charge the break */
		node->case_tree.expr = $2;
		current_table = change_table(current_table, table_stack, &current_table_num, CASE_TYPE);

		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>4;
		node->case_tree.block = $5;
		node->case_tree.table = current_table;
		node->common.parse = parse_case;
		current_table = pop(table_stack);
		$$ = node;
	}
	| DEFAULT ':' {
		tree_t* node = (tree_t*)create_node("default", DEFAULT_TYPE, sizeof(struct tree_default), &@$);
		current_table = change_table(current_table, table_stack, &current_table_num, DEFAULT_TYPE);
		$<tree_type>$ = node;
	}
	statement {
		tree_t* node = $<tree_type>3;
		node->default_tree.block = $4;
		node->default_tree.table = current_table;
		node->common.parse = parse_default;
		node->common.print_node = print_default;
		current_table = pop(table_stack);
		$$ = node;
	}
	| GOTO ident ';' {
		tree_t* node = (tree_t*)create_node("goto", GOTO_TYPE, sizeof(struct tree_goto), &@$);
		node->goto_tree.label = $2;
		node->common.print_node = print_goto;
		node->common.parse = parse_goto;
		$$ = node;
	}
	| BREAK ';' {
		tree_t* node = (tree_t*)create_node("break", BREAK_TYPE, sizeof(struct tree_common), &@$);
		node->common.print_node = print_break;
		node->common.translate = translate_break;
		$$ = node;
	}
	| CONTINUE ';' {
		tree_t* node = (tree_t*)create_node("continue", CONTINUE_TYPE, sizeof(struct tree_common), &@$);
		node->common.print_node = print_continue;
		node->common.translate = translate_continue;
		$$ = node;
	}
	| THROW ';' {
		tree_t* node = (tree_t*)create_node("throw", THROW_TYPE, sizeof(struct tree_common), &@$);
		node->common.print_node = print_throw;
		$$ = node;
	}
	| RETURN ';' {
		tree_t* node = (tree_t*)create_node("return", RETURN_TYPE, sizeof(struct tree_common), &@$);
		node->common.print_node = print_return;
		$$ = node;
	}
	| ERROR ';' {
		tree_t* node = (tree_t*)create_node("error", ERROR_TYPE, sizeof(struct tree_error), &@$);
		node->error.str = NULL;
		node->common.print_node = print_error;
		$$ = node;
	}
	| ERROR STRING_LITERAL ';' {
		tree_t* node = (tree_t*)create_node("error", ERROR_TYPE, sizeof(struct tree_error), &@$);
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
		tree_t* node = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block), &@$);
		node->common.translate = translate_block;
		node->block.statement = $2;
		$$ = node;
	}
	| '{' '}' {
		tree_t* node = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block), &@$);
		node->common.translate = translate_block;
		node->block.table = current_table;
		node->block.statement = NULL;
		$$ = node;
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
		tree_t* node = (tree_t*)create_node("local_keyword", LOCAL_KEYWORD_TYPE, sizeof(struct tree_local_keyword), &@$);
		node->local_keyword.name = strdup("local");
		node->common.print_node = print_local_keyword;
		$$ = node;
	}
	| AUTO {
		tree_t* node = (tree_t*)create_node("local_keyword", LOCAL_KEYWORD_TYPE, sizeof(struct tree_local_keyword), &@$);
		node->local_keyword.name = strdup("auto");
		node->common.print_node = print_local_keyword;
		$$ = node;
	}
	;

local
	: local_keyword cdecl ';' {
		tree_t* node = (tree_t*)create_node("local decl", LOCAL_TYPE, sizeof(struct tree_local), &@$);
		node->local_tree.local_keyword = $1;
		node->local_tree.cdecl = $2;
		node->common.print_node = print_local;
		node->common.translate = translate_local;
		$$ = node;
	}
	| STATIC cdecl ';' {
		DBG("In STATIC \n");
		tree_t* node = (tree_t*)create_node("local static", LOCAL_TYPE, sizeof(struct tree_local), &@$);
		node->local_tree.is_static = 1;
		node->local_tree.cdecl = $2;
		node->common.print_node = print_local;
		$$ = node;
	}
	| local_keyword cdecl '=' expression ';' {
		tree_t* node = (tree_t*)create_node("local assign", LOCAL_TYPE, sizeof(struct tree_local), &@$);
		node->local_tree.local_keyword = $1;
		node->local_tree.cdecl = $2;
		node->local_tree.expr = $4;
		node->common.print_node = print_local;
		node->common.translate = translate_local;
		$$ = node;
	}
	| STATIC cdecl '=' expression ';' {
		tree_t* node = (tree_t*)create_node("local static assign", LOCAL_TYPE, sizeof(struct tree_local), &@$);
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
		tree_t* node = (tree_t*)dml_keyword_node("this", &@$);
		node->ident.type = THIS_TYPE;
		$$ = node;
	}
	| REGISTER {
		tree_t* node = (tree_t*)dml_keyword_node("register", &@$);
		node->ident.type = REGISTER_TYPE;
		$$ = node;
	}
	| SIGNED {
		tree_t* node = (tree_t*)c_keyword_node("signed", &@$);
		node->ident.type = SIGNED_TYPE;
		$$ = node;
	}
	| UNSIGNED {
		tree_t* node = (tree_t*)c_keyword_node("unsigned", &@$);
		node->ident.type = UNSIGNED_TYPE;
		$$ = node;
	}
	;

ident
	: IDENTIFIER {
		tree_t* ident = (tree_t*)create_node("identifier", IDENT_TYPE, sizeof(struct tree_ident), &@$);
		ident->ident.str = (char*)($1);
		ident->ident.len = strlen(((char*)($1)));
		ident->common.print_node = print_ident;
		ident->common.translate = translate_ident;
		$$ = ident;
	}
	| ATTRIBUTE {
		tree_t* node = (tree_t*)dml_keyword_node("attribute", &@$);
		node->ident.type = ATTRIBUTE_TYPE;
		$$ = node;
	}
	| BANK {
		tree_t* node = (tree_t*)dml_keyword_node("bank", &@$);
		node->ident.type = BANK_TYPE;
		$$ = node;
	}
	| BITORDER {
		tree_t* node = (tree_t*)dml_keyword_node("bitorder", &@$);
		node->ident.type = BITORDER_TYPE;
		$$ = node;
	}
	| CONNECT {
		tree_t* node = (tree_t*)dml_keyword_node("connect", &@$);
		node->ident.type = CONNECT_TYPE;
		$$ = node;
	}
	| CONSTANT {
		tree_t* node = (tree_t*)dml_keyword_node("constant", &@$);
		node->ident.type = CONSTANT_TYPE;
		$$ = node;
	}
	| DATA {
		tree_t* node = (tree_t*)dml_keyword_node("data", &@$);
		node->ident.type = DATA_TYPE;
		$$ = node;
	}
	| DEVICE {
		tree_t* node = (tree_t*)dml_keyword_node("device", &@$);
		node->ident.type = DEVICE_TYPE;
		$$ = node;
	}
	| EVENT {
		tree_t* node = (tree_t*)dml_keyword_node("event", &@$);
		node->ident.type = EVENT_TYPE;
		$$ = node;
	}
	| FIELD {
		tree_t* node = (tree_t*)dml_keyword_node("field", &@$);
		node->ident.type = FIELD_TYPE;
		$$ = node;
	}
	| FOOTER {
		tree_t* node = (tree_t*)dml_keyword_node("footer", &@$);
		node->ident.type = FOOTER_TYPE;
		$$ = node;
	}
	| GROUP {
		tree_t* node = (tree_t*)dml_keyword_node("group", &@$);
		node->ident.type = GROUP_TYPE;
		$$ = node;
	}
	| HEADER {
		tree_t* node = (tree_t*)dml_keyword_node("header", &@$);
		node->ident.type = HEADER_TYPE;
		$$ = node;
	}
	| IMPLEMENT {
		tree_t* node = (tree_t*)dml_keyword_node("implement", &@$);
		node->ident.type = IMPLEMENT_TYPE;
		$$ = node;
	}
	| IMPORT {
		tree_t* node = (tree_t*)dml_keyword_node("import", &@$);
		node->ident.type = IMPORT_TYPE;
		$$ = node;
	}
	| INTERFACE {
		tree_t* node = (tree_t*)dml_keyword_node("interface", &@$);
		node->ident.type = INTERFACE_TYPE;
		$$ = node;
	}
	| LOGGROUP {
		tree_t* node = (tree_t*)dml_keyword_node("loggroup", &@$);
		node->ident.type = LOGGROUP_TYPE;
		$$ = node;
	}
	| METHOD {
		tree_t* node = (tree_t*)dml_keyword_node("method", &@$);
		node->ident.type = METHOD_TYPE;
		$$ = node;
	}
	| PORT {
		tree_t* node = (tree_t*)dml_keyword_node("port", &@$);
		node->ident.type = PORT_TYPE;
		$$ = node;
	}
	| SIZE {
		tree_t* node = (tree_t*)dml_keyword_node("size", &@$);
		node->ident.type = SIZE_TYPE;
		node->common.translate = translate_dml_keyword;
		$$ = node;
	}
	| CLASS {
		tree_t* node = (tree_t*)dml_keyword_node("class", &@$);
		node->ident.type = CLASS_TYPE;
		$$ = node;
	}
	| ENUM {
		tree_t* node = (tree_t*)dml_keyword_node("enum", &@$);
		node->ident.type = ENUM_TYPE;
		$$ = node;
	}
	| NAMESPACE {
		tree_t* node = (tree_t*)dml_keyword_node("namespace", &@$);
		node->ident.type = NAMESPACE_TYPE;
		$$ = node;
	}
	| PRIVATE {
		tree_t* node = (tree_t*)dml_keyword_node("private", &@$);
		node->ident.type = PRIVATE_TYPE;
		$$ = node;
	}
	| PROTECTED {
		tree_t* node = (tree_t*)dml_keyword_node("protected", &@$);
		node->ident.type = PROTECTED_TYPE;
		$$ = node;
	}
	| PUBLIC {
		tree_t* node = (tree_t*)dml_keyword_node("public", &@$);
		node->ident.type = PUBLIC_TYPE;
		$$ = node;
	}
	| RESTRICT {
		tree_t* node = (tree_t*)dml_keyword_node("register", &@$);
		node->ident.type = RESTRICT_TYPE;
		$$ = node;
	}
	| UNION {
		tree_t* node = (tree_t*)dml_keyword_node("union", &@$);
		node->ident.type = UNION_TYPE;
		$$ = node;
	}
	| USING {
		tree_t* node = (tree_t*)dml_keyword_node("using", &@$);
		node->ident.type = USING_TYPE;
		$$ = node;
	}
	| VIRTUAL {
		tree_t* node = (tree_t*)dml_keyword_node("virtual", &@$);
		node->ident.type = VIRTUAL_TYPE;
		$$ = node;
	}
	| VOLATILE {
		tree_t* node  = (tree_t*)dml_keyword_node("volatile", &@$);
		node->ident.type = VOLATILE_TYPE;
		$$ = node;
	}
	;

%%
#include <stdio.h>
#include <stdlib.h>

//extern int column;
//int lineno  = 1;    /* number of current source line */

void yyerror(YYLTYPE* location, yyscan_t* scanner, tree_t** root_ptr, char *s) {
	fflush(stdout);
//	if(location->file) {
//		fprintf(stderr,"Syntax error on file: %s\n"
//						"line #%d, column #%d\n"
//						"reason: %s\n", location->file->name, location->first_line, location->first_column, s);
//	}else{
//		fprintf(stderr,"Syntax error on line #%d, column #%d: %s\n", location->first_line, location->first_column, s);
//	}
//	exit(1);
	PERROR("%s (current word \"%s\")", *location, s, yyget_text(scanner));
}

tree_t* parse_file(const char* name)
{
	char fullname[DIR_MAX_LEN];
	char filename[DIR_MAX_LEN];
	tree_t* root;
	int i;

	/* parse dml library in different directory  according to priority. */
	for(i = 0; import_dir_list[i] != NULL; i++) {
		/* try open dml file directory. */
		int rt = link_dir_filename(fullname, DIR_MAX_LEN, import_dir_list[i], name);
		assert(rt == 0);
		if(access(fullname, F_OK) == 0) {
			root = get_ast(fullname);
			break;
		}
	}
	if(import_dir_list[i] == NULL) {
		error("can't find %s.", name);
	}

	return root;
}

void insert_array_index(object_attr_t* attr, symtab_t table) {
	assert(attr != NULL);
	assert(table != NULL);
	arraydef_attr_t* array_attr = NULL;

	if ((attr->common.obj_type == REGISTER_TYPE) && (attr->reg.is_array == 1)) {
		array_attr = attr->reg.arraydef;
	}
	else if ((attr->common.obj_type == ATTRIBUTE_TYPE) && (attr->attribute.is_array == 1)) {
		array_attr = attr->attribute.arraydef;
	}
	else if ((attr->common.obj_type == GROUP_TYPE) && (attr->group.is_array == 1)) {
		array_attr = attr->group.arraydef;
	}
	else if ((attr->common.obj_type == PORT_TYPE) && (attr->port.is_array == 1)) {
		array_attr = attr->port.arraydef;
	}
	else if ((attr->common.obj_type == CONNECT_TYPE) && (attr->connect.is_array == 1)) {
		array_attr = attr->connect.arraydef;
	}

	/* index and indexvar is auto parameter when an object is array, can not redefined them */
	if ((array_attr != NULL) && (array_attr->ident != NULL)) {
		if ((strcmp("index", array_attr->ident) == 0) ||
				strcmp("indexvar", array_attr->ident) == 0) {
			fprintf(stderr, "error: duplicate assignment to parameter '%s'\n", array_attr->ident);
			/* TODO: handle the error*/
			return;
		}
		parameter_attr_t* param_attr = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));
		param_attr->name = strdup(array_attr->ident);
		symbol_insert(table, array_attr->ident, PARAMETER_TYPE, param_attr);
	}
	else if ((array_attr != NULL) && (array_attr->fix_array)) {
		/* parameter 'i' is the parameter about object array
		 * when a object is fix array,  such as a[10], it is
		 * inserted into table automatic
		 */
		parameter_attr_t* param_i = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));

		param_i->name = strdup("i");
		symbol_insert(table, "i", PARAMETER_TYPE, param_i);
	}

	/* 'index, i, indexvar' is the parameters about object array
	 * when a object is array, this three parameters are inserted
	 * into table automatic, but the parameter 'i' can be redefined,
	 * such as reg[j in 0..15], the 'i' parameter is replaces by j,
	 * and 'index' can not be muti-defined such as [index .. 0..15]*/
	parameter_attr_t* param_index = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));
	param_index->name = strdup("index");
	symbol_insert(table, "index", PARAMETER_TYPE, param_index);

	parameter_attr_t* param_indexvar = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));
	param_indexvar->name = strdup("indexvar");
	symbol_insert(table, "indexvar", PARAMETER_TYPE, param_indexvar);

	return;
}
