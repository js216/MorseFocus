CC = gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic -Imodules -MMD -MP
LDFLAGS = -lm 

modules = diff str gen weights
tests = test_diff test_str test_gen test_weights
tools = run_tests run_diff run_gen

all: $(addprefix build/, $(tools))
module_objs = $(addprefix build/, $(addsuffix .o, $(modules)))
test_objs = $(addprefix build/, $(addsuffix .o, $(tests)))

.PHONY: clean all

# Main program files

build/%.o: modules/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

build/%: tools/%.c $(module_objs)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Test files

build/%.o: tests/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

build/run_tests: tests/run_tests.c $(module_objs) $(test_objs)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	$@

clean:
	rm -f build/*

build:
	mkdir -p build

-include $(wildcard build/*.d)
