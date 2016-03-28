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
	valgrind --leak-check=full ./build/unit_tests.elf
	@echo -e "$(font_light_green)Running gcov...$(font_default)"
	gcov -b ./build/long_number.o | tee gcov-res.txt
	@echo -en '$(font_light_green)'
	@grep 'Lines executed:' gcov-res.txt
	@echo -en '$(font_default)'

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
