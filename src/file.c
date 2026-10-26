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
	if (fd < 0)
		perror("Error opening file");
	return (fd);
}

int	close_file_error(int fd)
{
	close(fd);
	return (-1);
}

int	check_file_size(int fd, t_file *file)
{
	struct stat st;

	if (fd < 0)
		return (-1);
	if (fstat(fd, &st) == -1)
		return (-1);
	if (st.st_size < 0)
		return (-1);
	if ((uintmax_t)st.st_size > (uintmax_t)(SIZE_MAX))
		return (-1);
	file->f_size = (size_t)st.st_size;
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
	if (file->f_data)
		munmap(file->f_data, file->f_size);
	file->f_data = NULL;
	file->f_size = 0;
}

int	handle_file(const char *filename, t_file *file)
{
	int fd;

	fd = open_file(filename);
	if (fd < 0)
	return (-1);
	init_file(file);
	if (check_file_size(fd, file) < 0)
		return (close_file_error(fd));
	if (map_file(fd, file) < 0)
		return (close_file_error(fd));
	strlcpy(file->f_name, filename, PATH_MAX);
	close(fd);
	return (0);
}
