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

//structs
typedef struct {
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct lval** cell;
} lval;

//Possible lval types
enum { LVAL_ERR,  LVAL_NUM, LVAL_SYM, LVAL_SEXPR };

//possible lval error types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

//create a new number type lval
lval* lval_num(long x){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

//Create a new error type lval
lval* lval_err(char* m){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

void lval_del(lval* v){
	switch(v->type){
		case LVAL_NUM:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_SEXPR:
			for(int i = 0; i < v->count; i++){
				lval_del(v->cell[i]);
			}
			break;
	}
	free(v);
}

lval* lval_read_num(mpc_ast_t* t){
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t){
	if(strstr(t->tag, "number")) { return lval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }

	for (int i = 0; i < t ->children_num; i++){
		if(strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if(strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if(strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if(strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if(strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return v;
}

void lval_print(lval* v); //lval_expr_print uses this function and vice versa, forward ddeclaration necesarry.

void lval_expr_print(lval* v, char open, char close) {
	putchar(open);
	for (int i = 0; i < v->count; i++){
		lval_print(v->cell[i]);
		if(i != (v->count - 1)){
			putchar(' ');
		}
	}
	putchar(close);
}

void lval_print(lval* v) {
	switch(v->type) {
		case LVAL_NUM:
			printf("%li", v->num);
			break;

		case LVAL_ERR:
			printf("Error: %s", v->err);
			break;
		case LVAL_SYM:
			printf("%s", v->sym);
			break;
		case LVAL_SEXPR:
			lval_expr_print(v, '(', ')');
			break;
	}
}

void lval_println(lval* v){
	lval_print(v);
	putchar('\n');
}

//Prototypes
lval eval(mpc_ast_t* t);
lval eval_op(lval x, char* op, lval y);

int main(int argc, char** argv){
	//Create parsers:
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("symbol");
	mpc_parser_t* Operator = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	//Define the parsers
	mpca_lang(MPCA_LANG_DEFAULT, //Sets default arg flags
	"							\
	number   : /-?[0-9]+\\.?[0-9]*/ ;					\
	symbol   : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\"; 	\
	sexpr    : '(' <expr>* ')';						\
	expr     : <number> | <symbol> | <sexpr>; 			\
	lispy     : /^/ <expr>* /$/;	\
	",
	Number, Symbol, Sexpr, Expr, Lispy);


	printf("rodLisp Version %s\n", VERSION);
	printf("Press Ctrl+c to Exit\n");

	while(1){
		char* input = readline("rodLisp> ");

		add_history(input);

		//Parse user input:
		mpc_result_t r;
		if(mpc_parse("<stdin", input, Lispy, &r)) { //If the input is a valid lisp program
			lval* x = lval_read(r.output);
			lval_println(x); 
			lval_del(x);
		} else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}


		free(input);
	}
	printf("Thank you for using rodLisp, I hope everything went well for you");
	//Delete Parsers
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
	return 0;
}

lval eval(mpc_ast_t* t){
	//Return numbers, don't evaluate those
	if(strstr(t->tag, "number")){ //If number is a substring of tag
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	//The operator is always the second child, no infix yet.
	char* op = t->children[1]->contents;

	//Store the third child in 'x'
	lval x = eval(t->children[2]);

	//Iterate the remaining children and combining.....
	int i = 3;
	while(strstr(t->children[i]->tag, "expr")){
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}

lval eval_op(lval x, char* op, lval y){ //Parameters in the order of infix notation
	
	if (x.type == LVAL_ERR) { return x; }
	if (y.type == LVAL_ERR) { return y; }

	if(strcmp(op, "+") == 0) {return lval_num(x.num + y.num); }
	if(strcmp(op, "-") == 0) {return lval_num(x.num - y.num); }
	if(strcmp(op, "*") == 0) {return lval_num(x.num * y.num); }
	if(strcmp(op, "/") == 0) { 
		return (y.num == 0) ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
	}
	if(strcmp(op, "%") == 0) {return lval_num(x.num % y.num); }
	if(strcmp(op, "^") == 0) {return lval_num(pow(x.num, y.num)); }
	if(strcmp(op, "max") == 0) {return (x.num > y.num) ? lval_num(x.num) : lval_num(y.num); }
	if(strcmp(op, "min") == 0) {return (x.num <= y.num) ? lval_num(x.num) : lval_num(y.num); } //End of min
	return lval_err(LERR_BAD_OP);
}
