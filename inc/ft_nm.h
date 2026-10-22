#ifndef FT_NM_H
# define FT_NM_H

/* Standard headers */
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h> // for perror
# include <error.h> 
# include <sys/stat.h>
# include <sys/mman.h>
# include <elf.h>
# include <limits.h> // for PATH_MAX
# include <fcntl.h>
# include <stdbool.h> // for bool type

/* Custom headers */
# include "struct.h"
# include "functions.h"
# include "./libft/libft.h"

#endif // FT_NM_H
