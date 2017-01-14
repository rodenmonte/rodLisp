#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#define VERSION "0.0.0.1"


int main(int argc, char** argv){

	printf("rodLisp Version %s\n", VERSION);
	printf("Press Ctrl+c to Exit\n");

	while(1){
		char* input = readline("rodLisp> ");

		add_history(input);

		printf("You said: %s\n", input);

		free(input);
	}
	return 0;
}
