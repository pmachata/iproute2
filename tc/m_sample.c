/*
 * m_sample.c		ingress/egress packet sampling module
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Yotam Gigi <yotamg@mellanox.com>
 *
 */

#include <stdio.h>
#include "utils.h"
#include "tc_util.h"
#include "tc_common.h"
#include <linux/tc_act/tc_sample.h>

static void explain(void)
{
	fprintf(stderr, "Usage: sample SAMPLE_CONF\n");
	fprintf(stderr, "where:\n");
	fprintf(stderr, "\tSAMPLE_CONF := SAMPLE_PARAMS | SAMPLE_INDEX\n");
	fprintf(stderr, "\tSAMPLE_PARAMS := rate RATE group GROUP [trunc SIZE] [SAMPLE_INDEX]\n");
	fprintf(stderr, "\tSAMPLE_INDEX := index INDEX\n");
	fprintf(stderr, "\tRATE := The ratio of packets observed at the data source to the samples generated.\n");
	fprintf(stderr, "\tGROUP := the psample sampling group\n");
	fprintf(stderr, "\tSIZE := the truncation size\n");
	fprintf(stderr, "\tINDEX := integer index of the sample action\n");
}

static void usage(void)
{
	explain();
	exit(-1);
}

static int parse_sample(struct action_util *a, int *argc_p, char ***argv_p,
			int tca_id, struct nlmsghdr *n)
{
	struct tc_sample p = { 0 };
	bool trunc_set = false;
	bool group_set = false;
	bool rate_set = false;
	char **argv = *argv_p;
	struct rtattr *tail;
	int argc = *argc_p;
	__u32 trunc;
	__u32 group;
	__u32 rate;

	if (argc <= 1) {
		fprintf(stderr, "sample bad argument count %d\n", argc);
		usage();
		return -1;
	}

	if (matches(*argv, "sample") == 0) {
		NEXT_ARG();
	} else {
		fprintf(stderr, "sample bad argument %s\n", *argv);
		return -1;
	}

	while (argc > 0) {
		if (matches(*argv, "rate") == 0) {
			NEXT_ARG();
			if (get_unsigned(&rate, *argv, 10) != 0) {
				fprintf(stderr, "Illegal rate %s\n", *argv);
				usage();
				return -1;
			}
			rate_set = true;
		} else if (matches(*argv, "group") == 0) {
			NEXT_ARG();
			if (get_unsigned(&group, *argv, 10) != 0) {
				fprintf(stderr, "Illegal group num %s\n",
					*argv);
				usage();
				return -1;
			}
			group_set = true;
		} else if (matches(*argv, "trunc") == 0) {
			NEXT_ARG();
			if (get_unsigned(&trunc, *argv, 10) != 0) {
				fprintf(stderr, "Illegal truncation size %s\n",
					*argv);
				usage();
				return -1;
			}
			trunc_set = true;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}

		NEXT_ARG_FWD();
	}

	parse_action_control_dflt(&argc, &argv, &p.action, false, TC_ACT_PIPE);

	NEXT_ARG_FWD();
	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&p.index, *argv, 10)) {
				fprintf(stderr, "sample: Illegal \"index\"\n");
				return -1;
			}
			NEXT_ARG_FWD();
		}
	}

	if (!p.index && !group_set) {
		fprintf(stderr, "param \"group\" not set\n");
		usage();
	}

	if (!p.index && !rate_set) {
		fprintf(stderr, "param \"rate\" not set\n");
		usage();
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, MAX_MSG, tca_id, NULL, 0);
	addattr_l(n, MAX_MSG, TCA_SAMPLE_PARMS, &p, sizeof(p));
	if (rate_set)
		addattr32(n, MAX_MSG, TCA_SAMPLE_RATE, rate);
	if (group_set)
		addattr32(n, MAX_MSG, TCA_SAMPLE_PSAMPLE_GROUP, group);
	if (trunc_set)
		addattr32(n, MAX_MSG, TCA_SAMPLE_TRUNC_SIZE, trunc);

	tail->rta_len = (char *)NLMSG_TAIL(n) - (char *)tail;

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_sample(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_SAMPLE_MAX + 1];
	struct tc_sample *p;

	if (arg == NULL)
		return -1;

	parse_rtattr_nested(tb, TCA_SAMPLE_MAX, arg);

	if (!tb[TCA_SAMPLE_PARMS] || !tb[TCA_SAMPLE_RATE] ||
	    !tb[TCA_SAMPLE_PSAMPLE_GROUP]) {
		fprintf(f, "[NULL sample parameters]");
		return -1;
	}
	p = RTA_DATA(tb[TCA_SAMPLE_PARMS]);

	fprintf(f, "sample rate 1/%d group %d",
		rta_getattr_u32(tb[TCA_SAMPLE_RATE]),
		rta_getattr_u32(tb[TCA_SAMPLE_PSAMPLE_GROUP]));

	if (tb[TCA_SAMPLE_TRUNC_SIZE])
		fprintf(f, " trunc_size %d",
			rta_getattr_u32(tb[TCA_SAMPLE_TRUNC_SIZE]));

	fprintf(f, "\n\tindex %d ref %d bind %d", p->index, p->refcnt,
		p->bindcnt);

	if (show_stats) {
		if (tb[TCA_SAMPLE_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_SAMPLE_TM]);

			print_tm(f, tm);
		}
	}
	fprintf(f, "\n");
	return 0;
}

struct action_util sample_action_util = {
	.id = "sample",
	.parse_aopt = parse_sample,
	.print_aopt = print_sample,
};
