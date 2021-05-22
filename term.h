/*===========================================================================

  term.h

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#ifndef __TERM_H
#define __TERM_H

/** Return the size of the terminal in *rows, *cols, 
    or -1,-1 if it is non-sized. */
void tm_size();

/** Get an uinterpreted character from the console, without echoing. */
int tm_g_rchar();

#endif /* term.h */

