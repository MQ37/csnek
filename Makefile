libs = -lncurses

snake : main.o
	gcc -o snake main.o $(libs)

main.o : main.c
	gcc -c main.c

clean :
	rm snake main.o
