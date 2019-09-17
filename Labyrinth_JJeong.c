// Labyrinth game by JJeong
// Note : Header files are missing due to copyright issues

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "ButtonLED_HAL.h"
#include "graphics_HAL.h"
#include "ADC_HAL.h"
#include "labyrinth.h"

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "HAL_I2C.h"
#include "HAL_OPT3001.h"
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"

extern HWTimer_t timer0, timer1;

#define EXIST 1
#define NOTEXIST 0

#define TARGET_X_AXIS 0
#define TARGET_Y_AXIS 7
#define NUM_CELLS_PER_ROW 8
#define TARGET_LENGTH 4
#define TARGET_ALLIGN 3

#define SCREEN_MIN 0
#define SCREEN_MAX 128
#define X_SPEED_LOCATION 2
#define Y_SPEED_LOCATION 11
#define NUMBER_LOCATION 23
#define CENTER_LOCATION 50
#define VERTICES_MARGIN 2

#define TWENTYMS 20000
#define ONESEC 1000000
#define FIVE_D_RIGHT 7800
#define FIFTEEN_D_RIGHT 8200
#define FIVE_D_LEFT 6970
#define FIFTEEN_D_LEFT 6400
#define FIVE_D_UP 7800
#define FIFTEEN_D_UP 8200
#define FIVE_D_DOWN 6800
#define FIFTEEN_D_DOWN 6300

#define NIGHT_MODE_LUX 30

float lux;

/* Timer_A Up Configuration Parameter */
const Timer_A_UpModeConfig upConfig =
{
		TIMER_A_CLOCKSOURCE_ACLK,               // ACLK Clock SOurce
		TIMER_A_CLOCKSOURCE_DIVIDER_1,          // ACLK/1 = 3MHz
		200,                                    // 200 tick period
		TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
		TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disable CCR0 interrupt
		TIMER_A_DO_CLEAR                        // Clear value
};

/* Timer_A Compare Configuration Parameter  (PWM) */
Timer_A_CompareModeConfig compareConfig_PWM =
{
		TIMER_A_CAPTURECOMPARE_REGISTER_3,          // Use CCR3
		TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
		TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		100                                         // 50% Duty Cycle
};

// this struct is to store the current marble coordinates to test
typedef struct {
	int x;
	int y;
} marbleCoord_t;

// this stores the data of the virtual vertices created on the marble
typedef struct {
	marbleCoord_t LeftTop;
	marbleCoord_t RightTop;
	marbleCoord_t LeftBottom;
	marbleCoord_t RightBottom;
} marbleVertices_t;

/* ADC results buffer */

// This function initializes all the peripherals except graphics
void initialize();

void makeString(unsigned int min, unsigned int sec, int unsigned tenth, char* string)
{
	unsigned int min_1 = 0;
	unsigned int min_2 = 0;

	unsigned int sec_1 = 0;
	unsigned int sec_2 = 0;

	// this function is utilizing the fact that in ASCII, character '0' is 48 in integer.

	// if min or sec is equal or greater than 10, need to break up the number to convert to string
	// if less than 10, simply add 48 to the existing integer, the tenth value never goes over 9
	if (min >= 10)
	{
		min_1 = min / 10;
		min_2 = min % 10;
		string[0] = min_1 + 48;
		string[1] = min_2 + 48;
	}

	if (min < 10)
	{
		string[0] = min_1 + 48;
		string[1] = min + 48;
	}

	if (sec >= 10)
	{
		sec_1 = sec / 10;
		sec_2 = sec % 10;
		string[3] = sec_1 + 48;
		string[4] = sec_2 + 48;
	}

	if (sec < 10)
	{
		string[3] = sec_1 + 48;
		string[4] = sec + 48;
	}

	string[6] = tenth + 48;
}


// draws basic background
void drawBackground(Graphics_Context* g_sContext_p)
{
	Graphics_Rectangle background;

	background.xMin = SCREEN_MIN;
	background.xMax = SCREEN_MAX;
	background.yMin = SCREEN_MIN;
	background.yMax = SCREEN_MAX;
	Graphics_fillRectangle(g_sContext_p, &background);

	if (lux > NIGHT_MODE_LUX)
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
	if (lux < NIGHT_MODE_LUX)
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

	Graphics_drawString(g_sContext_p, "Vx:", -1, CENTER_LOCATION, X_SPEED_LOCATION, false);
	Graphics_drawString(g_sContext_p, "Vy:", -1, CENTER_LOCATION, Y_SPEED_LOCATION, false);
}

