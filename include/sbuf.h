/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __SBUF_H__
#define __SBUF_H__ 1

#include <stdarg.h>
#include <stdint.h>

struct sbuf {
	uintptr_t buf;
	unsigned int len;
	unsigned int cap;
	int err;
};

const char *sbuf_va(struct sbuf *sb, const char *format, va_list va);
const char *sbuf_fmt(struct sbuf *sb, const char *format, ...)
	__attribute__((format(printf, 2, 3)));

const char *sbuf_append_fmt(struct sbuf *sb, const char *format, ...)
	__attribute__((format(printf, 2, 3)));

const char *sbuf_str(const struct sbuf *sb);

void sbuf_free(struct sbuf *sb);

#endif /* __SBUF_H__ */
