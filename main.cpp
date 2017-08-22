namespace boost {
void throw_exception( std::exception const & e ) {
  llvm::errs() << "user provided boost::throw_exception() called.\naborting...\n";
  abort();
}
}

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
  pre_dump();
}

StringMap<Value*> VariableTable;
StringMap<JsonDict*> DictTable;
StringMap<JsonTemplate*> TemplateTable;
list<string> StringTable;
Json * json_root;

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

class PreDumpVisitor : public boost::static_visitor<>{
  public:
    template <typename T>
    void operator()(T *&operand, const Twine & prefix) const {
      llvm::outs() <<  prefix << boost::core::demangle(typeid(T).name())
                   << ' ' << operand->name << '\n';
    }
    void operator()(JsonExport * & operand, const Twine & prefix) const {
      char str[] = "export: ";
      llvm::outs() << prefix << str << '\n';
      string str1(sizeof(str), ' ');
      auto bound_visitor = std::bind(PreDumpVisitor(), std::placeholders::_1, prefix + str1);
      boost::apply_visitor(bound_visitor, operand->exported);
    }
};

void pre_dump() {
  auto bound_visitor = std::bind(PreDumpVisitor(), std::placeholders::_1, "");
  for (auto & item : json_root->json_items) {
    boost::apply_visitor(bound_visitor, *item);
  }
}

