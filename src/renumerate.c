/* See LICENSE file for copyright and license details.
 *
 * Read a graph in edgelist format, renumerate its vertices with
 * numbers [0..N) or [INDEX..INDEX+N) and write it to output.
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "arg.h"
#include "timing.h"

void printUsage(void);
void printHelp(void);
void printArgError(void);
void showUserSomeInfo(void);

char *argv0;
char argErrDesc[128] = "unknown error";
int startFromZero = -1;
char debug = 0;


void printUsage(void) {
	printf("Usage: renumerate [-h] [-d] [-s INDEX] [INPUT] [OUTPUT]\n\n");
}

void printHelp(void) {
	printf("If INPUT is '-', read STDIN or write to STDOUT\n");
	printf("\t-h\t\tprint this help\n");
	printf("\t-d\t\tprint debugging information\n");
	printf("\t-s INDEX\tnew enumeration starts with INDEX, has to be non-negative\n");
}

void printArgError(void) {
	printf("renumerate: %s\n", argErrDesc);
	printUsage();
	printf("Try `renumerate -h' for more options.\n");
}


int main(int argc, char *argv[]) {
	unsigned long long t0 = currTimeNano();

	ARGBEGIN {
		case 's':
			sprintf(argErrDesc, "INDEX is mandatory");
			startFromZero = atoi(EARGF(printArgError()));
			if(startFromZero < 0) {
				sprintf(argErrDesc, "invalid INDEX value: '%d'", startFromZero);
				printArgError();
				return -1;
			}
			break;
		case 'd':
			debug = 1;
			break;
		case 'h':
			printUsage();
			printHelp();
			return 0;
		default:
			sprintf(argErrDesc, "invalid option: '%c'", ARGC());
			printArgError();
			return -1;
	} ARGEND

	if(argc < 1) {
		sprintf(argErrDesc, "missing INPUT");
		printArgError();
		return -1;
	} else if(argc < 2) {
		sprintf(argErrDesc, "missing OUTPUT");
		printArgError();
		return -1;
	} else if(argc > 2) {
		sprintf(argErrDesc, "too many arguments");
		printArgError();
		return -1;
	}

	if(strcmp(argv[0], "-") == 0) {
		printf("Reading input graph from STDIN not implemented.\n");
		return -1;
	}

	//read input file
	struct stat st;
	stat(argv[0], &st);
	off_t inFileSize = st.st_size;
	if(debug) {
		printf("Input file size: %llu bytes\n", inFileSize);
	}
	//TODO: create progress bar
	FILE* input = fopen(argv[0], 'r');
	if(input == NULL) {
		printf("Unable to read input file.\n");
		return -1;
	}
	//TODO: traverse input file and close input
	
	//TODO: construct renumeration structures
	
	//TODO: write output stuff

	unsigned long long t1 = currTimeNano();
	printf("Total end-to-end time: %lfs\n", (t1 - t0) * 1e-9);

	return 0;
}

