#include "color_graphics.h"
#include <stdio.h>

int x = 10;
int y = 10;
int dir = CGK_RIGHT;
char cc = GREEN;


void processKey(int key)
{
	if(key == CGK_UP||key==CGK_DOWN||key==CGK_LEFT||key==CGK_RIGHT)
		dir = key;
	else
		ColorGraphics_End();
}

void gameLoop()
{
        if(cc == GREEN)
                cc = BLUE;
        else if(cc == BLUE)
                cc = CYAN;
        else if(cc == CYAN)
                cc = MAGENTA;
        else if(cc == MAGENTA)
                cc = BLACK;
        else if(cc == BLACK)
                cc = YELLOW;
        else if(cc == YELLOW)
		cc = RED;
	else if(cc == RED)
                cc = GREEN;


	if(dir==CGK_UP)y--;
	else if(dir==CGK_RIGHT) x++;
	else if(dir==CGK_LEFT)x--;
	else if(dir==CGK_DOWN)y++;

	ColorGraphics_SetPixel(x,y,cc);

}

int main()
{
	ColorGraphics_Init(40,40);
	ColorGraphics_SetKeyFunction(processKey);
	ColorGraphics_SetGameLoopFunction(gameLoop,100);
	ColorGraphics_SetBackgroundColor(WHITE);
	ColorGraphics_Start();
}

