/*===========================================================================

  dir.h

  Functions for expanding BDOS directories

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#ifndef __DIR_H
#define __DIR_H

#include "defs.h"
#include "bdos.h"

/* Directory sort flags */
#define DST_NAME  0x01
#define DST_SIZE  0x02
#define DST_ASC   0x00
#define DST_DSC   0x04

#define DST_SZ    0x10

typedef struct _dirent
  {
  Drive drive;
  char name [BD_MAX_FNAME + 1];
  char sname [BD_MAX_DFNAME + 1];
  BOOL sys;
  BOOL ro;
  int recs;
  } dirent;

/** Expand a drive's directory as an array of dirent structures. 
    The caller must call dirs_free to free the memory. Args:
    Drive drive, char *pattern, uint8_t flags. If the operation
    fails, no memory is allocated (I hope), and errno will be set. */
dirent **dirs_list ();

/** Free the array of dirent objects created by dirs_list. */
void dirs_free ();

#endif /* dir.h */

