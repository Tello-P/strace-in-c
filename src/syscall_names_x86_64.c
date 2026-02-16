#include "syscall_names_x86_64.h"

const char *get_syscall_name(size_t nr)
{
    if (nr > SYSCALL_MAX_NR) {
        return "unknown";
    }
    return syscall_names[nr] ? syscall_names[nr] : "reserved";
}
