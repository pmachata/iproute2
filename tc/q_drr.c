/*
 * q_drr.c		DRR.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Patrick McHardy <kaber@trash.net>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... drr\n");
}

static void explain2(void)
{
	fprintf(stderr, "Usage: ... drr quantum SIZE [skbprio P1 P2...]\n");
}


static int drr_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n, const char *dev)
{
	while (argc) {
		if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
	}
	return 0;
}

static int drr_parse_class_opt(struct qdisc_util *qu, int argc, char **argv,
			       struct nlmsghdr *n, const char *dev)
{
	bool skbprio_mode = false;
	struct rtattr *tail;
	__u16 defmap = 0;
	__u32 tmp;

	tail = addattr_nest(n, 1024, TCA_OPTIONS);

	while (argc > 0) {
		if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_size(&tmp, *argv)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
			addattr_l(n, 1024, TCA_DRR_QUANTUM, &tmp, sizeof(tmp));
		} else if (strcmp(*argv, "skbprio") == 0) {
			if (skbprio_mode) {
				fprintf(stderr, "Error: duplicate skbprio\n");
				return -1;
			}
			skbprio_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			explain2();
			return -1;
		} else if (skbprio_mode) {
			unsigned int skbprio;

			if (get_unsigned(&skbprio, *argv, 10)) {
				fprintf(stderr, "Illegal \"skbprio\" element\n");
				return -1;
			}
			if (skbprio > TC_PRIO_MAX) {
				fprintf(stderr, "\"skbprio\" element > TC_PRIO_MAX=%u\n",
					TC_PRIO_MAX);
				return -1;
			}
			defmap |= 1 << skbprio;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain2();
			return -1;
		}
		argc--; argv++;
	}

	addattr_l(n, 1024, TCA_DRR_DEFMAP, &defmap, sizeof(defmap));
	addattr_nest_end(n, tail);
	return 0;
}

static int drr_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_DRR_MAX + 1];
	__u16 defmap;
	int skbprio;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_DRR_MAX, opt);

	if (tb[TCA_DRR_QUANTUM])
		print_uint(PRINT_ANY, "quantum", "quantum %d ",
			   rta_getattr_u32(tb[TCA_DRR_QUANTUM]));

	if (tb[TCA_DRR_DEFMAP]) {
		defmap = rta_getattr_u32(tb[TCA_DRR_DEFMAP]);
		if (defmap) {
			open_json_array(PRINT_ANY, "skbprio");
			for (skbprio = 0; skbprio <= TC_PRIO_MAX; skbprio++) {
				if (defmap & (1 << skbprio))
					print_uint(PRINT_ANY, NULL, " %d",
						   skbprio);
			}
			close_json_array(PRINT_ANY, "");
		}
	}

	return 0;
}

static int drr_print_xstats(struct qdisc_util *qu, FILE *f, struct rtattr *xstats)
{
	struct tc_drr_stats *x;

	if (xstats == NULL)
		return 0;
	if (RTA_PAYLOAD(xstats) < sizeof(*x))
		return -1;
	x = RTA_DATA(xstats);

	print_uint(PRINT_ANY, "deficit", "  deficit %u ", x->deficit);
	return 0;
}

struct qdisc_util drr_qdisc_util = {
	.id		= "drr",
	.parse_qopt	= drr_parse_opt,
	.print_qopt	= drr_print_opt,
	.print_xstats	= drr_print_xstats,
	.parse_copt	= drr_parse_class_opt,
	.print_copt	= drr_print_opt,
};
