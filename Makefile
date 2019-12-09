CXXFLAGS = `llvm-config --cxxflags --ldflags`
LIBS = `llvm-config --system-libs --libs core`

flex:
	flex -o blc.yy.cpp blc.l

bison:
	bison -d -o blc.tab.cpp blc.y

all: bison flex
	clang++ -g -std=c++14 $(CXXFLAGS) $(LIBS) -o o -I. blc.tab.cpp blc.yy.cpp main.cpp ast.cpp