// initializes the labyrinth by choosing which walls to exist or not
void initLabyrinth(labyrinth_t* myLabyrinth)
{
	int i;

	for (i = 0; i < MAX_H_WALLS; i++)
	{
		myLabyrinth->h_walls[i].valid = NOTEXIST;
	}

	for (i = 0; i < MAX_V_WALLS; i++)
	{
		myLabyrinth->v_walls[i].valid = NOTEXIST;
	}

	for (i = 0; i < NUM_CELLS; i++)
	{
		myLabyrinth->holes[i].valid = NOTEXIST;
	}

	for (i = 0; i < NUM_CELLS; i++)
	{
		myLabyrinth->wormholes[i].valid = NOTEXIST;
	}

	myLabyrinth->h_walls[0].valid = EXIST;
	myLabyrinth->h_walls[1].valid = EXIST;
	myLabyrinth->h_walls[2].valid = EXIST;
	myLabyrinth->h_walls[3].valid = EXIST;
	myLabyrinth->h_walls[8].valid = EXIST;
	myLabyrinth->h_walls[9].valid = EXIST;
	myLabyrinth->h_walls[11].valid = EXIST;
	myLabyrinth->h_walls[15].valid = EXIST;
	myLabyrinth->h_walls[16].valid = EXIST;
	myLabyrinth->h_walls[17].valid = EXIST;
	myLabyrinth->h_walls[18].valid = EXIST;
	myLabyrinth->h_walls[19].valid = EXIST;

	myLabyrinth->v_walls[0].valid = EXIST;
	myLabyrinth->v_walls[1].valid = EXIST;
	myLabyrinth->v_walls[2].valid = EXIST;
	myLabyrinth->v_walls[3].valid = EXIST;
	myLabyrinth->v_walls[5].valid = EXIST;
	myLabyrinth->v_walls[6].valid = EXIST;
	myLabyrinth->v_walls[9].valid = EXIST;
	myLabyrinth->v_walls[11].valid = EXIST;
	myLabyrinth->v_walls[13].valid = EXIST;
	myLabyrinth->v_walls[16].valid = EXIST;
	myLabyrinth->v_walls[17].valid = EXIST;
	myLabyrinth->v_walls[18].valid = EXIST;
	myLabyrinth->v_walls[19].valid = EXIST;

	myLabyrinth->holes[6].valid = EXIST;
	myLabyrinth->holes[11].valid = EXIST;
	myLabyrinth->holes[24].valid = EXIST;
	myLabyrinth->holes[31].valid = EXIST;
	myLabyrinth->holes[32].valid = EXIST;
	myLabyrinth->holes[39].valid = EXIST;
	myLabyrinth->holes[59].valid = EXIST;
	myLabyrinth->holes[61].valid = EXIST;

	myLabyrinth->wormholes[58].valid = EXIST;
	myLabyrinth->wormholes[28].valid = EXIST;
	myLabyrinth->wormholes[60].valid = EXIST;

	myLabyrinth->targetCell.valid = EXIST;
	myLabyrinth->targetCell.x_cell_index = TARGET_X_AXIS;
	myLabyrinth->targetCell.y_cell_index = TARGET_Y_AXIS;

	// calculations for the x and y coordinates for all walls and holes
	// were done by ackonwledging the block size and cell size

	for (i = 0; i < MAX_H_WALLS; i++)
	{
		if (myLabyrinth->h_walls[i].valid == EXIST)
		{
			if (i <= (WALLS_PER_AXIS - 1))
			{
				myLabyrinth->h_walls[i].x1 = LAB_MIN_X + (i * BLOCK_SIZE);
				myLabyrinth->h_walls[i].x2 = myLabyrinth->h_walls[i].x1 + BLOCK_SIZE;
				myLabyrinth->h_walls[i].y = LAB_MIN_Y;
			}

			if (i > (WALLS_PER_AXIS - 1))
			{
				myLabyrinth->h_walls[i].x1 = LAB_MIN_X + ((i % WALLS_PER_AXIS) * BLOCK_SIZE);
				myLabyrinth->h_walls[i].x2 = myLabyrinth->h_walls[i].x1 + BLOCK_SIZE;
				myLabyrinth->h_walls[i].y = LAB_MIN_Y + ((i / WALLS_PER_AXIS) * BLOCK_SIZE);
			}
		}

	}

	for (i = 0; i < MAX_V_WALLS; i++)
	{
		if (myLabyrinth->v_walls[i].valid == EXIST)
		{
			if (i <= (WALLS_PER_AXIS - 1))
			{
				myLabyrinth->v_walls[i].y1 = LAB_MIN_Y + (i * BLOCK_SIZE);
				myLabyrinth->v_walls[i].y2 = myLabyrinth->v_walls[i].y1 + BLOCK_SIZE;
				myLabyrinth->v_walls[i].x = LAB_MIN_X;
			}

			if (i >= (WALLS_PER_AXIS - 1))
			{
				myLabyrinth->v_walls[i].y1 = LAB_MIN_Y + ((i % WALLS_PER_AXIS) * BLOCK_SIZE);
				myLabyrinth->v_walls[i].y2 = myLabyrinth->v_walls[i].y1 + BLOCK_SIZE;
				myLabyrinth->v_walls[i].x = LAB_MIN_X + (i / WALLS_PER_AXIS) * BLOCK_SIZE;
			}
		}

	}

	for (i = 0; i < NUM_CELLS; i++)
	{
		if (myLabyrinth->holes[i].valid == EXIST)
		{
			if (i <= (NUM_CELLS_PER_ROW - 1))
			{
				myLabyrinth->holes[i].x = HALF_CELL + LAB_MIN_X + (i * CELL_SIZE);
				myLabyrinth->holes[i].y = HALF_CELL + LAB_MIN_Y;
			}
			if (i > (NUM_CELLS_PER_ROW - 1))
			{
				myLabyrinth->holes[i].x = HALF_CELL + LAB_MIN_X + ((i % NUM_CELLS_PER_ROW) * CELL_SIZE);
				myLabyrinth->holes[i].y = HALF_CELL + LAB_MIN_Y + ((i / NUM_CELLS_PER_ROW) * CELL_SIZE);
			}
		}
	}

	for (i = 0; i < NUM_CELLS; i++)
	{
		if (myLabyrinth->wormholes[i].valid == EXIST)
		{
			if (i <= (NUM_CELLS_PER_ROW - 1))
			{
				myLabyrinth->wormholes[i].x = HALF_CELL + LAB_MIN_X + (i * CELL_SIZE);
				myLabyrinth->wormholes[i].y = HALF_CELL + LAB_MIN_Y;
			}
			if (i > (NUM_CELLS_PER_ROW - 1))
			{
				myLabyrinth->wormholes[i].x = HALF_CELL + LAB_MIN_X + ((i % NUM_CELLS_PER_ROW) * CELL_SIZE);
				myLabyrinth->wormholes[i].y = HALF_CELL + LAB_MIN_Y + ((i / NUM_CELLS_PER_ROW) * CELL_SIZE);
			}
		}
	}

}

