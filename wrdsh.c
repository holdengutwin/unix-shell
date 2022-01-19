/*
Partner 1: Theodore Buckley
trb702
11279739

Partner 2:
Holden Gutwin
11273492
hcg369 

CMPT 332 - Assignment 1b
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "stack.h"

/*
Structure that represents a single command to be executed
*/
typedef struct command {
	
	char **args; /* string array that contains the command's name (index 0)
			and its arguments, if any */

	char next_operator; /* the operator that immediately follows this command,
			       will be either |, <, or x (no operator) */

	char prev_operator; /* the operator that immediately precedes this command,
			       will be either |, <, or x (no operator) */
} Command;

/*
Takes a single command and its arguments as a string and generates a Command structure.
*/
Command *string_to_command(char *input) {

	// create a new Command structure to represent the current command being read
	Command *current_read = (Command*) malloc(sizeof(Command));

	// allocate arguments list to initially hold 1 string (will grow if more arguments are read)
	current_read->args = (char**) malloc(sizeof(char*));

	// prepare to copy all space delimited arguments in the input line into the list of arguments
	const char delim[2] = " "; 

	char *save_ptr; // need to use strtok_r as nested strtok calls occur
	char *token = strtok_r(input, delim, &save_ptr);
	int n = 0; // counter variable
	while (token != NULL)
	{
		// add the current argument into its position in the array
		current_read->args[n] = strdup(token);
		// reallocate the arguments list to store one more argument
		n++;
		current_read->args = (char**) realloc(current_read->args, (sizeof(char*)) * n+1); // this statement causes a memory leak and idk how to fix :(
		token = strtok_r(NULL, delim, &save_ptr);
	}
	current_read->args[n] = NULL; // signify the end of the arguments array
	return current_read; // return the newly generated Command structure
}

