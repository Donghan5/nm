#ifndef STRUCT_H
# define STRUCT_H

typedef enum e_file_status
{
	FILE_ERROR = -1,
	FILE_EMPTY = -2,
	FILE_DIRECTORY = -3
} t_file_status;

/* Structure to hold symbol information */
typedef struct s_symbol 
{
    size_t s_original_index;

    uint64_t s_arch;
    uint64_t s_type;
    uint64_t s_bind;
    uint64_t s_shndx;
    uint64_t s_addr;
    uint64_t s_size;

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

/* Section table structure for elf32 and elf64 */
typedef struct s_section_table
{
    const t_file    *file;
    size_t          offset;
    size_t          count;
}   t_section_table;

/** String table structure for symbol types (tables) */
typedef struct s_string_table
{
    const char  *data;
    size_t      size;
}   t_string_table;

#endif // STRUCT_H
