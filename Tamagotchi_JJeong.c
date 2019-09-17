// Tamagotchi game by JJeong
// Note : Header files are missing due to copyright issues

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "ButtonLED_HAL.h"
#include "UART_HAL.h"

#define TEN_SECONDS_COUNT  480000000 // 10 seconds = 480000000 / 48MHz
#define FIVE_SECONDS_COUNT 240000000 // 5 seconds = 2400000000 / 4MHz

#define EXPIRED 0 // use this to check if timer has expired

typedef enum { egg, child, adult, elder, death } state_t;

// moveCount is to keep track of number of movements to update happy
typedef struct { state_t stage; int happy; int energy; int age; int x_Pos; int y_Pos; int moveCount; int warmth; } tamagotchi_t;

// this function initializes all the peripherals except graphics
void initialize();

// this function initializes the graphics part
void InitGraphics(Graphics_Context* g_sContext_p);

// this function draws the borders using rectangle graphics library functions
void drawBorders(Graphics_Context* g_sContext_p)
{
	Graphics_Rectangle Border_East;
	Graphics_Rectangle Border_West;
	Graphics_Rectangle Border_South1;
	Graphics_Rectangle Border_South2;
	Graphics_Rectangle Border_North1;
	Graphics_Rectangle Border_North2;

	Border_East.xMin = 122;
	Border_East.xMax = 127;
	Border_East.yMin = 18;
	Border_East.yMax = 97;

	Border_West.xMin = 0;
	Border_West.xMax = 5;
	Border_West.yMin = 18;
	Border_West.yMax = 97;

	Border_South1.xMin = 7;
	Border_South1.xMax = 56;
	Border_South1.yMin = 92;
	Border_South1.yMax = 97;

	Border_South2.xMin = 72;
	Border_South2.xMax = 120;
	Border_South2.yMin = 92;
	Border_South2.yMax = 97;

	Border_North1.xMin = 7;
	Border_North1.xMax = 56;
	Border_North1.yMin = 18;
	Border_North1.yMax = 23;

	Border_North2.xMin = 72;
	Border_North2.xMax = 120;
	Border_North2.yMin = 18;
	Border_North2.yMax = 23;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_KHAKI);
	Graphics_fillRectangle(g_sContext_p, &Border_East);
	Graphics_fillRectangle(g_sContext_p, &Border_West);
	Graphics_fillRectangle(g_sContext_p, &Border_South1);
	Graphics_fillRectangle(g_sContext_p, &Border_South2);
	Graphics_fillRectangle(g_sContext_p, &Border_North1);
	Graphics_fillRectangle(g_sContext_p, &Border_North2);

	// drawing circles to fill gap
	Graphics_fillCircle(g_sContext_p, 69, 20, 2);
	Graphics_fillCircle(g_sContext_p, 69, 94, 2);
	Graphics_fillCircle(g_sContext_p, 64, 20, 2);
	Graphics_fillCircle(g_sContext_p, 64, 94, 2);
	Graphics_fillCircle(g_sContext_p, 59, 20, 2);
	Graphics_fillCircle(g_sContext_p, 59, 94, 2);
}

// draws Egg
void drawEgg(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	// making sure egg is the default position
	myPet->stage = egg;
	myPet->x_Pos = 0;
	myPet->y_Pos = 0;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_LIGHT_GOLDENRON_YELLOW);
	Graphics_fillCircle(g_sContext_p, 63, 74, 15);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Egg_Stripe1;

	Egg_Stripe1.xMin = 54;
	Egg_Stripe1.xMax = 71;
	Egg_Stripe1.yMin = 84;
	Egg_Stripe1.yMax = 85;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe1);

	Graphics_Rectangle Egg_Stripe2;

	Egg_Stripe2.xMin = 50;
	Egg_Stripe2.xMax = 75;
	Egg_Stripe2.yMin = 78;
	Egg_Stripe2.yMax = 79;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe2);

	Graphics_Rectangle Egg_Stripe3;

	Egg_Stripe3.xMin = 48;
	Egg_Stripe3.xMax = 77;
	Egg_Stripe3.yMin = 72;
	Egg_Stripe3.yMax = 73;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe3);

	Graphics_Rectangle Egg_Stripe4;

	Egg_Stripe4.xMin = 50;
	Egg_Stripe4.xMax = 75;
	Egg_Stripe4.yMin = 66;
	Egg_Stripe4.yMax = 67;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe4);
}

// draws a chick (baby chicken), will be called "Mike"
void drawChild(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	// keeps track of position
	int x_move = myPet->x_Pos;
	int y_move = myPet->y_Pos;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 65 + y_move, 9);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 76 + y_move, 11);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 61 + y_move, 2);
	Graphics_fillCircle(g_sContext_p, 67 + +x_move, 61 + y_move, 2);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + x_move;
	Lips1.xMax = 65 + x_move;
	Lips1.yMin = 66 + y_move;
	Lips1.yMax = 67 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + x_move;
	Lips2.xMax = 65 + x_move;
	Lips2.yMin = 69 + y_move;
	Lips2.yMax = 70 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips2);


	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_Rectangle mouth;

	mouth.xMin = 61 + x_move;
	mouth.xMax = 65 + x_move;
	mouth.yMin = 68 + y_move;
	mouth.yMax = 69 + y_move;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + x_move;
	Leg1.xMax = 60 + x_move;
	Leg1.yMin = 87 + y_move;
	Leg1.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + x_move;
	Leg2.xMax = 69 + x_move;
	Leg2.yMin = 87 + y_move;
	Leg2.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

// draws a full grown rooster
void drawAdult(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	// keeps track of position
	int x_move = myPet->x_Pos;
	int y_move = myPet->y_Pos;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 65 + y_move, 9);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 76 + y_move, 11);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 61 + y_move, 2);
	Graphics_fillCircle(g_sContext_p, 67 + x_move, 61 + y_move, 2);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_Rectangle comb;

	comb.xMin = 62 + x_move;
	comb.xMax = 64 + x_move;
	comb.yMin = 52 + y_move;
	comb.yMax = 56 + y_move;

	Graphics_fillRectangle(g_sContext_p, &comb);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 52 + x_move, 76 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 73 + x_move, 76 + y_move, 4);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + x_move;
	Lips1.xMax = 65 + x_move;
	Lips1.yMin = 66 + y_move;
	Lips1.yMax = 67 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + x_move;
	Lips2.xMax = 65 + x_move;
	Lips2.yMin = 69 + y_move;
	Lips2.yMax = 70 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips2);


	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
	Graphics_Rectangle mouth;

	mouth.xMin = 61 + x_move;
	mouth.xMax = 65 + x_move;
	mouth.yMin = 68 + y_move;
	mouth.yMax = 69 + y_move;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + x_move;
	Leg1.xMax = 60 + x_move;
	Leg1.yMin = 87 + y_move;
	Leg1.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + x_move;
	Leg2.xMax = 69 + x_move;
	Leg2.yMin = 87 + y_move;
	Leg2.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

