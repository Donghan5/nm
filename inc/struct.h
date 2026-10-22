#ifndef STRUCT_H
# define STRUCT_H

/* Structure to hold symbol information */
typedef struct s_symbol 
{
    size_t s_original_index;

    uint64_t s_arch;

    uint64_t s_type;
    uint64_t s_bind;
    uint64_t s_shndx;
    
    uintptr_t s_addr;
    char s_name[PATH_MAX];
    char s_code;
} t_symbol;

/* Structure to hold file information */
typedef struct s_file
{
	char f_name[PATH_MAX];
	void *f_data;
	size_t f_size;
	uint8_t f_type;
} t_file;

typedef struct s_section_table
{
    const t_file    *file;
    size_t          offset;
    size_t          count;
}   t_section_table;

#endif // STRUCT_H