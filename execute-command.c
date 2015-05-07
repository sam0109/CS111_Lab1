// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
	//Takes in a command and then sends command to appropriate handler
	//Verifies command is a real command as well (not NULL)
	if(c==NULL)
	{
		return;
	}
	
	if(!time_travel || time_travel)
	{
		switch(c->type)
		{
			case AND_COMMAND:
				ANDExecuter(c);
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
				SUBSHELLExecuter(c);
				break;
		}
	}
}

void 
PIPEExecuter(command_t input)
{
	//First create the pipe and back up stdin and stdout
	int fd[2];
	pipe(fd);
	int saved_stdin = dup(0);
	int saved_stdout = dup(1);
	int firstPid = fork(); // execute right command
	if(firstPid == 0)
	{
		close(fd[1]); // close unused write end
		dup2(fd[0], 0); //Set pipe input
		execute_command(input->u.command[1], false); //execute command
		exit(input->u.command[1]->status); //exit process
	}
	else
	{
		int secondPid = fork();
		if(secondPid == 0)
		{   //Do other side of pipe (same as above)
			close(fd[0]); // close unused write end
			dup2(fd[1], 1);
			execute_command(input->u.command[0], false);
			exit(input->u.command[0]->status);
		}
		else
		{   //In parent wait for pipe to finish then restore I/O
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
			
			int exitStatus = WEXITSTATUS(status);
			input->status = exitStatus;
			
			dup2(saved_stdin, 0);
			dup2(saved_stdout, 1);
		}
	}

}

void
ORExecuter (command_t input)
{   //Fork from the parent, have parent continue perform left side
	int exitStatus;
	int p = fork();
    if(p==0)
	{
    	execute_command(input->u.command[0], false);
    	exit(input->u.command[0]->status);
	}
	else
	{   //let child wait for parent to die then continue and do right side or short circuit
		int status;
		waitpid(p, &status, 0);
		exitStatus = WEXITSTATUS(status);
		if(exitStatus != 0)
		{
			execute_command(input->u.command[1], false);
			input->status = input->u.command[1]->status;
			return;
		}
		else
		{
			input->status = exitStatus;
			return;
		}
	}
}

void
ANDExecuter (command_t input)
{
	//Same thought process as ORExecutor
	int exitStatus;
	int p = fork();
    if(p==0)
	{
		//Have parent execute left and then die
    	execute_command(input->u.command[0], false);
    	exit(input->u.command[0]->status);
	}
	else
	{
		//Let child wait for parent to die then execute right or short circuit
		int status;
		waitpid(p, &status, 0);
		exitStatus = WEXITSTATUS(status);
		if(exitStatus == 0)
		{
			execute_command(input->u.command[1], false);
			input->status = input->u.command[1]->status;
			return;
		}
		else
		{
			input->status = exitStatus;
			return;
		}
	}
}

void
SIMPLEExecuter (command_t input)
{
	//Set the last word equal to the null byte
	input->u.word[input->words] = malloc(sizeof(char*));
	
	char* end = {'\0'};
	
	input->u.word[input->words]= end;
	int saved_stdout, saved_stdin = 0;

	//Check to see if any redirects
	//If so adjust the stream and backup old I/O
	if(input->input != NULL)
	{
		saved_stdin = dup(0);
		int fd = open (input->input, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		if(fd<0) fd = 0;
		dup2(fd, 0);
	}

	if(input->output != NULL)
	{
		saved_stdout = dup(1);
		int fd = open (input->output, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		if(fd<0) fd = 1;
		dup2(fd, 1);
	}

	//Fork the process so that way executing the command won't kill the main process
	int p = fork();
    if(p==0)
	{
    	if(input->u.word[0] != NULL &&  input->u.word[0][0] == 'e' 
    								&&	input->u.word[0][1] == 'x' 
    								&&	input->u.word[0][2] == 'e' 
    								&&	input->u.word[0][3] == 'c')
    	{
    		//If exec is called handle special case
    		execvp(input->u.word[1], &input->u.word[1]);
    	}
    	else
    	{
    		//Otherwise just run main command
    		execvp(input->u.word[0], input->u.word);
    	}
	}
	else
	{
		//Wait for parent to die and let child carry on the torch
		int status;
		waitpid(p, &status, 0);
		input->status = WEXITSTATUS(status);
	}

	//Fix the I/O stream
    if(input->output != NULL)
    {
    	dup2(saved_stdout, 1);
    	close(saved_stdout);
    }

    if(input->input != NULL)
    {
     	dup2(saved_stdin, 0);
       	close(saved_stdin);
    }

    return;
}

void
SEQUENCEExecuter (command_t input)
{
	//Executes the left and right side of a sequence command
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
		
		if(input->u.command[1] != NULL)
		{	//exits with status of last command run in sequence
			execute_command(input->u.command[1], false);
			input->status = input->u.command[1]->status;
		}
		else
		{
			input->status = WEXITSTATUS(status);
		}
		
		return;
	}
}

void
SUBSHELLExecuter(command_t input)
{
	//Check and Adjust for any redirect
	//Save I/O
	int saved_stdout, saved_stdin = 0;


	if(input->input != NULL)
	{
		saved_stdin = dup(0);
		int fd = open (input->input, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		if(fd<0) fd = 0;
		dup2(fd, 0);
	}

	if(input->output != NULL)
	{
		saved_stdout = dup(1);
		int fd = open (input->output, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		if(fd<0) fd = 1;
		dup2(fd, 1);
	}

	//Execute command inside of subsheel
	execute_command(input->u.subshell_command, false);

	//Fix any I/O
    if(input->output != NULL)
    {
    	dup2(saved_stdout, 1);
    	close(saved_stdout);
    }

    if(input->input != NULL)
    {
     	dup2(saved_stdin, 0);
       	close(saved_stdin);
    }
}