void eraseErase(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	// keeps track of position
	int x_move = myPet->x_Pos;
	int y_move = myPet->y_Pos;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);

	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 65 + y_move, 9);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 76 + y_move, 11);
	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 61 + y_move, 2);
	Graphics_fillCircle(g_sContext_p, 67 + x_move, 61 + y_move, 2);

	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_Rectangle comb;

	comb.xMin = 62 + x_move;
	comb.xMax = 64 + x_move;
	comb.yMin = 52 + y_move;
	comb.yMax = 56 + y_move;

	Graphics_fillRectangle(g_sContext_p, &comb);

	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 52 + x_move, 76 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 73 + x_move, 76 + y_move, 4);

	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + x_move;
	Lips1.xMax = 65 + x_move;
	Lips1.yMin = 66 + y_move;
	Lips1.yMax = 67 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + x_move;
	Lips2.xMax = 65 + x_move;
	Lips2.yMin = 69 + y_move;
	Lips2.yMax = 70 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips2);


	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
	Graphics_Rectangle mouth;

	mouth.xMin = 61 + x_move;
	mouth.xMax = 65 + x_move;
	mouth.yMin = 68 + y_move;
	mouth.yMax = 69 + y_move;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	//Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + x_move;
	Leg1.xMax = 60 + x_move;
	Leg1.yMin = 87 + y_move;
	Leg1.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + x_move;
	Leg2.xMax = 69 + x_move;
	Leg2.yMin = 87 + y_move;
	Leg2.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

// bonus feature, at age 13 the tamagotchi turns into an elder with a beard and glasses !
void drawElder(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	// keeps track of position
	int x_move = myPet->x_Pos;
	int y_move = myPet->y_Pos;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 65 + y_move, 9);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 76 + y_move, 11);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 61 + y_move, 2);
	Graphics_fillCircle(g_sContext_p, 67 + x_move, 61 + y_move, 2);

	// drawing the elder wearing glasses
	Graphics_drawLine(g_sContext_p, 60 + x_move, 60 + y_move, 65 + x_move, 60 + y_move);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GRAY);
	Graphics_Rectangle comb;

	comb.xMin = 62 + x_move;
	comb.xMax = 64 + x_move;
	comb.yMin = 52 + y_move;
	comb.yMax = 56 + y_move;

	Graphics_fillRectangle(g_sContext_p, &comb);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 52 + x_move, 76 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 73 + x_move, 76 + y_move, 4);

	// drawing beard
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GRAY);
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 71 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 71 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 67 + x_move, 71 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 61 + x_move, 72 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 65 + x_move, 72 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 73 + y_move, 4);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + x_move;
	Lips1.xMax = 65 + x_move;
	Lips1.yMin = 66 + y_move;
	Lips1.yMax = 67 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + x_move;
	Lips2.xMax = 65 + x_move;
	Lips2.yMin = 69 + y_move;
	Lips2.yMax = 70 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips2);


	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
	Graphics_Rectangle mouth;

	mouth.xMin = 61 + x_move;
	mouth.xMax = 65 + x_move;
	mouth.yMin = 68 + y_move;
	mouth.yMax = 69 + y_move;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);
	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + x_move;
	Leg1.xMax = 60 + x_move;
	Leg1.yMin = 87 + y_move;
	Leg1.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + x_move;
	Leg2.xMax = 69 + x_move;
	Leg2.yMin = 87 + y_move;
	Leg2.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

// erasing elder by coloring it in blue, which is the background color
void eraseElder(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	// keeps track of position
	int x_move = myPet->x_Pos;
	int y_move = myPet->y_Pos;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);

	Graphics_fillCircle(g_sContext_p, 63 + x_move, 65 + y_move, 9);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 76 + y_move, 11);
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 61 + y_move, 2);
	Graphics_fillCircle(g_sContext_p, 67 + x_move, 61 + y_move, 2);

	// drawing the elder wearing glasses
	Graphics_drawLine(g_sContext_p, 60 + x_move, 60 + y_move, 65 + x_move, 60 + y_move);

	Graphics_Rectangle comb;

	comb.xMin = 62 + x_move;
	comb.xMax = 64 + x_move;
	comb.yMin = 52 + y_move;
	comb.yMax = 56 + y_move;

	Graphics_fillRectangle(g_sContext_p, &comb);

	Graphics_fillCircle(g_sContext_p, 52 + x_move, 76 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 73 + x_move, 76 + y_move, 4);

	// drawing beard
	Graphics_fillCircle(g_sContext_p, 59 + x_move, 71 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 71 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 67 + x_move, 71 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 61 + x_move, 72 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 65 + x_move, 72 + y_move, 4);
	Graphics_fillCircle(g_sContext_p, 63 + x_move, 73 + y_move, 4);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + x_move;
	Lips1.xMax = 65 + x_move;
	Lips1.yMin = 66 + y_move;
	Lips1.yMax = 67 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + x_move;
	Lips2.xMax = 65 + x_move;
	Lips2.yMin = 69 + y_move;
	Lips2.yMax = 70 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Lips2);

	Graphics_Rectangle mouth;

	mouth.xMin = 61 + x_move;
	mouth.xMax = 65 + x_move;
	mouth.yMin = 68 + y_move;
	mouth.yMax = 69 + y_move;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + x_move;
	Leg1.xMax = 60 + x_move;
	Leg1.yMin = 87 + y_move;
	Leg1.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + x_move;
	Leg2.xMax = 69 + x_move;
	Leg2.yMin = 87 + y_move;
	Leg2.yMax = 90 + y_move;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

// draws a tombstone with a message
void drawDeath(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 1;
	background.xMax = 127;
	background.yMin = 1;
	background.yMax = 127;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GRAY);
	Graphics_Rectangle Tombstone_base;

	Tombstone_base.xMin = 32;
	Tombstone_base.xMax = 94;
	Tombstone_base.yMin = 45;
	Tombstone_base.yMax = 108;

	Graphics_fillRectangle(g_sContext_p, &Tombstone_base);
	Graphics_fillCircle(g_sContext_p, 63, 45, 30);

	char deathMessage[7] = " R.I.P";
	Graphics_setFont(g_sContext_p, &g_sFontCmss22b);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_drawString(g_sContext_p, deathMessage, -1, 38, 45, true);
}

// erases the egg off the screen from the current position by coloring it blue
void eraseEgg(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);

	Graphics_fillCircle(g_sContext_p, 63, 74, 15);

	Graphics_Rectangle Egg_Stripe1;

	Egg_Stripe1.xMin = 54;
	Egg_Stripe1.xMax = 71;
	Egg_Stripe1.yMin = 84;
	Egg_Stripe1.yMax = 85;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe1);

	Graphics_Rectangle Egg_Stripe2;

	Egg_Stripe2.xMin = 50;
	Egg_Stripe2.xMax = 75;
	Egg_Stripe2.yMin = 78;
	Egg_Stripe2.yMax = 79;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe2);

	Graphics_Rectangle Egg_Stripe3;

	Egg_Stripe3.xMin = 48;
	Egg_Stripe3.xMax = 77;
	Egg_Stripe3.yMin = 72;
	Egg_Stripe3.yMax = 73;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe3);

	Graphics_Rectangle Egg_Stripe4;

	Egg_Stripe4.xMin = 50;
	Egg_Stripe4.xMax = 75;
	Egg_Stripe4.yMin = 66;
	Egg_Stripe4.yMax = 67;

	Graphics_fillRectangle(g_sContext_p, &Egg_Stripe4);
}

