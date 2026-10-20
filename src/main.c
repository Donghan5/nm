#include "ft_nm.h"

static void usage( )
{
    ft_putstr_fd();
}


int main(int argc, char **argv)
{
    // --  peusdo code
    // 1. verify arguments, if no file name is provided, using a.out as default
    // 2. check the validity of the flag, if invalid flag is provided, print usage and exit
	// 3. Read file bytes into memory and record the size
	// 4. Validate ELF identification (minimum size, magic, class, endian)
	// 5. Parse and validate the matching ELF header and section header table
	// 6. symbol table parsing, loop in section header table, find symbol table and string table, parse them
	// 7. Connect symbol
	// 8. Sort string
	// 9. filtering and printing symbol table
	// 10. Free memory and exit
	return (0);
}
