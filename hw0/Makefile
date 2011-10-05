# Makefile for cs171 HW 0
# Just type "make" to compile; if it works right, you should have an
# executable called hw0.

CXX		= g++
# add -g for debugging info
CXXFLAGS	= -Wall -g

LDFLAGS	= -L/usr/X11R6/lib -lglut -lGL -lGLU -lXi -lXmu

SRCS	= main.cpp
OBJS	= $(SRCS:.cpp=.o)
PROG	= hw0

all:	$(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(LDFLAGS)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o $(PROG)
