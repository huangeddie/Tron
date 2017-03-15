#ifndef COLOR_GRAPHICS_H
#define COLOR_GRAPHICS_H

#define BLUE 'b'
#define BLACK 'B'
#define RED 'r'
#define YELLOW 'y'
#define MAGENTA 'm'
#define CYAN 'c'
#define WHITE 'w'
#define GREEN 'g'
#define NONE ' '


#define BLUE_CP 1
#define BLACK_CP 2
#define RED_CP 3
#define YELLOW_CP 4
#define MAGENTA_CP 5
#define CYAN_CP 6
#define WHITE_CP 7
#define GREEN_CP 8

#define CGK_UP 4
#define CGK_RIGHT 5
#define CGK_DOWN 2
#define CGK_LEFT 3

#define IS_ARROW_KEY(n) (n>=2&&n<=5)
#define COLOR char

void ColorGraphics_Init(int w, int h);
void ColorGraphics_Start();
void ColorGraphics_End();

void ColorGraphics_SetKeyFunction(void (*function)(int key));
void ColorGraphics_SetGameLoopFunction(void (*gf)(), int delay);

int ColorGraphics_GetTimeMS();

void ColorGraphics_SetPixel(int x, int y, COLOR color);
char ColorGraphics_ColorAt(int x, int y);

void ColorGraphics_SetBackgroundColor(COLOR c);
void ColorGraphics_Repaint();

#endif
