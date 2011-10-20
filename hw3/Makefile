ZMATRIX=../zmatrix
CXXFLAGS=-g -O0 -Wall -I../zmatrix
LDFLAGS=

all: shaded

shaded: shaded.o shaded.tab.o shaded.yy.o transforms.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

shaded.tab.cpp shaded.tab.hpp: shaded.ypp
	bison -d $^

shaded.yy.cpp: shaded.lex
	flex -+ -o$@ $^

transforms.o: $(ZMATRIX)/transforms.cpp
	g++ $(CXXFLAGS) -c $^

clean:
	rm -f *.o shaded shaded.yy.cpp shaded.tab.cpp shaded.tab.hpp transform.o
