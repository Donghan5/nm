#include "../../inc/ft_nm.h"

// getting section character by 32 / 64 reader
char get_section_character_32(const t_file *file, const Elf32_Shdr *section)
{
    if (!file || !section)
        return ('\0');
    if (section->sh_flags & SHF_EXECINSTR)
    {
        if (/* global symbol */)
            return ('T');
        else
            return ('t');
    }
    if (section->sh_flags & SHF_WRITE)
    {
        if (/* global symbol */)
            return ('D');
        else
            return ('d');
    }
    if (section->sh_flags & SHF_ALLOC)
    {
        if (/* global symbol */)
            return ('B');
        else
            return ('b');
    }
    if (section->sh_type == SHT_REL)
    {
        if (/* global symbol */)
            return ('R');
        else
            return ('r');
    }
    if (section->sh_type == /* undefined section */)
        return ('U');
    if (section->sh_type == /* absolute section */)
    {
        if (/* global symbol */)
            return ('A');
        else
            return ('a');
    }
    if (section->sh_type == /* common section */)
        return ('C');
    return ('\0');
}

char get_section_character_64(const t_file *file, const Elf64_Shdr *section)
{
    if (!file || !section)
        return ('\0');
    if (/* Code Section */)
    {
        if (/* global symbol */)
            return ('T');
        else
            return ('t');
    }
    if (/* Data Section (initialized)*/)
    {
        if (/* global symbol */)
            return ('D');
        else
            return ('d');
    }
    if (/* BSS Section */)
    {
        if (/* global symbol */)
            return ('B');
        else
            return ('b');
    }
    if (/* Read only Section */)
    {
        if (/* global symbol */)
            return ('R');
        else
            return ('r');
    }
    if (/* undefined section */)
        return ('U');
    if (/* absolute section */)
    {
        if (/* global symbol */)
            return ('A');
        else
            return ('a');
    }
    if (/* common section */)
        return ('C');
    return ('\0');
}

// Special symbol like weak
bool is_special_symbol(uint64_t type, uint64_t bind)
{
    if (type == STT_NOTYPE && bind == STB_WEAK)
        return (true);
    if (type == STT_OBJECT && bind == STB_WEAK)
        return (true);
    return (false);
}

// Special section index: undefined, absolute, common (have to add?)

// general section: executable writable nobits

// local / global

// main entrance for this file