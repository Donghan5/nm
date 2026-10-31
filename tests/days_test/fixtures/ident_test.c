#include <stdint.h>
#include <string.h>
#include "ft_nm.h"

int check_common_range(const t_file *file, size_t offset, size_t size);
int check_elf_ident(const t_file *file);

static void valid_ident(unsigned char *data)
{
	memset(data, 0, EI_NIDENT);
	memcpy(data, ELFMAG, SELFMAG);
	data[EI_CLASS] = ELFCLASS64;
	data[EI_DATA] = ELFDATA2LSB;
	data[EI_VERSION] = EV_CURRENT;
}

static int fail(const char *message)
{
	fprintf(stderr, "FAIL: %s\n", message);
	return (1);
}

static int ranges(void)
{
	t_file file = {0};
	const size_t cases[][4] = {
		{0, 0, 0, 1}, {0, 0, 1, 0},
		{16, 0, 16, 1}, {16, 15, 1, 1}, {16, 16, 0, 1},
		{16, 16, 1, 0}, {16, 17, 0, 0}, {16, 0, 17, 0},
		{16, SIZE_MAX, 2, 0}, {16, 1, SIZE_MAX, 0},
		{SIZE_MAX, SIZE_MAX, 0, 1}, {SIZE_MAX, SIZE_MAX, 1, 0},
		{SIZE_MAX, 1, SIZE_MAX - 1, 1}
	};
	size_t i;

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		file.f_size = cases[i][0];
		if (check_common_range(&file, cases[i][1], cases[i][2])
			!= (int)cases[i][3])
		{
			fprintf(stderr, "Range case %zu failed\n", i);
			return (1);
		}
	}
	return (0);
}

static int short_inputs(void)
{
	long page = sysconf(_SC_PAGESIZE);
	unsigned char *mapping;
	t_file file = {0};
	size_t n;
	int result = 0;

	if (page < EI_NIDENT)
		return (fail("cannot determine a suitable page size"));
	mapping = mmap(NULL, (size_t)page * 2, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mapping == MAP_FAILED)
		return (fail("guard mapping failed"));
	if (mprotect(mapping + page, (size_t)page, PROT_NONE))
	{
		munmap(mapping, (size_t)page * 2);
		return (fail("guard page protection failed"));
	}
	/* Even one byte past each advertised input reaches an inaccessible page. */
	for (n = 0; n < EI_NIDENT; n++)
	{
		file.f_data = mapping + page - n;
		file.f_size = n;
		memset(file.f_data, 0, n);
		if (n >= SELFMAG)
			memcpy(file.f_data, ELFMAG, SELFMAG);
		if (check_elf_ident(&file) != 0)
			result = fail("truncated identification was accepted");
	}
	munmap(mapping, (size_t)page * 2);
	return (result);
}

static int field_values(const char *name)
{
	unsigned char data[EI_NIDENT];
	t_file file = {0};
	int index;
	int value;
	int expected;
	int first;
	int last;

	file.f_data = data;
	file.f_size = sizeof(data);
	if (!strcmp(name, "magic"))
		first = EI_MAG0, last = EI_MAG3;
	else if (!strcmp(name, "class"))
		first = EI_CLASS, last = EI_CLASS;
	else if (!strcmp(name, "endian"))
		first = EI_DATA, last = EI_DATA;
	else
		first = EI_VERSION, last = EI_VERSION;
	for (index = first; index <= last; index++)
		for (value = 0; value <= 255; value++)
		{
			valid_ident(data);
			expected = value == data[index];
			if (index == EI_CLASS)
				expected = value == ELFCLASS32 || value == ELFCLASS64;
			if (index == EI_DATA)
				expected = value == ELFDATA2LSB || value == ELFDATA2MSB;
			data[index] = (unsigned char)value;
			if (check_elf_ident(&file) != expected)
			{
				fprintf(stderr, "FAIL: field %d, value %d\n", index, value);
				return (1);
			}
		}
	return (0);
}

int main(int argc, char **argv)
{
	unsigned char data[EI_NIDENT];
	t_file file = {0};
	int cls;
	int endian;

	if (argc != 2)
		return (fail("expected one test case name"));
	if (!strcmp(argv[1], "ranges"))
		return (ranges());
	if (!strcmp(argv[1], "short-inputs"))
		return (short_inputs());
	if (strcmp(argv[1], "valid-ident"))
		return (field_values(argv[1]));
	file.f_data = data;
	file.f_size = sizeof(data);
	for (cls = ELFCLASS32; cls <= ELFCLASS64; cls++)
		for (endian = ELFDATA2LSB; endian <= ELFDATA2MSB; endian++)
		{
			valid_ident(data);
			data[EI_CLASS] = (unsigned char)cls;
			data[EI_DATA] = (unsigned char)endian;
			if (check_elf_ident(&file) != 1)
				return (fail("valid identification was rejected"));
		}
	return (0);
}
