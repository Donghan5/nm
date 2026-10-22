#include "../../inc/ft_nm.h"

// return 1 if the range is valid, 0 otherwise
int check_common_range(const t_file *elf, size_t offset, size_t size)
{
    return (offset <= elf->f_size && size <= elf->f_size - offset);
}

// check ELF magic: 0x7f, 'E', 'L', 'F'
int check_elf_magic(const t_file *elf)
{
    if (!check_common_range(elf, 0, EI_NIDENT))
        return (0);

    const unsigned char *data;
    data = elf->f_data;
    
    if (data[EI_MAG0] != ELFMAG0 || data[EI_MAG1] != ELFMAG1
        || data[EI_MAG2] != ELFMAG2 || data[EI_MAG3] != ELFMAG3)
        return (0);
    return (1);
}

// 32/64 bit checks, version checks, bytes order
int check_elf_ident(const t_file *elf)
{
    if (!check_common_range(elf, 0, EI_NIDENT))
        return (0);

    if (!check_elf_magic(elf))
        return (0);

    const unsigned char *data;
    data = elf->f_data;

    if (data[EI_CLASS] != ELFCLASS32 && data[EI_CLASS] != ELFCLASS64)
        return (0);
    if (data[EI_DATA] != ELFDATA2LSB && data[EI_DATA] != ELFDATA2MSB)
        return (0);
    if (data[EI_VERSION] != EV_CURRENT)
        return (0);
    return (1);
}