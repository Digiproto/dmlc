%{
#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "symbol.h"
#include "ast.h"
#include "Parser.h"
#define YYDEBUG 1
const char* dir = "/opt/virtutech/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0/";

//#define PARSE_DEBUG
#ifdef PARSE_DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
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
extern char* builtin_filename;
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

%token DML IDENTIFIER INTEGER_LITERAL STRING_LITERAL SIZEOF
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

%type	<ival> DEVICE
%type	<ival> IMPORT
%type	<sval> IDENTIFIER
%type	<sval> CONSTANT
%type	<sval> SIGNED
%type	<sval> HEADER
%type	<tree_type> objident
%type	<tree_type> maybe_objident
%type	<tree_type> ident
%type	<sval> STRING_LITERAL
%type	<sval> INTEGER_LITERAL
%type	<tree_type> typeident
%type	<tree_type> local_keyword
%type	<tree_type> const_opt
%type	<tree_type> dml
%type	<tree_type> syntax_modifiers
%type	<tree_type> device_statements
%type	<tree_type> device_statement
%type	<tree_type> statement
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
%right UNARY INC_OP DEC_OP
%left HYPERUNARY
%left POINTSAT '.' '(' '['

%%
begin_unit
	: DML INTEGER_LITERAL ';' dml {
		dml_attr_t* attr = (dml_attr_t*)malloc(sizeof(dml_attr_t));
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
		if($4 != NULL)
			add_child(*root_ptr, $4);
	}
	;

dml
	: DEVICE objident ';' syntax_modifiers device_statements {
		device_attr_t* attr = (device_attr_t*)malloc(sizeof(device_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, DEVICE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("device", DEVICE_TYPE, sizeof(struct tree_device));
		node->device.name = $2->ident.str;

		printf("Line = %d, node name: %s, device name: %s\n", __LINE__, node->device.common.name, node->device.name);
		tree_t* import_ast = NULL;
		if(builtin_filename != NULL) {
			import_ast = (tree_t*)get_ast(builtin_filename);
			if(import_ast->common.child != NULL)
				create_node_list(node, import_ast->common.child);
		}

		if($4 != NULL)	
			create_node_list(node, $4);
		if($5 != NULL)
			create_node_list(node, $5);

		DBG("Device type is %s\n", $2);
		$$ = node;
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
		bitorder_attr_t*  attr = (bitorder_attr_t*)malloc(sizeof(bitorder_attr_t));
		memset(attr, 0, sizeof(bitorder_attr_t));
		attr->endian= strdup($2->ident.str);
		//symbol_insert($2->ident.str, BITORDER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("bitorder", BITORDER_TYPE, sizeof(struct tree_bitorder));
		node->bitorder.endian = $2->ident.str;
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
	: BANK maybe_objident istemplate object_spec {
		if ($2 == NULL) {
			fprintf(stderr, "There must be objident.\n");
			exit(-1);
		}
		DBG("BANK is %s\n", $2->ident.str);

		bank_attr_t* attr = (bank_attr_t*)gdml_malloc(sizeof(bank_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, BANK_TYPE, attr);

		tree_t* node = (tree_t*)create_node("bank", BANK_TYPE, sizeof(struct tree_bank));
		node->bank.name = $2->ident.str;
		node->bank.templates = $3;
		node->bank.spec = $4;
		$$ = node;
	}
	| REGISTER objident sizespec offsetspec istemplate object_spec {
		DBG("register is %s\n", $2->ident.str);
		register_attr_t* attr = (register_attr_t*)gdml_malloc(sizeof(register_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 0;
		//symbol_insert($2->ident.str, REGISTER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("register", REGISTER_TYPE, sizeof(struct tree_register));
		node->reg.name = $2->ident.str;
		node->reg.sizespec = $3;
		node->reg.offset = $4;
		node->reg.templates = $5;
		node->reg.spec = $6;
		$$ = node;
	}
	| REGISTER objident '[' arraydef ']' sizespec offsetspec istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		DBG("Register is %s\n", $2->ident.str);
		register_attr_t* attr = (register_attr_t*)gdml_malloc(sizeof(register_attr_t));
		attr->name = $2->ident.str;
		attr->is_array = 1;
		//symbol_insert($2->ident.str, REGISTER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("register", REGISTER_TYPE, sizeof(struct tree_register));
		node->reg.name = $2->ident.str;
		node->reg.array = $4;
		node->reg.sizespec = $6;
		node->reg.offset = $7;
		node->reg.templates = $8;
		node->reg.spec = $9;
		$$ = node;
	}
	| FIELD objident bitrange istemplate object_spec {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		field_attr_t* attr = (field_attr_t*)gdml_malloc(sizeof(field_attr_t));
		attr->name = $2->ident.str;
		attr->is_range = 1;
		//symbol_insert($2->ident.str, FIELD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("field", FIELD_TYPE, sizeof(struct tree_field));
		node->field.name = $2->ident.str;
		node->field.bitrange = $3;
		node->field.templates = $4;
		node->field.spec = $5;
		$$ = node;
	}
	| FIELD objident istemplate object_spec {
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		field_attr_t* attr = (field_attr_t*)gdml_malloc(sizeof(field_attr_t));
		attr->name = $2->ident.str;
		attr->is_range = 0;
		//symbol_insert($2->ident.str, FIELD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("field", FIELD_TYPE, sizeof(struct tree_field));
		node->field.name = $2->ident.str;
		node->field.bitrange = NULL;
		node->field.templates = $3;
		node->field.spec = $4;
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
		node->cdecl.state.is_data = 1;
		node->cdecl.decl = $2;
		$$ = node;
	}
	| CONNECT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		connect_attr_t* attr = (connect_attr_t*)gdml_malloc(sizeof(connect_attr_t));
		attr->name = $2->ident.str;
		attr->arraydef = 0;
		//symbol_insert($2->ident.str, CONNECT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("connect", CONNECT_TYPE, sizeof(struct tree_connect));
		node->connect.name = $2->ident.str;
		node->connect.templates = $3;
		node->connect.spec = $4;
		DBG("CONNECT_TYPE: %s\n", $2->ident.str);
		$$ = node;
	}
	| INTERFACE objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		interface_attr_t* attr = (interface_attr_t*)gdml_malloc(sizeof(interface_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, INTERFACE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("interface", INTERFACE_TYPE, sizeof(struct tree_interface));
		node->interface.name = $2->ident.str;
		node->interface.templates = $3;
		node->interface.spec = $4;
		DBG("Interface_type: %s\n", $2->ident.str);
		$$ = node;
	}
	| ATTRIBUTE objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		if ($2 == NULL) {
			fprintf(stderr, "need the identifier of field\n");
			exit(-1);
		}
		attribute_attr_t* attr = (attribute_attr_t*)gdml_malloc(sizeof(attribute_attr_t));
		attr->name = $2->ident.str;
		attr->arraydef = 0;

		tree_t* node = (tree_t*)create_node("attribute", ATTRIBUTE_TYPE, sizeof(struct tree_attribute));
		node->attribute.name = $2->ident.str;
		node->attribute.templates = $3;
		node->attribute.spec = $4;
		DBG("Attribute: %s\n", $2->ident.str);
		$$ = node;
	}
	| EVENT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		event_attr_t* attr = (event_attr_t*)gdml_malloc(sizeof(event_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, EVENT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("event", EVENT_TYPE, sizeof(struct tree_event));
		node->event.name = $2->ident.str;
		node->event.templates = $3;
		node->event.spec = $4;
		$$ = node;
	}
	| GROUP objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("group", GROUP_TYPE, sizeof(struct tree_group));
		node->group.name = $2->ident.str;
		node->group.templates = $3;
		node->group.spec = $4;
		$$ = node;
	}
	| PORT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("port", PORT_TYPE, sizeof(struct tree_port));
		node->port.name = $2->ident.str;
		node->port.templates = $3;
		node->port.spec = $4;
		$$ = node;
	}
	| IMPLEMENT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		implement_attr_t* attr = (implement_attr_t*)gdml_malloc(sizeof(implement_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, IMPLEMENT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("implement", IMPLEMENT_TYPE, sizeof(struct tree_implement));
		node->implement.name = $2->ident.str;
		node->implement.templates = $3;
		node->implement.spec = $4;
		DBG("objident: %s\n", $2->ident.str);
		$$ = node;
	}
	| ATTRIBUTE objident '[' arraydef ']' istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		attribute_attr_t* attr = (attribute_attr_t*)gdml_malloc(sizeof(attribute_attr_t));
		attr->name = $2->ident.str;
		attr->arraydef = 1;

		tree_t* node = (tree_t*)create_node("attribute", ATTRIBUTE_TYPE, sizeof(struct tree_attribute));
		node->attribute.name = $2->ident.str;
		node->attribute.arraydef = $4;
		node->attribute.templates = $6;
		node->attribute.spec = $7;
		$$ = node;
	}
	| GROUP objident '[' arraydef ']' istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("group", GROUP_TYPE, sizeof(struct tree_group));
		node->group.name = $2->ident.str;
		node->group.is_array = 1;
		node->group.array = $4;
		node->group.templates = $6;
		node->group.spec = $7;
		$$ = node;
	}
	| CONNECT objident '[' arraydef ']' istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		connect_attr_t* attr = (connect_attr_t*)gdml_malloc(sizeof(connect_attr_t));
		attr->name = $2->ident.str;
		attr->arraydef = 1;
		//symbol_insert($2->ident.str, CONNECT_TYPE, attr);

		tree_t* node = (tree_t*)create_node("connect", CONNECT_TYPE, sizeof(struct tree_connect));
		node->connect.name = $2->ident.str;
		node->connect.arraydef = $4;
		node->connect.templates = $6;
		node->connect.spec = $7;
		$$ = node;
	}
	;

method
	: METHOD objident method_params compound_statement {
		method_attr_t* attr = (method_attr_t*)gdml_malloc(sizeof(method_attr_t));
		attr->name = $2->ident.str;
		attr->is_extern = 0;
		attr->is_default = 0;
		//symbol_insert($2->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $2->ident.str;
		node->method.is_extern = 0;
		node->method.is_default = 0;
		node->method.params = $3;
		node->method.block = $4;
		DBG("method is %s\n", $2->ident.str);
		$$ = node;
	}
	| METHOD objident method_params DEFAULT compound_statement {
		method_attr_t* attr = (method_attr_t*)gdml_malloc(sizeof(method_attr_t));
		attr->name = $2->ident.str;
		attr->is_extern = 0;
		attr->is_default = 1;
		//symbol_insert($2->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $2->ident.str;
		node->method.is_extern = 0;
		node->method.is_default = 1;
		node->method.params = $3;
		node->method.block = $5;
		DBG("method is %s\n", $2->ident.str);
		$$ = node;
	}
	| METHOD EXTERN objident method_params compound_statement {
		method_attr_t* attr = (method_attr_t*)gdml_malloc(sizeof(method_attr_t));
		attr->name = $3->ident.str;
		attr->is_extern = 1;
		attr->is_default = 0;
		//symbol_insert($3->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $3->ident.str;
		node->method.is_extern = 1;
		node->method.is_default = 0;
		node->method.params = $4;
		node->method.block = $5;
		DBG("method extern is %s\n", $3->ident.str);
		$$ = node;
	}
	| METHOD EXTERN objident method_params DEFAULT compound_statement {
		method_attr_t* attr = (method_attr_t*)gdml_malloc(sizeof(method_attr_t));
		attr->name = $3->ident.str;
		attr->is_extern = 1;
		attr->is_default = 1;
		//symbol_insert($3->ident.str, METHOD_TYPE, attr);

		tree_t* node = (tree_t*)create_node("method", METHOD_TYPE, sizeof(struct tree_method));
		node->method.name = $3->ident.str;
		node->method.is_extern = 1;
		node->method.is_default = 1;
		node->method.params = $4;
		node->method.block = $6;
		DBG("method extern is %s\n", $3->ident.str);
		$$ = node;
	}
	;

arraydef
	: expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 1;
		node->array.expr = $1;
		$$ = node;
	}
	| ident IN expression RANGE_SIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		if ($1 == NULL) {
			fprintf(stderr, "There must be identifier!\n");
			exit(-1);
		}
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 0;
		node->array.ident = $1;
		node->array.expr = $3;
		node->array.expr_end = $5;
		$$ = node;
	}
	;

