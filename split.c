/*
 * UNG's Not GNU
 *
 * Copyright (c) 2011-2019, Jakob Kaivo <jkk@ung.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define K_BYTES			1024
#define M_BYTES			1048576

#define DEFAULT_BASE		"x"
#define DEFAULT_SUFFIX_LENGTH	2
#define DEFAULT_LINES		1000

static char *nextsuffix(size_t n, char s[])
{
	const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
	if (s[0] == '\0') {
		memset(s, alphabet[0], n);
		return s;
	}

	n--;
	for (;;) {
		s[n] = *(strchr(alphabet, s[n]) + 1);
		if (s[n] == '\0') {
			if (n == 0) {
				return NULL;
			}
			s[n--] = alphabet[0];
		} else {
			break;
		}
	}
	return s;
}

static int split(const char *in, const char *base, size_t suffixlen, uintmax_t lines, uintmax_t bytes)
{
	char oname[FILENAME_MAX] = {0};
	char suffix[suffixlen + 2];
	FILE *o = NULL;
	FILE *f = stdin;
	uintmax_t count = 0;

	memset(suffix, '\0', suffixlen + 1);

	if (in != NULL && strcmp("-", in) != 0) {
		f = fopen(in, "r");
	}

	if (f == NULL) {
		perror("fopen");
		return 1;
	}

	if (in != NULL && base == NULL) {
		base = DEFAULT_BASE;
	}

	int c;
	while ((c = fgetc(f)) != EOF) {
		if (o == NULL) {
			char *s = nextsuffix(suffixlen, suffix);
			if (s == NULL) {
				fprintf(stderr, "too many chunks\n");
				return 1;
			}
			snprintf(oname, sizeof(oname), "%s%s", base, s);
			o = fopen(oname, "w");
			if (o == NULL) {
				perror("fopen");
				return 1;
			}
		}

		fputc(c, o);

		if (bytes != 0 || c == '\n') {
			count++;
		}

		if ((bytes != 0 && count == bytes) || count == lines) {
			fclose(o);
			o = NULL;
			count = 0;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	uintmax_t lines = 0;
	uintmax_t bytes = 0;
	size_t suffix = DEFAULT_SUFFIX_LENGTH;
	char *end;

	setlocale(LC_ALL, "");

	int c;
	while ((c = getopt(argc, argv, "l:a:b:")) != -1) {
		switch (c) {
		case 'l':	/* line_count */
			bytes = 0;
			lines = strtoumax(optarg, &end, 10);
			if (*end != '\0') {
				return 1;
			}
			break;

		case 'a':	/* suffix_length */
			suffix = strtoul(optarg, &end, 10);
			if (end != NULL && strlen(end) > 0) {
				return 1;
			}
			break;

		case 'b':	/* bytes */
			lines = 0;
			bytes = strtoumax(optarg, &end, 10);
			if (*end != '\0') {
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
		lines = DEFAULT_LINES;
	}

	if (optind >= argc) {
		split(NULL, NULL, suffix, lines, bytes);
	} else if (optind == argc - 1) {
		split(argv[optind], NULL, suffix, lines, bytes);
	} else if (optind == argc - 2) {
		split(argv[optind], argv[optind + 1], suffix, lines, bytes);
	} else {
		return 1;
	}

	return 0;
}
