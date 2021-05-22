/*===========================================================================

  ls.c

  Main body of the "ls" command. See the ls_help() function for
  command line usage.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "ctype.h"
#include "config.h"
#include "defs.h"
#include "dirs.h"
#include "getopt.h"
#include "error.h"
#include "term.h"

/* Display modes. */
#define DF_LONG 0x01
#define DF_PAGE 0x02

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

  ls 

  On entry 'thing' may be a file, a file pattern including drive, or
  just a drive. If there's a drive, the : must be in position 1, or it's
  a bad filename.
===========================================================================*/
void ls (thing, flags, d_flag)
char *thing;
uint8_t flags;
uint8_t d_flag;
  {
  dirent **dirs;
  char *path = thing;
  Drive drive = 0;
  int colpos;
  BOOL lng = FALSE;
  BOOL page = FALSE;

  if (d_flag & DF_LONG)
    lng = TRUE;

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

  dirs = dirs_list (drive, path, flags | DST_SZ); 
  if (dirs)
    {
    int i = 0;
    while (dirs[i]) 
      {
      long size = (long)dirs[i]->recs * (long)BD_SEC_SZ;
      if (lng)
        {
        printf ("%-13s %2s %1s %ld", dirs[i]->sname, 
         dirs[i]->ro ? "ro" : "rw", 
         dirs[i]->sys ? "s" : " ", size);
        }
      else
        printf ("%-13s", dirs[i]->sname);
      if (!lng)
        across++;
      if (lng || across == nacross)
        {
        printf ("\r\n");
        across = 0;
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
      i++;
      }
    dirs_free (dirs);
    if (across != 0)
      printf ("\r\n");
    }
  else
    {
    fprintf (stderr, "\n%s: %s\n", thing, strerror (errno));
    return;
    }
  }

/*===========================================================================

  ls_help 

===========================================================================*/
void ls_help ()
  {
  printf ("Usage: ls [options] [paths...]\r\n");
  printf ("Display drive contents.\r\n");
  printf ("Paths may contain wildcard and/or drive letter, e.g., ");
  printf ("\"C:\", \"*.txt\", \r\n\"o:*.com\". ");
  printf ("Paths and options are case-insensitive. Filenames are sorted\r\n");
  printf ("by name by default.\r\n");
  printf ("Options:\r\n");
  printf ("  /l  long listing\r\n");
  printf ("  /n  no sorting\r\n");
  printf ("  /p  page mode\r\n");
  printf ("  /s  sort by size\r\n");
  }

/*===========================================================================

  main

===========================================================================*/
int main (argc, argv)
int argc;
char **argv;
  {
  ErrCode err_code;
  int i, opt;
  uint8_t srt_flag = DST_NAME;
  uint8_t d_flag = 0;

  argv[0] = "ls";
  
  while ((opt = getopt (argc, argv, "LHPSU")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        ls_help ();
        exit (0);
      case 'L': d_flag |= DF_LONG; break;
      case 'P': d_flag |= DF_PAGE; break;
      case 'S': 
        srt_flag &= ~DST_NAME; 
        srt_flag |= DST_SIZE; 
        break;
      case 'U': 
        srt_flag &= ~DST_NAME; 
        break;
      default: exit (-1); 
      }
    }

  tm_size (&tm_rows, &tm_cols);
  nacross = tm_cols / (BD_MAX_DFNAME + 1);

  if (optind == argc)
    {
    ls ("*", srt_flag, d_flag); 
    }
  else
    {
    for (i = optind; i < argc; i++)
      {
      ls (argv[i], srt_flag, d_flag);
      }
    }

  return 0;
  }


