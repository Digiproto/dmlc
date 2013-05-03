
D           [0-9]  
L           [a-zA-Z_]  
H           [a-fA-F0-9]  
E           [Ee][+-]?{D}+  
FS          (f|F|l|L)  
INTS          (u|U|l|L)*  

%{  
#include <stdio.h>  
#include "types.h"
#include "ast.h"
#include "Parser.h"  
extern int lineno;
int column = 0;
void count(yyscanner);
/* the max length of include file name */
#define MAX_HEAD_LEN 1024
%}  

%option outfile="Lexer.c" header-file="Lexer.h"

%option bison-bridge
%option case-insensitive never-interactive
%option nounistd noyywrap
%option reentrant
%option yylineno
      
%%  
"/*"            { 
					//printf("Comment begin:");
					char c, c1;
loop:
					while ((c = input(yyscanner)) != '*' && c != 0) {
						//putchar(c);
						if(c == '\n'){
							lineno ++;
							column = 0;
						}
					}

					c1 = input(yyscanner);
					//printf("In comment, c=%d, c1=%c\n", c, c1);
					if(c1 == '\n')
					lineno ++;

					if ((c1) != '/' && c != 0) {
						//printf("In comment, c1 = %c, c=%c\n",c1, c);
						unput(c1);
						goto loop;
					}

					if (c != 0) {
						//putchar(c);
						//putchar(c1);
					}
				}

"//"			{
					char c;
					//printf("Comment begin:");
					while((c = input(yyscanner)) != '\n'){
						//putchar(c);
					}
					//putchar('\n');
					lineno++;
					column = 0;
				}

"dml"			{ count(yyscanner); return(DML);}
"device"		{ count(yyscanner); return(DEVICE);}
"auto"          { count(yyscanner); return(AUTO); }  
"break"         { count(yyscanner); return(BREAK); }  
"case"          { count(yyscanner); return(CASE); }  
"char"          { count(yyscanner); return(CHAR); }  
"const"         { count(yyscanner); return(CONST); }  
"continue"      { count(yyscanner); return(CONTINUE); }  
"default"       { count(yyscanner); return(DEFAULT); }  
"do"            { count(yyscanner); return(DO); }  
"double"        { count(yyscanner); return(DOUBLE); }  
"else"          { count(yyscanner); return(ELSE); }  
"enum"          { count(yyscanner); return(ENUM); }  
"extern"        { count(yyscanner); return(EXTERN); }  
"float"         { count(yyscanner); return(FLOAT); }  
"for"           { count(yyscanner); return(FOR); }  
"goto"          { count(yyscanner); return(GOTO); }  
"if"            { count(yyscanner); return(IF); }  
"int"           { count(yyscanner); return(INT); }  
"long"          { count(yyscanner); return(LONG); }  
"register"      { count(yyscanner); return(REGISTER); }  
"return"        { count(yyscanner); return(RETURN); }  
"short"         { count(yyscanner); return(SHORT); }  
"signed"        { count(yyscanner); return(SIGNED); }  
"sizeof"        { count(yyscanner); return(SIZEOF); }  
"static"        { count(yyscanner); return(STATIC); }  
"struct"        { count(yyscanner); return(STRUCT); }  
"switch"        { count(yyscanner); return(SWITCH); }  
"typedef"       { count(yyscanner); return(TYPEDEF); }  
"union"         { count(yyscanner); return(UNION); }  
"unsigned"      { count(yyscanner); return(UNSIGNED); }  
"void"          { count(yyscanner); return(VOID); }  
"volatile"      { count(yyscanner); return(VOLATILE); }  
"while"         { count(yyscanner); return(WHILE); }  
"method"		{ count(yyscanner); return(METHOD);}
"parameter"		{ count(yyscanner); return (PARAMETER);}
"bank"			{ count(yyscanner); return(BANK);}
"field"			{ count(yyscanner); return(FIELD);}
"data"			{ count(yyscanner); return(DATA);}
"connect"		{ count(yyscanner); return(CONNECT);}
"interface"		{ count(yyscanner); return(INTERFACE);}
"attribute"		{ count(yyscanner); return(ATTRIBUTE); }
"event"			{ count(yyscanner); return(EVENT);}
"group"			{ count(yyscanner); return(GROUP);}
"in"			{ count(yyscanner); return(IN);}
"template"		{ count(yyscanner); return(TEMPLATE);}
"foreach"		{ count(yyscanner); return(FOREACH);}
"try"			{ count(yyscanner); return(TRY);}
"catch"			{ count(yyscanner); return(CATCH);}
"inline"		{ count(yyscanner); return(INLINE);}
"throw"			{ count(yyscanner); return(THROW);}
"local"			{ count(yyscanner); return(LOCAL);}
"error"			{ count(yyscanner); return(ERROR);}
"typeof"		{ count(yyscanner); return(TYPEOF);}
"assert"		{ count(yyscanner); return(ASSERT);}
"cast"			{ count(yyscanner); return(CAST);}
"header"		{
					count(yyscanner);
					char c, c1;
					char head_buf[MAX_HEAD_LEN]; // store the include file name
					int i = 0;

					while ((c = input(yyscanner)) != '%' && c != 0) {
						if(c == ' '){
						}
						else if(c == '\n'){
							lineno ++;
							column = 0;
						}
						else{
							/* something wrong */
						}
					}
					/* the next character should be '{' */
					c1 = input(yyscanner);
					if(c1 != '{'){
						/* something wrong */
						lineno ++;
					}
header_loop:
					//printf("begin header_loop\n");
					/* try to get the end token "%}" */
					while ((c = input(yyscanner)) != '%' && c != 0) {
						if(c == '\n'){
							lineno ++;
							column = 0;
						}
						//putchar(c);
						head_buf[i++] = c;
						if (i >= MAX_HEAD_LEN) {
							fprintf(stderr, "The include file name is out of bounds!\n");
							exit(-1);
						}
					}
					head_buf[i] = '\0';
					yylval_param->sval = strdup(head_buf);
					c1 = input(yyscanner);
					if ((c1) != '}' && c != 0) {
						//printf("In comment, c1 = %c, c=%c\n",c1, c);
						unput(c1);
						goto header_loop;
					}
					/* get the complete header */
					return(HEADER);
				}
