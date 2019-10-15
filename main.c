#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "graph.h"
#include "dfs_stack.h"

/*test machine:  VOLE
* date: 10/09/2019
* name: Brandon Chan
* x500: chanx411
*/

//Parse the input makefile to determine targets, dependencies, and recipes
int process_file(char *fname) {
	FILE* fp = fopen(fname, "r");
	char line[LINE_SIZE];
	int i = 0;

	if (!fp) {
		printf("Failed to open the file: %s \n", fname);
		return -1;
	}

	//Read the contents and store in lines
	while (fgets(line, LINE_SIZE, fp))
		strncpy(lines[i++], line, strlen(line));

	fclose(fp);

	return 0;
}

/*A method that takes recipe and target : dependent lines from the lines array
	then finds the target and dependents, creates a target_block and adds it to a block array*/
void getBlocks(target blocks[], int *count) {
	for(int i=0; i<MAX_LINES; i++) {
		//if the line is null, at the end of file, so end the loop
		if(!strncmp(lines[i],"\000",1)) {
			break;
		}
		char *recipes[MAX_RECIPES_PT];
		int r_count = 0;

		//Grabbing the "statement," a target : dependent line
		char *statement = lines[i++];
		//Accounting for new lines
		if(!strcmp(statement, "\n")) {
			i--;
			continue;
		}
		//Finding all the recipe lines (if they exist) after a statement
		while(!strncmp(lines[i], "\t", 1)) {
			char *line = lines[i];
			line++;
			recipes[r_count++] = line;
			i++;
		}
		//Decrementing i, accounting for incrementation in previous lines
		i--;

		//Deliminating the statement to get the targets and dependents
		char *delim = ":";
		char *targets = strtok(statement, delim);
		char *depend = strtok(NULL, delim);
		char *dependents[MAX_DEP];
		int dep_c = 0;

		//Seperating all the dependents out into an array. dep_c is the count of dependents
		if(strcmp(&depend[0], "\001") && strcmp(&depend[0], "\000")) {
			char delim2[] = " ";
			char *ptr = strtok(depend, delim2);
			while(ptr!=NULL) {
				if(!strcmp(ptr, "\n")) {
					break;
				}
				dependents[dep_c] = ptr;
				dep_c = dep_c + 1;
				ptr = strtok(NULL, delim2);
			}
		}

		//Creating, assigning and putting the block into array
		target block = {};
		block.name = targets;
		block.name = strtok(block.name, " ");
		for(int i=0; i<MAX_DEP; i++) {
			block.depend[i] = dependents[i];
		}
		for(int i=0; i<MAX_RECIPES_PT; i++) {
			block.recipe[i] = recipes[i];
		}
		block.dep_count = dep_c;
		block.recipe_count = r_count;
		block.visited = 0;
		block.seen = 0;
		// a_block = &block; only use if blocks is **
		blocks[*count] = block;
		*count = *count + 1;
		}
}

//A method to print out using the -p flag
void print_p(int count, target blocks[]) {
	for(int i=0; i<count; i++) {
		printf("target \'%s\' has %d dependencies and %d recipe\n", blocks[i].name, blocks[i].dep_count, blocks[i].recipe_count);
		for(int j=0; j<blocks[i].dep_count; j++) {
			printf("Dependent %d is %s", j, blocks[i].depend[j]);
			if(j < blocks[i].dep_count - 1) {
				printf("\n");
			}
		}
		for(int k=0; k<blocks[i].recipe_count; k++) {
			printf("Recipe %d is %s", k, blocks[i].recipe[k]);
		}
		printf("\n");
	}
}

//A helper that gets the block given the block target
target *getBlockFromName(target *blocks, char *n, int count) {
	for(int i=0; i<count; i++) {
		if(!strcmp(blocks[i].name, n)) {
			target *b = &blocks[i];
			return b;
		}
	}
	//returns NULL if the block doesn't exist for the dependent
	return NULL;
}

//A method to print the recipes in DFS
void print_r(target *blocks, target block, int count) {
	//visit the block
	block.visited = 1;
	int len = block.dep_count;

	//if the block has no dependencies then we have a leaf (base case) - print
	if(block.dep_count == 0) {
		for(int j=0; j<block.recipe_count;j++) {
			printf("%s",block.recipe[j]);
		}
	}

	//iterates thru all the dependencies of a target (if they have at least 1)
	for(int i=0;i<len;i++) {
		//deliminator to get rid of new lines at the end of some dependencies
		char *delim = "\n";
		//seen is an attribute of target I added - it indicates how many of its dependencies have been visited
		block.seen++;
		block.depend[i] = strtok(block.depend[i], delim);
		target *next_block_ptr = getBlockFromName(blocks,block.depend[i], count);
		//if a block exists for a dependency
		if(next_block_ptr != NULL) {
			target next_block = *next_block_ptr;
			//and that block isn't visited
			if(!next_block.visited) {
				//recurse on that block and its dependencies
				print_r(blocks, next_block, count);
			}
		}

		//If all dependents have been seen, print blocks info
		if(block.seen == block.dep_count && block.dep_count != 0) {
			for(int j=0; j<block.recipe_count;j++) {
				printf("%s",block.recipe[j]);
			}
		}
	}
}

