%define api.pure full

%param {void * scanner}
%parse-param {ParserContext* context}

%locations

%code requires{

#include <cstdio>
#include <cstdarg>
#include <token.h>
#include <parser_context.h>

#include <ast/stmt.h>
#include <ast/expr.h>
#include <ast/decl.h>
#include <ast/type.h>

#include <pt/pt_node.h>

#define YYLEX_PARAM context->scanner()

}


%union {
  Token* token;
  NonTerminal* non_term;
  NonTerminal* start;
};

%{

int yylex(YYSTYPE* lvalp, YYLTYPE* llocp, void* scanner);
void yyerror(YYLTYPE* locp, void* scanner, ParserContext* context, const char *s, ...); 
void lyyerror(YYLTYPE t, char *s, ...); 

%}


/* regular tokens */
%token <token> IF ELSE FOR WHILE DO BREAK INT CHAR FLOAT DOUBLE VOID RETURN SWITCH CASE DEFAULT 
%token <token> CONTINUE CONST_INT CONST_FLOAT CONST_CHAR ADDOP MULOP INCOP DECOP RELOP ASSIGNOP LOGICOP 
%token <token> BITOP NOT LPAREN RPAREN LCURL RCURL LSQUARE RSQUARE COMMA SEMICOLON ID STRING MULTI_LINE_STRING

%type <start>    start
%type <non_term> program unit var_declaration func_declaration func_definition type_specifier 
%type <non_term> parameter_list compound_statement statements declaration_list statement 
%type <non_term> expression expression_statement logic_expression rel_expression simple_expression
%type <non_term> term unary_expression factor variable argument_list arguments 
%type <token>    lcurl



/* precedence rules */
%nonassoc UNMATCHED_ELSE
%nonassoc ELSE

%%

start : program {
    $$ = NonTerminal::create(context, @$, "start", $1);
    auto root = TranslationUnitDecl::create(context, @$, std::move($1->decls()));
    context->set_ast_root(std::move(root));
    context->set_pt_root($$);
	}
	;

program : program unit {
    $$ = NonTerminal::create(context, @$, "program", $1, $2);
    $$->ast = std::move($1->ast);
    auto& t = $$->decls();
    auto& f = $2->decls();
    std::move(f.begin(), f.end(), std::back_inserter(t));
  } 
	| unit {
    $$ = NonTerminal::create(context, @$, "program", $1);
    $$->ast = std::move($1->ast);
  }
	;
	
unit : var_declaration {
        $$ = NonTerminal::create(context, @$, "unit", $1);
        $$->ast = Decls();
        for(auto& v: $1->vardecls()){
          $$->decls().push_back(std::move(v));
        }
     }
     | func_declaration {
        $$ = NonTerminal::create(context, @$, "unit", $1);
        $$->ast = Decls(); 
        $$->decls().push_back(std::move($1->decl()));
     }
     | func_definition { 
        $$ = NonTerminal::create(context, @$, "unit", $1);
        $$->ast = Decls{}; 
        $$->decls().push_back(std::move($1->decl()));
     }
     ;
     
func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON {
        $$ = NonTerminal::create(context, @$, "func_declaration", $1, $2, $3, $4, $5, $6);
        $$->ast = FuncDecl::create(context, @$, $1->type(),
                                    std::move($4->paramdecls()),
                                    $2->value(), nullptr);
        
        /* no declaration to follow */
        context->current_params(nullptr);
    }
		| type_specifier ID LPAREN RPAREN SEMICOLON {
        $$ = NonTerminal::create(context, @$, "func_declaration", $1, $2, $3, $4, $5);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    ParamDecls(),
                                    $2->value(), nullptr);
        /* no declaration to follow */
        context->current_params(nullptr);
    }
		;
		 
func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement {
        $$ = NonTerminal::create(context, @$, "func_definition", $1, $2, $3, $4, $5, $6);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    std::move($4->paramdecls()),
                                    $2->value(), std::move($6->stmt()));
    }
		| type_specifier ID LPAREN RPAREN compound_statement {
        $$ = NonTerminal::create(context, @$, "func_definition", $1, $2, $3, $4, $5);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    ParamDecls(),
                                    $2->value(), std::move($5->stmt())); 
    }
 		;				


