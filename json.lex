%{
#include "json.tab.h"
#include "cstdio"
#include "cstdlib"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include <cstring>
#include <string>
#include <memory>

static int line_num = 0;

%}

%option yylineno
%x comment1 comment2

identifier [[:alpha:]][[:alnum:]]*
number_literal -?[[:digit:]]+
string_literal1 \"(\\.|[^\\"])*\"
string_literal2 \'(\\.|[^\\'])*\'
blank [,. \t]

%%
"/*" BEGIN(comment1);
<comment1>{
[^*\n]*
"*"+[^*/\n]*
\n ++line_num;
"*"+"/" BEGIN(INITIAL);
}
"//" BEGIN(comment2);
<comment2>{
\n {
  ++line_num;
  BEGIN(INITIAL);
}
.*
}
\n ++line_num;
interface return INTERFACE;
extends return EXTENDS;
const return CONST;
export return EXPORT;
namespace return NAMESPACE;
boolean return BOOLEAN;
string return STRING;
number return NUMBER;
null return JSON_NULL;
any return ANY;
type return TYPE;
true |
True return BOOLEAN_LITERAL;
false |
False return BOOLEAN_LITERAL;
"{" return '{';
"}" return '}';
: return ':';
"|" return '|';
"<" return '<';
">" return '>';
= return '=';
"[" return '[';
"]" return ']';
{identifier} return ID;
\? return '?';
{string_literal1} |
{string_literal2} return STRING_LITERAL;
{number_literal}  return NUMBER_LITERAL;
{blank}
%%

void yyerror (char const *s) {
  fprintf (stderr, "line: %d\n%s\n", yylineno, s);
}
