%{
#include <ir/ir_parser.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>

%}

%option reentrant noyywrap nodefault yylineno 
%option extra-type="IRParser *"

ws [ \t\r\n,]

digit [0-9]
num [-+]?{digit}+
letter [a-zA-Z]

alphanum ({letter}|{digit})
id ((_|{letter})(_|{alphanum})*)

crlf (\r)?\n

%%

{ws}             { }
";#"{num}{crlf}  { yyextra->source_line(std::atoi(yytext + 2)); 
                   yyextra->new_line();                          }
";".*{crlf}      { yyextra->new_line();                          }
":".*{crlf}      { yyextra->new_line();                          }

PTRST            { yyextra->add_token(IRToken(IROp::PTRST));     }
PTRLD            { yyextra->add_token(IRToken(IROp::PTRLD));     }
COPY             { yyextra->add_token(IRToken(IROp::COPY));      }
ADD              { yyextra->add_token(IRToken(IROp::ADD));       }
AND              { yyextra->add_token(IRToken(IROp::AND));       }
OR               { yyextra->add_token(IRToken(IROp::OR));        }
XOR              { yyextra->add_token(IRToken(IROp::XOR));       }
INC              { yyextra->add_token(IRToken(IROp::INC));       }
DEC              { yyextra->add_token(IRToken(IROp::DEC));       }
NEG              { yyextra->add_token(IRToken(IROp::NEG));       }
NOT              { yyextra->add_token(IRToken(IROp::NOT));       }
SUB              { yyextra->add_token(IRToken(IROp::SUB));       }
MUL              { yyextra->add_token(IRToken(IROp::MUL));       }
DIV              { yyextra->add_token(IRToken(IROp::DIV));       }
MOD              { yyextra->add_token(IRToken(IROp::MOD));       }
LSHIFT           { yyextra->add_token(IRToken(IROp::LSHIFT));    }
RSHIFT           { yyextra->add_token(IRToken(IROp::RSHIFT));    }
JMPIF            { yyextra->add_token(IRToken(IROp::JMPIF));     }
JMPIFNOT         { yyextra->add_token(IRToken(IROp::JMPIFNOT));  }
JMP              { yyextra->add_token(IRToken(IROp::JMP));       }
LESS             { yyextra->add_token(IRToken(IROp::LESS));      }
LEQ              { yyextra->add_token(IRToken(IROp::LEQ));       }
GREAT            { yyextra->add_token(IRToken(IROp::GREAT));     }
GEQ              { yyextra->add_token(IRToken(IROp::GEQ));       }
EQ               { yyextra->add_token(IRToken(IROp::EQ));        }
NEQ              { yyextra->add_token(IRToken(IROp::NEQ));       }
ALLOC            { yyextra->add_token(IRToken(IROp::ALLOC));     }
AALLOC           { yyextra->add_token(IRToken(IROp::AALLOC));    }
GLOBAL           { yyextra->add_token(IRToken(IROp::GLOBAL));    }
GLOBALARR        { yyextra->add_token(IRToken(IROp::GLOBALARR)); }
PALLOC           { yyextra->add_token(IRToken(IROp::PALLOC));    }
PARAM            { yyextra->add_token(IRToken(IROp::PARAM));     }
CALL             { yyextra->add_token(IRToken(IROp::CALL));      }
PROC             { yyextra->add_token(IRToken(IROp::PROC));      }
ENDP             { yyextra->add_token(IRToken(IROp::ENDP));      }
RET              { yyextra->add_token(IRToken(IROp::RET));       }
LABEL            { yyextra->add_token(IRToken(IROp::LABEL));     }
ADDR             { yyextra->add_token(IRToken(IROp::ADDR));      }

"L"{num}         { yyextra->add_token(IRArg(yyextra->get_label(std::atoi(yytext + 1)))); }
"%"{num}         { yyextra->add_token(IRArg(yyextra->get_var(std::atoi(yytext + 1))));   }
"@"{id}          { yyextra->add_token(IRArg(yyextra->get_global(yytext + 1)));           }
{num}            { yyextra->add_token(IRArg(std::atoi(yytext)));                         }

.                { 
                    std::cerr << "Unrecognised char at line: " << yylineno << std::endl
                              <<  "at: " << yytext << std::endl;
                    exit(1);
                 }

%%

void IRParser::init_scanner()
{
    yylex_init_extra(this, &scanner_);
    yyset_in(in_file_, scanner_);
}

void IRParser::scan(){
    yylex(scanner_);
}

void IRParser::finish_scanner(){
    yylex_destroy(scanner_);
}


