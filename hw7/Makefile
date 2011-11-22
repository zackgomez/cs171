CXXFLAGS=-g -O0 -Wall -Iglm-0.9.2.7
LDFLAGS=-lGL -lglut -lGLU -lGLEW

all: keyframe

keyframe: keyframe.o parser.tab.o parser.yy.o
	g++ $(LDFLAGS) $(CXXFLAGS) -o $@ $^

parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -d $^

parser.yy.cpp: parser.lex
	flex -+ -o$@ $^

clean:
	rm -f *.o keyframe parser.yy.cpp parser.tab.cpp parser.tab.hpp