parameter_list  : parameter_list COMMA type_specifier ID {
	        $$ = NonTerminal::create(context, @$, "parameter_list", $1, $2, $3, $4);
	        $$->ast = std::move($1->ast);
	        $$->paramdecls().push_back(ParamDecl::create(context, @$, $3->type(), 
	                                      $4->value()));
	        context->current_params(&$$->paramdecls());
    	}
		| parameter_list COMMA type_specifier {
	        $$ = NonTerminal::create(context, @$, "parameter_list", $1, $2, $3);
	        $$->ast = std::move($1->ast);
	        $$->paramdecls().push_back(ParamDecl::create(context, @$, $3->type(), ""));
	        context->current_params(&$$->paramdecls());
    	}
 		| type_specifier ID {
	        $$ = NonTerminal::create(context, @$, "parameter_list", $1, $2);
	        $$->ast = ParamDecls();
	        $$->paramdecls().push_back(ParamDecl::create(context, @$, $1->type(), $2->value()));
	        context->current_params(&$$->paramdecls());
    	}
		| type_specifier {
	        $$ = NonTerminal::create(context, @$, "parameter_list", $1);
	        $$->ast = ParamDecls();
	        $$->paramdecls().push_back(ParamDecl::create(context, @$, $1->type(), ""));
	        context->current_params(&$$->paramdecls());
    	}
 		;

 		
compound_statement : lcurl statements RCURL {
          $$ = NonTerminal::create(context, @$, "compound_statement", $1, $2, $3);
          $$->ast = CompoundStmt::create(context, @$, std::move($2->stmts()));
          context->exit_scope();
        }
 		    | lcurl RCURL {
          $$ = NonTerminal::create(context, @$, "compound_statement", $1, $2);
          $$->ast = CompoundStmt::create(context, @$, Stmts());
          context->exit_scope();
        }
 		;
 		    
var_declaration : type_specifier declaration_list SEMICOLON {
          $$ = NonTerminal::create(context, @$, "var_declaration", $1, $2, $3);
          $$->ast = std::move($2->ast);
     	 }
 		 ;
 		 
type_specifier	: INT { 
			$$ = NonTerminal::create(context, @$, "type_specifier", $1); 
			$$->ast = context->get_built_in_type(BuiltInTypeName::INT);
      context->current_type($$->type());
		}
 		| FLOAT {
			$$ = NonTerminal::create(context, @$, "type_specifier", $1); 
			$$->ast = context->get_built_in_type(BuiltInTypeName::FLOAT);
      context->current_type($$->type());
		}
 		| VOID {
			$$ = NonTerminal::create(context, @$, "type_specifier", $1); 
			$$->ast = context->get_built_in_type(BuiltInTypeName::VOID);
      context->current_type($$->type());
		}
 		;
 		
declaration_list : declaration_list COMMA ID {
          $$ = NonTerminal::create(context, @$, "declaration_list", $1, $2, $3);
          $$->ast = std::move($1->ast);
          $$->vardecls().push_back(
              VarDecl::create(context, @3, context->current_type(), $3->value())
          );
      }
 		  | declaration_list COMMA ID LSQUARE CONST_INT RSQUARE {
          $$ = NonTerminal::create(context, @$, "declaration_list", $1, $2, $3, $4, $5, $6);
          size_t size = std::atoi($5->value().c_str());
          auto type = context->current_type()->array_type()->sized_array(size);
          $$->ast = std::move($1->ast);
          $$->vardecls().push_back(
              VarDecl::create(context, @3, type, $3->value()));
      }
 		  | ID {
          $$ = NonTerminal::create(context, @$, "declaration_list", $1);
          $$->ast = VarDecls{};
          $$->vardecls().push_back(
              VarDecl::create(context, @$, context->current_type(), $1->value()));
      }
 		  | ID LSQUARE CONST_INT RSQUARE {
          $$ = NonTerminal::create(context, @$, "declaration_list", $1, $2, $3, $4);
          $$->ast = VarDecls{};
          size_t size = std::atoi($3->value().c_str());
          auto type = context->current_type()->sized_array(size);
          $$->vardecls().push_back(
              VarDecl::create(context, @3, type, $1->value()));
      }
      ;
 		  
statements : statement {
          $$ = NonTerminal::create(context, @$, "statements", $1);
          $$->ast = Stmts{};
          $$->stmts().push_back(std::move($1->stmt()));
      }
	   | statements statement {
          $$ = NonTerminal::create(context, @$, "statements", $1, $2);
          $$->ast = std::move($1->ast);
          $$->stmts().push_back(std::move($2->stmt()));
     }
	   ;
	   