// erases child
void eraseChild(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
	Graphics_fillCircle(g_sContext_p, 63 + myPet->x_Pos, 65 + myPet->y_Pos, 9);
	Graphics_fillCircle(g_sContext_p, 63 + myPet->x_Pos, 76 + myPet->y_Pos, 11);
	Graphics_fillCircle(g_sContext_p, 59 + myPet->x_Pos, 61 + myPet->y_Pos, 2);
	Graphics_fillCircle(g_sContext_p, 67 + myPet->x_Pos, 61 + myPet->y_Pos, 2);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + myPet->x_Pos;
	Lips1.xMax = 65 + myPet->x_Pos;
	Lips1.yMin = 66 + myPet->y_Pos;
	Lips1.yMax = 67 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + myPet->x_Pos;
	Lips2.xMax = 65 + myPet->x_Pos;
	Lips2.yMin = 69 + myPet->y_Pos;
	Lips2.yMax = 70 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Lips2);

	Graphics_Rectangle mouth;

	mouth.xMin = 61 + myPet->x_Pos;
	mouth.xMax = 65 + myPet->x_Pos;
	mouth.yMin = 68 + myPet->y_Pos;
	mouth.yMax = 69 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + myPet->x_Pos;
	Leg1.xMax = 60 + myPet->x_Pos;
	Leg1.yMin = 87 + myPet->y_Pos;
	Leg1.yMax = 90 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + myPet->x_Pos;
	Leg2.xMax = 69 + myPet->x_Pos;
	Leg2.yMin = 87 + myPet->y_Pos;
	Leg2.yMax = 90 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

// erases the grown rooster
void eraseAdult(Graphics_Context* g_sContext_p, tamagotchi_t* myPet)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);

	Graphics_fillCircle(g_sContext_p, 63 + myPet->x_Pos, 65 + myPet->y_Pos, 9);
	Graphics_fillCircle(g_sContext_p, 63 + myPet->x_Pos, 76 + myPet->y_Pos, 11);
	Graphics_fillCircle(g_sContext_p, 59 + myPet->x_Pos, 61 + myPet->y_Pos, 2);
	Graphics_fillCircle(g_sContext_p, 67 + myPet->x_Pos, 61 + myPet->y_Pos, 2);

	Graphics_Rectangle comb;

	comb.xMin = 62 + myPet->x_Pos;
	comb.xMax = 64 + myPet->x_Pos;
	comb.yMin = 52 + myPet->y_Pos;
	comb.yMax = 56 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &comb);
	Graphics_fillCircle(g_sContext_p, 52 + myPet->x_Pos, 76 + myPet->y_Pos, 4);
	Graphics_fillCircle(g_sContext_p, 73 + myPet->x_Pos, 76 + myPet->y_Pos, 4);

	Graphics_Rectangle Lips1;

	Lips1.xMin = 61 + myPet->x_Pos;
	Lips1.xMax = 65 + myPet->x_Pos;
	Lips1.yMin = 66 + myPet->y_Pos;
	Lips1.yMax = 67 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Lips1);

	Graphics_Rectangle Lips2;

	Lips2.xMin = 61 + myPet->x_Pos;
	Lips2.xMax = 65 + myPet->x_Pos;
	Lips2.yMin = 69 + myPet->y_Pos;
	Lips2.yMax = 70 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Lips2);

	Graphics_Rectangle mouth;

	mouth.xMin = 61 + myPet->x_Pos;
	mouth.xMax = 65 + myPet->x_Pos;
	mouth.yMin = 68 + myPet->y_Pos;
	mouth.yMax = 69 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &mouth);

	Graphics_Rectangle Leg1;

	Leg1.xMin = 57 + myPet->x_Pos;
	Leg1.xMax = 60 + myPet->x_Pos;
	Leg1.yMin = 87 + myPet->y_Pos;
	Leg1.yMax = 90 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Leg1);

	Graphics_Rectangle Leg2;

	Leg2.xMin = 66 + myPet->x_Pos;
	Leg2.xMax = 69 + myPet->x_Pos;
	Leg2.yMin = 87 + myPet->y_Pos;
	Leg2.yMax = 90 + myPet->y_Pos;

	Graphics_fillRectangle(g_sContext_p, &Leg2);
}

// this updates the age status into my declared tamagotchi
void updateAgeStatus(tamagotchi_t* myPet)
{
	if ((Timer32_getValue(TIMER32_0_BASE) == EXPIRED))
	{
		myPet->age = myPet->age + 1;
	}
}

// updates both energy and happy status
void updateEnergyHappyStatus(tamagotchi_t* myPet, unsigned char command)
{
	static int tamIsFull = 0; //using static int variable to keep track of fullness of tamagotchi

	if ((Timer32_getValue(TIMER32_1_BASE) == EXPIRED) && command != 'f') // energy decreases every 5 seconds
		if (myPet->energy >= 1)
			myPet->energy = myPet->energy - 1;

	if (command == 'f' && myPet->stage != egg) // cannot feed when egg
	{
		if (myPet->energy == 5) // Bonus feature, if tamagotchi is full, feeding causes negative effects
		{
			tamIsFull++;
			if (tamIsFull >= 1 && tamIsFull < 3)
				myPet->happy = myPet->happy - 1;
			if (tamIsFull == 3) // when tamagotchi is fed 3 times consecutively when full, it dies
				myPet->stage = death;
		}

		if (myPet->energy <= 4 && myPet->stage != elder)
		{
			myPet->energy = myPet->energy + 1;
			tamIsFull = 0; // Bonus feature, this resets fullness of tamagotchi
		}
		if (myPet->energy <= 2 && myPet->stage == elder) // Bonus feature, when elder the energy does not increase more than 3
			myPet->energy = myPet->energy + 1;
	}

	if ((Timer32_getValue(TIMER32_0_BASE) == EXPIRED) && command != 'm')
		if (myPet->happy >= 1)
			myPet->happy = myPet->happy - 1;

	if (((Timer32_getValue(TIMER32_0_BASE) != EXPIRED)) && (command == 'm') && (myPet->stage != egg))
	{
		myPet->moveCount = myPet->moveCount + 1;
		if (myPet->happy <= 5 && myPet->moveCount == 7) //to make sure happy only increases when moved 7
		{
			myPet->moveCount = 0;
			if (myPet->happy <= 4)
			{
				myPet->happy = myPet->happy + 1;
				myPet->energy = myPet->energy - 1;
			}
			if (myPet->happy == 5)
				myPet->energy = myPet->energy - 1;
		}
	}
}

