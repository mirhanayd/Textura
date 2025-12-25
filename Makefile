CFLAGS=-I. -I./PNG -I./zlib -O2
CC=gcc
CXX=g++

PNG_SRCS := $(wildcard PNG/*.c)
ZLIB_SRCS := $(wildcard zlib/*.c)
CPP_SRCS := main.cpp

# Put all object files into build/obj/ (flat layout)
PNG_OBJS := $(patsubst %.c,build/obj/%.o,$(notdir $(PNG_SRCS)))
ZLIB_OBJS := $(patsubst %.c,build/obj/%.o,$(notdir $(ZLIB_SRCS)))
CPP_OBJS := build/obj/main.o

all: bin/textura_exe

# ensure outputs dir exists inside textura
outputs:
	@mkdir -p outputs
	@chmod 755 outputs

# Rules: compile PNG/*.c -> build/obj/<basename>.o
build/obj/%.o: PNG/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# compile zlib/*.c -> build/obj/<basename>.o
build/obj/%.o: zlib/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# compile main.cpp -> build/obj/main.o
build/obj/main.o: main.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

bin/textura_exe: $(PNG_OBJS) $(ZLIB_OBJS) $(CPP_OBJS)
	@mkdir -p bin
	$(CXX) $(CPP_OBJS) $(PNG_OBJS) $(ZLIB_OBJS) -lpng -lz -o $@

run: outputs bin/textura_exe
	./bin/textura_exe

clean:
	@echo "Cleaning build, bin, and outputs (force removing outputs/* if present)"
	-chmod -R u+rw outputs 2>/dev/null || true
	rm -rf build bin outputs

.PHONY: all run clean