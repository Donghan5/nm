#include "ft_nm.h"

static bool check_symbols_print(const t_symbol *symbols)
{
    if (symbols->s_name[0] != '\0' && symbols->s_type != STT_FILE && symbols->s_type != STT_SECTION)
        return (true);
    return (false);
}

/*
    Modification of the empty name and STT_FILE, STT_SECTION get rid of it.
*/
static void print_symbols_for_64(t_symbol *symbols, size_t count)
{
    size_t i;
    uint64_t value;

    i = 0;
    sort_symbols(symbols, count);
    while (i < count)
    {
        if (check_symbols_print(&symbols[i]))
        {
            value = symbols[i].s_addr;
            if (symbols[i].s_shndx == SHN_COMMON)
                value = symbols[i].s_size;
            if (symbols[i].s_shndx == SHN_UNDEF)
                printf("%*s %c %s\n", 16, "", symbols[i].s_code, symbols[i].s_name);
            else
                printf("%0*" PRIx64 " %c %s\n", 16, value, symbols[i].s_code, symbols[i].s_name);
        }
        i++;
    }
}

static void print_symbols_for_32(t_symbol *symbols, size_t count)
{
    size_t i;
    uint64_t value;

    i = 0;
    sort_symbols(symbols, count);
    while (i < count)
    {
        if (check_symbols_print(&symbols[i]))
        {
            value = symbols[i].s_addr;
            if (symbols[i].s_shndx == SHN_COMMON)
                value = symbols[i].s_size;
            if (symbols[i].s_shndx == SHN_UNDEF)
                printf("%*s %c %s\n", 8, "", symbols[i].s_code, symbols[i].s_name);
            else
                printf("%0*" PRIx64 " %c %s\n", 8, value, symbols[i].s_code, symbols[i].s_name);
        }
        i++;
    }
}

void print_symbols(t_symbol *symbols, size_t count)
{
    if (count == 0 || !symbols)
        return;

    if (symbols[0].s_arch == 32)
        print_symbols_for_32(symbols, count);
    else
        print_symbols_for_64(symbols, count);
}