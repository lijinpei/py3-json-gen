%code requires {
#include "requires.h"
}

%define api.value.type union
%locations
%debug
%token <StringRef *> ID STRING_LITERAL
%token INTERFACE NAMESPACE EXTENDS  EXPORT CONST TYPE BOOLEAN NUMBER STRING
%token <SpecialValue> ANY JSON_NULL
%token <bool> BOOLEAN_LITERAL
%token <int> NUMBER_LITERAL
%type <Json*> json
%type <JsonItem*> json_item
%type <JsonDict*> json_dict
%type <NamedType*> named_type
%type <DictBody*> dict_body
%type <DictBody*> dict_member_seq
%type <DictMember*> dict_member
%type <TypeOrValue*> type_or_value
%type <Types*> type2_seq
%type <Type*> type non_list_type
%type <TemplateID*> template_id
%type <JsonList*> list
%type <Values*> values
%type <Value*> value literal
%type <DictLiteral*> dict_literal kv_seq
%type <StrValuePair*> kv
%type <ListLiteral*> list_literal val_seq
%type <JsonTemplate*> json_template
%type <TemplateParameterList*> template_parameter_list
%type <TemplateArgumentList*> template_argument_list
%type <JsonNamespace*> json_namespace ns_member_seq
%type <JsonType*> json_type
%type <JsonVariable*> json_variable json_variable1
%type <JsonExport*> json_export

%%

/* Json */
json
  : json_item {
    $$ = new Json;
    $$->json_items.push_front($1);
  }
  | json json_item {
    $1->json_items.push_front($2);
    $$ = $1;
  }
  ;

/* JsonItem */
json_item
  : json_dict {
    $$ = new JsonItem($1);
  }
  | json_template {
    $$ = new JsonItem($1);
  }
  | json_namespace{
    $$ = new JsonItem($1);
  }
  | json_type{
    $$ = new JsonItem($1);
  }
  | json_variable{
    $$ = new JsonItem($1);
  }
  | json_export{
    $$ = new JsonItem($1);
  }
  ;

/* JsonDict */
json_dict
  : INTERFACE ID dict_body {
    $$ = new JsonDict(*$2, nullptr, $3);
    delete $2;
  }
  | INTERFACE ID EXTENDS named_type dict_body {
    $$ = new JsonDict(*$2, $4, $5);
    delete $2;
  }
  ;

named_type
  : ID {
    $$ = new NamedType;
    $$->type = *$1;
    delete $1;
  }
  | template_id {
    $$ = new NamedType($1);
  }
  ;

/* DictBody */
dict_body
  : '{' dict_member_seq '}' {
    $$ = $2;
  }
  ;

/* DictBody */
dict_member_seq
  : dict_member {
    $$ = new DictBody;
    $$->members.push_front($1);
  }
  | dict_member_seq dict_member {
    $$ = $1;
    $$->members.push_front($2);
  }
  ;

/* DictMember */
dict_member
  : ID ':' type_or_value {
    $$ = new DictMember(*$1, $3, false);
    delete $1;
  }
  | ID '?'':' type_or_value {
    $$ = new DictMember(*$1, $4, true);
    delete $1;
  }
  ;

/* TypeOrValue */
type_or_value
  : type {
    $$ = new TypeOrValue;
    $$->types.push_front($1);
  }
  | type2_seq {
    $$ = new TypeOrValue;
    $$->types = std::move(*$1);
    delete $1;
  }
  | type '=' values {
    $$ = new TypeOrValue;
    $$->types.push_front($1);
    $$->values = std::move(*$3);
    delete $3;
  }
  | '=' values {
    $$ = new TypeOrValue;
    $$->values = std::move(*$2);
    delete $2;
  }
  ;

/* Types */
type2_seq
  : type '|' type {
    $$ = new Types;
    $$->push_front($3);
    $$->push_front($1);
  }
  | type2_seq '|' type {
    $$ = $1;
    $$->push_front($3);
  }
  ;

/* Type */
type
  : non_list_type {
    $$ = $1;
  }
  | list {
    $$ = new Type{$1};
  }
  ;

/* Type */
non_list_type
  : ID {
    $$ = new Type{*$1};
    delete $1;
  }
  | template_id {
    $$ = new Type{$1};
  }
  | BOOLEAN {
    $$ = new Type{BuiltinTypes::BT_Boolean};
  }
  | NUMBER {
    $$ = new Type{BuiltinTypes::BT_Number};
  }
  | STRING {
    $$ = new Type{BuiltinTypes::BT_String};
  }
  | ANY {
    $$ = new Type{BuiltinTypes::BT_Any};
  }
  | JSON_NULL {
    $$ = new Type{BuiltinTypes::BT_JsonNull};
  }
  | dict_body {
    $$ = new Type{$1};
  }
  ;

/* TemplateID */
template_id
  : ID '<' template_argument_list '>' {
    $$ = new TemplateID($3); 
    $$->name = *$1;
    $$->temp = nullptr;
    $$->body = nullptr;
    delete $1;
  }
  ;

/* JsonList */
list
  : non_list_type '[' ']' {
    $$ = new JsonList;
    $$->type = $1;
  }
  ;

/* Values */
values
  : value {
    $$ = new Values;
    $$->push_front($1);
  }
  | values '|' value {
    $$ = $1;
    $$->push_front($3);
  }
  ;

/* Value */
value
  : ID {
    fprintf(stderr, "getVariableValue()\n");
    $$ = getVariableValue(*$1);
    delete $1;
  }
  | literal {
    $$ = $1;
  }
  ;

