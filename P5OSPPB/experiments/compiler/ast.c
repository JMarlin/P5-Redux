#include <stdio.h>

typedef struct ast_node {
	unsigned char type;
	unsigned char* symbol;
	unsigned int child_count;
	struct ast_node* parent;
	struct ast_node** child;
} ast_node;

//The idea, here, is that we'll have a pre-populated list of function pointers
//to functions which take a current position in a string, the string itself, and
//attempt to turn the segment following the current position into the particular
//type of token/ast_node which that function handles. If it fails, it returns
//that it read nothing and the loop tries applying the next function until it
//runs out of functions.
//The functions are also responsible for updating the current line/column
//position in the source string
int main(void) {
	
	//In real life, we'll read this in from a file
	unsigned char code[] = "6-((12+5)/3)"; 
	unsigned int code_length = strlen(code);
	unsigned int cur_location = 0;
	unsigned int chars_consumed = 0;
	ast_node* active_node = (ast_node*)0;
	unsigned int cur_line = 1;
	unsigned int cur_column = 1;
	
	while(cur_location < code_length) {
		
		//Iterate through all the factory functions for 
		for(i = 0; i < node_builder_count; i++) {
		
			//Try to use a factory function on the stream
			chars_consumed = build_node[i](active_node, &cur_line, &cur_column, code, cur_location);
			
			//If we were able to interpret any of the stream, we're done
			if(chars_consumed)
				break;
		}
		
		if(!chars_consumed) {
			
			//If we iterated through all of the node builders and none of them
			//could complete any work, then we know we had a syntax error
			printf("The system encountered a syntax error and will quit.\n");
			break;
		}
		
		cur_location += chars_consumed;
	}
	
	//Here, if the entire string was consumed we would proceed on to parsing
	//the AST tree we just generated
	
	//Can probably get rid of this, but why not
	prints("Done.\n");
	
	return 0;
}