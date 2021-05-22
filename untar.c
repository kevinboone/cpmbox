/*===========================================================================

  untar.c

  Main body of the "untar" command. See the untar_help() function for
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
#include "getopt.h"
#include "error.h"

#define DF_VERB 0x01
#define DF_T 0x02

char buff[512];

/*===========================================================================

  untar_help 

===========================================================================*/
void untar_help ()
  {
  /* TODO */
  printf ("Usage: untar [/v] [/t] {file.tar} [drive:]\r\n");
  printf ("Unpack an uncompressed tarfile. If no drive is specified,\r\n");
  printf ("unpacks to the current drive.\r\n");
  printf ("Options:\r\n");
  printf ("  /t  show contents only\r\n");
  printf ("  /v  show filenames when unpacking\r\n");
  printf ("\r\n");
  }

/*===========================================================================

  parse_octal 

===========================================================================*/
static long parse_octal (p, n)
char *p;
int n;
  {
  long i = 0;

  while ((*p < '0' || *p > '7') && n > 0) 
    {
    ++p;
    --n;
    }

  while (*p >= '0' && *p <= '7' && n > 0) 
    {
    i *= 8;
    i += *p - '0';
    ++p;
    --n;
    }

  return i;
  }

/*===========================================================================

  verify_checksum 

===========================================================================*/
static BOOL verify_checksum (p)
char *p;
  {
  int stored_cs, n, u = 0;
  for (n = 0; n < 512; ++n) 
    {
    if (n < 148 || n > 155)
       u += ((unsigned char *)p)[n];
    else
       u += 0x20;
    }

  stored_cs = (int) parse_octal (p + 148, 8);
  return (u == stored_cs);
  }

/*===========================================================================

  is_end_of_archive 

===========================================================================*/
static BOOL is_end_of_archive (p)
char *p;
  {
  int n;
  for (n = 511; n >= 0; --n)
    if (p[n] != '\0')
      return  FALSE;
  return TRUE;
  }


/*===========================================================================

  untar_do_untar 

===========================================================================*/
void untar_untar (tarfile, odrive, d_flag)
char *tarfile;
Drive odrive;
uint8_t d_flag;
  {
  FILE *a;
  a = fopen (tarfile, "r");
  if (!a)
    {
    fprintf (stderr, "%s: %s\r\n", tarfile, strerror (errno));
    return;
    }

  for (;;)
    {
    FILE *f = NULL;
    long filesize;
    int bytes_read = fread (buff, 1, sizeof (buff), a);
    if (bytes_read < sizeof (buff)) 
      {
      fprintf (stderr,  "Short read: expected %d, got %d\r\n",
         sizeof (buff), (int)bytes_read);
         return;
      }
    if (is_end_of_archive (buff)) 
      {
      return;
      }
    if (!verify_checksum (buff)) 
      {
      fprintf (stderr, "Checksum failure\r\n");
         return;
      }
    filesize = parse_octal (buff + 124, 12);
    switch (buff[156]) 
      {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
        fprintf (stderr, "Ignoring '%s'\r\n", buff);
        break;
      default:
        if (d_flag & DF_T)
          {
          printf ("%s %ld\r\n", buff, filesize);
          }
        else
          {
          if (d_flag & DF_VERB)
            printf ("Extracting file '%s'\r\n", buff);
          f = fopen (buff, "w");
          if (!f)
            {
            fprintf (stderr, "%s: %s\r\n", buff, strerror (errno));
            }
          }
        break;
      }
    while (filesize > 0) 
      {
      bytes_read = fread (buff, 1, sizeof(buff), a);
      if (bytes_read < sizeof (buff)) 
          {
          fprintf (stderr, "Short read: Expected %d, got %d\r\n",
             sizeof (buff), (int)bytes_read);
           return;
           }
      if (filesize < (int)sizeof (buff))
            bytes_read = filesize;
      if (f) 
        {
        if (fwrite (buff, 1, bytes_read, f) != (int)bytes_read)
          {
          fprintf (stderr, "Failed write: %s\r\n", strerror (errno));
          fclose (f);
          f = NULL;
          }
        }
      filesize -= bytes_read;
      }
    if (f) 
      {
      fclose (f);
      f = NULL;
      }
    }

  fclose (a);
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
  Drive odrive;

  argv[0] = "untar";
  
  while ((opt = getopt (argc, argv, "HVT")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        untar_help ();
        exit (0);
      case 'V': d_flag |= DF_VERB; break;
      case 'T': d_flag |= DF_T; break;
      default: exit (EINVAL); 
      }
    }

  myargs = argc - optind;
  if (myargs == 1 || myargs == 2)
    {
    char *tarfile = argv[optind];
    if (myargs == 2)
      {
      char *drvarg = argv[argc - 1];
      if (strlen (drvarg) == 2 && drvarg[1] == ':')
        {
        odrive = drvarg[0] - 'A' + 1;
        if (odrive > 0 && odrive <= 26)
          {
          /* OK */
          }
        else
          {
          fprintf (stderr, "%s: %c: %s\r\n", argv[0],
            drvarg[0], strerror (E_DLET));
          exit (-1); /* TODO */
          }
        }
      else 
        {
        fprintf (stderr, "%s: Last argument must be a drive.\r\n", argv[0]);
        exit (EINVAL);
        }
      }
    else
      {
      odrive = bd_cur_drv();
      }

    untar_untar (tarfile, odrive, d_flag);
    }
  else
    {
    fprintf (stderr, "Too many arguments. '%s /h' for information\r\n", 
      argv[0]); 
    exit (EINVAL);
    }

  return 0;
  }


