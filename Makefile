# Makefile
# aoneill - 05/23/17

# Top level
SRC := src
OBJ := obj

EXEC_NAME := tzme
SYSTEMD_SERVICE := $(EXEC_NAME).service

# System
ifeq ($(origin PREFIX), undefined)
	PREFIX := /usr
endif

################################################################################

# Tool-chain
CXX := gcc
CXXFLAGS := -Wall -fPIC -DPIC -I./$(SRC)

PKGS := libcurl libsystemd
CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
CXXFLAGS += $(shell xml2-config --cflags)

LDFLAGS  := $(shell pkg-config --libs $(PKGS))
LDFLAGS += $(shell xml2-config --libs)

################################################################################

# Basic functionality
SUPPORT_C = $(shell cd $(SRC)/ && find * -name "*.c" -type f)
SUPPORT_O = $(addprefix $(OBJ)/,$(SUPPORT_C:%.c=%.o))

all: CXXFLAGS += -Ofast
all: init $(LIB_NAME) $(EXEC_NAME)
debug: CXXFLAGS += -DDEBUG -g -pg
debug: init $(LIB_NAME) $(EXEC_NAME)

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $^ -o $@

################################################################################

# Executable
$(EXEC_NAME): $(SUPPORT_O) main.c
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

################################################################################

# Structure
$(OBJ):
	mkdir -p "$(OBJ)"

.PHONY: init
init: $(OBJ)

.PHONY: clean
clean:
	rm -rf ./$(OBJ)/
	rm -rf $(EXEC_NAME) gmon.out

# Distribute
.PHONY: install
install: all
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib/systemd/system
	cp $(EXEC_NAME) $(PREFIX)/bin/
	cp $(SYSTEMD_SERVICE) $(PREFIX)/lib/systemd/system/

