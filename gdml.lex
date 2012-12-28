
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
%}  

%option outfile="Lexer.c" header-file="Lexer.h"

%option bison-bridge
%option case-insensitive never-interactive
%option nounistd noyywrap
%option reentrant
%option yylineno
      
%%  
"/*"            { 
	//comment(); 
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
//     printf("In comment, c=%d, c1=%c\n", c, c1); 
	if(c1 == '\n')
		lineno ++;
        if ((c1) != '/' && c != 0)  
        {  
//		printf("In comment, c1 = %c, c=%c\n",c1, c);
            unput(c1);  
            goto loop;  
        }  
      
        if (c != 0) { 
		//putchar(c);
		//putchar(c1);  
	}
}  
"//"		{  
	char c;
	//printf("Comment begin:");
	while((c = input(yyscanner)) != '\n'){
		//putchar(c);
	}
	//putchar('\n');
	lineno++;
	column = 0;
}

"dml"		{ count(yyscanner); return(DML);}      
"device"	{ count(yyscanner); return(DEVICE);}
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
"method"	{ count(yyscanner); return(METHOD);}
"parameter"	{ count(yyscanner); return (PARAMETER);}
"bank"		{ count(yyscanner); return(BANK);}
"field"		{ count(yyscanner); return(FIELD);}
"data"		{ count(yyscanner); return(DATA);}
"connect"	{ count(yyscanner); return(CONNECT);}
"interface"	{ count(yyscanner); return(INTERFACE);}
"attribute"	{ count(yyscanner); return(ATTRIBUTE); }
"event"		{ count(yyscanner); return(EVENT);}
"group"		{ count(yyscanner); return(GROUP);}
"in"		{ count(yyscanner); return(IN);}
"template"	{ count(yyscanner); return(TEMPLATE);}
"foreach"	{ count(yyscanner); return(FOREACH);}
"try"		{ count(yyscanner); return(TRY);}
"catch"		{ count(yyscanner); return(CATCH);}
"inline"	{ count(yyscanner); return(INLINE);}
"throw"		{ count(yyscanner); return(THROW);}
"local"		{ count(yyscanner); return(LOCAL);}
"error"		{ count(yyscanner); return(ERROR);}
"typeof"		{ count(yyscanner); return(TYPEOF);}
"assert"	{count(yyscanner); return(ASSERT);}
"cast"		{count(yyscanner); return(CAST);}
"header"	{ 
	count(yyscanner);
	char c, c1;
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
	printf("begin header_loop\n");
	/* try to get the end token "%}" */
	while ((c = input(yyscanner)) != '%' && c != 0) { 
		if(c == '\n'){
			lineno ++;
			column = 0;
		}
	}
	c1 = input(yyscanner);
        if ((c1) != '}' && c != 0)  
        {  
//		printf("In comment, c1 = %c, c=%c\n",c1, c);
		unput(c1);  
		goto header_loop;  
        }  
     	/* get the complete header */ 
	return(HEADER);
}
"footer"	{ count(yyscanner); return(FOOTER);}
"loggroup"	{ count(yyscanner); return(LOGGROUP);}
"log"		{ count(yyscanner); return(LOG);}
"import"	{ count(yyscanner); return(IMPORT);}
"size"		{ count(yyscanner); return(SIZE);}
"layout"	{ count(yyscanner); return(LAYOUT);}
"bitfields"	{ count(yyscanner); return(BITFIELDS);}
"call"		{ count(yyscanner); return(CALL);}
"public"	{ count(yyscanner); return(PUBLIC);}
"port"		{ count(yyscanner); return(PORT);}
"restrict"	{ count(yyscanner); return(RESTRICT);}
"using"		{ count(yyscanner); return(USING);}
"new"		{ count(yyscanner); return(NEW);}
"delete"	{ count(yyscanner); return(DELETE);}
"is"		{ count(yyscanner); return(IS);}
"defined"		{ count(yyscanner); return(DEFINED);}
"bitorder"	{count(yyscanner); return(BITORDER);}
"constant"	{count(yyscanner); return(CONSTANT);}
"\".*\""	{ count(yyscanner); 
			yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
			return(STRING_LITERAL);
		}
    
{L}({L}|{D})*       { count(yyscanner); 
			yylval_param->sval = (char *) strdup(yyget_text(yyscanner));  
			return(IDENTIFIER);  
			}  
[a-zA-Z_][a-zA-Z_0-9]*       { count(yyscanner); 
			yylval_param->sval = (char *) strdup(yyget_text(yyscanner));  
			return(IDENTIFIER);  
			}  
   
0[xX]{H}+{INTS}?      { count(yyscanner); return(INTEGER_LITERAL); }  
0{D}+{INTS}?      { count(yyscanner); return(INTEGER_LITERAL); }  
{D}+{INTS}?       { count(yyscanner); 
			yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
			return(INTEGER_LITERAL); 
		}  
L?'(\\.|[^\\'])+'   { count(yyscanner); return(INTEGER_LITERAL); }  
      
{D}+{E}{FS}?        { count(yyscanner); return(INTEGER_LITERAL); }  
{D}*"."{D}+({E})?{FS}?  { count(yyscanner); return(INTEGER_LITERAL); }  
{D}+"."{D}*({E})?{FS}?  { count(yyscanner); return(INTEGER_LITERAL); }  
      
