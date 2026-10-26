#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

_Static_assert(sizeof(size_t) == 4, "32-bit size_t required");
_Static_assert(sizeof(off_t) == 8, "64-bit off_t required");

int main(void)
{
	return (0);
}
