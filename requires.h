#ifndef REQUIRES_H
#define REQUIRES_H

#include "boost/variant/get.hpp"
#include "boost/variant/variant.hpp"
#include <boost/core/demangle.hpp>
#include <functional>
#include <typeinfo>
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <list>

void yyerror(char const *s);
extern "C" int yylex(void);
extern int yylineno;

using std::vector;
using std::string;
using std::list;
using llvm::Twine;
using llvm::StringRef;
using llvm::StringMap;
using llvm::raw_ostream;
using boost::variant;
using boost::get;
using std::make_pair;

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
  list<JsonItem *> json_items;
};

/* To define JsonDict */
/* Type includes a dict name. After name lookup, the dict name will be replaced
 * with the corresponding dict type. */
struct Type;
struct Value;

using Types = list<Type *>;
using Values = list<Value *>;

/* type and value appeared in dict member can be distinguished by syntax. */
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
  list<DictMember *> members;
};

struct TemplateID;
struct NamedType {
  variant<TemplateID *, Type*> type;
  NamedType(std::nullptr_t np) : type() {}
  NamedType(TemplateID * tid) : type(tid) {}
  NamedType(Type * t) : type(t) {}
};

/* We only support concrete class as base.
 * ie. we don't support template to extend template
 * Currently, we conly support single inheritage.
 */
struct JsonDict {
  StringRef name;
  NamedType *base;
  DictBody *body;
  JsonDict(StringRef name, NamedType *base, DictBody *body)
      : name(name), base(base), body(body) {}
};

/* To define JsonTemplate */

using TemplateParameterList = list<StringRef *>;
/* p */
struct JsonTemplate {
  StringRef name;
  TemplateParameterList *temp_par_list;
  DictBody *body;
};

using TemplateArgumentList = list<Type *>;

/* Before name lookup, name store a template name which should be resolved */
struct TemplateID {
  StringRef name;
  TemplateArgumentList *temp_arg_list;
  JsonTemplate *temp;
  DictBody *body; // body after template instantiation
  TemplateID(StringRef name, TemplateArgumentList *tal) : name(name), temp_arg_list(tal), temp(nullptr), body(nullptr) {}
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
  list<StrValuePair *> *members;
  DictLiteral() : members(new list<StrValuePair *>) {}
};

struct ListLiteral {
  list<Value *> *values;
  ListLiteral() : values(new list<Value *>) {}
};

struct IDRef {
  StringRef name;
  IDRef(StringRef name) : name(name) {}
};

struct Value {
  variant<SpecialValue, IDRef *, StringRef *, bool, int, DictLiteral *,
          ListLiteral *>
      value;
};

/* To define JsonVariable and JsonNamespace */
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
  list<JsonVariable *> members;
};

struct JsonExport {
  variant<JsonDict *, JsonTemplate *, JsonNamespace *, JsonType *> exported;
};

extern Json * json_root;
extern StringMap<Value *> VariableTable;
extern StringMap<JsonDict *> DictTable;
extern StringMap<JsonTemplate *> TemplateTable;
extern list<string> StringTable;
Value *addVariableDef(StringRef name, Value *v);
Value *getVariableValue(StringRef name);
JsonDict *addDictDef(StringRef name, JsonDict *dict);
JsonDict *getDictDef(StringRef name);
JsonTemplate *addTemplateDef(StringRef name, JsonTemplate *temp);
JsonTemplate *getTemplateDef(StringRef Name);
StringRef *addToStringTable();

#include "pre_dump.h"
#endif // REQUIRES_H
