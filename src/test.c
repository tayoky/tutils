#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

//simple test command

int main(int argc,char **argv){
	if(argc < 3){
		//not engouth args
		return 1;
	}

	//first check for one operand test
	if(argc == 3){
		//check for a one char option
		if(argv[1][0] != '-' || strlen(argv[1]) != 2){
			return 1;
		}
		//stat it
		struct stat st;
		int exist = stat(argv[2],&st) >= 0;

		//don't exist ? fail
		if(!exist){
			return 1;
		}

		switch(argv[1][1]){
		case 'a':
		case 'e':
			//aready check if exist
			return 0;
		case 'b':
			return !S_ISBLK(st.st_mode);
		case 'c':
			return !S_ISCHR(st.st_mode);
		case 'd':
			return !S_ISDIR(st.st_mode);
		case 'f':
			return !S_ISREG(st.st_mode);
		case 'h':
		case 'L':
			return !S_ISLNK(st.st_mode);

		default:
			//invalid option
			return 1;
		}
	}
	//string or aritgmetic 
	//not supported
	return 1;
}
			
