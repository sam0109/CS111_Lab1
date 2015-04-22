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

