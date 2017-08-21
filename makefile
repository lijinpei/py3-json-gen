test: lexer json.tab.c json.tab.h
	g++ -g json.tab.c lexer -lfl -o test `llvm-config --cxxflags` `llvm-config --libs core` `llvm-config --ldflags`

json.tab.c json.tab.h: json.y
	bison -d json.y

lexer: lex.yy.c json.tab.h
	g++ -g -c lex.yy.c -lfl -o lexer `llvm-config --cxxflags` `llvm-config --libs core`

lex.yy.c: json.lex
	flex json.lex
