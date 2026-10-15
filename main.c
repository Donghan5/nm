#include "ft_nm.h"

static void usage( )
{
    ft_putstr_fd();
}


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        ft_putstr_fd("Usage: ./ft_nm <file>\n", 2);
        return (1);
    }
    for (int i = 1; i < argc; i++)
    {
        ft_nm(argv[i]);
    }
    return (0);
}
