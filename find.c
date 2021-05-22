/*===========================================================================

  find.c

  Main body of the "find" command. See the find_help() function for
  command line usage.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "fcntl.h"
#include "errno.h"
#include "ctype.h"
#include "config.h"
#include "compat.h"
#include "defs.h"
#include "bdos.h"
#include "dirs.h"
#include "getopt.h"
#include "error.h"
#include "term.h"

#define DF_VERB 0x01
#define DF_PAGE 0x02

/*===========================================================================

  globals

===========================================================================*/
/* Terminal size. */
int tm_rows;
int tm_cols;

/* Current number of lines written, for paging purposes. */
int lines = 0;


/*===========================================================================

  find_help 

===========================================================================*/
void find_help ()
  {
  /* TODO */
  printf ("Usage: find [/p] {pattern}\r\n");
  printf ("Searches drives for files matching the pattern.\r\n");
  printf ("Options:\r\n");
  printf ("  /p  page mode\n");
  }

/*===========================================================================

  find_on_drive 

===========================================================================*/
void find_on_drive (drive, pattern, d_flag)
Drive drive;
char *pattern;
uint8_t d_flag;  
  {
  dirent **dirs = dirs_list (drive, pattern, 0); 
printf ("df=%02x\n", d_flag);
  if (dirs)
    {
    int i = 0;
    while (dirs[i]) 
      {
      printf ("%c:%s\r\n", drive + 'A' - 1, dirs[i]->sname);
      i++;
      if (lines++ ==  tm_rows - 2 && (d_flag & DF_PAGE))
	{
	while (lines == tm_rows - 1)
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
    dirs_free (dirs);
    }
  } 

/*===========================================================================

  main

===========================================================================*/
int main (argc, argv)
int argc;
char **argv;
  {
  int i, opt, myargs;
  uint8_t d_flag = 0;

  argv[0] = "find";
  
  while ((opt = getopt (argc, argv, "HVP")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        find_help ();
        exit (0);
      case 'V': d_flag |= DF_VERB; break;
      case 'P': d_flag |= DF_PAGE; break;
      default: exit (-1); 
      }
    }

  tm_size (&tm_rows, &tm_cols);

  myargs = argc - optind;
  if (myargs == 1)
    {
    int i;
    for (i = 1; i <= 26; i++)
      {
      find_on_drive (i, argv[optind], d_flag);
      } 
    }
  else
    {
    fprintf (stderr, "Specify one file pattern.\r\n");
    exit (EINVAL); 
    }

  return 0;
  }