// makes the string for age (possible age is 999999, which fills out the screen before the "BR" string)
// similar function used from previous homework (using division and remainder to create a string value from integers)
void makeAgeString(tamagotchi_t* myPet, char* string)
{
	unsigned int num = myPet->age;

	// these are the digits that will be converted to string values
	unsigned int first_digit = 0;
	unsigned int second_digit = 0;
	unsigned int third_digit = 0;
	unsigned int fourth_digit = 0;
	unsigned int fifth_digit = 0;
	unsigned int sixth_digit = 0;

	// the two if conditions are there to prevent a case where the first or/and second digit is a zero
	if (num >= 100000 && num < 1000000)
	{
		first_digit = num / 100000;
		num = num - (100000 * first_digit);

		second_digit = num / 10000;
		num = num - (10000 * second_digit);

		third_digit = num / 1000;
		num = num - (1000 * third_digit);

		fourth_digit = num / 100;
		num = num - (100 * fourth_digit);

		fifth_digit = num / 10;
		num = num - (10 * fifth_digit);

		sixth_digit = num / 1;
		num = num - (1 * sixth_digit);

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit + 48;
		string[3] = fourth_digit + 48;
		string[4] = fifth_digit + 48;
		string[5] = sixth_digit + 48;
	}

	else if (num >= 10000 && num < 100000)
	{
		first_digit = num / 10000;
		num = num - (10000 * first_digit);

		second_digit = num / 1000;
		num = num - (1000 * second_digit);

		third_digit = num / 100;
		num = num - (100 * third_digit);

		fourth_digit = num / 10;
		num = num - (10 * fourth_digit);

		fifth_digit = num / 1;
		num = num - (1 * fifth_digit);

		sixth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit + 48;
		string[3] = fourth_digit + 48;
		string[4] = fifth_digit + 48;
		string[5] = sixth_digit;
	}

	else if (num >= 1000 && num < 10000)
	{
		first_digit = num / 1000;
		num = num - (1000 * first_digit);

		second_digit = num / 100;
		num = num - (100 * second_digit);

		third_digit = num / 10;
		num = num - (10 * third_digit);

		fourth_digit = num / 1;
		num = num - (1 * fourth_digit);

		fifth_digit = 32;

		sixth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit + 48;
		string[3] = fourth_digit + 48;
		string[4] = fifth_digit;
		string[5] = sixth_digit;
	}

	else if (num >= 100 && num < 1000)
	{
		first_digit = num / 100;
		num = num - (100 * first_digit);

		second_digit = num / 10;
		num = num - (10 * second_digit);

		third_digit = num / 1;
		num = num - (1 * third_digit);

		fourth_digit = 32;

		fifth_digit = 32;

		sixth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit + 48;
		string[3] = fourth_digit;
		string[4] = fifth_digit;
		string[5] = sixth_digit;
	}

	else if (num >= 10 && num < 100)
	{
		first_digit = num / 10;
		num = num - (10 * first_digit);

		second_digit = num / 1;
		num = num - (1 * second_digit);

		third_digit = 32;

		fourth_digit = 32;

		fifth_digit = 32;

		sixth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit;
		string[3] = fourth_digit;
		string[4] = fifth_digit;
		string[5] = sixth_digit;
	}

	else if (num < 10)
	{
		first_digit = num / 1;
		num = num - (1 * first_digit);

		second_digit = 32;

		third_digit = 32;

		fourth_digit = 32;

		fifth_digit = 32;

		sixth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit;
		string[2] = third_digit;
		string[3] = fourth_digit;
		string[4] = fifth_digit;
		string[5] = sixth_digit;
	}
}

// simply draws updated age status
void drawAgeStatus(Graphics_Context* g_sContext_p, char* string)
{
	Graphics_drawString(g_sContext_p, string, -1, 36, 3, true);
}