// this function draws all the valid walls and holes
void drawMaze(Graphics_Context* g_sContext_p, labyrinth_t* myLabyrinth)
{
	int i;
	for (i = 0; i < MAX_H_WALLS; i++)
	{
		if (myLabyrinth->h_walls[i].valid == EXIST)
		{
			if (lux > NIGHT_MODE_LUX)
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
			if (lux < NIGHT_MODE_LUX)
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

			Graphics_drawLineH(g_sContext_p, myLabyrinth->h_walls[i].x1, myLabyrinth->h_walls[i].x2, myLabyrinth->h_walls[i].y);
		}
	}

	for (i = 0; i < MAX_V_WALLS; i++)
	{
		if (myLabyrinth->v_walls[i].valid == EXIST)
		{
			if (lux > NIGHT_MODE_LUX)
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
			if (lux < NIGHT_MODE_LUX)
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

			Graphics_drawLineV(g_sContext_p, myLabyrinth->v_walls[i].x, myLabyrinth->v_walls[i].y1, myLabyrinth->v_walls[i].y2);
		}
	}

	for (i = 0; i < NUM_CELLS; i++)
	{
		if (myLabyrinth->holes[i].valid == EXIST)
		{
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
			Graphics_fillCircle(g_sContext_p, myLabyrinth->holes[i].x, myLabyrinth->holes[i].y, HOLE_RADIUS);
		}
	}

	for (i = 0; i < NUM_CELLS; i++)
	{
		if (myLabyrinth->wormholes[i].valid == EXIST)
		{
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_MAGENTA);
			Graphics_fillCircle(g_sContext_p, myLabyrinth->wormholes[i].x, myLabyrinth->wormholes[i].y, HOLE_RADIUS);
		}
	}

	if (myLabyrinth->targetCell.valid == EXIST)
	{
		Graphics_Rectangle targetCell;

		targetCell.xMin = CELL_SIZE * (myLabyrinth->targetCell.x_cell_index) + LAB_MIN_X + TARGET_ALLIGN;
		targetCell.xMax = targetCell.xMin + TARGET_LENGTH;
		targetCell.yMin = CELL_SIZE * (myLabyrinth->targetCell.y_cell_index) + LAB_MIN_Y + TARGET_ALLIGN;
		targetCell.yMax = targetCell.yMin + TARGET_LENGTH;

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
		Graphics_fillRectangle(g_sContext_p, &targetCell);
	}
}

