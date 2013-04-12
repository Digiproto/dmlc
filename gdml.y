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
%parse-param { node_t** root_ptr }

%union  {
	int ival;
	char* sval;
	node_t* nodeval;
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
%type	<sval> objident
%type	<sval> maybe_objident
%type	<sval> ident
%type	<sval> STRING_LITERAL
%type	<sval> INTEGER_LITERAL
%type	<sval> object_desc
%type	<sval> typeident
%type	<sval> local_keyword
%type	<sval> const_opt
%type	<nodeval> dml
%type	<nodeval> syntax_modifiers
%type	<nodeval> device_statements
%type	<nodeval> device_statement
%type	<nodeval> syntax_modifier
%type	<nodeval> toplevel
%type	<nodeval> import
%type	<nodeval> parameter
%type	<nodeval> offsetspec
%type	<nodeval> object_statement
%type	<nodeval> method
%type	<nodeval> sizespec
%type	<nodeval> object
%type	<nodeval> object_spec
%type	<nodeval> object_statements
%type	<nodeval> istemplate_stmt
%type	<nodeval> statement
%type	<nodeval> compound_statement
%type	<nodeval> objident_list
%type	<nodeval> istemplate
%type	<nodeval> arraydef
%type	<nodeval> bitrange
%type	<nodeval> paramspec
%type	<nodeval> basetype
%type	<nodeval> expression
%type	<nodeval> comma_expression
%type	<nodeval> expression_list
%type	<nodeval> method_params
%type	<nodeval> method_def
%type	<nodeval> cdecl_or_ident_list
%type	<nodeval> cdecl_or_ident_list2
%type	<nodeval> cdecl
%type	<nodeval> cdecl2
%type	<nodeval> cdecl3
%type	<nodeval> cdecl_list
%type	<nodeval> cdecl_list2
%type	<nodeval> local
%type	<nodeval> typeof
%type	<nodeval> cdecl_or_ident
%type	<nodeval> statement_list
%type	<nodeval> log_args
%type	<nodeval> endianflag
%type	<nodeval> struct
%type	<nodeval> stars
%type	<nodeval> struct_decls
%type	<nodeval> returnargs
%type	<nodeval> ctypedecl_ptr
%type	<nodeval> ctypedecl
%type	<nodeval> ctypedecl_array
%type	<nodeval> ctypedecl_simple
%type	<nodeval> object_if
%type	<nodeval> layout
%type	<nodeval> layout_decls
%type	<nodeval> bitfields
%type	<nodeval> bitfields_decls
%type	<nodeval> typeoparg
%type	<nodeval> expression_opt
%type	<nodeval> comma_expression_opt

%%
begin_unit
	: DML INTEGER_LITERAL ';' dml {
		dml_attr_t* attr = (dml_attr_t*)malloc(sizeof(dml_attr_t));
		symbol_insert("DML", DML_TYPE, attr);

		if(*root_ptr != NULL) {
			/* something wrong */
			printf("root of ast already exists\n");
			exit(-1);
		}
		*root_ptr = create_node("DML", DML_TYPE);
		if($4 != NULL)
			add_child(*root_ptr, $4);
	}
	;

dml
	: DEVICE objident ';' syntax_modifiers device_statements {
		device_attr_t* attr = (device_attr_t*)malloc(sizeof(device_attr_t));
		attr->name = strdup($2);
		symbol_insert("DEVICE", DEVICE_TYPE, attr);
		node_t* node = create_node($2, DEVICE_TYPE);
		node_t* import_ast = NULL;
		if(builtin_filename != NULL) {
			import_ast = get_ast(builtin_filename);
			if(import_ast->child != NULL)
				create_node_list(node, import_ast->child);
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
			printf("maybe something Wrong\n");
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
			$$ = create_node_list($1, $2);
		}
		else {
			$$ = NULL;
		}
	}
	;

syntax_modifier
	: BITORDER ident ';' {
		DBG("In BITORDER: %s\n", $2);
		bitorder_attr_t*  attr = (bitorder_attr_t*)malloc(sizeof(bitorder_attr_t));
		memset(attr, 0, sizeof(bitorder_attr_t));
		attr->name = strdup($2);
		symbol_insert($2, BITORDER_TYPE, attr);
		node_t* bitorder = create_node($2, BITORDER_TYPE);
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		create_node_list(bitorder, semicolon);
		$$ = bitorder;
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
			$$ = create_node_list($1, $2);
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
		$$ = $1;
	}
	;

object
	: BANK maybe_objident istemplate object_spec {
		if ($2 == NULL) {
			fprintf(stderr, "There must be objident.\n");
			exit(-1);
		}
		DBG("BANK is %s\n", $2);

		bank_attr_t* attr = (bank_attr_t*)malloc(sizeof(bank_attr_t));
		attr->name = strdup($2);
		symbol_insert($2, BANK_TYPE, attr);

		node_t* bank = create_node($2, BANK_TYPE);
		if (($3 == NULL) && ($4 != NULL)) {
			add_child(bank, $4);
		}
		else if (($3 != NULL) && ($4 == NULL)) {
			add_child(bank, $3);
		}
		if(($3 != NULL) && ($4 != NULL)) {
			add_child(bank, $3);
			create_node_list($3, $4);
		}
		$$ = bank;
	}
	| REGISTER objident sizespec offsetspec istemplate object_spec {
		DBG("register is %s\n", $2);
		register_attr_t* attr = (register_attr_t*)malloc(sizeof(register_attr_t));
		memset(attr, 0, sizeof(register_attr_t));
		attr->name = strdup($2);
		attr->is_array = 0;
		if ($3 != NULL) {
			node_t* size = find_node($3, INTEGER_TYPE);
			if ((attr->size = (int)strtoi(size->name)) < 0) {
				fprintf(stderr, "The sizespec maybe wrong: %s\n", size->name);
				exit(-1);
			}
			DBG("sizespec: %s : %d\n", size->name, attr->size);
		}
		if ($4 != NULL) {
			node_t* offset = find_node($4, INTEGER_TYPE);
			if ((attr->offset = (int)strtoi(offset->name)) < 0) {
				fprintf(stderr, "The offsetspec maybe wrong: %s\n", offset->name);
				exit(-1);
			}
			DBG("offsetspec: %s : 0x%x\n", offset->name, attr->offset);
		}
		symbol_insert($2, REGISTER_TYPE, attr);

		node_t* reg = create_node($2, REGISTER_TYPE);
		if(($5 != NULL) && ($6 == NULL)) {
			DBG("add_child for register\n");
			add_child(reg, $5);
		}
		else if (($5 == NULL) && ($6 != NULL)) {
			DBG("add_child for register\n");
			add_child(reg, $6);

		}
		else  if (($5 != NULL) && ($6 != NULL)) {
			DBG("add_child for register\n");
			add_child(reg, $5);
			create_node_list($5, $6);
		}
		$$ = reg;
	}
	| REGISTER objident '[' arraydef ']' sizespec offsetspec istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		DBG("Register is %s\n", $2);
		register_attr_t* attr = (register_attr_t*)malloc(sizeof(register_attr_t));
		memset(attr, 0, sizeof(register_attr_t));
		attr->name = strdup($2);
		attr->is_array = 1;
		if ($6 != NULL) {
			node_t* size = find_node($6, INTEGER_TYPE);
			if ((attr->size = (int)strtoi(size->name)) < 0) {
				fprintf(stderr, "The sizespec maybe wrong: %s\n", size->name);
				exit(-1);
			}
			DBG("sizespec: %s : %d\n", size->name, attr->size);
		}
		if ($7 != NULL) {
			node_t* offset = find_node($7, INTEGER_TYPE);
			if ((attr->offset = (int)strtoi(offset->name)) < 0) {
				fprintf(stderr, "The offsetspec maybe wrong: %s\n", offset->name);
				exit(-1);
			}
			DBG("offsetspec: %s : 0x%x\n", offset->name, attr->offset);
		}
		symbol_insert($2, REGISTER_TYPE, attr);

		node_t* reg = create_node($2, REGISTER_TYPE);
		if ($4 == NULL) {
			fprintf(stderr, "In range register, the arraydef is must!\n");
			exit(-1);
		}
		DBG("add_child for register\n");
		add_child(reg, $4);
		if ($8 != NULL) {
			create_node_list($4, $8);
		}
		if ($9 != NULL) {
			create_node_list($4, $9);
		}
		$$ = reg;
	}
	| FIELD objident bitrange istemplate object_spec {
		if ($2 == NULL) {
			fprintf(stderr, "There must be objident\n");
			exit(-1);
		}
		field_attr_t* attr = (field_attr_t*)malloc(sizeof(field_attr_t));
		memset(attr, 0, sizeof(field_attr_t));
		attr->name = strdup($2);
		attr->is_range = 1;
		symbol_insert($2, FIELD_TYPE, attr);

		node_t* field = create_node($2, FIELD_TYPE);
		add_child(field, $3);
		if ($4 != NULL) {
			create_node_list($3, $4);
		}
		create_node_list($3, $5);
		$$ = field;
	}
	| FIELD objident istemplate object_spec {
		field_attr_t* attr = (field_attr_t*)malloc(sizeof(field_attr_t));
		memset(attr, 0, sizeof(field_attr_t));
		attr->name = strdup($2);
		attr->is_range = 0;
		symbol_insert($2, FIELD_TYPE, attr);

		node_t* field = create_node($2, FIELD_TYPE);
		if (($3 != NULL) && ($4 == NULL)) {
			add_child(field, $3);
		}
		else if (($3 == NULL) && ($4 != NULL)) {
			add_child(field, $4);
		}
		else if (($3 != NULL) && ($4 != NULL)) {
			add_child(field, $3);
			create_node_list($3, $4);
		}
		$$ = field;
	}
	| DATA cdecl ';' {
		node_t* node = find_node($2, INDENTIFIER_TYPE);
		data_attr_t* attr = malloc(sizeof(data_attr_t));
		memset(attr, 0, sizeof(data_attr_t));
		attr->name = strdup(node->name);
		symbol_insert(node->name, DATA_TYPE, attr);

		node_t* data = create_node(node->name, DATA_TYPE);
		add_child(data, $2);
		$$ = data;
	}
	| CONNECT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = create_node($2, CONNECT_TYPE);
		DBG("CONNECT_TYPE: %s\n", $2);
	}
	| INTERFACE objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = create_node($2, INTERFACE_TYPE);
		DBG("Interface_type: %s\n", $2);
	}
	| ATTRIBUTE objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = create_node($2, ATTRIBUTE_TYPE);
		DBG("Attribute: %s\n", $2);
	}
	| EVENT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| GROUP objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| PORT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| IMPLEMENT objident istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = create_node($2, IMPLEMENT_TYPE);
		DBG("objident: %s\n", $2);
	}
	| ATTRIBUTE objident '[' arraydef ']' istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| GROUP objident '[' arraydef ']' istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| CONNECT objident '[' arraydef ']' istemplate object_spec {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

method
	: METHOD objident method_params method_def {
		method_attr_t* attr = (method_attr_t*)malloc(sizeof(method_attr_t));
		memset(attr, 0, sizeof(method_attr_t));
		attr->name = strdup($2);
		symbol_insert($2, METHOD_TYPE, attr);

		node_t* node = create_node($2, METHOD_TYPE);
		if ($3 != NULL) {
			add_child(node, $3);
			create_node_list($3, $4);
		}
		else {
			add_child(node, $4);
		}
		DBG("method is %s\n", $2);
		$$ = node;
	}
	| METHOD EXTERN objident method_params method_def {
		node_t* node = create_node($3, METHOD_EXTERN_TYPE);
		if ($4 != NULL) {
			add_child(node, $4);
			create_node_list($4, $5);
		}
		else {
			add_child(node, $5);
		}
		DBG("method extern is %s\n", $3);
		$$ = node;

	}
	;

arraydef
	: expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = $1;
	}
	| ident IN expression RANGE_SIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		if ($1 == NULL) {
			fprintf(stderr, "There must be identifier!\n");
			exit(-1);
		}
		node_t* node = create_node($1, ARRAY_RANGE_TYPE);
		add_child(node, $3);
		create_node_list($3, $5);
		$$ = node;
	}
	;

