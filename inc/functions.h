#ifndef FUNCTOINS_H
# define FUNCTOINS_H

// error.c
void	cleanup_and_exit(t_file *file, const char *msg);

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

#endif