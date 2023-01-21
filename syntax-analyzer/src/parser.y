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
%type <non_term> term unary_expression factor variable argument_list arguments lcurl



/* precedence rules */
%nonassoc UNMATCHED_ELSE
%nonassoc ELSE

%%

start : program {
    $$ = NonTerminal::create("start", $1);
    auto root = TranslationUnitDecl::create(context, @$, std::move($1->decls()));
    context->set_ast_root(std::move(root));
    context->set_pt_root($$);
	}
	;

program : program unit {
    $$ = NonTerminal::create("program", $1, $2);
    $$->ast = std::move($$->ast);
    auto& t = $$->decls();
    auto& f = $1->decls();
    std::move(f.begin(), f.end(), std::back_inserter(t));
  } 
	| unit {
    $$ = NonTerminal::create("program", $1);
    $$->ast = std::move($1->ast);
  }
	;
	
unit : var_declaration {
        $$ = NonTerminal::create("unit", $1);
        $$->ast = Decls();
        for(auto& v: $1->vardecls()){
          $$->decls().push_back(std::move(v));
        }
     }
     | func_declaration {
        $$ = NonTerminal::create("unit", $1);
        $$->ast = Decls(); 
        $$->decls().push_back(std::move($1->decl()));
     }
     | func_definition { 
        $$ = NonTerminal::create("unit", $1);
        $$->ast = Decls{}; 
        $$->decls().push_back(std::move($1->decl()));
     }
     ;
     
func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON {
        $$ = NonTerminal::create("func_declaration", $1, $2, $3, $4, $5, $6);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    std::move($4->paramdecls()),
                                    $2->move_value(), nullptr);
        /* no declaration to follow */
        context->current_params(nullptr);
    }
		| type_specifier ID LPAREN RPAREN SEMICOLON {
        $$ = NonTerminal::create("func_declaration", $1, $2, $3, $4, $5);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    ParamDecls(),
                                    $2->move_value(), nullptr);
        /* no declaration to follow */
        context->current_params(nullptr);
    }
		;
		 
func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement {
        $$ = NonTerminal::create("func_definition", $1, $2, $3, $4, $5, $6);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    std::move($4->paramdecls()),
                                    $2->move_value(), std::move($6->compound_stmt()));
    }
		| type_specifier ID LPAREN RPAREN compound_statement {
        $$ = NonTerminal::create("func_definition", $1, $2, $3, $4, $5);
        $$->ast = FuncDecl::create(context, @$, $1->type(), 
                                    ParamDecls(),
                                    $2->move_value(), std::move($5->compound_stmt()));
    }
 		;				


parameter_list  : parameter_list COMMA type_specifier ID {
        $$ = NonTerminal::create("parameter_list", $1, $2, $3, $4);
        $$->paramdecls().push_back(ParamDecl::create(context, @$, $3->type(), 
                                      $4->move_value()));
    }
		| parameter_list COMMA type_specifier {
        $$ = NonTerminal::create("parameter_list", $1, $2, $3);
        $$->paramdecls().push_back(ParamDecl::create(context, @$, $3->type(), ""));
    }
 		| type_specifier ID {
        $$ = NonTerminal::create("parameter_list", $1, $2);
        $$->ast = ParamDecls();
        $$->paramdecls().push_back(ParamDecl::create(context, @$, $1->type(), $2->move_value()));
        context->current_params(&$$->paramdecls());
    }
		| type_specifier {
        $$ = NonTerminal::create("parameter_list", $1);
        $$->ast = ParamDecls();
        $$->paramdecls().push_back(ParamDecl::create(context, @$, $1->type(), ""));
        context->current_params(&$$->paramdecls());
    }
 		;

 		
compound_statement : LCURL statements RCURL {
          $$ = NonTerminal::create("compound_statement", $1, $2, $3);
          $$->ast = CompoundStmt::create(context, @$, std::move($$->stmts()));
          context->exit_scope();
        }
 		    | LCURL RCURL {
          $$ = NonTerminal::create("compound_statement", $1, $2);
          $$->ast = CompoundStmt::create(context, @$, std::move($$->stmts()));
          context->exit_scope();
        }
 		    ;
 		    
var_declaration : type_specifier declaration_list SEMICOLON {
          $$ = NonTerminal::create("var_declaration", $1, $2, $3);
          $$->ast = std::move($$->ast);
     }
 		 ;
 		 
