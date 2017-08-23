#ifndef CODEGEN_H
#define CODEGEN_H
#include "boost/type_index.hpp"
#include "boost/variant.hpp"

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


/* resolve t(which should have StringRef* as t->type) to a type(non StingRef*),
 * based on the current base stack, return the type resolved.
 */
Type *resolveType(Type *t, list<StringRef *>&);
void codegen();

#endif // CODEGEN_H
