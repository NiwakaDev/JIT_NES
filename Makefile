.PHONY:clean build all

EMULATOR = fami
WASM_EMULATOR = wasm_fami

CC = g++
G++_OPTIONS = -Wall -std=c++11 -g -O0  -I ${INCLUDE_DIR} -m32
G++_OPTIONS += -lSDL2

SOURCE_DIR      = ./src/
INCLUDE_DIR     = ./include/

vpath %.hpp ${INCLUDE_DIR}
vpath %.cpp ${SOURCE_DIR}

HEADERS = $(wildcard ${INCLUDE_DIR}*.hpp)
SOURCES = $(wildcard ${SOURCE_DIR}*.cpp)
OBJECTS = ${SOURCES:.cpp=.o}
WASM_OBJECTS = ${SOURCES:.cpp=.o}

all:${EMULATOR}

%.o : %.cpp ${HEADERS}
	${CC} ${G++_OPTIONS} -o $@ -c $<

${EMULATOR} : ${OBJECTS} 
	${CC} ${LD_FLGS} ${G++_OPTIONS} -o ${EMULATOR} $^


#%.o : %.cpp ${HEADERS}
#	emcc  -o $@ -c $< -s USE_SDL=2 -s USE_SDL_IMAGE=2 -I ${INCLUDE_DIR} -lSDL2

#${WASM_EMULATOR}:${WASM_OBJECTS}
#	emcc  -s WASM=1 -o fami.html $^ -s USE_SDL=2 -s USE_SDL_IMAGE=2

clean :
	rm ${SOURCE_DIR}*.o
	rm ${EMULATOR}
