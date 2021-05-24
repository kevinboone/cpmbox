/*===========================================================================

  cp.c

  Main body of the "cp" command. See the cp_help() function for
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

uint8_t buff[256];

#define DF_VERB 0x01
#define DF_PROG 0x02

/*===========================================================================

  cp_do_cp

  Returns TRUE on success

===========================================================================*/
BOOL cp_do_cp (from, to, d_flag)
char *from;
char *to;
uint8_t d_flag;
  {
  int fin, fout;
  char fullto [BD_MAX_PATH + 1];
  char fullfrom [BD_MAX_PATH + 1];

  if (from[1] == ':')
    {
    strcpy (fullfrom, from);
    }
  else
    {
    fullfrom[0] = bd_cur_drv () - 1 + 'A';
    fullfrom[1] = ':';
    strcpy (fullfrom + 2, from);
    }

  if (strlen (to) == 2 && to[1] == ':')
    {
    /* "to" argument is just a drive letter. */

    fullto[0] = to[0];
    fullto[1] = ':';
    if (from[1] == ':')
      strcpy (fullto + 2, from + 2);
    else
      strcpy (fullto + 2, from);
    }
  else
    strcpy (fullto, to);

  if (fullto[1] == ':')
    {
    }
  else
    {
    char temp [BD_MAX_PATH + 1];
    strcpy (temp, fullto);
    fullto[0] = bd_cur_drv () - 1 + 'A';
    fullto[1] = ':';
    strcpy (fullto + 2, temp);
    }


  if (d_flag & DF_VERB)
    {
    printf ("%s -> %s", fullfrom, fullto);
    fflush (stdout);
    if (!(d_flag & DF_PROG))
      printf ("\r\n");
    }

  if (strcmp (fullfrom, fullto) == 0)
    {
    fprintf (stderr, "%s: Source and destination are the same\r\n", to);
    return FALSE;
    }

  fin = open (fullfrom, O_RDONLY);
  if (fin >= 0)
    {
    fout = open (fullto, O_WRONLY | O_CREAT | O_TRUNC);
    if (fout >= 0)
      {
      int n = read (fin, &buff, sizeof (buff));
      while (n > 0)
        {
        write (fout, &buff, n);
        n = read (fin, &buff, sizeof (buff));
        if (d_flag & DF_PROG)
          {
          printf (".");
          fflush (stdout);
          }
        }
      close (fout);
      if (d_flag & DF_PROG)
        printf ("\r\n");
      if (n < 0)
        {
        fprintf (stderr, "%s: %s\r\n", fullto, strerror (errno));
        }
      }
    else
      {
      fprintf (stderr, "%s: %s\r\n", fullto, strerror (errno));
      return FALSE;
      }
    close (fin);
    }
  else
    {
    fprintf (stderr, "%s: %s\r\n", fullfrom, strerror (errno));
    return FALSE;
    }

  return TRUE;
  }

/*===========================================================================

  cp_help 

===========================================================================*/
void cp_help ()
  {
  /* TODO */
  printf ("Usage: cp [/v] [/p] {from...} {to} \r\n");
  printf ("Copy a file to a file, or multiple files to a drive, or a drive\r\n");
  printf ("to a drive. For example: \"cp *.c b:\"\r\n");
  printf ("Options:\r\n");
  printf ("  /v  show files being copied\r\n");
  printf ("  /d  show copy progress visually\r\n");
  }

/*===========================================================================

  cp_expand

  on entry, 'thing' is a file and/or drive spec

===========================================================================*/
void cp_expand (thing, odrive, d_flag)
char *thing;
uint8_t d_flag;
int odrive;
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

  dirs = dirs_list (drive, path, 0); /* Don't sort or set sizes */
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

      fn2[0] = (odrive - 1) + 'A';
      fn2[1] = ':';
      strcpy (fn2 + 2, dirs[i]->sname);

      ok = cp_do_cp (fn, fn2, d_flag);

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

  argv[0] = "cp";
  
  while ((opt = getopt (argc, argv, "HVD")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        cp_help ();
        exit (0);
      case 'V': d_flag |= DF_VERB; break;
      case 'D': d_flag |= DF_PROG; break;
      default: exit (-1); 
      }
    }

  myargs = argc - optind;
  if (myargs >= 2)
    {
    BOOL mult_src = FALSE;

    if (myargs > 2)
      mult_src = TRUE;
    else
      {
      char *arg1 = argv[optind];
      if (strchr (arg1, '*') || strchr (arg1, '?'))
        mult_src = TRUE;
      }

    if (mult_src)
      {
      char *drvarg = argv[argc - 1];
      if (strlen (drvarg) == 2 && drvarg[1] == ':')
        {
        Drive drive = drvarg[0] - 'A' + 1;
        if (drive > 0 && drive <= 26)
          {
          for (i = optind; i < argc - 1; i++)
            {
            cp_expand (argv[i], drive, d_flag);
            }
          }
        else
          {
          fprintf (stderr, "%s: %c: %s\r\n", argv[0], 
             drvarg[0], strerror (E_DLET));
          exit (EINVAL); 
          }
        }
      else
        {
        fprintf (stderr, "%s: Last argument must be a drive.\r\n", argv[0]);
        exit (EINVAL); 
        }
      }
    else
      cp_do_cp (argv[optind], argv[optind + 1], d_flag);
    }
  else
    {
    fprintf (stderr, "Not enough files specified.\r\n");
    exit (EINVAL); 
    }

  return 0;
  }


