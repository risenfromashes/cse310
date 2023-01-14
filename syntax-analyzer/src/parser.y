%language "C++"

%define api.token.constructor false
%define api.value.type variant
%parse-param {ParserContext* context}
%locations

%{

#include <cstdio>
#include <token.h>

#define YYLEX_PARAM context->scanner()

int yylex(YYSTYPE* lvalp, YYLTYPE* llocp, void* scanner);
void yyerror(YYLTYPE* locp, ParserContext* context, const char *s, ...); 
void lyyerror(YYLTYPE t, char *s, ...); 

%}

/* regular tokens */
%token <Token> IF ELSE FOR WHILE DO BREAK INT CHAR FLOAT DOUBLE VOID RETURN SWITCH CASE DEFAULT 
%token <Token> CONTINUE CONST_INT CONST_FLOAT CONST_CHAR ADDOP MULOP INCOP DECOP RELOP ASSIGNOP LOGICOP 
%token <Token> BITOP NOT LPAREN RPAREN LCURL RCURL LSQUARE RSQUARE COMMA SEMICOLON ID STRING MULTI_LINE_STRING


%%

start : program
	{
		//write your code in this block in all the similar blocks below
	}
	;

program : program unit 
	| unit
	;
	
unit : var_declaration
     | func_declaration
     | func_definition
     ;
     
func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON
		| type_specifier ID LPAREN RPAREN SEMICOLON
		;
		 
func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement
		| type_specifier ID LPAREN RPAREN compound_statement
 		;				


parameter_list  : parameter_list COMMA type_specifier ID
		| parameter_list COMMA type_specifier
 		| type_specifier ID
		| type_specifier
 		;

 		
compound_statement : LCURL statements RCURL
 		    | LCURL RCURL
 		    ;
 		    
var_declaration : type_specifier declaration_list SEMICOLON
 		 ;
 		 
type_specifier	: INT
 		| FLOAT
 		| VOID
 		;
 		
declaration_list : declaration_list COMMA ID
 		  | declaration_list COMMA ID LSQUARE CONST_INT RSQUARE
 		  | ID
 		  | ID LSQUARE CONST_INT RSQUARE
 		  ;
 		  
statements : statement
	   | statements statement
	   ;
	   
statement : var_declaration
	  | expression_statement
	  | compound_statement
	  | FOR LPAREN expression_statement expression_statement expression RPAREN statement
	  | IF LPAREN expression RPAREN statement
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
 

%%

void yyerror(YYLTYPE* locp, ParserContext* context, const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  if(t.first_line) {
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
