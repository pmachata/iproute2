/*
 * f_event.c		Qdisc Event Classifier
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <linux/limits.h>
#include <linux/ip.h>

#include "utils.h"
#include "tc_util.h"
#include "rt_names.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... event drop [ skip_sw | skip_hw ]\n"
		"		  [ action ACTION-SPEC ]\n"
		"\n"
		"Where:	ACTION-SPEC := ... look at individual actions\n");
}

static int qevent_parse_opt(struct filter_util *qu, char *handle,
			    int argc, char **argv, struct nlmsghdr *n)
{
	int ret;
	struct tcmsg *t = NLMSG_DATA(n);
	struct rtattr *tail;
	bool has_hook = false;
	__u32 flags = 0;

	if (handle) {
		ret = get_u32(&t->tcm_handle, handle, 0);
		if (ret) {
			fprintf(stderr, "Illegal \"handle\"\n");
			return -1;
		}
	}

	tail = (struct rtattr *) (((void *) n) + NLMSG_ALIGN(n->nlmsg_len));
	addattr_l(n, MAX_MSG, TCA_OPTIONS, NULL, 0);

	while (argc > 0) {
		if (!strcmp(*argv, "help")) {
			explain();
			return -1;
		} else if (!matches(*argv, "action")) {
			NEXT_ARG();
			ret = parse_action(&argc, &argv, TCA_QEVENT_ACT, n);
			if (ret) {
				fprintf(stderr, "Illegal \"action\"\n");
				return -1;
			}
			continue;
		} else if (!matches(*argv, "skip_hw")) {
			flags |= TCA_CLS_FLAGS_SKIP_HW;
		} else if (!matches(*argv, "skip_sw")) {
			flags |= TCA_CLS_FLAGS_SKIP_SW;
		} else if (!has_hook && !matches(*argv, "drop")) {
			ret = addattr32(n, MAX_MSG, TCA_QEVENT_HOOK,
					QEVENT_HOOK_DROP);
			if (ret)
				return ret;

			has_hook = true;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!has_hook) {
		fprintf(stderr, "No hook specified.\n");
		explain();
		return -1;
	}

	ret = addattr32(n, MAX_MSG, TCA_QEVENT_FLAGS, flags);
	if (ret)
		return ret;

	tail->rta_len = (((void *)n)+n->nlmsg_len) - (void *)tail;

	return 0;
}

static const char *qevent_hook_str[] = {
	"drop",
};

static int qevent_print_opt(struct filter_util *qu, FILE *f,
			    struct rtattr *opt, __u32 handle)
{
	struct rtattr *tb[TCA_QEVENT_MAX + 1];
	char *buf;

	if (!opt)
		return 0;

	parse_rtattr_nested(tb, TCA_QEVENT_MAX, opt);

	if (handle)
		print_uint(PRINT_ANY, "handle", "handle 0x%x ", handle);

	if (tb[TCA_QEVENT_HOOK]) {
		__u32 hook = rta_getattr_u32(tb[TCA_QEVENT_HOOK]);
		int rc;

		if (hook < ARRAY_SIZE(qevent_hook_str))
			rc = asprintf(&buf, "%s", qevent_hook_str[hook]);
		else
			rc = asprintf(&buf, "unknown(%d)", hook);
		if (rc < 0)
			return rc;

		print_string(PRINT_ANY, "hook", "%s ", buf);
		free(buf);
	}

	if (tb[TCA_QEVENT_ACT])
		tc_print_action(f, tb[TCA_QEVENT_ACT], 0);

	return 0;
}

struct filter_util qevent_filter_util = {
	.id = "qevent",
	.parse_fopt = qevent_parse_opt,
	.print_fopt = qevent_print_opt,
};
