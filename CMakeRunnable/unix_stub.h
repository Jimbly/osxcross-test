#pragma once
#if defined(__unix) || defined(__APPLE__)
#include <stdio.h>

typedef int errno_t;

errno_t fopen_s(FILE **f, const char *name, const char *mode);
#endif