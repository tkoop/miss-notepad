CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2
CPPFLAGS += -Iinclude -Itests -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700
DEPFLAGS = -MMD -MP
LDFLAGS ?=

SRC_DIR   := src
INC_DIR   := include
TEST_DIR  := tests
BUILD_DIR := build
BIN_DIR   := bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

LIB_SRCS := $(filter-out $(SRC_DIR)/main.c,$(SRCS))
LIB_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

APP      := $(BIN_DIR)/miss
TEST_BIN := $(BUILD_DIR)/run_tests

PREFIX  ?= /usr/local
BINDIR  := $(PREFIX)/bin

.PHONY: all test clean dirs install uninstall static

all: dirs $(APP)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(APP): $(OBJS) | dirs
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.c | dirs
	@mkdir -p $(BUILD_DIR)/tests
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

-include $(wildcard $(BUILD_DIR)/*.d)
-include $(wildcard $(BUILD_DIR)/tests/*.d)

$(TEST_BIN): $(LIB_OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(LIB_OBJS) $(TEST_OBJS) $(LDFLAGS)

test: $(TEST_BIN) $(APP)
	$(TEST_BIN)
	@echo "--- CLI checks ---"
	$(APP) --version | grep -q "MissNotepad "
	$(APP) --help | grep -qF "Usage: miss [FILE]"
	$(APP) -v | grep -q "MissNotepad "
	$(APP) -h | grep -qF "Usage: miss [FILE]"
	! $(APP) --bogus >/dev/null 2>&1
	! $(APP) </dev/null >/dev/null 2>&1
	@echo "All tests passed."

# Install/uninstall. Override PREFIX for distro packaging, e.g.
#   make install PREFIX=/usr
#   make install DESTDIR=/tmp/stage PREFIX=/usr
install: $(APP)
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(APP) $(DESTDIR)$(BINDIR)/miss

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/miss

# Fully static build: the resulting bin/miss has no shared-library
# dependencies and runs on any Linux (glibc or musl). Always rebuilds
# from scratch so no stale dynamic objects linger.
static:
	$(MAKE) clean
	$(MAKE) LDFLAGS="$(LDFLAGS) -static" $(APP)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(APP)
