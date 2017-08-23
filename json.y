%code requires {
#include "ast.h"
#include "pre_dump.h"
#include "codegen.h"
}

%define api.value.type union
%locations
%debug
%token <StringRef *> STRING_LITERAL ID
%token INTERFACE NAMESPACE EXTENDS  EXPORT CONST TYPE BOOLEAN NUMBER STRING
%token <SpecialValue> ANY JSON_NULL
%token <bool> BOOLEAN_LITERAL
%token <int> NUMBER_LITERAL
%type <Json*> json
%type <JsonItem*> json_item
%type <JsonDict*> json_dict
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
    $$->json_items.push_back($1);
    json_root = $$;
  }
  | json json_item {
    $1->json_items.push_back($2);
    $$ = $1;
    json_root = $$;
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
    resolveDictBody($3);
    $$ = new JsonDict($2, nullptr, $3);
    resolveDictBody($3);
    Type * t = addType(new Type{$$});
    addNamedType($2, t);
  }
  | INTERFACE ID EXTENDS type dict_body {
    if ($4->type.type() != boost::typeindex::type_id<StringRef*>()) {
      llvm::errs() << "syntax error: only Dict can serve as base class \n";
      llvm::errs() << "line: " << yylineno << "\naborting\n";
      abort();
    }
    StringRef * name = boost::get<StringRef*>($4->type);
    Type * t = getNamedType(name);
    if (t == nullptr) {
      llvm::errs() << "Error: unknown base type " << *name << "\naborting\n";
      abort();
    }
    delete name;
    delete $4;
    resolveDictBody($5);
    $$ = new JsonDict($2, t, $5);
    t = addType(new Type{$$});
    addNamedType($2, t);
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
    $$->members.push_back($1);
  }
  | dict_member_seq dict_member {
    $$ = $1;
    $$->members.push_back($2);
  }
  ;

/* DictMember */
dict_member
  : ID type_or_value {
    $$ = new DictMember($1, $2, false);
  }
  | ID '?' type_or_value {
    $$ = new DictMember($1, $3, true);
  }
  ;

/* TypeOrValue */
type_or_value
  : ':' type {
    $$ = new TypeOrValue;
    $$->types.push_back($2);
  }
  | ':' type2_seq {
    $$ = new TypeOrValue;
    $$->types = std::move(*$2);
    delete $2;
  }
  | ':' type '=' values {
    $$ = new TypeOrValue;
    $$->types.push_back($2);
    $$->values = std::move(*$4);
    delete $4;
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
    $$->push_back($3);
    $$->push_back($1);
  }
  | type2_seq '|' type {
    $$ = $1;
    $$->push_back($3);
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
  /*
    Type * t = getNamedType($1);
    if (nullptr == t) {
      llvm::errs() << "unable to resolve type: " << *$1 <<"\nline number: " << yylineno <<"\naborting\n";
      abort();
    }
  */
    $$ = new Type{$1};
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
    /* need to resolve type now */
    $$ = new Type{$1};
    resolveDictBody($1);
  }
  ;

/* TemplateID */
template_id
  : ID '<' template_argument_list '>' {
    $$ = new TemplateID($1, $3); 
    resolveTemplateArgumentList($3);
    $$->Instantiate();
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
    $$->push_back($1);
  }
  | values '|' value {
    $$ = $1;
    $$->push_back($3);
  }
  ;

/* Value */
value
  : ID {
    fprintf(stderr, "getVariableValue()\n");
    $$ = getVariableValue($1);
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
    p->members->push_back($1);
    $$ = p;
  }
  | kv_seq kv {
    $$ = $1;
    $$->members->push_back($2);
  }
  ;

/* StrValuePair */
kv
  : STRING_LITERAL ':' value {
    $$ = new StrValuePair{$1, $3};
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
    $$->values->push_back($1);
  }
  | val_seq value {
    auto p = $1;
    p->values->push_back($2);
    $$ = p;
  }
  ;

/* JsonTemplate */
json_template
  : INTERFACE ID '<'template_parameter_list '>' dict_body {
    $$ = new JsonTemplate{$2, $4, $6};
    addTemplateDef($2, $$);
    resolveDictBody($6, $4);
  }
  ;

/* TemplateParameterList */
template_parameter_list
  : ID {
    $$ = new TemplateParameterList;
    $$->push_back($1);
  }
  | template_parameter_list ID {
    $$ = $1;
    $$->push_back($2);
  }
  ;

/* TemplateArgumentList */
template_argument_list
  : type {
    $$ = new TemplateArgumentList();
    $$->push_back($1);
  }
  | template_argument_list type {
    $$ = $1;
    $$->push_back($2);
  }
  ;

/* JsonNamespace */
json_namespace
  : NAMESPACE ID '{' ns_member_seq '}' {
    $$ = $4;
    $$->name = $2;
  }
  ;

/* JsonNamespace */
ns_member_seq
  : json_variable {
    $$ = new JsonNamespace;
  }
  | ns_member_seq json_variable {
    $$ = $1;
    $1->members.push_back($2);
  }
  ;

/* JsonType */
json_type
  : TYPE ID '=' type {
    $$ = new JsonType;
    $$->name = $2;
    Type * t = $4;
    if (boost::typeindex::type_id<StringRef*>() == t->type.type()) {
      StringRef * name = boost::get<StringRef*>(t->type);
      t = getNamedType(name);
      if (nullptr == t) {
        llvm::errs() << "Error when  declaring type alias: unknow type " << *name << "\naborting\n";
        abort();
      }
      delete name;
      delete $4;
    }
    $$->type = t;
    addNamedType($$->name, t);
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
    $$->name = $1;
    $$->type = $3;
    $$->value = $5;
  }
  | ID '=' value {
    $$ = new JsonVariable;
    $$->name = $1;
    $$->type = nullptr;
    $$->value = $3;
  }
  ;

/* json_variable eat the 'export' them self */
/* JsonExport */
json_export
  : EXPORT json_dict {
    $$ = new JsonExport{$2};
  }
  | EXPORT json_template {
    $$ = new JsonExport{$2};
  }
  | EXPORT json_namespace {
    $$ = new JsonExport{$2};
  }
  | EXPORT json_type {
    $$ = new JsonExport{$2};
  }
  ;

%%
#include "main.cpp"
