%{
#include "types.h"
#include "ast.h"
#define YYDEBUG 1
%}

%union  {
	int ival;
	char* sval;
	//objtype* objval;	
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

%start dml

%type	<ival> DEVICE
%type	<ival> IMPORT
%type	<sval> IDENTIFIER
%type   <sval> objident
%type   <sval> maybe_objident
%type   <sval> ident
%type   <sval> paramspec
%type   <ival> expression
%type   <sval> STRING_LITERAL
%type   <sval> INTEGER_LITERAL
%type 	<nodeval> syntax_modifiers
%type 	<nodeval> device_statements
%type 	<nodeval> device_statement
%type 	<nodeval> syntax_modifier
%type 	<nodeval> toplevel
%type 	<nodeval> import
%type 	<nodeval> parameter
%type 	<nodeval> object_statement
%type 	<nodeval> method
%type 	<nodeval> object
%type	<nodeval> object_spec
%type	<nodeval> object_statements

%%

dml
	: DEVICE objident ';' syntax_modifiers device_statements {
		symbol_insert($2, DEVICE_TYPE);
		node_t* root = create_ast($2);
		if($4 != NULL)	
			add_child(root, $4);
		if($5 != NULL)
			add_child(root, $5);
		printf("Device type is %s\n", $2);
	}
	| syntax_modifiers device_statements
	;

syntax_modifiers
	: {
		$$ = NULL;
	}
	| syntax_modifiers syntax_modifier{
		printf("In syntax_modifiers\n");
		$$ = create_node_list($1, $2);
	}
	;

syntax_modifier
	: BITORDER ident ';' {
		$$ = create_node($2);
	}
	;

device_statements
	:{
		$$ = NULL;
	} 
	| device_statements device_statement{
		printf("In device_statements\n");
		if($1 == NULL && $2 != NULL)
			$$ = $2;
		else if($1 != NULL && $2 != NULL)
			$$ = create_node_list($1, $2);
		else
			printf("something wrong in device_statements\n");
	}
	;

device_statement
	: object_statement{
		printf("object_statement In device_statement\n");
		$$ = $1;
	}
	| toplevel{
		$$ = $1;
	}
	| import{
		$$ = $1;
	}
	;

object
	: BANK maybe_objident istemplate object_spec {
		//insert_symbol()
		printf("BANK is %s\n", $2);
		#if 1
		node_t* bank = create_node($2, BANK_TYPE);
		if($4 != NULL)
			add_child(bank, $4);
		$$ = bank;
		#endif
	}
	| REGISTER objident sizespec offsetspec istemplate object_spec{
		printf("register is %s\n", $2);
		node_t* reg = create_node($2, REGISTER_TYPE);
		if($6 != NULL){
			printf("add_child for register\n");
			add_child(reg, $6);
		}
		$$ = reg;
	}
	| REGISTER objident '[' arraydef ']' sizespec istemplate object_spec
	| FIELD objident bitrange istemplate object_spec
	| FIELD objident istemplate object_spec
	| DATA cdecl ';'
	| CONNECT objident istemplate object_spec
	| INTERFACE objident istemplate object_spec
	| ATTRIBUTE objident istemplate object_spec
	| EVENT objident istemplate object_spec
	| GROUP objident istemplate object_spec
	| PORT objident istemplate object_spec
	| IMPLEMENT objident istemplate object_spec
	| ATTRIBUTE objident '[' arraydef ']' istemplate object_spec
	| GROUP objident '[' arraydef ']' istemplate object_spec
	| CONNECT objident '[' arraydef ']' istemplate object_spec
	;
method
	: METHOD objident method_params method_def{
		symbol_insert($2, METHOD_TYPE);	
		$$ = create_node($2, METHOD_TYPE);
		printf("method is %s\n", $2);
	}
	| METHOD EXTERN objident method_params method_def
	;

arraydef
	: expression
	| ident IN expression RANGE_SIGN expression
	;

toplevel
	: TEMPLATE objident object_spec
	| LOGGROUP ident ';'
	| CONSTANT ident '=' expression ';'
	| EXTERN cdecl_or_ident ';'
	| TYPEDEF cdecl ';'
	| EXTERN TYPEDEF cdecl ';'
	| STRUCT ident '{' struct_decls '}'
	;

istemplate_stmt
	: IS objident ';' {
		//$$ = $2;
	}
	;

import
	: IMPORT STRING_LITERAL ';'{
		symbol_insert($2, IMPORT_TYPE);	
		printf("import file is %s\n", $2);

		$$ = create_node($2, IMPORT_TYPE);
	}
	;
object_desc
	: STRING_LITERAL
	| 
	;
object_spec
	:object_desc ';'
	| object_desc '{' object_statements '}'{
		printf("object_statements for object_spec\n");
		$$ = $3;
	}
	;
object_statements
	: object_statements object_statement{
		printf("In object_statements\n");
		if($1 == NULL && $2 != NULL)
			$$ = $2;
		else if($1 != NULL && $2 != NULL)
			$$ = create_node_list($1, $2);
		else
			printf("something wrong in object_statements\n");
	}
	| {
		$$ = NULL;
	}
	;
object_statement
	: object{
		printf("in object for object_statement\n");
		$$ = $1;
	}
	
	| parameter{
		printf("parameter \n");
		$$ = $1;
	}
	| method{
		printf("method \n");
		$$ = $1;
	}
	| istemplate_stmt
	| object_if
	;

object_if
	: IF '(' expression ')' '{' object_statements '}'
	| IF '(' expression ')' '{' object_statements '}' ELSE '{' object_statements '}'
	| IF '(' expression ')' '{' object_statements '}' ELSE object_if
	;
parameter
	: PARAMETER objident paramspec{
		//symbol_insert($2, PARAMETER_TYPE);	
		parameter_insert($2, $3);
		$$ = create_node($2, PARAMETER_TYPE);
		printf("parameter name is %s\n", $2);
	}
	;

paramspec
	: ';'
	| '=' expression ';'{
		//printf("paramspec, expression=%d\n", $2);
		//$$ = $2;
	}
	| '=' STRING_LITERAL ';'{
		$$ = $2;
	}
	| DEFAULT expression ';'
	| AUTO ';'
	;

method_params
	: 
	| '(' cdecl_or_ident_list ')'
	| METHOD_RETURN '(' cdecl_or_ident_list ')'
	| '(' cdecl_or_ident_list ')' METHOD_RETURN '(' cdecl_or_ident_list ')'
	;

returnargs
	: 
	| METHOD_RETURN '(' expression_list ')'
	;

method_def
	: compound_statement
	| DEFAULT compound_statement
	;

istemplate
	: 
	| IS '(' objident_list ')'
	;

sizespec
	: SIZE expression
	| 
	;

offsetspec
	: REG_OFFSET expression
	| 
	;
bitrange
	: '[' expression ']'
	| '[' expression ':' expression ']'
	;

cdecl_or_ident
	: cdecl
	;
cdecl
	: basetype cdecl2
	| CONST basetype cdecl2
	;
basetype
	:typeident
	| struct
	| layout
	| bitfields
	| typeof
	;
cdecl2
	: cdecl3
	| CONST cdecl2
	| '*' cdecl2
	| VECT cdecl2
	;
cdecl3
	: typeident
	| 
	| cdecl3 '[' expression ']'
	| cdecl3 '(' cdecl_list ')'
	| '(' cdecl2 ')'
	;
cdecl_list
	: 
	| cdecl_list2
	;
cdecl_list2
	: cdecl
	| ELLIPSIS
	| cdecl_list2 ',' cdecl
	| cdecl_list2 ',' ELLIPSIS
	;
cdecl_or_ident_list
	: 
	| cdecl_or_ident_list2
	;
cdecl_or_ident_list2
	: cdecl_or_ident
	| cdecl_or_ident_list2 ',' cdecl_or_ident
	;

typeof
	: TYPEOF expression
	;
struct
	: STRUCT '{' struct_decls '}'
	;

struct_decls
	: struct_decls cdecl ';'
	| 
	;
layout
	:LAYOUT STRING_LITERAL '{' layout_decls '}'
	;

layout_decls
	:layout_decls cdecl ';'
	| 
	;

bitfields
	: BITFIELDS INTEGER_LITERAL '{' bitfields_decls '}'
	;

bitfields_decls
	: bitfields_decls cdecl '@' '[' expression ':' expression ']' ';'
	| 
	;
ctypedecl
	: const_opt basetype ctypedecl_ptr
	;
ctypedecl_ptr
	: stars ctypedecl_array
	;

stars
	: 
	| '*' CONST stars
	| '*' stars
	;

ctypedecl_array
	: ctypedecl_simple
	;

ctypedecl_simple
	: '(' ctypedecl_ptr ')'
	| 
	;

const_opt
	: CONST 
	| 
	;

typeident
	: ident
	| CHAR
	| DOUBLE
	| FLOAT
	| INT
	| LONG
	| SHORT 
	| SIGNED
	| UNSIGNED
	| VOID
	;

comma_expression
	: expression
	| comma_expression ',' expression
	;

expression
	: expression '=' expression {
		//assign_exec($1, $3);
		//$$ = $3;
	}
	| expression ADD_ASSIGN expression
	| expression SUB_ASSIGN expression
	| expression MUL_ASSIGN expression
	| expression DIV_ASSIGN expression
	| expression MOD_ASSIGN expression
	| expression OR_ASSIGN expression
	| expression AND_ASSIGN expression
	| expression XOR_ASSIGN expression
	| expression LEFT_ASSIGN expression
	| expression RIGHT_ASSIGN expression
	| expression '?' expression ':' expression
	| expression '+' expression
	| expression '-' expression
	| expression '*' expression
	| expression '/' expression
	| expression '%' expression
	| expression LEFT_OP expression
	| expression RIGHT_OP expression
	| expression EQ_OP expression
	| expression NE_OP expression
	| expression '<' expression
	| expression '>' expression
	| expression LE_OP expression
	| expression GE_OP expression
	| expression OR_OP expression
	| expression AND_OP expression
	| expression '|' expression
	| expression '^' expression
	| expression '&' expression
	| CAST '(' expression ',' ctypedecl ')'
	| SIZEOF expression
	| '-' expression
	| '+' expression
	| '!' expression
	| '~' expression
	| '&' expression
	| '*' expression
	| DEFINED expression
	| '#' expression
	| INC_OP expression
	| DEC_OP expression
	| expression INC_OP
	| expression DEC_OP
	| expression '(' expression_list ')'
	| INTEGER_LITERAL{
		$$=$1;
	}
	| STRING_LITERAL{
		$$=$1;
	}
	| UNDEFINED
	| '$' objident
	| ident
	| expression '.' objident
	| expression METHOD_RETURN objident
	| SIZEOFTYPE typeoparg
	;

typeoparg
	: ctypedecl
	| '(' ctypedecl ')'
	;

expression
	: NEW ctypedecl
	| NEW ctypedecl '[' expression ']'
	| '(' expression ')'
	| '[' expression_list ']'
	| expression '[' expression endianflag ']'
	| expression '[' expression ':' expression endianflag ']'
	;

endianflag
	: ',' IDENTIFIER
	|
	;

expression_opt
	: expression
	|
	;
comma_expression_opt
	: comma_expression
	| 
	;

expression_list
	:
	| expression
	| expression ',' expression_list
	;

statement
	: compound_statement
	| local
	| ';'
	| expression ';'
	| IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' comma_expression_opt ';' expression_opt ';' comma_expression_opt ')' statement
	| SWITCH '(' expression ')' statement
	| DELETE expression ';'
	| TRY statement CATCH statement
	| AFTER '(' expression ')' CALL expression ';'
	| CALL expression returnargs ';'
	| INLINE expression returnargs ';'
	| ASSERT expression ';'
	| LOG STRING_LITERAL ',' expression ',' expression ':' STRING_LITERAL log_args ';'
	| LOG STRING_LITERAL ',' expression ':' STRING_LITERAL log_args ';'
	| LOG STRING_LITERAL ':' STRING_LITERAL log_args ';'
	| SELECT ident IN '(' expression ')' WHERE '(' expression ')' statement ELSE statement
	| FOREACH ident IN '(' expression ')' statement
	| ident ':' statement
	| CASE expression ':' statement
	| DEFAULT ':' statement
	| GOTO ident ';'
	| BREAK ';'
	| CONTINUE ';'
	| THROW ';'
	| RETURN ';'
	| ERROR ';'
	| ERROR STRING_LITERAL ';'
	;