// draws energy and happy string using updated energy and happy values in my tamagotchi
// the string values commented out are the previous one before the bonus creative design meter feature was added
// colored rectangles were used to indicate energy and happy status (green, yellow: filled, red: unfilled)
void drawEnergyHappyStatus(tamagotchi_t* myPet, Graphics_Context* g_sContext_p)
{
	if (myPet->energy == 5)
	{
		/*energy[0] = '*';
		energy[1] = ' ';
		energy[2] = '*';
		energy[3] = ' ';
		energy[4] = '*';
		energy[5] = ' ';
		energy[6] = '*';
		energy[7] = ' ';
		energy[8] = '*';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle EnergyBar;

		EnergyBar.xMin = 51;
		EnergyBar.xMax = 107;
		EnergyBar.yMin = 102;
		EnergyBar.yMax = 112;

		Graphics_drawRectangle(g_sContext_p, &EnergyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
		Graphics_Rectangle Energy1;

		Energy1.xMin = 53;
		Energy1.xMax = 61;
		Energy1.yMin = 103;
		Energy1.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy1);

		Graphics_Rectangle Energy2;

		Energy2.xMin = 64;
		Energy2.xMax = 72;
		Energy2.yMin = 103;
		Energy2.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy2);

		Graphics_Rectangle Energy3;

		Energy3.xMin = 75;
		Energy3.xMax = 83;
		Energy3.yMin = 103;
		Energy3.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy3);

		Graphics_Rectangle Energy4;

		Energy4.xMin = 86;
		Energy4.xMax = 94;
		Energy4.yMin = 103;
		Energy4.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy4);

		Graphics_Rectangle Energy5;

		Energy5.xMin = 97;
		Energy5.xMax = 105;
		Energy5.yMin = 103;
		Energy5.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->energy == 4)
	{
		/*energy[0] = '*';
		energy[1] = ' ';
		energy[2] = '*';
		energy[3] = ' ';
		energy[4] = '*';
		energy[5] = ' ';
		energy[6] = '*';
		energy[7] = ' ';
		energy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle EnergyBar;

		EnergyBar.xMin = 51;
		EnergyBar.xMax = 107;
		EnergyBar.yMin = 102;
		EnergyBar.yMax = 112;

		Graphics_drawRectangle(g_sContext_p, &EnergyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
		Graphics_Rectangle Energy1;

		Energy1.xMin = 53;
		Energy1.xMax = 61;
		Energy1.yMin = 103;
		Energy1.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy1);

		Graphics_Rectangle Energy2;

		Energy2.xMin = 64;
		Energy2.xMax = 72;
		Energy2.yMin = 103;
		Energy2.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy2);

		Graphics_Rectangle Energy3;

		Energy3.xMin = 75;
		Energy3.xMax = 83;
		Energy3.yMin = 103;
		Energy3.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy3);

		Graphics_Rectangle Energy4;

		Energy4.xMin = 86;
		Energy4.xMax = 94;
		Energy4.yMin = 103;
		Energy4.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy4);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Energy5;

		Energy5.xMin = 97;
		Energy5.xMax = 105;
		Energy5.yMin = 103;
		Energy5.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->energy == 3)
	{
		/*energy[0] = '*';
		energy[1] = ' ';
		energy[2] = '*';
		energy[3] = ' ';
		energy[4] = '*';
		energy[5] = ' ';
		energy[6] = '-';
		energy[7] = ' ';
		energy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle EnergyBar;

		EnergyBar.xMin = 51;
		EnergyBar.xMax = 107;
		EnergyBar.yMin = 102;
		EnergyBar.yMax = 112;

		Graphics_drawRectangle(g_sContext_p, &EnergyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
		Graphics_Rectangle Energy1;

		Energy1.xMin = 53;
		Energy1.xMax = 61;
		Energy1.yMin = 103;
		Energy1.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy1);

		Graphics_Rectangle Energy2;

		Energy2.xMin = 64;
		Energy2.xMax = 72;
		Energy2.yMin = 103;
		Energy2.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy2);

		Graphics_Rectangle Energy3;

		Energy3.xMin = 75;
		Energy3.xMax = 83;
		Energy3.yMin = 103;
		Energy3.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy3);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Energy4;

		Energy4.xMin = 86;
		Energy4.xMax = 94;
		Energy4.yMin = 103;
		Energy4.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy4);


		Graphics_Rectangle Energy5;

		Energy5.xMin = 97;
		Energy5.xMax = 105;
		Energy5.yMin = 103;
		Energy5.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->energy == 2)
	{
		/*energy[0] = '*';
		energy[1] = ' ';
		energy[2] = '*';
		energy[3] = ' ';
		energy[4] = '-';
		energy[5] = ' ';
		energy[6] = '-';
		energy[7] = ' ';
		energy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle EnergyBar;

		EnergyBar.xMin = 51;
		EnergyBar.xMax = 107;
		EnergyBar.yMin = 102;
		EnergyBar.yMax = 112;

		Graphics_drawRectangle(g_sContext_p, &EnergyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
		Graphics_Rectangle Energy1;

		Energy1.xMin = 53;
		Energy1.xMax = 61;
		Energy1.yMin = 103;
		Energy1.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy1);

		Graphics_Rectangle Energy2;

		Energy2.xMin = 64;
		Energy2.xMax = 72;
		Energy2.yMin = 103;
		Energy2.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy2);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Energy3;

		Energy3.xMin = 75;
		Energy3.xMax = 83;
		Energy3.yMin = 103;
		Energy3.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy3);

		Graphics_Rectangle Energy4;

		Energy4.xMin = 86;
		Energy4.xMax = 94;
		Energy4.yMin = 103;
		Energy4.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy4);


		Graphics_Rectangle Energy5;

		Energy5.xMin = 97;
		Energy5.xMax = 105;
		Energy5.yMin = 103;
		Energy5.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->energy == 1)
	{
		/*energy[0] = '*';
		energy[1] = ' ';
		energy[2] = '-';
		energy[3] = ' ';
		energy[4] = '-';
		energy[5] = ' ';
		energy[6] = '-';
		energy[7] = ' ';
		energy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle EnergyBar;

		EnergyBar.xMin = 51;
		EnergyBar.xMax = 107;
		EnergyBar.yMin = 102;
		EnergyBar.yMax = 112;

		Graphics_drawRectangle(g_sContext_p, &EnergyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
		Graphics_Rectangle Energy1;

		Energy1.xMin = 53;
		Energy1.xMax = 61;
		Energy1.yMin = 103;
		Energy1.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy1);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Energy2;

		Energy2.xMin = 64;
		Energy2.xMax = 72;
		Energy2.yMin = 103;
		Energy2.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy2);


		Graphics_Rectangle Energy3;

		Energy3.xMin = 75;
		Energy3.xMax = 83;
		Energy3.yMin = 103;
		Energy3.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy3);

		Graphics_Rectangle Energy4;

		Energy4.xMin = 86;
		Energy4.xMax = 94;
		Energy4.yMin = 103;
		Energy4.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy4);


		Graphics_Rectangle Energy5;

		Energy5.xMin = 97;
		Energy5.xMax = 105;
		Energy5.yMin = 103;
		Energy5.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->energy == 0)
	{
		/*energy[0] = '-';
		energy[1] = ' ';
		energy[2] = '-';
		energy[3] = ' ';
		energy[4] = '-';
		energy[5] = ' ';
		energy[6] = '-';
		energy[7] = ' ';
		energy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle EnergyBar;

		EnergyBar.xMin = 51;
		EnergyBar.xMax = 107;
		EnergyBar.yMin = 102;
		EnergyBar.yMax = 112;

		Graphics_drawRectangle(g_sContext_p, &EnergyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Energy1;

		Energy1.xMin = 53;
		Energy1.xMax = 61;
		Energy1.yMin = 103;
		Energy1.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy1);

		Graphics_Rectangle Energy2;

		Energy2.xMin = 64;
		Energy2.xMax = 72;
		Energy2.yMin = 103;
		Energy2.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy2);


		Graphics_Rectangle Energy3;

		Energy3.xMin = 75;
		Energy3.xMax = 83;
		Energy3.yMin = 103;
		Energy3.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy3);

		Graphics_Rectangle Energy4;

		Energy4.xMin = 86;
		Energy4.xMax = 94;
		Energy4.yMin = 103;
		Energy4.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy4);


		Graphics_Rectangle Energy5;

		Energy5.xMin = 97;
		Energy5.xMax = 105;
		Energy5.yMin = 103;
		Energy5.yMax = 111;

		Graphics_fillRectangle(g_sContext_p, &Energy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->happy == 5)
	{
		/*happy[0] = '*';
		happy[1] = ' ';
		happy[2] = '*';
		happy[3] = ' ';
		happy[4] = '*';
		happy[5] = ' ';
		happy[6] = '*';
		happy[7] = ' ';
		happy[8] = '*';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle HappyBar;

		HappyBar.xMin = 51;
		HappyBar.xMax = 107;
		HappyBar.yMin = 115;
		HappyBar.yMax = 125;

		Graphics_drawRectangle(g_sContext_p, &HappyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
		Graphics_Rectangle Happy1;

		Happy1.xMin = 53;
		Happy1.xMax = 61;
		Happy1.yMin = 116;
		Happy1.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy1);

		Graphics_Rectangle Happy2;

		Happy2.xMin = 64;
		Happy2.xMax = 72;
		Happy2.yMin = 116;
		Happy2.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy2);

		Graphics_Rectangle Happy3;

		Happy3.xMin = 75;
		Happy3.xMax = 83;
		Happy3.yMin = 116;
		Happy3.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy3);

		Graphics_Rectangle Happy4;

		Happy4.xMin = 86;
		Happy4.xMax = 94;
		Happy4.yMin = 116;
		Happy4.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy4);

		Graphics_Rectangle Happy5;

		Happy5.xMin = 97;
		Happy5.xMax = 105;
		Happy5.yMin = 116;
		Happy5.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->happy == 4)
	{
		/*happy[0] = '*';
		happy[1] = ' ';
		happy[2] = '*';
		happy[3] = ' ';
		happy[4] = '*';
		happy[5] = ' ';
		happy[6] = '*';
		happy[7] = ' ';
		happy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle HappyBar;

		HappyBar.xMin = 51;
		HappyBar.xMax = 107;
		HappyBar.yMin = 115;
		HappyBar.yMax = 125;

		Graphics_drawRectangle(g_sContext_p, &HappyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
		Graphics_Rectangle Happy1;

		Happy1.xMin = 53;
		Happy1.xMax = 61;
		Happy1.yMin = 116;
		Happy1.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy1);

		Graphics_Rectangle Happy2;

		Happy2.xMin = 64;
		Happy2.xMax = 72;
		Happy2.yMin = 116;
		Happy2.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy2);

		Graphics_Rectangle Happy3;

		Happy3.xMin = 75;
		Happy3.xMax = 83;
		Happy3.yMin = 116;
		Happy3.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy3);


		Graphics_Rectangle Happy4;

		Happy4.xMin = 86;
		Happy4.xMax = 94;
		Happy4.yMin = 116;
		Happy4.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy4);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Happy5;

		Happy5.xMin = 97;
		Happy5.xMax = 105;
		Happy5.yMin = 116;
		Happy5.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}


	if (myPet->happy == 3)
	{
		/*happy[0] = '*';
		happy[1] = ' ';
		happy[2] = '*';
		happy[3] = ' ';
		happy[4] = '*';
		happy[5] = ' ';
		happy[6] = '-';
		happy[7] = ' ';
		happy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle HappyBar;

		HappyBar.xMin = 51;
		HappyBar.xMax = 107;
		HappyBar.yMin = 115;
		HappyBar.yMax = 125;

		Graphics_drawRectangle(g_sContext_p, &HappyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
		Graphics_Rectangle Happy1;

		Happy1.xMin = 53;
		Happy1.xMax = 61;
		Happy1.yMin = 116;
		Happy1.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy1);

		Graphics_Rectangle Happy2;

		Happy2.xMin = 64;
		Happy2.xMax = 72;
		Happy2.yMin = 116;
		Happy2.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy2);

		Graphics_Rectangle Happy3;

		Happy3.xMin = 75;
		Happy3.xMax = 83;
		Happy3.yMin = 116;
		Happy3.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy3);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Happy4;

		Happy4.xMin = 86;
		Happy4.xMax = 94;
		Happy4.yMin = 116;
		Happy4.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy4);

		Graphics_Rectangle Happy5;

		Happy5.xMin = 97;
		Happy5.xMax = 105;
		Happy5.yMin = 116;
		Happy5.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	if (myPet->happy == 2)
	{
		/*happy[0] = '*';
		happy[1] = ' ';
		happy[2] = '*';
		happy[3] = ' ';
		happy[4] = '-';
		happy[5] = ' ';
		happy[6] = '-';
		happy[7] = ' ';
		happy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle HappyBar;

		HappyBar.xMin = 51;
		HappyBar.xMax = 107;
		HappyBar.yMin = 115;
		HappyBar.yMax = 125;

		Graphics_drawRectangle(g_sContext_p, &HappyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
		Graphics_Rectangle Happy1;

		Happy1.xMin = 53;
		Happy1.xMax = 61;
		Happy1.yMin = 116;
		Happy1.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy1);

		Graphics_Rectangle Happy2;

		Happy2.xMin = 64;
		Happy2.xMax = 72;
		Happy2.yMin = 116;
		Happy2.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy2);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Happy3;

		Happy3.xMin = 75;
		Happy3.xMax = 83;
		Happy3.yMin = 116;
		Happy3.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy3);

		Graphics_Rectangle Happy4;

		Happy4.xMin = 86;
		Happy4.xMax = 94;
		Happy4.yMin = 116;
		Happy4.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy4);

		Graphics_Rectangle Happy5;

		Happy5.xMin = 97;
		Happy5.xMax = 105;
		Happy5.yMin = 116;
		Happy5.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}


	if (myPet->happy == 1)
	{
		/*happy[0] = '*';
		happy[1] = ' ';
		happy[2] = '-';
		happy[3] = ' ';
		happy[4] = '-';
		happy[5] = ' ';
		happy[6] = '-';
		happy[7] = ' ';
		happy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle HappyBar;

		HappyBar.xMin = 51;
		HappyBar.xMax = 107;
		HappyBar.yMin = 115;
		HappyBar.yMax = 125;

		Graphics_drawRectangle(g_sContext_p, &HappyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
		Graphics_Rectangle Happy1;

		Happy1.xMin = 53;
		Happy1.xMax = 61;
		Happy1.yMin = 116;
		Happy1.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy1);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Happy2;

		Happy2.xMin = 64;
		Happy2.xMax = 72;
		Happy2.yMin = 116;
		Happy2.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy2);

		Graphics_Rectangle Happy3;

		Happy3.xMin = 75;
		Happy3.xMax = 83;
		Happy3.yMin = 116;
		Happy3.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy3);

		Graphics_Rectangle Happy4;

		Happy4.xMin = 86;
		Happy4.xMax = 94;
		Happy4.yMin = 116;
		Happy4.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy4);

		Graphics_Rectangle Happy5;

		Happy5.xMin = 97;
		Happy5.xMax = 105;
		Happy5.yMin = 116;
		Happy5.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}


	if (myPet->happy == 0)
	{
		/*happy[0] = '-';
		happy[1] = ' ';
		happy[2] = '-';
		happy[3] = ' ';
		happy[4] = '-';
		happy[5] = ' ';
		happy[6] = '-';
		happy[7] = ' ';
		happy[8] = '-';*/

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_Rectangle HappyBar;

		HappyBar.xMin = 51;
		HappyBar.xMax = 107;
		HappyBar.yMin = 115;
		HappyBar.yMax = 125;

		Graphics_drawRectangle(g_sContext_p, &HappyBar);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_Rectangle Happy1;

		Happy1.xMin = 53;
		Happy1.xMax = 61;
		Happy1.yMin = 116;
		Happy1.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy1);

		Graphics_Rectangle Happy2;

		Happy2.xMin = 64;
		Happy2.xMax = 72;
		Happy2.yMin = 116;
		Happy2.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy2);

		Graphics_Rectangle Happy3;

		Happy3.xMin = 75;
		Happy3.xMax = 83;
		Happy3.yMin = 116;
		Happy3.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy3);

		Graphics_Rectangle Happy4;

		Happy4.xMin = 86;
		Happy4.xMax = 94;
		Happy4.yMin = 116;
		Happy4.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy4);

		Graphics_Rectangle Happy5;

		Happy5.xMin = 97;
		Happy5.xMax = 105;
		Happy5.yMin = 116;
		Happy5.yMax = 124;

		Graphics_fillRectangle(g_sContext_p, &Happy5);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHEAT);
	}

	// these two functions below were used when bonus featuer was not added

	//Graphics_drawString(g_sContext_p, energy, -1, 55, 104, true);
	//Graphics_drawString(g_sContext_p, happy, -1, 55, 118, true);
}

