#include "ft_nm.h"

static bool return_false_with_free(t_symbol **symbols, size_t *count)
{
    if (*symbols)
        free(*symbols);
    *symbols = NULL;
    *count = 0;
    return (false);
}

static void init_symbol(t_symbol *symbol)
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
}

bool check_symbol_section_index(uint32_t index, size_t section_count)
{
    if (index == SHN_UNDEF || index == SHN_ABS || index == SHN_COMMON)
        return (true);
    if (index >= SHN_LORESERVE)
        return (false);
    return (index < section_count);
}

bool collect_elf32_section(const t_file *file, t_symbol **symbols, size_t *count)
{
    t_section_table table;
    Elf32_Shdr symtab;
    Elf32_Shdr strtab;
    size_t  index;
    size_t  entry_count;
    size_t  sym_index;
    const char *name;
    size_t len;

    index = 0;
    *count = 0;
    *symbols = NULL;

    if (get_elf32_section_table(file, &table) < 0)
        return (false);
    
    while (index < table.count)
    {
        if (!read_elf32_section(&table, index, &symtab))
            return (false);
        if (symtab.sh_type == SHT_SYMTAB)
        {
            if (!read_elf32_section(&table, symtab.sh_link, &strtab))
                return (false);
            if (strtab.sh_type != SHT_STRTAB)
                return (false);
            if (!check_sym_type_32(file, &symtab, &strtab))
                return (false);
            entry_count = symtab.sh_size / symtab.sh_entsize;
            if (entry_count > SIZE_MAX / sizeof(t_symbol))
                return (false);
            *symbols = malloc(sizeof(t_symbol) * entry_count);
            if (!*symbols)
                return (false);
            sym_index = 0;
            while (sym_index < entry_count)
            {
                Elf32_Sym sym;
                if (!read_elf32_symbol(file, &symtab, sym_index, &sym))
                    return (return_false_with_free(symbols, count));
                
                if (!check_symbol_section_index(sym.st_shndx, table.count))
                    return (return_false_with_free(symbols, count));
                
                init_symbol(&(*symbols)[*count]);
                
                name = get_elf32_string(file, &strtab, sym.st_name);
                if (!name)
                    return (return_false_with_free(symbols, count));
                len = strlen(name);
                if (len >= sizeof((*symbols)[*count].s_name))
                    return (return_false_with_free(symbols, count));
                memcpy((*symbols)[*count].s_name, name, len + 1);
                (*symbols)[*count].s_original_index = sym_index;
                (*symbols)[*count].s_arch = 32;
                (*symbols)[*count].s_type = ELF32_ST_TYPE(sym.st_info);
                (*symbols)[*count].s_bind = ELF32_ST_BIND(sym.st_info);
                (*symbols)[*count].s_shndx = sym.st_shndx;
                (*symbols)[*count].s_addr = sym.st_value;
                (*symbols)[*count].s_size = sym.st_size;
                (*count)++;
                sym_index++;
            }
            return (true);
        }
        index++;
    }
    return (true);
}

bool collect_elf64_section(const t_file *file, t_symbol **symbols, size_t *count)
{
    t_section_table table;
    Elf64_Shdr symtab;
    Elf64_Shdr strtab;
    size_t  index;
    size_t  entry_count;
    size_t  sym_index;
    const char *name;
    size_t len;

    index = 0;
    *symbols = NULL;
    *count = 0;

    if (get_elf64_section_table(file, &table) < 0)
        return (false);

    while (index < table.count)
    {
        if (!read_elf64_section(&table, index, &symtab))
            return (false);
        if (symtab.sh_type == SHT_SYMTAB)
        {
            if (!read_elf64_section(&table, symtab.sh_link, &strtab))
                return (false);
            if (strtab.sh_type != SHT_STRTAB)
                return (false);
            if (!check_sym_type_64(file, &symtab, &strtab))
                return (false);
            entry_count = symtab.sh_size / symtab.sh_entsize;
            if (entry_count > SIZE_MAX / sizeof(t_symbol))
                return (false);
            *symbols = malloc(sizeof(t_symbol) * entry_count);
            if (!*symbols)
                return (false);
            sym_index = 0;
            while (sym_index < entry_count)
            {
                Elf64_Sym sym;
                if (!read_elf64_symbol(file, &symtab, sym_index, &sym))
                    return (return_false_with_free(symbols, count));

                if (!check_symbol_section_index(sym.st_shndx, table.count))
                    return (return_false_with_free(symbols, count));

                init_symbol(&(*symbols)[*count]);
                name = get_elf64_string(file, &strtab, sym.st_name);
                if (!name)
                    return (return_false_with_free(symbols, count));
                len = strlen(name);
                if (len >= sizeof((*symbols)[*count].s_name))
                    return (return_false_with_free(symbols, count));
                memcpy((*symbols)[*count].s_name, name, len + 1);
                (*symbols)[*count].s_original_index = sym_index;
                (*symbols)[*count].s_arch = 64;
                (*symbols)[*count].s_type = ELF64_ST_TYPE(sym.st_info);
                (*symbols)[*count].s_bind = ELF64_ST_BIND(sym.st_info);
                (*symbols)[*count].s_shndx = sym.st_shndx;
                (*symbols)[*count].s_addr = sym.st_value;
                (*symbols)[*count].s_size = sym.st_size;
                (*count)++;
                sym_index++;
            }
            return (true);
        }
        index++;
    }
    return (true);
}

// main function to get the symbol type with SYMSHT_SYMTAB
bool get_elf_section(const t_file *file, t_symbol **symbols, size_t *count)
{
    const unsigned char *data;

    if (!check_elf_ident(file))
        return (false);
    data = file->f_data;
    if (data[EI_CLASS] == ELFCLASS32)
        return (collect_elf32_section(file, symbols, count));
    if (data[EI_CLASS] == ELFCLASS64)
        return (collect_elf64_section(file, symbols, count));
    return (false);
}