%{
#include <ir_parser.h>

%}

%option reentrant noyywrap nodefault yylineno bison-bridge bison-locations
%option extra-type="IRParser *"

%%


%%

void IRParser::init_scanner()
{
    yylex_init_extra(this, &scanner_);
    yyset_in(in_file_, scanner_);
}

void IRParser::finish_scanner(){
    yylex_destroy(scanner_);
}


