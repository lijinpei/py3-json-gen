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
static int curly_bracket_depth = 0;
#define ID_OR(x) {\
  if (!curly_bracket_depth) {\
    return x;\
  } else {\
    yylval.ID = addToStringTable();\
    return ID;\
  }\
}
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
interface ID_OR(INTERFACE)
extends ID_OR(EXTENDS)
const return CONST;
export return EXPORT;
namespace ID_OR(NAMESPACE)
boolean return BOOLEAN;
string return STRING;
number return NUMBER;
null return JSON_NULL;
any return ANY;
type ID_OR(TYPE)
true |
True { yylval.BOOLEAN_LITERAL = true; return BOOLEAN_LITERAL;}
false |
False { yylval.BOOLEAN_LITERAL = false; return BOOLEAN_LITERAL; }
"{" { ++curly_bracket_depth; return '{'; }
"}" { --curly_bracket_depth; return '}'; }
: return ':';
"|" return '|';
"<" return '<';
">" return '>';
= return '=';
"[" return '[';
"]" return ']';
{identifier} { yylval.ID = addToStringTable(); return ID;}
\? return '?';
{string_literal1} |
{string_literal2} { yylval.STRING_LITERAL = addToStringTable(); return STRING_LITERAL;}
{number_literal} { yylval.NUMBER_LITERAL = atoi(yytext); return NUMBER_LITERAL;}
{blank}
%%

void yyerror (char const *s) {
  fprintf (stderr, "line: %d\n%s\n", yylineno, s);
}


StringRef * addToStringTable() {
    stringTable.push_front(string(yytext));
    return new StringRef(stringTable.front());
}

