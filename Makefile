CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -I. $(INC)
LFLAGS := -levent -levent_openssl -lpq -lssl -lcrypto
DEFINES:= $(DEFINES)
CC     := gcc
BINARY := ventistipes
DEPS   := build/main.o build/smtp.o build/string_helpers.o build/email.o build/postgres.o build/safefree.o \
build/push/push.o build/push/android.o

.PHONY: all clean dev

all: build $(DEPS) link

dev: clean
	DEFINES="-DDEV" $(MAKE)

build:
	-mkdir -p build/push bin

%.o: $(patsubst build/%o,%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst build/%o,%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build bin

install:
	cp bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
