// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
	if(!time_travel)
	{
		switch(c->type)
		{
			case AND_COMMAND:
		
				break;
	
			case SEQUENCE_COMMAND:
				SEQUENCEExecutor(c);
				break;
	
			case OR_COMMAND:
				ORExecutor(c);
				break;
	
			case PIPE_COMMAND:
		
				break;
	
			case SIMPLE_COMMAND:
				SIMPLEExecutor(c);
				break;
	
			case SUBSHELL_COMMAND:
		
				break;
		}
	}
}

void
ORExecuter (command_t input){
	int exitStatus;
	int p = fork();
    if(p==0)
	{
    	execute_command(input->u.command[0]);
    	exit(input->u.command[0]->status);
	}
	else
	{
		int status;
		waitpid(p, &status, 0);
		exitStatus = WEXITSTATUS(status);
		if(exitStatus != 0)
		{
			execute_command(input->u.command[1]);
			input->status = input->u.command[1]->status;
			return;
		}
	}
}

void
SIMPLEExecuter (command_t input){
	input->u.word[0][input->words] = '\0';
	int p = fork();
    if(p==0)
	{
    	execvp(input->u.word[0], input->u.word);
	}
	else
	{
		int status;
		waitpid(p, &status, 0);
		input->status = WEXITSTATUS(status);
	}
    return;
}

void SEQUENCEExecuter (command_t input){
	int p = fork();
    if(p==0)
	{
    	execute_command(input->u.command[0]);
    	exit(input->u.command[0]->status);
	}
	else
	{
		int status;
		waitpid(p, &status, 0);
		execute_command(input->u.command[1]);
		input->status = input->u.command[1]->status;
		return;
	}
}
