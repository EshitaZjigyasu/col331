#include "types.h"
#include "x86.h"

void *
memset(void *dst, int c, uint n) //
{
    if ((int)dst % 4 == 0 && n % 4 == 0) // write 32 bits at once if the starting address is 4 byte aligned and the number of copies that need to be written is a multiple of 4
    {
        c &= 0xFF;
        stosl(dst, (c << 24) | (c << 16) | (c << 8) | c, n / 4);
    }
    else // else fallback to writing the lowest 8 bits of c byte by byte n times (this is slower than writing in multiples of 32 bits)
        stosb(dst, c, n);
    return dst;
}

int memcmp(const void *v1, const void *v2, uint n)
{
    const uchar *s1, *s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0)
    {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void *
memmove(void *dst, const void *src, uint n)
{
    const char *s;
    char *d;

    s = src;
    d = dst;
    if (s < d && s + n > d) // in this case, if we move in the forward direction, then at some point it will happen that what we have to copy has already been overwritten by a previous move. hence in this case, we move in the backward direction
    {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    }
    else
        while (n-- > 0)
            *d++ = *s++;

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void *
memcpy(void *dst, const void *src, uint n)
{
    return memmove(dst, src, n);
}

int strncmp(const char *p, const char *q, uint n) // compare the first n characters of p and q
{
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (uchar)*p - (uchar)*q;
}

char *
strncpy(char *s, const char *t, int n)
{
    char *os;

    os = s;
    while (n-- > 0 && (*s++ = *t++) != 0) // *s++ = *t++ will return the value that is assigned is *t. so the second expression remains true till we do not copy the null terminator. so in this case, null termination is not guaranteed
        ;
    while (n-- > 0) // if t has less than n characters, make the remaining characters in s as 0 (the null terminator)
        *s++ = 0;
    return os; // return the pointer to the start of the destination where the string t was copied
}

// Like strncpy but guaranteed to NUL-terminate.
char *
safestrcpy(char *s, const char *t, int n)
{
    char *os;

    os = s;
    if (n <= 0)
        return os;
    while (--n > 0 && (*s++ = *t++) != 0)
        ;
    *s = 0; // guaranteed null termination
    return os;
}

int strlen(const char *s)
{
    int n;

    for (n = 0; s[n]; n++)
        ;
    return n;
}
