CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: modelo.cpp
	$(CC) modelo.cpp lib/utils.cpp -o modelo $(GLLIBS)

clean:
	rm -f modelo


