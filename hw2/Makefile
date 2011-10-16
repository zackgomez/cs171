CXXFLAGS=-g -O0 -Wall -I../zmatrix
LDFLAGS=

all: wireframe

wireframe: wireframe.o wireframe.tab.o wireframe.yy.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

wireframe.tab.cpp wireframe.tab.hpp: wireframe.ypp
	bison -d $^

wireframe.yy.cpp: wireframe.lex
	flex -+ -o$@ $^

clean:
	rm -f *.o wireframe wireframe.yy.cpp wireframe.tab.cpp wireframe.tab.hpp
