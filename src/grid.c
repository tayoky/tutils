#include <grid.h>
#ifdef __unix__
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

//a library to align text serously???
//i'm crazzzyyy !


//lenght of string without counting escape sequences
static size_t estrlen(const char *str){
	size_t len = 0;
	int emode = 0;
	while(*str){
		if(*str == '\033')emode = 1;
		if(!emode)len++;
		if(emode == 1 && isalpha(*(const unsigned char *)str))emode = 0;
		str++;
	}
	return len;
}

size_t *grid_calculate(char **array,size_t array_count,size_t column){
	size_t *sizes = calloc(sizeof(size_t),column);
	for(size_t i=0; i<column; i++){
		for(size_t j=i; j<array_count; j+= column){
			if(estrlen(array[j]) > sizes[i]){
				sizes[i] = estrlen(array[j]);
			}
		}
		sizes[i]++;
	}

	return sizes;
}

void grid_print(char **array,size_t array_count,size_t column){
	size_t *sizes = grid_calculate(array,array_count,column);

	for(size_t i=0; i<array_count; i++){
		printf("%s",array[i]);
		if(i % column == column-1 || i+1 == array_count){
			putchar('\n');
		} else {
			for(size_t j=estrlen(array[i]); j<sizes[i%column]; j++)putchar(' ');
		}
	}
	free(sizes);
}

void grid_print_auto(char **array,size_t array_count){
	size_t column = 1;
	size_t width = 25;
#ifdef __unix__
	struct winsize win;
	if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&win) >= 0){
		width = win.ws_col;
	}
#endif
	for(;;){
		column++;
		size_t *sizes = grid_calculate(array,array_count,column);
		size_t total = 0;
		for(size_t i=0; i<column; i++){
			total += sizes[i];
		}
		free(sizes);
		if(total > width || column > array_count)break;
	}
	column--;
	return grid_print(array,array_count,column);
}
