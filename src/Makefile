CXX=g++ -std=c++2a -c
CXXFLAGS=-O2 -ggdb -I /usr/include/SDL2 -I thirdparty -I GL
LINK=g++
LINKFLAGS=-lm -lSDL2 -lGL

EXE=mjewels

HEADERS=MapFile.h LoadFont.h ToUnicode.h SDLWrapper.h

OBJS=Main.o MapFile.o LoadFont.o ToUnicode.o SDLWrapper.o

.PHONY: all clean

all: ${EXE}

clean:
	rm ${OBJS}

${EXE}: ${OBJS}
	${LINK} ${LINKFLAGS} $^ -o ${EXE}

%.o : %.cpp ${HEADERS} Makefile
	${CXX} ${CXXFLAGS} $*.cpp -o $*.o
