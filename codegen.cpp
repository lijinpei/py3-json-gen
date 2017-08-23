#include "codegen.h"

Type *resolveType(Type *t, list<StringRef *> & base_stack) {
  assert(boost::typeindex::type_id<StringRef*>() == t->type.type());
  StringRef *name = boost::get<StringRef*>(t->type);
  for (auto n : base_stack) {
    if (*name == *n) {
      llvm::errs() << "Error in resolveType(): circular base dependency\naborting\n";
      abort();
    }
  }
}

/*
Type* generateDict(StringRef *type, list<StringRef *> & base_stack) {
  if (auto ret = getGeneratedType(type)) {
    return ret;
  }
  auto ty = getType(type);
  if (ty->type.type() != boost::typeindex::type_id<JsonDict*>()) {
    llvm::errs() << "Error in generateDict(): type " << *type << "is not a dict.\naborting\n";
    abort();
  }
  auto dict = boost::get<JsonDict*>(ty->type);
  // currently we only suppoer JsonDict as base
  if (dict->base) {
    auto base = boost::get<Type*>(d->base->type);
    assert(base);
    auto name = boost::get<StringRef*>(base->type);
    if (name && !getGeneratedType(name)) {
      for (const auto & b1 : base_stack) {
        if (*b1 == *n) {
          llvm::errs() << "circular base detected at " << *n << '\n';
          abort();
        }
      }
      base_stack.push_front(d->name);
      generateDict(n, base_stack);
      base_stack.pop_front();
    }
  }
  for (const auto & m : d->body->members) {
    for (const auto & d : m->type_or_value->types) {
      if ((d->type).type() == boost::typeindex::type_id<StringRef*>()) {
        auto s = boost::get<StringRef*>(d->type);
        auto ret = getGeneratedType(s);
        if (ret) {
          ->type = ret;
        } else {
          list<StringRef *> tmp_stack;
          d->type = generateType(s, tmp_stack);
        }
      }
    }
  }
  pre_dump(d, "");
  return addGeneratedType(*type, t);
}

void codegen(DictBody * b) {
}

void codegen(JsonDict *d) {
  list<StringRef*> base_stack;
  generateType(d->name, base_stack);
}

void codegen(JsonTemplate *t) {
  // template without instantiation doesn't generate real code
  return;
}

void codegen(JsonNamespace *n) {
}

void codegen(JsonType * t) {
}

void codegen(JsonVariable * v) {
}

void codegen(JsonExport * e) {
}

class CodegenVisitor : public boost::static_visitor<>{
  public:
    template <typename T>
    void operator()(T *&operand) const {
      codegen(operand);
    }
};

void codegen() {
  for (auto & item : json_root->json_items) {
    boost::apply_visitor(CodegenVisitor(), *item);
  }
}
*/
