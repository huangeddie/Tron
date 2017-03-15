#include "terminal_graphics/color_graphics.h"
#include "networking/client.h"
#include "constants.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#define FPS_WAIT (1000/fps)

#define MODE_NONE 0
#define MODE_TERMINAL 1
#define MODE_TRON 2
#define MODE_EXIT 3
#define MODE_PRETRON 4

int currentMode = MODE_NONE;

int server_fd = -1;

int board_width = 1;
int board_height = 1;
int fps = 1;

int game_counter;
int game_start_time;

COLOR * game_board;

int debug_mode = 0;
int graphics_open = 0;
int exit_debug_loop = 0;

void getCTRLC(int dummy)
{
//	if(server_fd > 0)
//		send_to_server(server_fd, CLIENT_WILL_EXIT);
//	if(graphics_open)
//		ColorGraphics_End();
//	exit_debug_loop = 1;

	currentMode = MODE_EXIT;
}

int getCurrentGameTick()
{
	return (ColorGraphics_GetTimeMS() - game_start_time) / FPS_WAIT;
}

void processKey(int keycode)
{
	if(currentMode != MODE_TRON)
		return;

	int tickno = getCurrentGameTick();

	if(!IS_ARROW_KEY(keycode))
		return; 

	if(server_fd <= 0) return;

	char * message= malloc(50);
	sprintf(message, "%d %d", tickno, keycode);
	send_to_server(server_fd, message);
	if(debug_mode)
	{
		printf("(debug) String Sent: '%s'\n", message);
	}
	free(message);
}

// This is the graphics routine that should repaint the screen 
void gameLoop()
{
	char * message = listen_to_server(server_fd);

	if(!message)
		return;

	if(!strcmp(message, SMODE_TERMINAL))
        {
               // Enter terminal mode
               currentMode = MODE_TERMINAL;
        }
        else if(!strcmp(message, SMODE_PRETRON))
        {
               // Enter pretron mode
               currentMode = MODE_PRETRON;
        }
        else if(!strcmp(message, SMODE_EXIT))
        {
               // Enter exit mode (so, exit)
               currentMode = MODE_EXIT;
        }

	if(currentMode != MODE_TRON)
	{
		ColorGraphics_End();
		return;
	}

    assert(message);

	// Take 'message' from the server and copy it to our 'game_board'
    int c = 0;
    while(message[c])
    {
        game_board[c] = message[c];
        c++;
    }
	// Then, draw 'game_board' on the screen using ColorGraphics
	
	for(int x = 0; x < board_width; x++)
	{
		for(int y = 0; y < board_height; y++)
		{
			ColorGraphics_SetPixel(x, y, game_board[y * board_width + x]);
		}
	}
}

void printUsage()
{
	printf("TRON CLIENT USAGE:\n\t-ip <server-ip-address>\n\t-port <server-port-number>\n\t-debug (sets debug logging, no graphics)\n");
}

int main(int argc, char** argv)
{

	//char * ip = "172.22.155.23";
	//char * port = "8850";

	char * ip = "127.0.0.1";
	char * port = "1234";

	int i;
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i],"-ip"))
		{
			i++;
			ip = argv[i];
		}
		else if(!strcmp(argv[i],"-port"))
		{
			i++;
			port = argv[i];
		}
		else if(!strcmp(argv[i],"-debug"))
		{
			debug_mode = 1;
		}
		else
		{
			printUsage();
			return 0;
		}
	}

	signal(SIGINT, getCTRLC);

	printf("Connecting to server at IP '%s', port '%s'...\n", ip, port);

	server_fd = connect_to_server(ip, port);	
	
	if(server_fd <= 0)
	{
		printf("Failed to connect to server.\n");
        perror("connect_to_server()");
		exit(0);
	}

	printf("Connected To Server!\n");
    
	while(currentMode != MODE_EXIT)
	{
		char * message = NULL;
		do
		{
			message = listen_to_server(server_fd);
		}
		while(!message);

		// Handle mode switch commands
		if(!strcmp(message, SMODE_TERMINAL))
		{
			// Enter terminal mode
			currentMode = MODE_TERMINAL;
		}
		else if(!strcmp(message, SMODE_PRETRON))
		{
			// Enter pretron mode
			currentMode = MODE_PRETRON;
        }
        else if(!strcmp(message, SMODE_TRON))
        {
            // Enter tron mode
            memset(game_board, 0, board_width * board_height + 1);
            currentMode = MODE_TRON;
            game_start_time = ColorGraphics_GetTimeMS();
            ColorGraphics_Init(board_width, board_height);
            ColorGraphics_SetKeyFunction(processKey);
            ColorGraphics_SetGameLoopFunction(gameLoop, 0);
            ColorGraphics_SetBackgroundColor(WHITE);
            
            ColorGraphics_Start();
        }
        else if(!strcmp(message, SMODE_EXIT))
		{
			// Enter exit mode (so, exit)
			currentMode = MODE_EXIT;	
		}

		// If it wasn't a switch message

		else if(currentMode == MODE_TERMINAL)
		{
			printf("%s", message);
		}
		else if(currentMode == MODE_PRETRON)
		{
			sscanf(message, "%d %d %d", &board_width, &board_height, &fps);
            game_board = malloc(board_width * board_height + 1); //initialize board
		}
	}
    
    free(game_board);

	close_client(server_fd);	
}
