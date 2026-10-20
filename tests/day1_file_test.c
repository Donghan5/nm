#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "ft_nm.h"

/* Current file API. Remove these declarations once the project header provides them. */
int handle_file(const char *filename, t_file *file);
void cleanup_file(t_file *file);
int check_file_size(int fd, t_file *file);

/* GNU ld wrapping injects metadata only while a boundary case is active. */
#ifdef DAY1_TEST_32
_Static_assert(sizeof(size_t) == 4, "32-bit size_t required");
_Static_assert(sizeof(off_t) == 8, "64-bit off_t required");
# define __real_fstat __real_fstat64
# define __wrap_fstat __wrap_fstat64
#endif
int __real_fstat(int fd, struct stat *st);
static int inject_stat;
static off_t injected_size;

int __wrap_fstat(int fd, struct stat *st)
{
	if (!inject_stat)
		return (__real_fstat(fd, st));
	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFREG | 0600;
	st->st_size = injected_size;
	return (0);
}

static int fd_count(void)
{
	DIR *dir = opendir("/proc/self/fd");
	struct dirent *entry;
	int count = 0;

	if (!dir)
		return (-1);
	while ((entry = readdir(dir)) != NULL)
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			count++;
	closedir(dir);
	return (count);
}

static int fail(const char *message)
{
	fprintf(stderr, "FAIL: %s\n", message);
	return (1);
}

static int size_boundary(const char *name, const char *path)
{
	t_file file = {0};
	uintmax_t off_max;
	uintmax_t value;
	int fd;
	int result;
	int reject = strcmp(name, "size-max-valid") != 0;

	/* Linux off_t is signed. Construct its maximum without signed overflow. */
	off_max = UINTMAX_MAX >> (sizeof(uintmax_t) * CHAR_BIT
		- sizeof(off_t) * CHAR_BIT + 1);
	if (!strcmp(name, "size-negative"))
		injected_size = (off_t)-1;
	else if (!strcmp(name, "size-overflow"))
	{
		if (off_max <= (uintmax_t)SIZE_MAX)
		{
			puts("SIZE_MAX + 1 cannot be represented by off_t on this platform; overflow rejection was not tested");
			return (77);
		}
		value = (uintmax_t)SIZE_MAX;
		injected_size = (off_t)(value + 1);
	}
	else
	{
		value = off_max < (uintmax_t)SIZE_MAX ? off_max : (uintmax_t)SIZE_MAX;
		injected_size = (off_t)value;
	}
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (fail("cannot open boundary test fixture"));
	inject_stat = 1;
	result = check_file_size(fd, &file);
	inject_stat = 0;
	close(fd);
	if (reject && result >= 0)
		return (fail("unrepresentable file size was accepted"));
	if (reject && file.f_size != 0)
		return (fail("rejected size changed the output size"));
	if (!reject && (result != 0 || file.f_size != (size_t)injected_size))
		return (fail("largest representable file size was not preserved"));
	return (0);
}

int main(int argc, char **argv)
{
	t_file file = {0};
	int before;
	int result;
	int valid;
	int repeated;
	int i;
	int rounds;

	if (argc != 3)
		return (fail("usage: day1_file_test CASE PATH"));
	if (!strncmp(argv[1], "size-", 5))
		return (size_boundary(argv[1], argv[2]));
	before = fd_count();
	if (before < 0)
		return (fail("cannot inspect /proc/self/fd"));
	if (!strcmp(argv[1], "cleanup-zero"))
	{
		cleanup_file(&file);
		cleanup_file(&file);
		return (0);
	}
	valid = !strcmp(argv[1], "regular") || !strcmp(argv[1], "repeat");
	repeated = !strcmp(argv[1], "repeat");
	rounds = repeated ? 64 : 1;
	for (i = 0; i < rounds; i++)
	{
		result = handle_file(argv[2], &file);
		if (valid && result != 0)
			return (fail("regular file was rejected"));
		if (!valid && result >= 0)
			return (fail("invalid input was accepted"));
		if (fd_count() != before)
			return (fail("file descriptor leaked before cleanup"));
		if (valid)
		{
			if (file.f_size != 6 || !file.f_data || file.f_data == MAP_FAILED)
				return (fail("incorrect mapping or size"));
			if (memcmp(file.f_data, "hello\n", 6))
				return (fail("mapped bytes differ after fd close"));
			if (strcmp((const char *)file.f_name, argv[2]))
				return (fail("filename was not stored correctly"));
		}
		cleanup_file(&file);
		if (file.f_data != NULL || file.f_size != 0)
			return (fail("cleanup must reset mapping and size"));
		cleanup_file(&file);
		if (fd_count() != before)
			return (fail("file descriptor leaked after cleanup"));
	}
	return (0);
}
