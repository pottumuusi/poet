SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP := $(SRC:.c=.d)
HEADERS := $(wildcard *.h) $(wildcard *.const)

# Produce header dependency files to be included below
CFLAGS := -MMD

EXECUTABLE := poet
#COMPILER := clang
COMPILER := gcc

LDFLAGS := -lncurses

.PHONY: clean run slow

$(EXECUTABLE): $(OBJ)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS)

-include $(DEP)

slow: $(SRC) $(HEADERS)
	$(COMPILER) -Wall -o $(EXECUTABLE) $(SRC) $(LDFLAGS)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm $(OBJ) $(EXECUTABLE)