// the below two function simply draws and erases the marble to make the marble move
void drawMarble(Graphics_Context* g_sContext_p, marble_t* myMarble)
{
	Graphics_fillCircle(g_sContext_p, myMarble->x, myMarble->y, MARBLE_RADIUS);
}

void eraseMarble(Graphics_Context* g_sContext_p, marble_t* myMarble)
{
	Graphics_fillCircle(g_sContext_p, myMarble->x, myMarble->y, MARBLE_RADIUS);
}

// this draws the labyrinth, but notice how depending on the lux value the colors change
void drawLabyrinth(Graphics_Context* g_sContext_p, labyrinth_t* myLabyrinth, marble_t* myMarble, bool* isNight)
{

	if (lux > NIGHT_MODE_LUX && *isNight == false)
	{
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		drawBackground(g_sContext_p);
		drawMaze(g_sContext_p, myLabyrinth);
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
		drawMarble(g_sContext_p, myMarble);
		*isNight = true;
	}

	if (lux < NIGHT_MODE_LUX && *isNight == true)
	{
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);
		drawBackground(g_sContext_p);
		drawMaze(g_sContext_p, myLabyrinth);
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_ORANGE);
		drawMarble(g_sContext_p, myMarble);
		*isNight = false;
	}
}

// this gets the marble coordinate for future testing
marbleCoord_t getMarbleCoord(Graphics_Context* g_sContext_p, uint16_t* resultsBuffer)
{
	int x = 0;
	int y = 0;

	//up
	if (resultsBuffer[1] > FIVE_D_UP)
	{
		if (resultsBuffer[1] > FIFTEEN_D_UP)
		{
			y = -2;
		}
		else
		{
			y = -1;
		}
	}

	//down
	if (resultsBuffer[1] < FIVE_D_DOWN)
	{
		if (resultsBuffer[1] < FIFTEEN_D_DOWN)
		{
			y = 2;
		}
		else
		{
			y = 1;
		}
	}


	//right
	if (resultsBuffer[0] > FIVE_D_RIGHT)
	{
		if (resultsBuffer[0] > FIFTEEN_D_RIGHT)
		{
			x = 2;
		}
		else
		{
			x = 1;
		}
	}

	//left
	if (resultsBuffer[0] < FIVE_D_LEFT)
	{
		if (resultsBuffer[0] < FIFTEEN_D_LEFT)
		{
			x = -2;
		}
		else
		{
			x = -1;
		}
	}

	drawSpeed(g_sContext_p, x, y);
	marbleCoord_t coord = { x, y };

	return coord;
}

