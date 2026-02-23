CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -O2 -std=c11
LDFLAGS     = 

PREFIX     ?= /usr/local
BINDIR      = $(PREFIX)/bin

SRC_DIR     = src
OBJ_DIR     = obj
BIN_DIR     = bin
INC_DIR     = include

TARGET      = $(BIN_DIR)/mystrace
SOURCES     = $(wildcard $(SRC_DIR)/*.c)
OBJECTS     = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
HEADERS     = $(wildcard $(INC_DIR)/*.h)


all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@


install: $(TARGET)
	@echo "Installing binnary in $(DESTDIR)$(BINDIR)..."
	install -D -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/mystrace

uninstall:
	@echo "Deleting binary from $(DESTDIR)$(BINDIR)..."
	rm -f $(DESTDIR)$(BINDIR)/mystrace


clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

rebuild: clean all

run: $(TARGET)
	./$(TARGET) $(ARGS)

.PHONY: all clean rebuild run install uninstall
