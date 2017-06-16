#include <stdlib.h>
#include <argp.h>
#include "server.h"
#include "encryption.h"

static char doc[] =
  "controller";

static char args_doc[] = "callback";

static struct argp_option options[] = {
  {"service", 's', "service", 0, "FTP SSH HTTP ALL (DEFAULT:HTTP)"},
  { 0 }
};

struct arguments
{
  char* args[1];
  char* service;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;
  switch (key)
    {
    case 's':
      {
        arguments->service = arg;
        break;
      }
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
      {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 1)
      {
        argp_usage (state);
      }
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
  arguments.service = "HTTP";
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  run_server(arguments.service, arguments.args[0]);
  exit (0);
}
