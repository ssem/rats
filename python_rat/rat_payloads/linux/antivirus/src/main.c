#include <stdlib.h>
#include <argp.h>
#include "discover.h"


static char doc[] =
  "Discover and disable antivirus";

static char args_doc[] = "";

static struct argp_option options[] = {
  {"discover", 'd', 0, 0, "Discover antivirus" },
  { 0 }
};

struct arguments
{
  int discover;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;
  switch (key)
    {
    case 'd':
      arguments->discover = 1;
      break;
    case ARGP_KEY_END:
      if (arguments->discover == 0)
        argp_usage(state);
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
  arguments.discover = 0;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.discover) {
    discover();
  }
  exit (0);
}
