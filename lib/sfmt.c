/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "sfmt.h"

#include <stdarg.h>

struct SFMT_initializer SFMT_initialize(char *restrict buf,
					const char *restrict format, ...)
{
	va_list va;

	va_start(va, format);
	vsprintf(buf, format, va);
	va_end(va);

	return (struct SFMT_initializer){};
}
