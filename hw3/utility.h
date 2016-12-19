#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int valid_filename(const char* str)
{
	int i = 0;
	char c;
	while (str[i] != '\0') {
		c = str[i++];
		if (!isalnum(c) && (c != '_')) {
			return 0;
		}
	}

	return 1;
}
