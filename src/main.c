#include "ft_nm.h"

int g_flags = 0; // Global variable to hold flags

static void usage(void)
{
    fprintf(stderr, "Usage: ft_nm [option(s)] [file(s)]\n");
	fprintf(stderr, "List symbols in [file(s)] (a.out by default)\n");
	fprintf(stderr, "The Options are:\n");
	fprintf(stderr, "  -r  Sort symbols in reverse order\n");
	fprintf(stderr, "  -p  Do not sort symbols\n");
	fprintf(stderr, "  --  End Options processing\n");
	fprintf(stderr, "Other options are not supported in this version.\n");
	exit (1);	// Exit after printing usage
}

static void parse_short_flags(const char *arg)
{
	size_t i;

	i = 1;
	while (arg[i])
	{
		if (arg[i] == 'r')
				g_flags |= FLAG_R;
		else if (arg[i] == 'p')
				g_flags |= FLAG_P;
		else
		{
			fprintf(stderr, "ft_nm: invalid option -- '%c'\n", arg[i]);
			usage();
		}
		i++;
	}
}

static int parse_flags(int argc, char **argv)
{
	int i = 1;
	int file_count = 0;
	bool options_enabled = true;

	while (i < argc)
	{
		if (options_enabled && strcmp(argv[i], "--") == 0)
			options_enabled = false;
		else if (options_enabled && argv[i][0] == '-' && argv[i][1] == '-')
		{
			fprintf(stderr, "ft_nm: unrecognized option '%s'\n", argv[i]);
			usage();
		}
		else if (options_enabled && argv[i][0] == '-' && argv[i][1] != '\0')	
			parse_short_flags(argv[i]);
		else
			file_count++;
		i++;
	}
	return (file_count);
}

static int process_file(const char *filename, bool show_filename)
{
	t_file file;
	t_symbol *symbols;
	t_section_table table;
	size_t i;
	size_t count;
	const unsigned char *data;
	int	saved_errno;
	int file_status;

	count = 0;
	symbols = NULL;
	init_file(&file);
	file_status = handle_file(filename, &file);
	if (file_status < 0)
	{
		saved_errno = errno;
		cleanup_file(&file);
		if (file_status == FILE_EMPTY)
			return (-1);
		if (file_status == FILE_DIRECTORY)
		{
			fprintf(stderr, "ft_nm: Warning: '%s' is a directory\n", filename);
			return (-1);
		}
		return (report_error(filename, strerror(saved_errno)));
	}

	if (!get_elf_section(&file, &symbols, &count))
	{
		cleanup_file(&file);
		return (report_error(filename, "Could not read ELF symbols"));
	}

	data = file.f_data;
	if (data[EI_CLASS] == ELFCLASS32)
	{
		if (get_elf32_section_table(&file, &table) < 0)
		{
			free(symbols);
			cleanup_file(&file);
			return (report_error(filename, "Could not read ELF32 section table"));
		}
	}
	else
	{
		if (get_elf64_section_table(&file, &table) < 0)
		{
			free(symbols);
			cleanup_file(&file);
			return (report_error(filename, "Could not read ELF64 section table"));
		}
	}

	i = 0;
	while (i < count)
	{
		if (symbol_type(symbols + i, &table) < 0)
		{
			free(symbols);
			cleanup_file(&file);
			return (report_error(filename, "Could not determine symbol type"));
		}
		i++;
	}
	if (show_filename)
		printf("\n%s:\n", filename);
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
	bool show_filename;
	int file_count;
	bool options_enabled;

	file_count = parse_flags(argc, argv);
	status = 0;
	start_index = 1;
	options_enabled = true;
	if (file_count == 0)
	{
		if (process_file("a.out", false) < 0)
			return (1);
		return (0);
	}

	show_filename = (file_count > 1);
	while (start_index < argc)
	{
		if (options_enabled && strcmp(argv[start_index],  "--") == 0)
		{
			options_enabled = false;
			start_index++;
			continue;
		}
		if (options_enabled && argv[start_index][0] == '-' && argv[start_index][1] != '\0')
		{
			start_index++;
			continue;
		}
		if (process_file(argv[start_index], show_filename) < 0)
			status = 1;
		start_index++;
	}
	return (status);
}
