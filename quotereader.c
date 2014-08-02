#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "quotereader.h"

FILE * quotestxt;
char ** quotebuffer;
int quotenum;

static void convert_to_linebuffer( char * source, size_t source_sz)
{
	int i = 0, start = 0, end = 0, quotebufferlen = 100; // TODO: Fix the reallocation

	quotebuffer = malloc(sizeof(char*) * quotebufferlen);

	for (; end < source_sz; end++) {
		if ('\n' == source[end]) {
			quotebuffer[i] = calloc(1,end - start + 2);
			strncpy(quotebuffer[i], source + start, end - start);
printf("%s\n",quotebuffer[i]);
			if (++i == quotebufferlen) {
				realloc(quotebuffer, sizeof(char*) * (quotebufferlen *= 2));
				assert(quotebuffer != NULL);
			}
			start = ++end;
		}

	}
	quotenum = i;
}

int init_quotes(void)
{
	char * tempbuffer;
	long size;

	quotestxt = fopen("quotes.txt","r");
	if (NULL == quotestxt) {
		fprintf(stderr,"Could not open quotes.txt, my life is meaningless!\n");
		return 1;
	}
	fseek(quotestxt, 0, SEEK_END);
	size = ftell(quotestxt);
	rewind(quotestxt);

	tempbuffer = malloc(size);
	fread(tempbuffer, 1, size, quotestxt);

	fclose(quotestxt);

	convert_to_linebuffer(tempbuffer, size);

	free(tempbuffer);
	return 0;
}

char * get_random_quote(void)
{
	return quotebuffer[random() % quotenum];
}
