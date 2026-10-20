#include "../../inc/ft_nm.h"

void	ft_perror(const char *msg, const char *file)
{
	perror(msg);
	exit(1);
}

void	cleanup_and_exit(t_file *file, const char *msg)
{
	if (file->f_data)
		munmap(file->f_data, file->f_size);
	if (file->f_name)
		free(file->f_name);
	ft_perror(msg, NULL);
}
