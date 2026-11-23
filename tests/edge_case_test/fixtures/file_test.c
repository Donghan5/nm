#include <dirent.h>
#include <string.h>
#include "ft_nm.h"

/* Current file API. Remove these declarations once the project header provides them. */
int handle_file(const char *filename, t_file *file);
void cleanup_file(t_file *file);
int check_file_size(int fd, t_file *file);

/* GNU ld wrapping injects metadata only while a boundary case is active. */
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

static int negative_size(const char *path)
{
	t_file file = {0};
	int fd;
	int result;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (fail("cannot open boundary test fixture"));
	injected_size = (off_t)-1;
	inject_stat = 1;
	result = check_file_size(fd, &file);
	inject_stat = 0;
	close(fd);
	if (result != FILE_ERROR)
	{
		fprintf(stderr, "FAIL: expected FILE_ERROR (%d), got %d\n",
			FILE_ERROR, result);
		return (1);
	}
	if (file.f_size != 0)
		return (fail("rejected size changed the output size"));
	return (0);
}

int main(int argc, char **argv)
{
	t_file file = {0};
	const char *name;
	int before;
	int result;
	int expected;
	int valid;
	int i;
	int rounds;

	if (argc != 3)
		return (fail("usage: file_test CASE PATH"));
	name = argv[1];
	if (!strcmp(name, "size-negative"))
		return (negative_size(argv[2]));
	if (!strcmp(name, "cleanup-zero"))
	{
		cleanup_file(&file);
		cleanup_file(&file);
		if (file.f_data != NULL || file.f_size != 0)
			return (fail("cleanup on a zeroed file must stay reset"));
		return (0);
	}
	/* Every case asserts one specific status, so a misspelled case name
	   cannot fall through and silently exercise the positive control. */
	if (!strcmp(name, "regular") || !strcmp(name, "repeat"))
		expected = 0;
	else if (!strcmp(name, "empty"))
		expected = FILE_EMPTY;
	else if (!strcmp(name, "missing"))
		expected = FILE_ERROR;
	else if (!strcmp(name, "directory"))
		expected = FILE_DIRECTORY;
	else
	{
		fprintf(stderr, "FAIL: unknown case name \"%s\"\n", name);
		return (1);
	}
	valid = expected == 0;
	rounds = !strcmp(name, "repeat") ? 64 : 1;
	before = fd_count();
	if (before < 0)
		return (fail("cannot inspect /proc/self/fd"));
	for (i = 0; i < rounds; i++)
	{
		result = handle_file(argv[2], &file);
		if (result != expected)
		{
			fprintf(stderr, "FAIL: %s: expected status %d, got %d\n",
				name, expected, result);
			return (1);
		}
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
		else if (file.f_data != NULL)
			return (fail("a rejected input must not leave a mapping"));
		cleanup_file(&file);
		if (file.f_data != NULL || file.f_size != 0)
			return (fail("cleanup must reset mapping and size"));
		cleanup_file(&file);
		if (fd_count() != before)
			return (fail("file descriptor leaked after cleanup"));
	}
	return (0);
}