// draws updated baudrate status
void drawBRStatus(Graphics_Context* g_sContext_p, char* string, int BR)
{
	string[0] = BR + 48;
	Graphics_drawString(g_sContext_p, string, -1, 113, 3, true);
}

// this function moves the tamagotchi when it is a child or adult
void moveTamagotchi(Graphics_Context* g_sContext_p, tamagotchi_t* myPet, unsigned  char direction)
{
	if (myPet->stage == child)
	{
		// when tamagotchi is at x_Pos 35, it shouldn't be able to move right again or else hits border
		if (direction == 'r' && myPet->energy != 0 && myPet->x_Pos != 35)
		{
			eraseChild(g_sContext_p, myPet); // first draw original position
			myPet->x_Pos = myPet->x_Pos + 35; // updates position
			drawChild(g_sContext_p, myPet); // reflect the updated position when drawing newly positioned child
		}

		// same here, when tamagotchi is at x_Pos -35, it shouldn't be able to move left again
		if (direction == 'l' && myPet->energy != 0 && myPet->x_Pos != -35)
		{
			eraseChild(g_sContext_p, myPet);
			myPet->x_Pos = myPet->x_Pos - 35;
			drawChild(g_sContext_p, myPet);
		}

		// shouldn't move when y_Pos is at -28
		if (direction == 'u' && myPet->energy != 0 && myPet->y_Pos != -28)
		{
			eraseChild(g_sContext_p, myPet);
			myPet->y_Pos = myPet->y_Pos - 28;
			drawChild(g_sContext_p, myPet);
		}

		// shouldn't move when y_Pos is at 0
		if (direction == 'd' && myPet->energy != 0 && myPet->y_Pos != 0)
		{
			eraseChild(g_sContext_p, myPet);
			myPet->y_Pos = myPet->y_Pos + 28;
			drawChild(g_sContext_p, myPet);
		}

	}

	if (myPet->stage == adult)
	{
		// shouldn't move when x_Pos is at 35
		if (direction == 'r' && myPet->energy != 0 && myPet->x_Pos != 35)
		{
			eraseAdult(g_sContext_p, myPet);
			myPet->x_Pos = myPet->x_Pos + 35;
			drawAdult(g_sContext_p, myPet);
		}

		// shouldn't move when x_Pos is at -35
		if (direction == 'l' && myPet->energy != 0 && myPet->x_Pos != -35)
		{
			eraseAdult(g_sContext_p, myPet);
			myPet->x_Pos = myPet->x_Pos - 35;
			drawAdult(g_sContext_p, myPet);
		}

		// shouldn't move when y_Pos is at -28
		if (direction == 'u' && myPet->energy != 0 && myPet->y_Pos != -28)
		{
			eraseAdult(g_sContext_p, myPet);
			myPet->y_Pos = myPet->y_Pos - 28;
			drawAdult(g_sContext_p, myPet);
		}

		// shouldn't move when y_Pos is at 0
		if (direction == 'd' && myPet->energy != 0 && myPet->y_Pos != 0)
		{
			eraseAdult(g_sContext_p, myPet);
			myPet->y_Pos = myPet->y_Pos + 28;
			drawAdult(g_sContext_p, myPet);
		}

	}

	if (myPet->stage == elder)
	{
		// shouldn't move when x_Pos is at 35
		if (direction == 'r' && myPet->energy != 0 && myPet->x_Pos != 35)
		{
			eraseElder(g_sContext_p, myPet);
			myPet->x_Pos = myPet->x_Pos + 35;
			drawElder(g_sContext_p, myPet);
		}

		// shouldn't move when x_Pos is at -35
		if (direction == 'l' && myPet->energy != 0 && myPet->x_Pos != -35)
		{
			eraseElder(g_sContext_p, myPet);
			myPet->x_Pos = myPet->x_Pos - 35;
			drawElder(g_sContext_p, myPet);
		}

		// shouldn't move when y_Pos is at -28
		if (direction == 'u' && myPet->energy != 0 && myPet->y_Pos != -28)
		{
			eraseElder(g_sContext_p, myPet);
			myPet->y_Pos = myPet->y_Pos - 28;
			drawElder(g_sContext_p, myPet);
		}

		// shouldn't move when y_Pos is at 0
		if (direction == 'd' && myPet->energy != 0 && myPet->y_Pos != 0)
		{
			eraseElder(g_sContext_p, myPet);
			myPet->y_Pos = myPet->y_Pos + 28;
			drawElder(g_sContext_p, myPet);
		}

	}
}

