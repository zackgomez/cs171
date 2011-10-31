ZMATRIX=../zmatrix
CXXFLAGS=-g -O0 -Wall -I$(ZMATRIX)
LDFLAGS=-lGL -lglut

all: oglRenderer

oglRenderer: oglRenderer.o parser.tab.o parser.yy.o matrix.o transforms.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -d $^

parser.yy.cpp: parser.lex
	flex -+ -o$@ $^

matrix.o: $(ZMATRIX)/matrix.cpp
	g++ $(CXXFLAGS) -c $^

transforms.o: $(ZMATRIX)/transforms.cpp
	g++ $(CXXFLAGS) -c $^

clean:
	rm -f *.o oglRenderer parser.yy.cpp parser.tab.cpp parser.tab.hpp matrix.o transforms.o
