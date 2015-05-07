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
	 if (command->type == SIMPLE_COMMAND)
	 {
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
	 		if(command->u.word[i][0] != '-')
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
     	if(command->u.command[0])
     		generate_read_write_lists(node,command->u.command[0]);
     	if(command->u.command[1])
     		generate_read_write_lists(node,command->u.command[1]);
     }
}

GraphNode* generate_GraphNode(command_t command){
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

bool check_dependencies(GraphNode* a, GraphNode* b)
{
	int i = 0;
	int j = 0;
	int k = 0;
	while(i < a->write_size )
	{
		while(j < b->write_size)
		{
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

DependencyGraph create_dependency_graph(command_stream_t stream)
{
	DependencyGraph* graph = malloc(sizeof(DependencyGraph));
	graph->dependencies = malloc(sizeof(Queue));
	graph->no_dependencies = malloc(sizeof(Queue));
	command_t command;
	while ((command = read_command_stream (stream)))
	{
		GraphNode* g = generate_GraphNode(command);
		while(check_dependencies()){

		}
	}
	
	stream->cursor = stream->head;
	
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

  return print_tree || !last_command ? 0 : command_status (last_command);
}
