#include <stdio.h>

#define ESC "\033"

int main(){
	printf(ESC"[2J");
	printf(ESC"[H");
	return 0;
}
