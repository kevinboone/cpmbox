/*===========================================================================

  cal.c

  Main body of the "cal" command. See the cal_help() function for
  command line usage.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "ctype.h"
#include "errno.h"
#include "config.h"
#include "compat.h"
#include "defs.h"
#include "getopt.h"
#include "error.h"
#include "term.h"


/*===========================================================================


===========================================================================*/

/*===========================================================================

  cal_help 

===========================================================================*/
void cal_help ()
  {
  printf ("Usage: cal [/m] {month} {year}\r\n");
  printf ("Display a calendar for the selected month (1-12) and year.\r\n");
  printf ("Options:\r\n");
  printf ("  /m  Week starts on Monday\r\n");
  }

/*===========================================================================

  main

===========================================================================*/
int main (argc, argv)
int argc;
char **argv;
  {
  char *argv0 = "cal";
  BOOL start_mon = FALSE;
  int opt;
  while ((opt = getopt (argc, argv, "HM")) != -1)  
    {
    switch (opt)
      {
      case 'H':
        cal_help ();
        exit (0);
      case 'M':
        start_mon = TRUE;
        break;
      default: exit (-1); 
      }
    }

  if (argc - optind != 2)
    {
    fprintf (stderr, "Usage: %s {month} {year}\r\n", argv0);
    return EINVAL;
    }

  {
  char caption[30];
  int month = atoi (argv[optind]);
  int year = atoi (argv[optind + 1]);
  int done = 0;
  int i, l;
  int offs;
  int first_of_month; 
  int t = 1;
  if (year < 100) year += 2000;

  if (year < 1)
    {
    fprintf (stderr, "Year must be 1 or greater\r\n");
    exit (EINVAL);
    }

  if (month < 1 || month > 12)
    {
    fprintf (stderr, "Month must be 1-12\r\n");
    exit (EINVAL);
    }

  sprintf (caption, "%s %d\r\n", dt_mname (month), year);
  l = (20 - strlen (caption)) / 2;
  for (i = 0; i < l; i++)
    putchar (' ');
  printf (caption);

  first_of_month = dt_dow (1, month, year);

  if (start_mon)
    {
    printf ("Mo Tu We Th Fr Sa Su\r\n");
    offs = 1;
    }
  else
    {
    printf ("Su Mo Tu We Th Fr Sa\r\n");
    offs = 0;
    }

  for (i = 0; i < 7; i++)
    {
    if (i >= first_of_month - offs) 
      {
      printf ("%2d ", t);  
      t++;
      }
    else
      printf ("   ");  
    }
  printf ("\r\n");
  for (i = 0; i < 6 && !done; i++)
    {
    int j;
    for (j = 0; j < 7 && !done; j++)
      {
      printf ("%2d ", t);  
      if (t >= dt_mdays (month)) done = 1;
      t++;
      }
    printf ("\r\n");
    }
  }

  printf ("\r\n");
  return 0;
  }


