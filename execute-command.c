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
				SEQUENCEExecuter(c);
				break;
	
			case OR_COMMAND:
				ORExecuter(c);
				break;
	
			case PIPE_COMMAND:
				PIPEExecuter(c);
				break;
	
			case SIMPLE_COMMAND:
				SIMPLEExecuter(c);
				break;
	
			case SUBSHELL_COMMAND:
		
				break;
		}
	}
}

void 
PIPEExecuter(command_t input)
{
	int fd[2];
	pipe(fd);
	int firstPid = fork(); // execute right command
	if(firstPid == 0)
	{
		close(fd[1]); // close unused write end
		dup2(fd[0], 0);
		execute_command(input->u.command[0], false);
	}
	else
	{
		int secondPid = fork();
		if(secondPid == 0)
		{
			close(fd[0]); // close unused write end
			dup2(fd[1], 1);
			execute_command(input->u.command[0], false);
		}
		else
		{
			close(fd[0]);
			close(fd[1]);
			int status;
			int returnedPID = waitpid(-1, &status, 0);
			if(returnedPID == secondPid)
			{
				waitpid(firstPid, &status, 0);
			}
			else if(returnedPID == firstPid)
			{
				waitpid(secondPid, &status, 0);
			}
		}
	}

}

void
ORExecuter (command_t input){
	int exitStatus;
	int p = fork();
    if(p==0)
	{
    	execute_command(input->u.command[0], false);
    	exit(input->u.command[0]->status);
	}
	else
	{
		int status;
		waitpid(p, &status, 0);
		exitStatus = WEXITSTATUS(status);
		if(exitStatus != 0)
		{
			execute_command(input->u.command[1], false);
			input->status = input->u.command[1]->status;
			return;
		}
	}
}

void
SIMPLEExecuter (command_t input){
	input->u.word[input->words] = malloc(sizeof(char));
	input->u.word[input->words][0] = '\0';
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
    	execute_command(input->u.command[0], false);
    	exit(input->u.command[0]->status);
	}
	else
	{
		int status;
		waitpid(p, &status, 0);
		execute_command(input->u.command[1], false);
		input->status = input->u.command[1]->status;
		return;
	}
}