toplevel
	: TEMPLATE objident object_spec {
		DBG("in TEMPLATE %s\n", $2);
		template_attr_t* attr = (template_attr_t*)gdml_malloc(sizeof(template_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, TEMPLATE_TYPE, attr);

		tree_t* node = (tree_t*)create_node("template", TEMPLATE_TYPE, sizeof(struct tree_template));
		node->temp.name = $2->ident.str;
		node->temp.spec = $3;
		$$ = node;
	}
	| LOGGROUP ident ';' {
		DBG("in LOGGROUP %s\n", $2);
		//symbol_insert($2->ident.str, LOGGROUP_TYPE, NULL);

		tree_t* node = (tree_t*)create_node("loggroup", LOGGROUP_TYPE, sizeof(struct tree_loggroup));
		node->loggroup.name = $2->ident.str;
		$$ = node;
	}
	| CONSTANT ident '=' expression ';' {
		/* TODO: Find the symbol and insert it */
		cdecl_attr_t* attr = (cdecl_attr_t*)gdml_malloc(sizeof(cdecl_attr_t));
		attr->is_constant = 1;

		tree_t* node = (tree_t*)create_node("assign", ASSIGN_TYPE, sizeof(struct tree_assign));
		node->assign.is_constant = 1;
		node->assign.decl = $2;
		node->assign.expr = $4;
		$$ = node;
	}
	| EXTERN cdecl_or_ident ';' {
		/* TODO: we should find the name of extern */
		//symbol_t* symbol = symbol_find()
		DBG("\nPay attention: we should find the name of extern\n\n");
		cdecl_attr_t* attr = (cdecl_attr_t*)gdml_malloc(sizeof(cdecl_attr_t));
		attr->is_extern = 1;

		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.state.is_extern = 1;
		node->cdecl.decl = $2;
		$$ = node;
	}
	| TYPEDEF cdecl ';' {
		/* FIXME: we should find the name cdecl */
		DBG("\nPay attention: we should find the name of typedef cdecl\n\n");
		//$$ = create_node("UNIMP", TYPEDEF_TYPE);
		cdecl_attr_t* attr = (cdecl_attr_t*)gdml_malloc(sizeof(cdecl_attr_t));
		attr->is_typedef = 1;

		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.state.is_typedef = 1;
		node->cdecl.decl = $2;
		$$ = node;
	}
	| EXTERN TYPEDEF cdecl ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| STRUCT ident '{' struct_decls '}' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| HEADER {
		/* FIXME: the header include much content */
		tree_t* node = (tree_t*)create_node("head", HEADER_TYPE, sizeof(struct tree_head));
		node->head.str = $1;
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
		tree_t* root = (tree_t*)create_node($2, IMPORT_TYPE);
		DBG("Begin parse the import file %s\n", fullname);
		tree_t* ast = NULL;
		yylex_init(&scanner);
		yyrestart(file, scanner);
		yyparse(scanner, &ast);
		yylex_destroy(scanner);
		fclose(file);
		DBG("End of parse the import file %s\n", fullname);

		$$ = ast->common.child;
	}
	;

object_desc
	: STRING_LITERAL {
		tree_t* node = (tree_t*)create_node("const_string", CONST_STRING_TYPE, sizeof(struct tree_string));
		node->string.pointer = $1;
		node->string.length = strlen($1);
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
			$$ = node;
		}
	}
	| object_desc '{' object_statements '}' {
		DBG("object_statements for object_spec\n");
		tree_t* node = (tree_t*)create_node("object_spec", SPEC_TYPE, sizeof(struct tree_object_spec));
		node->spec.desc = $1;
		tree_t* block = (tree_t*)create_node("block", BLOCK_TYPE, sizeof(struct tree_block));
		block->block.statement = $3;
		node->spec.block = block;
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
		debug_proc("Line : %d\n", __LINE__);
		DBG("object_if in object_statement \n");
		$$ = $1;
	}
	;

