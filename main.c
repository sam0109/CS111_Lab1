// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>

#include "command.h"
#include "command-internals.h"

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

void generate_read_write_lists(GraphNode* node, command_t command)
{
	//We want to generate lists of all read and writes of a command tree
	 if (command->type == SIMPLE_COMMAND)
	 {
	 	//Add all files that are being read which can potentially be dependencies
	 	if(command->input)
	 	{
	 		if(node->read_size >= node->max_read_size)
	 		{
	 			node->max_read_size += 100;
	 			node->read_list = (char**)realloc(node->read_list, node->max_read_size*sizeof(char*));
	 		}
	 		node->read_list[node->read_size] = command->input;
	 		node->read_size = node->read_size + 1;
	 	}
	 	int i = 1;
	 	while(i < command->words)
	 	{
	 		if(command->u.word[i] != NULL && command->u.word[i][0] != '-')
	 		{
	 			if(node->read_size >= node->max_read_size)
	 			{
	 				node->max_read_size += 100;
	 				node->read_list = (char**)realloc(node->read_list, node->max_read_size*sizeof(char*));
	 			}	 			
	 			node->read_list[node->read_size] = command->u.word[i];
	 			node->read_size = node->read_size + 1;
	 		}
	 		i++;
	 	}
	 	if(command->output)
	 	{
	 		//Add all outputs
	 		if(node->write_size >= node->max_write_size)
	 		{
	 			node->max_write_size += 100;
	 			node->write_list = (char**)realloc(node->write_list, node->max_write_size*sizeof(char*));
	 		}
	 		node->write_list[node->write_size] = command->output;
	 		node->write_size = node->write_size + 1;
	 	}
     }
     else if (command->type == SUBSHELL_COMMAND)
     {
     	//Repeat for subshell command because also can have dependencies
     	if(command->input)
	 	{
	 		if(node->read_size >= node->max_read_size)
	 		{
	 			node->max_read_size += 100;
	 			node->read_list = (char**)realloc(node->read_list, node->max_read_size*sizeof(char*));
	 		}
	 		node->read_list[node->read_size] = command->input;
	 		node->read_size = node->read_size + 1;
	 	}
     	if(command->output)
	 	{
	 		if(node->write_size >= node->max_write_size)
	 		{
	 			node->max_write_size += 100;
	 			node->write_list = (char**)realloc(node->write_list, node->max_write_size*sizeof(char*));
	 		}
	 		node->write_list[node->write_size] = command->output;
	 		node->write_size = node->write_size + 1;
	 	}
	 	if(command->u.subshell_command)
     		generate_read_write_lists(node,command->u.subshell_command);
     }
     else
     {
     	//Recursively traverse the command tree
     	if(command->u.command[0])
     		generate_read_write_lists(node,command->u.command[0]);
     	if(command->u.command[1])
     		generate_read_write_lists(node,command->u.command[1]);
     }
}

GraphNode* generate_GraphNode(command_t command)
{
	//Create a GraphNode and get all of its guts initialized in useful state
	GraphNode* return_graph = malloc(sizeof(GraphNode));
	return_graph->max_read_size = 100;
	return_graph->read_size = 0;
	return_graph->max_write_size = 100;
	return_graph->write_size = 0;
	return_graph->read_list = malloc(sizeof(char*) * return_graph->max_read_size);
	return_graph->write_list = malloc(sizeof(char*) * return_graph->max_write_size);
	return_graph->command = command;
	generate_read_write_lists(return_graph,command);
	return return_graph;
}

bool check_dependencies( GraphNode* a, GraphNode* b)
{
	//This function checks read and write list to determine dependencies
	int i = 0;
	int j = 0;
	int k = 0;
	while(i < a->write_size )
	{
		while(j < b->write_size)
		{
			//Check to see if there are any matches in write and write
			bool match = true;
			while(a->write_list[i][k] != '\0' && b->write_list[j][k] != '\0')
			{
				if(a->write_list[i][k] != b->write_list[j][k])
					match = false;
				k++;
			}
			if(match == true && a->write_list[i][k] == b->write_list[j][k])
			{
				return true;
			}
			k = 0;
			j++;
		}
		i++;
	}
	i = 0;
	j = 0;
	while(i < a->read_size )
	{
		while(j < b->write_size)
		{
			//check for read and write
			bool match = true;
			while(a->read_list[i][k] != '\0' && b->write_list[j][k] != '\0')
			{
				if(a->read_list[i][k] != b->write_list[j][k])
					match = false;
				k++;
			}
			if(match == true && a->read_list[i][k] == b->write_list[j][k])
			{
				return true;
			}
			k = 0;
			j++;
		}
		i++;
	}
	i = 0;
	j = 0;
	while(i < a->write_size )
	{
		while(j < b->read_size)
		{
			//check for write and write
			bool match = true;
			while(a->write_list[i][k] != '\0' && b->read_list[j][k] != '\0')
			{
				if(a->write_list[i][k] != b->read_list[j][k])
					match = false;
				k++;
			}
			if(match == true && a->write_list[i][k] == b->read_list[j][k])
			{
				return true;
			}
			k = 0;
			j++;
		}
		i++;
	}
	return false;
}

