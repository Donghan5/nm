#ifndef FT_NM_H
# define FT_NM_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h> // for perror
# include <error.h> 
# include <sys/stat.h>
# include <sys/mman.h>
# include <elf.h>
# include <fcntl.h>

# include "libft/libft.h"

struct s_symbol 
{
    size_t s_original_index;

    uint64_t s_arch;

    uint64_t s_type;
    uint64_t s_bind;
    uint64_t s_shndx;
    
    uintptr_t s_addr;
    char s_name[PATH_MAX];
    char s_code;
};

struct s_file
{
	char *f_name[PATH_MAX];
	void *f_data;
	size_t f_size;
	uint8_t f_type;
} t_file;


#endif
