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
    char code;

    if (!HAS_ALL_FLAGS(section_flags, SHF_ALLOC))
        return ;
    if (HAS_ANY_FLAG(section_flags, SHF_EXECINSTR))
        code = 'T';
    else if (section_type == SHT_NOBITS)
        code = 'B';
    else if (HAS_ANY_FLAG(section_flags, SHF_WRITE))
        code = 'D';
    else
        code = 'R';
    if (symbol->s_bind == STB_LOCAL)
        code += 'a' - 'A';
    symbol->s_code = code;
}

static bool    get_symbol_section_info(const t_symbol *symbol, const t_section_table *section, uint32_t *type, uint64_t *flags)
{
    Elf32_Shdr section32;
    Elf64_Shdr section64;

    if (symbol->s_arch == 32)
    {
        if (!read_elf32_section(section, symbol->s_shndx, &section32))
            return (false);
        *type = section32.sh_type;
        *flags = section32.sh_flags;
    }
    else if (symbol->s_arch == 64)
    {
        if (!read_elf64_section(section, symbol->s_shndx, &section64))
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

    // default to '?' if no type is determined
    symbol->s_code = '?';
    if (set_symbol_type_by_shndx(symbol))
        return (0);
    if (!get_symbol_section_info(symbol, section, &type, &flags))
        return (-1);
    set_symbol_type_by_section(symbol, type, flags);
    return (0);
}
