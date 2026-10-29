#ifndef FUNCTOINS_H
# define FUNCTOINS_H

// file.c
int	handle_file(const char *filename, t_file *file);
void cleanup_file(t_file *file);
void init_file(t_file *file);

// error.c
int report_error(const char *filename, const char *message);


// elf_ident.c
int		check_common_range(const t_file *elf, size_t offset, size_t size);
int     check_elf_ident(const t_file *elf);

// elf32.c
bool read_elf32_section(const t_section_table *table, size_t index, Elf32_Shdr *section);
const char  *get_elf32_string(const t_file *elf, const Elf32_Shdr *strtab, size_t name_offset);
int get_elf32_section_table(const t_file *elf, t_section_table *section_table);
bool read_elf32_symbol(const t_file *file, const Elf32_Shdr *symtab, size_t sym_index, Elf32_Sym *sym);

// elf64.c
bool read_elf64_section(const t_section_table *table, size_t index, Elf64_Shdr *section);
const char  *get_elf64_string(const t_file *elf, const Elf64_Shdr *strtab, size_t name_offset);
int get_elf64_section_table(const t_file *elf, t_section_table *section_table);
bool read_elf64_symbol(const t_file *file, const Elf64_Shdr *symtab, size_t sym_index, Elf64_Sym *sym);

// symbol_validate.c
bool check_sym_type_32(const t_file *elf, const Elf32_Shdr *symtab, const Elf32_Shdr *strtab);
bool check_sym_type_64(const t_file *elf, const Elf64_Shdr *symtab, const Elf64_Shdr *strtab);

// symbol_type.c
int symbol_type(t_symbol *symbol, t_section_table *section);

// symbol_collect.c
bool get_elf_section(const t_file *file, t_symbol **symbols, size_t *count);

// sort_symbols.c
void sort_symbols(t_symbol *symbols, size_t count);

// print_symbols.c
void print_symbols(t_symbol *symbols, size_t count);
#endif