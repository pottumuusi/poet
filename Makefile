SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

EXECUTABLE := poet

# LDFLAGS := 

.PHONY: clean

$(EXECUTABLE): $(OBJ)
	gcc -Wall -o $@ $<

run:
	./$(EXECUTABLE)

clean:
	rm $(OBJ) $(EXECUTABLE)