"footer"		{ count(yyscanner); return(FOOTER);}
"loggroup"		{ count(yyscanner); return(LOGGROUP);}
"log"			{ count(yyscanner); return(LOG);}
"import"		{ count(yyscanner); return(IMPORT);}
"size"			{ count(yyscanner); return(SIZE);}
"layout"		{ count(yyscanner); return(LAYOUT);}
"bitfields"		{ count(yyscanner); return(BITFIELDS);}
"call"			{ count(yyscanner); return(CALL);}
"public"		{ count(yyscanner); return(PUBLIC);}
"port"			{ count(yyscanner); return(PORT);}
"restrict"		{ count(yyscanner); return(RESTRICT);}
"using"			{ count(yyscanner); return(USING);}
"new"			{ count(yyscanner); return(NEW);}
"delete"		{ count(yyscanner); return(DELETE);}
"is"			{ count(yyscanner); return(IS);}
"defined"		{ count(yyscanner); return(DEFINED);}
"undefined"		{ count(yyscanner); return(UNDEFINED);}
"sizeoftype"	{ count(yyscanner); return(SIZEOFTYPE);}
"bitorder"		{count(yyscanner); return(BITORDER);}
"constant"		{count(yyscanner); return(CONSTANT);}
"implement"		{count(yyscanner); return(IMPLEMENT);}
"select"		{count(yyscanner); return(SELECT);}
"where"			{count(yyscanner); return(WHERE);}
"after"			{count(yyscanner); return(AFTER);}
"\".*\""		{
					count(yyscanner);
					yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
					return(STRING_LITERAL);
				}
    
{L}({L}|{D})*   {
					count(yyscanner);
					yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
					return(IDENTIFIER);
				}

[a-zA-Z_][a-zA-Z_0-9]*		{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(IDENTIFIER);
							}
   
0[xX]{H}+{INTS}?			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

0{D}+{INTS}?				{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

{D}+{INTS}?					{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

L?'(\\.|[^\\'])+'			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