type_specifier	: INT { 
			$$ = NonTerminal::create("type_specifier", $1); 
			$$->ast = context->get_built_in_type(BuiltInTypeName::INT);
      context->current_type($$->type());
		}
 		| FLOAT {
			$$ = NonTerminal::create("type_specifier", $1); 
			$$->ast = context->get_built_in_type(BuiltInTypeName::FLOAT);
      context->current_type($$->type());
		}
 		| VOID {
			$$ = NonTerminal::create("type_specifier", $1); 
			$$->ast = context->get_built_in_type(BuiltInTypeName::VOID);
      context->current_type($$->type());
		}
 		;
 		
declaration_list : declaration_list COMMA ID {
          $$ = NonTerminal::create("declaration_list", $1, $2, $3);
          $$->vardecls().push_back(
              VarDecl::create(context, @3, context->current_type(), $3->move_value())
          );
      }
 		  | declaration_list COMMA ID LSQUARE CONST_INT RSQUARE {
          $$ = NonTerminal::create("declaration_list", $1, $2, $3, $4, $5, $6);
          size_t size = std::atoi($5->value()->data());
          auto type = context->current_type()->array_type()->sized_array(size);
          $$->vardecls().push_back(
              VarDecl::create(context, @3, type, $3->move_value()));
      }
 		  | ID {
          $$ = NonTerminal::create("declaration_list", $1);
          $$->ast = VarDecls{};
          $$->vardecls().push_back(
              VarDecl::create(context, @$, context->current_type(), $1->move_value()));
      }
 		  | ID LSQUARE CONST_INT RSQUARE {
          $$ = NonTerminal::create("declaration_list", $1, $2, $3, $4);
          $$->ast = VarDecls{};
          size_t size = std::atoi($3->value()->data());
          auto type = context->current_type()->array_type()->sized_array(size);
          $$->vardecls().push_back(
              VarDecl::create(context, @3, type, $1->move_value()));
      }
 		  ;
 		  
statements : statement {
          $$ = NonTerminal::create("statements", $1);
          $$->ast = Stmts{};
          $$->stmts().push_back(std::move($1->stmt()));
      }
	   | statements statement {
          $$ = NonTerminal::create("statements", $1, $2);
          $$->ast = std::move($1->ast);
          $$->stmts().push_back(std::move($2->stmt()));
     }
	   ;
	   
statement : var_declaration 
	  | expression_statement
	  | compound_statement {
          $$ = NonTerminal::create("statement", $1);
          $$->ast = std::move($1->ast);
    }
	  | FOR LPAREN expression_statement expression_statement expression RPAREN statement
	  | IF LPAREN expression RPAREN statement %prec UNMATCHED_ELSE {}
	  | IF LPAREN expression RPAREN statement ELSE statement
	  | WHILE LPAREN expression RPAREN statement
	  /* | PRINTLN LPAREN ID RPAREN SEMICOLON */
	  | RETURN expression SEMICOLON
	  ;
	  
expression_statement 	: SEMICOLON			
			| expression SEMICOLON 
			;
	  
variable : ID 		
	 | ID LSQUARE expression RSQUARE 
	 ;
	 
 expression : logic_expression	
	   | variable ASSIGNOP logic_expression 	
	   ;
			
logic_expression : rel_expression 	
		 | rel_expression LOGICOP rel_expression 	
		 ;
			
rel_expression	: simple_expression 
		| simple_expression RELOP simple_expression	
		;
				
simple_expression : term 
		  | simple_expression ADDOP term 
		  ;
					
term :	unary_expression
     |  term MULOP unary_expression
     ;

unary_expression : ADDOP unary_expression  
		 | NOT unary_expression 
		 | factor 
		 ;
	
factor	: variable 
	| ID LPAREN argument_list RPAREN
	| LPAREN expression RPAREN
	| CONST_INT 
	| CONST_FLOAT
	| variable INCOP 
	| variable DECOP
	;
	
argument_list : arguments
			  |
			  ;
	
arguments : arguments COMMA logic_expression
	      | logic_expression
	      ;
 
 lcurl	: LCURL {
      context->enter_scope();
      if (context->current_params()){
        for(auto& param : *context->current_params()){
          if (param->name() == ""){
            context->report_error(param->location(), "Nameless param");
            continue;
          }	
          if (!context->insert_symbol(param->name(), SymbolType::PARAM, param->decl())){
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
