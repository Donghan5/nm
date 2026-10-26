#include "ft_nm.h"

static void    check_and_set_weak_symbol(t_symbol *symbol)
{
    if (symbol->s_type == STT_OBJECT)
    {
        if (symbol->s_shndx == SHN_UNDEF)
            symbol->s_code = 'v';
        else
            symbol->s_code = 'V';
    }
    else
    {
        if (symbol->s_shndx == SHN_UNDEF)
            symbol->s_code = 'w';
        else
            symbol->s_code = 'W';
    }
}

static bool    set_symbol_type_by_shndx(t_symbol *symbol)
{
    if (symbol->s_bind == STB_WEAK)
        check_and_set_weak_symbol(symbol);
    else if (symbol->s_shndx == SHN_UNDEF)
        symbol->s_code = 'U';
    else if (symbol->s_shndx == SHN_ABS)
    {
        if (symbol->s_bind == STB_LOCAL)
            symbol->s_code = 'a';
        else
            symbol->s_code = 'A';
    }    
    else if (symbol->s_shndx == SHN_COMMON)
        symbol->s_code = 'C';
    else
        return (false);
    return (true);
}

static void    set_symbol_type_by_section(t_symbol *symbol, uint32_t section_type, uint64_t section_flags)
{
    if (section_type == SHT_NOBITS && section_flags == (SHF_ALLOC | SHF_WRITE))
    {
        if (symbol->s_bind == STB_LOCAL)
            symbol->s_code = 'b';
        else
            symbol->s_code = 'B';
    }
    
    if (section_type == SHT_PROGBITS && section_flags == (SHF_ALLOC | SHF_WRITE))
    {
        if (symbol->s_bind == STB_LOCAL)
            symbol->s_code = 'd';
        else
            symbol->s_code = 'D';
    }

    if (section_type == SHT_PROGBITS && section_flags == SHF_ALLOC)
    {
        if (symbol->s_bind == STB_LOCAL)
            symbol->s_code = 'r';
        else
            symbol->s_code = 'R';
    }

    if (section_type == SHT_PROGBITS && section_flags == (SHF_ALLOC | SHF_EXECINSTR))
    {
        if (symbol->s_bind == STB_LOCAL)
            symbol->s_code = 't';
        else
            symbol->s_code = 'T';
    }
}

static bool    get_symbol_section_info(const t_symbol *symbol, const t_section_table *section, uint32_t *type, uint64_t *flags)
{
    Elf32_Shdr section32;
    Elf64_Shdr section64;

    if (symbol->s_arch == 32)
    {
        if (!read_elf32_section(table, symbol->s_shndx, &section32))
            return (false);
        *type = section32.sh_type;
        *flags = section32.sh_flags;
    }
    else if (symbol->s_arch == 64)
    {
        if (!read_elf64_section(table, symbol->s_shndx, &section64))
            return (false);
        *type = section64.sh_type;
        *flags = section64.sh_flags;
    }
    else
        return (false);
    return (true);
}

int symbol_type(t_symbol *symbol, t_section_table *section)
{
    uint32_t type;
    uint64_t flags;

    if (set_symbol_type_by_shndx(symbol))
        return (0);
    if (!get_symbol_section_info(symbol, section, &type, &flags))
        return (-1);
    set_symbol_type_by_section(symbol, type, flags);
    return (0);
}