// this draws the string to indicate the current speed
void drawSpeed(Graphics_Context* g_sContext_p, int x, int y)
{
	Graphics_Rectangle textboxX;

	textboxX.xMin = 73;
	textboxX.xMax = 83;
	textboxX.yMin = 3;
	textboxX.yMax = 10;

	Graphics_Rectangle textboxY;

	textboxY.xMin = 73;
	textboxY.xMax = 83;
	textboxY.yMin = 12;
	textboxY.yMax = 19;

	if (x == 1 || x == -1)
	{
		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

		Graphics_fillRectangle(g_sContext_p, &textboxX);

		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

		Graphics_drawString(g_sContext_p, "1", -1, CENTER_LOCATION + NUMBER_LOCATION, X_SPEED_LOCATION, false);
	}
	else if (x == 2 || x == -2)
	{
		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

		Graphics_fillRectangle(g_sContext_p, &textboxX);

		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

		Graphics_drawString(g_sContext_p, "2", -1, CENTER_LOCATION + NUMBER_LOCATION, X_SPEED_LOCATION, false);
	}
	else
	{
		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

		Graphics_fillRectangle(g_sContext_p, &textboxX);

		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

		Graphics_drawString(g_sContext_p, "0", -1, CENTER_LOCATION + NUMBER_LOCATION, X_SPEED_LOCATION, false);
	}

	if (y == 1 || y == -1)
	{
		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

		Graphics_fillRectangle(g_sContext_p, &textboxY);

		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

		Graphics_drawString(g_sContext_p, "1", -1, CENTER_LOCATION + NUMBER_LOCATION, Y_SPEED_LOCATION, false);
	}
	else if (y == 2 || y == -2)
	{
		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

		Graphics_fillRectangle(g_sContext_p, &textboxY);

		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

		Graphics_drawString(g_sContext_p, "2", -1, CENTER_LOCATION + NUMBER_LOCATION, Y_SPEED_LOCATION, false);
	}
	else
	{
		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

		Graphics_fillRectangle(g_sContext_p, &textboxY);

		if (lux > NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
		if (lux < NIGHT_MODE_LUX)
			Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_CRIMSON);

		Graphics_drawString(g_sContext_p, "0", -1, CENTER_LOCATION + NUMBER_LOCATION, Y_SPEED_LOCATION, false);
	}
}

// this function moves the marble
void moveMarble(Graphics_Context* g_sContext_p, marble_t* myMarble, uint16_t* resultsBuffer)
{
	marbleCoord_t newCoord = getMarbleCoord(g_sContext_p, resultsBuffer);

	myMarble->y += newCoord.y;
	myMarble->x += newCoord.x;
}

// this function helpes to constantly get the data of the marble vertices
marbleVertices_t getMarbleVertices(marble_t* myMarble)
{
	marbleVertices_t vertices;
	vertices.LeftTop.x = myMarble->x - VERTICES_MARGIN;
	vertices.LeftTop.y = myMarble->y - VERTICES_MARGIN;
	vertices.RightTop.x = myMarble->x + VERTICES_MARGIN;
	vertices.RightTop.y = myMarble->y - VERTICES_MARGIN;
	vertices.LeftBottom.x = myMarble->x - VERTICES_MARGIN;
	vertices.LeftBottom.y = myMarble->y + VERTICES_MARGIN;
	vertices.RightBottom.x = myMarble->x + VERTICES_MARGIN;
	vertices.RightBottom.y = myMarble->y + VERTICES_MARGIN;

	return vertices;
}

// this test if the horizontal walls are inside the vertices to determine to help determine to move the actual marble or not
bool isVerticalWallinMarble(marbleVertices_t marbleVertices, v_wall_t vWall)
{
	if (marbleVertices.LeftTop.x <= vWall.x && marbleVertices.RightTop.x >= vWall.x)
	{
		if ((marbleVertices.RightTop.y >= vWall.y1 && marbleVertices.RightTop.y <= vWall.y2)
			|| (marbleVertices.RightBottom.y >= vWall.y1 && marbleVertices.RightBottom.y <= vWall.y2))
		{
			return true;
		}
	}

	if (marbleVertices.LeftTop.x >= vWall.x && marbleVertices.RightTop.x <= vWall.x)
	{
		if ((marbleVertices.RightTop.y >= vWall.y1 && marbleVertices.RightTop.y <= vWall.y2)
			|| (marbleVertices.RightBottom.y >= vWall.y1 && marbleVertices.RightBottom.y <= vWall.y2))
		{
			return true;
		}
	}

	return false;
}