statement : var_declaration {
          $$ = NonTerminal::create(context, @$, "statement", $1);
          $$->ast = DeclStmt::create(context, @$, std::move($1->vardecls()));
    } 
	  | expression_statement {
          $$ = NonTerminal::create(context, @$, "statement", $1);
          $$->ast = std::move($1->ast);
    }
	  | compound_statement {
          $$ = NonTerminal::create(context, @$, "statement", $1);
          $$->ast = std::move($1->ast);
    }
	  | FOR LPAREN expression_statement expression_statement expression RPAREN statement {
          $$ = NonTerminal::create(context, @$, "statement", $1, $2, $3, $4, $5, $6, $7);
          $$->ast = ForStmt::create(context, @$, std::move($3->expr()), std::move($4->expr()), 
                                      std::move($5->expr()));
    }
	  | IF LPAREN expression RPAREN statement %prec UNMATCHED_ELSE {
          $$ = NonTerminal::create(context, @$, "statement", $1, $2, $3, $4, $5);
          $$->ast = IfStmt::create(context, @$, std::move($3->expr()), std::move($5->stmt())
                                              , nullptr);
    }
	  | IF LPAREN expression RPAREN statement ELSE statement {
          $$ = NonTerminal::create(context, @$, "statement", $1, $2, $3, $4, $5, $6, $7);
          $$->ast = IfStmt::create(context, @$, std::move($3->expr()), std::move($5->stmt())
                                              , std::move($7->stmt()));
    }
	  | WHILE LPAREN expression RPAREN statement {
          $$ = NonTerminal::create(context, @$, "statement", $1, $2, $3, $4, $5);
          $$->ast = WhileStmt::create(context, @$, std::move($3->expr()), std::move($5->stmt()));
    }
	  /* | PRINTLN LPAREN ID RPAREN SEMICOLON */
	  | RETURN expression SEMICOLON {
          $$ = NonTerminal::create(context, @$, "statement", $1, $2, $3);
          $$->ast = ReturnStmt::create(context, @$, std::move($2->expr()));
    }
	  ;
	  
expression_statement 	: SEMICOLON	{
          $$ = NonTerminal::create(context, @$, "expression_statement", $1);
          $$->ast = ExprStmt::create(context, @$, nullptr);
      }
			| expression SEMICOLON {
          $$ = NonTerminal::create(context, @$, "expression_statement", $1, $2);
          $$->ast = ExprStmt::create(context, @$, std::move($1->expr()));
      }
			;
	  
variable : ID {
      $$ = NonTerminal::create(context, @$, "variable", $1);
      $$->ast = RefExpr::create(context, @$, $1);
   }
	 | ID LSQUARE expression RSQUARE {
      $$ = NonTerminal::create(context, @$, "variable", $1, $2, $3, $4);
      auto ref = RefExpr::create(context, @1, $1);
      $$->ast = ArraySubscriptExpr::create(context, @$, std::move(ref), std::move($3->expr()));
   } 
	 ;
	 
 expression : logic_expression	{
      $$ = NonTerminal::create(context, @$, "expression", $1);
      $$->ast = std::move($1->ast);
      }
	   | variable ASSIGNOP logic_expression {
      $$ = NonTerminal::create(context, @$, "expression", $1, $2, $3);
      $$->ast = BinaryExpr::create(context, @$, BinaryOp::ASSIGN, std::move($1->expr())
                                              , std::move($3->expr()));
     } 	
	   ;
			
logic_expression : rel_expression 	{
      $$ = NonTerminal::create(context, @$, "logic_expression", $1);
      $$->ast = std::move($1->ast);
      }
		 | rel_expression LOGICOP rel_expression 	{
      $$ = NonTerminal::create(context, @$, "logic_expression", $1, $2, $3);
      auto op = $2->value() == "&&" ? BinaryOp::LOGIC_AND : BinaryOp::LOGIC_OR;
      $$->ast = BinaryExpr::create(context, @$, op, std::move($1->expr())
                                              , std::move($3->expr()));
     }
		 ;
			
rel_expression	: simple_expression {
      $$ = NonTerminal::create(context, @$, "rel_expression", $1);
      $$->ast = std::move($1->ast);
    }
		| simple_expression RELOP simple_expression	{

      $$ = NonTerminal::create(context, @$, "rel_expression", $1, $2, $3);
      auto op = get_relop($2->value());
      $$->ast = BinaryExpr::create(context, @$, op, std::move($1->expr())
                                              , std::move($3->expr()));
    }
		;
				
simple_expression : term {
        $$ = NonTerminal::create(context, @$, "simple_expression", $1);
        $$->ast = std::move($1->ast);
      }
		  | simple_expression ADDOP term {
        $$ = NonTerminal::create(context, @$, "simple_expression", $1, $2, $3);
        auto op = $2->value() == "+" ? BinaryOp::ADD : BinaryOp::SUB;
        $$->ast = BinaryExpr::create(context, @$, op, std::move($1->expr())
                                              , std::move($3->expr()));
      } 
		  ;
					
