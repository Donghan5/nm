#include <stdint.h>
#include <string.h>
#include "ft_nm.h"

#if DAY2_BITS == 32
# define EH Elf32_Ehdr
# define SH Elf32_Shdr
# define PH Elf32_Phdr
# define CLASS ELFCLASS32
# define MACHINE EM_386
# define GET get_elf32_section_table
# define READ read_elf32_section
#else
# define EH Elf64_Ehdr
# define SH Elf64_Shdr
# define PH Elf64_Phdr
# define CLASS ELFCLASS64
# define MACHINE EM_X86_64
# define GET get_elf64_section_table
# define READ read_elf64_section
#endif

static int fail(const char *name)
{
    fprintf(stderr, "FAIL: %s\n", name);
    return (1);
}

int main(int argc, char **argv)
{
    EH h = {0};
    SH zero = {0}, marker = {0}, output;
    t_file file = {0};
    t_section_table table = {0};
    unsigned char *bytes;
    const char *name;
    size_t count = 2, size, offset = sizeof(EH) + 1;
    int expected = 0;
    int result;

    if (argc != 2)
        return (fail("expected a case name"));
    name = argv[1];
    memcpy(h.e_ident, ELFMAG, SELFMAG);
    h.e_ident[EI_CLASS] = CLASS;
    h.e_ident[EI_DATA] = ELFDATA2LSB;
    h.e_ident[EI_VERSION] = EV_CURRENT;
    h.e_type = ET_REL;
    h.e_machine = MACHINE;
    h.e_version = EV_CURRENT;
    h.e_ehsize = sizeof(EH);
    h.e_shentsize = sizeof(SH);
    h.e_shoff = offset;
    h.e_shnum = 2;
    if (!strcmp(name, "extended-sections") || !strcmp(name, "extended-truncated"))
    {
        count = SHN_LORESERVE;
        h.e_shnum = 0;
        zero.sh_size = count;
    }
    if (!strcmp(name, "extended-programs"))
    {
        h.e_phnum = PN_XNUM;
        zero.sh_info = PN_XNUM;
    }
    if (!strcmp(name, "program") || !strcmp(name, "program-truncated")
        || !strcmp(name, "program-entry") || !strcmp(name, "program-zero-offset"))
        h.e_phnum = 1;
    size = offset + count * sizeof(SH);
    if (h.e_phnum)
    {
        h.e_phentsize = sizeof(PH);
        h.e_phoff = size;
        size += (size_t)(h.e_phnum == PN_XNUM ? zero.sh_info : h.e_phnum) * sizeof(PH);
    }
    bytes = calloc(1, size);
    if (!bytes)
        return (fail("fixture allocation"));
    marker.sh_type = SHT_PROGBITS;
    marker.sh_name = 7;
    memcpy(bytes + offset, &zero, sizeof(zero));
    memcpy(bytes + offset + sizeof(SH), &marker, sizeof(marker));
    file.f_data = bytes;
    file.f_size = size;

    if (!strcmp(name, "none"))
        h.e_shoff = 0, h.e_shnum = 0, h.e_shentsize = 0;
    else if (!strcmp(name, "exec")) h.e_type = ET_EXEC;
    else if (!strcmp(name, "shared")) h.e_type = ET_DYN;
    else if (!strcmp(name, "bad-magic")) h.e_ident[EI_MAG0] = 0, expected = -1;
    else if (!strcmp(name, "wrong-class")) h.e_ident[EI_CLASS] = CLASS == ELFCLASS32 ? ELFCLASS64 : ELFCLASS32, expected = -1;
    else if (!strcmp(name, "big-endian")) h.e_ident[EI_DATA] = ELFDATA2MSB, expected = -1;
    else if (!strcmp(name, "header-size")) h.e_ehsize = 0, expected = -1;
    else if (!strcmp(name, "header-version")) h.e_version = 0, expected = -1;
    else if (!strcmp(name, "machine")) h.e_machine = EM_NONE, expected = -1;
    else if (!strcmp(name, "type")) h.e_type = ET_NONE, expected = -1;
    else if (!strcmp(name, "section-entry")) h.e_shentsize = 1, expected = -1;
    else if (!strcmp(name, "section-truncated") || !strcmp(name, "extended-truncated")) file.f_size--, expected = -1;
    else if (!strcmp(name, "section-offset")) h.e_shoff = size + 1, expected = -1;
    else if (!strcmp(name, "zero-offset-count")) h.e_shoff = 0, expected = -1;
    else if (!strcmp(name, "reserved-count")) h.e_shnum = SHN_LORESERVE, expected = -1;
    else if (!strcmp(name, "extended-small")) h.e_shnum = 0, expected = -1;
    else if (!strcmp(name, "program-truncated")) file.f_size--, expected = -1;
    else if (!strcmp(name, "program-entry")) h.e_phentsize = 1, expected = -1;
    else if (!strcmp(name, "program-zero-offset")) h.e_phoff = 0, expected = -1;
    else if (!strcmp(name, "short-header")) file.f_size = sizeof(EH) - 1, expected = -1;
#if DAY2_BITS == 64
    else if (!strcmp(name, "huge-offset")) h.e_shoff = UINT64_MAX, expected = -1;
    else if (!strcmp(name, "huge-count"))
    {
        h.e_shnum = 0;
        zero.sh_size = UINT64_MAX;
        memcpy(bytes + offset, &zero, sizeof(zero));
        expected = -1;
    }
#endif
    memcpy(bytes, &h, sizeof(h));
    if (!strcmp(name, "null-args"))
        result = GET(NULL, &table) == -1 && GET(&file, NULL) == -1 ? 0 : 1;
    else
    {
        result = GET(&file, &table);
        if (result != expected)
            result = fail("getter return value" );
        else if (expected == -1)
            result = 0;
        else if (!strcmp(name, "none"))
            result = table.count != 0 || table.offset != 0 || READ(&table, 0, &output);
        else
        {
            result = table.file != &file || table.offset != offset || table.count != count;
            if (!result && (!READ(&table, 1, &output) || memcmp(&output, &marker, sizeof(output))))
                result = fail("reader bytes");
            if (!result && (!READ(&table, count - 1, &output) || READ(&table, count, &output)
                || READ(&table, SIZE_MAX, &output) || READ(NULL, 0, &output) || READ(&table, 0, NULL)))
                result = fail("reader boundaries");
            table.offset = SIZE_MAX;
            if (!result && READ(&table, 0, &output)) result = fail("forged offset");
            table.offset = offset;
            table.count = SIZE_MAX;
            if (!result && READ(&table, 0, &output)) result = fail("forged count");
        }
    }
    free(bytes);
    return (result ? fail(name) : 0);
}
