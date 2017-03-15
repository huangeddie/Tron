#include <stdlib.h>
#include "terminal_graphics/color_graphics.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

typedef struct move_t
{
	int game_tick;
	int direction;
	struct move_t * next;
} pmove_t;

typedef struct move_manager_t
{
	int num_players;
	pmove_t ** player_moves;
	int * player_starting_xs;
	int * player_starting_ys;
	int * player_starting_dirs;
	pthread_mutex_t mutex;

} move_manager_t;

typedef struct location_t
{
	int x;
	int y;
} location_t;

typedef struct player_status_t
{
	location_t location;
	int dir;
} player_status_t;

move_manager_t * MoveManager_Create(int num_players)
{
	move_manager_t * manager = malloc(sizeof(move_manager_t));
	manager->num_players = num_players;
	pmove_t ** playersMoves = calloc(num_players, sizeof(pmove_t*));
	manager->player_moves = playersMoves;
	int * startingXs = calloc(num_players, sizeof(int));
	int * startingYs = calloc(num_players, sizeof(int));
	int * startingDirs = calloc(num_players, sizeof(int));
	manager->player_starting_xs = startingXs;
	manager->player_starting_ys = startingYs;
	manager->player_starting_dirs = startingDirs;
	pthread_mutex_init(&manager->mutex, 0);
	return manager;
}

void MoveManager_SetStartingInfo(move_manager_t * manager, int player, int x, int y, int dir)
{
	pthread_mutex_lock(&manager->mutex);
	manager->player_starting_xs[player] = x;
	manager->player_starting_ys[player] = y;
	manager->player_starting_dirs[player] = dir;
	pthread_mutex_unlock(&manager->mutex);
}

void MoveManager_AddMove(move_manager_t * manager, int player, int gameTick, int dir)
{
	pthread_mutex_lock(&manager->mutex);
	pmove_t * newMove = malloc(sizeof(pmove_t));
	newMove->game_tick = gameTick;
	newMove->direction = dir;

	if(manager->player_moves[player] == NULL)
	{
		manager->player_moves[player] = newMove;
		newMove->next = NULL;
		goto cleanup;
	}

	pmove_t * firstMove = manager->player_moves[player];
	if(gameTick < firstMove->game_tick)
	{
		manager->player_moves[player] = newMove;
		newMove->next = firstMove;
		goto cleanup;
	}


	pmove_t * prev = firstMove;
	pmove_t * curr = firstMove->next;

	while(curr)
	{
		if(curr->game_tick > gameTick)
		{
			newMove->next = curr;
			prev->next = newMove;
			goto cleanup;
		}

		prev = curr;
		curr = curr->next;
	}

	prev->next = newMove;
	newMove->next = NULL;

	cleanup:;
	pthread_mutex_unlock(&manager->mutex);
}

player_status_t MoveManager_PlayerStatus(move_manager_t * manager, int player, int tick)
{
	pthread_mutex_lock(&manager->mutex);
	player_status_t status;
	status.location.x = manager->player_starting_xs[player];
	status.location.y = manager->player_starting_ys[player];
	status.dir = manager->player_starting_dirs[player];
	
	pmove_t * nextMove = manager->player_moves[player];

	int currTick = 1;
	while(currTick <= tick)
	{
		if(status.dir == CGK_UP)
			status.location.y--;
		else if(status.dir == CGK_DOWN)
			status.location.y++;
		else if(status.dir == CGK_RIGHT)
			status.location.x++;
		else if(status.dir == CGK_LEFT)
			status.location.x--;

		while(nextMove && nextMove->game_tick == currTick)
		{
			status.dir = nextMove->direction;
			nextMove = nextMove->next;
		}

		currTick++;
	}
	pthread_mutex_unlock(&manager->mutex);
	return status;
}

void MoveManager_Destroy(move_manager_t * manager)
{
	for(int i = 0; i < manager->num_players; i++)
	{
		pmove_t * curr = manager->player_moves[i];
		while(curr)
		{
			pmove_t * temp = curr->next;
			free(curr);
			curr = temp;
		}
	}
	free(manager->player_starting_xs);
	free(manager->player_starting_ys);
	free(manager->player_starting_dirs);
	free(manager->player_moves);
	pthread_mutex_destroy(&manager->mutex);
	free(manager);
}

typedef struct Buffer {
	void *data;
	int next;
	size_t size;
} buffer_t;

