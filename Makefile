helpmake: fuzzer.c
	gcc -o fuzzer fuzzer.c

all: helpmake