term :	unary_expression {
        $$ = NonTerminal::create(context, @$, "term", $1);
        $$->ast = std::move($1->ast);
      }
     |  term MULOP unary_expression {
        $$ = NonTerminal::create(context, @$, "term", $1, $2, $3);
        auto op = $2->value() == "*" ? BinaryOp::MUL : BinaryOp::DIV;
        $$->ast = BinaryExpr::create(context, @$, op, std::move($1->expr())
                                              , std::move($3->expr()));
     }  
     ;

unary_expression : ADDOP unary_expression  {
        $$ = NonTerminal::create(context, @$, "unary_expression", $1, $2);
        auto op = $1->value() == "+" ? UnaryOp::PLUS : UnaryOp::MINUS;
        $$->ast = UnaryExpr::create(context, @$, op, std::move($2->expr()));
      }
		 | NOT unary_expression {
        $$ = NonTerminal::create(context, @$, "unary_expression", $1);
        $$->ast = UnaryExpr::create(context, @$, UnaryOp::LOGIC_NOT, std::move($2->expr()));
     } 
		 | factor {
        $$ = NonTerminal::create(context, @$, "unary_expression", $1);
        $$->ast = std::move($1->ast);
     } 
		 ;
	
factor	: variable {
      $$ = NonTerminal::create(context, @$, "factor", $1);
      $$->ast = std::move($1->expr());
  }
	| ID LPAREN argument_list RPAREN {
      $$ = NonTerminal::create(context, @$, "factor", $1, $2, $3, $4);
      auto ref = RefExpr::create(context, @1, $1);
      $$->ast = CallExpr::create(context, @$, std::move(ref), std::move($3->exprs()));
  }
	| LPAREN expression RPAREN {
      $$ = NonTerminal::create(context, @$, "factor", $1, $2, $3);
      $$->ast = std::move($2->expr());
  }
	| CONST_INT {
      $$ = NonTerminal::create(context, @$, "factor", $1);
      $$->ast = IntLiteral::create(context, @$, $1);
  }
	| CONST_FLOAT {
      $$ = NonTerminal::create(context, @$, "factor", $1);
      $$->ast = FloatLiteral::create(context, @$, $1);
  }
	| variable INCOP {
      $$ = NonTerminal::create(context, @$, "factor", $1, $2);
      $$->ast = UnaryExpr::create(context, @$, UnaryOp::POST_INC, std::move($1->expr()));
  }
	| variable DECOP {
      $$ = NonTerminal::create(context, @$, "factor", $1, $2);
      $$->ast = UnaryExpr::create(context, @$, UnaryOp::POST_DEC, std::move($1->expr()));
  }
	;
	
argument_list : arguments {
            $$ = NonTerminal::create(context, @$, "argument_list", $1);
            $$->ast = std::move($1->exprs());
        }
			  | {
            $$ = NonTerminal::create(context, @$, "argument_list");
            $$->ast = Exprs{};
        }
			  ;
	
arguments : arguments COMMA logic_expression {
            $$ = NonTerminal::create(context, @$, "arguments", $1, $2, $3);
            $$->ast = std::move($1->ast);
            $$->exprs().push_back(std::move($3->expr()));
        }
	      | logic_expression {
            $$ = NonTerminal::create(context, @$, "arguments", $1);
            $$->ast = Exprs{};
            $$->exprs().push_back(std::move($1->expr()));
        }
	      ;
 
 lcurl	: LCURL {
      $$ = $1;
      context->enter_scope();
      if (context->current_params()){
        for(auto& param : *context->current_params()){
          if (param->name() == ""){
            context->report_error(param->location(), "Nameless param");
            continue;
          }	
          if (!context->insert_symbol(param->name(), SymbolType::PARAM, param.get())){
            context->report_error(param->location(), "Param redefinition");
          }
        }
        context->current_params(nullptr);
      }
 		}
		;

%%

void yyerror(YYLTYPE* locp, void* scanner, ParserContext* context, const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  if(locp->first_line) {
    std::fprintf(stderr, "Line #%d: error: ", locp->first_line);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

void lyyerror(YYLTYPE t, char *s, ...) {
  va_list ap;
  va_start(ap, s);
  if(t.first_line) {
    std::fprintf(stderr, "Line #%d: error: ", t.first_line);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
