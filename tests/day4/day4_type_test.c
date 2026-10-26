#include "ft_nm.h"
#include <string.h>

/* Public entry point only: helper signatures remain implementation details. */
int symbol_type(t_symbol *symbol, t_section_table *section);

typedef struct s_case
{
    const char *name;
    unsigned bind;
    unsigned type;
    unsigned index;
    unsigned section_type;
    uint64_t flags;
    char code;
    int result;
} t_case;

static const t_case cases[] = {
    {"undefined", STB_GLOBAL, STT_NOTYPE, SHN_UNDEF, 0, 0, 'U', 0},
    {"absolute-global", STB_GLOBAL, STT_NOTYPE, SHN_ABS, 0, 0, 'A', 0},
    {"absolute-local", STB_LOCAL, STT_NOTYPE, SHN_ABS, 0, 0, 'a', 0},
    {"common", STB_GLOBAL, STT_OBJECT, SHN_COMMON, 0, 0, 'C', 0},
    {"weak-object-defined", STB_WEAK, STT_OBJECT, 2, SHT_PROGBITS, 3, 'V', 0},
    {"weak-object-undefined", STB_WEAK, STT_OBJECT, SHN_UNDEF, 0, 0, 'v', 0},
    {"weak-function-defined", STB_WEAK, STT_FUNC, 2, SHT_PROGBITS, 6, 'W', 0},
    {"weak-function-undefined", STB_WEAK, STT_FUNC, SHN_UNDEF, 0, 0, 'w', 0},
    {"weak-notype-defined", STB_WEAK, STT_NOTYPE, 2, SHT_PROGBITS, 6, 'W', 0},
    {"weak-notype-undefined", STB_WEAK, STT_NOTYPE, SHN_UNDEF, 0, 0, 'w', 0},
    {"text-global", STB_GLOBAL, STT_FUNC, 2, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, 'T', 0},
    {"text-local", STB_LOCAL, STT_FUNC, 2, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, 't', 0},
    {"text-notype", STB_GLOBAL, STT_NOTYPE, 2, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, 'T', 0},
    {"data-global", STB_GLOBAL, STT_OBJECT, 2, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, 'D', 0},
    {"data-local", STB_LOCAL, STT_OBJECT, 2, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, 'd', 0},
    {"bss-global", STB_GLOBAL, STT_OBJECT, 2, SHT_NOBITS, SHF_ALLOC | SHF_WRITE, 'B', 0},
    {"bss-local", STB_LOCAL, STT_OBJECT, 2, SHT_NOBITS, SHF_ALLOC | SHF_WRITE, 'b', 0},
    {"readonly-global", STB_GLOBAL, STT_OBJECT, 2, SHT_PROGBITS, SHF_ALLOC, 'R', 0},
    {"readonly-local", STB_LOCAL, STT_OBJECT, 2, SHT_PROGBITS, SHF_ALLOC, 'r', 0},
    {"readonly-merge-strings", STB_GLOBAL, STT_OBJECT, 2, SHT_PROGBITS, SHF_ALLOC | SHF_MERGE | SHF_STRINGS, 'R', 0},
    {"text-extra-group", STB_GLOBAL, STT_FUNC, 2, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR | SHF_GROUP, 'T', 0},
    {"tls-data", STB_GLOBAL, STT_TLS, 2, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE | SHF_TLS, 'D', 0},
    {"tls-bss", STB_GLOBAL, STT_TLS, 2, SHT_NOBITS, SHF_ALLOC | SHF_WRITE | SHF_TLS, 'B', 0},
    {"index-out-of-range", STB_GLOBAL, STT_FUNC, 3, SHT_PROGBITS, 6, 0, -1},
    {"index-huge", STB_GLOBAL, STT_FUNC, SHN_LORESERVE - 1, SHT_PROGBITS, 6, 0, -1}
};

int main(int argc, char **argv)
{
    Elf32_Shdr headers32[3] = {0};
    Elf64_Shdr headers64[3] = {0};
    t_file file = {0};
    t_section_table table = {0};
    t_symbol symbol = {0};
    const t_case *test;
    size_t i;
    int result;

    if (argc != 3)
        return (2);
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
        if (strcmp(argv[2], cases[i].name) == 0)
            break;
    if (i == sizeof(cases) / sizeof(cases[0]))
        return (2);
    test = &cases[i];
    symbol.s_arch = (unsigned)atoi(argv[1]);
    symbol.s_bind = test->bind;
    symbol.s_type = test->type;
    symbol.s_shndx = test->index;
    symbol.s_code = '?';
    symbol.s_addr = UINT64_C(0x123456789abcdef0);
    symbol.s_original_index = 17;
    strcpy(symbol.s_name, "preserve-me");
    headers32[2].sh_type = test->section_type;
    headers32[2].sh_flags = test->flags;
    headers64[2].sh_type = test->section_type;
    headers64[2].sh_flags = test->flags;
    file.f_data = symbol.s_arch == 32 ? (void *)headers32 : (void *)headers64;
    file.f_size = symbol.s_arch == 32 ? sizeof(headers32) : sizeof(headers64);
    table.file = &file;
    table.count = 3;
    /* No ELF header is needed: the section reader receives a validated table. */
    result = symbol_type(&symbol, &table);
    if (result != test->result || (result == 0 && symbol.s_code != test->code))
    {
        fprintf(stderr, "%s: expected result=%d code=%c; got result=%d code=%c\n",
            test->name, test->result, test->code ? test->code : '-', result, symbol.s_code);
        return (1);
    }
    if (symbol.s_type != test->type || symbol.s_bind != test->bind
        || symbol.s_shndx != test->index || symbol.s_arch != (unsigned)atoi(argv[1])
        || symbol.s_addr != UINT64_C(0x123456789abcdef0)
        || symbol.s_original_index != 17 || strcmp(symbol.s_name, "preserve-me"))
    {
        fprintf(stderr, "%s: classification changed source symbol metadata\n", test->name);
        return (1);
    }
    return (0);
}
