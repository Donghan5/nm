#include "ft_nm.h"

int g_flags = 0; // Global variable to hold flags

static void usage(void)
{
    fprintf(stderr, "Usage: ft_nm [options] [file...]\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -r : Sort symbols in reverse order\n");
	fprintf(stderr, "  -p : Do not sort symbols\n");
	fprintf(stderr, "Other options are not supported in this version.\n");
	exit (1);	// Exit after printing usage
}



static int parse_flags(int argc, char **argv)
{
	int i = 1;
	while (i < argc && argv[i][0] == '-')
	{
		if (strcmp(argv[i], "-r") == 0)
			g_flags |= FLAG_R;
		else if (strcmp(argv[i], "-p") == 0)
			g_flags |= FLAG_P;
		else if (strcmp(argv[i], "-h") == 0)
			usage();
		else if (argv[i][0] != '-')
			break;
		else
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			usage();
		}
		i++;
	}
	return (i);
}

static int process_file(const char *filename)
{
	t_file file;
	t_symbol *symbols;
	t_section_table table;
	size_t i;
	size_t count;
	const unsigned char *data;

	count = 0;
	symbols = NULL;
	init_file(&file);
	if (handle_file(filename, &file) < 0)
		return (handle_exit_with_error_and_cleanup("Could not process file", &file, NULL));

	if (!get_elf_section(&file, &symbols, &count))
		return (handle_exit_with_error_and_cleanup("Could not get ELF section for file", &file, NULL));

	data = file.f_data;
	if (data[EI_CLASS] == ELFCLASS32)
	{
		if (get_elf32_section_table(&file, &table) < 0)
			return (handle_exit_with_error_and_cleanup("Could not get ELF32 section table for file", &file, symbols));
	}
	else
	{
		if (get_elf64_section_table(&file, &table) < 0)
			return (handle_exit_with_error_and_cleanup("Could not get ELF64 section table for file", &file, symbols));
	}

	i = 0;
	while (i < count)
	{
		if (symbol_type(symbols + i, &table) < 0)
			return (handle_exit_with_error_and_cleanup("Could not determine symbol types for file", &file, symbols));
		i++;
	}
	print_symbols(symbols, count);
	free(symbols);
	symbols = NULL;
	count = 0;
	cleanup_file(&file);
	return (0);
}

int main(int argc, char **argv)
{
	int start_index;
	int status;

	start_index = parse_flags(argc, argv);
	status = 0;
	if (start_index == argc)
	{
		if (process_file("a.out") < 0)
			return (1);
	}

	while (start_index < argc)
	{
		if (process_file(argv[start_index]) < 0)
			status = 1;
		start_index++;
	}
	return (status);
}
