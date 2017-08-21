%code requires {
#include <vector>
#include <string>
#include <memory>
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
void yyerror (char const *s);
extern "C" int yylex(void);
}

%locations
%debug
%token INTERFACE NAMESPACE ID EXTENDS  EXPORT CONST TYPE
%token BOOLEAN NUMBER STRING ANY JSON_NULL
%token BOOLEAN_LITERAL NUMBER_LITERAL STRING_LITERAL
%%
json
  : json_item
  | json json_item
  ;

json_item
  : json_dict
  | json_template
  | json_namespace
  | json_type
  | json_variable
  | json_export
  ;

json_dict
  : INTERFACE ID dict_body
  | INTERFACE ID EXTENDS ID dict_body
  ;

dict_body
  : '{' dict_member_seq '}'
  ;

dict_member_seq
  : dict_member
  | dict_member_seq dict_member
  ;

dict_member
  : ID ':' type_or_value
  | ID '?'':' type_or_value
  ;

type_or_value
  : type
  | type2_seq
  | type '=' values
  | '=' values
  ;

type2_seq
  : type '|' type
  | type2_seq '|' type
  ;

type
  : non_list_type
  | list
  ;

non_list_type
  : ID
  | template_id
  | BOOLEAN
  | NUMBER
  | STRING
  | ANY
  | JSON_NULL
  | dict_body
  ;

template_id
  : ID '<' template_argument_list '>'
  ;

list
  : non_list_type '[' ']'
  ;

values
  : value
  | values '|' value
  ;

value
  : ID
  | literal
  ;

literal
  : dict_literal
  | list_literal
  | BOOLEAN_LITERAL
  | NUMBER_LITERAL
  | STRING_LITERAL
  | ANY
  | JSON_NULL
  ;

dict_literal
  : '{' kv_seq '}'
  ;

kv_seq
  : kv
  | kv_seq kv
  ;

kv
  : STRING_LITERAL ':' value
  ;

list_literal
  : '[' val_seq ']'
  ;

val_seq
  : value
  | val_seq value
  ;

json_template
  : INTERFACE ID '<'template_parameter_list '>' dict_body
  ;

template_parameter_list
  : ID
  | template_id ID
  ;

template_argument_list
  : type
  | template_argument_list type
  ;

json_namespace
  : NAMESPACE ID '{' ns_member_seq '}'
  ;

ns_member_seq
  : json_variable
  | ns_member_seq json_variable
  ;

json_type
  : TYPE ID '=' type
json_variable
  : json_variable1
  | CONST json_variable
  | EXPORT json_variable
  ;

json_variable1
  : ID ':' type '=' value
  | ID '=' value
  ;

/* json_variable eat the 'export' them self */
json_export
  : EXPORT json_dict
  | EXPORT json_template
  | EXPORT json_namespace
  | EXPORT json_type
  ;

%%
int main() {
  yydebug = 0;
  int res = yyparse();
  switch (res) {
    case(0):
      llvm::outs() << "yyparse() successfully finished.\n";
      break;
    case(1):
      llvm::outs() << "yyparse() failed: invalid input.\n";
      break;
    case(2):
      llvm::outs() << "yyparse() failed: memory exhaustion.\n";
      break;
    default:
      llvm::outs() << "wrong return value from yyparse()\n";
  }
}

namespace boost {
void throw_exception( std::exception const & e ) {
  llvm::errs() << "user provided boost::throw_exception() called.\naborting...\n";
  abort();
}
}
