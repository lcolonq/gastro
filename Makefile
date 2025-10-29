SRCS := src/gastro.c src/trig.c
OBJECTS := $(SRCS:src/%.c=build/%.o)
HEADERS := $(wildcard src/*.h)
EXE := gastro
LIB := libcolonq-gastro.a

CC ?= gcc
AR ?= ar
CHK_SOURCES ?= $(SRCS)
CPPFLAGS ?= -MMD -MP
CFLAGS ?= --std=c89 -g -Ideps/ -Isrc/ -Wall -Wextra -Wpedantic -Wconversion -Wformat-security -Wshadow -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wnull-dereference -Wfloat-equal -Wundef -Wpointer-arith -Wbad-function-cast -Wlogical-op -Wmissing-braces -Wcast-align -Wstrict-overflow=5 -ftrapv
LDFLAGS ?= -g -lraylib -lcolonq-qoi

prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
includedir ?= $(prefix)/include
libdir ?= $(exec_prefix)/lib

.PHONY: all clean install check-syntax

run: $(EXE)
	./$(EXE)

all: $(EXE) $(LIB)

$(EXE): build/main.o $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS)

$(LIB): $(OBJECTS)
	ar rcs $@ $^

build:
	mkdir build/

build/%.o: src/%.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

clean:
	-rm $(EXE)
	-rm -r build/

TAGS: $(SRCS)
	etags $^

install: $(EXE) $(LIB)
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(libdir) $(DESTDIR)$(includedir)
	install $(EXE) $(DESTDIR)$(bindir)/$(EXE)
	install $(LIB) $(DESTDIR)$(libdir)/$(LIB)
	install $(HEADERS) $(DESTDIR)$(includedir)

check-syntax: TAGS
	gcc $(CFLAGS) -fsyntax-only $(CHK_SOURCES)

-include $(OBJECTS:.o=.d)
