#include "ft_nm.h"

void	cleanup_and_exit(t_file *file, const char *msg)
{
	if (file->f_data)
		munmap(file->f_data, file->f_size);
	perror(msg);
}

int handle_exit_with_error(const char *message)
{
	fprintf(stderr, "Error: %s\n", message);
	return (-1);
}

int handle_exit_with_error_and_cleanup(const char *message, t_file *file, t_symbol *symbols)
{
	fprintf(stderr, "Error: %s\n", message);
	cleanup_file(file);
	if (symbols)
	{
		free(symbols);
		symbols = NULL;
	}
	return (-1);
}