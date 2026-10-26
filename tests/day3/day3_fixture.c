/* No system headers: ELF32 object generation does not require multilib libc. */
int global_data = 7;
static int local_data = 3;
int global_bss;
const int readonly_data = 9;
extern int external_function(int);
__attribute__((weak)) int weak_data = 5;
int example_function(int value)
{
    return external_function(value + global_data + local_data + global_bss);
}
