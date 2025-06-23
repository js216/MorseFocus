CFLAGS = -std=c99 -Wall -Wextra -pedantic -fanalyzer -MMD -MP -Iinclude -I.
OBJS = $(patsubst %.c, build/%.o, $(wildcard lib/*.c source/*.c))
TEST = $(patsubst %.c, build/%.o, $(wildcard tests/*.c))

.PHONY: test clean check check-format check-tidy check-scan
-include $(wildcard build/*.d)

# Main program

build/prog/morsefocus: build/prog/morsefocus.o $(OBJS)
	$(CC) $^ -o $@ -lm

build/lib/%.o: lib/%.c | build
	$(CC) $(filter-out -fanalyzer,$(CFLAGS)) -c $< -o $@

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build/lib build/prog build/source build/tests

clean:
	rm -rf build

# Testing and linting

test: build/prog/run_tests build/prog/morsefocus
	cd build/prog && ./run_tests || { rm run_tests; exit 1; }

build/prog/run_tests: build/prog/run_tests.o $(OBJS) $(TEST)
	$(CC) $^ -o $@ -lm

check-format:
	find . -path ./lib -prune -o \( -name '*.c' -o -name '*.h' \) -print \
		| xargs clang-format --dry-run -Werror

check-tidy: | build
	$(MAKE) clean
	intercept-build-14 --cdb build/compile_commands.json $(MAKE) test
	sed -i 's/-fanalyzer//g' build/compile_commands.json
	jq -r '.[].file' build/compile_commands.json | xargs clang-tidy \
		-p build -system-headers -warnings-as-errors=*

check-scan: | build
	$(MAKE) clean
	scan-build-14 --status-bugs $(MAKE) test \
		CFLAGS="$(filter-out -fanalyzer,$(CFLAGS))"

check: test check-format check-tidy check-scan
