%{
#include <parser_context.h>

#include <parser.tab.h>

#define new_token(type) yylval->token = yyextra->new_token(yylineno, yytext, type)
#define report(error) yyextra->report_error(yylineno, yytext, error)
#define append(str)  yyextra->append_buf(str)
#define startline(line) yyextra->start_line(line)
#define startcol(col) yyextra->start_col(col)
#define resetcol() yyextra->reset_col()

#define YY_USER_ACTION \
  yylloc->first_line = yylloc->last_line = yylineno; \
  yylloc->first_column = yycolumn; \
  yylloc->last_column = yycolumn + yyleng - 1;

%}

%option reentrant noyywrap nodefault yylineno bison-bridge bison-locations
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
illexp ([Ee]([-+]?({floatbase}|{illfloatbase}))?)
illdecimal {illfloatbase}({illexp}|{exp})?
illfloat ({floatbase}{illexp}|{digit}+{illexp})

escape ["'?\\abfnrtv]
illescape [^"'?\\abfnrtv]
char ([^"'\\\r\n]|\\{escape}|\\[0-7]{1,3})
charlit \'(["]|{char})\'
strlit \"([']|{char})*\"

illcharlit \'(["]|{char}){2,}\'
emptycharlit \'\'
unfcharlit \'(["]|{char})?
badcharlit \'\\{illescape}\'

%%

{ws} { /* do nothing */ }

"\/\/"                         { append(yytext); startline(yylineno); BEGIN(SCMNT); }
<SCMNT>{crlf}                  { new_token(COMMENT);                  BEGIN(INITIAL); }
<SCMNT>([^\r\n\\]|\\{crlf})+|. { append(yytext); }

"/*"                       { append(yytext); startline(yylineno);           BEGIN(CMNT); }
<CMNT>"*/"                 { append(yytext); new_token(MULTI_LINE_COMMENT); BEGIN(INITIAL); }
<CMNT>([^*\r\n]|{crlf})+|. { append(yytext); }
<CMNT><<EOF>>              { report("UNFINISHED_COMMENT"); return YYEOF; }


if        { new_token(IF);       return IF; }      
else      { new_token(ELSE);     return ELSE;     }
for       { new_token(FOR);      return FOR;      }
while     { new_token(WHILE);    return WHILE;    }
do        { new_token(DO);       return DO;       }
break     { new_token(BREAK);    return BREAK;    }
int       { new_token(INT);      return INT;      }
char      { new_token(CHAR);     return CHAR;     }
float     { new_token(FLOAT);    return FLOAT;    }
double    { new_token(DOUBLE);   return DOUBLE;   }
void      { new_token(VOID);     return VOID;     }
return    { new_token(RETURN);   return RETURN;   }
switch    { new_token(SWITCH);   return SWITCH;   }
case      { new_token(CASE);     return CASE;     }
default   { new_token(DEFAULT);  return DEFAULT;  }
continue  { new_token(CONTINUE); return CONTINUE; }

"+"|"-"                     { new_token(ADDOP);     return ADDOP;     }
"*"|"/"|"%"                 { new_token(MULOP);     return MULOP;     } 
"++"                        { new_token(INCOP);     return INCOP;     } 
"--"                        { new_token(INCOP);     return DECOP;     } 
"<"|">"|"<="|">="|"=="|"!=" { new_token(RELOP);     return RELOP;     } 
"="                         { new_token(ASSIGNOP);  return ASSIGNOP;  } 
"&&"|"||"                   { new_token(LOGICOP);   return LOGICOP;   } 
"&"|"|"|"^"|"<<"|">>"       { new_token(BITOP);     return BITOP;     } 
"!"                         { new_token(NOT);       return NOT;       } 
"("                         { new_token(LPAREN);    return LPAREN;    } 
")"                         { new_token(RPAREN);    return RPAREN;    } 
"{"                         { new_token(LCURL);     return LCURL;     } 
"}"                         { new_token(RCURL);     return RCURL;     } 
"["                         { new_token(LSQUARE);   return LSQUARE;   } 
"]"                         { new_token(RSQUARE);   return RSQUARE;   } 
","                         { new_token(COMMA);     return COMMA;     } 
";"                         { new_token(SEMICOLON); return SEMICOLON; } 

{int}   { new_token(CONST_INT);   return CONST_INT;   }
{float} { new_token(CONST_FLOAT); return CONST_FLOAT; }
{id}    { new_token(ID);          return ID;          }

{illintid}      { report("INVALID_ID_SUFFIX_NUM_PREFIX"); return YYerror; }
{illdecimal}    { report("TOO_MANY_DECIMAL_POINTS");      return YYerror; }
{illfloat}      { report("ILLFORMED_NUMBER");             return YYerror; }

{charlit}      { new_token(CONST_CHAR); return CONST_CHAR; }
{strlit}       { new_token(STRING);     return STRING;     }

\"([']|{char})*                 { append(yytext); startline(yylineno); startcol(yyextra->col()); BEGIN(MSTR); }
<MSTR>\"                        { 
  append(yytext); 
  new_token(MULTI_LINE_STRING); 
  yylloc->first_line = yyextra->start_line();
  yylloc->last_line = yylineno;
  yylloc->first_column = yyextra->start_col();
  yylloc->last_column = yycolumn;
  BEGIN(INITIAL);   
  return MULTI_LINE_STRING; 
}
<MSTR>\\{crlf}        { append(yytext); }
<MSTR>([']|{char})+|. { append(yytext); }
<MSTR>{crlf}          { report("UNFINISHED_STRING"); BEGIN(INITIAL); return YYerror; }
<MSTR><<EOF>>         { report("UNFINISHED_STRING"); return YYEOF; }

{unfcharlit}   { report("UNFINISHED_CONST_CHAR");        return YYerror; } 
{illcharlit}   { report("MULTICHAR_CONST_CHAR");         return YYerror; } 
{emptycharlit} { report("EMPTY_CONST_CHAR");             return YYerror; }
{badcharlit}   { report("UNRECOGNIZED_ESCAPE_SEQUENCE"); return YYerror; }


. { report("UNRECOGNIZED_CHAR"); }

%%

void ParserContext::init_scanner()
{
    yylex_init_extra(this, &scanner_);
    yyset_in(in_file_, scanner_);
}

void ParserContext::finish_scanner(){
    auto line = yyget_lineno(scanner_);
    table_.log_all_scopes();
    logger_.write("Total lines: {}\n", line);
    logger_.write("Total errors: {}\n", error_count_);
    yylex_destroy(scanner_);
}


