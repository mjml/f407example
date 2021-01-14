#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main (int argc, char* argv[])
{
	int r=0;
	setuid(0);
	if (argc > 1) {
		if (!strcmp(argv[1],"run")) {
			printf("Running vm...\n");
			r=system("./vm.sh");
		} else if (!strcmp(argv[1], "reattach")) {
			printf("Reattaching usb device...\n");
			r=system("./reconnect-usb.sh");
		} else {
			goto usage;
		}
	} else {
	usage:
		printf("Usage:\n  vmctl run|reattach\n");
	}
	
	return r;
}
