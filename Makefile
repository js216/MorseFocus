CC = gcc
INTERCEPT = intercept-build-14
SCAN = scan-build-14

INCLUDE = -Imodules -Ilib
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -fanalyzer -MMD -MP $(INCLUDE)
CFLAGS = -std=c99 -Wall -Wextra -pedantic -MMD -MP $(INCLUDE)
LDFLAGS = -lm

lib = miniaudio
modules = diff str gen record cw
tests = test_diff test_str test_gen test_record test_cw
tools = run_diff run_gen run_words run_cw morsefocus

lib_objs = $(addprefix build/, $(addsuffix .o, $(lib)))
module_objs  = $(addprefix build/, $(addsuffix .o, $(modules)))
test_objs    = $(addprefix build/, $(addsuffix .o, $(tests)))

all: $(addprefix build/, $(tools))
.PHONY: clean all check format

# Main program files

build/%.o: lib/%.c | build
	$(CC) $(CLAGS_LIBS) -c $< -o $@

build/%.o: modules/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: tools/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%: build/%.o $(lib_objs) $(module_objs) build/run_tests
	$(CC) $(filter-out build/run_tests, $^) -o $@ $(LDFLAGS)

# Test files

build/%.o: tests/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/run_tests: build/run_tests.o $(lib_objs) $(module_objs) $(test_objs)
	$(CC) $^ -o $@ $(LDFLAGS)
	cd build && ./run_tests || { rm -f run_tests; exit 1; }

# Static code analysis

format:
	find . -name '*.c' -o -name '*.h' | xargs clang-format -i

check:
	# clang-format
	find . -path ./lib -prune -o \( -name '*.c' -o -name '*.h' \) -print | xargs clang-format --dry-run -Werror
	# clang-tidy
	make clean
	$(INTERCEPT) --cdb build/compile_commands.json make all
	sed -i 's/-fanalyzer//g' build/compile_commands.json
	jq -r '.[].file' build/compile_commands.json | xargs clang-tidy -p build -system-headers -warnings-as-errors=*
	# scan-build
	make clean
	$(SCAN) --status-bugs make all CFLAGS="$(filter-out -fanalyzer,$(CFLAGS))"

# Common recipes

clean:
	rm -f build/*

build:
	mkdir -p build

-include $(wildcard build/*.d)

