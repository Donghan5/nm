#include "ft_nm.h"

bool get_elf_section(const t_file *file, t_symbol **symbols, size_t *count)
{
    const unsigned char *data;

    if (!check_elf_ident(file))
        return (false);
    data = file->f_data;
    if (data[EI_CLASS] == ELFCLASS32)
        return (collect_elf32_section(file, symbols, count));
    if (data[EI_CLASS] == ELFCLASS64)
        return (collect_elf64_section(file, symbols, count));
    return (false);
}