#ifndef __TOOLS_MATH_H__
#define __TOOLS_MATH_H__

static int pow(int base, int exp) {
    int result = 1;
    for (; exp > 0; exp--) {
        result *= base;
    }
    return result;
}

#endif