buffer_t *new_buffer() {
	int INITIAL_SIZE = 32;
	buffer_t *b = malloc(sizeof(buffer_t));

	b->data = malloc(INITIAL_SIZE);
	b->size = INITIAL_SIZE;
	b->next = 0;

	return b;
}

void reserve_space(buffer_t *b, size_t bytes) {
	if((b->next + bytes) > b->size) {
		b->data = realloc(b->data, b->size * 2);
		b->size *= 2;
	}
}

// needs to be freed
char *itoa(int num){	
	int LEN = (int)((ceil(log10(num))+1)*sizeof(char));
	if(!num) LEN = 1;
	char *str = malloc(sizeof(char) * LEN + 1);
	str[LEN] = '\0';
	sprintf(str, "%d", num);

	return str;
}

void serialize_int(buffer_t *b, int num){
	char *num_str = itoa(num);
	char *num_len = itoa(strlen(num_str));

	// +4 for "INT_"
	reserve_space(b, strlen(num_str) + strlen(num_len) + 4);
	memcpy(b->data + b->next, "INT", 3);
	b->next += 3;
	memcpy(b->data + b->next, num_len, strlen(num_len));
	b->next += strlen(num_len);
	memcpy(b->data + b->next, "_", 1);
	b->next += 1;
	memcpy(b->data + b->next, num_str, strlen(num_str));
	b->next += strlen(num_str);

	free(num_str);
	free(num_len);
}

int deserialize_int(char **num){
	assert(!strncmp(*num, "INT", 3));
	char *replace;
	assert(replace = strchr(*num, '_'));
	*replace = '\0';
	int num_len = atoi(&(*num)[3]);

	char *num_str = strndup(replace+1, num_len);
	int ret_num = atoi(num_str);
	free(num_str);

	*num = replace + 1 + num_len;

	return ret_num;
}

void serialize_pmove(buffer_t *b, pmove_t *pm){
	while(pm){
		reserve_space(b, 5);
		memcpy(b->data + b->next, "PMOVE", 5);
		b->next += 5;
		serialize_int(b, pm->game_tick);
		serialize_int(b, pm->direction);
		pm = pm->next;
	}
	reserve_space(b, 9);
	memcpy(b->data + b->next, "END_PMOVE", 9);
	b->next += 9;
}

pmove_t *deserialize_pmove(char **num){
	pmove_t *head = NULL;
	pmove_t *iter = NULL;
	while(strncmp(*num, "END_PMOVE", 9)){
		assert(!strncmp(*num, "PMOVE", 5));
		*num += 5;
		pmove_t *temp = malloc(sizeof(pmove_t));
		temp->game_tick = deserialize_int(num);
		temp->direction = deserialize_int(num);
		temp->next = NULL;
		if(!head){
			head = temp;
			iter = temp;
		}
		iter->next = temp;
		iter = temp;
	}
	*num += 9;
	return head;
}

//Serializes a move_manager struct to be able to send over the network.
//returned buffer must be freed by user
void *serialize(move_manager_t *manager){
	buffer_t *buffer = new_buffer();
	serialize_int(buffer, manager->num_players);

	int iter;
	for(iter = 0; iter < manager->num_players; iter++){
		serialize_pmove(buffer, manager->player_moves[iter]);

		serialize_int(buffer, manager->player_starting_xs[iter]);
		serialize_int(buffer, manager->player_starting_ys[iter]);
		serialize_int(buffer, manager->player_starting_dirs[iter]);
	}

	// marking the end of the message to be sent over the network
	reserve_space(buffer, 1);
	memcpy(buffer->data + buffer->next, "\0", 1);

	void *ret = buffer->data;
	free(buffer);
	return ret;
}

//Deserializes the received message into a struct.
//returned move_manager_t must be freed by user
move_manager_t *deserialize(void *buffer){
	char **buf_ptr = (char**)&buffer;
	int num_players = deserialize_int(buf_ptr);
	move_manager_t *manager = MoveManager_Create(num_players);
	int iter;
	for(iter = 0; iter < num_players; iter++){
		manager->player_moves[iter] = deserialize_pmove(buf_ptr);

		manager->player_starting_xs[iter] = deserialize_int(buf_ptr);
		manager->player_starting_ys[iter] = deserialize_int(buf_ptr);
		manager->player_starting_dirs[iter] = deserialize_int(buf_ptr);
	}

	return manager;
}
