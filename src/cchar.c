#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pfasta.h"

int count(const pfasta_seq *);
void print_counts(const size_t *counts);

//const int CHARS = 128;
#define CHARS 128
size_t counts_total[CHARS];
size_t counts_local[CHARS];

enum {
	NONE = 0,
	PRINT_TOTAL = 1,
	CASE_INSENSITIVE = 2
} FLAGS = 0;

int main(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "hit")) != -1) {
		switch (c) {
			case 'i':
				FLAGS |= CASE_INSENSITIVE;
				break;
			case 't':
				FLAGS |= PRINT_TOTAL;
				break;
			case 'h':
			case '?':
			default:
				fprintf(stderr, "Usage: %s [-hit] [FASTA...]\n", argv[0]);
				return 1;
		}
	}

	bzero(counts_total, sizeof(counts_total));


	argv += optind;

	int firsttime = 1;
	int exit_code = EXIT_SUCCESS;

	for (;; firsttime = 0) {
		int file_descriptor;
		const char *file_name;
		if (!*argv) {
			if (!firsttime) break;

			file_descriptor = STDIN_FILENO;
			file_name = "stdin";
		} else {
			file_name = *argv++;
			file_descriptor = open(file_name, O_RDONLY);
			if (file_descriptor < 0) err(1, "%s", file_name);
		}

		int l;
		pfasta_file pf;
		if ((l = pfasta_parse(&pf, file_descriptor)) != 0) {
			warnx("%s: %s", file_name, pfasta_strerror(&pf));
			exit_code = EXIT_FAILURE;
			goto fail;
		}

		pfasta_seq ps;
		while ((l = pfasta_read(&pf, &ps)) == 0) {
			count(&ps);
			printf(">%s\n", ps.name);
			print_counts(counts_local);
			pfasta_seq_free(&ps);
			for (size_t i = 0; i< CHARS; i++){
				counts_total[i] += counts_local[i];
			}
		}

		if (l < 0) {
			warnx("%s: %s", file_name, pfasta_strerror(&pf));
			exit_code = EXIT_FAILURE;
			pfasta_seq_free(&ps);
		}

		if (FLAGS & PRINT_TOTAL) {
			printf(">%s\n", file_name);
			print_counts(counts_total);
		}

	fail:
		pfasta_free(&pf);
		close(file_descriptor);
	}

	return exit_code;
}

int count(const pfasta_seq *ps)
{
	int ret = 0;
	bzero(counts_local, sizeof(counts_local));
	const char *ptr = ps->seq;

	while (*ptr) {
		unsigned char c = *ptr;
		if (FLAGS & CASE_INSENSITIVE) {
			c = toupper(c);
		}
		if (c < CHARS) {
			counts_local[c]++;
		}
		ptr++;
	}

	return ret;
}

void print_counts(const size_t *counts)
{
	for (int i = 1; i < CHARS; ++i) {
		if (counts[i]) {
			printf("%c: %zu\n", i, counts[i]);
		}
	}
}
