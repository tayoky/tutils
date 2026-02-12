#include <string.h>
#include <stddef.h>
#include <tutils.h>

int glob_match(const char *glob, const char *str) {
	const char *str_start  = NULL;
	const char *glob_start = NULL;

	while (*glob || *str) {
		switch (*glob) {
		case '\0':
			break;
		case '?':
			if (*str) {
				str++;
				glob++;
				continue;
			}
			break;
		case '*':
			// try to find the seach
			// if not found retry one char later
			glob_start = glob;
			str_start  = str;
			glob++;
			continue;
		default:
			if (*str == *glob){
				str++;
				glob++;
				continue;
			}
			break;
		}
		if (str_start && *str_start) {
			glob = glob_start;
			str  = ++str_start;
			continue;
		}
		return 0;
	}
	return 1;
}
