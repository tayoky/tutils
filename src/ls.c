#include "stdopt.h"
#include "grid.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>

#define ESC "\033"
#define FLAG_REVERSE 0x01
#define FLAG_SORT_SZ 0x02
#define FLAG_SORT_TM 0x04
#define FLAG_ALMOST  0x08
#define FLAG_ALL     0x10
#define FLAG_NO_SORT 0x20
#define FLAG_LIST    0x40
#define FLAG_CTIME   0x80
#define FLAG_ATIME   0x100

struct opt opts[] = {
	OPT('a',"--all",FLAG_ALL | FLAG_ALMOST,"show all files (including . and ..)"),
	OPT('A',"--almost-all",FLAG_ALMOST,"show all files except . and .."),
	OPT('U',NULL,FLAG_NO_SORT,"show in directory order without sorting"),
	OPT('l',"--list",FLAG_LIST,"show more information about files in a list mode"),
	OPT('r',"--reverse",FLAG_REVERSE,"reverse order while sorting"),
	OPT('S',NULL,FLAG_SORT_SZ,"sort by file size, biggest fist"),
	OPT('t',NULL,FLAG_SORT_TM,"sort by time, newest fist"),
	OPT('c',NULL,FLAG_CTIME,"use change time instead of modify time and automaticly short by time if -l is not specified"),
	OPT('u',NULL,FLAG_ATIME,"use acces time instead of modify time and automaticly short by time if -l is not specified"),
};

const char *usage = "ls [-laAUrStuc] [DIRECTORY]\n"
"list files in a directory\n";


int to_tty = 0;
int ret;

struct ent {
	char *name;
	char *path;
	char *owner;
	char *group;
	struct stat meta;
	int unknow;
	time_t time;
};

void print_entry(struct ent *entry,char **array){
	//show info if needed
	if(flags & FLAG_LIST){
		char mode[11];
		memset(mode,entry->unknow ? '?' : '-',sizeof(mode));
		mode[sizeof(mode)-1] = '\0';
		if(entry->unknow){
			mode[0] = '?';
		} else if(S_ISLNK(entry->meta.st_mode)){
			mode[0] = 'l';
		} else if(S_ISDIR(entry->meta.st_mode)){
			mode[0] = 'd';
		} else if(S_ISCHR(entry->meta.st_mode)){
			mode[0] = 'c';
		} else if(S_ISBLK(entry->meta.st_mode)){
			mode[0] = 'b';
		} else if(S_ISFIFO(entry->meta.st_mode)){
			mode[0] = 'p';
		} else {
			mode[0] = '-';
		}
		if(entry->meta.st_mode & S_IRUSR)mode[1] = 'r';
		if(entry->meta.st_mode & S_IWUSR)mode[2] = 'w';
		if(entry->meta.st_mode & S_IXUSR)mode[3] = 'x';
		if(entry->meta.st_mode & S_IRGRP)mode[4] = 'r';
		if(entry->meta.st_mode & S_IWGRP)mode[5] = 'w';
		if(entry->meta.st_mode & S_IXGRP)mode[6] = 'x';
		if(entry->meta.st_mode & S_IROTH)mode[7] = 'r';
		if(entry->meta.st_mode & S_IWOTH)mode[8] = 'w';
		if(entry->meta.st_mode & S_IXOTH)mode[9] = 'x';

		if(entry->meta.st_mode & S_ISUID)mode[3] = entry->meta.st_mode & S_IXUSR ? 's' : 'S';
		if(entry->meta.st_mode & S_ISGID)mode[6] = entry->meta.st_mode & S_IXGRP ? 's' : 'S';
		*(array++) = strdup(mode);

		*(array++) = strdup(entry->owner);
		*(array)   = malloc(30);
		sprintf(*(array++),"%ld",entry->meta.st_size);
	}
	
	//color only for tty
	char *name;
	if(to_tty){
		name = malloc(strlen(entry->name) + 20);
		if(S_ISLNK(entry->meta.st_mode)){
			strcpy(name,ESC"[1;36m");
		} else if(S_ISCHR(entry->meta.st_mode) || S_ISBLK(entry->meta.st_mode)){
			strcpy(name,ESC"[1;33m");
		} else if(S_ISDIR(entry->meta.st_mode)){
			strcpy(name,ESC"[1;34m");
		} else if(entry->meta.st_mode & S_IXUSR){
			strcpy(name,ESC"[1;32m");
		} else {
			strcpy(name,"");
		}
	} else {
		name = malloc(strlen(entry->name) + 1);
		name[0] = '\0';
	}
	strcat(name,entry->name);
	if(to_tty)strcat(name,ESC"[0m");
	*(array) = name;
}


void print_entries(struct ent *entry,size_t count){
	size_t per_ent = flags & FLAG_LIST ? 4 : 1;
	size_t array_count = per_ent * count;
	char **array = calloc(array_count,sizeof(char *));
	for(size_t i=0; i<count; i++){
		print_entry(&entry[i],&array[i*per_ent]);
	}
	if(per_ent == 1 && to_tty){
		grid_print_auto(array,array_count);
	} else {
		grid_print(array,array_count,per_ent);
	}
	for(size_t i=0; i<array_count; i++){
		free(array[i]);
	}
	free(array);
}
	
int ent_sort(const void *e1,const void *e2){
	const struct ent *ent1 = e1;
	const struct ent *ent2 = e2;
	int ret;
	if(flags & FLAG_SORT_SZ){
		if(ent1->meta.st_size > ent2->meta.st_size)ret = -1;
		else if(ent1->meta.st_size < ent2->meta.st_size)ret = 1;
		else ret = 0;
	} else if(flags & FLAG_SORT_TM){
		if(ent1->time > ent2->time)ret = -1;
		else if(ent1->time < ent2->time)ret = 1;
		else ret = 0;
	} else {
		ret = strcmp(ent1->name,ent2->name);
	}
	if(flags & FLAG_REVERSE){
		ret = -ret;
	}
	return ret;
}

void mkent(struct ent *entry,const char *name,const char *path){
	memset(entry,0,sizeof(struct ent));
	entry->name = strdup(name);
	entry->path = strdup(path);
	if(lstat(path,&entry->meta) < 0){
		perror(path);
		entry->owner = strdup("?");
		entry->unknow = 1;
		return;
	}
	if(flags & FLAG_CTIME){
		entry->time = entry->meta.st_ctime;
	} else if(flags & FLAG_ATIME){
		entry->time = entry->meta.st_atime;
	} else {
		entry->time = entry->meta.st_mtime;
	}
	//owner name
	struct passwd *pwd = getpwuid(entry->meta.st_uid);
	if(pwd){
		entry->owner = strdup(pwd->pw_name);
	} else {
		entry->owner = malloc(32);
#ifdef __stanix__
		sprintf(entry->owner,"%ld",entry->meta.st_uid);
#else
		sprintf(entry->owner,"%d ",entry->meta.st_uid);
#endif
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

	//print time !!
	print_entries(entries,entries_count);

	closedir(dir);
}


int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));

	if((flags & FLAG_CTIME) || (flags & FLAG_ATIME) && !(flags & FLAG_LIST)){
		flags |= FLAG_SORT_TM;
	}

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
