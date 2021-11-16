SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

EXECUTABLE := poet

LDFLAGS := -lncurses

.PHONY: clean

$(EXECUTABLE): $(OBJ)
	gcc -Wall -o $@ $^ $(LDFLAGS)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm $(OBJ) $(EXECUTABLE)
