OBJECTS = main.o window.o parse.o
CFLAGS = -g -lncurses 

default: bondtop

$(OBJECTS): %.o : src/%.c
	-mkdir -p build
	gcc $(CFLAGS) -Isrc/c -c $< -o build/$@ 

bondtop: $(OBJECTS)
	gcc $(OBJECTS:%.o=build/%.o) $(CFLAGS) -o $@