log_args
	: 
	| log_args ',' expression
	;

compound_statement
	: '{' statement_list '}'
	| '{' '}'
	;

statement_list
	: statement
	| statement_list statement
	;

local_keyword
	: LOCAL
	| AUTO
	;

local
	: local_keyword cdecl ';'
	| STATIC cdecl ';'
	| local_keyword cdecl '=' expression ';'
	| STATIC cdecl '=' expression ';'
	;

objident_list
	: objident
	| objident_list ',' objident
	;

maybe_objident
	: objident {
		$$ = $1;
	}
	| 
	;

objident
	:ident
	| THIS
	| REGISTER
	| SIGNED
	| UNSIGNED
	;

ident
	: IDENTIFIER{
		$$ = $1;
	}
	| ATTRIBUTE
	| BANK
	| BITORDER
	| CONNECT
	| CONSTANT
	| DATA
	| DEVICE
	| EVENT
	| FIELD
	| FOOTER
	| GROUP
	| HEADER
	| IMPLEMENT
	| IMPORT
	| INTERFACE
	| LOGGROUP
	| METHOD
	| PORT
	| SIZE
	| CLASS
	| ENUM
	| NAMESPACE
	| PRIVATE
	| PROTECTED
	| PUBLIC
	| RESTRICT
	| UNION
	| USING
	| VIRTUAL
	| VOLATILE
	;

%%
#include <stdio.h>
#include <stdlib.h>

//extern char yytext[];
extern char     *yytext;        /* last token, defined in lex.l  */
extern int column;
extern FILE *yyin;
int             lineno  = 1;    /* number of current source line */
/*
yyerror(s)
char *s;
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}
*/
void
yyerror(char *s)
{
	fflush(stdout);
	fprintf(stderr,"Syntax error on line #%d, column #%d: %s\n", lineno, column, s);
	fprintf(stderr,"Last token was \"%s\"\n",yytext);
	exit(1);
}

int main(int argc, char* argv[])
{
	if(argc != 2){
                fprintf(stderr,"Usage: %s source\n", argv[0]);
                exit(-1);
        }
        char* filename = argv[1];
        yyin = fopen(filename,"r");
//        init();
        yyparse();
        fclose(yyin);
	print_ast();
	return 0;
}
