MKDIR_P=@mkdir -p
RM_R=@rm -r
CC=@gcc
CFLAGS=-Wall -Wextra -ggdb -O0 -fprofile-arcs -ftest-coverage
font_light_green=\e[92m
font_light_red=\e[91m
font_default=\e[37m

.PHONY: all, run_tests, clean

all: dirs run_tests

run_tests: dirs build/unit_tests.elf
	@echo -e "$(font_light_green)Running tests (valgrind)...$(font_default)"
	@LC_ALL="C" valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all \
		./build/unit_tests.elf 2>valgrind-res.txt
	@echo -e "$(font_light_green)Running gcov...$(font_default)"
	LC_ALL="C" gcov -b ./build/long_number.o | tee gcov-res.txt
	@echo -en '$(font_light_green)'
	@grep -o 'in use at exit:.*' valgrind-res.txt
	@grep -o 'ERROR SUMMARY:.*' valgrind-res.txt
	@grep 'Lines executed:' gcov-res.txt
	@echo -en '$(font_default)'
	@grep -o 'in use at exit: 0 bytes' valgrind-res.txt >/dev/null \
		|| (echo -e "$(font_light_red)There were memory leaks$(font_default)"; exit 1)
	@grep -o 'ERROR SUMMARY: 0 errors' valgrind-res.txt >/dev/null \
		|| (echo -e "$(font_light_red)There were memory errors$(font_default)"; exit 1)
	@grep -o 'Taken at least once:100.00%' gcov-res.txt >/dev/null \
		|| (echo -e "$(font_light_red)Not all branches are taken at least once$(font_default)"; exit 1)
	@grep -o 'Lines executed:100.00%' gcov-res.txt >/dev/null \
		|| (echo -e "$(font_light_red)Not all lines are executed$(font_default)"; exit 1)

clean:
	$(RM_R) build

dirs:
	$(MKDIR_P) -p build

build/unit_tests.elf: build/unit_tests.o build/long_number.o
	$(CC) $(CFLAGS) build/unit_tests.o build/long_number.o -o build/unit_tests.elf

build/unit_tests.o: src/unit_tests.c src/long_number.h
	$(CC) $(CFLAGS) -c src/unit_tests.c -o build/unit_tests.o

build/long_number.o: src/long_number.c src/long_number.h
	$(CC) $(CFLAGS) -c src/long_number.c -o build/long_number.o
