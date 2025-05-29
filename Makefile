CC = gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic -Isource -lm

tests = test_diff test_str test_gen test_weights
tools = run_diff run_gen

all: $(patsubst %,build/%.exe,$(tests) $(tools))

build/run_diff.exe: tools/run_diff.c build/diff.o build/str.o build/weights.o
build/run_gen.exe: tools/run_gen.c build/gen.o build/weights.o build/str.o

build/test_diff.exe: test/test_diff.c build/diff.o build/str.o build/weights.o
build/test_str.exe: test/test_str.c build/str.o
build/test_gen.exe: test/test_gen.c build/gen.o build/str.o
build/test_weights.exe: test/test_weights.c build/weights.o build/str.o

build/diff.o: source/diff.c source/diff.h
build/str.o: source/str.c source/str.h
build/gen.o: source/gen.c source/gen.h
build/weights.o: source/weights.c source/weights.h source/str.h

.PHONY: clean all tests

clean:
	rm -f build/*

%.exe:
	$(CC) -o $@ $^ $(CFLAGS)

%.o: | build
	$(CC) -c -o $@ $< $(CFLAGS)

build:
		mkdir -p build
