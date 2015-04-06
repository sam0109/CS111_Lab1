// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

//A linked list for command trees
struct command_node
{
    struct command* command; //stores root of a command tree
    struct command_node* next;
    struct command_node* cursor;
};

struct stack
{
	struct stack* down;
	command_t command;
};

typedef struct command_stream
{
    
};

struct stack*
pop(struct stack* head)
{
	struct stack* temp = head;
	head = head->down;
	return temp;
}

void
push(struct stack* head, command_t new_command)
{
	struct stack* new_head = malloc(sizeof(struct stack));
	new_head.command = new_command;
	new_head->down = head;
	head = new_head;
}

void
push_new_command(struct stack* head, enum command_type in_command)
{
	struct command new_command = malloc(sizeof(struct command));
	new_command.type = AND_COMMAND;
	new_command.status = -1;
	push(head, new_command);
}

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

// a function to find the next word in the command
int
scan_to_next_word(char* start, int beginning_of_next_word, enum word_type word) //return the number of characters in the next word (return 4 for "this")
{																			// and find the beginning of the next word
	return 0;
}

command_t
pop_and_combine(command_t command, struct stack operators, struct stack commands)
{
	return 0;
}

command_t
generate_command_tree (char* input_string)
{
	enum command_type word;
	struct stack operators = malloc(sizeof(struct stack));
	operators.command = 0;
	struct stack commands = malloc(sizeof(struct stack));
	commands.command = 0;
	int beginning_of_next_word = 0;
	int end_of_next_word = scan_to_next_word(input_string[beginning_of_next_word], &beginning_of_next_word, &word);
	while(end_of_next_word != beginning_of_next_word)
	{
		switch(word)
		{
			case SIMPLE_COMMAND:
				struct command new_command = malloc(sizeof(struct command));
				new_command.type = SIMPLE_COMMAND;
				new_command.status = -1;
				int j = 0;
				do
				{
					int word_len = end_of_next_word - beginning_of_next_word;
					new_command.u->word[j] = malloc(word_len + 1);
					for (int i = 0; i < word_len; i++)
					{
						new_command.u->word[j][i] = input_string[beginning_of_next_word + i];
					}
					new_command.u->word[j][word_len] = '\0';
					j++;
					end_of_next_word = scan_to_next_word(input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				}while(word == SIMPLE_COMMAND);
				push(commands, new_command);
				break;
			case SUBSHELL_COMMAND:
				if(input_string[beginning_of_next_word] == '(')
				{
					struct command new_command = malloc(sizeof(struct command));
					new_command.type = SUBSHELL_COMMAND;
					new_command.status = -1;
					end_of_next_word = scan_to_next_word(input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				}
				else
				{
					pop_and_combine();
				}
				break;
			case AND_COMMAND:
				if(operators.command == 0)
				{
					push_new_command(operators, AND_COMMAND);
				}
				break;
			case SEQUENCE_COMMAND:
				if(operators.command == 0)
				{
					push_new_command(operators, SEQUENCE_COMMAND);
				}
				break;
			case OR_COMMAND:
				if(operators.command == 0)
				{
					push_new_command(operators, OR_COMMAND);
				}
				break;
			case PIPE_COMMAND:
				if(operators.command == 0)
				{
					push_new_command(operators, PIPE_COMMAND);
				}
				break;
			default:
				break;
		}
		char* beginning_of_next_word = end_of_next_word;
		char* end_of_next_word = scan_to_next_word(end_of_next_word, *word);
	}
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












