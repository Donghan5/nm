#include "../../inc/ft_nm.h"

void	cleanup_and_exit(t_file *file, const char *msg)
{
	if (file->f_data)
		munmap(file->f_data, file->f_size);
	perror(msg);
}
