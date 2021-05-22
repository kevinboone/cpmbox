/*===========================================================================

  error.c

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "errno.h"
#include "ctype.h"
#include "defs.h"
#include "error.h"

/*===========================================================================

  strerror 

===========================================================================*/
char *strerror (err)
ErrCode err;
  {
  switch (err)
    {
    case 0: return "OK";
    case ENOENT: return "File not found";
    case ENOMEM: return "Out of memory";
    case EMFILE: return "Too many open files";
    case ENFILE: return "Too many file descriptors";
    case EEXIST: return "File already exists";
    case EACCES: return "Invalid access";
    case ENOTTY: return "Not a TTY";
    case EBADF: return "Bad file descriptor";
    case E2BIG: return "Arg list too long";
    case EDOM: return "Arg out of domain";
    case ERANGE: return "Arg out of range";
    case EINVAL: return "Invalid argument";
    case E_FNAME: return "Bad filename";
    case E_EDIR: return "Can't enumerate drive";
    case E_DLET: return "Bad drive letter";
    default: return "Unknown error";
    }
  }

