#ifndef CODEGEN_H
#define CODEGEN_H
#include "boost/type_index.hpp"
#include "boost/variant.hpp"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include <system_error>

using llvm::raw_fd_ostream;

void print_value(Value * v, raw_ostream * outf_);
// all the named type that has been generated are added to this
extern StringMap<Type*> generatedTypes;

inline Type * addGeneratedType(StringRef *name, Type * type) {
  bool ret = generatedTypes.insert(make_pair(*name, type)).second;
  if (!ret) {
    llvm::errs() << "Error in addGeneratedType(): dict already exists\n";
    abort();
  }
  return type;
}

inline Type *getGeneratedType(StringRef *name) {
  auto res = generatedTypes.find(*name);
  if (res != generatedTypes.end()) {
    return res->getValue();
  }
  return nullptr;
}

/* two name may refer to the same type using type alias
 * they both need to be added to namedTypes, and both need to own that pointer
 * added
 * so we need to do a deep-copy before add a type alias to namedTypes
 */
inline Type *deepCopy(Type *t) {
  return new Type{t->type};
}

void codegen(JsonDict *d, raw_ostream *outf_, bool checkTempInst = true);
void codegen(JsonTemplate *t, raw_ostream * outf_);
void codegen(JsonNamespace *n, raw_ostream * outf_);
void codegen(JsonType * t, raw_ostream * outf_);
void codegen(JsonVariable * v, raw_ostream * outf_);
void codegen(JsonExport * e, raw_ostream * outf_);
void codegen(Type * t, raw_ostream * outf_);
void codegen(Types * ts, raw_ostream * outf_);
void codegen(raw_ostream * outf_);

void checkTemplateInstantiation(Type *t, raw_ostream *outf_);
void checkTemplateInstantiation(Types *ts, raw_ostream *outf_);
void checkTemplateInstantiation(DictBody *b, raw_ostream *outf_);
void checkTemplateInstantiation(JsonDict * d, raw_ostream *outf_);
#endif // CODEGEN_H
