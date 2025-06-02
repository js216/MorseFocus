CC = gcc -fanalyzer
INTERCEPT = intercept-build-14
SCAN = scan-build-14

CFLAGS := -std=c99 -Wall -Wextra -Werror -pedantic -Imodules -MMD -MP
LDFLAGS = -lm

modules = diff str gen record
tests = test_diff test_str test_gen test_record
tools = run_tests run_diff run_gen run_words

all: $(addprefix build/, $(tools))
module_objs = $(addprefix build/, $(addsuffix .o, $(modules)))
test_objs = $(addprefix build/, $(addsuffix .o, $(tests)))

.PHONY: clean all check

# Main program files

build/%.o: modules/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: tools/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%: build/%.o $(module_objs)
	$(CC) $^ -o $@ $(LDFLAGS)

# Test files

build/%.o: tests/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/run_tests: tests/run_tests.o $(module_objs) $(test_objs)
	$(CC) $^ -o $@ $(LDFLAGS)
	cd build && ./run_tests

# Static code analysis

check:
	# clang-format
	find . -name '*.c' -o -name '*.h' | xargs clang-format --dry-run -Werror
	# clang-tidy
	make clean
	$(INTERCEPT) --cdb build/compile_commands.json make all
	sed -i 's/-fanalyzer//g' build/compile_commands.json
	jq -r '.[].file' build/compile_commands.json | xargs clang-tidy -p build -system-headers
	# scan-build
	make clean
	$(SCAN) --status-bugs make all

# Common recipes

clean:
	rm -f build/*

build:
	mkdir -p build

-include $(wildcard build/*.d)