// this bool function is to check if movement is valid (checking if child or adult, if energy is >=2, or crashing into border?)
// the number -28, -35, 35, 28 are keep being used to keep track of the border, which is stored in the tamagotchi struct
bool checkValidMovement(tamagotchi_t* myPet, unsigned char command)
{
	if ((myPet->stage == egg) || (myPet->stage == child) || (myPet->stage == adult) || (myPet->stage == elder))
	{
		if (command == 'a')
		{
			if (!(myPet->x_Pos == -35 && myPet->y_Pos == -28) && !(myPet->x_Pos == -35 && myPet->y_Pos == 0) && (myPet->energy >= 2))
				return true;
			else
				return false;
		}

		if (command == 'd')
		{
			if (!(myPet->x_Pos == 35 && myPet->y_Pos == 0) && !(myPet->x_Pos == 35 && myPet->y_Pos == -28) && (myPet->energy >= 2))
				return true;
			else
				return false;
		}

		if (command == 'w')
		{
			if (!(myPet->x_Pos == 0 && myPet->y_Pos == -28) && !(myPet->x_Pos == -35 && myPet->y_Pos == -28) && !(myPet->x_Pos == 35 && myPet->y_Pos == -28)
				&& (myPet->energy >= 2))
				return true;
			else
				return false;
		}

		if (command == 's')
		{
			if (!(myPet->x_Pos == -35 && myPet->y_Pos == 0) && !(myPet->x_Pos == 0 && myPet->y_Pos == 0) && !(myPet->x_Pos == 35 && myPet->y_Pos == 0)
				&& (myPet->energy >= 2))
				return true;
			else
				return false;
		}
	}

	else
		return false;
}

// this is the for the tamagotchi's life stages
void changingStageFSM(tamagotchi_t* myPet, Graphics_Context* g_sContext_p)
{

	// inputs: age, energy, happy
	// outputs: erase prev state and draw new state or stay in original state

	state_t newState = myPet->stage;

	switch (newState)
	{
	case egg:
		if (myPet->age == 1 && !(myPet->warmth >= 5))
		{
			// changing state
			myPet->stage = child;

			// displaying output: egg becomes child
			eraseEgg(g_sContext_p, myPet);
			drawChild(g_sContext_p, myPet);
		}
		if (myPet->warmth == 5) //this is one of the bonus feature, when warmth is 5 egg hatches
		{
			// changing state
			myPet->stage = child;

			// displaying output: egg becomes child
			eraseEgg(g_sContext_p, myPet);
			drawChild(g_sContext_p, myPet);
		}
		break;

	case child:
		if ((myPet->age >= 8) && (myPet->happy >= 4 && myPet->energy >= 4))
		{
			// changing state
			myPet->stage = adult;

			// displaying output: child becomes adult
			eraseChild(g_sContext_p, myPet);
			drawAdult(g_sContext_p, myPet);
		}

		if (myPet->energy == 0 && myPet->happy == 0)
		{
			myPet->stage = death;
			Graphics_clearDisplay(g_sContext_p);
			drawDeath(g_sContext_p, myPet);
			while (1) // enters infinite while loop if dead
			{

			}
		}

		break;

	case adult:
		if (myPet->age == 13) // bonus feature, tamagotchi turns into elder at age 13 or greater
		{
			myPet->stage = elder;

			// displaying output: child becomes adult
			eraseAdult(g_sContext_p, myPet);
			drawElder(g_sContext_p, myPet);
		}

		if (myPet->energy == 0 && myPet->happy == 0)
		{
			myPet->stage = death; // enters infinite while loop if dead
			Graphics_clearDisplay(g_sContext_p);
			drawDeath(g_sContext_p, myPet);
			while (1)
			{

			}
		}
		break;

	case elder:
		if (myPet->energy == 0 && myPet->happy == 0)
		{
			myPet->stage = death; // enters infinite while loop if dead
			Graphics_clearDisplay(g_sContext_p);
			drawDeath(g_sContext_p, myPet);
			while (1)
			{

			}
		}

	case death: // this state was necessary because when tamagotchi is overfed too much, it dies
		if (myPet->stage == death)
		{
			myPet->stage = death; // enters infinite while loop if dead
			Graphics_clearDisplay(g_sContext_p);
			drawDeath(g_sContext_p, myPet);
			while (1)
			{

			}
		}
	}
}

