%code requires {
#include <vector>
#include <string>
#include <memory>
#include <forward_list>
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
#include "boost/variant/variant.hpp"

void yyerror (char const *s);
extern "C" int yylex(void);

using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::forward_list;
using llvm::StringRef;
using llvm::StringMap;
using llvm::raw_ostream;
using boost::variant;

/* All the ast nodes must be move-able (perhaps except the root node) */
/* v means this class is move-able */
/* p means move of this class is implemented through unique_ptr (as directly move the class would be costly) */
/* All the ast nodes that needs forward declaration can't be 'using, typedef' */
/* All the string literals and names appears as StringRef to StringTable */
/* All named types (including templates) appears as shared_ptr to NamedTypeTable */
/* All ast node class appears directly in other ast node class (directly means not throught pointer) 
 * as all the ast node are cheap to move */
/* YYSTYPE is a variant of all the move-able ast nodes */

/* To define Json */
struct JsonDict;
struct JsonTemplate;
struct JsonNamespace;
struct JsonType;
struct JsonVariable;
struct JsonExport;

using JsonItem = variant<shared_ptr<JsonDict>, shared_ptr<JsonTemplate>, shared_ptr<JsonNamespace>, shared_ptr<JsonType>, shared_ptr<JsonVariable>, shared_ptr<JsonExport>>;

struct Json {
  forward_list<JsonItem> json_items;
};

/* To define JsonDict */
struct Type;
struct Value;

/* v */
using Types = forward_list<Type>;
/* v */
using Values = forward_list<Value>;

/* v */
struct TypeOrValue {
  Types types;
  Values values;
};

/* p */
struct DictMember {
  struct DictMember_impl {
    StringRef name;
    unique_ptr<TypeOrValue> type_or_value;
    bool optional;
  };
  unique_ptr<DictMember_impl> p;
};

/* v */
struct DictBody {
  unique_ptr<forward_list<unique_ptr<DictMember>>> members;
};

/* v */
struct JsonDict {
  StringRef name;
  shared_ptr<JsonDict> base;
  unique_ptr<DictBody> body;
};

/* To define JsonTemplate */
/* v */
using TemplateParameterList = unique_ptr<forward_list<StringRef>>;
struct Template {
  StringRef name;
  TemplateParameterList temp_par_list;
  unique_ptr<DictBody> body;
};
/* v */
using TemplateArgumentList = unique_ptr<forward_list<shared_ptr<Type>>>;
/* v */
struct TemplateID {
  shared_ptr<Template> temp;
  TemplateArgumentList temp_arg_list;
  unique_ptr<DictBody> body; // body after template instantiation
};

/* To define Type */
enum BuitinTypes {
  BT_Boolean,
  BT_Number,
  BT_String,
  BT_JsonNull,
  BT_Any
};

/* v */
struct JsonList {
  unique_ptr<Type> type;
};

/* v */
struct Type {
  variant<BuitinTypes, shared_ptr<JsonDict>, shared_ptr<JsonTemplate>, shared_ptr<JsonType>, unique_ptr<JsonList>, unique_ptr<DictBody>> type;
};

/* To define Value */
enum SpecialValue {
  SV_Any,
  SV_JsonNull
};
/* v */
using StringLiteral = StringRef;
/* v */
using BooleanLiteral = bool;
/* v */
using NumberLiteral = int;

/* v */
struct StrValuePair {
  StringRef str;
  unique_ptr<Value> val;
};

/* v */
struct DictLiteral {
  unique_ptr<forward_list<StrValuePair>> members;
};

/* v */
struct ListLiteral {
  unique_ptr<forward_list<unique_ptr<Value>>> values;
};

/* We won't record which variable the value is form */
/* v */
struct Value {
  variant<SpecialValue, StringLiteral, BooleanLiteral, NumberLiteral, DictLiteral, ListLiteral> value;
};

/* To define JsonVariable and JsonNamepace */
/* p */
struct JsonVariable {
  struct JsonVariable_ {
    StringRef name;
    Value value;
    bool constant;
    bool exported;
  };
  unique_ptr<JsonVariable_> p;
};

/* v */
struct JsonNamespace {
  unique_ptr<forward_list<unique_ptr<JsonVariable>>> members;
};

/* v */
struct JsonExport {
  variant<unique_ptr<JsonDict>, unique_ptr<JsonTemplate>, unique_ptr<JsonNamespace>, unique_ptr<JsonType>> exported;
};


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
