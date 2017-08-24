#include "codegen.h"

class CodeGenVisitor : public boost::static_visitor<> {
public:
  template <typename T>
  void operator()(T *operand, raw_ostream *outf_) const {
    codegen(operand, outf_);
  }
};

void codegen(Types *ts, raw_ostream *outf_);

class CodeGenTypeNameVisitor : public boost::static_visitor<> {
public:
  void operator()(BuiltinTypes b, raw_ostream *outf_) const {
    raw_ostream &outf = *outf_;
    switch (b) {
    case (BT_Any):
      outf << "any";
      break;
    case (BT_Number):
      //outf << "number";
      outf << "int";
      break;
    case (BT_String):
      //outf << "string";
      outf << "str";
      break;
    case (BT_Boolean):
      outf << "bool";
      break;
    case (BT_JsonNull):
      //outf << "null";
      outf << "None";
      break;
    default:
      llvm::errs() << "Error in "
                      "CodeGenTypeNameVisitor::operator()(BuiltinTypes b, "
                      "...): unknow enum\naborting\n";
      abort();
    }
  }

  void operator()(JsonDict *d, raw_ostream *outf_) const {
    raw_ostream &outf = *outf_;
    outf << *d->name;
  }

  void operator()(TemplateID *tid, raw_ostream *outf_) const {
    raw_ostream &outf = *outf_;
    assert(tid->number);
    outf << *tid->name << tid->number;
    /*
    outf << *tid->name << '<';
    auto flag = false;
    auto type_name_visitor =
        std::bind(CodeGenTypeNameVisitor(), std::placeholders::_1, outf_);
    for (const auto &arg : *tid->temp_arg_list) {
      if (flag) {
        outf << ", ";
      }
      flag = true;
      boost::apply_visitor(type_name_visitor, arg->type);
    }
    outf << '>';
    */
  }

  void operator()(JsonList *l, raw_ostream *outf_) const {
    raw_ostream &outf = *outf_;
    outf << '[';
    codegen(l->type, outf_);
    outf << ']';
  }

  void operator()(DictBody *db, raw_ostream *outf_) const {
    raw_ostream &outf = *outf_;
    outf << '{';
    bool flag = false;
    for (const auto &mber : db->members) {
      if (flag) {
        outf << ", ";
      }
      flag = true;
      outf << '"';
      outf << *mber->name << "\": ";
      codegen(&(mber->type_or_value->types), outf_);
    }
    outf << '}';
  }

  void operator()(StringRef *s, raw_ostream *) const {
    llvm::errs() << "Error in CodeGenTypeNameVisitor::operator()(StringRef * "
                    "s, ...): this funcntion should not be called\naborting\n";
    abort();
  }

  void operator()(TemplateParameter tp, raw_ostream *) const {
    llvm::errs() << "Error in "
                    "CodeGenTypeNameVisitor::operator()(TempalteParameter tp, "
                    "...): this funcntion should not be called\naborting\n";
    abort();
  }
};

void codegen(Type * t, raw_ostream * outf_) {
  auto type_name_visitor =
      std::bind(CodeGenTypeNameVisitor(), std::placeholders::_1, outf_);
    boost::apply_visitor(type_name_visitor, t->type);
}

void codegen(Types *ts, raw_ostream *outf_) {
  raw_ostream &outf = *outf_;
  outf << '[';
  bool flag = false;
  auto type_name_visitor =
      std::bind(CodeGenTypeNameVisitor(), std::placeholders::_1, outf_);
  for (const auto &t : *ts) {
    if (flag) {
      outf << ", ";
    }
    flag = true;
    boost::apply_visitor(type_name_visitor, t->type);
  }
  outf << ']';
}

void codegen(JsonDict *d, raw_ostream *outf_, bool checkTempInst) {
  if (checkTempInst) {
    checkTemplateInstantiation(d, outf_);
  }
  raw_ostream &outf = *outf_;
  Type *base = d->base;
  StringRef *base_name;
  if (base) {
    assert(boost::typeindex::type_index<JsonDict *>() == base->type.type());
    base_name = boost::get<JsonDict *>(base->type)->name;
  }
  outf << "class " << *d->name << '(' << (base ? *base_name : "Json") << "):\n";
  // Generate member type
  StringRef indent("    ");
  for (const auto &m : d->body->members) {
    outf << indent << *m->name << " = ";
    codegen(&m->type_or_value->types, outf_);
    outf << '\n';
  }
  outf << indent << "def __init__(self";
  for (const auto &m : d->body->members) {
    outf << ", ";
    outf << *m->name;
  }
  if (base) {
    outf << ", base";
  }
  outf << "):\n";
  if (base) {
    outf << indent << indent << "self.__dict__ = base.__dict__\n";
  } else {
    outf << indent << indent << "Json.__init__(self)\n";
  }
  for (const auto & m : d->body->members) {
    outf << indent << indent << "self." << *m->name << " = " << *m->name << '\n';
  }
  outf << '\n';
}

