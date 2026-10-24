#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ft_nm.h"

bool get_elf_section(const t_file *file, t_symbol **symbols, size_t *count);

#if DAY3_BITS == 32
# define EH Elf32_Ehdr
# define SH Elf32_Shdr
# define SYM Elf32_Sym
# define CLASS ELFCLASS32
# define MACHINE EM_386
# define GET_STRING get_elf32_string
# define READ_SYM read_elf32_symbol
# define VALUE UINT64_C(0x12345678)
#else
# define EH Elf64_Ehdr
# define SH Elf64_Shdr
# define SYM Elf64_Sym
# define CLASS ELFCLASS64
# define MACHINE EM_X86_64
# define GET_STRING get_elf64_string
# define READ_SYM read_elf64_symbol
# define VALUE UINT64_C(0x100001234)
#endif

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return (1);
}

static int real_object(const char *path)
{
    FILE *stream = fopen(path, "rb");
    t_file file = {0};
    t_symbol *symbols = NULL;
    size_t count = 0, i;
    long length;
    int result = 0;

    if (!stream) return (fail("cannot open real object"));
    if (fseek(stream, 0, SEEK_END) || (length = ftell(stream)) < 0
        || fseek(stream, 0, SEEK_SET))
    {
        fclose(stream);
        return (fail("cannot measure real object"));
    }
    file.f_size = (size_t)length;
    file.f_data = malloc(file.f_size);
    if (!file.f_data || fread(file.f_data, 1, file.f_size, stream) != file.f_size)
    {
        fclose(stream);
        free(file.f_data);
        return (fail("cannot read real object"));
    }
    fclose(stream);
    if (!get_elf_section(&file, &symbols, &count)) result = fail("real object collection failed");
    else for (i = 0; i < count; i++)
    {
        const unsigned char *name = (const unsigned char *)symbols[i].s_name;
        printf("%zu %llu %llu %llu %llu %llu ", symbols[i].s_original_index,
            (unsigned long long)symbols[i].s_arch, (unsigned long long)symbols[i].s_type,
            (unsigned long long)symbols[i].s_bind, (unsigned long long)symbols[i].s_shndx,
            (unsigned long long)symbols[i].s_addr);
        while (*name) printf("%02x", *name++);
        putchar('\n');
    }
    free(symbols);
    free(file.f_data);
    return (result);
}

