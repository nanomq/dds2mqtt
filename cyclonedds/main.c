#include <stdio.h>
#include <string.h>
#include "subpub.h"

const char *usage = "dds2mqtt {sub|subscriber|pub|publisher}";

int
main(int argc, char ** argv) {
	if (argc < 2)
		goto helper;

	if ((strcmp(argv[1], "sub") == 0) || (strcmp(argv[1], "subscriber") == 0))
	{
		subscriber(argc, argv);
	}
	else if ((strcmp(argv[1], "pub") == 0) || (strcmp(argv[1], "publisher") == 0))
	{
		publisher(argc, argv);
	}

	return 0;

helper:

	printf("%s\n", usage);
	return 1;
}
