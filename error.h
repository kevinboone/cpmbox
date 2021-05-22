/*===========================================================================

  error.h

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#ifndef __ERROR_H
#define __ERROR_H

/* Bad filename. */
#define E_FNAME 1

/* Can't enumerate directory on drive */
#define E_EDIR 2

/* Bad drive letter */
#define E_DLET 3

typedef int ErrCode;

extern int errno;

#endif /* error.h */

