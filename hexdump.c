/*===========================================================================

  hd.c

  Main body of the "hd" command. See the hd_help() function for
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
#define DF_CHARS 0x01
#define DF_PAGE 0x02

char buff[BD_SEC_SZ];
long file_offset = 0;

/*===========================================================================

  globals

===========================================================================*/
/* Terminal size. */
int tm_rows;
int tm_cols;
/* Number of filenames we can fit across the screen. */
int nacross;
/* Current number of files written on the line. This has to be
   global because it spans multiple invohdions of ls(). */
int across = 0;

/* Current number of lines written, for paging purposes. */
int lines = 0;

/*===========================================================================

  hd_fmt_line

===========================================================================*/
void hd_fmt_line (offset, buffpos, d_flag)
long offset;
int buffpos;
uint8_t d_flag;
  {
  int i;
  printf ("%08lx", offset + buffpos);
  for (i = 0; i < 16; i++) 
    {
    printf (" %02x", buff[buffpos + i]);
    }
  if (d_flag & DF_CHARS)
    {
    printf (" ");
    for (i = 0; i < 16; i++) 
      {
      char c = buff[buffpos + i];
      if (c >= 32 && c <= 126)
        printf ("%c", c); 
      else
        printf (".");
      }
    }
  printf ("\r\n");
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

  hd_fmt_sec

  If filename is null, do stdin

===========================================================================*/
void hd_fmt_sec (offset, n, d_flag)
long offset;
int n;
uint8_t d_flag;
  {
  int printed = 0;
  while (printed < n)
    {
    hd_fmt_line (offset, printed, d_flag);
    printed += 16;
    }
  }

/*===========================================================================

  hd_do_mem

===========================================================================*/
void hd_do_mem (d_flag)
uint8_t d_flag;
  {
  BOOL page = d_flag & DF_PAGE;
  long offset = file_offset; 
  do
    {
    memcpy (buff, (uint8_t *)offset, BD_SEC_SIZE);
    hd_fmt_sec (offset, BD_SEC_SIZE, d_flag);
    offset += BD_SEC_SZ;
    } while (offset < 65536L);
  }

/*===========================================================================

  hd_do_file

  If filename is null, do stdin

===========================================================================*/
void hd_do_file (filename, d_flag)
char *filename;
uint8_t d_flag;
  {
  BOOL page = d_flag & DF_PAGE;
  FILE *f;
  long offset = file_offset; 
  if (filename == 0)
    f = stdin;
  else
    f = fopen (filename, "r");
  if (f)
    {
    int n;
    fseek (f, offset, 0);
    do
      {
      /* Pad the buffer if the offset means that we won't read a 
         whole buffer. */
      if (file_offset != 0)
        memset (buff, 0, BD_SEC_SZ);
      n = fread (buff, 1, BD_SEC_SZ, f);
      hd_fmt_sec (offset, n, d_flag);
      offset += BD_SEC_SZ;
      } while (n > 0);
    fclose (f);
    }
  else
    fprintf (stderr, "%s: %s\r\n", filename, strerror (ENOENT));
  }

/*===========================================================================

  hd_expand

  On entry 'thing' may be a file, a file pattern including drive, or
  just a drive. If there's a drive, the : must be in position 1, or it's
  a bad filename.
===========================================================================*/
void hd_expand (thing, d_flag)
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

  dirs = dirs_list (drive, path, 0); /* Don't sort or set size */
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

      hd_do_file (fn, d_flag);

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

  hd_help 

===========================================================================*/
void hd_help ()
  {
  printf ("Usage: hd [options] [paths...]\r\n");
  printf ("Dump files or memory in hexadecminal.\r\n");
  printf ("Paths may contain wildcard and/or drive letter. Paths and options \r\n");
  printf ("  are case-insensitive.\r\n");
  printf ("If no path is given, read from stdin, or memory with \"/m\".\r\n");
  printf ("Options:\r\n");
  printf ("  /c  dump characters as well\r\n");
  printf ("  /m  dump memory\r\n");
  printf ("  /o  start at offset\r\n");
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
  BOOL mem = FALSE;

  argv[0] = "hd";
  
  while ((opt = getopt (argc, argv, "HMPCO:")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        hd_help ();
        exit (0);
      case 'C': d_flag |= DF_CHARS; break;
      case 'M': mem = TRUE; break;
      case 'P': d_flag |= DF_PAGE; break;
      case 'O': 
        if (optarg[0] == '#')
          sscanf (optarg + 1, "%lx", &file_offset); 
	else
          sscanf (optarg, "%ld", &file_offset); 
	break;
      default: exit (-1); 
      }
    }

  tm_size (&tm_rows, &tm_cols);

  if (optind == argc)
    {
    if (mem)
      hd_do_mem (d_flag);
    else
      hd_do_file (0, d_flag);
    }
  else
    {
    for (i = optind; i < argc; i++)
      {
      char *arg = argv[i];
      if (strchr (arg, '*') || strchr (arg, '?'))
        hd_expand (argv[i], d_flag);
      else 
        hd_do_file (arg, d_flag);
      }
    }

  return 0;
  }


