#include <stdint.h>

__attribute__((__naked__)) void __umoddi3() {
    asm volatile(
        "movl 4(%%esp), %%eax\n\n"
        "movl 8(%%esp), %%edx\n\t"
        "divl 12(%%esp)\n\t"
        "movl %%edx, %%eax\n\t"
        "xorl %%edx, %%edx\n\t"
        "ret\n\t"
        ::: "edx"
    );
}

__attribute__((__naked__)) void __moddi3() {
    asm volatile(
        "movl 4(%%esp), %%eax\n\n"
        "movl 8(%%esp), %%edx\n\t"
        "idivl 12(%%esp)\n\t"
        "movl %%edx, %%eax\n\t"
        "xorl %%edx, %%edx\n\t"
        "ret\n\t"
        ::: "edx"
    );
}

__attribute__((__naked__)) void __udivdi3() {
    asm volatile(
        "movl 4(%%esp), %%eax\n\n"
        "movl 8(%%esp), %%edx\n\t"
        "divl 12(%%esp)\n\t"
        "xorl %%edx, %%edx\n\t"
        "ret\n\t"
        ::: "edx"
    );
}

__attribute__((__naked__)) void __divdi3() {
    asm volatile(
        "movl 4(%%esp), %%eax\n\n"
        "movl 8(%%esp), %%edx\n\t"
        "idivl 12(%%esp)\n\t"
        "xorl %%edx, %%edx\n\t"
        "ret\n\t"
        ::: "edx"
    );
}
