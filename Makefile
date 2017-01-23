OBJECTS = main.o window.o parse.o
CFLAGS = -g -lncurses 
PREFIX = /usr/local

default: bondtop

$(OBJECTS): %.o : src/%.c
	-mkdir -p build
	gcc $(CFLAGS) -Isrc/c -c $< -o build/$@ 

bondtop: $(OBJECTS)
	gcc $(OBJECTS:%.o=build/%.o) $(CFLAGS) -o $@

install: bondtop
	install -m 0755 bondtop $(PREFIX)/bin

.PHONY: install