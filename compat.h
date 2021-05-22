/*===========================================================================

  compat.h

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#ifndef __COMPAT_H
#define __COMPAT_H

/* Constants for fnmatch() */
#define FNM_NOMATCH     -1
#define FNM_LEADING_DIR 0x0001 
#define FNM_NOESCAPE    0x0002 
#define FNM_PERIOD      0x0004 
#define FNM_FILE_NAME   0x0008 
#define FNM_CASEFOLD    0x0010 

extern void memset ();
extern void memcpy ();
extern int fnmatch ();
extern char *strchr ();
extern void strlower ();

#endif /* compat.h */

