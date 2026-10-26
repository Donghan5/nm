#include "ft_nm.h"

static void print_symbols_for_64(t_symbol *symbols, size_t count)
{
    size_t i;

    i = 0;
    sort_symbols(symbols, count);
    while (i < count)
    {
        if (symbols[i].s_shndx == SHN_UNDEF)
            printf("%*s %c %s\n", 16, "", symbols[i].s_code, symbols[i].s_name);
        else
            printf("%0*" PRIx64 " %c %s\n", 16, symbols[i].s_addr, symbols[i].s_code, symbols[i].s_name);
        i++;
    }
}

static void print_symbols_for_32(t_symbol *symbols, size_t count)
{
    size_t i;

    i = 0;
    sort_symbols(symbols, count);
    while (i < count)
    {
        if (symbols[i].s_shndx == SHN_UNDEF)
            printf("%*s %c %s\n", 8, "", symbols[i].s_code, symbols[i].s_name);
        else
            printf("%0*" PRIx64 " %c %s\n", 8, symbols[i].s_addr, symbols[i].s_code, symbols[i].s_name);
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