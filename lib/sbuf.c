/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "sbuf.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *sbuf_buf(struct sbuf *sb)
{
	return (char *) sb->buf;
}

static char *sbuf_end(const struct sbuf *sb)
{
	return sbuf_buf((struct sbuf *) sb) + sb->len;
}

static unsigned int sbuf_nleft(const struct sbuf *sb)
{
	return sb->cap - sb->len;
}

static void sbuf_reset(struct sbuf *sb)
{
	sb->err = 0;
	sb->len = 0;
}

void sbuf_free(struct sbuf *sb)
{
	free(sbuf_buf(sb));
}

const char *sbuf_str(const struct sbuf *sb)
{
	char *buf = sbuf_buf((struct sbuf *) sb);
	if (sb->len)
		/* Even for errors, return the buffer that we have, to give any
		 * partial information available. */
		return buf;
	if (sb->err)
		/* If we have nothing of value to give back, at least inform the
		 * user about the error. */
		return strerror(sb->err);
	return "";
}

static int sbuf_reserve(struct sbuf *sb, unsigned int additional)
{
	unsigned int min_len = sb->len + additional;
	unsigned int new_len = sb->cap;
	char *new_buf;

	if (new_len < 16)
		new_len = 16;

	while (new_len < min_len)
		new_len *= 2;

	new_buf = realloc(sbuf_buf(sb), new_len);
	if (new_buf == NULL) {
		sb->err = errno;
		return -1;
	}

	sb->buf = (uintptr_t) new_buf;
	sb->cap = new_len;
	return 0;
}

static const char *
sbuf_append_va(struct sbuf *sb, const char *format, va_list va)
{
	va_list va2;
	int len;
	int rc;

	if (sb->err)
		return sbuf_str(sb);

	va_copy(va2, va);
	len = vsnprintf(sbuf_end(sb), sbuf_nleft(sb), format, va2);
	va_end(va2);

	if (len + 1 > sbuf_nleft(sb)) {
		rc = sbuf_reserve(sb, len + 1);
		if (rc)
			return sbuf_str(sb);

		vsprintf(sbuf_end(sb), format, va);
	}

	sb->len += len;
	return sbuf_str(sb);
}

const char *sbuf_append_fmt(struct sbuf *sb, const char *format, ...)
{
	const char *ret;
	va_list va;

	va_start(va, format);
	ret = sbuf_append_va(sb, format, va);
	va_end(va);

	return ret;
}

const char *sbuf_va(struct sbuf *sb, const char *format, va_list va)
{
	sbuf_reset(sb);
	return sbuf_append_va(sb, format, va);
}

const char *sbuf_fmt(struct sbuf *sb, const char *format, ...)
{
	const char *ret;
	va_list va;

	va_start(va, format);
	ret = sbuf_va(sb, format, va);
	va_end(va);

	return ret;
}