void codegen(JsonTemplate *t, raw_ostream *outf_) { return; }

void codegen(JsonNamespace *n, raw_ostream *outf_) {
  raw_ostream &outf = *outf_;
  for (auto & mber : n->members) {
    outf << *n->name;
    codegen(mber, outf_);
  }
  outf << '\n';
}

void codegen(JsonType *t, raw_ostream *outf_) {
  return;
  raw_ostream &outf = *outf_;
  outf << *t->name << " = ";
  if (nullptr != t->alias_name) {
    outf << *t->alias_name;
  } else {
    codegen(t->type, outf_);
  }
  outf << '\n';
}

void codegen(JsonVariable *v, raw_ostream *outf_) {
  raw_ostream &outf = *outf_;
  outf << *v->name << " = ";
  print_value(v->value, outf_);
  outf << '\n';
}

void codegen(JsonExport *e, raw_ostream *outf_) {
  // raw_ostream &outf = *outf_;
  // outf << "export ";
  auto bound_visitor =
      std::bind(CodeGenVisitor(), std::placeholders::_1, outf_);
  boost::apply_visitor(bound_visitor, e->exported);
}

// instantiate a template
void codegen(TemplateID *tid, raw_ostream *outf_) {
  checkTemplateInstantiation(tid->instantiation, outf_);
  tid->number = ++tid->temp->number;
  string s((*tid->name + std::to_string(tid->number)).str());
  StringRef sref(s);
  JsonDict *d = new JsonDict(&sref, nullptr, tid->instantiation);
  codegen(d, outf_, false);
  delete d;
}

void codegen(raw_ostream *outf_) {
  raw_ostream & outf = *outf_;
  outf <<
"#! /usr/bin/python3\n\
\n\
import json\n\
\n\
class Json:\n\
    json_decoder = json.JSONDecoder()\n\
    json_encoder = json.JSONEncoder()\n\
\n\
    def __init__(self, **kwargs):\n\
        self.__dict__ = kwargs\n\
\n\
    def toJSON(self):\n\
		    return JSON.json_encoder.encode(self.__dict__)\n\
\n\
    @staticmethod\n\
    def fromJSON(str):\n\
		    return JSON(**JSON.json_decoder.decode(str))\n\
\n\
class any:\n\
    pass\n\n";

  auto bound_visitor =
      std::bind(CodeGenVisitor(), std::placeholders::_1, outf_);
  for (auto &item : json_root->json_items) {
    boost::apply_visitor(bound_visitor, *item);
    if (boost::typeindex::type_id<JsonVariable*>() == item->type()) {
      *outf_ << '\n';
    }
  }
}

class CheckTemplateVisitor : public boost::static_visitor<> {
public:
  template <typename T>
  void operator()(const T &, raw_ostream *outf_) const {
    return;
  }
  void operator()(TemplateID *tid, raw_ostream *outf_) const {
    codegen(tid, outf_);
  }
  void operator()(JsonList *l, raw_ostream *outf_) const {
    checkTemplateInstantiation(l->type, outf_);
  }
};

void checkTemplateInstantiation(Type *t, raw_ostream *outf_) {
  auto bound_visitor = std::bind(CheckTemplateVisitor(), std::placeholders::_1, outf_);
  boost::apply_visitor(bound_visitor, t->type);
}

void checkTemplateInstantiation(Types *ts, raw_ostream *outf_) {
  for (auto & type : *ts) {
    checkTemplateInstantiation(type, outf_);
  }
}

void checkTemplateInstantiation(DictBody *b, raw_ostream *outf_) {
  for (auto & mber : b ->members) {
    checkTemplateInstantiation(&mber->type_or_value->types, outf_);
  }
}

void checkTemplateInstantiation(JsonDict * d, raw_ostream *outf_) {
  if (d->base) {
    checkTemplateInstantiation(d->base, outf_);
  }
  checkTemplateInstantiation(d->body, outf_);
}


