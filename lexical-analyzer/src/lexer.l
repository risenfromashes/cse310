%{
#include <parser_context.h>

#define log(type) yyextra->log_token(yylineno, yytext, type)
#define report(error) yyextra->report_error(yylineno, yytext, error)
#define append(str)  yyextra->append_buf(str)
#define startline(line) yyextra->start_line(line)

%}

%option reentrant noyywrap nodefault yylineno 
%option extra-type="ParserContext *"

%x CMNT
%x MSTR
%x SCMNT

sp [ \t\f\r\v]
ws [ \t\f\r\v\n]+ 
lf \n
crlf (\r\n|\n)
letter [a-zA-Z]
digit [0-9]
alphanum ({letter}|{digit})
id ((_|{letter})(_|{alphanum})*)
suffix ([Uu](L|l|LL|ll)?|(L|l|LL|ll)[Uu]?)

oct 0[0-7]*
hex 0[xX][0-9a-fA-F]+
dec [1-9][0-9]*
int ({oct}|{hex}|{dec}){suffix}?

illintid ({oct}|{hex}|{dec}){id}

exp ([Ee][-+]?{digit}+) 
floatbase ({digit}+\.{digit}*|\.{digit}+)
float ({floatbase}{exp}?|{digit}+{exp})[fFlL]?

illfloatbase {floatbase}(\.{digit}*)+
illexp ([Ee][-+]?({floatbase}|{illfloatbase}))
illfloat ({floatbase}{illexp}|{illfloatbase}{exp}?|{illfloatbase}{illexp}|{digit}+{illexp})

escape ["'?\\abfnrtv]
char ([^"'\\\r\n]|\\{escape}|\\[0-7]{1,3})
charlit \'(["]|{char})\'
strlit \"([']|{char})*\"
cmnt \/\/([^\n]|\\{sp}*{lf})*

illcharlit \'(["]|{char}){2,}\'
emptycharlit \'\'
unfcharlit \'((["]|{char})|(\\\'))?

%%

{ws} { /* do nothing */ }

"\/\/"                              { append(yytext); startline(yylineno); BEGIN(SCMNT); }
<SCMNT>([^\r\n\\]|\\{sp}*{crlf}?)*  { append(yytext); }
<SCMNT>{crlf}                       { log(COMMENT);                        BEGIN(INITIAL); }

"/*"                   { append(yytext); yyextra->start_line(yylineno); BEGIN(CMNT); }
<CMNT>"*/"             { append(yytext); log(MULTI_LINE_COMMENT);       BEGIN(INITIAL); }
<CMNT>([^*]|{crlf})+|. { append(yytext); }
<CMNT><<EOF>>          { report("UNFINISHED_COMMENT");                  return 0; }


if        { log(IF); }   
else      { log(ELSE); }
for       { log(FOR); }
while     { log(WHILE); }
do        { log(DO); }
break     { log(BREAK); }
int       { log(INT); }
char      { log(CHAR); }
float     { log(FLOAT); }
double    { log(DOUBLE); }
void      { log(VOID); }
return    { log(RETURN); }
switch    { log(SWITCH); }
case      { log(CASE); }
default   { log(DEFAULT); }
continue  { log(CONTINUE); }

"+"|"-"                     { log(ADDOP); }
"*"|"/"|"%"                 { log(MULOP); } 
"++"|"--"                   { log(INCOP); } 
"<"|">"|"<="|">="|"=="|"!=" { log(RELOP); } 
"="                         { log(ASSIGNOP); } 
"&&"|"||"                   { log(LOGICOP); } 
"&"|"|"|"^"|"<<"|">>"       { log(BITOP); } 
"!"                         { log(NOT); } 
"("                         { log(LPAREN); } 
")"                         { log(RPAREN); } 
"{"                         { log(LCURL); } 
"}"                         { log(RCURL); } 
"["                         { log(LSQUARE); } 
"]"                         { log(RSQUARE); } 
","                         { log(COMMA); } 
";"                         { log(SEMICOLON); } 

{int}   { log(CONST_INT); }
{float} { log(CONST_FLOAT); }

{illfloatbase}  { report("TOO_MANY_DECIMAL_POINTS"); }
{illfloat}      { report("ILLFORMED_NUMBER"); }

{charlit}      { log(CONST_CHAR); }
{strlit}       { log(STRING); }

\"([']|{char})*                   { append(yytext); startline(yylineno);    BEGIN(MSTR); }
<MSTR>\"                          { append(yytext); log(MULTI_LINE_STRING); BEGIN(INITIAL); }
<MSTR>([']|{char}|\\{sp}*{crlf})* { append(yytext); ; }
<MSTR>{crlf}                      { report("UNFINISHED_STRING");            BEGIN(INITIAL); }
<MSTR><<EOF>>                     { report("UNFINISHED_STRING");            return 0; }

{unfcharlit}   { report("UNFINISHED_CONST_CHAR"); } 
{emptycharlit} { report("EMPTY_CONST_CHAR"); }
{illcharlit}   { report("MULTICHAR_CONST_CHAR"); } 

{id}        { log(ID); }
{illintid}  { report("INVALID_ID_SUFFIX_NUM_PREFIX"); }

. { report("UNRECOGNIZED_CHAR"); }

%%

#undef log
#undef report
#undef append

void scan_file(ParserContext* context, FILE* in)
{
    yyscan_t scanner;
    yylex_init_extra(context, &scanner);
    yyset_in(in, scanner);
    yylex(scanner);
    context->finish(yyget_lineno(scanner));
    yylex_destroy(scanner);
}