int main(int argc, char **argv)
{
    EH header = {0};
    SH sections[4] = {{0}};
    SYM entries[3] = {{0}}, scratch;
    t_file file = {0};
    t_symbol *symbols = NULL;
    unsigned char *data;
    size_t count = 0, i;
    const size_t table_offset = sizeof(EH) + 1;
    const size_t symbol_offset = table_offset + sizeof(sections);
    const size_t string_offset = symbol_offset + sizeof(entries);
    const size_t string_size = PATH_MAX + 2;
    const size_t total = string_offset + string_size;
    const char *name;
    bool expected = true;
    int result = 0;

    if (argc == 3 && !strcmp(argv[1], "object")) return (real_object(argv[2]));
    if (argc != 2) return (fail("expected case name or object PATH"));
    name = argv[1];
    data = calloc(1, total);
    if (!data) return (fail("fixture allocation failed"));
    memcpy(header.e_ident, ELFMAG, SELFMAG);
    header.e_ident[EI_CLASS] = CLASS;
    header.e_ident[EI_DATA] = ELFDATA2LSB;
    header.e_ident[EI_VERSION] = EV_CURRENT;
    header.e_type = ET_REL;
    header.e_machine = MACHINE;
    header.e_version = EV_CURRENT;
    header.e_ehsize = sizeof(EH);
    header.e_shoff = table_offset;
    header.e_shentsize = sizeof(SH);
    header.e_shnum = 4;
    /* No section-name table: discovery must use sh_type, not names. */
    sections[1].sh_type = SHT_PROGBITS;
    sections[2].sh_type = SHT_SYMTAB;
    sections[2].sh_offset = symbol_offset;
    sections[2].sh_size = sizeof(entries);
    sections[2].sh_entsize = sizeof(SYM);
    sections[2].sh_link = 3;
    sections[2].sh_info = 1;
    sections[3].sh_type = SHT_STRTAB;
    sections[3].sh_offset = string_offset;
    sections[3].sh_size = string_size;
    entries[1].st_name = 1;
    entries[1].st_value = VALUE;
    entries[1].st_info = (STB_GLOBAL << 4) | STT_FUNC;
    entries[1].st_shndx = 1;
    entries[2].st_name = 7;
    entries[2].st_info = (STB_WEAK << 4) | STT_OBJECT;
    entries[2].st_shndx = SHN_UNDEF;
    memcpy(data + string_offset, "\0alpha\0beta\0", 12);
    file.f_data = data;
    file.f_size = total;

    if (!strcmp(name, "abs")) entries[1].st_shndx = SHN_ABS;
    else if (!strcmp(name, "common")) entries[1].st_shndx = SHN_COMMON;
    else if (!strcmp(name, "local")) entries[1].st_info = (STB_LOCAL << 4) | STT_OBJECT;
    else if (!strcmp(name, "missing")) sections[2].sh_type = SHT_NULL;
    else if (!strcmp(name, "dynsym-only")) sections[2].sh_type = SHT_DYNSYM;
    else if (!strcmp(name, "link-oob")) sections[2].sh_link = 4, expected = false;
    else if (!strcmp(name, "link-type")) sections[2].sh_link = 1, expected = false;
    else if (!strcmp(name, "entry-zero")) sections[2].sh_entsize = 0, expected = false;
    else if (!strcmp(name, "entry-wrong")) sections[2].sh_entsize++, expected = false;
    else if (!strcmp(name, "entry-partial")) sections[2].sh_size--, expected = false;
    else if (!strcmp(name, "symbol-payload")) sections[2].sh_offset = total, expected = false;
    else if (!strcmp(name, "string-payload")) sections[3].sh_offset = total, expected = false;
    else if (!strcmp(name, "name-oob")) entries[2].st_name = string_size, expected = false;
    else if (!strcmp(name, "name-no-nul"))
        entries[2].st_name = string_size - 1, data[total - 1] = 'X', expected = false;
    else if (!strcmp(name, "section-oob")) entries[2].st_shndx = 4, expected = false;
    else if (!strcmp(name, "xindex-unsupported")) entries[2].st_shndx = SHN_XINDEX, expected = false;
    else if (!strcmp(name, "name-max-fit") || !strcmp(name, "name-too-long"))
    {
        size_t len = !strcmp(name, "name-max-fit") ? PATH_MAX - 1 : PATH_MAX;
        memset(data + string_offset + 1, 'a', len);
        data[string_offset + 1 + len] = 0;
        entries[2].st_name = 0;
        expected = len < PATH_MAX;
    }
#if DAY3_BITS == 64
    else if (!strcmp(name, "huge-offset")) sections[2].sh_offset = UINT64_MAX, expected = false;
    else if (!strcmp(name, "huge-size")) sections[2].sh_size = UINT64_MAX, expected = false;
#endif
    memcpy(data, &header, sizeof(header));
    memcpy(data + table_offset, sections, sizeof(sections));
    memcpy(data + symbol_offset, entries, sizeof(entries));

    if (!strcmp(name, "reader-contract"))
    {
        if (!READ_SYM(&file, &sections[2], 1, &scratch) || memcmp(&scratch, &entries[1], sizeof(scratch))
            || READ_SYM(&file, &sections[2], 3, &scratch)
            || READ_SYM(&file, &sections[2], SIZE_MAX, &scratch)) result = fail("reader bytes/index");
        sections[2].sh_entsize = 0;
        if (READ_SYM(&file, &sections[2], 0, &scratch)) result = fail("reader entry validation");
    }
    else if (!strcmp(name, "string-contract"))
    {
        const char *value = GET_STRING(&file, &sections[3], 1);
        if (!value || strcmp(value, "alpha") || !GET_STRING(&file, &sections[3], 0)
            || GET_STRING(&file, &sections[3], string_size)) result = fail("string boundaries");
        data[total - 1] = 'X';
        if (GET_STRING(&file, &sections[3], string_size - 1)) result = fail("unterminated string");
    }
    else
    {
        bool ok = get_elf_section(&file, &symbols, &count);
        if (ok != expected) result = fail("unexpected collection status");
        else if (!ok && (symbols != NULL || count != 0)) result = fail("failure must release and reset outputs");
        else if (ok && (!strcmp(name, "missing") || !strcmp(name, "dynsym-only")))
        {
            if (symbols || count) result = fail("no regular symtab must return an empty result");
        }
        else if (ok)
        {
            if (!symbols || count != 3) result = fail("incorrect symbol count");
            else for (i = 0; i < count; i++)
            {
                const char *expected_name = (const char *)data + string_offset + entries[i].st_name;
                if (symbols[i].s_original_index != i || symbols[i].s_arch != DAY3_BITS
                    || symbols[i].s_type != (entries[i].st_info & 15)
                    || symbols[i].s_bind != (entries[i].st_info >> 4)
                    || symbols[i].s_shndx != entries[i].st_shndx
                    || symbols[i].s_addr != entries[i].st_value
                    || strcmp(symbols[i].s_name, expected_name) || symbols[i].s_code != 0)
                    result = fail("collected record differs from original symbol");
            }
        }
    }
    free(symbols);
    free(data);
    return (result);
}
