CXXFLAGS = `llvm-config --cxxflags --ldflags`
LIBS = `llvm-config --system-libs --libs core`

all: bison flex
	clang++ -g -std=c++14 -I. blc.tab.cpp blc.yy.cpp ast.cpp main.cpp `llvm-config --cxxflags --ldflags` `llvm-config --system-libs --libs core` -frtti -fcxx-exceptions -o main

flex:
	flex -o blc.yy.cpp blc.l

bison:
	bison -d -o blc.tab.cpp blc.y
