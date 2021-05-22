/*===========================================================================

  du.c

  Main body of the "du" command. See the du_help() function for
  command line usage.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "io.h"
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
/* Number of filenames we can fit across the screen. */
int nacross;
/* Current number of files written on the line. This has to be
   global because it spans multiple invocations of ls(). */
int across = 0;

/* Current number of lines written, for paging purposes. */
int lines = 0;

long total_rec = 0;
int total_files = 0; /* unlikely to be larger than 64k on CP/M */

extern long lseek();

/*===========================================================================

  du_report

===========================================================================*/
void du_report (file, recs, d_flag)
char *file;
int recs;
uint8_t d_flag;
  {
  printf ("%-5d %s\r\n", recs, file);
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

/*===========================================================================

  du_do_du

===========================================================================*/
BOOL du_do_du (file, d_flag)
char *file;
uint8_t d_flag;
  {
  static char f[26]; /* Too big for stack. */
  fcbinit (file, f);
  if (bdos (BDOS_OPEN, f) == 0)
    {
    int recs;
    bdos (BDOS_FSIZE, f);
    recs = f[33] + 256 * f[34];
    total_rec += recs;
    total_files++;
    if (d_flag & DF_VERB) du_report (file, recs,d_flag);
    }
  else
    {
    fprintf (stderr, "%s: Can't open file\r\n", file);
    }
  bdos (BDOS_CLOSE, f);
  }

/*===========================================================================

  du_help 

===========================================================================*/
void du_help ()
  {
  /* TODO */
  printf ("Usage: du [/v] [/p] [files...]\r\n");
  printf ("Report disk space used by one or more files.\r\n");
  printf ("Options:\r\n");
  printf ("  /v  Show files being counted\r\n");
  printf ("  /p  Paging mode\r\n");
  }

/*===========================================================================

  du_expand

  on entry, 'thing' is a file and/or drive spec

===========================================================================*/
void du_expand (thing, d_flag)
char *thing;
uint8_t d_flag;
  {
  dirent **dirs;
  char *path = thing;
  Drive drive = 0;
  int colpos;

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

  dirs = dirs_list (drive, path, DST_SZ); /* No need to sort. */ 
  if (dirs)
    {
    BOOL match = FALSE;
    int i = 0;
    BOOL ok = TRUE;
    while (dirs[i] && ok) 
      {
      char fn [BD_MAX_PATH + 1];
      char fn2 [BD_MAX_PATH + 1];
      Drive drive = dirs[i]->drive; 
      if (dirs[i]->drive == 0)
        drive = bd_cur_drv(); 

      fn[0] = (drive - 1) + 'A';
      fn[1] = ':';
      strcpy (fn + 2, dirs[i]->sname);

      total_recs += dirs[i]->recs; 
      total_files++;
      if (d_flag & DF_VERB) du_report (fn, dirs[i]->recs, d_flag);

      match = TRUE;
      i++;
      }
    dirs_free (dirs);
    if (!match)
      fprintf (stderr, "%s: no matching files\r\n", thing);
    }
  else
    {
    fprintf (stderr, "\n%s: %s\n", thing, strerror (errno));
    return;
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

  argv[0] = "du";
  
  while ((opt = getopt (argc, argv, "HPV")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        du_help ();
        exit (0);
      case 'V': d_flag |= DF_VERB; break;
      case 'P': d_flag |= DF_PAGE; break;
      default: exit (-1); 
      }
    }

  tm_size (&tm_rows, &tm_cols);
  myargs = argc - optind;
  if (myargs >= 1)
    {
    char *arg1 = argv[optind];
    if (strchr (arg1, '*') || strchr (arg1, '?'))
      du_expand (arg1, d_flag);
    else if (arg1[1] == ':')
      du_expand (arg1, d_flag);
    else 
      du_do_du (arg1, d_flag);
    }
  else
    {
    du_expand ("*", d_flag);
    }

  printf ("%d file%s, %ld record%s, %ld bytes\r\n", 
    total_files, total_files == 1 ? "" : "(s)",
    total_recs, total_recs == 1 ? "" : "(s)",
    total_recs * (long)BD_SEC_SZ);


  return 0;
  }