object_if
	: IF '(' expression ')' '{' object_statements '}' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node =  (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		node->if_else.if_block = $6;
		node->if_else.else_block = NULL;
		$$ = node;
	}
	| IF '(' expression ')' '{' object_statements '}' ELSE '{' object_statements '}' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node =  (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		node->if_else.if_block = $6;
		node->if_else.else_block = $10;
		$$ = node;
	}
	| IF '(' expression ')' '{' object_statements '}' ELSE object_if {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node =  (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		node->if_else.if_block = $6;
		node->if_else.else_block = $9;
		$$ = node;
	}
	;

parameter
	: PARAMETER objident paramspec {
		if ($2 == NULL) {
			fprintf(stderr, "parameter should ojbidentifier!\n");
			exit(-1);
		}
		parameter_attr_t* attr = (parameter_attr_t*)gdml_malloc(sizeof(parameter_attr_t));
		attr->name = $2->ident.str;
		//symbol_insert($2->ident.str, PARAMETER_TYPE, attr);

		tree_t* node = (tree_t*)create_node("parameter", PARAMETER_TYPE, sizeof(struct tree_param));
		node->param.name = $2->ident.str;
		node->param.paramspec = $3;
		DBG("parameter name is %s\n", $2);
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
		$$ = node;
	}
	| '=' STRING_LITERAL ';' {
		tree_t* str = (tree_t*)create_node("const_string", CONST_STRING_TYPE, sizeof(struct tree_string));
		str->string.pointer = $2;
		str->string.length = strlen($2);

		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.string = str;
		DBG("paramspec: %s\n", $2);
		$$ = node;
	}
	| DEFAULT expression ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.is_default = 1;
		node->paramspec.expr = $2;
		$$ = node;
	}
	| AUTO ';' {
		tree_t* node = (tree_t*)create_node("paramspec", SPEC_TYPE, sizeof(struct tree_paramspec));
		node->paramspec.is_auto = 1;
		$$ = node;
	}
	;

