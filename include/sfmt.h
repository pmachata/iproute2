/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __SBUF_H__
#define __SBUF_H__ 1

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <alloca.h>

struct SFMT_initializer {};
struct SFMT_initializer SFMT_initialize(char *restrict buf,
					const char *restrict format, ...);

#define SFMT_BUFLEN(FMT, ...)						\
	({								\
		int SFMT_len = snprintf(NULL, 0, (FMT), __VA_ARGS__);	\
		assert(SFMT_len >= 0);					\
		SFMT_len + 1;						\
	})

#define SFMT_ARRAY(NAME, FMT, ...)					\
	char NAME[SFMT_BUFLEN(FMT, ##__VA_ARGS__)];			\
	__attribute__((unused)) struct SFMT_initializer			\
	NAME##_SFMT_initializer = SFMT_initialize(NAME, (FMT), __VA_ARGS__)

#define SFMT_ALLOCA(NAME, FMT, ...)					\
	char *NAME = alloca(SFMT_BUFLEN(FMT, ##__VA_ARGS__));		\
	__attribute__((unused)) struct SFMT_initializer			\
	NAME##_SFMT_initializer =					\
		SFMT_initialize(NAME, (FMT), __VA_ARGS__);

#define SFMT_0(IMPL, NAME, FMT, ...)					\
	SFMT_##IMPL(NAME, (FMT), ##__VA_ARGS__)

#define SFMT_1(IMPL, NAME, FMT, A1, ...)				\
	typeof(A1) SFMT_ ## NAME ## _A1 = (A1);				\
	SFMT_0(IMPL, NAME, FMT,						\
	       SFMT_ ## NAME ## _A1, ##__VA_ARGS__)

#define SFMT_2(IMPL, NAME, FMT, A1, A2, ...)				\
	typeof(A2) SFMT_ ## NAME ## _A2 = (A2);				\
	SFMT_1(IMPL, NAME, FMT, A1,					\
	       SFMT_ ## NAME ## _A2, ##__VA_ARGS__)

#define SFMT_3(IMPL, NAME, FMT, A1, A2, A3, ...)			\
	typeof(A3) SFMT_ ## NAME ## _A3 = (A3);				\
	SFMT_2(IMPL, NAME, FMT, A1, A2,					\
	       SFMT_ ## NAME ## _A3, ##__VA_ARGS__)

#define SFMT_4(IMPL, NAME, FMT, A1, A2, A3, A4, ...)			\
	typeof(A4) SFMT_ ## NAME ## _A4 = (A4);				\
	SFMT_3(IMPL, NAME, FMT, A1, A2, A3,				\
	       SFMT_ ## NAME ## _A4, ##__VA_ARGS__)

#define SFMT_5(IMPL, NAME, FMT, A1, A2, A3, A4, A5, ...)		\
	typeof(A5) SFMT_ ## NAME ## _A5 = (A5);				\
	SFMT_4(IMPL, NAME, FMT, A1, A2, A3, A4,				\
	       SFMT_ ## NAME ## _A5, ##__VA_ARGS__)

#define SFMT_6(IMPL, NAME, FMT, A1, A2, A3, A4, A5, A6, ...)		\
	typeof(A6) SFMT_ ## NAME ## _A6 = (A6);				\
	SFMT_5(IMPL, NAME, FMT, A1, A2, A3, A4, A5,			\
	       SFMT_ ## NAME ## _A6, ##__VA_ARGS__)

#define PP_CONCAT2(A, B) A ## B
#define PP_CONCAT(A, B) PP_CONCAT2(A, B)

#define PP_NARG3(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define PP_NARG2(...) PP_NARG3(__VA_ARGS__)
#define PP_NARG(...) PP_NARG2(_, ##__VA_ARGS__,				\
			      9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define DO_SFMT(IMPL, NAME, FMT, ...)					\
	PP_CONCAT(SFMT_, PP_NARG(__VA_ARGS__))				\
		(IMPL,NAME, FMT, ##__VA_ARGS__)

#define SFMT(NAME, FMT, ...)	DO_SFMT(ARRAY, NAME, FMT, ##__VA_ARGS__)

#define ASFMT(FMT, ...)							\
	({								\
		DO_SFMT(ALLOCA, _##__LINE__, FMT, ##__VA_ARGS__);	\
		_##__LINE__;						\
	})

#endif /* __SBUF_H__ */
