#include <stdlib.h>
#include <argp.h>
#include "server.h"

static char doc[] =
  "simple";

static char args_doc[] = "";

static struct argp_option options[] = {
  {"command", 'c', "command", 0, "command"},
};

struct arguments
{
  char* args[1];
  char* command;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;
  switch (key)
    {
    case 'c':
      {
        arguments->command = arg;
        break;
      }
    case ARGP_KEY_ARG:
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int
main (int argc, char **argv)
{
  struct arguments arguments;
  arguments.command = "ls";
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  run_server(arguments.command, arguments.args[0]);
  exit (0);
}
