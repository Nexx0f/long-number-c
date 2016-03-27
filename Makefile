CC=gcc
CFLAGS=-c -Wall -Wextra -g

.PHONY: all, run_tests, clean

all: dirs run_tests

run_tests: dirs build/unit_tests.elf
	./build/unit_tests.elf

clean:
	rm -r build

dirs:
	mkdir -p build
	
build/unit_tests.elf: build/unit_tests.o build/long_number.o
	$(CC) build/unit_tests.o build/long_number.o -o build/unit_tests.elf

build/unit_tests.o: src/unit_tests.c src/long_number.h
	$(CC) $(CFLAGS) src/unit_tests.c -o build/unit_tests.o
	
build/long_number.o: src/long_number.c src/long_number.h
	$(CC) $(CFLAGS) src/long_number.c -o build/long_number.o
