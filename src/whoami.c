#include <stdio.h>
#include <stdlib.h>

int main(){
	char *username = getenv("USER");
	if(username == NULL){
		fprintf(stderr,"whoami : $USER environement variable not set\n");
		return 1;
	}
	printf("%s\n",username);
	return 0;
}
