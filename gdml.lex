
D           [0-9]  
L           [a-zA-Z_]  
H           [a-fA-F0-9]  
E           [Ee][+-]?{D}+  
FS          (f|F|l|L)  
IS          (u|U|l|L)*  

%{  
#include <stdio.h>  
#include "gdml.tab.h"  
extern int      lineno;      
void count();  
%}  
      
%%  
"/*"            { comment(); }  
"//"		{ comment1(); }

"dml"		{ count(); return(DML);}      
"device"	{ count(); return(DEVICE);}
"auto"          { count(); return(AUTO); }  
"break"         { count(); return(BREAK); }  
"case"          { count(); return(CASE); }  
"char"          { count(); return(CHAR); }  
"const"         { count(); return(CONST); }  
"continue"      { count(); return(CONTINUE); }  
"default"       { count(); return(DEFAULT); }  
"do"            { count(); return(DO); }  
"double"        { count(); return(DOUBLE); }  
"else"          { count(); return(ELSE); }  
"enum"          { count(); return(ENUM); }  
"extern"        { count(); return(EXTERN); }  
"float"         { count(); return(FLOAT); }  
"for"           { count(); return(FOR); }  
"goto"          { count(); return(GOTO); }  
"if"            { count(); return(IF); }  
"int"           { count(); return(INT); }  
"long"          { count(); return(LONG); }  
"register"      { count(); return(REGISTER); }  
"return"        { count(); return(RETURN); }  
"short"         { count(); return(SHORT); }  
"signed"        { count(); return(SIGNED); }  
"sizeof"        { count(); return(SIZEOF); }  
"static"        { count(); return(STATIC); }  
"struct"        { count(); return(STRUCT); }  
"switch"        { count(); return(SWITCH); }  
"typedef"       { count(); return(TYPEDEF); }  
"union"         { count(); return(UNION); }  
"unsigned"      { count(); return(UNSIGNED); }  
"void"          { count(); return(VOID); }  
"volatile"      { count(); return(VOLATILE); }  
"while"         { count(); return(WHILE); }  
"method"	{ count(); return(METHOD);}
"parameter"	{ count(); return (PARAMETER);}
"bank"		{ count(); return(BANK);}
"field"		{ count(); return(FIELD);}
"data"		{ count(); return(DATA);}
"connect"	{ count(); return(CONNECT);}
"interface"	{ count(); return(INTERFACE);}
"attribute"	{ count(); return(ATTRIBUTE); }
"event"		{ count(); return(EVENT);}
"group"		{ count(); return(GROUP);}
"in"		{ count(); return(IN);}
"template"	{ count(); return(TEMPLATE);}
"header"	{ count(); return(HEADER);}
"footer"	{ count(); return(FOOTER);}
"loggroup"	{ count(); return(LOGGROUP);}
"import"	{ count(); return(IMPORT);}
"size"		{ count(); return(SIZE);}
"layout"	{ count(); return(LAYOUT);}
"bitfields"	{ count(); return(BITFIELDS);}
"call"		{ count(); return(CALL);}
"public"	{ count(); return(PUBLIC);}
"port"		{ count(); return(PORT);}
"restrict"	{ count(); return(RESTRICT);}
"using"		{ count(); return(USING);}
"new"		{ count(); return(NEW);}
"delete"	{ count(); return(DELETE);}
"\".*\""	{ count(); return(STRING_LITERAL);}
    
{L}({L}|{D})*       { count(); return(check_type()); }  
    
0[xX]{H}+{IS}?      { count(); return(INTEGER_LITERAL); }  
0{D}+{IS}?      { count(); return(INTEGER_LITERAL); }  
{D}+{IS}?       { count(); return(INTEGER_LITERAL); }  
L?'(\\.|[^\\'])+'   { count(); return(INTEGER_LITERAL); }  
      
{D}+{E}{FS}?        { count(); return(INTEGER_LITERAL); }  
{D}*"."{D}+({E})?{FS}?  { count(); return(INTEGER_LITERAL); }  
{D}+"."{D}*({E})?{FS}?  { count(); return(INTEGER_LITERAL); }  
      
L?\"(\\.|[^\\"])*\" { count(); return(STRING_LITERAL); }  

"@"		{ count(); return(REG_OFFSET);}
"..."           { count(); return(ELLIPSIS); }  
".."		{count(); return(RANGE_SIGN);}
"->"		{count(); return(METHOD_RETURN);}
">>="         { count(); return(RIGHT_ASSIGN); }  
"<<="         { count(); return(LEFT_ASSIGN); }  
"+="            { count(); return(ADD_ASSIGN); }  
"-="            { count(); return(SUB_ASSIGN); }  
"*="            { count(); return(MUL_ASSIGN); }  
"/="            { count(); return(DIV_ASSIGN); }  
"%="            { count(); return(MOD_ASSIGN); }  
"&="            { count(); return(AND_ASSIGN); }  
"^="            { count(); return(XOR_ASSIGN); }  
"|="            { count(); return(OR_ASSIGN); }  
">>"          { count(); return(RIGHT_OP); }  
"<<"          { count(); return(LEFT_OP); }  
"++"            { count(); return(INC_OP); }  
"--"            { count(); return(DEC_OP); }  
"&&"            { count(); return(AND_OP); }  
"||"            { count(); return(OR_OP); }  
"<="         { count(); return(LE_OP); }  
">="         { count(); return(GE_OP); }  
"=="            { count(); return(EQ_OP); }  
"!="            { count(); return(NE_OP); }  
";"         { count(); return(';'); }  
("{"|"<%")       { count(); return('{'); }  
("}"|"%>")       { count(); return('}'); }  
","         { count(); return(','); }  
":"         { count(); return(':'); }  
"="         { count(); return('='); }  
"("         { count(); return('('); }  
")"         { count(); return(')'); }  
("["|"<:")       { count(); return('['); }  
("]"|":>")       { count(); return(']'); }  
"."         { count(); return('.'); }  
"&"         { count(); return('&'); }  
"!"         { count(); return('!'); }  
"~"         { count(); return('~'); }  
"-"         { count(); return('-'); }  
"+"         { count(); return('+'); }  
"*"         { count(); return('*'); }  
"/"         { count(); return('/'); }  
"%"         { count(); return('%'); }  
"<"          { count(); return('<'); }  
">"          { count(); return('>'); }  
"^"         { count(); return('^'); }  
"|"         { count(); return('|'); }  
"?"         { count(); return('?'); }  
"\n"                    ++lineno;     
[ \t\v\f]     { count(); }  
.           { /* ignore bad characters */ }  
    
%%  
      
yywrap()  
{  
        return(1);  
}  
      
      
comment()  
{  
        char c, c1;  
      
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
}

comment1()
{
	char c;
	while((c = input()) != '\n')
		putchar(c);
	lineno++;
}
      
int column = 0;  
     
void count()  
{  
    int i;  
      
        for (i = 0; yytext[i] != '\0'; i++)  
            if (yytext[i] == '\n')  
                column = 0;  
            else if (yytext[i] == '\t')  
                column += 8 - (column % 8);  
            else  
                column++;  
      
        ECHO;  
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
      
        return(IDENTIFIER);  
}  
