#include "ft_nm.h"
#include <string.h>

#if TEST_BITS == 32
# define EH Elf32_Ehdr
# define SH Elf32_Shdr
# define SYM Elf32_Sym
# define CLASS ELFCLASS32
# define MACHINE EM_386
#else
# define EH Elf64_Ehdr
# define SH Elf64_Shdr
# define SYM Elf64_Sym
# define CLASS ELFCLASS64
# define MACHINE EM_X86_64
#endif

int main(int argc, char **argv)
{
	const size_t target = SHN_COMMON;
	const size_t sections = SHN_COMMON + 1;
	size_t symoff = sizeof(EH) + sections * sizeof(SH);
	size_t stroff = symoff + 2 * sizeof(SYM);
	size_t xoff = stroff + 3;
	t_file file = {0};
	EH h = {0};
	SH *sh;
	SYM sym[2] = {{0}};
	uint32_t words[2] = {0, SHN_COMMON};
	t_symbol *symbols = NULL;
	size_t count = 0;
	bool expected, ok;
	int result;

	if (argc != 2 && argc != 3) return (1);
	expected = !strcmp(argv[1], "valid");
	file.f_size = xoff + sizeof(words);
	file.f_data = calloc(1, file.f_size);
	if (!file.f_data) return (1);
	memcpy(h.e_ident, ELFMAG, SELFMAG);
	h.e_ident[EI_CLASS] = CLASS;
	h.e_ident[EI_DATA] = ELFDATA2LSB;
	h.e_ident[EI_VERSION] = EV_CURRENT;
	h.e_type = ET_REL; h.e_machine = MACHINE; h.e_version = EV_CURRENT;
	h.e_ehsize = sizeof(EH); h.e_shoff = sizeof(EH); h.e_shentsize = sizeof(SH);
	h.e_shstrndx = 2;
	memcpy(file.f_data, &h, sizeof(h));
	sh = (SH *)((unsigned char *)file.f_data + sizeof(EH));
	sh[0].sh_size = sections;
	sh[1].sh_type = SHT_SYMTAB; sh[1].sh_offset = symoff;
	sh[1].sh_size = sizeof(sym); sh[1].sh_link = 2; sh[1].sh_entsize = sizeof(SYM);
	sh[2].sh_type = SHT_STRTAB; sh[2].sh_offset = stroff; sh[2].sh_size = 3;
	sh[3].sh_type = SHT_SYMTAB_SHNDX; sh[3].sh_offset = xoff;
	sh[3].sh_size = sizeof(words); sh[3].sh_link = 1; sh[3].sh_entsize = 4;
	sh[target].sh_type = SHT_NOBITS; sh[target].sh_flags = SHF_ALLOC | SHF_WRITE;
	sym[1].st_name = 1; sym[1].st_info = ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT);
	sym[1].st_shndx = SHN_XINDEX;
	if (!strcmp(argv[1], "nonzero-companion")) words[0] = 1;
	if (!strcmp(argv[1], "missing")) sh[3].sh_type = SHT_NULL;
	if (!strcmp(argv[1], "duplicate")) sh[4] = sh[3];
	memcpy((unsigned char *)file.f_data + symoff, sym, sizeof(sym));
	memcpy((unsigned char *)file.f_data + stroff, "\0x", 3);
	memcpy((unsigned char *)file.f_data + xoff, words, sizeof(words));
	if (!strcmp(argv[1], "shstr-range")) sh[2].sh_offset = file.f_size + 1;
	if (!strcmp(argv[1], "section-name-oob")) sh[target].sh_name = 3;
	if (argc == 3)
	{
		FILE *stream;
		sh[4].sh_type = SHT_PROGBITS;
		if (!strcmp(argv[1], "emit-note")) sh[4].sh_type = SHT_NOTE;
		if (!strcmp(argv[1], "emit-nobits")) sh[4].sh_type = SHT_NOBITS;
		sh[4].sh_offset = file.f_size + 16; sh[4].sh_size = 1;
		stream = fopen(argv[2], "wb");
		if (!stream) { free(file.f_data); return (1); }
		result = (fwrite(file.f_data, 1, file.f_size, stream) != file.f_size);
		if (fclose(stream)) result = 1;
		free(file.f_data);
		return (result);
	}
	ok = get_elf_section(&file, &symbols, &count);
	result = (ok != expected);
	if (ok && (count != 2 || symbols[1].s_shndx != target || !symbols[1].s_extended_index)) result = 1;
	if (!ok && (symbols || count)) result = 1;
	free(symbols);
	free(file.f_data);
	return (result);
}
