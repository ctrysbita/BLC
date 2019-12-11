CXX = g++
LLVM_FLAGS = `llvm-config --cxxflags --ldflags`
LLVM_LIBS = `llvm-config --system-libs --libs core`

objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

ng: $(objects)
	$(CXX) $(objects) $(LLVM_FLAGS) $(LLVM_LIBS) -o main

all: bison flex $(objects)
	$(CXX) $(objects) $(LLVM_FLAGS) $(LLVM_LIBS) -o main

%.o: %.cpp
	$(CXX) -g $(LLVM_FLAGS) $(LLVM_LIBS) -I. -I../third_party -frtti -fexceptions -std=c++17 -c $< -o $@

flex:
	flex -o blc.yy.cpp blc.l

bison:
	bison -d -o blc.tab.cpp blc.y

clean:
	rm *.o