L?\"(\\.|[^\\"])*\" { count(yyscanner); 
			yylval_param->sval = (char *) strdup(yyget_text(yyscanner));
			return(STRING_LITERAL); 
			}  

"@"		{ count(yyscanner); return(REG_OFFSET);}
"..."           { count(yyscanner); return(ELLIPSIS); }  
".."		{count(yyscanner); return(RANGE_SIGN);}
"->"		{count(yyscanner); return(METHOD_RETURN);}
">>="         { count(yyscanner); return(RIGHT_ASSIGN); }  
"<<="         { count(yyscanner); return(LEFT_ASSIGN); }  
"+="            { count(yyscanner); return(ADD_ASSIGN); }  
"-="            { count(yyscanner); return(SUB_ASSIGN); }  
"*="            { count(yyscanner); return(MUL_ASSIGN); }  
"/="            { count(yyscanner); return(DIV_ASSIGN); }  
"%="            { count(yyscanner); return(MOD_ASSIGN); }  
"&="            { count(yyscanner); return(AND_ASSIGN); }  
"^="            { count(yyscanner); return(XOR_ASSIGN); }  
"|="            { count(yyscanner); return(OR_ASSIGN); }  
">>"          { count(yyscanner); return(RIGHT_OP); }  
"<<"          { count(yyscanner); return(LEFT_OP); }  
"++"            { count(yyscanner); return(INC_OP); }  
"--"            { count(yyscanner); return(DEC_OP); }  
"&&"            { count(yyscanner); return(AND_OP); }  
"||"            { count(yyscanner); return(OR_OP); }  
"<="         { count(yyscanner); return(LE_OP); }  
">="         { count(yyscanner); return(GE_OP); }  
"=="            { count(yyscanner); return(EQ_OP); }  
"!="            { count(yyscanner); return(NE_OP); }  
";"         { count(yyscanner); return(';'); }  
("{"|"<%")       { count(yyscanner); return('{'); }  
("}"|"%>")       { count(yyscanner); return('}'); }  
","         { count(yyscanner); return(','); }  
":"         { count(yyscanner); return(':'); }  
"="         { count(yyscanner); return('='); }  
"("         { count(yyscanner); return('('); }  
")"         { count(yyscanner); return(')'); }  
("["|"<:")       { count(yyscanner); return('['); }  
("]"|":>")       { count(yyscanner); return(']'); }  
"."         { count(yyscanner); return('.'); }  
"&"         { count(yyscanner); return('&'); }  
"!"         { count(yyscanner); return('!'); }  
"~"         { count(yyscanner); return('~'); }  
"-"         { count(yyscanner); return('-'); }  
"+"         { count(yyscanner); return('+'); }  
"*"         { count(yyscanner); return('*'); }  
"/"         { count(yyscanner); return('/'); }  
"%"         { count(yyscanner); return('%'); }  
"<"          { count(yyscanner); return('<'); }  
">"          { count(yyscanner); return('>'); }  
"^"         { count(yyscanner); return('^'); }  
"|"         { count(yyscanner); return('|'); }  
"?"         { count(yyscanner); return('?'); }  
"\n"                {    ++lineno; column = 0;}
[ \t\v\f]     { count(yyscanner); }  
.           { /* ignore bad characters */ }  
    
%%  

/*      
yywrap()  
{  
        return(1);  
}  
  */    
      
comment()  
{  
        char c, c1;  
     /* 
    loop:  
        while ((c = input()) != '*' && c != 0)  
            putchar(c);  
      
        if ((c1 = input()) != '/' && c != 0)  
        {  
            unput(c1);  
            goto loop;  
        }  
      
        if (c != 0)  
            putchar(c1);  
	*/
}

comment1()
{
/*
	char c;
	printf("Comment begin:");
	while((c = input()) != '\n')
		putchar(c);
	putchar('\n');
	lineno++;
*/
}
      
     
void count(yyscan_t scanner)
{  
	int i;  
	char* text = yyget_text(scanner);   
        for (i = 0; text[i] != '\0'; i++)  
            if (text[i] == '\n')  
                column = 0;  
            else if (text[i] == '\t')  
                column += 8 - (column % 8);  
            else  
                column++;
        //ECHO;  
	//printf("In %s, lineno=%d, yytext=%s\n", __FUNCTION__, lineno, text);
}  
      
      
int check_type()  
{  
    /*  
    * pseudo code --- this is what it should check  
    *  
    *   if (yytext == type_name)  
    *       return(TYPE_NAME);  
    *  
    *   return(IDENTIFIER);  
    */  
      
    /*  
    *   it actually will only return IDENTIFIER  
    */  
	//printf("lineno=%d,yytext=%s, in %s\n", lineno, yytext, __FUNCTION__);
//	yylval.sval = (char *) strdup(yytext);  
        return(IDENTIFIER);  
}

int get_string(yyscan_t scanner){
//int get_string(){
	//YYSTYPE* yylval = yyget_lval(scanner);
	//yylval.sval = (char *) strdup(yyget_text(scanner));
	//yyset_lval(yylval, scanner);
	//char* yytext = (char *) strdup(yyget_text(scanner));
        return(STRING_LITERAL);  
} 

int get_integer(){
//	yylval.sval = (char *) strdup(yytext);
	return(INTEGER_LITERAL);
}
