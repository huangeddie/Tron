#include "networking/client.h"
#include "move_manager.c"

#include <stdio.h>

void print_manager(move_manager_t *manager){
	printf("num_players: %d\n", manager->num_players);
	for (int iter = 0; iter < manager->num_players; iter++){
		printf("player %d\n", iter);

		pmove_t *move_iter = manager->player_moves[iter];
		while(move_iter){
			printf("game_tick: %d, direction: %d\n", move_iter->game_tick, move_iter->direction);
			move_iter = move_iter->next;
		}

		printf("starting_x: %d\n", manager->player_starting_xs[iter]);
		printf("starting_y: %d\n", manager->player_starting_ys[iter]);
		printf("starting_dir: %d\n", manager->player_starting_dirs[iter]);
	}
	puts("");
}

int main(){
	int server_fd = connect_to_server("127.0.0.1", "3333");
		
	if(server_fd <= 0)
	{
		printf("Failed to connect to server.\n");
        perror("connect_to_server()");
		exit(0);
	}

	printf("Connected To Server!\n");

	char * message = NULL;
	do
	{
		message = listen_to_server(server_fd);
	}
	while(!message);

	/*printf("received strlen %lu\n", strlen(message));
	for(int i=0; i<20; i++){
		printf("%02X", (unsigned)message[i]);
	}*/
	move_manager_t *manager = deserialize((void*)message);
	//printf("messege received: %s", message);
	print_manager(manager);
	MoveManager_Destroy(manager);

	close_client(server_fd);

	return 0;
}