// this test if the vertical walls are inside the vertices to determine to help determine to move the actual marble or not
bool isHorizontalWallinMarble(marbleVertices_t marbleVertices, h_wall_t hWall)
{
	if (marbleVertices.LeftTop.y <= hWall.y && marbleVertices.LeftBottom.y >= hWall.y)
	{
		if ((marbleVertices.LeftBottom.x >= hWall.x1 && marbleVertices.LeftBottom.x <= hWall.x2)
			|| (marbleVertices.RightBottom.x >= hWall.x1 && marbleVertices.RightBottom.x <= hWall.x2))
		{
			return true;
		}
	}

	if (marbleVertices.LeftTop.y >= hWall.y && marbleVertices.LeftBottom.y <= hWall.y)
	{
		if ((marbleVertices.LeftBottom.x >= hWall.x1 && marbleVertices.LeftBottom.x <= hWall.x2)
			|| (marbleVertices.RightBottom.x >= hWall.x1 && marbleVertices.RightBottom.x <= hWall.x2))
		{
			return true;
		}
	}

	return false;
}

// now implementing the functions of getting the marble vertices, testing whether the vertices are touching the walls
// this function determines if there is a wall on the way of the marble
bool isWallinMarble(labyrinth_t* myLabyrinth, marble_t* myMarble)
{
	marbleVertices_t marbleVertices = getMarbleVertices(myMarble);
	int i;

	for (i = 0; i < MAX_H_WALLS; i++)
	{
		if (myLabyrinth->h_walls[i].valid == EXIST)
		{
			if (isHorizontalWallinMarble(marbleVertices, myLabyrinth->h_walls[i]))
				return true;
		}
	}

	for (i = 0; i < MAX_V_WALLS; i++)
	{
		if (myLabyrinth->v_walls[i].valid == EXIST)
		{
			if (isVerticalWallinMarble(marbleVertices, myLabyrinth->v_walls[i]))
				return true;
		}
	}

	return false;
}

// this function checks if the marble has reached the hole
bool checkHoles(labyrinth_t* myLabyrinth, marble_t* myMarble)
{
	int i;
	for (i = 0; i < NUM_CELLS; i++)
		if (myLabyrinth->holes[i].valid == EXIST)
			if (myMarble->x - HOLE_MARGIN <= myLabyrinth->holes[i].x && myMarble->x + HOLE_MARGIN >= myLabyrinth->holes[i].x)
				if (myMarble->y - HOLE_MARGIN <= myLabyrinth->holes[i].y && myMarble->y + HOLE_MARGIN >= myLabyrinth->holes[i].y)
					return true;

	return false;
}

bool checkWormHoles(labyrinth_t* myLabyrinth, marble_t* myMarble)
{
	int i;
	for (i = 0; i < NUM_CELLS; i++)
		if (myLabyrinth->wormholes[i].valid == EXIST)
			if (myMarble->x - HOLE_MARGIN <= myLabyrinth->wormholes[i].x && myMarble->x + HOLE_MARGIN >= myLabyrinth->wormholes[i].x)
				if (myMarble->y - HOLE_MARGIN <= myLabyrinth->wormholes[i].y && myMarble->y + HOLE_MARGIN >= myLabyrinth->wormholes[i].y)
					return true;

	return false;
}

// this function checks if the marble has reached the target
bool checkTarget(labyrinth_t* myLabyrinth, marble_t* myMarble)
{
	int TARGET_X_CENTER = CELL_SIZE * (myLabyrinth->targetCell.x_cell_index) + LAB_MIN_X + TARGET_ALLIGN + MARBLE_RADIUS;
	int TARGET_Y_CENTER = CELL_SIZE * (myLabyrinth->targetCell.y_cell_index) + LAB_MIN_Y + TARGET_ALLIGN + MARBLE_RADIUS;

	if (myLabyrinth->targetCell.valid == EXIST)
		if (myMarble->x - TARGET_MARGIN <= TARGET_X_CENTER && myMarble->x + TARGET_MARGIN >= TARGET_X_CENTER)
			if (myMarble->y - TARGET_MARGIN <= TARGET_Y_CENTER && myMarble->y + TARGET_MARGIN >= TARGET_Y_CENTER)
				return true;

	return false;
}

