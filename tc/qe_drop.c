#include "utils.h"
#include "tc_util.h"

static int qe_drop_parse(struct qevent_util *qe, int *p_argc, char ***p_argv,
			 struct nlmsghdr *n)
{
	char **argv = *p_argv;
	int argc = *p_argc;
	__u32 flags = 0;

	while (argc > 0) {
		if (strcmp(*argv, "skip_hw") == 0) {
			NEXT_ARG();
			flags |= TCA_CLS_FLAGS_SKIP_HW;
			continue;
		} else if (strcmp(*argv, "skip_sw") == 0) {
			NEXT_ARG();
			flags |= TCA_CLS_FLAGS_SKIP_SW;
			continue;
		} else {
			break;
		}
	}

	if (flags) {
		// extract to a helper
		if (!(flags ^ (TCA_CLS_FLAGS_SKIP_HW |
			       TCA_CLS_FLAGS_SKIP_SW))) {
			fprintf(stderr,
				"skip_hw and skip_sw are mutually exclusive\n");
			return -1;
		}
		addattr32(n, MAX_MSG, TCA_QEVENT_FLAGS, flags);
	}

	*p_argc = argc;
	*p_argv = argv;
	return 0;
}

struct qevent_util drop_qevent_util = {
	.id = "drop",
	.parse_qevent = qe_drop_parse,
};