toplevel
	: TEMPLATE objident object_spec {
		DBG("in TEMPLATE %s\n", $2);
		template_attr_t* attr = malloc(sizeof(template_attr_t));
		memset(attr, 0, sizeof(template_attr_t));
		attr->name = strdup($2);
		symbol_insert($2, TEMPLATE_TYPE, attr);

		node_t* node = create_node($2, TEMPLATE_TYPE);
		add_child(node, $3);
		$$ = node;
	}
	| LOGGROUP ident ';' {
		DBG("in LOGGROUP %s\n", $2);
		symbol_insert($2, LOGGROUP_TYPE, NULL);

		node_t* node = create_node($2, LOGGROUP_TYPE);
		$$ = node;
	}
	| CONSTANT ident '=' expression ';' {
		constant_attr_t* attr = malloc(sizeof(constant_attr_t));
		memset(attr, 0, sizeof(attr));
		/* FIXME, should provide a correct expression value */
		attr->value = NULL;
		symbol_insert($2, CONSTANT_TYPE, attr);
		node_t* node = create_node($2, CONSTANT_TYPE);
		node_t* assign = create_node("=", ASSIGN_TYPE);
		create_node_list(node, assign);
		add_child(assign, $4);
		$$ = node;
	}
	| EXTERN cdecl_or_ident ';' {
		/* FIXME: we should find the name of extern */
		//symbol_t* symbol = symbol_find()
		DBG("\nPay attention: we should find the name of extern\n\n");
		node_t* extern_key = create_node("extern", EXTERN_KEY_TYPE);
		add_child(extern_key, $2);
		$$ = extern_key;
	}
	| TYPEDEF cdecl ';' {
		/* FIXME: we should find the name cdecl */
		DBG("\nPay attention: we should find the name of typedef cdecl\n\n");
		//$$ = create_node("UNIMP", TYPEDEF_TYPE);
		node_t* typedef_key = create_node("typedef", TYPEDEF_TYPE);
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		add_child(typedef_key, $2);
		create_node_list($2, semicolon);
		$$ = typedef_key;
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
		$$ = create_node($1, HEADER_TYPE);
	}
	;

