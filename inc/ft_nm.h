#ifndef FT_NM_H
# define FT_NM_H

/* 
------- Standard headers --------
!!! Note: Follwing of the subject's allowed all standard libraries in C.
          We allow to use the function like memcpy, memeset, printf, etc.
*/
# include <unistd.h>
# include <stdlib.h> // for qsort
# include <string.h> // for strncmp, memcpy
# include <stdio.h> // for perror
# include <error.h>
# include <errno.h> // for errno
# include <sys/stat.h>
# include <sys/mman.h>
# include <elf.h>
# include <limits.h> // for PATH_MAX
# include <fcntl.h>
# include <stdbool.h> // for bool type
# include <inttypes.h> // for PRIx64 macros 

/* Custom headers */
# include "struct.h"
# include "functions.h"


/* Macros for flags */
# define FLAG_R  (1 << 0)
# define FLAG_P  (1 << 1)

/* Macros for bits */
# define HAS_ALL_FLAGS(value, mask) (((value) & (mask)) == (mask))
# define HAS_ANY_FLAG(value, mask) (((value) & (mask)) != 0)

/* Global variables */
extern int g_flags;

#endif // FT_NM_H
