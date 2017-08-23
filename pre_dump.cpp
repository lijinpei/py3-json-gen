#include "pre_dump.h"

class PreDumpVisitor : public boost::static_visitor<>{
  public:
    template <typename T>
    void operator()(T *&operand, const Twine & prefix) const {
      pre_dump(operand, prefix);
    }
};

void pre_dump() {
  auto bound_visitor = std::bind(PreDumpVisitor(), std::placeholders::_1, "");
  for (auto & item : json_root->json_items) {
    boost::apply_visitor(bound_visitor, *item);
  }
}

class TypeNameVisitor : public boost::static_visitor<> {
public:
  // this template will match JsonList* and DictBody*
  template<typename T>
  void operator()(T * t) const {
    llvm::outs() << boost::core::demangle(typeid(T).name());
  }

  // we need this overload to handle NamedType
  void operator()(Type * t) const {
    boost::apply_visitor(TypeNameVisitor(), t->type);
  }

  void operator()(JsonDict * d) const {
    llvm::outs() << "JsonDict " << d->name;
  }

  void operator()(TemplateID * t) const {
    llvm::outs() << "TemplateID " << t->name << '<';
    bool flag = false;
    for (auto & a : *(t->temp_arg_list)) {
      if (flag)
        llvm::outs() << ", ";
      flag = true;
      boost::apply_visitor(TypeNameVisitor(), a->type);
    }
    llvm::outs() << '>';
  }

  void operator()(BuiltinTypes t) const {
    switch (t) {
      case (BT_Boolean):
        llvm::outs() << "boolean";
        break;
      case (BT_Number):
        llvm::outs() << "number";
        break;
      case (BT_String):
        llvm::outs() << "string";
        break;
      case (BT_JsonNull):
        llvm::outs() << "null";
        break;
      case (BT_Any):
        llvm::outs() << "any";
        break;
      default:
        llvm::outs() << "Error in TypeNameVisitor::operator()(BuiltinTypes)\n"
          "unknown enum value\n" "aborting\n";
        abort();
    }
  }

  void operator()(const StringRef &s) const {
    llvm::outs() << s << "(unresolved type)";
  }
};

void print_type(Type * t) {
    boost::apply_visitor(TypeNameVisitor(), t->type);
}

class ValueVisitor : public boost::static_visitor<> {
public:
    void operator()(const SpecialValue & v) const {
      switch (v) {
        case(SV_Any):
          llvm::outs() << "any";
          break;
        case(SV_JsonNull):
          llvm::outs() << "null";
          break;
        default:
          llvm::outs() << "Error in ValueVisitor::ioerator(const SpecialValue &)\n"
            "unknown enum valua\n" "aborting\n";
      }
    }

    void operator()(StringRef * s) const {
      llvm::outs() << *s;
    }

    void operator()(IDRef * i) const {
      llvm::outs() << i->name << "(variable ref)";
    }

    void operator()(bool b) const {
      llvm::outs() << (b ? "true" : "false");
    }

    void operator()(int i) const {
      llvm::outs() << i;
    }
    
    void operator()(DictLiteral *d) const {
      llvm::outs() << '{';
      bool flag = false;
      for (const auto & m : *(d->members)) {
        if (flag) {
          llvm::outs() << ",\n";
        }
        llvm::outs() << m->str << ':';
        boost::apply_visitor(ValueVisitor(), m->val->value);
      }
      llvm::outs() << "\n}";
    }

    void operator()(ListLiteral *l) const {
      llvm::outs() << '[';
      bool flag = false;
      for (const auto & v : *(l->values)) {
        if (flag) {
          llvm::outs() << ", ";
        }
        flag = true;
        boost::apply_visitor(ValueVisitor(), v->value);
      }
      llvm::outs() << ']';
    }
};

void print_value(Value * v) {
  boost::apply_visitor(ValueVisitor(), v->value);
}

void pre_dump(DictBody * b, const Twine & prefix) {
  auto flag = false;
  for (auto & m : b->members) {
    if (flag) {
      llvm::outs() << ",\n";
    }
    flag = true;
    llvm::outs() << prefix << m->name;
    auto & t = m->type_or_value->types;
    auto & v = m->type_or_value->values;
    if (t.size()) {
      llvm::outs() << ": ";
      bool flag = false;
      for (auto & t1 : t) {
        if (flag) {
          llvm::outs() << " | ";
        }
        flag = true;
        boost::apply_visitor(TypeNameVisitor(), t1->type);
      }
    }
    if (v.size()) {
      llvm::outs() << " = ";
      bool flag = false;
      for (auto v1 : v) {
        if (flag) {
          llvm::outs() << " | ";
        }
        flag = true;
        boost::apply_visitor(ValueVisitor(), v1->value);
      }
    }
  }
  llvm::outs() << '\n';
}

void pre_dump(JsonDict * dict, const Twine & prefix) {
  llvm::outs() << prefix << "JsonDict: " << dict->name;
  if (dict->base) {
    llvm::outs() << " extends ";
    boost::apply_visitor(TypeNameVisitor(), dict->base->type);
  }
  llvm::outs() << '\n';
  pre_dump(dict->body, prefix + "    ");
}

void pre_dump(JsonTemplate * t, const Twine & prefix) {
  llvm::outs() << prefix << "template: " << t->name << '<';
  bool flag = false;
  for (const auto v : *(t->temp_par_list)) {
    if (flag) {
      llvm::outs() << ", ";
    }
    flag = true;
    llvm::outs() << *v;
  }
  llvm::outs() << ">\n";
  pre_dump(t->body, prefix + "    ");
}

void pre_dump(JsonNamespace * n, const Twine & prefix) {
  llvm::outs() << prefix << "namespace: " << n->name << '\n';
  const Twine pre1 = prefix + "    ";
  for (const auto & m : n->members) {
    pre_dump(m, pre1);
  }
}
void pre_dump(JsonType *t, const Twine & prefix) {
  llvm::outs() << prefix << "type: " << t->name << ':';
  print_type(t->type);
  llvm::outs() << '\n';
}

void pre_dump(JsonVariable * v, const Twine & prefix) {
  llvm::outs() << prefix << "variable: " << v->name;
  if (v->type) {
    llvm::outs() << " : ";
    print_type(v->type);
  }
  if (v->value) {
    llvm::outs() << " = ";
    print_value(v->value);
  }
  llvm::outs() << '\n';
}

void pre_dump(JsonExport * e, const Twine & prefix) {
  llvm::outs() << prefix << "export\n";
  auto bound_visitor = std::bind(PreDumpVisitor(), std::placeholders::_1, prefix);
  boost::apply_visitor(bound_visitor, e->exported);
}
