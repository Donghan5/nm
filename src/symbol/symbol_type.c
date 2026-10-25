#include "../../inc/ft_nm.h"

// input t_symbol -> special input like weak -> s_shndx --> UNDEF / ABS / COMMON
    // -> 32/64 render in general index -> decide default char with sh_type, sh_flag
    // -> local or global in general (T, D, B, R, A)

void    set_symbol_type_by_shndx(t_symbol *symbol, t_section_table *section)
{
    if (symbol->shndx == STB_WEAK)
        check_and_set_weak_symbol(symbol, section);
    else if (symbol->shndx == SHN_UNDEF)
        symbol->type = 'U';
    else if (symbol->shndx == SHN_ABS) // absolute symbol verify upper case and lower case
    {
        if (symbol->bind == STB_LOCAL)
            symbol->type = 'a';
        else
            symbol->type = 'A';
    }    
    else if (symbol->shndx == SHN_COMMON)
        symbol->type = 'C';
    else
        set_symbol_type_by_section(symbol, section);
}

void    check_and_set_weak_symbol(t_symbol *symbol, t_section_table *section)
{
    if (symbol->sh_type == STT_OBJECT)
    {
        if (symbol->sh_shndx == SHN_UNDEF)
            symbol->type = 'v';
        else
            symbol->type = 'V';
    }
    else if (symbol->sh_type == STT_FUNC)
    {
        if (symbol->sh_shndx == SHN_UNDEF)
            symbol->type = 'w';
        else
        symbol->type = 'W';
    }
}

void    set_symbol_type_by_section(t_symbol *symbol, t_section_table *section)
{
    // check section type and flag to determine symbol type T, D, B, R
    if (symbol->sh_type == SHT_NOBITS && symbol->sh_flags == (SHF_ALLOC | SHF_WRITE))
    {
        if (symbol->bind == STB_LOCAL)
            symbol->type = 'b';
        else
            symbol->type = 'B';
    }
    
    if (symbol->sh_type == SHT_PROGBITS && symbol->sh_flags == (SHF_ALLOC | SHF_WRITE))
    {
        if (symbol->bind == STB_LOCAL)
            symbol->type = 'd';
        else
            symbol->type = 'D';
    }

    if (symbol->sh_type == SHT_PROGBITS && symbol->sh_flags == SHF_ALLOC)
    {
        if (symbol->bind == STB_LOCAL)
            symbol->type = 'r';
        else
            symbol->type = 'R';
    }

    if (symbol->sh_type == SHT_PROGBITS && symbol->sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
    {
        if (symbol->bind == STB_LOCAL)
            symbol->type = 't';
        else
            symbol->type = 'T';
    }
}

bool    get_symbol_section_info(const t_symbol *symbol, const t_section_table *section, uint32_t *type, uint64_t *flags)
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

    if (!get_symbol_section_info(symbol, section, &type, &flags))
        return (-1);
    symbol->sh_type = type;
    symbol->sh_flags = flags;
    set_symbol_type_by_shndx(symbol, section);
    return (0);
}