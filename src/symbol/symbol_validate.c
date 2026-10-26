#include "ft_nm.h"

// Get the symbol type with SYMSHT_SYMTAB
int get_sym_type_32(t_file *elf)
{
    t_section_table table;
    Elf32_Shdr symtab;
    Elf32_Shdr strtab;
    size_t  index;
    
    if (get_elf32_section_table(elf, &table) < 0)
        return (-1);

    index = 0;
    while (index < table.count)
    {
        if (!read_elf32_section(&table, index, &symtab))
            return (-1);
        if (symtab.sh_type == SHT_SYMTAB)
        {
            if (!read_elf32_section(&table, symtab.sh_link, &strtab))
                return (-1);
            if (strtab.sh_type != SHT_STRTAB)
                return (-1);
            return (0);
        }
        index++;
    }
    return (-1);
}

int get_sym_type_64(t_file *elf)
{
    t_section_table table;
    Elf64_Shdr symtab;
    Elf64_Shdr strtab;
    size_t  index;
    
    if (get_elf64_section_table(elf, &table) < 0)
        return (-1);

    index = 0;
    while (index < table.count)
    {
        if (!read_elf64_section(&table, index, &symtab))
            return (-1);
        if (symtab.sh_type == SHT_SYMTAB)
        {
            if (!read_elf64_section(&table, symtab.sh_link, &strtab))
                return (-1);
            if (strtab.sh_type != SHT_STRTAB)
                return (-1);
            return (0);
        }
        index++;
    }
    return (-1);
}

// check sh_link and search connected string table for symbol type
bool check_sym_type_32(const t_file *elf, const Elf32_Shdr *symtab, const Elf32_Shdr *strtab)
{
    if (symtab->sh_type != SHT_SYMTAB || strtab->sh_type != SHT_STRTAB)
        return (false);
    if (!check_common_range(elf, symtab->sh_offset, symtab->sh_size) \
        || !check_common_range(elf, strtab->sh_offset, strtab->sh_size))
        return (false);
    if (symtab->sh_entsize != sizeof(Elf32_Sym))
        return (false);
    if (symtab->sh_size % symtab->sh_entsize != 0)
        return (false);
    if (symtab->sh_size == 0 || strtab->sh_size == 0)
        return (false);
    return (true);
}


// sh_entsize, size of table and entry count relationship I'm going to divide it to 32 and 64
bool check_sym_type_64(const t_file *elf, const Elf64_Shdr *symtab, const Elf64_Shdr *strtab)
{
    if (symtab->sh_type != SHT_SYMTAB || strtab->sh_type != SHT_STRTAB)
        return (false);
    if (symtab->sh_offset > SIZE_MAX || symtab->sh_size > SIZE_MAX \
        || strtab->sh_offset > SIZE_MAX || strtab->sh_size > SIZE_MAX)
        return (false);
    if (!check_common_range(elf, symtab->sh_offset, symtab->sh_size) \
        || !check_common_range(elf, strtab->sh_offset, strtab->sh_size))
        return (false);
    if (symtab->sh_entsize != sizeof(Elf64_Sym))
        return (false);
    if (symtab->sh_size % symtab->sh_entsize != 0)
        return (false);
    if (symtab->sh_size == 0 || strtab->sh_size == 0)
        return (false);
    return (true);
}

// range of st_name and string table size
bool check_sym_name_range(const t_string_table *strtab, size_t name_offset)
{
    size_t i;

    if (name_offset >= strtab->size)
        return (false);
    i = name_offset;
    while (i < strtab->size)
    {
        if (strtab->data[i] == '\0')
            return (true);
        i++;
    }
    return (false);
}