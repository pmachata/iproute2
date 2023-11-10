/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "sbuf.h"

#include <stdarg.h>

struct SFMT_initializer SFMT_initialize(char *restrict buf, size_t len,
					const char *restrict format, ...)
{
	va_list va;
	int ret;
	va_start(va, format);
	ret = vsnprintf(buf, len, format, va);
	assert(ret == len - 1);
	va_end(va);
	return (struct SFMT_initializer){};
}