method_params
	:  {
		DBG("Waring: the method have no parameters \n");
		$$ = NULL;
	}
	| '(' cdecl_or_ident_list ')' {
		tree_t* node = (tree_t*)create_node("method_params", PARM_TYPE, sizeof(struct tree_params));
		node->params.have_in_param = 1;
		/* TODO: maybe we should get the pararmeters' type */
		node->params.in_argc = get_node_num($2);
		node->params.in_params = $2;
		node->params.have_ret_param = 0;
		node->params.ret_params = NULL;
		$$ = node;
	}
	| METHOD_RETURN '(' cdecl_or_ident_list ')' {
		tree_t* node = (tree_t*)create_node("method_params", PARM_TYPE, sizeof(struct tree_params));
		node->params.in_params = NULL;
		node->params.have_in_param = 0;
		node->params.have_ret_param = 1;
		node->params.ret_argc = get_node_num($3);
		/* TODO: maybe we should get the pararmeters' type */
		node->params.ret_params = $3;
		$$ = node;
	}
	| '(' cdecl_or_ident_list ')' METHOD_RETURN '(' cdecl_or_ident_list ')' {
		tree_t* node = (tree_t*)create_node("method_params", PARM_TYPE, sizeof(struct tree_params));
		node->params.have_in_param = 1;
		node->params.in_argc = get_node_num($2);
		/* TODO: maybe we should get the pararmeters' type */
		node->params.in_params = $2;
		node->params.have_ret_param = 1;
		node->params.ret_argc = get_node_num($6);
		/* TODO: maybe we should get the pararmeters' type */
		node->params.ret_params = $6;
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
		debug_proc("Line : %d\n", __LINE__);
		$$ = $3;
	}
	| {
		$$ = NULL;
	}
	;

sizespec
	: SIZE expression {
		debug_proc("Line : %d\n", __LINE__);
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
		debug_proc("Line : %d\n", __LINE__);
		/* FIXME: this should make special handle */
		DBG("\nPay attention: the default value should read template2!\n");
		$$ = NULL;
	}
	;

