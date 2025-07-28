#include "stdopt.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>

#define ESC "\033"
#define FLAG_ALMOST  0x08
#define FLAG_ALL     0x10
#define FLAG_NO_SORT 0x20
#define FLAG_LIST    0x40

struct opt opts[] = {
	OPT('a',"--all",FLAG_ALL | FLAG_ALMOST,"show all files (including . and ..)"),
	OPT('A',"--almost-all",FLAG_ALMOST,"show all files except . and .."),
	OPT('U',NULL,FLAG_NO_SORT,"show in directory order without sorting"),
	OPT('l',"--list",FLAG_LIST,"show one file/directory per line"),
};

const char *usage = "ls [-laAU] [DIRECTORY]\n"
"list files in a directory\n";


int to_tty = 0;
int ret;

struct ent {
	char *name;
	char *path;
	struct stat meta;
	int unknow;
};

void print_ent(struct ent *entry){
	//show mode if needed
	if(flags & FLAG_LIST){
		if(entry->unknow){
			putchar('?');
		} else if(S_ISLNK(entry->meta.st_mode)){
			putchar('l');
		} else if(S_ISDIR(entry->meta.st_mode)){
			putchar('d');
		} else if(S_ISCHR(entry->meta.st_mode)){
			putchar('c');
		} else if(S_ISBLK(entry->meta.st_mode)){
			putchar('b');
		} else if(S_ISFIFO(entry->meta.st_mode)){
			putchar('p');
		} else {
			putchar('-');
		}
		char mode[10];
		memset(mode,entry->unknow ? '?' : '-',sizeof(mode));
		mode[sizeof(mode)-1] = '\0';
		if(entry->meta.st_mode & S_IRUSR)mode[0] = 'r';
		if(entry->meta.st_mode & S_IWUSR)mode[1] = 'w';
		if(entry->meta.st_mode & S_IXUSR)mode[2] = 'x';
		if(entry->meta.st_mode & S_IRGRP)mode[3] = 'r';
		if(entry->meta.st_mode & S_IWGRP)mode[4] = 'w';
		if(entry->meta.st_mode & S_IXGRP)mode[5] = 'x';
		if(entry->meta.st_mode & S_IROTH)mode[6] = 'r';
		if(entry->meta.st_mode & S_IWOTH)mode[7] = 'w';
		if(entry->meta.st_mode & S_IXOTH)mode[8] = 'x';

		if(entry->meta.st_mode & S_ISUID)mode[2] = entry->meta.st_mode & S_IXUSR ? 's' : 'S';
		if(entry->meta.st_mode & S_ISGID)mode[5] = entry->meta.st_mode & S_IXGRP ? 's' : 'S';
		printf("%s ",mode);

		//owner name
		struct passwd *pwd = getpwuid(entry->meta.st_uid);
		if(pwd){
			printf("%s ",pwd->pw_name);
		} else {
#ifdef __stanix__
			printf("%ld ",entry->meta.st_uid);
#else
			printf("%d ",entry->meta.st_uid);
#endif
		}
	
		printf("%4ld ",entry->meta.st_size);
	}
	
	//color only for tty
	if(to_tty){
		if(S_ISLNK(entry->meta.st_mode)){
			printf(ESC"[1;36m");
		} else if(S_ISCHR(entry->meta.st_mode)){
			printf(ESC"[1;33m");
		} else if(S_ISDIR(entry->meta.st_mode)){
			printf(ESC"[1;34m");
		} else if(entry->meta.st_mode & S_IXUSR){
			printf(ESC"[1;32m");
		}
	}
	printf("%s",entry->name);
	if(to_tty)printf(ESC"[0m");
}

int ent_sort(const void *e1,const void *e2){
	const struct ent *ent1 = e1;
	const struct ent *ent2 = e2;
	int ret = strcmp(ent1->name,ent2->name);
	return ret;
}

void mkent(struct ent *entry,const char *name,const char *path){
	memset(entry,0,sizeof(struct ent));
	entry->name = strdup(name);
	entry->path = strdup(path);
	if(lstat(path,&entry->meta) < 0){
		perror(path);
		entry->unknow = 1;
	}
}

void ls(const char *path){
	DIR *dir = opendir(path);

	if(dir == NULL){
		iprintf("%s : %s\n",path,strerror(errno));
		ret = 1;
		return ;
	}

	struct ent* entries = malloc(0);
	size_t entries_count = 0;
	struct dirent *entry;

	for(;;){
		entry = readdir(dir);
		if(!entry)break;

		char *name = entry->d_name;

		//check for hidden entry
		if(!(flags & FLAG_ALMOST) && name[0] == '.'){
			continue;
		}

		//check for . and ..
		if(!(flags & FLAG_ALL)){
			if(!strcmp(name,".")){
				continue;
			}
			if(!strcmp(name,"..")){
				continue;
			}	
		}

		char *p = malloc(strlen(name) + strlen(path) + 2);
		if(path[strlen(path)-1] == '/'){
			sprintf(p,"%s%s",path,name);
		} else {
			sprintf(p,"%s/%s",path,name);
		}

		entries = realloc(entries,(++entries_count) * sizeof(struct ent));
		mkent(&entries[entries_count-1],name,p);
		free(p);
	}

#ifndef NO_QSORT
	//sort time ?
	if(!(flags & FLAG_NO_SORT)){
		qsort(entries,entries_count,sizeof(struct ent),ent_sort);
	}
#endif

	//find cell_size and cell per line
	size_t cell_size = 1;
	size_t cell_per_line = 1;
	if(to_tty && !(flags & FLAG_LIST)){
		for(size_t i=0; i<entries_count; i++){
			if(strlen(entries[i].name) + 1 > cell_size){
				cell_size = strlen(entries[i].name) + 1;
			}
		}
		struct winsize win;
		if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&win) < 0){
			perror("ioctl");
		} else {
			cell_per_line = win.ws_col / cell_size;
		}
	}


	//print time !!
	for(size_t i=0; i<entries_count; i++){
		print_ent(&entries[i]);
		for(size_t j=strlen(entries[i].name); j<cell_size; j++)putchar(' ');
		if(i % cell_per_line == cell_per_line - 1 || i + 1 == entries_count)putchar('\n');
	}

	closedir(dir);
}


int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));	

	//are we wrinting to a tty ?
	to_tty = isatty(STDOUT_FILENO);
	if(to_tty < 0){
		to_tty = 0;
	}

	ret = 0;

	if(i >= argc){
		ls(".");
	} else {
		int header = i + 1 < argc;
		for(;i<argc;i++){
			if(header){
				printf("%s:\n",argv[i]);
			}
			ls(argv[i]);
			if(header && i + 1 < argc){
				putchar('\n');
			}
		}
	}
	return ret;
}