istemplate_stmt
	: IS objident ';' {
		DBG("In IS statement\n");
		symbol_t* symbol = symbol_find($2, TEMPLATE_TYPE);	
		if(symbol == NULL){
			fprintf(stderr, "Pay attention: No such template %s in IS statement\n", $2);
		}
		node_t* temp = create_node("is", ISTEMPLATE_TYPE);
		node_t* temp_name = create_node($2, CONST_STRING_TYPE);
		add_child(temp, temp_name);
		$$ = temp;
	}
	;

import
	: IMPORT STRING_LITERAL ';' {
		DBG("import file is %s\n", $2);
		char fullname[1024];
		int dirlen = strlen(dir);
		int filelen = strlen($2);

		/* FIXME, should check if the same filename is imported already. */
		symbol_insert($2, IMPORT_TYPE, NULL);
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
		node_t* root = create_node($2, IMPORT_TYPE);
		DBG("Begin parse the import file %s\n", fullname);
		node_t* ast = NULL;
		yylex_init(&scanner);
		yyrestart(file, scanner);
		yyparse(scanner, &ast);
		yylex_destroy(scanner);
		fclose(file);
		DBG("End of parse the import file %s\n", fullname);

		$$ = ast->child;
	}
	;

object_desc
	: STRING_LITERAL {
		$$ = $1;
	}
	| {
		$$ = NULL;
	}
	;

object_spec
	: object_desc ';' {
		if ($1 == NULL) {
			node_t* semicolon = create_node(";", SEMICOLON_TYPE);
			$$ = semicolon;
		}
		else {
			node_t* node = create_node($1, CONST_STRING_TYPE);
			node_t* semicolon = create_node(";", SEMICOLON_TYPE);
			create_node_list(node, semicolon);
			$$ = node;
		}
	}
	| object_desc '{' object_statements '}' {
		DBG("object_statements for object_spec\n");
		node_t* pre_braces = create_node("{", PRE_BRACES_TYPE);
		node_t* aft_braces = create_node("}", AFTER_BRACES_TYPE);
		if (($1 == NULL) && ($3 == NULL)) {
			DBG("Pay attention: there should be object description!\n");
			create_node_list(pre_braces, aft_braces);
			$$ = pre_braces;
		}
		else if (($1 == NULL) && ($3 != NULL)) {
			create_node_list(pre_braces, $3);
			create_node_list(pre_braces, aft_braces);
			$$ = pre_braces;
		}
		else if (($1 != NULL) && ($3 == NULL)) {
			node_t* node = create_node($1, CONST_STRING_TYPE);
			create_node_list(node, pre_braces);
			create_node_list(node, aft_braces);
			$$ = node;
		}
		else if (($1 != NULL) && ($3 != NULL)) {
			node_t* node = create_node($1, CONST_STRING_TYPE);
			create_node_list(node, pre_braces);
			create_node_list(node, $3);
			create_node_list(node, aft_braces);
			$$ = node;
		}
	}
	;

