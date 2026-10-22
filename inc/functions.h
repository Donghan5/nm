#ifndef FUNCTOINS_H
# define FUNCTOINS_H

// error.c
void	cleanup_and_exit(t_file *file, const char *msg);

// elf_ident.c
int		check_common_range(const t_file *elf, size_t offset, size_t size);

#endif