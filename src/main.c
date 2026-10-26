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
	return (i); // Return the index of the first non-flag argument
}

static int handle_aout_file(t_symbol **symbols, size_t *count)
{
	t_file file;
	int i;
	i = 0;
	if (handle_file("a.out", &file) < 0)
	{
		fprintf(stderr, "Error: Could not process file 'a.out'\n");
		return (-1);
	}
	if (!get_elf_section(&file, symbols, count))
	{
		fprintf(stderr, "Error: Could not get ELF section for file 'a.out'\n");
		cleanup_file(&file);
		return (-1);
	}
	while (i < *count)
	{
		if (symbol_type(*symbols + i, NULL) < 0)
		{
			fprintf(stderr, "Error: Could not determine symbol types for file 'a.out'\n");
			cleanup_file(&file);
			return (-1);
		}
		i++;
	}
	print_symbols(*symbols, *count);
	free(*symbols);
	*symbols = NULL;
	*count = 0;
	cleanup_file(&file);
	return (0);
}

static int process_file(const char *filename)
{
	t_symbol *symbols = NULL;
	size_t count = 0;
	t_file file;

	if (handle_file(filename, &file) < 0)
	{
		fprintf(stderr, "Error: Could not process file '%s'\n", filename);
		return (-1);
	}

}

int main(int argc, char **argv)
{
	int i;

	start_index = parse_flags(argc, argv);
	if (start_index == argc)
	{
		if (handle_aout_file(&symbols, &count) < 0)
			return (1);
		print_symbols(symbols, count);
		free(symbols);
		return (0);
	}
	while (start_index < argc)
	{
		process_file(argv[start_index]);
		start_index++;
	}
	return (0);
}
