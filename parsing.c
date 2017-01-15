#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpc.h" //Makes defining a grammar easier

#ifdef _WIN32 //Windows alreadu has the equivalent to line history and such

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
#define VERSION "0.0.0.2"

//Prototypes
long eval(mpc_ast_t* t);
long eval_op(long x, char* op, long y);


int main(int argc, char** argv){
	//Create parsers:
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	//Define the parsers
	mpca_lang(MPCA_LANG_DEFAULT, //Sets default arg flags
	"							\
	number   : /-?[0-9]+\\.?[0-9]*/ ;					\
	operator : '+' | '-' | '*' | '/' | '%' | \"add\" | \"sub\" | \"mul\" | \"div\"; \
	expr     : <number> | '(' <operator> <expr>+ ')'; 			\
	lispy     : /^/ <operator> <expr>+ /$/ | /^/ <expr>+ <operator> <expr>+ /$/;		\
	",
	Number, Operator, Expr, Lispy);


	printf("rodLisp Version %s\n", VERSION);
	printf("Press Ctrl+c to Exit\n");

	while(1){
		char* input = readline("rodLisp> ");

		add_history(input);

		//Parse user input:
		mpc_result_t r;
		if(mpc_parse("<stdin", input, Lispy, &r)) { //If the input is a valid lisp program
			long result = eval(r.output);
			printf("%li\n", result); //%li is printf for long
			mpc_ast_delete(r.output);
		} else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}


		free(input);
	}
	printf("Thank you for using rodLisp, I hope everything went well for you");
	//Delete Parsers
	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}

long eval(mpc_ast_t* t){
	//Return numbers, don't evaluate those
	if(strstr(t->tag, "number")){ //If number is a substring of tag
		return atoi(t->contents); //Convert our number to a long and return it.
	}

	//The operator is always the second child, no infix yet.
	char* op = t->children[1]->contents;

	//Store the third child in 'x'
	long x = eval(t->children[2]);

	//Iterate the remaining children and combining.....
	int i = 3;
	while(strstr(t->children[i]->tag, "expr")){
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}

long eval_op(long x, char* op, long y){ //Parameters in the order of infix notation
	if(strcmp(op, "+") == 0) {return x + y; }
	if(strcmp(op, "-") == 0) {return x - y; }
	if(strcmp(op, "*") == 0) {return x * y; }
	if(strcmp(op, "/") == 0) { if(y != 0)
		{ return x / y; } else 
		{ return 0; }} //Babby's first error checking
	return 0;
}