DependencyGraph* create_dependency_graph(command_stream_t stream)
{
	//Create Dependency Tree
	DependencyGraph* graph = malloc(sizeof(DependencyGraph));
	//graph->dependencies = malloc(sizeof(Queue));
	//graph->no_dependencies = malloc(sizeof(Queue));
	//graph->dependencies_tail = malloc(sizeof(Queue));
	//graph->no_dependencies_tail = malloc(sizeof(Queue));
	graph->size_dependencies = 0;
	graph->size_no_dependencies = 0;
	command_t command;
	while ((command = read_command_stream (stream)))
	{
		//Read each command tree and generate graph Node
	    GraphNode* g = generate_GraphNode(command);
		
		g->num_dependencies = 0;
		g->max_dependencies = 100;
		g->before = malloc(sizeof(GraphNode*) * g->max_dependencies);
		
	    Queue* cursor = graph->no_dependencies;
		bool hasDependency = false;
		int i = 0;
		while(i < graph->size_no_dependencies)
		{
			//Determine where graph node belongs
			if(check_dependencies(g, cursor->node))
			{
				hasDependency = true;
				g->before[g->num_dependencies] = cursor->node;
				g->num_dependencies += 1;
				
				if(g->num_dependencies >= g->max_dependencies)
				{
					g->max_dependencies += 100;
	 				g->before = (GraphNode**)realloc(g->before, g->max_dependencies*sizeof(GraphNode*));
				}
			}
			
			cursor = cursor->next;
			i++;
		}
		
		cursor = graph->dependencies;
		i = 0;
		while(i < graph->size_dependencies)
		{
			//Keep determining where graph node belongs
			if(check_dependencies(g, cursor->node))
			{
				hasDependency = true;
				g->before[g->num_dependencies] = cursor->node;
				g->num_dependencies += 1;
				
				if(g->num_dependencies >= g->max_dependencies)
				{
					g->max_dependencies += 100;
	 				g->before = (GraphNode**)realloc(g->before, g->max_dependencies*sizeof(GraphNode*));
				}
			}
			
			cursor = cursor->next;
			i++;
		}
		
		//Add graph node to appropriate Queue (note queue is very similar to linked list :D)
		if(!hasDependency)
		{
			Queue* add = malloc(sizeof(Queue));
			add->node = g;
			if(graph->size_no_dependencies == 0)
			{
				graph->no_dependencies = add;
				graph->no_dependencies_tail = graph->no_dependencies;
			}
			else
			{
				graph->no_dependencies_tail->next = add;
				graph->no_dependencies_tail = graph->no_dependencies_tail->next;
			}
			graph->size_no_dependencies += 1;
		}
		else
		{
			Queue* add = malloc(sizeof(Queue));
			add->node = g;
			if(graph->size_dependencies == 0)
			{
				graph->dependencies = add;
				graph->dependencies_tail = graph->dependencies;
			}
			else
			{
				graph->dependencies_tail->next = add;
				graph->dependencies_tail = graph->dependencies_tail->next;
			}
			graph->size_dependencies += 1;
		}
	}
	
	//Make sure we return stream in a valid state
	stream->cursor = stream->head;
	return graph;
}

int
main (int argc, char **argv)
{
  int command_number = 1;
  bool print_tree = false;
  bool time_travel = false;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = true; break;
      case 't': time_travel = true; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
	command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

	command_t last_command = NULL;

	if(!time_travel)
	{
		command_t command;
		while ((command = read_command_stream (command_stream)))
		{
		  	if (print_tree)
			{
			  printf ("# %d\n", command_number++);
			  print_command (command);
			}
			  else
			{
			  last_command = command;
			  execute_command (command, time_travel);
			}
		}
	}
	else
	{
		//If time_travel we then want to parallize everything
		DependencyGraph *g = create_dependency_graph(command_stream);
		
		Queue* cursor = g->no_dependencies;
		int i = 0;

		pid_t* pids = malloc(sizeof(int)*(g->size_dependencies + g->size_no_dependencies));

		while(i < g->size_no_dependencies)
		{
			//They have no dependencies so they can run wild!
			cursor->node->pid = fork();
			last_command = cursor->node->command;
			if(cursor->node->pid == 0){
				execute_command(cursor->node->command, time_travel);
				exit(command_status (last_command));
			}

			pids[i] = cursor->node->pid;
			cursor = cursor->next;
			i++;
		}
		
		cursor = g->dependencies;
		i = 0;
		while(i < g->size_dependencies)
		{
			//Make sure these wait for their dependencies to run first using waitpid
			cursor->node->pid = fork();
			last_command = cursor->node->command;
			if(cursor->node->pid == 0){
				int i = 0;
				while(i < cursor->node->num_dependencies)
				{
					int status;
					waitpid(cursor->node->before[i]->pid, &status, WEXITED);
					i++;
				}

				int p = fork();

				if(p == 0)
				{
					execute_command(cursor->node->command, time_travel);
					exit(command_status (last_command));
				}
				wait(NULL);
			}
			pids[i+ g->size_no_dependencies] = cursor->node->pid;
			i++;
			cursor = cursor->next;
		}
		while(waitpid(-1, NULL, 0))
		{
			if(errno == ECHILD)
			{
				break;
			}
		}
		last_command->status = WEXITSTATUS(0);
	}
  return print_tree || !last_command ? 0 : command_status (last_command);
}
