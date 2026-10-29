#include "ft_nm.h"

void	init_file(t_file *file)
{
	file->f_name[0] = '\0';
	file->f_data = NULL;
	file->f_size = 0;
	file->f_type = 0;
}

int	open_file(const char *filename)
{
	int fd;

	fd = open(filename, O_RDONLY);
	return (fd);
}

int	close_file_error(int fd)
{
	int saved_errno;

	saved_errno = errno;
	close(fd);
	errno = saved_errno;
	return (-1);
}

int	check_file_size(int fd, t_file *file)
{
	struct stat st;

	if (fd < 0)
		return (-1);
	if (fstat(fd, &st) == -1)
		return (-1);
	if (S_ISDIR(st.st_mode))
		return (FILE_DIRECTORY);
	if (st.st_size < 0)
		return (-1);
	if ((uintmax_t)st.st_size > (uintmax_t)(SIZE_MAX))
		return (-1);
	file->f_size = (size_t)st.st_size;
	if (file->f_size == 0)
		return (FILE_EMPTY);
	return (0);
}

int	map_file(int fd, t_file *file)
{
	file->f_data = mmap(NULL, file->f_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file->f_data == MAP_FAILED)
		return (-1);
	return (0);
}

void	cleanup_file(t_file *file)
{
	if (file->f_data && file->f_data != MAP_FAILED)
		munmap(file->f_data, file->f_size);
	file->f_data = NULL;
	file->f_size = 0;
}

int	handle_file(const char *filename, t_file *file)
{
	int fd;
	int status;

	fd = open_file(filename);
	if (fd < 0)
	return (-1);
	init_file(file);
	status = check_file_size(fd, file);
	if (status < 0)
	{
		close_file_error(fd);
		return (status);
	}
	if (map_file(fd, file) < 0)
		return (close_file_error(fd));
	strlcpy(file->f_name, filename, PATH_MAX);
	close(fd);
	return (0);
}
