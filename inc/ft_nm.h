#ifndef FT_NM_H
# define FT_NM_H

/* 
------- Standard headers --------
!!! Note: Follwing of the subject's allowed all standard libraries in C.
          We allow to use the function like memcpy, memeset, printf, etc.
*/
# include <unistd.h>
# include <stdlib.h> // for qsort
# include <stdio.h> // for perror
# include <error.h> 
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
# include "./libft/libft.h"
# include "./libft/ft_printf.h"


/* Macros for flags */
#define FLAG_R  (1 << 0)
#define FLAG_P  (1 << 1)

/* Global variables */
extern int g_flags;

#endif // FT_NM_H
