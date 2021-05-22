/*===========================================================================

  bdos.h

  various constants related to BDOS

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/
#ifndef __BDOS_H
#define __BDOS_H

#define FCB 0x005c	/* Default fcb address */
#define DMABUF 0x0080	/* Default DMA buffer address */

/* BDOS syscall numbers */
#define BDOS_OPEN 15
#define BDOS_CLOSE 16
#define BDOS_DFIRST 17
#define BDOS_DNEXT 18
#define BDOS_DGET 25 
#define BDOS_FSIZE 35 

/* Max filename, not including drive -- 8 + 3 */
#define BD_MAX_FNAME 11
/* Max filename with dot separator, not including drive -- 8 + 1 +  3 */
#define BD_MAX_DFNAME 12
/* Max filename with dot separator and drive -- 2 + 8 + 1 +  3 */
#define BD_MAX_PATH 14

/* BDOS FCBs high file attributes in the top bits of filename
   characters. Define masks to get, or ignore, them */
#define ATTR_MASK 0x80
#define CHAR_MASK 0x7F

#define BD_SEC_SZ 128

/** A=1, B=2... */
extern Drive bd_cur_drv(); 


#endif /* bdos.h */
