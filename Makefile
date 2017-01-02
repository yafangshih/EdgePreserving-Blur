CC=g++
#CFLAGS+=-g
CFLAGS+=`pkg-config --cflags opencv`
LDFLAGS+=`pkg-config --libs opencv`
CFLAGS+=-fopenmp

#Current make system
BIN=./bin/
SOURCE=./src/

LIST=$(BIN)/EdgePreservedBlur

all: $(LIST)

$(BIN)/%: $(SOURCE)%.cpp
	$(CC) $< $(CFLAGS) -o $@  $(LDFLAGS)