// this function draws the game over string if it reaches the hole
// turns on the red boosterpack LED
void drawGameOverHole(Graphics_Context* g_sContext_p, char* timeString)
{
	Graphics_Rectangle textbox;

	textbox.xMin = 18;
	textbox.xMax = 112;
	textbox.yMin = 50;
	textbox.yMax = 90;

	if (lux > NIGHT_MODE_LUX)
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
	if (lux < NIGHT_MODE_LUX)
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

	Graphics_fillRectangle(g_sContext_p, &textbox);

	Graphics_setFont(g_sContext_p, &g_sFontCmss16b);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_drawString(g_sContext_p, "GAME OVER", -1, 20, 55, false);


	if (lux > NIGHT_MODE_LUX)
		Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
	if (lux < NIGHT_MODE_LUX)
		Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

	Graphics_drawString(g_sContext_p, timeString, -1, 44, 70, true);

	turnOn_BoosterpackLED_red();

	while (1)
	{

	}
}

// this function draws the game over string if it reaches the target
// turns on the green boosterpack LED
void drawGameOverTarget(Graphics_Context* g_sContext_p, char* timeString)
{
	Graphics_Rectangle textbox;

	textbox.xMin = 18;
	textbox.xMax = 112;
	textbox.yMin = 50;
	textbox.yMax = 70;

	if (lux > NIGHT_MODE_LUX)
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
	if (lux < NIGHT_MODE_LUX)
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

	Graphics_fillRectangle(g_sContext_p, &textbox);

	Graphics_setFont(g_sContext_p, &g_sFontCmss16b);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_drawString(g_sContext_p, "GAME OVER", -1, 20, 55, false);

	if (lux > NIGHT_MODE_LUX)
		Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_TAN);
	if (lux < NIGHT_MODE_LUX)
		Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN_YELLOW);

	Graphics_drawString(g_sContext_p, timeString, -1, 44, 70, true);

	turnOn_BoosterpackLED_green();
	while (1)
	{

	}
}

