#ifndef REQUIRES_H
#define REQUIRES_H

#include "boost/variant/get.hpp"
#include "boost/variant/variant.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
#include <boost/core/demangle.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

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
  StringRef *name;
  TypeOrValue *type_or_value;
  bool optional;
  DictMember(StringRef *name, TypeOrValue *ty, bool opt)
      : name(name), type_or_value(ty), optional(opt) {}
};

struct DictBody {
  list<DictMember *> members;
};


/* We only support concrete class as base.
 * ie. we don't support template to extend template
 * Currently, we conly support single inheritage.
 */
struct JsonDict {
  StringRef *name;
  Type *base;
  DictBody *body;
  JsonDict(StringRef *name, Type *base, DictBody *body)
      : name(name), base(base), body(body) {}
};

/* To define JsonTemplate */

using TemplateParameterList = list<StringRef *>;
/* p */
struct JsonTemplate {
  StringRef *name;
  TemplateParameterList *temp_par_list;
  DictBody *body;
  int number;
};

using TemplateArgumentList = list<Type *>;

struct TemplateParameter {
  int pos;
};

/* Before name lookup, name store a template name which should be resolved */
inline JsonTemplate * getTemplateDef(StringRef * name);
struct TemplateID;

/* To define Type */
enum BuiltinTypes { BT_Boolean, BT_Number, BT_String, BT_JsonNull, BT_Any };

struct JsonList {
  Type *type;
};

struct Type {
  /* StringRef* is only used in template */
  variant<BuiltinTypes, JsonDict *, TemplateID *, JsonList *, DictBody *,
          StringRef *, TemplateParameter>
      type;
};

struct TemplateID {
  StringRef *name;
  TemplateArgumentList *temp_arg_list;
  JsonTemplate *temp;
  DictBody *instantiation; // the Dict Body this template instantiate
  int number = 0;

  // This function bind template parameters to template arguments and generate
  // the dictbody
  void Instantiate() {
    temp = getTemplateDef(name);
    if (nullptr == temp) {
      llvm::errs() << "Error in TemplateID::instantiateTemplate(): unable to instantiate template " << *name << "unknown template name\naaborting\n";
      abort();
    }
    if (temp_arg_list->size() != temp->temp_par_list->size()) {
      llvm::errs() << "Error in TemplateID::instantiateTemplate(): template parameter list's length and template argument list's length not match\naborting\n";
      abort();
    }

    instantiation = new DictBody;
    for (const auto & mber : temp->body->members) {
      TypeOrValue * tv = new TypeOrValue{};
      tv->values = mber->type_or_value->values;
      for (const auto & t : mber->type_or_value->types) {
        if (boost::typeindex::type_id<TemplateParameter>() == t->type.type()) {
          int pos = boost::get<TemplateParameter>(t->type).pos;
          auto itor = temp_arg_list->begin();
          while (pos--) {
            itor = std::next(itor);
          }
          tv->types.push_back(*itor);
        }
      }
      DictMember *mber1 = new DictMember(mber->name, tv, mber->optional);
      instantiation->members.push_back(mber1);
    }
  }

  TemplateID(StringRef *name, TemplateArgumentList *tal)
      : name(name), temp_arg_list(tal), temp(nullptr), instantiation(nullptr) {
        Instantiate();
      }
};

/* To define Value */
enum SpecialValue { SV_Any, SV_JsonNull };

struct StringLiteral {
  StringRef *value;
};

struct BooleanLiteral {
  bool value;
};

struct NumberLiteral {
  int value;
};

struct StrValuePair {
  StringRef *str;
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
  StringRef *name;
  IDRef(StringRef *name) : name(name) {}
};

struct Value {
  variant<SpecialValue, IDRef *, StringRef *, bool, int, DictLiteral *,
          ListLiteral *>
      value;
};

/* To define JsonVariable and JsonNamespace */
/* p */
struct JsonVariable {
  StringRef *name;
  Value *value;
  Type *type;
  bool constant = false;
  bool exported = false;
};

struct JsonType {
  StringRef *name;
  StringRef *alias_name;
  Type *type;
};

struct JsonNamespace {
  StringRef *name;
  list<JsonVariable *> members;
};

struct JsonExport {
  variant<JsonDict *, JsonTemplate *, JsonNamespace *, JsonType *> exported;
};

extern Json *json_root;
extern StringMap<Value *> variableTable;
extern list<string> stringTable;
extern list<Type*> typeTable;
extern StringMap<JsonTemplate *> templateTable;
// all the named Dict and type alias are added to this 
extern StringMap<Type *> namedTypes;

