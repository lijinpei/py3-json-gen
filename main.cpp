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
  //pre_dump();
  std::error_code ec;
  raw_fd_ostream outf("test.py", ec,  llvm::sys::fs::OpenFlags::F_None);
  //raw_fd_ostream outf(1, false);
  codegen(&outf);
}

StringMap<Value*> variableTable;
StringMap<Type*> namedTypes;
StringMap<JsonTemplate*> templateTable;
list<string> stringTable;
Json * json_root;
list<Type*> typeTable;


#include "pre_dump.cpp"
#include "codegen.cpp"
