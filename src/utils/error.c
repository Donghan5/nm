#include "ft_nm.h"

int report_error(const char *filename, const char *message)
{
	fprintf(stderr, "Error: %s: %s\n", filename, message);
	return (-1);
}