object_statements
	: object_statements object_statement {
		DBG("In object_statements\n");
		if($1 == NULL && $2 != NULL) {
			$$ = $2;
		}
		else if($1 != NULL && $2 != NULL) {
			$$ = create_node_list($1, $2);
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
		$$ = NULL;
	}
	;

object_if
	: IF '(' expression ')' '{' object_statements '}' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| IF '(' expression ')' '{' object_statements '}' ELSE '{' object_statements '}' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| IF '(' expression ')' '{' object_statements '}' ELSE object_if {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

parameter
	: PARAMETER objident paramspec {
		parameter_attr_t* attr = malloc(sizeof(parameter_attr_t));
		memset(attr, 0, sizeof(parameter_attr_t));
		attr->name = strdup($2);
		symbol_insert($2, PARAMETER_TYPE, attr);

		node_t* node = create_node($2, PARAMETER_TYPE);
		if ($3 != NULL) {
			add_child(node, $3);
		}
		DBG("parameter name is %s\n", $2);
		$$ = node;
	}
	;

paramspec
	: ';' {
		node_t* node = create_node(";", SEMICOLON_TYPE);
		$$ = node;
	}
	| '=' expression ';' {
		node_t* node = create_node("=", ASSIGN_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '=' STRING_LITERAL ';' {
		node_t* node = create_node("=", ASSIGN_TYPE);
		node_t* str = create_node($2, CONST_STRING_TYPE);
		add_child(node, str);
		DBG("paramspec: %s\n", $2);
		$$ = node;
	}
	| DEFAULT expression ';' {
		node_t* node = create_node("default", DEFAULT_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| AUTO ';' {
		node_t* node = create_node("auto", AUTO_TYPE);
		$$ = node;
	}
	;

method_params
	:  {
		DBG("Pay attention: the method parameters maybe some problems\n");
		$$ = NULL;
	}
	| '(' cdecl_or_ident_list ')' {
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		if ($2 == NULL) {
			create_node_list(pre_brack, aft_brack);
		}
		else {
			create_node_list(pre_brack, $2);
			create_node_list(pre_brack, aft_brack);
		}
		$$ = pre_brack;
	}
	| METHOD_RETURN '(' cdecl_or_ident_list ')' {
		node_t* method_ret = create_node("->", METHOD_RETURN_TYPE);
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		if ($3 == NULL) {
			create_node_list(method_ret, pre_brack);
			create_node_list(method_ret, aft_brack);
		}
		else {
			create_node_list(method_ret, pre_brack);
			create_node_list(method_ret, $3);
			create_node_list(method_ret, aft_brack);
		}
		$$ = method_ret;
	}
	| '(' cdecl_or_ident_list ')' METHOD_RETURN '(' cdecl_or_ident_list ')' {
		node_t* method_ret = create_node("->", METHOD_RETURN_TYPE);
		node_t* pre_brack_1 = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack_1 = create_node(")", AFTER_BRACKETS_TYPE);
		node_t* pre_brack_2 = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack_2 = create_node(")", AFTER_BRACKETS_TYPE);

		if (($2 == NULL) && ($6 == NULL)) {
			create_node_list(pre_brack_1, aft_brack_1);
			create_node_list(pre_brack_1, method_ret);
			create_node_list(pre_brack_1, pre_brack_2);
			create_node_list(pre_brack_1, aft_brack_2);
			$$ = pre_brack_1;
		}
		else if (($2 == NULL) && ($6 != NULL)) {
			create_node_list(pre_brack_1, aft_brack_1);
			create_node_list(pre_brack_1, method_ret);
			create_node_list(pre_brack_1, pre_brack_2);
			create_node_list(pre_brack_1, $6);
			create_node_list(pre_brack_1, aft_brack_2);
			$$ = pre_brack_1;
		}
		else if (($2 != NULL) && ($6 == NULL)) {
			create_node_list(pre_brack_1, $2);
			create_node_list(pre_brack_1, aft_brack_1);
			create_node_list(pre_brack_1, method_ret);
			create_node_list(pre_brack_1, pre_brack_2);
			create_node_list(pre_brack_1, aft_brack_2);
			$$ = pre_brack_1;
		}
		else {
			create_node_list(pre_brack_1, $2);
			create_node_list(pre_brack_1, aft_brack_1);
			create_node_list(pre_brack_1, method_ret);
			create_node_list(pre_brack_1, pre_brack_2);
			create_node_list(pre_brack_1, $6);
			create_node_list(pre_brack_1, aft_brack_2);
			$$ = pre_brack_1;
		}
		DBG("with METHOD_RETURN in method_params\n");
	}
	;

returnargs
	: {
		$$ = NULL;
	}
	| METHOD_RETURN '(' expression_list ')' {
		node_t* method_ret = create_node("->", METHOD_RETURN_TYPE);
        node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
        node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);

		create_node_list(method_ret, pre_brack);
		if ($3 != NULL) {
			create_node_list(method_ret, $3);
		}
		create_node_list(method_ret, aft_brack);
		$$ = method_ret;
	}
	;

method_def
	: compound_statement {
		DBG("compound_statement in method_def\n");
		$$ = $1;
	}
	| DEFAULT compound_statement {
		node_t* node = create_node("default", DEFAULT_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	;

istemplate
	: IS '(' objident_list ')' {
		debug_proc("Line : %d\n", __LINE__);
		node_t* node = create_node("istemplate", ISTEMPLATE_TYPE);
		add_child(node, $3);
		$$ = node;
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
		node_t* reg_offset = create_node("@", REG_OFFSET_TYPE);
		add_child(reg_offset, $2);
		$$ = reg_offset;
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
		node_t* pre_mid_brace =  create_node("[", PRE_MID_BRACES_TYPE);
		node_t* aft_mid_brace = create_node("]", AFTER_MID_BRACES_TYPE);
		create_node_list(pre_mid_brace, $2);
		create_node_list(pre_mid_brace, aft_mid_brace);
		$$ = pre_mid_brace;
	}
	| '[' expression ':' expression ']' {
		debug_proc("Line : %d\n", __LINE__);
		node_t* pre_mid_brace =  create_node("[", PRE_MID_BRACES_TYPE);
		node_t* aft_mid_brace = create_node("]", AFTER_MID_BRACES_TYPE);
		node_t* colon = create_node(":", COLON_TYPE);
		create_node_list(pre_mid_brace, $2);
		create_node_list(pre_mid_brace, colon);
		create_node_list(pre_mid_brace, $4);
		create_node_list(pre_mid_brace, aft_mid_brace);
		$$ = pre_mid_brace;
	}
	;

cdecl_or_ident
	: cdecl {
		$$ = $1;
	}
	;

cdecl
	: basetype cdecl2 {
		if ($2 == NULL) {
			$1->type = INDENTIFIER_TYPE;
			$$ = $1;
		}
		else {
			create_node_list($1, $2);
			$$ = $1;
		}
	}
	| CONST basetype cdecl2 {
		node_t* const_key = create_node("const", CONST_KEY_TYPE);
		add_child(const_key, $2);
		create_node_list($2, $3);
		$$ = const_key;
	}
	;

basetype
	: typeident {
		DBG("typeident: %s\n", $1);
		node_t* node = create_node($1, BASETYPE_TYPE);
		$$ = node;
	}
	| struct {
		DBG("struct\n");
		$$ = $1;
	}
	| layout {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| bitfields {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
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
		$$ = NULL;
	}
	| '*' cdecl2 {
		node_t* node = create_node("*", POINT_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| VECT cdecl2 {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

cdecl3
	: typeident {
		DBG("typeident: %s\n", $1);
		node_t* typeid = create_node($1, INDENTIFIER_TYPE);
		$$ = typeid;
	}
	| {
		$$ = NULL;
	}
	| cdecl3 '[' expression ']' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| cdecl3 '(' cdecl_list ')' {
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		if (($1 != NULL) && ($3 != NULL)) {
			create_node_list($1, pre_brack);
			create_node_list($1, $3);
			create_node_list($1, aft_brack);
			$$ = $1;
		}
		else if (($1 != NULL) && ($3 == NULL)) {
			create_node_list($1, pre_brack);
			create_node_list($1, aft_brack);
			$$ = $1;
		}
		else if (($1 == NULL) && ($3 != NULL)) {
			create_node_list(pre_brack, $3);
			create_node_list(pre_brack, aft_brack);
			$$ = pre_brack;
		}
		else {
			create_node_list(pre_brack, aft_brack);
			$$ = pre_brack;
		}
	}
	| '(' cdecl2 ')' {
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t*  aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		if ($2 != NULL) {
			create_node_list(pre_brack, $2);
		}
		create_node_list(pre_brack, aft_brack);
		$$ = pre_brack;
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
		$$ = create_node("...", ELLIPSIS_TYPE);
	}
	| cdecl_list2 ',' cdecl {
		create_node_list($1, $3);
		$$ = $1;
	}
	| cdecl_list2 ',' ELLIPSIS {
		node_t* ellipsis = create_node("...", ELLIPSIS_TYPE);
		create_node_list($1, ellipsis);
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
		node_t* node = create_node("typeof", TYPEOF_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	;

struct
	: STRUCT '{' struct_decls '}' {
		node_t* node = create_node("struct", STRUCT_TYPE);
		node_t* pre_braces = create_node("{", PRE_BRACES_TYPE);
		node_t* aft_braces = create_node("}", AFTER_BRACES_TYPE);
		create_node_list(node, pre_braces);
		if ($3 != NULL) {
			create_node_list(node, $3);
		}
		create_node_list(node, aft_braces);
		$$ = node;
	}
	;

struct_decls
	: struct_decls cdecl ';' {
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		if (($1 == NULL) && ($2 == NULL)) {
			fprintf(stderr, "The struct have something wrong.\n");
			exit(-1);
		}
		else if (($1 == NULL) && ($2 != NULL)) {
			create_node_list($2, semicolon);
			$$ = $2;
		}
		else if (($1 != NULL) && ($2 == NULL)) {
			create_node_list($1, semicolon);
			$$ = $1;
		}
		else {
			create_node_list($1, $2);
			create_node_list($1, semicolon);
			$$ = $1;
		}
	}
	| {
		$$ = NULL;
	}
	;

layout
	:LAYOUT STRING_LITERAL '{' layout_decls '}' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

layout_decls
	:layout_decls cdecl ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	|  {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

bitfields
	: BITFIELDS INTEGER_LITERAL '{' bitfields_decls '}' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

bitfields_decls
	: bitfields_decls cdecl '@' '[' expression ':' expression ']' ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

ctypedecl
	: const_opt basetype ctypedecl_ptr {
		if ($2 == NULL) {
			fprintf(stderr, "There must be indentifier or basetype.\n");
			exit(-1);
		}
		else if (($1 != NULL) && ($3 != NULL)) {
			node_t* node = create_node("const", CONST_KEY_TYPE);
			create_node_list(node, $2);
			create_node_list(node, $3);
			$$ = node;
		}
		else if (($1 != NULL) && ($3 == NULL)) {
			node_t* node = create_node("const", CONST_KEY_TYPE);
			create_node_list(node, $2);
			$$ = node;
		}
		else if (($1 == NULL) && ($3 != NULL)) {
			create_node_list($2, $3);
			$$ = $2;
		}
		else if (($1 == NULL) && ($3 == NULL)) {
			$$ = $2;
		}
	}
	;

ctypedecl_ptr
	: stars ctypedecl_array {
		if (($1 != NULL) && ($2 != NULL)) {
			create_node_list($1, $2);
			$$ = $1;
		}
		else if (($1 != NULL) && ($2 == NULL)) {
			$$ = $1;
		}
		else if (($1 == NULL) && ($2 != NULL)) {
			$$ = $2;
		}
		else {
			$$ = NULL;
		}
	}
	;

stars
	:  {
		$$ = NULL;
	}
	| '*' CONST stars {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| '*' stars {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

ctypedecl_array
	: ctypedecl_simple {
		$$ = $1;
	}
	;

ctypedecl_simple
	: '(' ctypedecl_ptr ')' {
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		if ($2 != NULL) {
			create_node_list(pre_brack, $2);
		}
		create_node_list(pre_brack, aft_brack);
		$$ = pre_brack;
	}
	| {
		$$ = NULL;
	}
	;

const_opt
	: CONST  {
		$$ = "const";
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
		$$ = "char";
	}
	| DOUBLE {
		$$ = "double";
	}
	| FLOAT {
		$$ = "float";
	}
	| INT {
		$$ = "int";
	}
	| LONG {
		$$ = "long";
	}
	| SHORT  {
		$$ = "short";
	}
	| SIGNED {
		$$ = "signed";
	}
	| UNSIGNED {
		$$ = "unsigned";
	}
	| VOID {
		$$ = "void";
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
		node_t* assign = create_node("=", ASSIGN_TYPE);
		create_node_list($1, assign);
		add_child(assign, $3);
		$$ = $1;
	}
	| expression ADD_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression SUB_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression MUL_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression DIV_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression MOD_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression OR_ASSIGN expression {
		node_t* or_assign = create_node("|=", OR_ASSIGN_TYPE);
		create_node_list($1, or_assign);
		add_child(or_assign, $3);
		$$ = $1;
	}
	| expression AND_ASSIGN expression {
		node_t* and_assign = create_node("&=", AND_ASSIGN_TYPE);
		create_node_list($1, and_assign);
		add_child(and_assign, $3);
		$$ = $1;
	}
	| expression XOR_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression LEFT_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression RIGHT_ASSIGN expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression '?' expression ':' expression {
		node_t* quest_mark = create_node("?", QUEST_MARK_TYPE);
		node_t* colon = create_node(":", COLON_TYPE);
		create_node_list($1, quest_mark);
		create_node_list($1, $3);
		create_node_list($1, colon);
		create_node_list($1, $5);
		$$ = $1;
	}
	| expression '+' expression {
		node_t* node = create_node("+", ADD_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression '-' expression {
		node_t* minus = create_node("-", MINUS_TYPE);
		create_node_list($1, minus);
		add_child(minus, $3);
		$$ = $1;
	}
	| expression '*' expression {
		node_t* node = create_node("*", MUL_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression '/' expression {
		node_t* node = create_node("/", DIV_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression '%' expression {
		node_t* node = create_node("%", REMAIN_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression LEFT_OP expression {
		node_t* node = create_node("<<", LEFT_OP_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression RIGHT_OP expression {
		node_t* node = create_node(">>", RIGHT_OP_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression EQ_OP expression {
		node_t* node = create_node("==", EQ_OP_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression NE_OP expression {
		node_t* en_op = create_node("!=", EN_OP_TYPE);
		create_node_list($1, en_op);
		add_child(en_op, $3);
		$$ = $1;
	}
	| expression '<' expression {
		node_t* node = create_node("<", LESS_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression '>' expression {
		node_t* node = create_node(">", GREATER_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression LE_OP expression {
		node_t* node = create_node("<=", LE_OP_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression GE_OP expression {
		node_t* node = create_node(">=", GE_OP_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression OR_OP expression {
		node_t* or_op = create_node("||", OR_OP_TYPE);
		create_node_list($1, or_op);
		add_child(or_op, $3);
		$$ = $1;
	}
	| expression AND_OP expression {
		node_t* and_op = create_node("&&", AND_OP_TYPE);
		create_node_list($1, and_op);
		add_child(and_op, $3);
		$$ = $1;
	}
	| expression '|' expression {
		node_t* node = create_node("|", OR_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| expression '^' expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression '&' expression {
		node_t* node = create_node("&", AND_TYPE);
		create_node_list($1, node);
		add_child(node, $3);
		$$ = $1;
	}
	| CAST '(' expression ',' ctypedecl ')' {
		node_t* node = create_node("cast", CAST_TYPE);
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		if ($5 == NULL) {
			fprintf(stderr, "There must be something wrong.\n");
			exit(-1);
		}
		create_node_list(node, pre_brack);
		create_node_list(node, $3);
		create_node_list(node, $5);
		create_node_list(node, aft_brack);
		$$ = node;
	}
	| SIZEOF expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| '-' expression {
		node_t* node = create_node("-", NEGAT_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '+' expression {
		node_t* node = create_node("+", POSIT_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '!' expression {
		node_t* node = create_node("!", EXCLAM_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '~' expression {
		node_t* node = create_node("~", NON_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '&' expression {
		node_t* node = create_node("&", ADDRESS_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '*' expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| DEFINED expression {
		node_t* node = create_node("defined", DEFINED_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| '#' expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| INC_OP expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| DEC_OP expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression INC_OP {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression DEC_OP {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression '(' expression_list ')' {
		node_t* pre_brackets = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brackets = create_node(")", AFTER_BRACKETS_TYPE);
		create_node_list($1, pre_brackets);
		if ($3 != NULL) {
			create_node_list($1, $3);
		}
		create_node_list($1, aft_brackets);
		$$ = $1;
	}
	| INTEGER_LITERAL {
		DBG("Integer_literal: %s\n", $1);
		node_t* integer = create_node($1, INTEGER_TYPE);
		$$= integer;
	}
	| STRING_LITERAL{
		DBG("String_literal: %s\n", $1);
		node_t* node = create_node($1, CONST_STRING_TYPE);
		$$=node;
	}
	| UNDEFINED {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| '$' objident {
		node_t* node = create_node("$", QUOTE_TYPE);
		node_t* ident = create_node($2, CONST_STRING_TYPE);
		create_node_list(node, ident);
		DBG("In $objident: %s\n", $2);
		$$ = node;
	}
	| ident {
		DBG("ident: %s\n", $1);
		node_t* node = create_node($1, IDENTIFER_TYPE);
		$$ = node;
	}
	| expression '.' objident {
		node_t* period = create_node(".", PERIOD_TYPE);
		node_t* objident = create_node($3, INDENTIFIER_TYPE);
		create_node_list($1, period);
		add_child(period, objident);
		$$ = $1;
	}
	| expression METHOD_RETURN objident {
		node_t* method_ret = create_node("->", METHOD_RETURN_TYPE);
		node_t* objident = create_node($3, INDENTIFIER_TYPE);
		create_node_list($1, method_ret);
		create_node_list($1, objident);
		$$ = $1;
	}
	| SIZEOFTYPE typeoparg {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

typeoparg
	: ctypedecl {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| '(' ctypedecl ')' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

expression
	: NEW ctypedecl {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| NEW ctypedecl '[' expression ']' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| '(' expression ')' {
		node_t* pre_brackets = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brackets = create_node(")", AFTER_BRACKETS_TYPE);
		create_node_list(pre_brackets, $2);
		create_node_list(pre_brackets, aft_brackets);
		$$ = pre_brackets;
	}
	| '[' expression_list ']' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| expression '[' expression endianflag ']' {
		node_t* pre_mid_brace = create_node("[", PRE_MID_BRACES_TYPE);
		node_t* aft_mid_brace = create_node("]", AFTER_MID_BRACES_TYPE);
		create_node_list($1, pre_mid_brace);
		create_node_list($1, $3);
		if ($4 != NULL) {
			create_node_list($1, $4);
		}
		create_node_list($1, aft_mid_brace);
		$$ = $1;
	}
	| expression '[' expression ':' expression endianflag ']' {
		node_t* pre_mid_brace = create_node("[", PRE_MID_BRACES_TYPE);
		node_t* aft_mid_brace = create_node("]", AFTER_MID_BRACES_TYPE);
		node_t* colon = create_node(":", COLON_TYPE);
		create_node_list($1, pre_mid_brace);
		create_node_list($1, $3);
		create_node_list($1, colon);
		create_node_list($1, $5);
		if ($6 != NULL) {
			create_node_list($1, $6);
		}
		create_node_list($1, aft_mid_brace);
		$$ = $1;
	}
	;

endianflag
	: ',' IDENTIFIER {
		node_t* comma = create_node(",", COMMA_TYPE);
		node_t* indent = create_node($2, INDENTIFIER_TYPE);
		create_node_list(comma, indent);
		$$ = comma;
	}
	| {
		$$ = NULL;
	}
	;

expression_opt
	: expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

comma_expression_opt
	: comma_expression {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
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
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		create_node_list($1, semicolon);
		$$ =  $1;
	}
	| IF '(' expression ')' statement {
		node_t* if_node = create_node("if", IF_TYPE);
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		add_child(if_node, pre_brack);
		create_node_list(pre_brack, $3);
		create_node_list(pre_brack, aft_brack);
		create_node_list(pre_brack, $5);
		$$ = if_node;
	}
	| IF '(' expression ')' statement ELSE statement {
		node_t* if_node = create_node("if", IF_TYPE);
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		node_t* else_node = create_node("else", ELSE_TYPE);
		add_child(if_node, pre_brack);
		create_node_list(pre_brack, $3);
		create_node_list(pre_brack, aft_brack);
		create_node_list(pre_brack, $5);
		create_node_list(if_node, else_node);
		add_child(else_node, $7);
		$$ = if_node;
	}
	| WHILE '(' expression ')' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| DO statement WHILE '(' expression ')' ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| FOR '(' comma_expression_opt ';' expression_opt ';' comma_expression_opt ')' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| SWITCH '(' expression ')' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| DELETE expression ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| TRY statement CATCH statement {
		node_t* try_node = create_node("try", TRY_TYPE);
		node_t* catch_node = create_node("catch", CATCH_TYPE);
		add_child(try_node, $2);
		create_node_list(try_node, catch_node);
		add_child(catch_node, $4);
		DBG(" try catch in statement\n");
		$$ = try_node;
	}
	| AFTER '(' expression ')' CALL expression ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = create_node("AFTER", AFTER_TYPE);
		DBG("AFTER CALL statement\n");
	}
	| CALL expression returnargs ';' {
		node_t* node = create_node("call", CALL_TYPE);
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		add_child(node, $2);
		if ($3 != NULL) {
			create_node_list($2, $3);
		}
		create_node_list($2, semicolon);
		DBG("CALL statement\n");
		$$ = node;
	}
	| INLINE expression returnargs ';' {
		node_t* node = create_node("inline", INLINE_TYPE);
		add_child(node, $2);
		if ($3 != NULL) {
			create_node_list($2, $3);
		}
		$$ = node;
	}
	| ASSERT expression ';' {
		/* TODO: we should find the identifier from symbol table */
		node_t* node = create_node("assert", ASSERT_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ',' expression ':' STRING_LITERAL ',' log_args ';' {
		DBG("In LOG statement: %s\n", $8);
		node_t* node = create_node("log", LOG_KYE_TYPE);
		node_t* log_type = create_node($2, LOG_TYPE);
		node_t* colon = create_node(":", COLON_TYPE);
		node_t* format_str = create_node($8, CONST_STRING_TYPE);
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		add_child(node, log_type);
		create_node_list(log_type, $4);
		create_node_list(log_type, $6);
		create_node_list(log_type, colon);
		create_node_list(log_type, format_str);
		if ($10 != NULL) {
			create_node_list(log_type, $10);
		}
		create_node_list(log_type, semicolon);
		$$ = node;
	}
	| LOG STRING_LITERAL ',' expression ':' STRING_LITERAL log_args ';' {
		node_t* node = create_node("log", LOG_KYE_TYPE);
		node_t* log_type = create_node($2, LOG_TYPE);
		node_t* colon = create_node(":", COLON_TYPE);
		node_t* format_str = create_node($6, CONST_STRING_TYPE);
		add_child(node, log_type);
		create_node_list(log_type, $4);
		create_node_list(log_type, colon);
		create_node_list(log_type, format_str);
		if ($7 != NULL) {
			create_node_list(log_type, $7);
		}
		$$ = node;
	}
	| LOG STRING_LITERAL ':' STRING_LITERAL log_args ';' {
		node_t* node = create_node("log", LOG_KYE_TYPE);
		node_t* log_type = create_node($2, LOG_TYPE);
		node_t* colon = create_node(":", COLON_TYPE);
		node_t* format_str = create_node($4, CONST_STRING_TYPE);
		add_child(node, log_type);
		create_node_list(log_type, colon);
		create_node_list(log_type, format_str);
		if ($5 != NULL) {
			create_node_list(log_type, $5);
		}
		$$ = node;
	}
	| SELECT ident IN '(' expression ')' WHERE '(' expression ')' statement ELSE statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| FOREACH ident IN '(' expression ')' statement {
		node_t* node = create_node($2, FOREACH_TYPE);
		node_t* pre_brack = create_node("(", PRE_BRACKETS_TYPE);
		node_t* aft_brack = create_node(")", AFTER_BRACKETS_TYPE);
		add_child(node, pre_brack);
		create_node_list(pre_brack, $5);
		create_node_list(pre_brack, aft_brack);
		create_node_list(pre_brack, $7);
		DBG("FOREACH in statement\n");
		$$ = node;
	}
	| ident ':' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| CASE expression ':' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| DEFAULT ':' statement {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| GOTO ident ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| BREAK ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| CONTINUE ';' {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	| THROW ';' {
		node_t* node = create_node("throw", THROW_TYPE);
		node_t* semicolon = create_node(";", SEMICOLON_TYPE);
		create_node_list(node, semicolon);
		$$ = node;
	}
	| RETURN ';' {
		$$ = create_node("return", RETURN_TYPE);
	}
	| ERROR ';' {
		node_t* node = create_node("error", ERROR_TYPE);
		$$ = node;
	}
	| ERROR STRING_LITERAL ';' {
		node_t* node = create_node("error", ERROR_TYPE);
		node_t* str = create_node($2, CONST_STRING_TYPE);
		add_child(node, str);
		$$ = node;
	}
	;

log_args
	: {
		$$ = NULL;
	}
	| log_args ',' expression {
		if ($1 != NULL) {
			create_node_list($1, $3);
			$$ = $1;
		}
		else {
			$$ = $3;
		}
	}
	| expression {
		$$ = $1;
	}
	;

compound_statement
	: '{' statement_list '}' {
		node_t* pre_braces = create_node("{", PRE_BRACES_TYPE);
		node_t* aft_braces = create_node("}", AFTER_BRACES_TYPE);
		create_node_list(pre_braces, $2);
		create_node_list(pre_braces, aft_braces);
		$$ = pre_braces;
	}
	| '{' '}' {
		node_t* pre_braces = create_node("{", PRE_BRACES_TYPE);
		node_t* aft_braces = create_node("}", AFTER_BRACES_TYPE);
		create_node_list(pre_braces, aft_braces);
		$$ = pre_braces;
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
		$$ = "local";
	}
	| AUTO {
		debug_proc("Line : %d\n", __LINE__);
		$$ = NULL;
	}
	;

local
	: local_keyword cdecl ';' {
		node_t* node = create_node($1, LOCAL_KEYWORD_TYPE);
		add_child(node, $2);
		$$ = node;
	}
	| STATIC cdecl ';' {
		debug_proc("Line : %d\n", __LINE__);
		DBG("In STATIC \n");
		$$ = NULL;
	}
	| local_keyword cdecl '=' expression ';' {
		node_t* node = create_node($1, LOCAL_KEYWORD_TYPE);
		node_t* assign = create_node("=", ASSIGN_TYPE);
		add_child(node, $2);
		create_node_list($2, assign);
		add_child(assign, $4);
		$$ = node;
	}
	| STATIC cdecl '=' expression ';' {
		node_t* node = create_node("static", KEY_WORD_TYPE);
		node_t* assign = create_node("=", ASSIGN_TYPE);
		add_child(node, $2);
		create_node_list($2, assign);
		add_child(assign, $4);
		$$ = node;
	}
	;

objident_list
	: objident {
		debug_proc("Line : %d\n", __LINE__);
		ident_attr_t* attr = malloc(sizeof(ident_attr_t));
		memset(attr, 0, sizeof(ident_attr_t));
		attr->name = strdup($1);
		symbol_insert($1, IDENTIFER_TYPE, attr);

		node_t* node = create_node($1, IDENTIFER_TYPE);
		$$ = node;
	}
	| objident_list ',' objident {
		debug_proc("Line : %d\n", __LINE__);
		if ($1 != NULL) {
			create_node_list($1, $3);
		}
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
		$$ = "this";
	}
	| REGISTER {
		$$ ="register";
	}
	| SIGNED {
		$$ = "signed";
	}
	| UNSIGNED {
		$$ = "unsigned";
	}
	;

ident
	: IDENTIFIER {
		$$ = $1;
	}
	| ATTRIBUTE {
		$$ = "attribute";
	}
	| BANK {
		$$ = "bank";
	}
	| BITORDER {
		$$ = "bitorder";
	}
	| CONNECT {
		$$ = "connect";
	}
	| CONSTANT {
		$$ = "constant";
	}
	| DATA {
		$$ = "data";
	}
	| DEVICE {
		$$ = "device";
	}
	| EVENT {
		$$ = "event";
	}
	| FIELD {
		$$ = "field";
	}
	| FOOTER {
		$$ = "footer";
	}
	| GROUP {
		$$ = "group";
	}
	| IMPLEMENT {
		$$ = "implement";
	}
	| IMPORT {
		$$ = "import";
	}
	| INTERFACE {
		$$ = "interface";
	}
	| LOGGROUP {
		$$ = "loggroup";
	}
	| METHOD {
		$$ = "method";
	}
	| PORT {
		$$ = "port";
	}
	| SIZE {
		$$ = "size";
	}
	| CLASS {
		$$ = "class";
	}
	| ENUM {
		$$ = "enum";
	}
	| NAMESPACE {
		$$ = "namespace";
	}
	| PRIVATE {
		$$ = "private";
	}
	| PROTECTED {
		$$ = "protected";
	}
	| PUBLIC {
		$$ = "public";
	}
	| RESTRICT {
		$$ = "register";
	}
	| UNION {
		$$ = "union";
	}
	| USING {
		$$ = "using";
	}
	| VIRTUAL {
		$$ = "virtual";
	}
	| VOLATILE {
		$$ = "volatile";
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
