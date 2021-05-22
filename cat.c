/*===========================================================================

  cat.c

  Main body of the "cat" command. See the cat_help() function for
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

/* Display modes. */
#define DF_LONG 0x01
#define DF_PAGE 0x02

char buff[256];

/*===========================================================================

  globals

===========================================================================*/
/* Terminal size. */
int tm_rows;
int tm_cols;
/* Number of filenames we can fit across the screen. */
int nacross;
/* Current number of files written on the line. This has to be
   global because it spans multiple invocations of ls(). */
int across = 0;

/* Current number of lines written, for paging purposes. */
int lines = 0;

/*===========================================================================

  cat_do_file
===========================================================================*/
void cat_do_file (filename, d_flag)
char *filename;
uint8_t d_flag;
  {
  BOOL page = d_flag & DF_PAGE;
  FILE *f = fopen (filename, "r");
  if (f)
    {
    char line[256];
    while (fgets (line, tm_cols - 1, f))
      {
      line [tm_cols] = 0;
      printf ("%s", line);

      if (lines++ ==  tm_rows - 2)
        {
        while (lines == tm_rows - 1 && page)
          {
          int c = tm_g_rchar(); 
          switch (c)
            {
            case I_INTR: exit(0); 
            case 13: case 10: lines = tm_rows - 2; break;
            case ' ': lines = 0; 
            default: break; 
            }
          }
        }
      }
    fclose (f);
    }
  else
    fprintf (stderr, "%s: %s\r\n", filename, strerror (ENOENT));
  }

/*===========================================================================

  cat_expand

  On entry 'thing' may be a file, a file pattern including drive, or
  just a drive. If there's a drive, the : must be in position 1, or it's
  a bad filename.
===========================================================================*/
void cat_expand (thing, d_flag)
char *thing;
uint8_t d_flag;
  {
  dirent **dirs;
  char *path = thing;
  Drive drive = 0;
  int colpos;
  BOOL page = FALSE;

  if (d_flag & DF_PAGE)
    page = TRUE;

  colpos = strchr (thing, ':'); 
  if (colpos)
    {
    if (colpos == thing + 1)
      {
      path = thing + 2;
      drive = thing[0] - 'A' + 1;
      if (drive < 1 || drive > 26)
        {
	fprintf (stderr, "\n%c: %s\n", thing[0], strerror (E_DLET));
        return;
	}
      }
    else
      {
      fprintf (stderr, "\n%s: %s\n", thing, strerror (E_FNAME));
      return;
      }
    }

  if (!path[0]) path = "*";

  dirs = dirs_list (drive, path, 0); /* don't sort or set size */
  if (dirs)
    {
    int i = 0;
    while (dirs[i]) 
      {
      char fn[BD_MAX_PATH + 1];
      Drive drive = dirs[i]->drive; 
      if (dirs[i]->drive == 0)
        drive = bd_cur_drv(); 

      fn[0] = (drive - 1) + 'A';
      fn[1] = ':';
      strcpy (fn + 2, dirs[i]->sname);

      cat_do_file (fn, d_flag);

      i++;
      }
    dirs_free (dirs);
    if (i == 0)
      fprintf (stderr, "%s: no files matched\r\n", thing); 

    printf ("\r\n");
    }
  else
    {
    fprintf (stderr, "\n%s: %s\n", thing, strerror (errno));
    return;
    }
  }

/*===========================================================================

  cat_help 

===========================================================================*/
void cat_help ()
  {
  printf ("Usage: cat [/p] [paths...]\r\n");
  printf ("Display text files.\r\n");
  printf ("Paths may contain wildcard and/or drive letter, e.g., ");
  printf ("\"C:\", \"*.txt\", \r\n\"o:*.com\". ");
  printf ("Paths and options are case-insensitive.\r\n");
  printf ("by name by default.\r\n");
  printf ("Options:\r\n");
  printf ("  /p  page mode\r\n");
  }

/*===========================================================================

  main

===========================================================================*/
int main (argc, argv)
int argc;
char **argv;
  {
  int i, opt;
  uint8_t d_flag = 0;

  argv[0] = "cat";
  
  while ((opt = getopt (argc, argv, "HP")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        cat_help ();
        exit (0);
      case 'P': d_flag |= DF_PAGE; break;
      default: exit (-1); 
      }
    }

  tm_size (&tm_rows, &tm_cols);

  if (optind == argc)
    {
    /* Cat stdin to stout. Ignore paging */
    int n;
    do
      {
      n = fread (buff, 1, sizeof (buff), stdin);
      fwrite (buff, 1, n, stdout);
      fflush (stdout);
      }
    while (n == sizeof (buff));
    }
  else
    {
    for (i = optind; i < argc; i++)
      {
      char *arg = argv[i];
      if (strchr (arg, '*') || strchr (arg, '?'))
        cat_expand (argv[i], d_flag);
      else 
        cat_do_file (arg, d_flag);
      }
    }

  return 0;
  }


