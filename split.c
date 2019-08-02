/*
 * UNG's Not GNU
 * 
 * Copyright (c) 2011, Jakob Kaivo <jakob@kaivo.net>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define K_BYTES	1024
#define M_BYTES 1048576

static unsigned long power(int base, int power)
{
	unsigned long r = base;
	if (power < 1)
		return 1;
	for (r = base; power > 1; power--)
		r *= r;
	return r;
}

static int
split(const char *in, const char *base, int suffix, uintmax_t lines, uintmax_t bytes)
{
	FILE *f = stdin;
	FILE *o = NULL;
	char oname[FILENAME_MAX];
	uintmax_t chunk = 0;
	int i;
	char *buf;
	size_t len;
	ssize_t nread;

	if (strcmp("-", in))
		f = fopen(in, "r");

	if (bytes > 0)
		buf = malloc(bytes);
	else
		buf = malloc(BUFSIZ);

	memset(oname, '\0', FILENAME_MAX);
	strcpy(oname, base);

	while (!feof(f)) {
		if (chunk >= power(26, suffix))
			return 2;	// to much data

		for (i = suffix; i > 0; i--)
			oname[strlen(base) + suffix - i] =
			    ((chunk % power(26, i)) / power(26, i - 1)) + 'a';
		o = fopen(oname, "w");

		if (lines > 0) {
			for (i = lines; !feof(f) && i > 0; i--) {
				if ((nread = getline(&buf, &len, f)) != -1)
					fwrite(buf, sizeof(char), nread, o);
			}
		} else {
			if ((nread = fread(buf, sizeof(char), bytes, f)) != -1)
				fwrite(buf, sizeof(char), nread, o);
		}

		fclose(o);
		chunk++;
	}

	if (strcmp("-", in))
		fclose(f);

	return 0;
}

int main(int argc, char **argv)
{
	int c;
	uintmax_t lines = 0;
	uintmax_t bytes = 0;
	int suffix = 2;
	char *end;

	while ((c = getopt(argc, argv, "l:a:b:")) != -1) {
		switch (c) {
		case 'l':
			bytes = 0;
			lines = strtoumax(optarg, &end, 10);
			if (end != NULL && strlen(end) > 0) {
				return 1;
			}
			break;

		case 'a':
			suffix = strtol(optarg, &end, 10);
			if (end != NULL && strlen(end) > 0) {
				return 1;
			}
			break;

		case 'b':
			lines = 0;
			bytes = strtoumax(optarg, &end, 10);
			if (end != NULL) {
				if (!strcmp("k", end)) {
					bytes *= K_BYTES;
				} else if (!strcmp("m", end)) {
					bytes *= M_BYTES;
				} else {
					return 1;
				}
			}
			break;

		default:
			return 1;
		}
	}

	if (lines == 0 && bytes == 0) {
		lines = 1000;
	}

	if (optind >= argc) {
		split("-", "x", suffix, lines, bytes);
	} else if (optind == argc - 1) {
		split(argv[optind], "x", suffix, lines, bytes);
	} else if (optind == argc - 2) {
		split(argv[optind], argv[optind + 1], suffix, lines, bytes);
	} else {
		return 1;
	}

	return 0;
}
