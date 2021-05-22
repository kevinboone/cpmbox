/*===========================================================================

  date.h

  Various date-handling functions

  In general, sunday = 0. Day of month is 1-31, month is 1-12, year
    is four digits.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#ifndef __DATE_H
#define __DATE_H

/* Get the number of days in the month and year. */
/*int dt_mdays (mon_num, year) */
extern int dt_mdays ();

/* Get the day of week, starting at sunday = 0.. */
/* int dt_dow (day, month, year) */
extern int dt_dow ();

/* Get the full name of the day of week, with 0 = sunday */
char *dt_dname (dow);

/* Get the full name of the month of the year, with 1 = January */
char *dt_mname (mow);

#endif /* date.h */

