helpmake: help.c
	gcc -o help help.c

clean:
	rm Sacha.txt

all: helpmake clean
