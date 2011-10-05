CXXFLAGS=-g -O0 -Wall -I../zmatrix
LDFLAGS=

all: transform4x4 draw2d

transform4x4: transform4x4.tab.o transform4x4.yy.o transform4x4.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

transform4x4.tab.cpp transform4x4.tab.hpp: transform4x4.ypp
	bison -d $^

transform4x4.yy.cpp: transform4x4.lex
	flex -+ -o$@ $^

draw2d: draw2d.o draw2d.tab.o draw2d.yy.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

draw2d.tab.cpp draw2d.tab.hpp: draw2d.ypp
	bison -d $^

draw2d.yy.cpp: draw2d.lex
	flex -+ -o$@ $^

clean: cleantransform cleandraw2d

cleantransform:
	rm -f *.o transform4x4.yy.cpp transform4x4.tab.cpp transform4x4.tab.hpp transform4x4

cleandraw2d:
	rm -f *.o draw2d draw2d.yy.cpp draw2d.tab.cpp draw2d.tab.hpp
