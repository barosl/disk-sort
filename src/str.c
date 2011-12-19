#include "str.h"

#define IS_WHITESPACE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == '\r')

char *str_strip(char *text) {
	while (*text && IS_WHITESPACE(*text) && text++);
	char *start = text;
	while (*text && text++);
	text--;
	while (text >= start && IS_WHITESPACE(*text) && text--);
	*(text+1) = '\0';
	return start;
}
