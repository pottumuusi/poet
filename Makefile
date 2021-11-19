SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

EXECUTABLE := poet
#COMPILER := clang
COMPILER := gcc

LDFLAGS := -lncurses

.PHONY: clean

$(EXECUTABLE): $(OBJ)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm $(OBJ) $(EXECUTABLE)
