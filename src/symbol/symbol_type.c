#include "../../inc/ft_nm.h"

// Get the symbol type with SYMSHT_SYMTAB
bool get_sym_type(t_file *elf)
{
    t_section_table table;
    Elf32_Shdr symtab;
    Elf32_Shdr strtab;
    
    if (get_elf32_section_table(file, &table) < 0)
        return (false);
    if (!read_elf32_section(&table, index, &symtab))
        return (false);
    if (symtab.sh_type != SHT_SYMTAB)
        return (false);
    if (!read_elf32_section(&table, index, &strtab))
        return (false);
    if (strtab.sh_type != SHT_STRTAB)
        return (false);
    return (true);
}

// check sh_link and search connected string table for symbol type
bool check_sym_type(t_symbol *sym, t_file *elf)
{
    if (sym->shdr->sh_link < elf->ehdr->e_shnum)
    {
        t_shdr *strtab = &elf->shdr[sym->shdr->sh_link];
        if (strtab->sh_type == SHT_STRTAB)
        {
            if (sym->st_name < strtab->sh_size)
                return (true);
        }
    }
    return (false);
}

// sh_entsize, size of table and entry count relationship I'm going to divide it to 32 and 64
bool check_sym_type_32(t_symbol *sym, t_file *elf)
{
    if (sym->shdr->sh_entsize != sizeof(Elf32_Sym))
        return (false);

    if (sym->shdr->sh_entsize > 0)
    {
        size_t entry_count = sym->shdr->sh_size / sym->shdr->sh_entsize;
        if (entry_count > 0)
            return (true);
    }
    return (false);
}

bool check_sym_type_64(t_symbol *sym, t_file *elf)
{
    if (sym->shdr->sh_entsize != sizeof(Elf64_Sym))
        return (false);

    if (sym->shdr->sh_entsize > 0)
    {
        size_t entry_count = sym->shdr->sh_size / sym->shdr->sh_entsize;
        if (entry_count > 0)
            return (true);
    }
    return (false);
}

// range of st_name and string table size
bool check_sym_name_range(t_symbol *sym, t_file *elf)
{
    if (sym->shdr->sh_link < elf->ehdr->e_shnum)
    {
        t_shdr *strtab = &elf->shdr[sym->shdr->sh_link];
        if (strtab->sh_type == SHT_STRTAB)
        {
            if (sym->st_name < strtab->sh_size)
                return (true);
        }
    }
    return (false);
}