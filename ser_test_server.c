#include "move_manager.c"
#include "networking/server.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void client_accept_function(client_t **clients, int num_connected, int total){
	return;
}

int main(){
	server_t *server = start_server(3333);

	int num_players = 1;

	client_t **clients = wait_for_connections(server, num_players, client_accept_function);
	if(!clients){
		printf("Error getting connections.\n");
		return 0;
	}

	num_players = 3;
	move_manager_t *manager = MoveManager_Create(num_players);
	for(int i = 0; i < num_players; i++)
	{
		float angle = i * 3.14159 * 2 / num_players;
		int starting_x = (int)(30 / 2 + 30 * 3.0 / 8.0 * cos(angle));
		int starting_y = (int)(30 / 2 - 30 * 3.0 / 8.0 * sin(angle));
		int starting_dir = 4;
		if(angle < 3.14159 / 2)
			starting_dir = 3;
		else if(angle < 3.14159)
			starting_dir = 2;
		else if(angle < 3.0 / 2.0 * 3.14159)
			starting_dir = 5;
		MoveManager_SetStartingInfo(manager, i, starting_x, starting_y, starting_dir);
	}

	for(int i = 0; i < 3; i++){
		MoveManager_AddMove(manager, 0, i*10, 2);
	}

	print_manager(manager);

	char *buffer = (char*)serialize(manager);
	/*printf("sending strlen %lu\n", strlen(buffer));
	for(int i=0; i<20; i++){
		printf("%02X", (unsigned)buffer[i]);
	}
	puts("");*/

	size_t i;
	for(i = 0; i < 1; i++)
	{
		send_to_client(clients[i], buffer);
	}

	//clean up
	free(buffer);
	close_server(server);
	MoveManager_Destroy(manager);
puts("closed server");
	return 0;
}
