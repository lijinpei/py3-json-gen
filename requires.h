#include "boost/variant/get.hpp"
#include "boost/variant/variant.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <forward_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

void yyerror(char const *s);
extern "C" int yylex(void);
extern int yylineno;

using std::vector;
using std::string;
using std::forward_list;
using llvm::StringRef;
using llvm::StringMap;
using llvm::raw_ostream;
using boost::variant;
using boost::get;
using std::make_pair;

/* All the ast nodes must be move-able (perhaps except the root node) */
/* p means move of this class is implemented through shared_ptr (as directly
 * move the class would be costly) */
/* All the ast nodes that needs forward declaration can't be 'using, typedef' */
/* All the string literals and names appears as StringRef to StringTable */
/* All named types (including templates) appears as shared_ptr to NamedTypeTable
 */
/* All ast node class appears directly in other ast node class (directly means
 * not throught pointer)
 * as all the ast node are cheap to move.
 * Except when the node needs to be merged with other same node */
/* YYSTYPE is a variant of all the move-able ast nodes */

/* To define Json */
struct JsonDict;
struct JsonTemplate;
struct JsonNamespace;
struct JsonType;
struct JsonVariable;
struct JsonExport;

using JsonItem = variant<JsonDict *, JsonTemplate *, JsonNamespace *,
                         JsonType *, JsonVariable *, JsonExport *>;

struct Json {
  forward_list<JsonItem *> json_items;
};

/* To define JsonDict */
struct Type;
struct Value;

using Types = forward_list<Type *>;
using Values = forward_list<Value *>;

struct TypeOrValue {
  Types types;
  Values values;
};

struct DictMember {
  StringRef name;
  TypeOrValue *type_or_value;
  bool optional;
  DictMember(StringRef name, TypeOrValue *ty, bool opt)
      : name(name), type_or_value(ty), optional(opt) {}
};

struct DictBody {
  forward_list<DictMember *> members;
};

struct TemplateID;
struct NamedType {
  variant<TemplateID *, StringRef> type;
  NamedType(TemplateID *tid = nullptr) : type(tid) {}
};

/* p */
struct JsonDict {
  StringRef name;
  NamedType *base;
  DictBody *body;
  JsonDict(StringRef name, NamedType *base, DictBody *body)
      : name(name), base(base), body(body) {}
};

/* To define JsonTemplate */

using TemplateParameterList = forward_list<StringRef *>;
/* p */
struct JsonTemplate {
  StringRef *name;
  TemplateParameterList *temp_par_list;
  DictBody *body;
};

using TemplateArgumentList = forward_list<Type *>;

struct TemplateID {
  JsonTemplate *temp;
  StringRef name;
  TemplateArgumentList *temp_arg_list;
  DictBody *body; // body after template instantiation
  TemplateID(TemplateArgumentList *tal) : temp_arg_list(tal) {}
};

/* To define Type */
enum BuiltinTypes { BT_Boolean, BT_Number, BT_String, BT_JsonNull, BT_Any };

struct JsonList {
  Type *type;
};

struct Type {
  /* if type stores a StringRef, it means a dict type name not lookup-ed yet
   * template_id's name and argument list are stored in TemplateID before
   * instantiation */
  variant<BuiltinTypes, JsonDict *, TemplateID *, JsonList *, DictBody *,
          StringRef>
      type;
};

/* To define Value */
enum SpecialValue { SV_Any, SV_JsonNull };

struct StringLiteral {
  StringRef value;
};

struct BooleanLiteral {
  bool value;
};

struct NumberLiteral {
  int value;
};

struct StrValuePair {
  StringRef str;
  Value *val;
};

struct DictLiteral {
  forward_list<StrValuePair *> *members;
  DictLiteral() : members(new forward_list<StrValuePair *>) {}
};

struct ListLiteral {
  forward_list<Value *> *values;
  ListLiteral() : values(new forward_list<Value *>) {}
};

/* We won't record which variable the value is form */
struct Value {
  variant<SpecialValue, StringRef *, bool, int, DictLiteral *, ListLiteral *>
      value;
};

/* To define JsonVariable and JsonNamepace */
/* p */
struct JsonVariable {
  StringRef name;
  Value *value;
  Type *type;
  bool constant = false;
  bool exported = false;
};

struct JsonType {
  StringRef name;
  Type *type;
};

struct JsonNamespace {
  StringRef name;
  forward_list<JsonVariable *> members;
};

struct JsonExport {
  variant<JsonDict *, JsonTemplate *, JsonNamespace *, JsonType *> exported;
};

// using my_yystype = variant<JsonItem, DictMember, DictBody, JsonDict,
// JsonTemplate, TemplateParameterList, TemplateArgumentList, TemplateID,
// NamedType, JsonList, Type, StringLiteral, BooleanLiteral, NumberLiteral,
// StrValuePair, DictLiteral, ListLiteral, Value, JsonVariable, JsonNamespace,
// JsonExport>;
// using my_yystype = variant<JsonItem, Types, Values, TypeOrValue, DictMember,
// DictBody, JsonDict, JsonTemplate, TemplateParameterList,
// TemplateArgumentList, TemplateID, NamedType, JsonList, Type, StringLiteral,
// BooleanLiteral, NumberLiteral, StrValuePair, DictLiteral, ListLiteral, Value,
// JsonVariable, JsonNamespace, JsonExport>;
union my_yystype {
  int number;
  bool boolean;
  StringRef *p_str;
  JsonItem *json_item;
  JsonDict *json_dict;
  JsonTemplate *json_template;
  JsonNamespace *json_namespace;
  JsonType *json_type;
  JsonVariable *json_variable;
  JsonExport *json_export;
  DictBody *dict_body;
  DictMember *dict_member;
  TemplateParameterList *template_parameter_list;
  TemplateArgumentList *template_argument_list;
  TemplateID *template_id;
  NamedType *named_type;
  JsonList *json_list;
  Type *type;
  Value *value;
  // Literal return Value *
  StringLiteral *string_literal;
  BooleanLiteral *boolean_literal;
  NumberLiteral *number_literal;
  DictLiteral *dict_literal;
  ListLiteral *list_literal;
  StrValuePair *str_value_pair;
};

extern StringMap<Value *> VariableTable;
extern StringMap<JsonDict *> DictTable;
extern StringMap<JsonTemplate *> TemplateTable;
extern forward_list<string> StringTable;
Value *addVariableDef(StringRef name, Value *v);
Value *getVariableValue(StringRef name);
JsonDict *addDictDef(StringRef name, JsonDict *dict);
JsonDict *getDictDef(StringRef name);
JsonTemplate *addTemplateDef(StringRef name, JsonTemplate *temp);
JsonTemplate *getTemplateDef(StringRef Name);
StringRef *addToStringTable();
