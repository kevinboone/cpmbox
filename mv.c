/*===========================================================================

  mv.c

  Main body of the "mv" command. See the mv_help() function for
  command line usage.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "ctype.h"
#include "errno.h"
#include "config.h"
#include "compat.h"
#include "defs.h"
#include "bdos.h"
#include "dirs.h"
#include "getopt.h"
#include "error.h"
#include "term.h"

/*===========================================================================

  mv_help 

===========================================================================*/
void mv_help ()
  {
  printf ("Usage: mv [old_name] [new_name]\r\n");
  printf ("Rename a file.\r\n");
  }

/*===========================================================================

  main

===========================================================================*/
int main (argc, argv)
int argc;
char **argv;
  {
  int i, opt;

  argv[0] = "mv";
  
  while ((opt = getopt (argc, argv, "H")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        mv_help ();
        exit (0);
      default: exit (-1); 
      }
    }

  if (argc - optind == 2)
    {
    char *from = argv[optind];
    char *to = argv[optind + 1];

    FILE *f = fopen (from, "r");
    if (f)
      {
      fclose (f);
      rename (from, to);
      }
    else
      {
      fprintf (stderr, "%s: %s\r\n", from, strerror (ENOENT));
      }
    }
  else
    {
    fprintf (stderr, "No files specified\r\n");
    exit (EINVAL);
    }

  return 0;
  }