int main () {
	while(1) {
		// create a stack to hold Command structs
		Stack *command_stack;
		command_stack = init_stack();

		// hold a variable for the name of the file we should redirect to, if we are doing file redirection
		char *outfile = NULL; 

		// print the shell prompt message
		printf("wrdsh> "); 
		// set up arguments for getline() so that it will allocate a buffer that is the size of the input string plus 1
		char *input = NULL;
		size_t bufsize = 0;

		// read the line from stdin
		ssize_t n_characters = getline(&input, &bufsize, stdin);
		
		if (n_characters < 0) { // if true, read failed
            int error = errno;
            printf("%d\n", error);
            free(input);
			free(command_stack);
            printf("Line read failed, exiting...\n");
            exit(1);
        }

        //Checking for enter as the command
        if (n_characters == 1 && input[0] == '\n'){
            continue;
        }


		// strip the newline character
		input[n_characters - 1] = 0;

		// if the user typed "exit" exit the program
		if(!strcmp(input,"exit")) {
			free(input);
			printf("Exiting...\n");
			exit(1);
		}

		// if there is a redirect operator in the command string, parse it
		if (strchr(input, '<') != NULL) {

			char *redirect_token = strtok(input, "<");

			outfile = strdup(redirect_token);
			redirect_token = strtok(NULL, "<"); // grab the portion of the string after the redirect operator
			strcpy(input,redirect_token);

			// super gross way of trimming whitespace from outfile
			int n = 0;
			int length = strlen(outfile);
			for (int i = 0; i < length; i++) {
				if (outfile[i] == ' ') {
				}
				else {
					outfile[n] = outfile[i];
					n++;
				}
			}
			outfile[n] = 0;
		}
		
		
		
		

		// split up string by pipes
		char *save_ptr; // since nested calls to strtok occur, must use strtok_r which uses a save ptr
		char *pipe_token = strtok_r(input, "|", &save_ptr);
		while (pipe_token != NULL)
		{
			// convert the current string token into a Command
			Command *current_read = string_to_command(pipe_token);
			current_read->prev_operator = '|';

			// peek at the stack to get the next operator
			if (peek(command_stack) == NULL) { // if this is the first command in the stack
				if(outfile != NULL) { // if we are doing redirection 
					current_read->next_operator = '<';
				}
				else { // if we are not
					current_read->next_operator = 'x';
				}
				
			}
			else {
				current_read->next_operator = ((Command*)peek(command_stack))->prev_operator;
			}

			push(command_stack, (void*) current_read);
			pipe_token = strtok_r(NULL, "|", &save_ptr);
		}
		// top of the stack will be the first command, which has no previous operator
		((Command*)peek(command_stack))->prev_operator = 'x';

		int fd[2];
		int out_fd[2];		
		// store file descriptors from the previous pipe, if multiple commands are present
		// this is required in order to carry data through multiple pipes, as we remake the pipe each time
		int prev_read;
		int prev_write;
		
		int n_commands = stack_size(command_stack);
		int do_duplication = 0;

		//execute all commands in the stack
		while (peek(command_stack) != NULL) {
			
			// pop a command off the stack
			Command *current_command = (Command*) pop(command_stack);

			// create a new pipe, unless this is the last command in the sequence
			if (current_command->next_operator == '|') {
				pipe(fd);
			}
			if (current_command->next_operator == 'x') {
				pipe(out_fd);
				do_duplication = 1; //  
			}

			int rc = fork(); 
			if (rc < 0) { // fork failed, exit
				fprintf(stderr, "Fork failed! :))\n");
				exit(1);
			}

			else if (rc == 0) // child goes down this path
			{	

				// if previous command exists
				if (current_command->prev_operator != 'x') {
					// since we remade the pipe, have the current command get stdin from the read end from the "previous" pipe
					dup2(prev_read, STDIN_FILENO);

					// two file descriptors for the read end of the "current" pipe now exist (stdin overwritten) so we can close one
					close(prev_read);

					// write end of the "current" pipe not needed
					close(prev_write);
				}
				
				// if next command is a pipe
				if (current_command->next_operator == '|') {
					// redirect the current command's stdout to the write end of the "current" pipe
					dup2(fd[1], STDOUT_FILENO);

					// similar to above, this is now a duplicate file descriptor
					close(fd[1]);

					// no longer needed
					close(fd[0]);
				}

				// if next command is a file to be redirected to
				if (current_command->next_operator == '<') {
					
					if(n_commands > 1) {
						close(fd[0]);
						close(fd[1]);
					}

					int new_out = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
					dup2(new_out, STDOUT_FILENO);
					close(new_out);
					// keep in mind that the "previous" pipe will have already been closed in this case as stdin was redirected
				}

				if (current_command->next_operator == 'x') {
					dup2(out_fd[1], STDOUT_FILENO);

					if(n_commands > 1) {
						close(fd[0]);
						close(fd[1]);
					}
					close(out_fd[0]);
					close(out_fd[1]);
				}
				
				// execute command
				execvp(current_command->args[0], current_command->args);
				
				// if we reach this point, the command did not exist
				fprintf(stderr, "Failed to execute command! :))\n");
				exit(1);
			}
			else { // parent goes down this path
				
				if (current_command->prev_operator != 'x') {
					// can now permanently close the "previous" pipe's file descriptors
					// this will only occur if this is not the first command in the pipe sequence
					close(prev_read);
					close(prev_write);
				}

				if (current_command->next_operator == '|') {
					// the "current" pipe becomes the "previous" pipe in the next command (only if there is a next command)
					prev_read = fd[0];
					prev_write = fd[1];
					// the "current" pipe in the next iteration will be generated at the top of the while loop
				}

			}

		}

		// if multiple commands were executed, there will be a lingering open pipe (the "previous" pipe)
		if (n_commands > 1) {
			close(prev_read);
			close(prev_write);
		}
		
		// wait for all children that were created, 
		for (int i = 0; i < n_commands; i++) {
			wait(NULL);
		}

		if (do_duplication) {
			char buf;
			close(out_fd[1]);
			int done = 0;
			while (!done) {
				int chars_read = read(out_fd[0], &buf, 1);
				if (buf == 'c' || buf == 'm' || buf == 'p' || buf == 't') {
					printf("%c", buf);
				}
				printf("%c", buf);

				if (chars_read == 0) {
					done = 1;
				}
			}
			close(out_fd[0]);
		}
		
		// free relevant memory
		free(input); 	
		free(outfile);
		free(command_stack);
	}
	
}
