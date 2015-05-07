// UCLA CS 111 Lab 1 command interface

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

//A linked list for command trees
struct command_node
{
    struct command* command; //stores root of a command tree
    struct command_node* next; //
};

struct command_stream
{
    struct command_node *head;
    struct command_node *tail;
    struct command_node* cursor;
};

typedef struct graphNode GraphNode;

struct graphNode
{
     command_t command;
     GraphNode** before;
     int num_dependencies;
     int max_dependencies;
     char** read_list;
     int read_size;
     int max_read_size;
     char** write_list;
     int write_size;
     int max_write_size;
     pid_t pid; //initialized to 1
};

typedef struct queue Queue;

struct queue
{
	GraphNode* node;
	Queue* next;
};

typedef struct dependencyGraph DependencyGraph;

struct dependencyGraph
{
     Queue* no_dependencies;
     Queue* no_dependencies_tail;
     int size_no_dependencies;
     Queue* dependencies;
     Queue* dependencies_tail;
     int size_dependencies;
};


/* Create a command stream from GETBYTE and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the flag is set.  */
void execute_command (command_t, bool);

/* Return the exit status of a command, which must have previously
   been executed.  Wait for the command, if it is not already finished.  */
int command_status (command_t);

command_t generate_command_tree (char* input_string);

int is_valid_character(char c);

int is_valid_operator(char c);

void ORExecuter (command_t input);

void SIMPLEExecuter (command_t input);

void PIPEExecuter(command_t input);

void SEQUENCEExecuter (command_t input);

void ANDExecuter (command_t input);

void SUBSHELLExecuter(command_t input);

bool check_dependencies(GraphNode* a, GraphNode* b);

DependencyGraph* create_dependency_graph(command_stream_t stream);
