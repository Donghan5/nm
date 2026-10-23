#include "../../inc/ft_nm.h"

static bool return_false_with_free(t_symbol **symbols, size_t *count)
{
    if (*symbols)
        free(*symbols);
    *symbols = NULL;
    *count = 0;
    return (false);
}

bool collect_elf32_section(const t_file *file, t_symbol **symbols, size_t *count)
{
    t_section_table table;
    Elf32_Shdr symtab;
    Elf32_Shdr strtab;
    size_t  index;
    size_t  entry_count;

    index = 0;
    *count = 0;
    *symbols = NULL;

    if (get_elf32_section_table(file, &table) < 0)
        return (false);
    
    while (index < table.count)
    {
        if (!read_elf32_section(&table, index, &symtab))
            return (false);
        if (symtab.sh_type == SHT_SYMTAB)
        {
            if (!read_elf32_section(&table, symtab.sh_link, &strtab))
                return (false);
            if (strtab.sh_type != SHT_STRTAB)
                return (false);
            if (!check_sym_type_32(file, &symtab, &strtab))
                return (false);
            entry_count = symtab.sh_size / symtab.sh_entsize;
            if (entry_count > SIZE_MAX / sizeof(t_symbol))
                return (false);
            *symbols = malloc(sizeof(t_symbol) * entry_count);
            if (!*symbols)
                return (false);
            (*symbols)[*count].s_original_index = index;;
            (*symbols)[*count].s_arch = 32;
            (*symbols)[*count].s_type = ELF32_ST_TYPE(sym.st_info);
            (*symbols)[*count].s_bind = ELF32_ST_BIND(sym.st_info);
            (*symbols)[*count].s_shndx = sym.st_shndx;
            (*symbols)[*count].s_addr = sym.st_value;
            (*count)++;
            index++;
        }
        index++;
    }
    return (true);
}

bool collect_elf64_section(const t_file *file, t_symbol **symbols, size_t *count)
{
    t_section_table table;
    Elf64_Shdr symtab;
    Elf64_Shdr strtab;
    size_t  index;
    size_t  entry_count;

    index = 0;
    *symbols = NULL;
    *count = 0;

    if (get_elf64_section_table(file, &table) < 0)
        return (false);

    while (index < table.count)
    {
        if (!read_elf64_section(&table, index, &symtab))
            return (false);
        if (symtab.sh_type == SHT_SYMTAB)
        {
            if (!read_elf64_section(&table, symtab.sh_link, &strtab))
                return (false);
            if (strtab.sh_type != SHT_STRTAB)
                return (false);
            if (!check_sym_type_64(file, &symtab, &strtab))
                return (false);
            entry_count = symtab.sh_size / symtab.sh_entsize;
            if (entry_count > SIZE_MAX / sizeof(t_symbol))
                return (false);
            *symbols = malloc(sizeof(t_symbol) * entry_count);
            if (!*symbols)
                return (false);
        }
        index++;
    }
    return (true);
}

// main function to get the symbol type with SYMSHT_SYMTAB
bool get_elf_section(const t_file *file)
{
    const unsigned char *data;

    if (!check_elf_ident(file))
        return (false);
    data = file->f_data;
    if (data[EI_CLASS] == ELFCLASS32)
        return (collect_elf32_section(file));
    if (data[EI_CLASS] == ELFCLASS64)
        return (collect_elf64_section(file));
    return (false);
}