#include <stdio.h>
#include "move_manager.c"

int main(int argc, char **argv){
	if(argc != 2){
		perror("Usage: ./test <int>");
		exit(0);
	}
	int a = atoi(argv[1]);
	void *message = serialize(a);
	for(int i=0; i<20; i++){
		printf("%02X", (unsigned)((char*)message)[i]);
	}
	puts("");
	printf("message is: %s\n", (char *)message);

	return 0;
}
