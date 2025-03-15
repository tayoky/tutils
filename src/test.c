#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//simple test command

int main(int argc,char **argv){
	if(argc < 3){
		//not engouth args
		return 1;
	}
	if(argc > 4){
		//too many args
		return 1;
	}

	//first check for one operand test
	if(argc == 3){
		//check for a one char option
		if(argv[1][0] != '-' || strlen(argv[1]) != 2){
			return 1;
		}
		//first check if it is a string op
		switch (argv[1][1]){
		case 'z':
			//empty check
			return strlen(argv[2])>0;
		case 'n':
			//not empty check
			return strlen(argv[2])==0;
		}

		//it's a file op
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
		case 'O':
			return st.st_uid != geteuid();
		case 'G':
			return st.st_gid != getegid();
		default:
			//invalid option
			return 1;
		}
	}

	//check for three operand
	if(!strcmp(argv[2],"=")){
		return strcmp(argv[1],argv[2]) != 0;
	}
	if(!strcmp(argv[2],"!=")){
		return strcmp(argv[1],argv[2]) == 0;
	}

	//other operand not supported
	return 1;
}
			
