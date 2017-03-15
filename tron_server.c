#include "terminal_graphics/color_graphics.h"
#include "move_manager.c"
#include "networking/server.h"
#include "constants.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

// Self explanatory.
int BOARD_WIDTH = 30;
int BOARD_HEIGHT = 30;
int FPS = 2;

#define FPS_WAIT (1000/FPS)

// Number of players (clients) in the game
int num_players;

// The system time (in MS) that the game started
int game_start_time;

// The MoveManager for managing Tron player locations
move_manager_t * move_manager = NULL;

// Array mapping an integer (team number) to a color (COLOR). eg: colorAssignments[0] is the COLOR of player 0
COLOR colorAssignments[] = {BLUE, YELLOW, RED, GREEN};

// Returns the current 'tick' of the game.
int getCurrentGameTick()
{
	return (ColorGraphics_GetTimeMS() - game_start_time) / FPS_WAIT;
}

// Returns the readable string of a COLOR. Eg: BLUE => "blue", etc...
char * stringFromColor(COLOR c)
{
	if(c == BLUE) return BOLD BLUE_TXT "blue" NO_COLOR_TXT;
	if(c == YELLOW) return BOLD YELLOW_TXT "yellow" NO_COLOR_TXT;
	if(c == GREEN) return BOLD GREEN_TXT "green" NO_COLOR_TXT;
	if(c == WHITE) return BOLD WHITE_TXT "white" NO_COLOR_TXT;
	if(c == RED) return BOLD RED_TXT "red" NO_COLOR_TXT;
	return "unknown";
}

// Prints the usage of the program, with command line flags explained
void print_usage()
{
	printf("TRON SERVER USAGE:\n\t-port <port-num-to-use>\n\t-players <number-of-players> (>2)\n\t-width <board-width> (>20)\n\t-height <board-height> (>20)\n\t-fps <frames-per-second> (>0)\n");
}

// Function called when a client is added.
// Puts the client in terminal mode, then sends the initial game starting printouts.
void client_accept_function(client_t ** clients, int num_connected, int total)
{
	printf("Client %d Connected.\n", num_connected);
	client_t * most_recent_client = clients[num_connected - 1];
	int clients_remaining = total - num_connected;

	send_to_client(most_recent_client, SMODE_TERMINAL);

	char message[256];
	sprintf(message, "--------------------\n|   WELCOME TO     |\n|      TRON        |\n--------------------\n\n");
	send_to_client(most_recent_client, message);

	sprintf(message, "You are player %d, the %s snake.\n", num_connected, stringFromColor(colorAssignments[num_connected-1]));
	send_to_client(most_recent_client, message);

	if(clients_remaining != 0)
	{
		sprintf(message, "We are waiting for %d more player(s). Please wait.\n", (total - num_connected));
		send_to_client(most_recent_client, message);
	}

	
	for(int i = 0; i < num_connected; i++)
	{
		printf("\tSending update message to client %d.\n", i);
		sprintf(message, "Player %d has joined!\n", num_connected);
		send_to_client(clients[i], message);
	}
}

