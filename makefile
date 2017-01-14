mymakefile: main.c
	gcc -std=c99 -Wall -pedantic main.c -ledit -o rodLisp
