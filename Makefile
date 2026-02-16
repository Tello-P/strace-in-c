CC          = gcc
CFLAGS      = -Wall -Wextra -O2 -std=c11
LDFLAGS     =

SRC_DIR     = src
OBJ_DIR     = obj
BIN_DIR     = bin
INC_DIR     = include

TARGET      = $(BIN_DIR)/mystrace

SOURCES     = $(wildcard $(SRC_DIR)/*.c)

OBJECTS     = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

HEADERS     = $(wildcard $(INC_DIR)/*.h)

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -f $(TARGET)

distclean: clean
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean distclean

rebuild: clean all

run: $(TARGET)
	./$(TARGET)