int main(int argc, char** argv)
{
	// First, process all the command line arguments
	
	// Default port number and players
	int port_number = 1234;
	int num_players = 2;

	int i;
	for(i = 1; i < argc; i++)
	{
		if(!strcmp("-port",argv[i]))
		{
			i++;
			int res = sscanf(argv[i], "%d", &port_number);
			if(res != 1)
			{
				print_usage();
				return 0;
			}
		}
		else if(!strcmp("-players", argv[i]))
		{
			i++;
			int res = sscanf(argv[i],"%d", &num_players);
			if(res != 1 || num_players < 1 || num_players > 4)
			{
				print_usage();
				return 0;
			}
		}
		else if(!strcmp("-width", argv[i]))
                {
                        i++;
                        int res = sscanf(argv[i],"%d", &BOARD_WIDTH);
                        if(res != 1 || BOARD_WIDTH < 20)
                        {
                                print_usage();
                                return 0;
                        }
                }
		else if(!strcmp("-height", argv[i]))
                {
                        i++;
                        int res = sscanf(argv[i],"%d", &BOARD_HEIGHT);
                        if(res != 1 || BOARD_HEIGHT < 20)
                        {
                                print_usage();
                                return 0;
                        }
                }
		else if(!strcmp("-fps", argv[i]))
                {
                        i++;
                        int res = sscanf(argv[i],"%d", &FPS);
                        if(res != 1 || FPS < 1)
                        {
                                print_usage();
                                return 0;
                        }
                }
        else if(!strcmp("-fullscreen", argv[i]))
                {
                        BOARD_HEIGHT = 54;
                        BOARD_WIDTH = 90;
                }
        else if(!strcmp("-huge", argv[i]))
        {
            BOARD_HEIGHT = 107;
            BOARD_WIDTH = 178;
        }
		else
		{
			print_usage();
			return 0;
		}
	}

	// Start the server

	server_t * server = start_server(port_number);
	if(!server)
	{
		printf("Error starting Tron Server.\n");
		return 0;
	}

	printf("Tron Server started on port %d.\n", port_number);

	printf("This Tron Server can be joined at the following IP Addresses:\n");
	char ** ips = get_local_ip_addresses();
	i = 0;
	while(ips[i])
	{
		printf("\t%s\n", ips[i]);
		i++;
	}
	free_local_ip_addresses(ips);

	// Wait for 'num_player' clients to connect to the server

	printf("Waiting For %d Clients to join...\n", num_players);

	client_t ** clients = wait_for_connections(server, num_players, client_accept_function);
	
	if(!clients)
	{
		printf("Error getting connections.\n");
		return 0;
	}

	printf("%d connections made!\n", num_players);

	// Message the clients that everyone has joined

	for(i = 0; i < num_players; i++)
	{
		send_to_client(clients[i], "\nEveryone has joined! Please wait...\n");
	}

	// Set up the games internal bookeeping

	printf("Setting up internal game book keeping.... ");

	move_manager = MoveManager_Create(num_players);
	// Set the starting location of all the players using unit-circle trig stuff
	for(int i = 0; i < num_players; i++)
	{
		float angle = i * 3.14159 * 2 / num_players;
		int starting_x = (int)(BOARD_WIDTH / 2 + BOARD_WIDTH * 3.0 / 8.0 * cos(angle));
		int starting_y = (int)(BOARD_HEIGHT / 2 - BOARD_HEIGHT * 3.0 / 8.0 * sin(angle));
		int starting_dir = CGK_UP;
		if(angle < 3.14159 / 2)
			starting_dir = CGK_LEFT;
		else if(angle < 3.14159)
			starting_dir = CGK_DOWN;
		else if(angle < 3.0 / 2.0 * 3.14159)
			starting_dir = CGK_RIGHT;
		MoveManager_SetStartingInfo(move_manager, i, starting_x, starting_y, starting_dir);
	}

	printf("Done!\n");
	
        // Send the clients their game-critical information. (board width, frames per second, client's team number, etc)

	printf("Sending clients starting info... ");

	for(i = 0; i < num_players; i++)
	{
		send_to_client(clients[i], SMODE_PRETRON);
	}

        char message[128];
        for(i = 0; i < num_players; i++)
        {
                sprintf(message, "%d %d %d %d %d", BOARD_WIDTH, BOARD_HEIGHT, FPS, i, colorAssignments[i]);
                send_to_client(clients[i], message);
        }

	printf("Done!\n");

	printf("Starting game in...\n");

	// Do the "3... 2.... 1...." countdown for the client

	int j = 0;
	for(j = 0; j < num_players; j++)
	{
		send_to_client(clients[j], SMODE_TERMINAL);
		sprintf(message, "The game will start in...\n");
		send_to_client(clients[j], message);
	}

	for(i = 3; i > 0; i--)
	{
		sprintf(message, "%d\n", i);
		for(j = 0; j < num_players; j++)
		{
			send_to_client(clients[j], message);
		}
		printf("%s", message);
		sleep(1);
	}

	for(i = 0; i < num_players; i++)
	{
		send_to_client(clients[i], SMODE_TRON);
	}

	printf("Game has started.\n");

	int num_players_alive = num_players;
	int game_winner = -1;
	int currTick = -1;
    
        char *graphics = malloc(BOARD_WIDTH * BOARD_HEIGHT + 1);
        int *player_lost = malloc(sizeof(int) * num_players);
    
	while(num_players_alive > 1)
	{
		int none_connected = 1;
		for(i = 0; i < num_players; i++)
		{
			if(clients[i]->disconnected) continue;

			none_connected = 0;

			const char * message = listen_to_client(clients[i]);
			if(message)
			{
				// If there's a message from the ith client...

				if(!strcmp(CLIENT_WILL_EXIT, message))
				{
					clients[i]->disconnected = 1;
					printf("Client %d has disconnected.\n", i);
				}
				else
				{
					// Read the move message from the client.

					int tickno;
					int dir;
					sscanf(message, "%d %d", &tickno, &dir);

					player_status_t status = MoveManager_PlayerStatus(move_manager, i, currTick);
					int cdir = 0;
					// Don't allow up key when moving down, etc
					if(	(status.dir == CGK_LEFT && dir == CGK_RIGHT) ||
						(status.dir == CGK_RIGHT && dir == CGK_LEFT) ||
						(status.dir == CGK_DOWN && dir == CGK_UP) ||
						(status.dir == CGK_UP && dir == CGK_DOWN))
						cdir = 1;
					if(!cdir)
					{			
						MoveManager_AddMove(move_manager, i, currTick, dir);
	                                        printf("Got Move: (player %d) tick = %d, dir = %d\n", i, /*tickno*/currTick, dir);
					}
				}
			}
		}

		if(none_connected)
		{
			printf("All clients have disconnected.\n");
			break;
		}

		if(currTick != getCurrentGameTick())
		{
			// If currTick is not what the timer says the tick should be, a tick has passed and we need to regenerate the graphics

			currTick++;
			printf("%d\n", currTick);
            		memset(graphics, 0, BOARD_WIDTH * BOARD_WIDTH + 1);
			int i;
			for(i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++)
				graphics[i] = NONE;
			int player;
            		memset(player_lost, 0, sizeof(int) * num_players);

			// For each tick, for each non-lost player, draw that players color at that tick on 'graphics'
			for(i = 0; i <= currTick; i++)
			{
				for(player = 0; player < num_players; player++)
				{
					if(!player_lost[player])
					{
						if(clients[player]->disconnected)
						{
							printf("Player %d has disconnected, so is now dead.\n", player);
							player_lost[player] = 1;
							continue;
						}

						player_status_t status = MoveManager_PlayerStatus(move_manager, player, i);
						int gloc = status.location.x + BOARD_WIDTH * status.location.y;
						if(status.location.x < 0 || status.location.y < 0 ||
							status.location.x >= BOARD_WIDTH || status.location.y >= BOARD_HEIGHT ||
							graphics[gloc] != NONE)
						{
							player_lost[player] = 1;
							continue;
						}
						
						graphics[gloc] = colorAssignments[player];
					}
				}
			}
			
			// Send the drawn graphics board to each client
			for(i = 0; i < num_players; i++)
			{
				if(!clients[i]->disconnected)
					send_to_client(clients[i], graphics);
			}

			num_players_alive = 0;
			for(i = 0; i < num_players; i++)
			{
				if(!player_lost[i])
				{
					game_winner = i;
					num_players_alive++;
				}
			}
		}
	}
    
    free(player_lost);
    free(graphics);

	printf("Player %d wins!\n", game_winner);

	sleep(2);

	printf("Sending gameover message to clients... ");

	for(i = 0; i < num_players; i++)
	{
		send_to_client(clients[i], SMODE_TERMINAL);
		if(i == game_winner)
		{
			send_to_client(clients[i], "\nYou won! Thank you for playing.\n");
		}
		else
			send_to_client(clients[i], "\nThe game is over, and you lost. Thank you for playing!\n");
	}

	for(i = 0; i < num_players; i++)
	{
		send_to_client(clients[i], SMODE_EXIT);
	}

	printf("Done!\nGame is over. Exiting Now.\n");

	MoveManager_Destroy(move_manager);
	remove_connections(clients);
	close_server(server);
}
