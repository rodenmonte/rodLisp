#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32 //Windows alreadu has the equivalent to line history and such
#include <string.h>

static char buffer[2048];

char* readline(char* prompt){
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy) - 1] = '\0';
	return cpy
}

void add_history(char* unused){}

#else //If we're not running windows, we need the editline functions
#include <editline/readline.h>
#include <editline/history.h>
#endif

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
	printf("Thank you for using rodLisp, I hope everything went well for you");
	return 0;
}
