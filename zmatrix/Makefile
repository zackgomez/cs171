CXXFLAGS=-g -Wall -O0


test: test.o
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $^ -o $@

test.o: matrix.h test.cpp transforms.h
	$(CXX) $(CXXFLAGS) -c test.cpp

clean:
	rm -rf *.o test