int main(void)
{
	MAP_Interrupt_disableMaster();

	/* Set the core voltage level to VCORE1 */
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

	/* Set 2 flash wait states for Flash bank 0 and 1*/
	MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
	MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

	/* Initializes Clock System */
	MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
	MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

	uint16_t resultsBuffer[3];
	OneShotSWTimer_t twentyMS;
	OneShotSWTimer_t hundredMS;
	labyrinth_t myLabyrinth;
	marble_t myMarble = { EXIST, LAB_MIN_X + HALF_CELL, LAB_MIN_Y + HALF_CELL };
	marble_t testMarble = { EXIST, LAB_MIN_X + HALF_CELL, LAB_MIN_Y + HALF_CELL };
	Graphics_Context g_sContext;

	/* Configures P2.6 to PM_TA0.3 for using Timer PWM to control LCD backlight */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6,
		GPIO_PRIMARY_MODULE_FUNCTION);

	/* Configuring Timer_A0 for Up Mode and starting */
	MAP_Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig);
	MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

	/* Initialize compare registers to generate PWM */
	MAP_Timer_A_initCompare(TIMER_A0_BASE, &compareConfig_PWM);

	/* Initialize I2C communication */
	Init_I2C_GPIO();
	I2C_init();

	/* Initialize OPT3001 digital ambient light sensor */
	OPT3001_init();

	__delay_cycles(100000);

	// the below lines intilizees everything to before we start the game
	InitGraphics(&g_sContext);
	initialize();
	initLabyrinth(&myLabyrinth);

	// this timer is set to be 20ms, so the marble moves every 20ms
	InitOneShotSWTimer(&twentyMS, &timer0, TWENTYMS);
	StartOneShotSWTimer(&twentyMS);

	InitOneShotSWTimer(&hundredMS, &timer0, ONESEC);
	StartOneShotSWTimer(&hundredMS);

	unsigned int min = 0;
	unsigned int sec = 0;
	unsigned int tenth = 0;

	char timeString[8] = "00:00";

	// this boolean indicates the mode of the game and helps
	// to prevent from flickering
	bool isNight = false;

	while (1)
	{

		// this displays the time in real time until the game is over
		if (OneShotSWTimerExpired(&hundredMS))
		{
			// keep incrementing tenth until tenth is 10
			tenth++;
			if (tenth == 10) // if tenth is 10, reset to 0 and increment sec
			{
				tenth = 0;
				sec++;
				if (sec == 60) // if sec is 60, reset to 0 and increment min
				{
					min++;
					sec = 0;
				}
				if (min == 60) // if min is 60, reset stopwatch to 0
				{
					min = 0;
					sec = 0;
					tenth = 0;
				}

				makeString(min, sec, tenth, &timeString);
				Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
				Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
				Graphics_drawString(&g_sContext, &timeString, -1, 3, 110, true);
				StartOneShotSWTimer(&hundredMS);
			}
		}


		if (OneShotSWTimerExpired(&twentyMS))
		{
			lux = OPT3001_getLux(); // this lux value is constantly receiving the lux data to determine to switch modes

			getSampleAccelerometer(resultsBuffer);
			drawLabyrinth(&g_sContext, &myLabyrinth, &myMarble, &isNight);

			// moves the test marble corresponding to its angle value
			moveMarble(&g_sContext, &testMarble, resultsBuffer);

			// checking if the test marble has a wall on its way or not
			if (isWallinMarble(&myLabyrinth, &testMarble)) // if there is a wall, revert the test marble coordinates back to the original
			{
				testMarble.x = myMarble.x;
				testMarble.y = myMarble.y;
			}
			else
			{
				// erasing and drawing the marble by different colors depending on its mode
				if (lux > NIGHT_MODE_LUX)
				{
					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_TAN);
					eraseMarble(&g_sContext, &myMarble);
				}
				if (lux < NIGHT_MODE_LUX)
				{
					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN_YELLOW);
					eraseMarble(&g_sContext, &myMarble);
				}

				// if the test marble has confirmred there is no wall on its way
				// move the actual marble

				myMarble.x = testMarble.x;
				myMarble.y = testMarble.y;

				// check if there is a hole near the marble if so game is over
				if (checkHoles(&myLabyrinth, &myMarble))
					drawGameOverHole(&g_sContext, &timeString);

				// this is a wormhole, when interacted the marble goes back to original position
				if (checkWormHoles(&myLabyrinth, &myMarble))
				{
					int i;

					for (i = 0; i < NUM_CELLS; i++)
					{
						if (myLabyrinth.wormholes[i].valid == EXIST)
						{
							Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_MAGENTA);
							Graphics_fillCircle(&g_sContext, myLabyrinth.wormholes[i].x, myLabyrinth.wormholes[i].y, HOLE_RADIUS);
						}
					}

					if (lux > NIGHT_MODE_LUX)
					{
						Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_TAN);
						eraseMarble(&g_sContext, &myMarble);
					}
					if (lux < NIGHT_MODE_LUX)
					{
						Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN_YELLOW);
						eraseMarble(&g_sContext, &myMarble);
					}

					myMarble.x = LAB_MIN_X + HALF_CELL;
					myMarble.y = LAB_MIN_Y + HALF_CELL;
					testMarble.x = LAB_MIN_X + HALF_CELL;
					testMarble.y = LAB_MIN_Y + HALF_CELL;
				}

				// check if there is the target near the marble if so game is over
				if (checkTarget(&myLabyrinth, &myMarble))
					drawGameOverTarget(&g_sContext, &timeString);

				if (lux > NIGHT_MODE_LUX)
				{
					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
					drawMarble(&g_sContext, &myMarble);
				}
				if (lux < NIGHT_MODE_LUX)
				{
					Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_ORANGE);
					drawMarble(&g_sContext, &myMarble);
				}
			}

			StartOneShotSWTimer(&twentyMS);
		}
	}
}

void initialize()
{
	// stop the watchdog timer
	WDT_A_hold(WDT_A_BASE);

	// initialize the boosterPack LEDs and turn them off except for red LED
	initialize_BoosterpackLED_red();
	initialize_BoosterpackLED_green();
	initialize_BoosterpackLED_blue();
	initialize_LaunchpadLED1();
	initialize_LaunchpadLED2_red();
	initialize_LaunchpadLED2_blue();
	initialize_LaunchpadLED2_green();

	turnOff_BoosterpackLED_red();
	turnOff_BoosterpackLED_green();
	turnOff_BoosterpackLED_blue();
	turnOff_LaunchpadLED1();
	turnOff_LaunchpadLED2_red();
	turnOff_LaunchpadLED2_blue();
	turnOff_LaunchpadLED2_green();

	initHWTimer0();
	initHWTimer1();


	initAccelerometer();
	initADC();
	startADC();
}