// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  
*/

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  
*/

command_stream_t
make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{
    /* FIXME: Replace this with your implementation.  You may need to
       add auxiliary functions and otherwise modify the source code.
       You can also use external functions defined in the GNU C Library.  
    */

    error (1, 0, "command reading not yet implemented");
    return 0;
}

command_t
read_command_stream (command_stream_t s)
{
    /* FIXME: Replace this with your implementation too.  */

    error (1, 0, "command reading not yet implemented");
    return 0;
}


char*
scan_to_next_word(char* start)
{
	
}
command_t
generate_command_tree (char* input_string)
{
/*
	1)if a simple command, push it onto the command stack
	2)if "(", push it onto the operator stack
	3)if an operator and operator stack is empty
	     a)push it onto the operator stack
	4)if an operator and operator stack is not empty
    	 a)pop all operators with >= precidence off operator stack 
    	 b)for each operator, pop 2 commands off command stacks
    	 c)combine new command and put it on command stack
    	 d)stop when reach and operator with lower precidence or a "("
    	 e)push new operator onto operator stack
	5)if encounter ")"
    	 a)pop operators off stack like 4a until see a matching "("
    	 b)create subshell command by popping top command from command stack
    	 c)push new command to command stack
	6)Back to 2 if stuff remains
	7)else pop remaining operators like 4a
	*/
    return 0;
}












