/*===========================================================================

  dirs.c

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "errno.h"
#include "ctype.h"
#include "defs.h"
#include "bdos.h"
#include "error.h"
#include "compat.h"
#include "dirs.h"

/*===========================================================================

  count_dir 

  Count the number of entries in the specified drive's directory.

  drive -- A=1, B=2...

  Returns zero on success

===========================================================================*/
static ErrCode count_dir (drive, total)
Drive drive;
int *total;
  {
  int n;
  char *fcb = FCB; 
  fcb[0] = drive;
  *total = 0;
  strcpy (fcb + 1, "???????????");

  if ((n = bdos (BDOS_DFIRST, FCB)) == 255) return E_EDIR; 
    {
    do
      {
      char *fcbbuf = DMABUF + 32*n;
      (*total)++;
      } while ((n=bdos (BDOS_DNEXT, FCB)) != 255);  
    }

  return 0;
  }

/*===========================================================================

  san_fname

  "Sanitize" a filename from an FCB. Convert an FCB (space-padded)
  name to "xxx.yyy" format. Note that the result will be one character
  longer, and there also needs to be room in 'name' for the terminating
  zero.

===========================================================================*/
void san_fname (name)
char *name;
  {
  char p1[9];
  char p2[4];
  char *t;

  strncpy (p1, name, 8); 
  p1[8] = 0;
  strncpy (p2, name + 8, 3); 
  p2[3] = 0;

  t = p1 + 7;
  while (*t == ' ' && t >= p1) { *t = 0; t--; } 
  t = p2 + 2;
  while (*t == ' ' && t >= p2) { *t = 0; t--; } 

  strlower (p1);
  strlower (p2);

  strcpy (name, p1);
  if (p2[0])
    {
    strcat (name, ".");
    strcat (name, p2);
    }
  }

/*===========================================================================

  fill_dirs 

  drive -- A=1, B=2...

  Returns zero on success

===========================================================================*/
static ErrCode fill_dirs (drive, max, dirents, pattern)
Drive drive;
int max;
char *pattern;
dirent *dirents[];
  {
  int i, n;
  int count = 0;
  char *fcb = FCB; 
  fcb[0] = drive;
  strcpy (fcb + 1, "???????????"); 

  if ((n = bdos (BDOS_DFIRST, FCB)) == 255) return E_EDIR; 
    {
    do
      {
      int ret;
      char *fcbbuf = DMABUF + 32 * n;
      char temp_name [BD_MAX_FNAME + 1]; 
      char temp_sname [BD_MAX_DFNAME + 1]; 

      for (i = 0; i < 11; i++)
        {
        temp_name[i] = fcbbuf[1 + i] & CHAR_MASK;
        }
      temp_name[BD_MAX_FNAME] = 0;
      strcpy (temp_sname, temp_name);
      san_fname (temp_sname);
      
      ret = fnmatch (pattern, temp_sname, FNM_CASEFOLD);
      if (ret == 0)
        {
        dirents[count] = malloc (sizeof (dirent));

        strcpy (dirents[count]->name, temp_name);
        strcpy (dirents[count]->sname, temp_sname);

        dirents[count]->drive = drive;
        dirents[count]->ro = fcbbuf[9] & ATTR_MASK; 
        dirents[count]->sys = fcbbuf[9] & ATTR_MASK;
        count++;
        }
      } while ((n=bdos (BDOS_DNEXT, FCB)) != 255 && count < max);  
    }
 
  dirents[count] = 0;

  return 0;
  }


/*===========================================================================

  size_dirs

===========================================================================*/
static void size_dirs (dirents)
dirent *dirents[];
  {
  int i = 0;
  while (dirents[i])
    {
    uint8_t _fcb[36];
    memset (_fcb, 0, sizeof (_fcb));
    _fcb[0] = dirents[i]->drive;
    memcpy (_fcb + 1, dirents[i]->name, BD_MAX_FNAME);
    if (bdos (BDOS_OPEN, _fcb) == 0)
      {
      bdos (BDOS_FSIZE, _fcb);
      dirents[i]->recs = _fcb[33] + 256 * _fcb[34];
      }
    else
      {
      /* Some files don't seem to open, even though they have
         directory entries. Lacking any other information, say
         they are one record. */
      dirents[i]->recs = 1;
      }
    bdos (BDOS_CLOSE, _fcb);
    i++;
    }
  }

/*===========================================================================

  free_dirs 

===========================================================================*/
void dirs_free (dirents)
dirent *dirents[];
  {
  int i = 0;
  while (dirents[i])
    {
    free (dirents[i]);
    i++;
    }
  free (dirents);
  }

/*===========================================================================

  dirs_len 

===========================================================================*/
int dirs_len (dirents)
dirent *dirents[];
  {
  int i = 0;
  while (dirents[i]) { i++; }
  return i;
  }

/*===========================================================================

  sort_dirs 

===========================================================================*/
static void sort_dirs (d, flags)
dirent *d[];
uint8_t flags;
  {
  int n;
  int i, j;

  if (!(flags & (DST_NAME | DST_SIZE)))
    return;

  n = dirs_len (d);
  for (j = 0; j < n; j++)
    {
    for (i = j; i < n; i++)
      {
      if (flags & DST_SIZE)
        {
        if (d[i]->recs > d[j]->recs)
          {
          dirent *t = d[i];
          d[i] = d[j];
          d[j] = t;
          }
	}
      else
        {
        char *n1 = d[i]->name;
        char *n2 = d[j]->name;
        if (strcmp (n1, n2) < 0)
          {
          dirent *t = d[i];
          d[i] = d[j];
          d[j] = t;
          }
	}
      }
    }
  }

/*===========================================================================

  dirs_list

===========================================================================*/
dirent **dirs_list (drive, pattern, flags)
Drive drive;
char *pattern;
uint8_t flags;
  {
  int n;
  dirent **d = 0; 
  errno = count_dir (drive, &n);
  if (errno == 0)
    {
    int i;
    d = malloc ((n + 1) * sizeof (dirent *));
    if (d)
      {
      d[0] = 0;
      fill_dirs (drive, n, d, pattern);
      if (flags & DST_SZ)
        size_dirs (d);
      if (flags & 0x0F)
        sort_dirs (d, flags); 
      }
    else
      errno = ENOMEM;
    }
  return d;
  }


