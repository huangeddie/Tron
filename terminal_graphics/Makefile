all : program.out

program.out : main.c color_graphics.o
	gcc -std=gnu99 main.c color_graphics.o -o program.out -lncurses -pthread -Wall -O0 -g

color_graphics.o : color_graphics.c color_graphics.h
	gcc -std=gnu99 -c color_graphics.c -lncurses -pthread -Wall -O0 -g

make clean:
	rm program.out -f
	rm color_graphics.o -f