//A helper method that creates a stack in order of execution (top of stack is the first executed)
//target block does NOT need to be the first block, can be any root of a dependency tree or subtree
void create_dep_stack(target *blocks,target block, int count, stack *st, int *stack_count) {
	int len = block.dep_count;
	//Visit and push the block
	block.visited = 1;
	push(st,block);
	*stack_count = *stack_count + 1;
	//Iterate through the dependency. Push latest dependency first so first depend on top of stack
	for(int i=len-1;i>=0;i--) {
		//deliminator to get rid of new lines at the end of some dependencies
		char *delim = "\n";
		block.seen++;
		block.depend[i] = strtok(block.depend[i], delim);
		target *next_block_ptr = getBlockFromName(blocks,block.depend[i], count);
		//if a block exists for a dependency
		if(next_block_ptr != NULL) {
			target next_block = *next_block_ptr;
			//and that block isn't visited
			if(!next_block.visited) {
				//recurse on that block and its dependencies
				create_dep_stack(blocks,next_block, count, st, stack_count);
			}
		}
	}
}

void execute(stack *st, int stack_count) {
	for(int i=0; i<stack_count; i++) {
		//pop off the stack
		target block = pop(st);
		int r_count = block.recipe_count;
		for(int j=0; j<r_count; j++) {
			//Iterate through all recipes and fork for each recipe
			pid_t pid = fork();
			if(pid>0) {
				//Parent waiting to iterate again
				wait(NULL);
			} else if(pid == 0) {
				//Executing and printing what is executed
				printf("excuting: %s",block.recipe[j]);
				char *args2[2] = {block.recipe[j], NULL};
				execv(args2[0], args2);
				exit(EXIT_SUCCESS);
			} else {
				exit(!EXIT_SUCCESS);
			}
		}
	}
}

//Validate the input arguments, bullet proof the program
int main(int argc, char *argv[]) {
	//$./mymake Makefile
	//blocks in an array that will hold all the blocks
	target blocks[MAX_LINES];
	//Count is number of blocks
	//stack count is number of blocks in the stack
	int zero = 0;
	int zero2 = 0;
	int *count = &zero;
	int *stack_count = &zero2;
	stack *st;
	if (argc == 2 && strncmp(argv[1], "-p", 2) && strncmp(argv[1], "-r", 2)) {
		process_file(argv[1]);
		getBlocks(blocks,count);
		st = createStack(*count);
		create_dep_stack(blocks,blocks[0],*count,st,stack_count);
		execute(st, *stack_count);
	}

	if (argc == 3) {
		if(!strcmp(argv[2], "-p") || !strcmp(argv[2], "-r")) {
			printf("Wrong order or arguments, -r and -p must be 2nd arg\n");
			exit(!EXIT_SUCCESS);
		}
		//$./mymake Makefile target - start same process but begin at target, not parent
		if (strncmp(argv[1], "-p", 2) && strncmp(argv[1], "-r", 2)) {
			if(!process_file(argv[1])) {
				char* targets = argv[2];
				getBlocks(blocks,count);
				target *targ_ptr = getBlockFromName(blocks,targets,*count);
				if(targ_ptr == NULL) {
					printf("target is not a block in this makefile\n");
					exit(!EXIT_SUCCESS);
				}
				st = createStack(*count);
				create_dep_stack(blocks,*targ_ptr,*count, st, stack_count);
				execute(st,*stack_count);
			}
		} else if (!strncmp(argv[1], "-p", 2)) {
			//$./mymake -p Makefile
			if (!process_file(argv[2])) {
				getBlocks(blocks, count);
				print_p(*count, blocks);
			}
		} else if(!strncmp(argv[1], "-r", 2)) {
				if(!process_file(argv[2])) {
					getBlocks(blocks,count);
					print_r(blocks,blocks[0],*count);
				}
			}
		}
	else if(argc > 3 || argc <2) {
		printf("wrong number of arguments\n");
		printf("must follow ./mymake (-r, -p) Makefile\n");
		printf("or ./mymake Makefile (target)\n");
		exit(!EXIT_SUCCESS);
	}
	exit(EXIT_SUCCESS);
}
