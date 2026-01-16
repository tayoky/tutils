#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tutils.h>

CMD_NOPT(test, "test [ARG1] OPTION [ARG2]\n"
      "test a condition\n");

//simple test command
int test(int *argc,char ***r_argv){
	char **argv = *r_argv;

	if(*argc < 1){
		return 1;
	}

	//first check for parenthese
	if(!strcmp(argv[0],"(")){
		(*argc)--;
		(*r_argv)++;
		if(test(argc,r_argv)){
			return 1;
		}
		//check close parenthese
		if(*argc < 1 || strcmp(*r_argv[0],")")){
			return 1;
		}
		(*argc)--;
		(*r_argv)++;
		return 0;
	}

	//then check for one operand test
	if(argv[0][0] == '-' && strlen(argv[0]) == 2){
		if(*argc < 2){
			return 1;
		}
		(*argc)-=2;
		(*r_argv)+=2;
		//first check if it is a string op
		switch (argv[0][1]){
		case 'z':
			//empty check
			return strlen(argv[1])>0;
		case 'n':
			//not empty check
			return strlen(argv[1])==0;
		}

		//it's a file op
		//stat it
		struct stat st;
		int exist = stat(argv[1],&st) >= 0;

		//don't exist ? fail
		if(!exist){
			return 1;
		}

		switch(argv[0][1]){
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
		case 'p':
			return !S_ISFIFO(st.st_mode);
		case 'h':
		case 'L':
			return !S_ISLNK(st.st_mode);
		case 'u':
			return !(st.st_mode & S_ISUID);
		case 'g':
			return !(st.st_mode & S_ISGID);
		case 'k':
			//TODO: sticky bit
			return 1;
		case 's':
			//TODO: not empty check
			return 1;
		case 'S':
			//TODO: socket check
			return 1;
		case 'O':
			return st.st_uid != geteuid();
		case 'G':
			return st.st_gid != getegid();
		case 'r':
		case 'w':
		case 'x':
			//TODO : perm check
			return 1;
		case 'N':
			//modify since last write
			return st.st_atime > st.st_mtime;
		default:
			//invalid option
			return 1;
		}
	}

	//check for three operand
	if(*argc < 3){
		return 1;
	}
	(*argc)-=3;
	(*r_argv)+=3;
	if(!strcmp(argv[1],"=")){
		return strcmp(argv[0],argv[2]) != 0;
	}
	if(!strcmp(argv[1],"!=")){
		return strcmp(argv[0],argv[2]) == 0;
	}

	//other operand not supported
	return 1;

}

static int test_main(int argc,char **argv){
	return test(&argc,&argv);
}	
