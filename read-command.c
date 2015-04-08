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

typedef struct command_stream
{
    struct command_node *head;
    struct command_node *tail;
    struct command_node* cursor;
};

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












