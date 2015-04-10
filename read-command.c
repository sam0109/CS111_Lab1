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

struct command_stream
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
pop(struct stack **head)
{
	if(head[0]->command == 0)
		return 0;
	struct stack *temp = *head;
	head[0] = head[0]->down;
	command_t command = temp->command;
	return command;
}

void
push(struct stack **head, command_t new_command)
{
	struct stack* new_head = malloc(sizeof(struct stack));
	new_head->command = new_command;
	new_head->down = *head;
	*head = new_head;
	return;
}

void
push_new_command(struct stack **head, enum command_type in_command)
{
	command_t new_command = malloc(sizeof(struct command));
	new_command->type = in_command;
	new_command->status = -1;
	new_command->u.subshell_command = 0;
	push(head, new_command);
	return;
}

int 
is_valid_character(char c)
{
	return (isalpha(c) || isdigit(c) || c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '#'
	   			                     || c == '^' || c == '_' || c == '.' || c == '/' || c == ':' || c == '@');
}

int 
is_valid_operator(char c)
{
	return (c == ';' || c == '|' || c == '&' || c == '(' || c == ')' || c == '<' || c == '>');
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
	
	int lineNum = 0;

    while ((c = get_next_byte(get_next_byte_argument)) != EOF)
    {
    	if(!is_valid_character(c) && !isCommented && c != ' ' && !is_valid_operator(c) && c != '\n' && c != '\t')
    	{
			fprintf(stderr,"%d: %c is not a valid character\n", lineNum, c);
			exit(1);
    	}
    	
    	if(c == ' ' || c == '\t')
    	{
    		//Eats white space after operator
			if(n == 0 || is_valid_operator(buffer[n-1]) || buffer[n-1] == ' ')
    		{
    			continue;
    		}
    	}
    	
    	if(n == 0)
    	{
    		if(is_valid_operator(c))
    		{
    			fprintf(stderr,"%d: %c operator cannot start line\n", lineNum, c);
				exit(1);
    		}
    	}
    	
    	if(is_valid_operator(c) && n > 0)
		{
			if(buffer[n-1] == '<' || buffer[n-1] == '>')
			{
				fprintf(stderr,"%d: %c cannot follow redirect\n", lineNum, c);
				exit(1);
			}
		}
    	
    	if(c == ';' && n > 0)
    	{
    		if(buffer[n-1] == ';')
    		{
    			fprintf(stderr,"%d: cannot have two %c consecutively\n", lineNum, c);
				exit(1);
    		}
    	}
    	
        if((n+1) == bufferSize)
        {
			//n+2 necessary to handle special cases
            bufferSize = bufferSize * 10;
            buffer = (char*)realloc(buffer, bufferSize*sizeof(char));
        }

		if(c == '&' && andCount == 0)
		{
			andCount++;
		}
		else if(andCount == 1 && c != '&')
		{
			fprintf(stderr,"%d: Cannot have single &\n", lineNum);
			exit(1);
		}
		else if(andCount == 1 && c == '&')
		{
			andCount = 0;
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
            
            switch(buffer[n-1])
            {
                case '&':
                    break;
                case '|':
                    break;
				case '<':
					//Fall to next case
				case '>':
					fprintf(stderr,"%d: new line cannot be directly after %c\n", lineNum, buffer[n-1]);
					exit(1);
					break;
				default:
					newLineCount++;
					if(newLineCount == 1)
					{
						buffer[n] = ';';
						n++;
					}
					break;
            }
            
            if(newLineCount == 2)
            {
            	newLineCount = 0;
            	buffer[n] = '\0';
            	n = 0;
            	
            	if(inSubshell != 0)
            	{
					fprintf(stderr,"%d: missing either a ( or )\n", lineNum);
					exit(1);
            	}
            	else
            	{
		        	struct command_node* cn = malloc(sizeof(struct command_node));
		        	cn->command = generate_command_tree(buffer);
		        	
		        	if(cn->command == NULL)
		        	{
		        		fprintf(stderr,"%d: check number of operators to operands\n", lineNum);
		        		exit(1);
		        	}
		        	
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
            }
            
            lineNum++;
        }
        else
        {
            if(isCommented == 0)
            {
				if(c == '(')
				{
					inSubshell++;
				}
				else if( c == ')')
				{
					inSubshell--;
				}
				

				if(newLineCount == 1)
				{
					//Only (,), and simple commands can follow a new line
					if(!is_valid_character(c) && c != '(' && c != ')')
					{
						fprintf(stderr,"%d: New line must start with either a (,), or simple command\n", lineNum);
						exit(1);
					}
				}
				
                buffer[n] = c;
                n++;
            }
        }
    }
    
    if(n > 0)
    {
    	if(inSubshell != 0)
    	{
			fprintf(stderr,"%d: missing either a ( or )\n", lineNum);
			exit(1);
    	}
    
    	if((n+1) == bufferSize)
        {
			//n+2 necessary to handle special cases
            bufferSize = bufferSize * 10;
            buffer = (char*)realloc(buffer, bufferSize*sizeof(char));
        }
        
        if(buffer[n-1] != ';')
        {
        	buffer[n] = ';';
        	n++;
        }
        
    	buffer[n] = '\0';
    	
    	struct command_node* cn = malloc(sizeof(struct command_node));
    	cn->command = generate_command_tree(buffer);
    	
    	if(cn->command == NULL)
    	{
    		fprintf(stderr,"%d: check number of operators to operands\n", lineNum);
    		exit(1);
    	}
    	
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
	
	free(buffer);
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
scan_to_next_word(char *string, int *beginning_of_next_word, enum command_type *word) 	//return the character number of the next character (return 4 for "this is a test")
{																						// and find the beginning of the next word
	while(string[*beginning_of_next_word] != '\0' && (string[*beginning_of_next_word] == ' ' || string[*beginning_of_next_word] == '>' || string[*beginning_of_next_word] == '<')) 	// get rid of leading spaces
	{
		*beginning_of_next_word = *beginning_of_next_word + 1;
	}
	if(string[*beginning_of_next_word] == '|' && string[*beginning_of_next_word + 1] == '|')	//check to see if it's an operator
	{
		*word = OR_COMMAND;
		return *beginning_of_next_word + 1;
	}
	if(string[*beginning_of_next_word] == '&' && string[*beginning_of_next_word + 1] == '&')
	{
		*word = AND_COMMAND;
		return *beginning_of_next_word + 1;
	}
	if(string[*beginning_of_next_word] == '|')
	{
		*word = PIPE_COMMAND;
		return *beginning_of_next_word + 1;
	}
	if(string[*beginning_of_next_word] == '(' || string[*beginning_of_next_word] == ')')
	{
		*word = SUBSHELL_COMMAND;
		return *beginning_of_next_word + 1;
	}
	if(string[*beginning_of_next_word] == ';')
	{
		*word = SEQUENCE_COMMAND;
		return *beginning_of_next_word + 1;
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
	if(a == 0 || b == 0)
		return false;
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

int
pop_and_combine(command_t command, struct stack *operators, struct stack *commands)
{
	while(operator_test(command, operators->command))
	{
		command_t popped_operator = pop(&operators);
		popped_operator->u.command[1] = pop(&commands);
		popped_operator->u.command[0] = pop(&commands);
		if((popped_operator->type == AND_COMMAND ||
		    popped_operator->type == OR_COMMAND ||
		    popped_operator->type == PIPE_COMMAND)&&(
		    popped_operator->u.command[1] == 0||
		    popped_operator->u.command[0] == 0) )
		{
			return 1;
		}
		push(&commands, popped_operator);
	}
	if(command->type == SUBSHELL_COMMAND && operators->command->type == SUBSHELL_COMMAND)
	{
		free_command_t_recursive(pop(&operators));
		command->u.subshell_command = pop(&commands);
		push(&commands, command);
	}
	else
	{
		push(&operators, command);
	}
	return 0;
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
	int end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
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
					if(input_string[end_of_next_word + 1] == '<' || input_string[end_of_next_word] == '<')
					{
						next_word_is_input = true;
					}
					else if(input_string[end_of_next_word + 1] == '>' || input_string[end_of_next_word] == '>')
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
					beginning_of_next_word = end_of_next_word;
					end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
				}while(word == SIMPLE_COMMAND && end_of_next_word != beginning_of_next_word);
				if(next_word_is_input || next_word_is_output)
					return 0;
				new_command->words = size_of_command;
				push(&commands, new_command);
				break;

			case SUBSHELL_COMMAND:
				if(input_string[beginning_of_next_word] == '(')
				{
					push_new_command(&operators, SUBSHELL_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = SUBSHELL_COMMAND;
					new_command->status = -1;
					new_command->u.subshell_command = 0;
					if(pop_and_combine(new_command, operators, commands))
						return 0;
				}
				beginning_of_next_word = end_of_next_word;
				end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
				break;

			case AND_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(&operators, AND_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = AND_COMMAND;
					new_command->status = -1;
					if(pop_and_combine(new_command, operators, commands))
						return 0;
				}
				beginning_of_next_word = end_of_next_word;
				end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
				break;

			case SEQUENCE_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(&operators, SEQUENCE_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = SEQUENCE_COMMAND;
					new_command->status = -1;
					if(pop_and_combine(new_command, operators, commands))
						return 0;
				}
				beginning_of_next_word = end_of_next_word;
				end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
				break;

			case OR_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(&operators, OR_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = OR_COMMAND;
					new_command->status = -1;
					if(pop_and_combine(new_command, operators, commands))
						return 0;
				}
				beginning_of_next_word = end_of_next_word;
				end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
				break;

			case PIPE_COMMAND:
				if(operators->command == 0)
				{
					push_new_command(&operators, PIPE_COMMAND);
				}
				else
				{
					command_t new_command = malloc(sizeof(struct command));
					new_command->type = PIPE_COMMAND;
					new_command->status = -1;
					if(pop_and_combine(new_command, operators, commands))
						return 0;
				}
				beginning_of_next_word = end_of_next_word;
				end_of_next_word = scan_to_next_word(input_string, &beginning_of_next_word, &word);
				break;
			default:
				break;
		}
	}
	if(pop_and_combine(pop(&operators), operators, commands))
		return 0;
	return pop(&commands);
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












