#ifndef PRE_DUMP_H
#define PRE_DUMP_H
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
using llvm::raw_fd_ostream;

void pre_dump();
void pre_dump(JsonDict *, const Twine & prefix);
void pre_dump(JsonTemplate *, const Twine & prefix);
void pre_dump(JsonNamespace *, const Twine & prefix);
void pre_dump(JsonType *, const Twine & prefix);
void pre_dump(JsonVariable *, const Twine & prefix);
void pre_dump(JsonExport *, const Twine & prefix);
void print_value(Value * v, raw_ostream * outf_);
#endif // PRE_DUMP_H
