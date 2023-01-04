#include <stdio.h>
#include <string.h>

#include "dds_client.h"

const char *usage = "dds2mqtt {sub|pub|proxy}";

int
main(int argc, char ** argv) {
	if (argc < 2)
		goto helper;

	if (strcmp(argv[1], "sub") == 0)
	{
		subscriber(argc, argv);
	}
	else if (strcmp(argv[1], "pub") == 0)
	{
		publisher(argc, argv);
	}
	else if (strcmp(argv[1], "proxy") == 0)
	{
		dds_proxy(argc, argv);
	}

	return 0;

helper:

	printf("%s\n", usage);
	return 1;
}
