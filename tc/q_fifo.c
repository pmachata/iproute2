/*
 * q_fifo.c		FIFO.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
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
#include "tc_common.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... <[p|b]fifo | pfifo_head_drop> [ limit NUMBER ]\n");
}

static int fifo_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			  __u32 *p_limit, bool *p_has_limit)
{
	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(p_limit, *argv)) {
				fprintf(stderr, "%s: Illegal value for \"limit\": \"%s\"\n", qu->id, *argv);
				return -1;
			}
			*p_has_limit = true;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "%s: unknown parameter \"%s\"\n", qu->id, *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	return 0;
}

static int fifo_parse_opt_newstyle(struct qdisc_util *qu, int argc, char **argv,
				   struct nlmsghdr *n, const char *dev)
{
	struct {
		struct nlmsghdr n;
		char buf[TCA_BUF_MAX];
	} opt = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tc_fifo_qopt)),
	};
	bool has_limit = false;
	__u32 limit;
	int err;

	err = fifo_parse_opt(qu, argc, argv, &limit, &has_limit);
	if (err)
		return err;

	if (!has_limit)
		return 0;

	addattr32(&opt.n, TCA_BUF_MAX, TCA_FIFO_LIMIT, limit);
	addattr_l(n, 1024, TCA_OPTIONS | NLA_F_NESTED, NLMSG_DATA(&opt.n),
		  NLMSG_PAYLOAD(&opt.n, 0));
	return 0;
}

static int fifo_parse_opt_oldstyle(struct qdisc_util *qu, int argc, char **argv,
				   struct nlmsghdr *n, const char *dev)
{
	struct tc_fifo_qopt opt = {};
	bool has_limit = false;
	int err;

	err = fifo_parse_opt(qu, argc, argv, &opt.limit, &has_limit);
	if (err)
		return err;

	if (has_limit)
		addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
	return 0;
}

static void fifo_print_limit(struct qdisc_util *qu, FILE *f, __u32 limit)
{
	if (strcmp(qu->id, "bfifo") == 0)
		print_size(PRINT_ANY, "limit", "limit %s", limit);
	else
		print_uint(PRINT_ANY, "limit", "limit %up", limit);
}

static int fifo_print_opt_newstyle(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_ETS_MAX + 1];
	struct rtattr *attr;
	__u32 limit;

	/* Convert tc_fifo_qopt at the beginning of the payload to an NLA header. */
	attr = RTA_DATA(opt);
	attr->rta_len = opt->rta_len - sizeof(struct tc_fifo_qopt);
	attr->rta_type = opt->rta_type;

	parse_rtattr_nested(tb, TCA_FIFO_MAX, attr);

	if (!tb[TCA_FIFO_LIMIT]) {
		fprintf(stderr, "FIFO limit not dumped\n");
		return -1;
	}

	limit = rta_getattr_u32(tb[TCA_FIFO_LIMIT]);
	fifo_print_limit(qu, f, limit);

	return 0;
}

static int fifo_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct tc_fifo_qopt *qopt;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;
        if (RTA_PAYLOAD(opt) > sizeof(*qopt))
            return fifo_print_opt_newstyle(qu, f, opt);

	qopt = RTA_DATA(opt);
	fifo_print_limit(qu, f, qopt->limit);
	return 0;
}


struct qdisc_util bfifo_qdisc_util = {
	.id = "bfifo",
	.parse_qopt = fifo_parse_opt_newstyle,
	.print_qopt = fifo_print_opt,
};

struct qdisc_util pfifo_qdisc_util = {
	.id = "pfifo",
	.parse_qopt = fifo_parse_opt_newstyle,
	.print_qopt = fifo_print_opt,
};

struct qdisc_util pfifo_head_drop_qdisc_util = {
	.id = "pfifo_head_drop",
	.parse_qopt = fifo_parse_opt_oldstyle,
	.print_qopt = fifo_print_opt,
};

struct qdisc_util pfifo_fast_qdisc_util = {
	.id = "pfifo_fast",
	.print_qopt = prio_print_opt,
};
