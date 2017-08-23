#ifndef PRE_DUMP_H
#define PRE_DUMP_H
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"

void pre_dump();
void pre_dump(JsonDict *, const Twine & prefix);
void pre_dump(JsonTemplate *, const Twine & prefix);
void pre_dump(JsonNamespace *, const Twine & prefix);
void pre_dump(JsonType *, const Twine & prefix);
void pre_dump(JsonVariable *, const Twine & prefix);
void pre_dump(JsonExport *, const Twine & prefix);
#endif // PRE_DUMP_H
