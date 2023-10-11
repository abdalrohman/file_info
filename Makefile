# tool macros
CC := gcc
# CCFLAGS :=-Wall
CCFLAGS :=-fsanitize=signed-integer-overflow -fsanitize=undefined -ggdb3 -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow

# path macros
BIN_PATH := bin
SRC_PATH := src

TARGET_NAME := file_info
INSTALL_NAME := fio
TARGET := $(BIN_PATH)/$(TARGET_NAME)

INSTALL_PREFIX = /usr/local/bin
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

CLEAN_LIST := $(BIN_PATH)

# default rule
default: makedir all

$(TARGET): $(SRC_PATH)/main.c
	$(CC) $(CCFLAGS) -o $@ $<

.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH)

.PHONY: all
all: $(TARGET)

# Installs to the set path
.PHONY: install
install:
	@echo "Installing to $(INSTALL_PREFIX)"
	@cp $(TARGET) $(INSTALL_PREFIX)/$(INSTALL_NAME)

.PHONY: uninstall
uninstall:
	@echo "Uninstalling $(INSTALL_NAME)"
	@rm -f $(INSTALL_PREFIX)/$(INSTALL_NAME)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -rf $(CLEAN_LIST)