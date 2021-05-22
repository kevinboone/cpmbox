/*===========================================================================

  date.c

  Various date-handling functions

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#include "stdio.h"
#include "ctype.h"
#include "config.h"
#include "defs.h"
#include "getopt.h"

static int t_mon[] = { 0, 3, 2, 5, 0, 3,
                       5, 1, 4, 6, 2, 4 };

static char *dnames[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                       "Thursday", "Friday", "Saturday"};

static char *mnames[] = {"January", "February", "March", "April", "May",
                       "June", "July", "August", "September", "October",
                       "November", "December"};

/*=========================================================================

  dt_dname

=========================================================================*/
char *dt_dname (dow)
int dow;
  {
  return dnames[dow]; 
  }

/*=========================================================================

  dt_mname

=========================================================================*/
char *dt_mname (moy)
int moy;
  {
  return mnames[moy - 1]; 
  }

/*=========================================================================

  dt_mdays
  
  Get the number of days in the month, for a specified months (1-12)
  and year.

=========================================================================*/
int dt_mdays (mon_num, year)
int mon_num;
int year;
  {
  if (mon_num == 2) 
    {
    /* In leap year, february has 29 days */
    if (year % 400 == 0
            || (year % 4 == 0
                && year % 100 != 0))
      return 29;
    else
      return 28;
    }
  
  if (mon_num == 4)
    return 30;
  
  if (mon_num == 6)
    return 30;
  
  if (mon_num == 9)
    return 30;
  
  if (mon_num == 11)
    return 30;
  
  return 31;
  }

/*=========================================================================

  dt_dow

  For a given day, month, and year, get the day of the week, with
  0 = sunday.

=========================================================================*/
int dt_dow (day, month, year)
int day;
int month;
int year;
  {
  year -= month < 3;
  return (year + year / 4
          - year / 100
          + year / 400
          + t_mon[month - 1] + day)
          % 7;
  }



