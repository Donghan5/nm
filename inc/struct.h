#include "ft_nm.h"

typedef struct s_elf32_sys
{
    Elf32_Word     st_name;
    Elf32_Addr     st_value;
    Elf32_Word     st_size;
    unsigned char  st_info;
    unsigned char  st_other;
    Elf32_Half     st_shndx;
}   t_elf32_sys;

typedef struct s_elf64_sys
{
    Elf64_Word     st_name;
    unsigned char  st_info;
    unsigned char  st_other;
    Elf64_Half     st_shndx;
    Elf64_Addr     st_value;
    Elf64_Xword    st_size;
} t_elf64_sys;