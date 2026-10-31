#include "ft_nm.h"

static bool check_elf32_program_header_size(const t_file *elf)
{
    const unsigned char *data;
    Elf32_Ehdr header;
    Elf32_Shdr section0;
    size_t  count;
    
    if (!check_common_range(elf, 0, sizeof(Elf32_Ehdr)))
        return (false);

    data = elf->f_data;
    memcpy(&header, data, sizeof(header));
    count = header.e_phnum;
    if (count == 0)
        return (true);

    if (header.e_phentsize != sizeof(Elf32_Phdr))
        return (false);
    if (header.e_phoff == 0)
        return (false);

    if (count == PN_XNUM)
    {
        if (header.e_shoff == 0)
            return (false);
        if (header.e_shentsize != sizeof(Elf32_Shdr))
            return (false);
        if (!check_common_range(elf, header.e_shoff, sizeof(section0)))
            return (false);

        memcpy(&section0, data + header.e_shoff, sizeof(section0));
        count = section0.sh_info;
        if (count < PN_XNUM)
            return (false);
    }

    if (count > SIZE_MAX / sizeof(Elf32_Phdr))
        return (false);

    return (check_common_range(elf, header.e_phoff, count * sizeof(Elf32_Phdr)));
}

static bool check_elf32_header_fields(const t_file *elf)
{
    if (!check_common_range(elf, 0, sizeof(Elf32_Ehdr)))
        return (false);

    Elf32_Ehdr header;
    memcpy(&header, elf->f_data, sizeof(header));

    if (header.e_ehsize != sizeof(Elf32_Ehdr))
        return (false);
    if (header.e_shoff != 0)
    {
        if (header.e_shentsize != sizeof(Elf32_Shdr))
            return (false);
    }
    if (header.e_version != EV_CURRENT)
        return (false);
    if (header.e_type != ET_EXEC && header.e_type != ET_DYN && header.e_type != ET_REL)
        return (false);
    
    // 32 bits check.
    if (header.e_machine != EM_386)
        return (false);
    return (true);
}

bool read_elf32_section(const t_section_table *table, size_t index, Elf32_Shdr *section)
{
    const unsigned char *data;
    size_t offset;

    if (!table || !table->file || !section)
        return (false);
    if (index >= table->count)
        return (false);
    if (table->count > SIZE_MAX / sizeof(*section))
        return (false);
    if (!check_common_range(table->file, table->offset, table->count * sizeof(*section)))
        return (false);

    offset = table->offset + index * sizeof(*section);
    data = table->file->f_data;
    memcpy(section, data + offset, sizeof(*section));
    return (true);
}

const char  *get_elf32_string(const t_file *elf, const Elf32_Shdr *strtab, size_t name_offset)
{
    const char *data;
    size_t i;

    if (!elf || !strtab || !elf->f_data)
        return (NULL);
    if (strtab->sh_type != SHT_STRTAB)
        return (NULL);
    if (!check_common_range(elf, (size_t)strtab->sh_offset, strtab->sh_size))
        return (NULL);
    if (name_offset >= strtab->sh_size)
        return (NULL);

    data = (const char *)elf->f_data + strtab->sh_offset;
    i = name_offset;
    while (i < strtab->sh_size)
    {
        if (data[i] == '\0')
            return (data + name_offset);
        i++;
    }
    return (NULL);
}

bool read_elf32_symbol(const t_file *file, const Elf32_Shdr *symtab, size_t sym_index, Elf32_Sym *sym)
{
    const unsigned char *data;
    size_t offset;

    if (!file || !symtab || !sym || !file->f_data)
        return (false);
    if (symtab->sh_type != SHT_SYMTAB)
        return (false);
    if (symtab->sh_entsize != sizeof(*sym))
        return (false);
    if (symtab->sh_size % sizeof(*sym) != 0)
        return (false);
    if (sym_index >= symtab->sh_size / sizeof(Elf32_Sym))
        return (false);
    if (!check_common_range(file, (size_t)symtab->sh_offset, symtab->sh_size))
        return (false);

    offset = symtab->sh_offset + sym_index * sizeof(Elf32_Sym);
    data = file->f_data;
    memcpy(sym, data + offset, sizeof(*sym));
    return (true);
}

bool check_elf32_section_names(const Elf32_Ehdr *header, t_section_table *section_table)
{
    Elf32_Shdr shstrtab;
    const unsigned char *data;
    size_t index;

    index = header->e_shstrndx;
    if (index == SHN_UNDEF)
        return (true);

    if (index == SHN_XINDEX)
    {
        if (!read_elf32_section(section_table, 0, &shstrtab))
            return (false);
        index = shstrtab.sh_link;
        if (index < SHN_LORESERVE)
            return (false);
    }
    if (!read_elf32_section(section_table, index, &shstrtab))
        return (false);
    if (!check_common_range(section_table->file, shstrtab.sh_offset, shstrtab.sh_size))
        return (false);

    if (shstrtab.sh_type != SHT_STRTAB)
        return (false);

    index = 0;
    if (shstrtab.sh_size > 0)
    {
        data = (const unsigned char *)section_table->file->f_data + shstrtab.sh_offset;
        if (data[0] != '\0' || data[shstrtab.sh_size - 1] != '\0')
            return (false);
    }
    while (index < section_table->count)
    {
        // check strtab section name
        Elf32_Shdr section;

        if (!read_elf32_section(section_table, index, &section))
            return (false);
        if (shstrtab.sh_size == 0)
        {
            if (section.sh_name != 0)
                return (false);
        }
        else if (!get_elf32_string(section_table->file, &shstrtab, section.sh_name))
            return (false);
            
        index++;
    }
    return (true);
}

/* Success return 0, failure return -1 */
int get_elf32_section_table(const t_file *elf, t_section_table *section_table)
{
    const unsigned char *data;
    Elf32_Ehdr header;
    Elf32_Shdr section0;
    size_t count;

    if (!elf || !section_table || !elf->f_data)
        return (-1);
    if (!check_elf_ident(elf))
        return (-1);

    data = elf->f_data;
    if (data[EI_CLASS] != ELFCLASS32)
        return (-1);
    if (data[EI_DATA] != ELFDATA2LSB)
        return (-1);

    if (!check_elf32_header_fields(elf))
        return (-1);
    if (!check_elf32_program_header_size(elf))
        return (-1);

    memcpy(&header, data, sizeof(header));
    count = header.e_shnum;
    if (count >= SHN_LORESERVE)
        return (-1);

    if (header.e_shoff == 0)
    {
        if (header.e_shnum != 0)
            return (-1);
        section_table->count = 0;
        section_table->offset = 0;
        section_table->file = elf;
        return (0);
    }
    
    if (count == 0)
    {
        if (!check_common_range(elf, header.e_shoff, sizeof(section0)))
            return (-1);
        memcpy(&section0, data + header.e_shoff, sizeof(section0));
        count = section0.sh_size;
        if (count < SHN_LORESERVE)
            return (-1);
    }

    if (count > SIZE_MAX / sizeof(Elf32_Shdr))
        return (-1);
    if (!check_common_range(elf, header.e_shoff, count * sizeof(Elf32_Shdr)))
        return (-1);

    section_table->count = count;
    section_table->offset = header.e_shoff;
    section_table->file = elf;
    if (!check_elf32_section_names(&header, section_table))
        return (-1);
    return (0);
}