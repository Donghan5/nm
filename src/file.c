#include "../inc/ft_nm.h"

// Open a file
int	open_file(const char *filename, t_file *file)
{
	int fd;

	fd = open(filename, O_RDONLY);
	return (fd);
}

// Check the size of the file
int	check_file_size(int fd, t_file *file)
{
	struct stat st;

	if (fd < 0)
		return (-1);
	if (fstat(fd, &st) == -1)
		return (-1);
	file->f_size = st.st_size;
	return (0);
}

// Mapping as read-only
int	map_file(int fd, t_file *file)
{
	file->f_data = mmap(NULL, file->f_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file->f_data == MAP_FAILED)
		return (-1);
	return (0);
}
// Stroe the start address and size of the file in the struct
int	store_file_info(const char *filename, t_file *file)
{
	file->f_name[0] = '\0';
	ft_strlcpy(file->f_name, filename, PATH_MAX);
	return (0);
}
// Cleaning resources
void	cleanup_file(t_file *file)
{
	if (file->f_data)
		munmap(file->f_data, file->f_size);
	if (file->f_name)
		free(file->f_name);
}

int	handle_file(const char *filename, t_file *file)
{
	int fd;

	fd = open_file(filename, file);
	if (fd < 0)
		return (-1);
	if (check_file_size(fd, file) < 0)
	{
		close(fd);
		return (-1);
	}
	if (map_file(fd, file) < 0)
	{
		close(fd);
		return (-1);
	}
	store_file_info(filename, file);
	close(fd);
	return (0);
}
