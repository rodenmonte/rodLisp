mymakefile: parsing.c
	gcc -std=c99 -Wall -pedantic parsing.c mpc.c -ledit -lm -o rodLisp