bitrange
	: '[' expression ']' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 1;
		node->array.expr = $2;
		$$ = node;
	}
	| '[' expression ':' expression ']' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 0;
		node->array.expr = $2;
		node->array.expr_end = $4;
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
		$$ = node;
	}
	| CONST basetype cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl))	;
		node->cdecl.state.is_const = 1;
		node->cdecl.basetype = $2;
		node->cdecl.decl = $3;
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
		debug_proc("Line : %d\n", __LINE__);
		$$ = $1;
	}
	| bitfields {
		debug_proc("Line : %d\n", __LINE__);
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
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.state.is_const = 1;
		node->cdecl.decl = $2;
		$$ = node;
	}
	| '*' cdecl2 {
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.state.is_point = 1;
		node->cdecl.decl = $2;
		$$ = node;
	}
	| VECT cdecl2 {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("cdecl", CDECL_TYPE, sizeof(struct tree_cdecl));
		node->cdecl.state.is_vect = 1;
		node->cdecl.decl = $2;
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
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("array_cdecl", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.is_fix = 1;
		node->array.expr = $3;
		node->array.decl = $1;
		$$ = node;
	}
	| cdecl3 '(' cdecl_list ')' {
		tree_t* node = (tree_t*)create_node("cdecl_brack", CDECL_BRACK_TYPE, sizeof(struct tree_cdecl_brack));
		node->cdecl_brack.cdecl = $1;
		node->cdecl_brack.decl_list = $3;
		$$ = node;
	}
	| '(' cdecl2 ')' {
		tree_t* node = (tree_t*)create_node("cdecl_brack", CDECL_BRACK_TYPE, sizeof(struct tree_cdecl_brack));
		node->cdecl_brack.cdecl = NULL;
		node->cdecl_brack.decl_list = $2;
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
		$$ = node;
	}
	| cdecl_list2 ',' cdecl {
		create_node_list($1, $3);
		$$ = $1;
	}
	| cdecl_list2 ',' ELLIPSIS {
		tree_t* node = (tree_t*)create_node("...", ELLIPSIS_TYPE, sizeof(struct tree_common));
		create_node_list($1, node);
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
		create_node_list($1, $3);
		$$ = $1;
	}
	;

typeof
	: TYPEOF expression {
		tree_t* node = (tree_t*)create_node("typeof", TYPEOF_TYPE, sizeof(struct tree_struct));
		node->typeof_tree.expr = $2;
		$$ = node;
	}
	;

struct
	: STRUCT '{' struct_decls '}' {
		tree_t* node = (tree_t*)create_node("struct", STRUCT_TYPE, sizeof(struct tree_struct));
		node->struct_tree.block = $3;
		$$ = node;
	}
	;

struct_decls
	: struct_decls cdecl ';' {
		create_node_list($1, $2);
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	;

layout
	:LAYOUT STRING_LITERAL '{' layout_decls '}' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("layout", LAYOUT_TYPE, sizeof(struct tree_layout));
		node->layout.name = $2;
		node->layout.block = $4;
		$$ = node;
	}
	;

layout_decls
	:layout_decls cdecl ';' {
		debug_proc("Line : %d\n", __LINE__);
		create_node_list($1, $2);
		$$ = $1;
	}
	|  {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

bitfields
	: BITFIELDS INTEGER_LITERAL '{' bitfields_decls '}' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("bitfields", BITFIELDS_TYPE, sizeof(struct tree_bitfields));
		node->bitfields.name = $2;
		node->bitfields.block = $4;
		$$ = node;
	}
	;

bitfields_decls
	: bitfields_decls cdecl '@' '[' expression ':' expression ']' ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("bitfields_decls", BITFIELDS_DECL_TYPE, sizeof(struct tree_bitfields_dec));
		node->bitfields_dec.decl = $2;
		node->bitfields_dec.start = $5;
		node->bitfields_dec.end = $7;
		if ($1 != NULL) {
			create_node_list($1, node);
			$$ = $1;
		}
		else {
			$$ = node;
		}
	}
	| {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

ctypedecl
	: const_opt basetype ctypedecl_ptr {
		tree_t* node = (tree_t*)create_node("ctypedecl", CTYPEDECL_TYPE, sizeof(sizeof(struct tree_ctypedecl)));
		node->ctypedecl.const_opt = $1;
		node->ctypedecl.basetype = $2;
		node->ctypedecl.ctypedecl_ptr = $3;
		$$ = node;
	}
	;

ctypedecl_ptr
	: stars ctypedecl_array {
		tree_t* node = (tree_t*)create_node("ctypedecl_ptr", CTYPEDECL_PTR_TYPE, sizeof(struct tree_ctypedecl_ptr));
		node->ctypedecl_ptr.stars = $1;
		node->ctypedecl_ptr.array = $2;
		$$ = node;
	}
	;

stars
	:  {
		$$ = NULL;
	}
	| '*' CONST stars {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("stars", STARS_TYPE, sizeof(struct tree_stars));
		node->stars.is_const = 1;
		node->stars.stars = $3;
		$$ = node;
	}
	| '*' stars {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("stars", STARS_TYPE, sizeof(struct tree_stars));
		node->stars.is_const = 0;
		node->stars.stars = $2;
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
		$$ = node;
	}
	| {
		$$ = NULL;
	}
	;

const_opt
	: CONST  {
		$$ = (tree_t*)c_keyword_node("const");
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
		$$ = (tree_t*)c_keyword_node("char");
	}
	| DOUBLE {
		$$ = (tree_t*)c_keyword_node("double");
	}
	| FLOAT {
		$$ = (tree_t*)c_keyword_node("float");
	}
	| INT {
		$$ = (tree_t*)c_keyword_node("int");
	}
	| LONG {
		$$ = (tree_t*)c_keyword_node("long");
	}
	| SHORT  {
		$$ = (tree_t*)c_keyword_node("short");
	}
	| SIGNED {
		$$ = (tree_t*)c_keyword_node("signed");
	}
	| UNSIGNED {
		$$ = (tree_t*)c_keyword_node("unsigned");
	}
	| VOID {
		$$ = (tree_t*)c_keyword_node("void");
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
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression ADD_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("add_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("+=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression SUB_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("sub_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("-=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression MUL_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("mul_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("*=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression DIV_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("div_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("/=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression MOD_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("mod_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("%=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression OR_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("or_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("|=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression AND_ASSIGN expression {
		tree_t* node = (tree_t*)create_node("and_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("&=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression XOR_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("xor_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("^=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression LEFT_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("left_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup("<<=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression RIGHT_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("right_assign", EXPR_ASSIGN_TYPE, sizeof(struct tree_expr_assign));
		node->expr_assign.assign_symbol = strdup(">>=");
		node->expr_assign.left = $1;
		node->expr_assign.right = $3;
		$$ = node;
	}
	| expression '?' expression ':' expression {
		tree_t* node = (tree_t*)create_node("ternary", TERNARY_TYPE, sizeof(struct tree_ternary));
		node->ternary.cond = $1;
		node->ternary.expr_true = $3;
		node->ternary.expr_false = $5;
		$$ = node;
	}
	| expression '+' expression {
		tree_t* node = (tree_t*)create_node("add", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("+");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '-' expression {
		tree_t* node = (tree_t*)create_node("sub", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("-");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '*' expression {
		tree_t* node = (tree_t*)create_node("mul", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("*");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '/' expression {
		tree_t* node = (tree_t*)create_node("div", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("/");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '%' expression {
		tree_t* node = (tree_t*)create_node("mod", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("%");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression LEFT_OP expression {
		tree_t* node = (tree_t*)create_node("left_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("<<");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression RIGHT_OP expression {
		tree_t* node = (tree_t*)create_node("right_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup(">>");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression EQ_OP expression {
		tree_t* node = (tree_t*)create_node("eq_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("==");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression NE_OP expression {
		tree_t* node = (tree_t*)create_node("ne_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("!=");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '<' expression {
		tree_t* node = (tree_t*)create_node("less", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("<");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '>' expression {
		tree_t* node = (tree_t*)create_node("great", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup(">");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression LE_OP expression {
		tree_t* node = (tree_t*)create_node("less_eq", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("<=");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression GE_OP expression {
		tree_t* node = (tree_t*)create_node("great_eq", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup(">=");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression OR_OP expression {
		tree_t* node = (tree_t*)create_node("or_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("||");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression AND_OP expression {
		tree_t* node = (tree_t*)create_node("and_op", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("&&");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '|' expression {
		tree_t* node = (tree_t*)create_node("or", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("|");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '^' expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("xor", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("^");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| expression '&' expression {
		tree_t* node = (tree_t*)create_node("and", BINARY_TYPE, sizeof(struct tree_binary));
		node->binary.operat = strdup("&");
		node->binary.left = $1;
		node->binary.right = $3;
		$$ = node;
	}
	| CAST '(' expression ',' ctypedecl ')' {
		tree_t* node = (tree_t*)create_node("cast", CAST_TYPE, sizeof(struct tree_cast));
		node->cast.expr = $3;
		node->cast.ctype = $5;
		$$ = node;
	}
	| SIZEOF expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("sizeof", SIZEOF_TYPE, sizeof(struct tree_sizeof));
		node->sizeof_tree.expr = $2;
		$$ = node;
	}
	| '-' expression {
		tree_t* node = (tree_t*)create_node("negative", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("-");
		node->unary.expr = $2;
		$$ = node;
	}
	| '+' expression {
		tree_t* node = (tree_t*)create_node("convert", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("+");
		node->unary.expr = $2;
		$$ = node;
	}
	| '!' expression {
		tree_t* node = (tree_t*)create_node("non_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("!");
		node->unary.expr = $2;
		$$ = node;
	}
	| '~' expression {
		tree_t* node = (tree_t*)create_node("bit_non", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("~");
		node->unary.expr = $2;
		$$ = node;
	}
	| '&' expression {
		tree_t* node = (tree_t*)create_node("addr", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("&");
		node->unary.expr = $2;
		$$ = node;
	}
	| '*' expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("pointer", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("*");
		node->unary.expr = $2;
		$$ = node;
	}
	| DEFINED expression {
		tree_t* node = (tree_t*)create_node("defined", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("defined");
		node->unary.expr = $2;
		$$ = node;
	}
	| '#' expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("translates", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("#");
		node->unary.expr = $2;
		$$ = node;
	}
	| INC_OP expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("pre_inc_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("pre_inc");
		node->unary.expr = $2;
		$$ = node;
	}
	| DEC_OP expression {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("pre_dec_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("pre_dec");
		node->unary.expr = $2;
		$$ = node;
	}
	| expression INC_OP {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("aft_inc_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("aft_inc");
		node->unary.expr = $1;
		$$ = node;
	}
	| expression DEC_OP {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("aft_dec_op", UNARY_TYPE, sizeof(struct tree_unary));
		node->unary.operat = strdup("aft_dec");
		node->unary.expr = $1;
		$$ = node;
	}
	| expression '(' expression_list ')' {
		tree_t* node  = (tree_t*)create_node("expr_brack", EXPR_BRACK_TYPE, sizeof(struct tree_expr_brack));
		node->expr_brack.expr = $1;
		node->expr_brack.expr_in_brack = $3;
		$$ = node;
	}
	| INTEGER_LITERAL {
		DBG("Integer_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst));
		node->int_cst.int_str = $1;
		node->int_cst.value = (int)strtoi($1);
		$$= node;
	}
	| STRING_LITERAL{
		DBG("String_literal: %s\n", $1);
		tree_t* node = (tree_t*)create_node("string_literal", CONST_STRING_TYPE, sizeof(struct tree_string));
		node->string.length = strlen($1);
		node->string.pointer = $1;
		$$=node;
	}
	| UNDEFINED {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("undefined", UNDEFINED_TYPE, sizeof(struct tree_common));
		$$ = node;
	}
	| '$' objident {
		DBG("In $objident: %s\n", $2);
		tree_t* node = (tree_t*)create_node("quote", QUOTE_TYPE, sizeof(struct tree_quote));
		node->quote.ident = $2;
		$$ = node;
	}
	| ident {
		DBG("ident: %s\n", $1);
		$$ = $1;
	}
	| expression '.' objident {
		tree_t* node = (tree_t*)create_node("dot", COMPONENT_TYPE, sizeof(struct tree_component));
		node->component.comp = strdup(".");
		node->component.expr = $1;
		node->component.ident = $3;
		$$ = node;
	}
	| expression METHOD_RETURN objident {
		tree_t* node = (tree_t*)create_node("pointer", COMPONENT_TYPE, sizeof(struct tree_component));
		node->component.comp = strdup("->");
		node->component.expr = $1;
		node->component.ident = $3;
		$$ = node;
	}
	| SIZEOFTYPE typeoparg {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("sizeoftype", SIZEOFTYPE_TYPE, sizeof(struct tree_sizeoftype));
		node->sizeoftype.typeoparg = $2;
		$$ = node;
	}
	;

typeoparg
	: ctypedecl {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("ctypedecl", TYPEOPARG_TYPE, sizeof(struct tree_sizeoftype));
		node->typeoparg.decl.ctypedecl = $1;
		$$ = node;
	}
	| '(' ctypedecl ')' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("ctypedecl_brack", TYPEOPARG_TYPE, sizeof(struct tree_sizeoftype));
		node->typeoparg.decl.ctypedecl_brack = $2;
		$$ = node;
	}
	;

expression
	: NEW ctypedecl {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("new", NEW_TYPE, sizeof(struct tree_new));
		node->new_tree.type = $2;
		$$ = node;
	}
	| NEW ctypedecl '[' expression ']' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("new", NEW_TYPE, sizeof(struct tree_new));
		node->new_tree.type = $2;
		node->new_tree.count = $4;
		$$ = node;
	}
	| '(' expression ')' {
		tree_t* node = (tree_t*)create_node("expr_brack", EXPR_BRACK_TYPE, sizeof(struct tree_expr_brack));
		 node->expr_brack.expr = NULL;
		 node->expr_brack.expr_in_brack = $2;
		 $$ = node;

	}
	| '[' expression_list ']' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("expr_array", ARRAY_TYPE, sizeof(struct tree_array));
		node->array.expr = $2;
		$$ = node;
	}
	| expression '[' expression endianflag ']' {
		tree_t* node = (tree_t*)create_node("bit_slicing_expr", BIT_SLIC_EXPR_TYPE, sizeof(struct tree_bit_slic));
		node->bit_slic.expr = $1;
		node->bit_slic.bit = $3;
		node->bit_slic.endian = $4;
		$$ = node;
	}
	| expression '[' expression ':' expression endianflag ']' {
		tree_t* node = (tree_t*)create_node("bit_slicing_expr", BIT_SLIC_EXPR_TYPE, sizeof(struct tree_bit_slic));
		node->bit_slic.expr = $1;
		node->bit_slic.bit = $3;
		node->bit_slic.bit_end = $5;
		node->bit_slic.endian = $6;
		$$ = node;
	}
	;

endianflag
	: ',' IDENTIFIER {
		tree_t* node = (tree_t*)create_node("endianflag", IDENT_TYPE, sizeof(struct tree_ident));
		node->ident.str = $2;
		node->ident.len = strlen($2);
		$$ = node;
	}
	| {
		$$ = NULL;
	}
	;

expression_opt
	: expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = $1;
	}
	| {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

comma_expression_opt
	: comma_expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = $1;
	}
	| {
		debug_proc("Line : %d\n", __LINE__);
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

statement
	: compound_statement {
		$$ = $1;
	}
	| local {
		$$ = $1;
	}
	| ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression ';'{
		DBG("expression in statement\n");
		$$ =  $1;
	}
	| IF '(' expression ')' statement {
		tree_t* node = (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		node->if_else.if_block = $5;
		$$ = node;
	}
	| IF '(' expression ')' statement ELSE statement {
		tree_t* node = (tree_t*)create_node("if_else", IF_ELSE_TYPE, sizeof(struct tree_if_else));
		node->if_else.cond = $3;
		node->if_else.if_block = $5;
		node->if_else.else_block = $7;
		$$ = node;
	}
	| WHILE '(' expression ')' statement {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("do_while", DO_WHILE_TYPE, sizeof(struct tree_do_while));
		node->do_while.cond = $3;
		node->do_while.block = $5;
		$$ = node;
	}
	| DO statement WHILE '(' expression ')' ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("do_while", DO_WHILE_TYPE, sizeof(struct tree_do_while));
		node->do_while.have_do = 1;
		node->do_while.cond = $5;
		node->do_while.block = $2;
		$$ = node;
	}
	| FOR '(' comma_expression_opt ';' expression_opt ';' comma_expression_opt ')' statement {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("for", FOR_TYPE, sizeof(struct tree_for));
		node->for_tree.init = $3;
		node->for_tree.cond = $5;
		node->for_tree.update = $7;
		node->for_tree.block = $9;
		$$ = node;
	}
	| SWITCH '(' expression ')' statement {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("switch", SWITCH_TYPE, sizeof(struct tree_switch));
		node->switch_tree.cond = $3;
		node->switch_tree.block = $5;
		$$ = node;
	}
	| DELETE expression ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("delete", DELETE_TYPE, sizeof(struct tree_delete));
		node->delete_tree.expr = $2;
		$$ = NULL;
	}
	| TRY statement CATCH statement {
		tree_t* node = (tree_t*)create_node("try_catch", TRY_CATCH_TYPE, sizeof(struct tree_try_catch));
		node->try_catch.try_block = $2;
		node->try_catch.catch_block = $4;
		DBG(" try catch in statement\n");
		$$ = node;
	}
	| AFTER '(' expression ')' CALL expression ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("after_call", AFTER_CALL_TYPE, sizeof(struct tree_after_call));
		node->after_call.cond = $3;
		node->after_call.call_expr = $6;
		DBG("AFTER CALL statement\n");
		$$ = node;
	}
	| CALL expression returnargs ';' {
		tree_t* node = (tree_t*)create_node("call", CALL_TYPE, sizeof(struct tree_call_inline));
		node->call_inline.expr = $2;
		node->call_inline.ret_args = $3;
		DBG("CALL statement\n");
		$$ = node;
	}
	| INLINE expression returnargs ';' {
		tree_t* node = (tree_t*)create_node("inline", INLINE_TYPE, sizeof(struct tree_call_inline));
		node->call_inline.expr = $2;
		node->call_inline.ret_args = $3;
		DBG("inline statement\n");
		$$ = node;
	}
	| ASSERT expression ';' {
		/* TODO: we should find the identifier from symbol table */
		tree_t* node = (tree_t*)create_node("assert", ASSERT_TYPE, sizeof(struct tree_assert));
		node->assert_tree.expr = $2;
		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ',' expression ':' STRING_LITERAL ',' log_args ';' {
		DBG("In LOG statement: %s\n", $8);
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log));
		node->log.log_type = $2;
		node->log.level = $4;
		node->log.group = $6;
		node->log.format = $8;
		/* TODO: step1: charge the print arg type and num in format */
		/* TODO: step2: get the actual arg type and num */
		/* TODO: step3: charge the args */
		node->log.argc = get_node_num($10);
		node->log.args = $10;

		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ':' STRING_LITERAL log_args ';' {
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log));
		node->log.log_type = $2;
		node->log.level = $4;
		node->log.format = $6;
		node->log.argc = get_node_num($7);
		node->log.args = $7;

		$$ = node;
	}
	| LOG STRING_LITERAL ':' STRING_LITERAL log_args ';' {
		tree_t* node = (tree_t*)create_node("log", LOG_TYPE, sizeof(struct tree_log));
		node->log.log_type = $2;
		node->log.format = $4;
		node->log.argc = get_node_num($5);
		node->log.args = $5;

		$$ = node;
	}
	| SELECT ident IN '(' expression ')' WHERE '(' expression ')' statement ELSE statement {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("select", SELECT_TYPE, sizeof(struct tree_select));
		node->select.ident = $2;
		node->select.in_expr = $5;
		node->select.cond = $9;
		node->select.where_block = $11;
		node->select.else_block = $13;
		$$ = node;
	}
	| FOREACH ident IN '(' expression ')' statement {
		tree_t* node = (tree_t*)create_node("foreach", FOREACH_TYPE, sizeof(struct tree_foreach));
		node->foreach.ident = $2;
		node->foreach.in_expr = $5;
		node->foreach.block = $7;
		DBG("FOREACH in statement\n");
		$$ = node;
	}
	| ident ':' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| CASE expression ':' statement {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("case", CASE_TYPE, sizeof(struct tree_case));
		/* TODO: charge the break */
		node->case_tree.expr = $2;
		node->case_tree.block = $4;
		$$ = node;
	}
	| DEFAULT ':' statement {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("default", DEFAULT_TYPE, sizeof(struct tree_default));
		node->default_tree.block = $3;
		$$ = node;
	}
	| GOTO ident ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("goto", GOTO_TYPE, sizeof(struct tree_goto));
		node->goto_tree.label = $2;
		$$ = node;
	}
	| BREAK ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("break", BREAK_TYPE, sizeof(struct tree_common));
		$$ = node;
	}
	| CONTINUE ';' {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("continue", CONTINUE_TYPE, sizeof(struct tree_common));
		$$ = node;
	}
	| THROW ';' {
		tree_t* node = (tree_t*)create_node("throw", THROW_TYPE, sizeof(struct tree_common));
		$$ = node;
	}
	| RETURN ';' {
		tree_t* node = (tree_t*)create_node("return", RETURN_TYPE, sizeof(struct tree_common));
		$$ = node;
	}
	| ERROR ';' {
		tree_t* node = (tree_t*)create_node("error", ERROR_TYPE, sizeof(struct tree_error));
		node->error.str = NULL;
		$$ = node;
	}
	| ERROR STRING_LITERAL ';' {
		tree_t* node = (tree_t*)create_node("error", ERROR_TYPE, sizeof(struct tree_error));
		node->error.str = $2;
		$$ = node;
	}
	;

log_args
	: {
		$$ = NULL;
	}
	| log_args ',' expression {
		create_node_list($1, $3);
		$$ = $1;
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
		create_node_list($1, $2);
		$$ = $1;
	}
	;

local_keyword
	: LOCAL {
		tree_t* node = (tree_t*)create_node("local_keyword", LOCAL_KEYWORD_TYPE, sizeof(struct tree_local_keyword));
		node->local_keyword.name = strdup("local");
		$$ = node;
	}
	| AUTO {
		debug_proc("Line : %d\n", __LINE__);
		tree_t* node = (tree_t*)create_node("local_keyword", LOCAL_KEYWORD_TYPE, sizeof(struct tree_local_keyword));
		node->local_keyword.name = strdup("local");
		$$ = node;
	}
	;

local
	: local_keyword cdecl ';' {
		tree_t* node = (tree_t*)create_node("local", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.local_keyword = $1;
		node->local_tree.cdecl = $2;
		$$ = node;
	}
	| STATIC cdecl ';' {
		debug_proc("Line : %d\n", __LINE__);
		DBG("In STATIC \n");
		tree_t* node = (tree_t*)create_node("local", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.is_static = 1;
		node->local_tree.cdecl = $2;
		$$ = node;
	}
	| local_keyword cdecl '=' expression ';' {
		tree_t* node = (tree_t*)create_node("local", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.local_keyword = $1;
		node->local_tree.cdecl = $2;
		node->local_tree.expr = $4;
		$$ = node;
	}
	| STATIC cdecl '=' expression ';' {
		tree_t* node = (tree_t*)create_node("local", LOCAL_TYPE, sizeof(struct tree_local));
		node->local_tree.is_static = 1;
		node->local_tree.cdecl = $2;
		node->local_tree.expr = $4;
		$$ = node;
	}
	;

objident_list
	: objident {
		debug_proc("Line : %d\n", __LINE__);
		$$ = $1;
	}
	| objident_list ',' objident {
		debug_proc("Line : %d\n", __LINE__);
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
		DBG("ident: %s\n", $1);
		$$ = $1;
	}
	| THIS {
		$$ = (tree_t*)dml_keyword_node("this");
	}
	| REGISTER {
		$$ = (tree_t*)dml_keyword_node("register");
	}
	| SIGNED {
		$$ = (tree_t*)c_keyword_node("signed");
	}
	| UNSIGNED {
		$$ = (tree_t*)c_keyword_node("unsigned");
	}
	;

ident
	: IDENTIFIER {
		tree_t* ident = (tree_t*)create_node("identifier", IDENT_TYPE, sizeof(struct tree_ident));
		ident->ident.str = $1;
		ident->ident.len = strlen($1);
		$$ = ident;
	}
	| ATTRIBUTE {
		$$ = (tree_t*)dml_keyword_node("attribute");
	}
	| BANK {
		$$ = (tree_t*)dml_keyword_node("bank");
	}
	| BITORDER {
		$$ = (tree_t*)dml_keyword_node("bitorder");
	}
	| CONNECT {
		$$ = (tree_t*)dml_keyword_node("connect");
	}
	| CONSTANT {
		$$ = (tree_t*)dml_keyword_node("constant");
	}
	| DATA {
		$$ = (tree_t*)dml_keyword_node("data");
	}
	| DEVICE {
		$$ = (tree_t*)dml_keyword_node("device");
	}
	| EVENT {
		$$ = (tree_t*)dml_keyword_node("event");
	}
	| FIELD {
		$$ = (tree_t*)dml_keyword_node("field");
	}
	| FOOTER {
		$$ = (tree_t*)dml_keyword_node("footer");
	}
	| GROUP {
		$$ = (tree_t*)dml_keyword_node("group");
	}
	| IMPLEMENT {
		$$ = (tree_t*)dml_keyword_node("implement");
	}
	| IMPORT {
		$$ = (tree_t*)dml_keyword_node("import");
	}
	| INTERFACE {
		$$ = (tree_t*)dml_keyword_node("interface");
	}
	| LOGGROUP {
		$$ = (tree_t*)dml_keyword_node("loggroup");
	}
	| METHOD {
		$$ = (tree_t*)dml_keyword_node("method");
	}
	| PORT {
		$$ = (tree_t*)dml_keyword_node("port");
	}
	| SIZE {
		$$ = (tree_t*)dml_keyword_node("size");
	}
	| CLASS {
		$$ = (tree_t*)dml_keyword_node("class");
	}
	| ENUM {
		$$ = (tree_t*)dml_keyword_node("enum");
	}
	| NAMESPACE {
		$$ = (tree_t*)dml_keyword_node("namespace");
	}
	| PRIVATE {
		$$ = (tree_t*)dml_keyword_node("private");
	}
	| PROTECTED {
		$$ = (tree_t*)dml_keyword_node("protected");
	}
	| PUBLIC {
		$$ = (tree_t*)dml_keyword_node("public");
	}
	| RESTRICT {
		$$ = (tree_t*)dml_keyword_node("register");
	}
	| UNION {
		$$ = (tree_t*)dml_keyword_node("union");
	}
	| USING {
		$$ = (tree_t*)dml_keyword_node("using");
	}
	| VIRTUAL {
		$$ = (tree_t*)dml_keyword_node("virtual");
	}
	| VOLATILE {
		$$ = (tree_t*)dml_keyword_node("volatile");
	}
	;

%%
#include <stdio.h>
#include <stdlib.h>

extern int column;
int lineno  = 1;    /* number of current source line */

void yyerror(yyscan_t* scanner, char *s) {
	fflush(stdout);
	fprintf(stderr,"Syntax error on line #%d, column #%d: %s\n", lineno, column, s);
	exit(1);
}