{D}+{E}{FS}?				{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

{D}*"."{D}+({E})?{FS}?		{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}
{D}+"."{D}*({E})?{FS}?		{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}
      
L?\"(\\.|[^\\"])*\"			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(STRING_LITERAL);
							}

"@"					{ count(yyscanner); return(REG_OFFSET);}
"..."				{ count(yyscanner); return(ELLIPSIS); }
".."				{ count(yyscanner); return(RANGE_SIGN);}
"->"				{ count(yyscanner); return(METHOD_RETURN);}
">>="				{ count(yyscanner); return(RIGHT_ASSIGN); }
"<<="				{ count(yyscanner); return(LEFT_ASSIGN); }
"+="				{ count(yyscanner); return(ADD_ASSIGN); }
"-="				{ count(yyscanner); return(SUB_ASSIGN); }
"*="				{ count(yyscanner); return(MUL_ASSIGN); }
"/="				{ count(yyscanner); return(DIV_ASSIGN); }
"%="				{ count(yyscanner); return(MOD_ASSIGN); }
"&="				{ count(yyscanner); return(AND_ASSIGN); }
"^="				{ count(yyscanner); return(XOR_ASSIGN); }
"|="				{ count(yyscanner); return(OR_ASSIGN); }
">>"				{ count(yyscanner); return(RIGHT_OP); }
"<<"				{ count(yyscanner); return(LEFT_OP); }
"++"				{ count(yyscanner); return(INC_OP); }
"--"				{ count(yyscanner); return(DEC_OP); }
"&&"				{ count(yyscanner); return(AND_OP); }
"||"				{ count(yyscanner); return(OR_OP); }
"<="				{ count(yyscanner); return(LE_OP); }
">="				{ count(yyscanner); return(GE_OP); }
"=="				{ count(yyscanner); return(EQ_OP); }
"!="				{ count(yyscanner); return(NE_OP); }
";"					{ count(yyscanner); return(';'); }
("{"|"<%")			{ count(yyscanner); return('{'); }
("}"|"%>")			{ count(yyscanner); return('}'); }
","					{ count(yyscanner); return(','); }
":"					{ count(yyscanner); return(':'); }
"="					{ count(yyscanner); return('='); }
"("					{ count(yyscanner); return('('); }
")"					{ count(yyscanner); return(')'); }
("["|"<:")			{ count(yyscanner); return('['); }
("]"|":>")			{ count(yyscanner); return(']'); }
"."					{ count(yyscanner); return('.'); }
"&"					{ count(yyscanner); return('&'); }
"!"					{ count(yyscanner); return('!'); }
"~"					{ count(yyscanner); return('~'); }
"-"					{ count(yyscanner); return('-'); }
"+"					{ count(yyscanner); return('+'); }
"*"					{ count(yyscanner); return('*'); }
"/"					{ count(yyscanner); return('/'); }
"%"					{ count(yyscanner); return('%'); }
"<"					{ count(yyscanner); return('<'); }
">"					{ count(yyscanner); return('>'); }
"^"					{ count(yyscanner); return('^'); }
"|"					{ count(yyscanner); return('|'); }
"?"					{ count(yyscanner); return('?'); }
"$"					{ count(yyscanner); return('$'); }
"\n"                { ++lineno; column = 0;}
[ \t\v\f]			{ count(yyscanner); }
.					{ /* ignore bad characters */ }
    
%%  

void count(yyscan_t scanner)
{  
	int i;  
	char* text = yyget_text(scanner);   

	for (i = 0; text[i] != '\0'; i++) {
		if (text[i] == '\n') {
			column = 0;
		}
		else if (text[i] == '\t') {
			column += 8 - (column % 8);
		}
		else {
			column++;
		}
	}

#if 0
	if (strcmp(text, " ") != 0) {
		printf("In %s, lineno=%d, yytext=%s\n", __FUNCTION__, lineno, text);
	}
#endif
}  
      
      
int check_type()  
{  
	return(IDENTIFIER);
}

int get_string(yyscan_t scanner){
	return(STRING_LITERAL);
} 

int get_integer(){
	return(INTEGER_LITERAL);
}
