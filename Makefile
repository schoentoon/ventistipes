CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -I. $(INC)
LFLAGS := -levent
DEFINES:= $(DEFINES)
CC     := gcc
BINARY := ventstipes
DEPS   := build/main.o build/smtp.o build/string_helpers.o build/email.o

.PHONY: all clean dev

all: build $(DEPS) link

dev: clean
	DEFINES="-DDEV" $(MAKE)

build:
	-mkdir build bin

%.o: $(patsubst build/%o,%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst build/%o,%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -fv $(DEPS) bin/$(BINARY)

install:
	cp bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
