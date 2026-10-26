#include "ft_nm.h"

// Compare two symbols by name
static int compare_symbols(const void *a, const void *b)
{
    const t_symbol *sym_a;
    const t_symbol *sym_b;
    int name_cmp;
    
    sym_a = (const t_symbol *)a;
    sym_b = (const t_symbol *)b;
    name_cmp = ft_strncmp(sym_a->s_name, sym_b->s_name, sizeof(sym_a->s_name));
    if (name_cmp != 0)
        return (name_cmp);
    return (0);
}

static int sort_ascending(const void *a, const void *b)
{
    return (compare_symbols(a, b));
}

static int sort_descending(const void *a, const void *b)
{
    return (compare_symbols(b, a));
}

// add some flags to -r, -p
void sort_symbols(t_symbol *symbols, size_t count)
{
    if (g_flags & FLAG_P)
        return;
    else if (g_flags & FLAG_R)
        qsort(symbols, count, sizeof(t_symbol), sort_descending);
    else
        qsort(symbols, count, sizeof(t_symbol), sort_ascending);
}
