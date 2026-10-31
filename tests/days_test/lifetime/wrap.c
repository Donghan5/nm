#include "ft_nm.h"
#include <dirent.h>
#include <stdarg.h>

int nm_cli_main(int argc, char **argv);
int __real_open(const char *path, int flags, ...);
int __real_close(int fd);
int __real_fstat(int fd, struct stat *st);
void *__real_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);
int __real_munmap(void *addr, size_t len);
int __real_symbol_type(t_symbol *symbol, t_section_table *table);
void __real_print_symbols(t_symbol *symbols, size_t count);

static int tracked_fd = -1;
static void *tracked_map;
static size_t tracked_len;
static unsigned opens, closes, stats, map_attempts, maps, unmaps;
static unsigned type_calls, print_calls, errors, injected, peak;
static const char *fault;
static unsigned fail_at;
static int fault_errno;
static bool poison_close;

static int fd_count(void)
{
	DIR *dir;
	struct dirent *entry;
	int count;

	dir = opendir("/proc/self/fd");
	if (!dir)
		return (-1);
	count = 0;
	while ((entry = readdir(dir)))
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			count++;
	closedir(dir);
	return (count);
}

static int fixture_map_count(void)
{
	FILE *file;
	char *line;
	size_t size;
	int count;

	file = fopen("/proc/self/maps", "r");
	if (!file)
		return (-1);
	line = NULL;
	size = 0;
	count = 0;
	while (getline(&line, &size, file) >= 0)
		if (strstr(line, "/fixtures/"))
			count++;
	free(line);
	fclose(file);
	return (count);
}

int __wrap_open(const char *path, int flags, ...)
{
	int fd;

	fd = __real_open(path, flags);
	if (fd >= 0)
	{
		if (tracked_fd >= 0)
			errors++;
		tracked_fd = fd;
		opens++;
	}
	return (fd);
}

int __wrap_close(int fd)
{
	int result;

	if (fd != tracked_fd)
		errors++;
	result = __real_close(fd);
	if (result == 0)
	{
		tracked_fd = -1;
		closes++;
	}
	if (poison_close)
		errno = EBUSY;
	return (result);
}

int __wrap_fstat(int fd, struct stat *st)
{
	stats++;
	if (!strcmp(fault, "fstat") && stats == fail_at)
	{
		injected++;
		errno = fault_errno;
		return (-1);
	}
	return (__real_fstat(fd, st));
}

void *__wrap_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	void *result;

	map_attempts++;
	if (!strcmp(fault, "mmap") && map_attempts == fail_at)
	{
		injected++;
		errno = fault_errno;
		return (MAP_FAILED);
	}
	result = __real_mmap(addr, len, prot, flags, fd, off);
	if (result != MAP_FAILED)
	{
		if (tracked_map)
			errors++;
		tracked_map = result;
		tracked_len = len;
		maps++;
		if (maps - unmaps > peak)
			peak = maps - unmaps;
	}
	return (result);
}

int __wrap_munmap(void *addr, size_t len)
{
	int result;

	if (addr == MAP_FAILED || addr != tracked_map || len != tracked_len)
	{
		errors++;
		errno = EINVAL;
		return (-1);
	}
	result = __real_munmap(addr, len);
	if (result == 0)
	{
		tracked_map = NULL;
		unmaps++;
	}
	else
		errors++;
	return (result);
}

int __wrap_symbol_type(t_symbol *symbol, t_section_table *table)
{
	if (!tracked_map)
		errors++;
	type_calls++;
	return (__real_symbol_type(symbol, table));
}

void __wrap_print_symbols(t_symbol *symbols, size_t count)
{
	if (!tracked_map)
		errors++;
	print_calls++;
	__real_print_symbols(symbols, count);
}

int main(int argc, char **argv)
{
	int before_fd, after_fd, before_maps, after_maps, result;
	FILE *report;
	const char *value;

	fault = getenv("NM_FAULT");
	if (!fault)
		fault = "none";
	value = getenv("NM_FAIL_AT");
	fail_at = 1;
	if (value)
		fail_at = (unsigned)strtoul(value, NULL, 10);
	value = getenv("NM_FAULT_ERRNO");
	fault_errno = EIO;
	if (value)
		fault_errno = atoi(value);
	poison_close = getenv("NM_POISON_CLOSE") != NULL;
	before_fd = fd_count();
	before_maps = fixture_map_count();
	result = nm_cli_main(argc, argv);
	after_fd = fd_count();
	after_maps = fixture_map_count();
	if (before_fd < 0 || before_maps < 0 || before_fd != after_fd
		|| before_maps != after_maps || tracked_fd >= 0 || tracked_map
		|| opens != closes || maps != unmaps)
		errors++;
	report = fopen(getenv("NM_AUDIT_REPORT"), "w");
	if (!report)
		return (98);
	fprintf(report, "{\"opens\":%u,\"closes\":%u,\"fstats\":%u,"
		"\"map_attempts\":%u,\"maps\":%u,\"unmaps\":%u,\"peak\":%u,"
		"\"type_calls\":%u,\"print_calls\":%u,\"injected\":%u,"
		"\"errors\":%u,\"fd_before\":%d,\"fd_after\":%d,"
		"\"fixture_maps_before\":%d,\"fixture_maps_after\":%d}\n",
		opens, closes, stats, map_attempts, maps, unmaps, peak, type_calls,
		print_calls, injected, errors, before_fd, after_fd, before_maps, after_maps);
	fclose(report);
	if (errors)
		return (97);
	return (result);
}
