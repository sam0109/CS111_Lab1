// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

//A linked list for command trees
struct command_node
{
    struct command* command; //stores root of a command tree
    struct command_node* next; //
};

struct stack
{
	struct stack* down;
	command_t command;
};

typedef struct command_stream
{
    struct command_node *head;
    struct command_node *tail;
    struct command_node* cursor;
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
	struct command_stream* s = malloc(sizeof(struct command_stream));
	
    char c = NULL;
    int bufferSize = 100;
    char* buffer = (char*)malloc(bufferSize*sizeof(char));
    int n = 0;

    int isCommented = 0;
	int newLineCount = 0;
	int inSubshell = 0;
	
	int andCount = 0;
	int commandCount = 0;

    while ((c = get_next_byte(get_next_byte_argument)) != EOF)
    {
        if((n+1) == bufferSize)
        {
			//n+2 necessary to handle special cases
            bufferSize = bufferSize * 10;
            buffer = (char*)realloc(buffer, bufferSize*sizeof(char));
        }

        if( c == '#')
        {
            isCommented = 1;
        }
        else if ( c == '\n')
        {
            if(n == 0)
            {
                continue;
            }
            
            if(isCommented == 1)
            {
                isCommented = 0;
            }
            
            if(andCount == 1)
            {
            	//TODO
            	fprintf(stderr,": & cannot be its own line \n");
				exit(0);
            }
            
            switch(buffer[n-1])
            {
                case '&':
					if( n - 2 >= 0)
					{
						if(buffer[n-2] != '&')
						{
							newLineCount++;
						}
					}
                    break;
                case '|':
                    break;
				case '<':
					//TODO
					fprintf(stderr,": No new line after < \n");
    				exit(0);
					break;
				case '>':
					//TODO
					fprintf(stderr,"': No new line after >\n");
    				exit(0);
					break;
				default:
					if( inSubshell == 0)
					{
						newLineCount++;
					}
					break;
            }
            
            if(newLineCount == 2)
            {
            	newLineCount = 0;
            	buffer[n] = '\0';
            	
            	struct command_node* cn = malloc(sizeof(struct command_node));
            	cn->command = generate_command_tree(buffer);
            	cn->next = NULL;
            	
            	if(commandCount == 0)
            	{
            		s->head= cn;
            		s->cursor = s->head;
            		s->tail = cn;
            	}
            	else
            	{
            		s->tail->next = cn;
            		s->tail = s->tail->next;
            	}
            	
            	commandCount++;
            	n = 0;
            }
        }
        else
        {
            if(isCommented == 0)
            {
				if(c == '(')
				{
					inSubshell = 1;
				}
				else if( c == ')')
				{
					inSubshell = 0;
				}
				
				if(newLineCount == 1)
				{
					//Determines if new Line is a complete command
					if(c != '|' || c != '<'|| c != '>' || c != ')' || c != ';')
					{
						if(c == '&')
						{
							//TODO this case needs work!!!
							andCount++;
							
							if(andCount == 2)
							{
								buffer[n] = '&';
								n++;
								newLineCount = 0;
								andCount = 0;
							}
						}
						else
						{
							if(andCount == 1)
							{
								buffer[n] = ';';
								n++;
								buffer[n] = '&';
								n++;
								newLineCount = 0;
								andCount = 0;
							}
							else if(n > 0 && buffer[n-1] != ';')
							{
								newLineCount = 0;
								buffer[n] = ';';
								n++;
							}
						}
					}
				}
				
                buffer[n] = c;
                n++;
            }
        }
    }
    
    if(n > 0)
    {
    	buffer[n] = '\0';
    	
    	struct command_node* cn = malloc(sizeof(struct command_node));
    	cn->command = generate_command_tree(buffer);
    	cn->next = NULL;
    	
    	if(commandCount == 0)
    	{
    		s->head= cn;
    		s->cursor = s->head;
    		s->tail = cn;
    	}
    	else
    	{
    		s->tail->next = cn;
    		s->tail = s->tail->next;
    	}
    	
    	commandCount++;
    }

    return s;
}

command_t
read_command_stream (command_stream_t s)
{
    if(s->cursor == NULL)
    {
    	return NULL;
    }
    
    struct command_node* temp = s->cursor;
    s->cursor = s->cursor->next;
    
    return temp->command;
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
				int max_size_of_command = 30;
				new_command->u.word = malloc(max_size_of_command * sizeof(char*));
				new_command->type = SIMPLE_COMMAND;
				new_command->status = -1;
				int j = 0;
				do
				{
					//if(size_of_command >= max_size_of_command - 1){
					//	max_size_of_command += 3;
					//	realloc(new_command->u.word, max_size_of_command * sizeof(char*));
					//}
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












