bool return_false_with_free(t_symbol **symbols, size_t *count)
{
    if (*symbols)
        free(*symbols);
    *symbols = NULL;
    *count = 0;
    return (false);
}

void init_symbol(t_symbol *symbol)
{
    symbol->s_name[0] = '\0';
    symbol->s_original_index = 0;
    symbol->s_arch = 0;
    symbol->s_type = 0;
    symbol->s_bind = 0;
    symbol->s_shndx = 0;
    symbol->s_addr = 0;
    symbol->s_size = 0;
    symbol->s_code = 0;
    symbol->s_extended_index = false;
}

bool check_symbol_section_index(uint32_t index, size_t section_count)
{
    if (index == SHN_UNDEF || index == SHN_ABS || index == SHN_COMMON)
        return (true);
    if (index >= SHN_LORESERVE)
        return (false);
    return (index < section_count);
}