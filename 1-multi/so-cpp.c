#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage() {
	puts("USAGE: so-cpp [-D <SYMBOL>[=<MAPPING>]] [-I <DIR>] [<INFILE>] [ [-o] <OUTFILE>]\n");
}
int main(int argc, char *argv[]) {
	if (argc < 2 ) {
		usage();
		return -1;
	}
	int i = 1;
	char arg = argv[i];
	while(i < argc) {
		if (arg[0] == '-') {
			/*
			 * We have a new argument
			 */
			if (arg[1] == 'D') {
				/* [-D <SYMBOL>[=<MAPPING>]] */
				arg = argv[++i];
				
			}
		}
	}
	
	return 0;
}