/* Value */
literal
  : dict_literal {
    $$ = new Value{$1};
  }
  | list_literal {
    $$ = new Value{$1};
  }
  | BOOLEAN_LITERAL {
    $$ = new Value{$1};
  }
  | NUMBER_LITERAL {
    $$ = new Value{$1};
  }
  | STRING_LITERAL {
    $$ = new Value{$1};
  }
  | ANY {
    $$ = new Value{SpecialValue::SV_Any};
  }
  | JSON_NULL {
    $$ = new Value{SpecialValue::SV_JsonNull};
  }
  ;

/* DictLiteral */
dict_literal
  : '{' kv_seq '}' {
    $$ = $2;
  }
  ;

/* DictLiteral */
kv_seq
  : kv {
    DictLiteral * p = new DictLiteral;
    p->members->push_front($1);
    $$ = p;
  }
  | kv_seq kv {
    $$ = $1;
    $$->members->push_front($2);
  }
  ;

/* StrValuePair */
kv
  : STRING_LITERAL ':' value {
    $$ = new StrValuePair{*$1, $3};
    delete $1;
  }
  ;

/* ListLiteral */
list_literal
  : '[' val_seq ']' {
    $$ = $2;
  }
  ;

/* ListLiteral */
val_seq
  : value {
    $$ = new ListLiteral;
    $$->values->push_front($1);
  }
  | val_seq value {
    auto p = $1;
    p->values->push_front($2);
    $$ = p;
  }
  ;

/* JsonTemplate */
json_template
  : INTERFACE ID '<'template_parameter_list '>' dict_body {
    $$ = new JsonTemplate{$2, $4, $6};
    addTemplateDef(*$2, $$);
  }
  ;

/* TemplateParameterList */
template_parameter_list
  : ID {
    $$ = new TemplateParameterList;
    $$->push_front($1);
  }
  | template_parameter_list ID {
    $$ = $1;
    $$->push_front($2);
  }
  ;

/* TemplateArgumentList */
template_argument_list
  : type { $$ = new TemplateArgumentList(); }
  | template_argument_list type {
    $$ = $1;
    $$->push_front($2);
  }
  ;

/* JsonNamespace */
json_namespace
  : NAMESPACE ID '{' ns_member_seq '}' {
    $$ = $4;
    $$->name = *$2;
    delete $2;
  }
  ;

/* JsonNamespace */
ns_member_seq
  : json_variable {
    $$ = new JsonNamespace;
  }
  | ns_member_seq json_variable {
    $$ = $1;
    $1->members.push_front($2);
  }
  ;

/* JsonType */
json_type
  : TYPE ID '=' type {
    $$ = new JsonType;
    $$->name = *$2;
    delete $2;
    $$->type = $4;
  }
  ;

/* JsonVariable */
json_variable
  : json_variable1 {
    $$ = $1;
  }
  | CONST json_variable {
    $$ = $2;
    $$->constant = true;
  }
  | EXPORT json_variable {
    $$ = $2;
    $$->exported = true;
  }
  ;

/* JsonVariable */
json_variable1
  : ID ':' type '=' value {
    $$ = new JsonVariable;
    $$->name = *$1;
    delete $1;
    $$->type = $3;
    $$->value = $5;
  }
  | ID '=' value {
    $$ = new JsonVariable;
    $$->name = *$1;
    delete $1;
    $$->type = nullptr;
    $$->value = $3;
  }
  ;

/* json_variable eat the 'export' them self */
/* JsonExport */
json_export
  : EXPORT json_dict {
    new JsonExport{$2};
  }
  | EXPORT json_template {
    new JsonExport{$2};
  }
  | EXPORT json_namespace {
    new JsonExport{$2};
  }
  | EXPORT json_type {
    new JsonExport{$2};
  }
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

StringMap<Value*> VariableTable;
StringMap<JsonDict*> DictTable;
StringMap<JsonTemplate*> TemplateTable;
forward_list<string> StringTable;

Value * addVariableDef(StringRef name, Value * v) {
  bool ret = VariableTable.insert(make_pair(name, v)).second;
  if (!ret) {
    llvm::errs() << "error in addVariableDef(): variable already exists\n";
    abort();
  }
  return v;
}

Value * getVariableValue(StringRef name) {
  auto iter = VariableTable.find(name);
  if (iter == VariableTable.end()) {
    llvm::errs() << "error in getVariableValue(): variable not found\n";
  }
  return iter->getValue();
}

JsonDict * addDictDef(StringRef name, JsonDict * dict) {
  bool ret = DictTable.insert(make_pair(name, dict)).second;
  if (!ret) {
    llvm::errs() << "error in addDictDef(): dict already exists\n";
    abort();
  }
  return dict;
}

JsonDict * getDictDef(StringRef name) {
  auto iter = DictTable.find(name);
  if (iter == DictTable.end()) {
    llvm::errs() << "error in getDictDef(): dict not found\n";
    llvm::errs() << "line number: " << yylineno << '\n';
    llvm::errs() << "dict name: " << name << '\n';
    abort();
  }
  return iter->getValue();
}

JsonTemplate * addTemplateDef(StringRef name, JsonTemplate * temp) {
  bool ret = TemplateTable.insert(make_pair(name, temp)).second;
  if (!ret) {
    llvm::errs() << "error in addTemplateDef(): template already exists\n";
    abort();
  }
  return temp;
}

JsonTemplate * getTemplateDef(StringRef name) {
  auto iter = TemplateTable.find(name);
  if (iter == TemplateTable.end()) {
    llvm::errs() << "error in getTemplateValue(): template not found\n";
    llvm::errs() << "line number: " << yylineno << '\n';
    llvm::errs() << "template name : " << name << '\n';
    abort();
  }
  return iter->getValue();
}

namespace boost {
void throw_exception( std::exception const & e ) {
  llvm::errs() << "user provided boost::throw_exception() called.\naborting...\n";
  abort();
}
}