inline Value * addVariableDef(StringRef * name, Value * v) {
  bool ret = variableTable.insert(make_pair(*name, v)).second;
  if (!ret) {
    llvm::errs() << "error in addVariableDef(): variable already exists\n";
    abort();
  }
  return v;
}

inline Value* deepCopyValue(Value *v) {
  return new Value{v->value};
}

inline Value * getVariableValue(StringRef * name) {
  auto iter = variableTable.find(*name);
  if (iter == variableTable.end()) {
    llvm::errs() << "error in getVariableValue(): variable not found\n";
  }
  return deepCopyValue(iter->getValue());
}

inline Type *addType(Type *type) {
  typeTable.push_back(type);
  return type;
}

inline Type *addNamedType(StringRef *name, Type *type) {
  bool ret = namedTypes.insert(make_pair(*name, type)).second;
  if (!ret) {
    llvm::errs() << "Error in addType(): dict already exists\n";
    abort();
  }
  return type;
}

inline Type *getNamedType(StringRef *name) {
  auto res = namedTypes.find(*name);
  if (res != namedTypes.end()) {
    return res->getValue();
  }
  return nullptr;
}

inline JsonTemplate * addTemplateDef(StringRef * name, JsonTemplate * temp) {
  bool ret = templateTable.insert(make_pair(*name, temp)).second;
  if (!ret) {
    llvm::errs() << "error in addTemplateDef(): template already exists\n";
    abort();
  }
  return temp;
}

inline JsonTemplate * getTemplateDef(StringRef * name) {
  auto iter = templateTable.find(*name);
  if (iter == templateTable.end()) {
    llvm::errs() << "error in getTemplateDef(): template not found\n";
    llvm::errs() << "line number: " << yylineno << '\n';
    llvm::errs() << "template name : " << *name << '\n';
    abort();
  }
  return iter->getValue();
}

inline Type* checkTypeNameExists(StringRef * name) {
  auto res = namedTypes.find(*name);
  if (res == namedTypes.end()) {
    llvm::errs() << "unknown type: " << *name << "\nline number: ";
    llvm::errs() << yylineno << "\naborting\n";
    abort();
  }
  return res->getValue();
}
inline void resolveType(Type * & t) {
  JsonList * list = nullptr;
  Type * type = t;
  if (boost::typeindex::type_id<JsonList*>() == t->type.type()) {
    list = boost::get<JsonList*>(t->type);
    type = list->type;
  }
  if (boost::typeindex::type_id<StringRef*>() == type->type.type()) {
    StringRef *name = boost::get<StringRef*>(type->type);
    Type * t1 = getNamedType(name);
    if (t1 == nullptr) {
      llvm::errs() << "Error in resolveType: unable to resolve type of dict memeber " << *name << "\naborting\n";
      abort();
    }
    delete name;
    if (list) {
      delete list->type;
      list->type = t1;
    } else {
      delete t;
      t = t1;
    }
  }
}

inline void resolveTemplateArgumentList(TemplateArgumentList * tal) {
  for (auto & mber : *tal) {
    resolveType(mber);
  }
}

inline void resolveDictBody(DictBody * b) {
  for (auto & mber : b->members) {
    Types & types = mber->type_or_value->types;
    for (auto & t : types) {
      resolveType(t);
    }
  }
}

inline void resolveDictBody(DictBody * b, TemplateParameterList * p) {
  for (auto & mber : b->members) {
    Types & types = mber->type_or_value->types;
    for (auto & t : types) {
      if (boost::typeindex::type_id<StringRef*>() == t->type.type()) {
        StringRef *name = boost::get<StringRef*>(t->type);
        bool flag = false;
        int pos = 0;
        for (const auto & par : *p) {
          if (*par == *name) {
            flag = true;
            break;
          }
          ++pos;
        }
        if (flag) {
          delete name;
          t->type = TemplateParameter{pos};
          continue;
        }
        Type * t1 = getNamedType(name);
        if (t1 == nullptr) {
          llvm::errs() << "Error in resolveDictBody(): unable to resolve type of dict memeber " << *name << "\naborting\n";
          abort();
        }
        delete name;
        delete t;
        t = t1;
      }
    }
  }
}

StringRef * addToStringTable();

/* To simplify implementation, we have limitted support on template: template-id
 * can only be used as variable type
 * that is to say, base class and template parameter must be non-template type
 */
// TODO: add more template functionality

#include "pre_dump.h"
#include "codegen.h"
#endif // REQUIRES_H
