#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "color_graphics.h"
#include "queue.c"

// 'boolean' key, if 1 the game loop will exit, terminating curses and restoring the terminal
int exit_at_next_frame = 0;

// the function that is called each time a key is pressed
void (*key_function)(int key) = NULL;

// the function that is called every n milliseconds, usually to run a game loop
void (*game_loop_function)() = NULL;

// the amount of time (in ms) between each call of the game_loop_function
int game_loop_pause = 1000;

// internal variable, holds the next time the game_loop_function should be called
int next_game_fire_time;

// internal array holding the current color of each pixel
char * graphics;

// the width of the graphics display drawn
int graphics_width;

// the height of the graphics display drawn
int graphics_height;

// The background color. 
char background_color = 'B';

// struct that holds one single graphical change
// if x >= 0 && y>= 0, then it sets color c at pixel (x,y)
typedef struct gchange_t
{
	int x;
	int y;
	COLOR c;
} gchange_t;

// The thread_safe queue holding every graphic change to execute
queue_t * gchange_queue;

// Initializes the graphics, setting up curses and the internal data structures used
void ColorGraphics_Init(int w, int h)
{
	initscr(); // Start Curses
        cbreak(); //
        noecho(); // Don't automatically echo everything people type in
        curs_set(0); // Don't show the cursor
        keypad(stdscr, TRUE); // get arrow keys in getch()
        nodelay(stdscr, TRUE); // Don't delay the keys

	graphics_width = w;
	graphics_height = h;

	graphics = malloc(w*h);
	for(size_t i = 0; i < w*h; i++)
	{
		graphics[i] = NONE;
	}

	// initialize the curses colors used by the display
	start_color();
	init_pair(RED_CP, COLOR_BLUE, COLOR_RED);
	init_pair(BLUE_CP, COLOR_BLUE, COLOR_BLUE);
	init_pair(YELLOW_CP, COLOR_BLUE, COLOR_YELLOW);
	init_pair(MAGENTA_CP, COLOR_BLUE, COLOR_MAGENTA);
	init_pair(CYAN_CP, COLOR_BLUE, COLOR_CYAN);
	init_pair(WHITE_CP, COLOR_BLUE, COLOR_WHITE);
	init_pair(BLACK_CP, COLOR_BLUE, COLOR_BLACK);
	init_pair(GREEN_CP, COLOR_BLUE, COLOR_GREEN);

	gchange_queue = queue_create(-1);
}

/////////////////////////////// Graphics Routines ////////////////////////////

// A purely graphical function. 
// Uses cureses to set the terminal pixel at x,y to be valid color 'c'
// Returns 0 if successful, -1 if an invalid color was passed in
int draw(int x, int y, char c)
{
	int color_pair;
	if(c == RED) color_pair = RED_CP;
        else if(c == BLUE) color_pair = BLUE_CP;
        else if(c == YELLOW) color_pair = YELLOW_CP;
        else if(c == MAGENTA) color_pair = MAGENTA_CP;
        else if(c == CYAN) color_pair = CYAN_CP;
        else if(c == WHITE) color_pair = WHITE_CP;
        else if(c == BLACK) color_pair = BLACK_CP;
        else if(c == GREEN) color_pair = GREEN_CP;
        else
        {
		return -1;
        }

        attron(COLOR_PAIR(color_pair));
        mvprintw(y, x*2, "  ");
        attroff(COLOR_PAIR(color_pair));

	return 0;
}

// Sets pixel at (x,y) to be color c.
// Updates the graphics array accordingly.
// If the color is 'none', none is stored in the graphics array
// but the background_color is drawn on the screen at that location
int pixel_set(int x, int y, COLOR c)
{
	graphics[y*graphics_width+x] = c;
	int dres = draw(x,y,c);
	if(dres < 0)
	{
		draw(x,y,background_color);
	}
	return 0;
}
///////////////////// Public Methods /////////////////////////

// Adds a gchange_t holding information to change pixel (x,y) to color c
void ColorGraphics_SetPixel(int x, int y, COLOR c)
{
	gchange_t * change = malloc(sizeof(gchange_t));
	change->x = x;
	change->y = y;
	change->c = c;
	queue_push(gchange_queue, change);
}

// Creates a new gchange_t to refresh the screen with a new background color
void ColorGraphics_SetBackgroundColor(char c)
{
	background_color = c;
	gchange_t * refreshChange = malloc(sizeof(gchange_t));
	refreshChange->y = -1;
	queue_push(gchange_queue, refreshChange);
}

// Sets the key function to be kf
void ColorGraphics_SetKeyFunction(void (*kf)(int key))
{
	key_function = kf;
}

// Sets the game loop function
void ColorGraphics_SetGameLoopFunction(void (*gf)(), int delay)
{
	game_loop_function = gf;
	game_loop_pause = delay;
}

// Starts color graphics. It continuously does the following:
// 	1: While there are color changes in the gchange_queue, pull them, and execute them.
void ColorGraphics_Start()
{
	int pressedChar;
	next_game_fire_time = ColorGraphics_GetTimeMS() + game_loop_pause;
	while(!exit_at_next_frame)
        {
		// Graphically perform every change in gchange_queue
		int changeMade = 0;
		while(gchange_queue->size > 0)
		{
			changeMade = 1;
                	gchange_t * nextChange = queue_pull(gchange_queue);
	                //fprintf(output, "(g) Graphics Thread Got Change: %d %d %d!\n", nextChange->x, nextChange->y, nextChange->c);
               		if(nextChange->y < 0)
                	{
                        	//fprintf(output, "(g) Graphics Thread Repainting All!\n");
	                        for(int i = 0; i < graphics_width; i++)
	                        {
	                                for(int j = 0; j < graphics_height; j++)
	                                {
	                                        pixel_set(i,j,graphics[j*graphics_width+i]);
	                                }
	                        }
	                }
	                else
	                {
	                        //fprintf(output, "(g) Graphics Change calling pixel_set!\n");
	                        pixel_set(nextChange->x, nextChange->y, nextChange->c);
	                }
	                //fprintf(output, "(g) Graphics Change calling Curses Refresh\n");
	                //refresh();
			free(nextChange);
	        }

		// if a change to graphics was made, have curses refresh the terminal
		if(changeMade) refresh();

		// Get the next key, and call keyFunction with it
		if(key_function)
		{
			pressedChar = getch();
       	 		if(pressedChar != ERR)
       	        	{
                        	if(pressedChar == KEY_LEFT)
                        	{
                                	pressedChar = CGK_LEFT;
                        	}
				else if(pressedChar == KEY_UP)
				{
                        	        pressedChar = CGK_UP;
                        	}
				else if(pressedChar == KEY_RIGHT)
				{
                        	        pressedChar = CGK_RIGHT;
                        	}
				else if(pressedChar == KEY_DOWN)
                        	{
                                	pressedChar = CGK_DOWN;
                        	}

                        	(*key_function)(pressedChar);
                	}
		}
		// Call the game loop function if it has to fire
		if(game_loop_function)
		{
			if(game_loop_pause == 0)
			{
				game_loop_function();
			}
			else
			{
				while(next_game_fire_time <= ColorGraphics_GetTimeMS())
				{
					next_game_fire_time += game_loop_pause;
					game_loop_function();
				}
			}
		}
	}

	endwin();
}

// End color graphics
void ColorGraphics_End()
{
	exit_at_next_frame = 1;
}

// Helper function, gets current time in MS
int ColorGraphics_GetTimeMS()
{
        clock_t now = clock();
        return now * 1000 / CLOCKS_PER_SEC;
}

