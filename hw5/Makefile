CXXFLAGS=-g -O0 -Wall -Iglm-0.9.2.7
LDFLAGS=-lGL -lglut

all: editSpline

editSpline: editSpline.o
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f *.o editSpline
