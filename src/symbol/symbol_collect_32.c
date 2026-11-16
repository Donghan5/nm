#include "ft_nm.h"



static bool find_sub_table_for_32(const t_section_table *table,
	size_t symtab_index, Elf32_Shdr *sub_table, bool *found)
{
	Elf32_Shdr	candidate;
	size_t		index;

	index = 0;
	*found = false;
	while (index < table->count)
	{
		if (!read_elf32_section(table, index, &candidate))
			return (false);
		if (candidate.sh_type == SHT_SYMTAB_SHNDX
			&& candidate.sh_link == symtab_index)
		{
			if (*found)
				return (false);
			*sub_table = candidate;
			*found = true;
		}
		index++;
	}
	return (true);
}

static bool check_sub_table_for_32(const t_section_table *table,
	const Elf32_Shdr *section, size_t entry_count)
{
	if (!check_common_range(table->file, section->sh_offset, section->sh_size))
		return (false);
	if (section->sh_entsize != sizeof(Elf32_Word))
		return (false);
	if (section->sh_size % sizeof(Elf32_Word) != 0)
		return (false);
	if (section->sh_size / sizeof(Elf32_Word) != entry_count)
		return (false);
	return (true);
}

static bool read_extended_index_for_32(const t_section_table *table,
	const Elf32_Shdr *sub_table, size_t sym_index, uint32_t *actual_index)
{
	const unsigned char	*data;

	if (sym_index >= sub_table->sh_size / sizeof(Elf32_Word))
		return (false);
	data = table->file->f_data;
	memcpy(actual_index,
		data + sub_table->sh_offset + sym_index * sizeof(Elf32_Word),
		sizeof(*actual_index));
	return (true);
}

static bool prepare_symbol_table(t_collect32 *context, size_t symtab_index)
{
	const t_file	*file;

	file = context->table.file;
	if (!read_elf32_section(&context->table,
			context->symtab.sh_link, &context->strtab))
		return (false);
	if (context->strtab.sh_type != SHT_STRTAB)
		return (false);
	if (!check_sym_type_32(file, &context->symtab, &context->strtab))
		return (false);
	context->entry_count = context->symtab.sh_size / context->symtab.sh_entsize;
	if (context->entry_count > SIZE_MAX / sizeof(t_symbol))
		return (false);
	if (!find_sub_table_for_32(&context->table, symtab_index,
			&context->sub_table, &context->found))
		return (false);
	if (context->found && !check_sub_table_for_32(&context->table,
			&context->sub_table, context->entry_count))
		return (false);
	return (true);
}

static bool resolve_symbol_index(t_collect32 *context,
	const Elf32_Sym *sym, size_t sym_index, uint32_t *actual_index)
{
	*actual_index = sym->st_shndx;
	if (context->found)
	{
		if (!read_extended_index_for_32(&context->table,
				&context->sub_table, sym_index, &context->sub_index))
			return (false);
	}
	if (sym->st_shndx == SHN_XINDEX)
	{
		if (!context->found || context->sub_index < SHN_LORESERVE
			|| context->sub_index >= context->table.count)
			return (false);
		*actual_index = context->sub_index;
	}
	else
	{
		if (context->sub_index != 0 || !check_symbol_section_index(
				*actual_index, context->table.count))
			return (false);
	}
	return (true);
}

static bool copy_symbol_name(const t_collect32 *context,
	const Elf32_Sym *sym, t_symbol *symbol)
{
	const char	*name;
	size_t		len;

	name = get_elf32_string(context->table.file, &context->strtab, sym->st_name);
	if (!name)
		return (false);
	len = strlen(name);
	if (len >= sizeof(symbol->s_name))
		return (false);
	memcpy(symbol->s_name, name, len + 1);
	return (true);
}

static void store_symbol_fields(t_symbol *symbol, const Elf32_Sym *sym,
	size_t sym_index, uint32_t actual_index)
{
	symbol->s_original_index = sym_index;
	symbol->s_arch = 32;
	symbol->s_type = ELF32_ST_TYPE(sym->st_info);
	symbol->s_bind = ELF32_ST_BIND(sym->st_info);
	symbol->s_shndx = actual_index;
	symbol->s_addr = sym->st_value;
	symbol->s_size = sym->st_size;
	symbol->s_extended_index = (sym->st_shndx == SHN_XINDEX);
}

static bool collect_one_symbol(t_collect32 *context,
	size_t sym_index, t_symbol *symbol)
{
	Elf32_Sym	sym;
	uint32_t	actual_index;

	if (!read_elf32_symbol(context->table.file,
			&context->symtab, sym_index, &sym))
		return (false);
	if (!resolve_symbol_index(context, &sym, sym_index, &actual_index))
		return (false);
	init_symbol(symbol);
	if (!copy_symbol_name(context, &sym, symbol))
		return (false);
	store_symbol_fields(symbol, &sym, sym_index, actual_index);
	return (true);
}

static bool collect_symbol_table(t_collect32 *context,
	t_symbol **symbols, size_t *count)
{
	size_t	sym_index;

	*symbols = malloc(sizeof(t_symbol) * context->entry_count);
	if (!*symbols)
		return (false);
	sym_index = 0;
	while (sym_index < context->entry_count)
	{
		if (!collect_one_symbol(context, sym_index, &(*symbols)[*count]))
			return (return_false_with_free(symbols, count));
		(*count)++;
		sym_index++;
	}
	return (true);
}

bool collect_elf32_section(const t_file *file, t_symbol **symbols, size_t *count)
{
	t_collect32	context;
	size_t		index;

	context.sub_index = 0;
	index = 0;
	*count = 0;
	*symbols = NULL;
	if (get_elf32_section_table(file, &context.table) < 0)
		return (false);
	context.found = false;
	while (index < context.table.count)
	{
		if (!read_elf32_section(&context.table, index, &context.symtab))
			return (false);
		if (context.symtab.sh_type == SHT_SYMTAB)
		{
			if (!prepare_symbol_table(&context, index))
				return (false);
			return (collect_symbol_table(&context, symbols, count));
		}
		index++;
	}
	return (true);
}
