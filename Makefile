SRC=src
OBJ=src
BIN=bin
DAT=data

CC=gcc

CFLAGS=-O0 -ggdb -Wall

ALL: mkdirs
	make $(BIN)/fdd

$(OBJ)/fdd.o: $(SRC)/fdd.c
	$(CC) $(CFLAGS) $(USERFLAGS) -c -o $@ $^

$(BIN)/fdd: $(OBJ)/fdd.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean mkdirs rmdirs

clean: rmdirs
	rm -f $(OBJ)/*.o
	rm -rf $(BIN)
	rm -rf $(DAT)

mkdirs:
	mkdir -p $(BIN)
	mkdir -p $(DAT)

rmdirs:
	rm -rf $(BIN)
	rm -rf $(DAT)
