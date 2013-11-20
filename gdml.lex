
D           [0-9]  
L           [a-zA-Z_]  
H           [a-fA-F0-9]  
E           [Ee][+-]?{D}+  
FS          (f|F|l|L)  
INTS          ([Uu](L|l|LL|ll)?|(L|l|LL|ll)[Uu]?)

%{  
#include <stdio.h>  
#include "types.h"
#include "ast.h"
#include "Parser.h"  
//extern int lineno;
//int column = 0;
void count(yyscan_t scanner);
/* the max length of include file name */
#define MAX_HEAD_LEN 1024
%}  

%{
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno;         \
	yylloc->first_column = yycolumn; yylloc->last_column = yycolumn + yyleng - 1; \
	yycolumn += yyleng; yylloc->file = filestack_top;
%}

%option outfile="Lexer.c" header-file="Lexer.h"

%option bison-bridge bison-locations
%option never-interactive
%option nounistd noyywrap
%option reentrant
%option yylineno
%option case-sensitive

%x COMMENT
%x BODYMODE

%%  
"/*"					{ BEGIN(COMMENT);}
<COMMENT>"*/"			{ BEGIN(INITIAL);}
<COMMENT>([^*]|\n)+|.	{ /* ignore. */}
<COMMENT><<EOF>>		{ fprintf(stderr, "Unterminated comment\n"); return 0;}
"//".*\n				{ /* ignore. */}

"dml"			{ count(yyscanner); return(DML);}
"device"		{ count(yyscanner); return(DEVICE);}
"auto"          { count(yyscanner); return(AUTO); }  
"break"         { count(yyscanner); return(BREAK); }  
"case"          { count(yyscanner); return(CASE); }  
"bool"			{ count(yyscanner); return(BOOL); }
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
("int"[1-9]*)|("uint"[1-9]*) {
					count(yyscanner);
					yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
					//printf("int: %s\n", yylval_param->sval);
					return(INT);
				}
"%{"					{ BEGIN(BODYMODE);}
<BODYMODE>(.|\n)*"%}"	{
							yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
							/* remove "%}". */
							yylval_param->sval[yyleng - 2] = '\0';
							BEGIN(INITIAL);
							return(BODY);
						}
<BODYMODE><<EOF>>		{ fprintf(stderr, "Unterminated header(footer) body\n"); return 0;}
"header"		{ count(yyscanner); return(HEADER);}
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

L?\"([^"\\]|\\['"?\\abfnrtv]|\\[0-7]{1,3}|\\[Xx][0-9a-fA-F]+|({L}|{D}))*\"	{
					count(yyscanner);
					yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
					return(STRING_LITERAL);
				}
    
{L}({L}|{D})*   {
					count(yyscanner);
					yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
					return(IDENTIFIER);
				}
0[xX]{H}+{INTS}?			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

0[0-9]*{INTS}?				{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}
0[Bb][0-9]+{INTS}?			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
					}

[1-9][0-9]*{INTS}?			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(INTEGER_LITERAL);
							}

[0-9]+".."					{
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								yylval_param->sval[yyleng - 2] = '\0';
								unput('.');
								unput('.');
								return(INTEGER_LITERAL);
							}

([0-9]*\.[0-9]+|[0-9]+\.){E}?[flFL]?	{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(FLOAT_LITERAL);
							}

[0-9]+{E}[flFL]?			{
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(FLOAT_LITERAL);
							}

0[Xx]({H}*\.{H}+|{H}+\.?)[Pp][-+]?[0-9]+[flFL]? {
								count(yyscanner);
								yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
								return(FLOAT_LITERAL);
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
"#"					{ count(yyscanner); return('#'); }
"/"					{ count(yyscanner); return('/'); }
"%"					{ count(yyscanner); return('%'); }
"<"					{ count(yyscanner); return('<'); }
">"					{ count(yyscanner); return('>'); }
"^"					{ count(yyscanner); return('^'); }
"|"					{ count(yyscanner); return('|'); }
"?"					{ count(yyscanner); return('?'); }
"$"					{ count(yyscanner); return('$'); }
"\n"				{ /* ignore. */}
\\$					{ /* ignore. */}
[ \t\v\f]			{ count(yyscanner); }
.					{
						/* bad characters */
						printf("[unknown] %s\n", yyget_text(yyscanner));
						return 0;
					}
    
%%  

void count(yyscan_t scanner)
{  
#if 0
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
#endif

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
