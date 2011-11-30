OBJECTS=main.o readpng.o
CFLAGS=-g
LDFLAGS= -L/usr/lib/nvidia -L/usr/X11R6/lib -lglut -lGL -lGLU -lpng #-lXi -lXmu

glslRenderer: $(OBJECTS)
	g++ $(LDFLAGS) -o glslRenderer $^

.cpp.o:
	g++ $(CFLAGS) -c $<

clean:
	-rm $(OBJECTS) glslRenderer
