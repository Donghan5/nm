#include "../../inc/ft_nm.h"

static bool check_elf64_program_header_size(const t_file *elf)
{
    const unsigned char *data;
    Elf64_Ehdr header;
    Elf64_Shdr section0;
    size_t  count;
    
    if (!check_common_range(elf, 0, sizeof(Elf64_Ehdr)))
        return (false);

    data = elf->f_data;
    ft_memcpy(&header, data, sizeof(header));
    count = header.e_phnum;
    if (count == 0)
        return (true);

    if (header.e_phentsize != sizeof(Elf64_Phdr))
        return (false);
    if (header.e_phoff == 0)
        return (false);

    if (count == PN_XNUM)
    {
        if (header.e_shoff == 0)
            return (false);
        if (header.e_shentsize != sizeof(Elf64_Shdr))
            return (false);
        if (!check_common_range(elf, header.e_shoff, sizeof(section0)))
            return (false);

        ft_memcpy(&section0, data + header.e_shoff, sizeof(section0));
        count = section0.sh_info;
        if (count < PN_XNUM)
            return (false);
    }

    if (count > SIZE_MAX / sizeof(Elf64_Phdr))
        return (false);

    return (check_common_range(elf, header.e_phoff, count * sizeof(Elf64_Phdr)));
}

static bool check_elf64_header_fields(const t_file *elf)
{
    if (!check_common_range(elf, 0, sizeof(Elf64_Ehdr)))
        return (false);

    Elf64_Ehdr header;
    ft_memcpy(&header, elf->f_data, sizeof(header));

    if (header.e_ehsize != sizeof(Elf64_Ehdr))
        return (false);
    if (header.e_shoff != 0)
    {
        if (header.e_shentsize != sizeof(Elf64_Shdr))
            return (false);
    }
    if (header.e_version != EV_CURRENT)
        return (false);
    if (header.e_type != ET_EXEC && header.e_type != ET_DYN && header.e_type != ET_REL)
        return (false);
    
    // 64 bits check.
    if (header.e_machine != EM_X86_64)
        return (false);
    return (true);
}


bool read_elf64_section(const t_section_table *table, size_t index, Elf64_Shdr *section)
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
    ft_memcpy(section, data + offset, sizeof(*section));
    return (true);
}

/* Success return 0, failure return -1 */
int get_elf64_section_table(const t_file *elf, t_section_table *section_table)
{
    const unsigned char *data;
    Elf64_Ehdr header;
    Elf64_Shdr section0;
    size_t count;

    if (!elf || !section_table || !elf->f_data)
        return (-1);
    if (!check_elf_ident(elf))
        return (-1);

    data = elf->f_data;
    if (data[EI_CLASS] != ELFCLASS64)
        return (-1);
    if (data[EI_DATA] != ELFDATA2LSB)
        return (-1);

    if (!check_elf64_header_fields(elf))
        return (-1);
    ft_memcpy(&header, data, sizeof(header));

    if (header.e_shoff > SIZE_MAX || header.e_phoff > SIZE_MAX)
        return (-1);
    if (!check_elf64_program_header_size(elf))
        return (-1);

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
        ft_memcpy(&section0, data + header.e_shoff, sizeof(section0));
        if (section0.sh_size > SIZE_MAX)
            return (-1);
        count = section0.sh_size;
        if (count < SHN_LORESERVE)
            return (-1);
    }
    if (count > SIZE_MAX / sizeof(Elf64_Shdr))
        return (-1);
    if (!check_common_range(elf, header.e_shoff, count * sizeof(Elf64_Shdr)))
        return (-1);

    section_table->count = count;
    section_table->offset = header.e_shoff;
    section_table->file = elf;
    return (0);
}