// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdlib.h>

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

void
free_command_t_recursive(command_t to_free)
{
	free(to_free->input);
	free(to_free->output);
	free_command_t_recursive(to_free->u.command[0]);
	free_command_t_recursive(to_free->u.command[1]);
	free_command_t_recursive(to_free->u.subshell_command);
	int n = to_free->words - 1;
	while(n >= 0)
	{
		free(to_free->u.word[n]);
		n--;
	}
}

command_t
pop(struct stack *head)
{
	struct stack* temp = head;
	head = head->down;
	command_t command = temp->command;
	free(temp);
	return command;
}

void
push(struct stack *head, command_t new_command)
{
	struct stack* new_head = malloc(sizeof(struct stack));
	new_head->command = new_command;
	new_head->down = head;
	head = new_head;
	return;
}

void
push_new_command(struct stack* head, enum command_type in_command)
{
	command_t new_command = malloc(sizeof(struct command));
	new_command->type = in_command;
	new_command->status = -1;
	new_command->u.subshell_command = 0;
	push(head, new_command);
	return;
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
scan_to_next_word(char *string, int *beginning_of_next_word, enum command_type *word) 	//return the number of characters in the next word (return 4 for "this is a test")
{																						// and find the beginning of the next word
	while(string[*beginning_of_next_word] != '\0' && string[*beginning_of_next_word] == ' ') 	// get rid of leading spaces
	{
		beginning_of_next_word++;
	}
	if(string[*beginning_of_next_word] == '|' && string[*beginning_of_next_word + 1] == '|')	//check to see if it's an operator
	{
		*word = OR_COMMAND;
		return 1;
	}
	if(string[*beginning_of_next_word] == '&' && string[*beginning_of_next_word + 1] == '&')
	{
		*word = AND_COMMAND;
		return 1;
	}
	if(string[*beginning_of_next_word] == '|')
	{
		*word = PIPE_COMMAND;
		return 1;
	}
	if(string[*beginning_of_next_word] == '(' || string[*beginning_of_next_word] == ')')
	{
		*word = SUBSHELL_COMMAND;
		return 1;
	}
	if(string[*beginning_of_next_word] == ';')
	{
		*word = SEQUENCE_COMMAND;
		return 1;
	}
	*word = SIMPLE_COMMAND;  //otherwise it's a simple command

	int current_char = *beginning_of_next_word;
	while(string[current_char] != '\0' &&
		  string[current_char] != ' ' &&
		  string[current_char] != '(' &&
		  string[current_char] != ')' &&
		  string[current_char] != ';' &&
		  string[current_char] != '|' &&
		  string[current_char] != '&' &&
		  string[current_char] != '<' &&
		  string[current_char] != '>'
		 )
	{
		current_char++;
	}
	return current_char;
}

bool
operator_test(command_t a, command_t b)
{
	if(a->type == SUBSHELL_COMMAND && b->type != SUBSHELL_COMMAND)
		return true;
	if(b->type == SUBSHELL_COMMAND && b->u.subshell_command == 0)
		return false;
	if(a->type == SEQUENCE_COMMAND)
		return true;
	if(b->type == SEQUENCE_COMMAND)
		return false;
	if(a->type == OR_COMMAND || a->type == AND_COMMAND)
		return true;
	if(b->type == OR_COMMAND || b->type == AND_COMMAND)
		return false;
	return true;
}

void
pop_and_combine(command_t command, struct stack *operators, struct stack *commands)
{
	while(operator_test(command, operators->command))
	{
		command_t popped_operator = pop(operators);
		popped_operator->u.command[1] = pop(commands);
		popped_operator->u.command[0] = pop(commands);
		push(commands, popped_operator);
	}
	if(command->type == SUBSHELL_COMMAND && operators->command->type == SUBSHELL_COMMAND)
	{
		free_command_t_recursive(pop(operators));
		command->u.subshell_command = pop(commands);
		push(commands, command);
	}
	else
	{
		push(operators, command);
	}
	return;
	/*
	   	 	 a)pop all operators with >= precidence off operator stack
	    	 b)for each operator, pop 2 commands off command stacks
	    	 c)combine new command and put it on command stack
	    	 d)stop when reach and operator with lower precidence or a "("
	    	 e)push new operator onto operator stack
		 */
}

command_t
generate_command_tree (char *input_string)
{
	enum command_type word;
	bool next_word_is_input = false;
	bool next_word_is_output = false;
	struct stack* operators = malloc(sizeof(struct stack));
	operators->command = 0;
	struct stack* commands = malloc(sizeof(struct stack));
	commands->command = 0;
	int beginning_of_next_word = 0;
	int end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
	while(end_of_next_word != beginning_of_next_word)
	{
		switch(word)
		{
			case SIMPLE_COMMAND: ;
				command_t new_command = malloc(sizeof(struct command));
				int size_of_command = 0;
				int max_size_of_command = 3;
				new_command->u.word = malloc(max_size_of_command);
				new_command->type = SIMPLE_COMMAND;
				new_command->status = -1;
				int j = 0;
				do
				{
					if(size_of_command >= max_size_of_command - 1){
						max_size_of_command += 3;
						realloc(new_command->u.word, max_size_of_command);
					}
					int word_len = end_of_next_word - beginning_of_next_word;
					int i = 0;
					if(next_word_is_input)
					{
						new_command->input = malloc(word_len + 1);
						while(i < word_len)
						{
							new_command->input[i] = input_string[beginning_of_next_word + i];
							i++;
						}
						new_command->input[word_len] = '\0';
						next_word_is_input = false;
					}
					if(next_word_is_output)
					{
						new_command->output = malloc(word_len + 1);
						while(i < word_len)
						{
							new_command->output[i] = input_string[beginning_of_next_word + i];
							i++;
						}
						new_command->output[word_len] = '\0';
						next_word_is_output = false;
					}
					if(input_string[beginning_of_next_word] == '<')
					{
						next_word_is_input = true;
					}
					else if(input_string[beginning_of_next_word] == '>')
					{
						next_word_is_output = true;
					}
					else
					{
						new_command->u.word[j] = malloc(word_len + 1);
						while(i < word_len)
						{
							new_command->u.word[j][i] = input_string[beginning_of_next_word + i];
							i++;
						}
						new_command->u.word[j][word_len] = '\0';
					}
					j++;
					size_of_command++;
					end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				}while(word == SIMPLE_COMMAND);
				new_command->words = size_of_command;
				push(commands, new_command);
				break;

			case SUBSHELL_COMMAND:
				if(input_string[beginning_of_next_word] == '(')
				{
					push_new_command(operators, SUBSHELL_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = SUBSHELL_COMMAND;
					new_command->status = -1;
					new_command->u.subshell_command = 0;
					pop_and_combine(new_command, operators, commands);
				}
				end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				break;

			case AND_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(operators, AND_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = AND_COMMAND;
					new_command->status = -1;
					pop_and_combine(new_command, operators, commands);
				}
				end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				break;

			case SEQUENCE_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(operators, SEQUENCE_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = SEQUENCE_COMMAND;
					new_command->status = -1;
					pop_and_combine(new_command, operators, commands);
				}
				end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				break;

			case OR_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(operators, OR_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = OR_COMMAND;
					new_command->status = -1;
					pop_and_combine(new_command, operators, commands);
				}
				end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				break;

			case PIPE_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(operators, PIPE_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = PIPE_COMMAND;
					new_command->status = -1;
					pop_and_combine(new_command, operators, commands);
				}
				end_of_next_word = scan_to_next_word(&input_string[beginning_of_next_word], &beginning_of_next_word, &word);
				break;
			default:
				break;
		}
		pop_and_combine(pop(operators), operators, commands);
		return pop(commands);
	}
/*
	1)if a simple command, push it onto the command stack
	2)if "(", push it onto the operator stack
	3)if an operator and operator stack is empty
	     a)push it onto the operator stack
	4)if an operator and operator stack is not empty
		pop_and_combine
	5)if encounter ")"
    	 a)pop operators off stack like 4a until see a matching "("
    	 b)create subshell command by popping top command from command stack
    	 c)push new command to command stack
	6)Back to 2 if stuff remains
	7)else pop remaining operators like 4a
	*/
    return 0;
}