int main(void)
{
	WDT_A_hold(WDT_A_BASE); // halting watch dog timer

	// declaring and initializing UART configuration using library configs and function
	eUSCI_UART_Config uartConfig = {
		 EUSCI_A_UART_CLOCKSOURCE_SMCLK,               // SMCLK Clock Source = 48MHz
		 312,                                           // UCBR   = 312
		 8,                                            // UCBRF  = 8
		 0x55,                                         // UCBRS  = 0x55
		 EUSCI_A_UART_NO_PARITY,                       // No Parity
		 EUSCI_A_UART_LSB_FIRST,                       // LSB First
		 EUSCI_A_UART_ONE_STOP_BIT,                    // One stop bit
		 EUSCI_A_UART_MODE,                            // UART mode
		 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
	};

	InitUART(EUSCI_A0_BASE, &uartConfig, GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3);

	// initialize graphics
	Graphics_Context g_sContext;

	initialize();
	InitGraphics(&g_sContext);

	// initialize my tamagotchi called Mike
	// notice we start out in the egg stage, 5 energy, 5 happy, 0 age, 0 in both x-y positions, 0 moveCounts, and 0 warmth
	tamagotchi_t Mike = { egg, 5, 5, 0, 0, 0, 0, 0 };
	drawBorders(&g_sContext);
	drawEgg(&g_sContext, &Mike);

	// all four strings to display necessary status of this game

	//char energyStrg[11] = "* * * * * "; // this string value has been replaced with graphics (bonus feature)
	//char happyStrg[11] = "* * * * * "; // this string value has been replaced with graphics (bonus feature)

	char ageStrg[9] = "0     "; // there are spaces after the 0 so that it fills out the screen (max age is 999,999 for this program)
	char brStrg[2] = "0";
	int index_BR = 0;

	Graphics_setFont(&g_sContext, &g_sFontCmss14b);
	Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHEAT);
	Graphics_drawString(&g_sContext, "Energy:", -1, 3, 99, true);
	Graphics_drawString(&g_sContext, "Happy:", -1, 3, 113, true);
	Graphics_drawString(&g_sContext, "Age:", -1, 3, 3, true);
	Graphics_drawString(&g_sContext, "BR:", -1, 88, 3, true);

	Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHEAT);

	// these two drawString functions were used before the bonus feature was added
	//Graphics_drawString(&g_sContext, &energyStrg, -1, 55, 104, true);
	//Graphics_drawString(&g_sContext, &happyStrg, -1, 55, 118, true);

	Graphics_drawString(&g_sContext, &ageStrg, -1, 36, 3, true);
	Graphics_drawString(&g_sContext, &brStrg, -1, 113, 3, true);

	// starting timer before entering loop
	Timer32_setCount(TIMER32_0_BASE, TEN_SECONDS_COUNT);
	Timer32_setCount(TIMER32_1_BASE, FIVE_SECONDS_COUNT);

	Timer32_startTimer(TIMER32_0_BASE, true);
	Timer32_startTimer(TIMER32_1_BASE, true);

	while (1)
	{
		// making sure these values start as a fixed value indicating nothing happened

		uint8_t rChar = 'X';
		uint8_t tChar = 'X';
		uint8_t mChar = 'X';

		if (UARTHasChar(EUSCI_A0_BASE)) // check if UART received data
		{
			rChar = UARTGetChar(EUSCI_A0_BASE);

			if (rChar == 'f' || rChar == 'F') // check if f is received
				tChar = 'f';

			// since w,s,d,a are movement keys, check if movement is valid at current state
			else if (rChar == 'w' || rChar == 'W')
			{
				if (checkValidMovement(&Mike, rChar))
				{
					tChar = 'm';
					mChar = 'u';
					Mike.warmth++;
					if (Mike.warmth >= 6)
						Mike.warmth = 6; // in case warmth value overflows, keep warmth value at 6
				}
			}

			else if (rChar == 's' || rChar == 'S')
			{
				if (checkValidMovement(&Mike, rChar))
				{
					tChar = 'm';
					mChar = 'd';
				}
			}

			else if (rChar == 'a' || rChar == 'A')
			{
				if (checkValidMovement(&Mike, rChar))
				{
					tChar = 'm';
					mChar = 'l';
				}
			}

			else if (rChar == 'd' || rChar == 'D')
			{
				if (checkValidMovement(&Mike, rChar))
				{
					tChar = 'm';
					mChar = 'r';
				}
			}

			else if (rChar == 'r' || rChar == 'R')
			{
				index_BR++; // incrementing baudrate index
				tChar = 'B';
				mChar = 'X'; // making sure 'r' or 'R' is not conisdered a movement
				switch (index_BR)
				{
				case 0:
					UARTSetBaud(EUSCI_A0_BASE,
						&uartConfig,
						baud9600);
					break;

				case 1:
					UARTSetBaud(EUSCI_A0_BASE,
						&uartConfig,
						baud19200);
					break;

				case 2:
					UARTSetBaud(EUSCI_A0_BASE,
						&uartConfig,
						baud38400);
					break;
				case 3:
					UARTSetBaud(EUSCI_A0_BASE,
						&uartConfig,
						baud57600);
					break;
				case 4:
					index_BR = 0; // reseting index for baudrate to go back to 9600
					UARTSetBaud(EUSCI_A0_BASE,
						&uartConfig,
						baud9600);
					break;
				}
			}

			else
			{
				mChar = 'X';
				tChar = 'X';
			}

			if (UARTCanSend(EUSCI_A0_BASE))
				UARTPutChar(EUSCI_A0_BASE, tChar);
		}

		// update energy, happy, age data (checks if timer is expired)
		updateEnergyHappyStatus(&Mike, tChar);
		updateAgeStatus(&Mike);

		// once all data is updated using timers, start the timers again if expired
		if ((Timer32_getValue(TIMER32_1_BASE) == EXPIRED))
		{
			Timer32_setCount(TIMER32_1_BASE, FIVE_SECONDS_COUNT);
			Timer32_startTimer(TIMER32_1_BASE, true);
		}

		if ((Timer32_getValue(TIMER32_0_BASE) == EXPIRED))
		{
			Timer32_setCount(TIMER32_0_BASE, TEN_SECONDS_COUNT);
			Timer32_startTimer(TIMER32_0_BASE, true);
		}

		Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHEAT); // making sure string colors are consistent
		drawEnergyHappyStatus(&Mike, &g_sContext); // drawing energy and happy status using graphics (colored rectangles)
		makeAgeString(&Mike, &ageStrg); // making age string
		drawAgeStatus(&g_sContext, &ageStrg); // drawing age status
		drawBRStatus(&g_sContext, &brStrg, index_BR); // drawing current buadrate (0,1,2,3)
		moveTamagotchi(&g_sContext, &Mike, mChar); // moves tamagotchi according to the UART received character and requirements
		changingStageFSM(&Mike, &g_sContext); // enters this FSM to check state of tamagotchi
	}
}

// initializing fonts
void InitFonts() {
	Crystalfontz128x128_Init();
	Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
}

// initializing graphics including font, foreground, background

void InitGraphics(Graphics_Context* g_sContext_p) {

	Graphics_initContext(g_sContext_p,
		&g_sCrystalfontz128x128,
		&g_sCrystalfontz128x128_funcs);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
	Graphics_setFont(g_sContext_p, &g_sFontCmss14b);

	InitFonts();

	Graphics_clearDisplay(g_sContext_p);
}

void initialize()
{
	// stop the watchdog timer
	WDT_A_hold(WDT_A_BASE);

	// initialize the boosterPack LEDs and turn them off except for red LED
	initialize_BoosterpackLED_red();
	initialize_BoosterpackLED_green();
	initialize_BoosterpackLED_blue();
	turnOn_BoosterpackLED_red();
	turnOff_BoosterpackLED_green();
	turnOff_BoosterpackLED_blue();

	// initialize the Launchpad buttons
	initialize_LaunchpadLeftButton();
	initialize_LaunchpadRightButton();


	// Initialize the timers needed for debouncing
	Timer32_initModule(TIMER32_0_BASE, // There are two timers, we are using the one with the index 0
		TIMER32_PRESCALER_1, // The prescaler value is 1; The clock is not divided before feeding the counter
		TIMER32_32BIT, // The counter is used in 32-bit mode; the alternative is 16-bit mode
		TIMER32_PERIODIC_MODE); //This options is irrelevant for a one-shot timer

	Timer32_initModule(TIMER32_1_BASE, // There are two timers, we are using the one with the index 1
		TIMER32_PRESCALER_1, // The prescaler value is 1; The clock is not divided before feeding the counter
		TIMER32_32BIT, // The counter is used in 32-bit mode; the alternative is 16-bit mode
		TIMER32_PERIODIC_MODE); //This options is irrelevant for a one-shot timer


}