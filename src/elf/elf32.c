#include "../../inc/ft_nm.h"

int check_elf32_program_header_size(const t_file *elf)
{
    if (!check_common_range(elf, 0, sizeof(Elf32_Ehdr)))
        return (0);

    const Elf32_Ehdr *ehdr;
    ehdr = (const Elf32_Ehdr *)elf->f_data;

    if (!check_common_range(elf, ehdr->e_phoff, ehdr->e_phnum * sizeof(Elf32_Phdr)))
        return (0);
    return (1);
}

int check_elf32_header_fields(const t_file *elf)
{
    if (!check_common_range(elf, 0, sizeof(Elf32_Ehdr)))
        return (0);

    const Elf32_Ehdr *ehdr;
    ehdr = (const Elf32_Ehdr *)elf->f_data;

    if (ehdr->e_ehsize != sizeof(Elf32_Ehdr))
        return (0);
    if (ehdr->e_shentsize != sizeof(Elf32_Shdr))
        return (0);
    if (ehdr->e_version != EV_CURRENT)
        return (0);
    if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN && ehdr->e_type != ET_REL)
        return (0);
    
    // 32 bits check.
    if (ehdr->e_machine != EM_386)
        return (0);
    return (1);
}

int check_elf32_section_table(const t_file *elf)
{
    if (!check_common_range(elf, 0, sizeof(Elf32_Ehdr)))
        return (0);

    const Elf32_Ehdr *ehdr;
    int offset;
    int num_entries;
    ehdr = (const Elf32_Ehdr *)elf->f_data;
    

    if (!check_common_range(elf, ehdr->e_shoff, ehdr->e_shnum * sizeof(Elf32_Shdr)))
        return (0);

    if (!check_elf32_program_header_size(elf))
        return (0);
    
    if (!check_elf32_header_fields(elf))
        return (0);

    if (ehdr->e_shentsize != sizeof(Elf32_Shdr))
        return (0);
    return (1);
} 

int get_elf32_section_table(const t_file *elf, const Elf32_Shdr **section_table)
{
    if (!check_elf32_section_table(elf))
        return (0);

    const Elf32_Ehdr *ehdr;
    ehdr = (const Elf32_Ehdr *)elf->f_data;

    *section_table = (const Elf32_Shdr *)(elf->f_data + ehdr->e_shoff);
    return (1);
}