// SPDX-License-Identifier: GPL-2.0+
#include "utils.h"
#include "ip_common.h"

static const struct ipstats_stat_desc *ipstats_stat_desc_toplev_subs[] = {
};

static const struct ipstats_stat_desc ipstats_stat_desc_toplev_group = {
	.name = "top-level",
	.kind = IPSTATS_STAT_DESC_KIND_GROUP,
	.subs = ipstats_stat_desc_toplev_subs,
	.nsubs = ARRAY_SIZE(ipstats_stat_desc_toplev_subs),
};

static int do_help(void)
{
	const struct ipstats_stat_desc *toplev = &ipstats_stat_desc_toplev_group;
	int i;

	fprintf(stderr,
		"Usage: ip stats help\n"
		);

	for (i = 0; i < toplev->nsubs; i++) {
		const struct ipstats_stat_desc *desc = toplev->subs[i];

		if (i == 0)
			fprintf(stderr, "G := { %s", desc->name);
		else
			fprintf(stderr, " | %s", desc->name);
	}
	if (i > 0)
		fprintf(stderr, " }\n");

	for (i = 0; i < toplev->nsubs; i++) {
		const struct ipstats_stat_desc *desc = toplev->subs[i];
		size_t j;

		if (desc->kind != IPSTATS_STAT_DESC_KIND_GROUP)
			continue;

		for (j = 0; j < desc->nsubs; j++) {
			const struct ipstats_stat_desc *sub = desc->subs[j];

			if (j == 0)
				fprintf(stderr, "%s SG := { %s",
					desc->name, sub->name);
			else
				fprintf(stderr, " | %s", sub->name);
		}
		if (j > 0)
			fprintf(stderr, " }\n");
	}

	return 0;
}

int do_ipstats(int argc, char **argv)
{
	int rc;

	if (argc == 0) {
		do_help();
		rc = -1;
	} else if (strcmp(*argv, "help") == 0) {
		do_help();
		rc = 0;
	} else {
		fprintf(stderr, "Command \"%s\" is unknown, try \"ip stats help\".\n",
			*argv);
		rc = -1;
	}

	return rc;
}
