ZMATRIX=../zmatrix
CXXFLAGS=-g -O0 -Wall -I../zmatrix
LDFLAGS=

all: wireframe

wireframe: wireframe.o wireframe.tab.o wireframe.yy.o transforms.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

wireframe.tab.cpp wireframe.tab.hpp: wireframe.ypp
	bison -d $^

wireframe.yy.cpp: wireframe.lex
	flex -+ -o$@ $^

transforms.o: $(ZMATRIX)/transforms.cpp
	g++ $(CXXFLAGS) -c $^

clean:
	rm -f *.o wireframe wireframe.yy.cpp wireframe.tab.cpp wireframe.tab.hpp transform.o
