CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude -I/usr/include/postgresql $(INC)
LFLAGS := -levent -levent_openssl -lpq -lssl -lcrypto
VERSION:= $(shell git describe)
DEFINES:= $(DEFINES) -DVERSION=\"$(VERSION)\"
CC     := gcc
BINARY := ventistipes
DEPS   := build/main.o build/smtp.o build/string_helpers.o build/email.o build/postgres.o build/safefree.o \
build/log.o build/push/push.o build/push/android.o

.PHONY: all clean dev

all: build $(DEPS) link

dev: clean
	DEFINES="-DDEV" $(MAKE)

build:
	-mkdir -p build/push bin

%.o: $(patsubst build/%o,src/%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst build/%o,src/%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build bin

install:
	cp bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
