// Mini Guitar Hero by JJeong
// Note : Header files are missing due to copyright issues

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "ButtonLED_HAL.h"
#include "graphics_HAL.h"
#include "ADC_HAL.h"
#include "sound.h"
#include "song.h"

extern HWTimer_t timer0, timer1;
extern tImage  Guitar_Hero_Logo_new8BPP_UNCOMP; // image from web, source of file is stated in .c file

// This function initializes all the peripherals except graphics
void initialize();

#define DURATION 100
#define BLOCKING 600000 // blocking variable when using empty for loop
#define PRESSED 0 // checking if button is pressed

extern song_t enter_sandman;
extern song_t hokie_fight;
extern song_t we_will_rock;

typedef enum { High, Neutral, Low } joystick_status_t; // this keeps up with the debouncing status
typedef enum { Red, Blue, Green, Yellow } noteColor_t; // this keeps up with the generated note color
typedef enum { splash, letsRock, howToPlay, highScores, hokieFight, enterNight } screenState_t; // this keeps up with the topFSM states

typedef struct { int yPos; joystick_status_t state; bool moveUp; bool moveDown; } joystick_t;
typedef struct { screenState_t state;  int currentScore; int highScore; int midScore; int lowScore; } screen_t;
typedef struct { noteColor_t color; } note_t;

// color mix function for fun background
unsigned colormix(unsigned r, unsigned g, unsigned b) {
	return (r & 0xff) | ((g & 0xff) << 8) | ((b & 0xff) << 16);
}

// making int values into string to update score
void make_4digit_NumString(unsigned int num, int8_t* string)
{
	unsigned int first_digit = 0;
	unsigned int second_digit = 0;
	unsigned int third_digit = 0;
	unsigned int fourth_digit = 0;

	if (num >= 1000 && num < 10000)
	{
		first_digit = num / 1000;
		num = num - (1000 * first_digit);

		second_digit = num / 100;
		num = num - (100 * second_digit);

		third_digit = num / 10;
		num = num - (10 * third_digit);

		fourth_digit = num / 1;
		num = num - (1 * fourth_digit);

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit + 48;
		string[3] = fourth_digit + 48;
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

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit + 48;
		string[3] = fourth_digit;
	}

	else if (num >= 10 && num < 100)
	{
		first_digit = num / 10;
		num = num - (10 * first_digit);

		second_digit = num / 1;
		num = num - (1 * second_digit);

		third_digit = 32;

		fourth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit + 48;
		string[2] = third_digit;
		string[3] = fourth_digit;
	}

	else if (num < 10)
	{
		first_digit = num / 1;
		num = num - (1 * first_digit);

		second_digit = 32;

		third_digit = 32;

		fourth_digit = 32;

		string[0] = first_digit + 48;
		string[1] = second_digit;
		string[2] = third_digit;
		string[3] = fourth_digit;
	}
}

// updates the score list in order
void updateScoreList(screen_t* screen)
{
	if (screen->currentScore >= screen->highScore)
	{
		screen->lowScore = screen->midScore;
		screen->midScore = screen->highScore;
		screen->highScore = screen->currentScore;
	}
	else if (screen->currentScore == screen->midScore)
	{
		screen->lowScore = screen->currentScore;
	}
	else if (screen->currentScore > screen->midScore && screen->currentScore < screen->highScore)
	{
		screen->midScore = screen->currentScore;
	}
	else if (screen->currentScore > screen->lowScore && screen->currentScore < screen->midScore)
	{
		screen->lowScore = screen->currentScore;
	}
}

// this is called when we first enter the splash screen
void startGraphics(Graphics_Context* g_sContext_p)
{
	unsigned i, r, g, b;

	for (i = 0; i < 128; i++) {
		r = 10;
		g = i * 2;
		b = 256 - i * 2;
		Graphics_setForegroundColor(g_sContext_p, colormix(r, g, b));
		Graphics_drawLineH(g_sContext_p, 0, 127, i);
	}

	Graphics_drawImage(g_sContext_p, &Guitar_Hero_Logo_new8BPP_UNCOMP, 24, 0); // calling image
	Graphics_setFont(g_sContext_p, &g_sFontCmss12b);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_drawString(g_sContext_p, "by Joseph Jeong", -1, 17, 54, false); // adding name as required

	// holds for three seconds before showing menu
	int threeSecCount = 3000000;
	OneShotSWTimer_t threeSecTimer;
	InitOneShotSWTimer(&threeSecTimer, &timer0, threeSecCount);
	StartOneShotSWTimer(&threeSecTimer);
	while (!OneShotSWTimerExpired(&threeSecTimer));

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 7;
	background.xMax = 120;
	background.yMin = 67;
	background.yMax = 120;

	Graphics_fillRectangle(g_sContext_p, &background);

	char option1[11] = "LET'S ROCK";
	char option2[12] = "HOW TO PLAY";
	char option3[12] = "HIGH SCORES";

	Graphics_setFont(g_sContext_p, &g_sFontCmsc12);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_drawString(g_sContext_p, option1, -1, 24, 68, true);
	Graphics_drawString(g_sContext_p, option2, -1, 24, 85, true);
	Graphics_drawString(g_sContext_p, option3, -1, 24, 102, true);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_drawString(g_sContext_p, ">", -1, 12, 68, false);
}

// draws the splash screen when returning from another page
void drawReturnHome(Graphics_Context* g_sContext_p, joystick_t* joystick)
{
	joystick->yPos = 0;

	unsigned i, r, g, b;

	for (i = 0; i < 128; i++) {
		r = 10;
		g = i * 2;
		b = 256 - i * 2;
		Graphics_setForegroundColor(g_sContext_p, colormix(r, g, b));
		Graphics_drawLineH(g_sContext_p, 0, 127, i);
	}

	Graphics_drawImage(g_sContext_p, &Guitar_Hero_Logo_new8BPP_UNCOMP, 24, 0); // calling image
	Graphics_setFont(g_sContext_p, &g_sFontCmss12b);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_drawString(g_sContext_p, "by Joseph Jeong", -1, 17, 54, false); // adding name as required

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 7;
	background.xMax = 120;
	background.yMin = 67;
	background.yMax = 120;

	Graphics_fillRectangle(g_sContext_p, &background);

	char option1[11] = "LET'S ROCK";
	char option2[12] = "HOW TO PLAY";
	char option3[12] = "HIGH SCORES";

	Graphics_setFont(g_sContext_p, &g_sFontCmsc12);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_drawString(g_sContext_p, option1, -1, 24, 68, true);
	Graphics_drawString(g_sContext_p, option2, -1, 24, 85, true);
	Graphics_drawString(g_sContext_p, option3, -1, 24, 102, true);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_drawString(g_sContext_p, ">", -1, 12, 68, false);
}

// this draws the current score for a while right after a game
void drawCurrentScore(Graphics_Context* g_sContext_p, joystick_t* joystick, screen_t* screen)
{
	joystick->yPos = 0;

	unsigned i, r, g, b;

	for (i = 0; i < 128; i++) {
		r = 10;
		g = i * 2;
		b = 256 - i * 2;
		Graphics_setForegroundColor(g_sContext_p, colormix(r, g, b));
		Graphics_drawLineH(g_sContext_p, 0, 127, i);
	}

	Graphics_drawImage(g_sContext_p, &Guitar_Hero_Logo_new8BPP_UNCOMP, 24, 0); // calling image
	Graphics_setFont(g_sContext_p, &g_sFontCmss12b);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_drawString(g_sContext_p, "by Joseph Jeong", -1, 17, 54, false); // adding name as required

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 17;
	background.xMax = 110;
	background.yMin = 67;
	background.yMax = 110;

	Graphics_fillRectangle(g_sContext_p, &background);

	char option1[9] = "ROCK ON!";
	char option2[16] = "SCORE: ";
	char scoreString[5] = "    ";

	make_4digit_NumString(screen->currentScore, &scoreString);

	Graphics_setFont(g_sContext_p, &g_sFontCmsc12);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_drawString(g_sContext_p, option1, -1, 33, 68, true);
	Graphics_drawString(g_sContext_p, option2, -1, 28, 85, true);
	Graphics_drawString(g_sContext_p, scoreString, -1, 85, 85, true);
}

// this function debounces the joystick to move up and down
bool debouncejoystickUpDown(unsigned int yPosition, joystick_t* joystick, screen_t* screen)
{
	// this blocking function prevents the joystick from moving up twice or more
	int i = 0;
	for (i = 0; i < BLOCKING; i++);

	//inputs
	unsigned int highH = 16400;
	unsigned int highL = 16100;
	unsigned int lowH = 200;
	unsigned int lowL = 0;

	// notice that we only move position at neutral state
	switch (joystick->state)
	{
	case Neutral:
		if (yPosition <= highH && yPosition >= highL)
		{
			if (screen->state == splash || screen->state == howToPlay || screen->state == hokieFight
				|| screen->state == enterNight)
			{
				if (joystick->yPos > 0)
				{
					joystick->moveUp = true;
					joystick->state = High;
				}
				else
					joystick->moveUp = false;
			}

			if (screen->state == letsRock)
			{
				if (joystick->yPos > 17)
				{
					joystick->moveUp = true;
					joystick->state = High;
				}
				else
					joystick->moveUp = false;
			}
		}
		if (yPosition <= lowH && yPosition >= lowL)
		{
			if (joystick->yPos < 34)
			{
				joystick->moveDown = true;
				joystick->state = Low;
			}
			else
				joystick->moveDown = false;
		}
		break;
	case High:
		if (yPosition < highL && yPosition > lowH)
		{
			joystick->moveUp = false;
			joystick->moveDown = false;
			joystick->state = Neutral;
		}
		break;
	case Low:
		if (yPosition < highL && yPosition > lowH)
		{
			joystick->moveUp = false;
			joystick->moveDown = false;
			joystick->state = Neutral;
		}
		break;
	}
}

// this function erases the current arrow
void eraseArrow(Graphics_Context* g_sContext_p)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_Rectangle eraser;

	eraser.xMin = 7;
	eraser.xMax = 20;
	eraser.yMin = 70;
	eraser.yMax = 120;

	Graphics_fillRectangle(g_sContext_p, &eraser);
}

// this function draws the newly positioned arrow depending on the screen
void drawArrow(Graphics_Context* g_sContext_p, joystick_t* joystick, screen_t* screen)
{
	if (screen->state == howToPlay)
	{
		if (joystick->moveUp == true && joystick->yPos > 0)
			joystick->yPos -= 17;
		if (joystick->moveDown == true && joystick->yPos < 34)
			joystick->yPos += 17;
	}

	if (screen->state == splash)
	{
		if (joystick->moveUp == true && joystick->yPos > 0)
			joystick->yPos -= 17;
		if (joystick->moveDown == true && joystick->yPos < 34)
			joystick->yPos += 17;
	}

	if (screen->state == letsRock)
	{
		if (joystick->moveUp == true && joystick->yPos > 17)
			joystick->yPos -= 17;
		if (joystick->moveDown == true && joystick->yPos < 34)
			joystick->yPos += 17;
	}


	if (screen->state == splash || screen->state == letsRock)
	{
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_drawString(g_sContext_p, ">", -1, 12, 68 + joystick->yPos, false);
	}
}

// this function moves the cursor up and down by erasing and drawing a new one
void moveCursorUpDown(Graphics_Context* g_sContext_p, joystick_t* joystick, screen_t* screen)
{
	eraseArrow(g_sContext_p);
	drawArrow(g_sContext_p, joystick, screen);
}

// this function draws the song menu when entering the "letsrock"
void drawSongMenu(Graphics_Context* g_sContext_p, joystick_t* joystick)
{
	joystick->yPos = 17;

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 7;
	background.xMax = 120;
	background.yMin = 67;
	background.yMax = 120;

	Graphics_fillRectangle(g_sContext_p, &background);

	Graphics_setFont(g_sContext_p, &g_sFontCmsc12);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_drawString(g_sContext_p, "SONG MENU", -1, 24, 68, true);
	Graphics_drawString(g_sContext_p, "Hokie Fight", -1, 24, 85, true);
	Graphics_drawString(g_sContext_p, "Enter Night", -1, 24, 102, true);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_drawString(g_sContext_p, ">", -1, 12, 85, false);
}

// this function draws the desciption of the game dependong on the page number
// there are total of 3 pages of description
void drawDescription(Graphics_Context* g_sContext_p, joystick_t* joystick, screen_t* screen)
{
	if (joystick->yPos == 0 && screen->state == howToPlay)
	{
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

		Graphics_Rectangle background;

		background.xMin = 0;
		background.xMax = 128;
		background.yMin = 0;
		background.yMax = 128;

		Graphics_fillRectangle(g_sContext_p, &background);


		Graphics_setFont(g_sContext_p, &g_sFontCmsc12);
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_drawString(g_sContext_p, "After selecting", -1, 1, 1, false);
		Graphics_drawString(g_sContext_p, "your song, wait", -1, 1, 16, false);
		Graphics_drawString(g_sContext_p, "for the notes to", -1, 1, 31, false);
		Graphics_drawString(g_sContext_p, "scroll down the", -1, 1, 46, false);
		Graphics_drawString(g_sContext_p, "screen.", -1, 1, 61, false);
		Graphics_drawString(g_sContext_p, "When a note is", -1, 1, 76, false);
		Graphics_drawString(g_sContext_p, "in line with the", -1, 1, 91, false);
		Graphics_drawString(g_sContext_p, "circles at the", -1, 1, 106, false);
	}

	if (joystick->yPos == 17 && screen->state == howToPlay)
	{
		int i = 0;
		int blocking = 25000;
		for (i = 0; i < blocking; i++);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

		Graphics_Rectangle background;

		background.xMin = 0;
		background.xMax = 128;
		background.yMin = 0;
		background.yMax = 128;

		Graphics_fillRectangle(g_sContext_p, &background);


		Graphics_setFont(g_sContext_p, &g_sFontCmsc14);
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_drawString(g_sContext_p, "bottom of the", -1, 1, 1, false);
		Graphics_drawString(g_sContext_p, "screen, tilt the", -1, 1, 16, false);
		Graphics_drawString(g_sContext_p, "joystick to the", -1, 1, 31, false);
		Graphics_drawString(g_sContext_p, "color of note", -1, 1, 46, false);
		Graphics_drawString(g_sContext_p, "and push the S1", -1, 1, 61, false);
		Graphics_drawString(g_sContext_p, "button. If the", -1, 1, 76, false);
		Graphics_drawString(g_sContext_p, "note is played", -1, 1, 91, false);
		Graphics_drawString(g_sContext_p, "points increase.", -1, 1, 106, false);
	}

	if (joystick->yPos == 34 && screen->state == howToPlay)
	{
		int i = 0;
		int blocking = 25000;
		for (i = 0; i < blocking; i++);

		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

		Graphics_Rectangle background;

		background.xMin = 0;
		background.xMax = 128;
		background.yMin = 0;
		background.yMax = 128;

		Graphics_fillRectangle(g_sContext_p, &background);


		Graphics_setFont(g_sContext_p, &g_sFontCmsc14);
		Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
		Graphics_drawString(g_sContext_p, "Green --> Left", -1, 1, 1, false);
		Graphics_drawString(g_sContext_p, "Red --> Down", -1, 1, 16, false);
		Graphics_drawString(g_sContext_p, "Yellow --> Up", -1, 1, 31, false);
		Graphics_drawString(g_sContext_p, "Blue --> Right", -1, 1, 46, false);
	}

}

// this function draws the updated score list
void drawScoresList(Graphics_Context* g_sContext_p, screen_t* screen)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 7;
	background.xMax = 120;
	background.yMin = 67;
	background.yMax = 128;

	Graphics_fillRectangle(g_sContext_p, &background);

	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_setFont(g_sContext_p, &g_sFontCmsc14);
	Graphics_drawString(g_sContext_p, "High Score", -1, 24, 68, true);
	Graphics_setFont(g_sContext_p, &g_sFontCmsc12);

	char scoreHigh[5] = "    ";
	char scoreMid[5] = "    ";
	char scoreLow[5] = "    ";

	make_4digit_NumString(screen->highScore, &scoreHigh);
	make_4digit_NumString(screen->midScore, &scoreMid);
	make_4digit_NumString(screen->lowScore, &scoreLow);

	Graphics_drawString(g_sContext_p, "SCORE:", -1, 24, 83, true);
	Graphics_drawString(g_sContext_p, "SCORE:", -1, 24, 95, true);
	Graphics_drawString(g_sContext_p, "SCORE:", -1, 24, 107, true);

	Graphics_drawString(g_sContext_p, scoreHigh, -1, 80, 83, false);
	Graphics_drawString(g_sContext_p, scoreMid, -1, 80, 95, false);
	Graphics_drawString(g_sContext_p, scoreLow, -1, 80, 107, false);
}

// this function draws the background grid for the play screen
void drawPlayScreenBackGround(Graphics_Context* g_sContext_p)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_Rectangle background;

	background.xMin = 0;
	background.xMax = 128;
	background.yMin = 0;
	background.yMax = 128;

	Graphics_fillRectangle(g_sContext_p, &background);

	Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_drawString(g_sContext_p, " Score       0", -1, 1, 20, false);

	Graphics_Rectangle leftBoundary;

	leftBoundary.xMin = 0;
	leftBoundary.xMax = 3;
	leftBoundary.yMin = 52;
	leftBoundary.yMax = 118;

	Graphics_fillRectangle(g_sContext_p, &leftBoundary);

	Graphics_Rectangle rightBoundary;

	rightBoundary.xMin = 125;
	rightBoundary.xMax = 128;
	rightBoundary.yMin = 52;
	rightBoundary.yMax = 118;

	Graphics_fillRectangle(g_sContext_p, &rightBoundary);
}

// this function draws the vertical lines depending on how much it moved
void drawV(Graphics_Context* g_sContext_p, unsigned int x)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);

	Graphics_drawLineV(g_sContext_p, x, 53, 118);
}

// this function erases the previous vertical lines in the game screen
void eraseV(Graphics_Context* g_sContext_p, unsigned int x)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_drawLineV(g_sContext_p, x, 53, 118);
}

// this function erases the previous horizontal lines in the game screen
void eraseH(Graphics_Context* g_sContext_p, unsigned int y)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);

	Graphics_drawLineH(g_sContext_p, 4, 28, y);
	Graphics_drawLineH(g_sContext_p, 28, 52, y);
	Graphics_drawLineH(g_sContext_p, 52, 74, y);
	Graphics_drawLineH(g_sContext_p, 74, 104, y);
	Graphics_drawLineH(g_sContext_p, 104, 124, y);
}

// this function draws the newly positioned horizontal lines in the game screen
void drawH(Graphics_Context* g_sContext_p, unsigned int y)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);

	Graphics_drawLineH(g_sContext_p, 4, 28, y);
	Graphics_drawLineH(g_sContext_p, 28, 52, y);
	Graphics_drawLineH(g_sContext_p, 52, 74, y);
	Graphics_drawLineH(g_sContext_p, 74, 104, y);
	Graphics_drawLineH(g_sContext_p, 104, 124, y);
}

// this function draws the newly positioned green note
void drawGreenNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
	Graphics_fillCircle(g_sContext_p, 27, 58 + pixel, 4);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 27, 58 + pixel, 2);
}

// this function draws the newly positioned blue note
void drawBlueNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
	Graphics_fillCircle(g_sContext_p, 99, 58 + pixel, 4);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 99, 58 + pixel, 2);
}

// this function draws the newly positioned yellow note
void drawYellowNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
	Graphics_fillCircle(g_sContext_p, 75, 58 + pixel, 4);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 75, 58 + pixel, 2);
}

// this function draws the newly positioned red note
void drawRedNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
	Graphics_fillCircle(g_sContext_p, 51, 58 + pixel, 4);
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
	Graphics_fillCircle(g_sContext_p, 51, 58 + pixel, 2);
}

// the below functions erase the previous positioned notes

void eraseRedNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 51, 58 + pixel, 4);
}

void eraseBlueNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 99, 58 + pixel, 4);
}

void eraseYellowNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 75, 58 + pixel, 4);
}

void eraseGreenNote(Graphics_Context* g_sContext_p, unsigned int pixel)
{
	Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
	Graphics_fillCircle(g_sContext_p, 27, 58 + pixel, 4);
}

// this top FSM can see all of the actions and states within the game
void topFSM(bool button, joystick_t* joystick, screen_t* screen, Graphics_Context* g_sContext_p, note_t* myNote)
{
	// states are stored in the screen_t which contains all the pages

	// inputs : timers, button, joystick
	OneShotSWTimer_t fifteenSecTimer;
	OneShotSWTimer_t twoSecTimer;
	OneShotSWTimer_t twentyMSTimer;
	InitOneShotSWTimer(&fifteenSecTimer, &timer0, 15000000);
	bool buttonPushed = button;

	// outputs : booleans to whether return home, go to next page, go back to prev page, move arrow, show certain pages, and game screen
	static bool returnHome = false;
	static bool nextPage = false;
	static bool prevPage = false;
	static bool moveArrow = false;
	static bool songMenu = false;
	static bool description = false;
	static bool scoresList = false;
	static bool startGame = false;

	switch (screen->state)
	{
	case splash:
		if (!buttonPushed && (joystick->moveUp == true || joystick->moveDown == true))
		{
			moveArrow = true;
		}
		if (buttonPushed && joystick->yPos == 0)
		{
			songMenu = true;
			screen->state = letsRock;
		}
		if (buttonPushed && joystick->yPos == 17)
		{
			description = true;
			screen->state = howToPlay;
		}
		if (buttonPushed && joystick->yPos == 34)
		{
			scoresList = true;
			screen->state = highScores;
		}

		break;
	case letsRock:
		if (!buttonPushed && (joystick->moveUp == true || joystick->moveDown == true))
		{
			moveArrow = true;
		}
		if (joystick->yPos == 17 && buttonPushed)
		{
			screen->state = hokieFight;
			startGame = true;
			StartOneShotSWTimer(&fifteenSecTimer);
		}
		if (joystick->yPos == 34 && buttonPushed)
		{
			screen->state = enterNight;
			startGame = true;
			StartOneShotSWTimer(&fifteenSecTimer);
		}
		break;
	case howToPlay:
		if (!buttonPushed && joystick->moveUp == true)
		{
			description = true;
			prevPage = true;
		}
		if (!buttonPushed && joystick->moveDown == true)
		{
			description = true;
			nextPage = true;
		}
		if (buttonPushed)
		{
			returnHome = true;
			screen->state = splash;
		}
		break;
	case highScores:
		if (buttonPushed)
		{
			returnHome = true;
			screen->state = splash;
		}
		break;
		// here we are entering the game
	case hokieFight:
		drawPlayScreenBackGround(g_sContext_p);
		InitOneShotSWTimer(&twentyMSTimer, &timer1, 20000);
		InitOneShotSWTimer(&twoSecTimer, &timer1, 2000000);
		StartOneShotSWTimer(&twentyMSTimer);
		StartOneShotSWTimer(&twoSecTimer);
		InitSound();
		InitSongList();
		PlaySong(hokie_fight); // this used to be unblocked but for now it is a blocking code
		while (startGame == true) // once the fifteen second timer starts, the game starts
		{
			static unsigned int yH1 = 55;
			static unsigned int yH2 = 71;
			static unsigned int yH3 = 89;
			static unsigned int yH4 = 106;
			static unsigned int yV1 = 54;
			static unsigned int yV2 = 56;
			static unsigned int pixelGreen = 0;
			static unsigned int pixelYellow = 0;
			static unsigned int pixelBlue = 0;
			static unsigned int pixelRed = 0;
			int vx_high = 15000;
			int vx_low = 300;
			int vy_high = 16000;
			int vy_low = 200;
			int lastNotePixel = 63;
			int lastLinePixel = 118;
			int initLinePixel = 54;

			// we need the vx, vy sampled data to play the notes
			unsigned int vx, vy;
			getSampleJoyStick(&vx, &vy);

			static enum { Green, Red, Yellow, Blue } noteColor;

			// moving the notes, lines, looking for joystick and button input, and updating the score inside the 20ms timer
			if (OneShotSWTimerExpired(&twentyMSTimer))
			{
				eraseV(g_sContext_p, 27);
				eraseV(g_sContext_p, 51);
				eraseV(g_sContext_p, 75);
				eraseV(g_sContext_p, 99);

				eraseH(g_sContext_p, yH1);
				eraseH(g_sContext_p, yH2);
				eraseH(g_sContext_p, yH3);
				eraseH(g_sContext_p, yH4);

				if (noteColor == Green)
				{
					eraseGreenNote(g_sContext_p, pixelGreen);
					pixelGreen++;
				}

				if (noteColor == Blue)
				{
					eraseBlueNote(g_sContext_p, pixelBlue);
					pixelBlue++;
				}

				if (noteColor == Red)
				{
					eraseRedNote(g_sContext_p, pixelRed);
					pixelRed++;
				}

				if (noteColor == Yellow)
				{
					eraseYellowNote(g_sContext_p, pixelYellow);
					pixelYellow++;
				}

				// incrementing these values to move down the lines
				yH1++;
				yH2++;
				yH3++;
				yH4++;
				yV1++;
				yV2++;

				// when it hits the end of the note grid, the values go back to default to redraw them on the top
				if (yH1 == lastLinePixel)
				{
					yH1 = initLinePixel;
				}

				if (yH2 == lastLinePixel)
				{
					yH2 = initLinePixel;
				}

				if (yH3 == lastLinePixel)
				{
					yH3 = initLinePixel;
				}

				if (yH4 == lastLinePixel)
				{
					yH4 = initLinePixel;
				}

				drawV(g_sContext_p, 27);
				drawV(g_sContext_p, 51);
				drawV(g_sContext_p, 75);
				drawV(g_sContext_p, 99);

				drawH(g_sContext_p, yH1);
				drawH(g_sContext_p, yH2);
				drawH(g_sContext_p, yH3);
				drawH(g_sContext_p, yH4);

				if (noteColor == Yellow)
					drawYellowNote(g_sContext_p, pixelYellow);

				if (noteColor == Blue)
					drawBlueNote(g_sContext_p, pixelBlue);

				if (noteColor == Red)
					drawRedNote(g_sContext_p, pixelRed);

				if (noteColor == Green)
					drawGreenNote(g_sContext_p, pixelGreen);

				// these are the circles at the end of the grid for the users to use them as to where to play the note
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
				Graphics_fillCircle(g_sContext_p, 51, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 51, 121, 4);

				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
				Graphics_fillCircle(g_sContext_p, 27, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 27, 121, 4);

				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
				Graphics_fillCircle(g_sContext_p, 75, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 75, 121, 4);

				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
				Graphics_fillCircle(g_sContext_p, 99, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 99, 121, 4);


				// the below code is to update the score whenver the correct joystick direction and S1 button is pushed
				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vy < vy_low)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
					Graphics_fillCircle(g_sContext_p, 51, 121, 3);

					if (pixelRed == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelRed++;
					}
				}

				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vy > vy_high)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
					Graphics_fillCircle(g_sContext_p, 75, 121, 3);

					if (pixelYellow == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelYellow++;
					}
				}

				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vx > vx_high)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
					Graphics_fillCircle(g_sContext_p, 99, 121, 3);

					if (pixelBlue == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelBlue++;
					}
				}

				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vx < vx_low)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
					Graphics_fillCircle(g_sContext_p, 27, 121, 3);

					if (pixelGreen == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelGreen++;
					}
				}

				InitOneShotSWTimer(&twentyMSTimer, &timer0, 20000);
				StartOneShotSWTimer(&twentyMSTimer);

				if (OneShotSWTimerExpired(&twoSecTimer))
				{
					switch (noteColor)
					{
					case Green:
						noteColor = Yellow;
						pixelYellow = 0;
						break;
					case Yellow:
						noteColor = Red;
						pixelRed = 0;
						break;
					case Red:
						noteColor = Blue;
						pixelBlue = 0;
						break;
					case Blue:
						noteColor = Green;
						pixelGreen = 0;
						break;
					}
					InitOneShotSWTimer(&twoSecTimer, &timer0, 2000000);
					StartOneShotSWTimer(&twoSecTimer);
				}
			}
			if (OneShotSWTimerExpired(&fifteenSecTimer))
				startGame = false;
		}

		// when the game ends, a pop up appears indicating current score and goes back to the splash screen
		if (startGame == false)
		{
			OneShotSWTimer_t threeSecTimer;
			InitOneShotSWTimer(&threeSecTimer, &timer0, 3000000);
			StartOneShotSWTimer(&threeSecTimer);
			drawCurrentScore(g_sContext_p, joystick, screen);
			while (!OneShotSWTimerExpired(&threeSecTimer));
			updateScoreList(screen);
			screen->currentScore = 0;
			screen->state = splash;
			returnHome = true;
		}
		break;
		break;

	case enterNight:
		drawPlayScreenBackGround(g_sContext_p);
		InitOneShotSWTimer(&twentyMSTimer, &timer1, 20000);
		InitOneShotSWTimer(&twoSecTimer, &timer1, 2000000);
		StartOneShotSWTimer(&twentyMSTimer);
		StartOneShotSWTimer(&twoSecTimer);
		InitSound();
		InitSongList();
		PlaySong(enter_sandman); // this used to be unblocked but for now it is a blocking code
		while (startGame == true) // once the fifteen second timer starts, the game starts
		{
			static unsigned int yH1 = 55;
			static unsigned int yH2 = 71;
			static unsigned int yH3 = 89;
			static unsigned int yH4 = 106;
			static unsigned int yV1 = 54;
			static unsigned int yV2 = 56;
			static unsigned int pixelGreen = 0;
			static unsigned int pixelYellow = 0;
			static unsigned int pixelBlue = 0;
			static unsigned int pixelRed = 0;
			int vx_high = 15000;
			int vx_low = 300;
			int vy_high = 16000;
			int vy_low = 200;
			int lastNotePixel = 63;
			int lastLinePixel = 118;
			int initLinePixel = 54;

			// we need the vx, vy sampled data to play the notes
			unsigned int vx, vy;
			getSampleJoyStick(&vx, &vy);

			static enum { Green, Red, Yellow, Blue } noteColor;

			// moving the notes, lines, looking for joystick and button input, and updating the score inside the 20ms timer
			if (OneShotSWTimerExpired(&twentyMSTimer))
			{
				eraseV(g_sContext_p, 27);
				eraseV(g_sContext_p, 51);
				eraseV(g_sContext_p, 75);
				eraseV(g_sContext_p, 99);

				eraseH(g_sContext_p, yH1);
				eraseH(g_sContext_p, yH2);
				eraseH(g_sContext_p, yH3);
				eraseH(g_sContext_p, yH4);

				if (noteColor == Green)
				{
					eraseGreenNote(g_sContext_p, pixelGreen);
					pixelGreen++;
				}

				if (noteColor == Blue)
				{
					eraseBlueNote(g_sContext_p, pixelBlue);
					pixelBlue++;
				}

				if (noteColor == Red)
				{
					eraseRedNote(g_sContext_p, pixelRed);
					pixelRed++;
				}

				if (noteColor == Yellow)
				{
					eraseYellowNote(g_sContext_p, pixelYellow);
					pixelYellow++;
				}

				// incrementing these values to move down the lines
				yH1++;
				yH2++;
				yH3++;
				yH4++;
				yV1++;
				yV2++;

				// when it hits the end of the note grid, the values go back to default to redraw them on the top
				if (yH1 == lastLinePixel)
				{
					yH1 = initLinePixel;
				}

				if (yH2 == lastLinePixel)
				{
					yH2 = initLinePixel;
				}

				if (yH3 == lastLinePixel)
				{
					yH3 = initLinePixel;
				}

				if (yH4 == lastLinePixel)
				{
					yH4 = initLinePixel;
				}

				drawV(g_sContext_p, 27);
				drawV(g_sContext_p, 51);
				drawV(g_sContext_p, 75);
				drawV(g_sContext_p, 99);

				drawH(g_sContext_p, yH1);
				drawH(g_sContext_p, yH2);
				drawH(g_sContext_p, yH3);
				drawH(g_sContext_p, yH4);

				if (noteColor == Yellow)
					drawYellowNote(g_sContext_p, pixelYellow);

				if (noteColor == Blue)
					drawBlueNote(g_sContext_p, pixelBlue);

				if (noteColor == Red)
					drawRedNote(g_sContext_p, pixelRed);

				if (noteColor == Green)
					drawGreenNote(g_sContext_p, pixelGreen);

				// these are the circles at the end of the grid for the users to use them as to where to play the note
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
				Graphics_fillCircle(g_sContext_p, 51, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 51, 121, 4);

				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
				Graphics_fillCircle(g_sContext_p, 27, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 27, 121, 4);

				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
				Graphics_fillCircle(g_sContext_p, 75, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 75, 121, 4);

				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
				Graphics_fillCircle(g_sContext_p, 99, 121, 5);
				Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
				Graphics_fillCircle(g_sContext_p, 99, 121, 4);


				// the below code is to update the score whenver the correct joystick direction and S1 button is pushed
				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vy < vy_low)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_RED);
					Graphics_fillCircle(g_sContext_p, 51, 121, 3);

					if (pixelRed == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelRed++;
					}
				}

				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vy > vy_high)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
					Graphics_fillCircle(g_sContext_p, 75, 121, 3);

					if (pixelYellow == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelYellow++;
					}
				}

				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vx > vx_high)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
					Graphics_fillCircle(g_sContext_p, 99, 121, 3);

					if (pixelBlue == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelBlue++;
					}
				}

				if (SwitchStatus_Boosterpack_Button1() == PRESSED && vx < vx_low)
				{
					Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_GREEN);
					Graphics_fillCircle(g_sContext_p, 27, 121, 3);

					if (pixelGreen == lastNotePixel)
					{
						screen->currentScore += 10;
						char scoreString[5] = "    ";
						make_4digit_NumString(screen->currentScore, &scoreString);

						Graphics_Rectangle eraseScore;

						eraseScore.xMin = 50;
						eraseScore.xMax = 128;
						eraseScore.yMin = 15;
						eraseScore.yMax = 35;

						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
						Graphics_fillRectangle(g_sContext_p, &eraseScore);

						Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
						Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
						Graphics_drawString(g_sContext_p, scoreString, -1, 90, 20, false);
						pixelGreen++;
					}
				}

				InitOneShotSWTimer(&twentyMSTimer, &timer0, 20000);
				StartOneShotSWTimer(&twentyMSTimer);

				if (OneShotSWTimerExpired(&twoSecTimer))
				{
					switch (noteColor)
					{
					case Green:
						noteColor = Yellow;
						pixelYellow = 0;
						break;
					case Yellow:
						noteColor = Red;
						pixelRed = 0;
						break;
					case Red:
						noteColor = Blue;
						pixelBlue = 0;
						break;
					case Blue:
						noteColor = Green;
						pixelGreen = 0;
						break;
					}
					InitOneShotSWTimer(&twoSecTimer, &timer0, 2000000);
					StartOneShotSWTimer(&twoSecTimer);
				}
			}
			if (OneShotSWTimerExpired(&fifteenSecTimer))
				startGame = false;
		}

		// when the game ends, a pop up appears indicating current score and goes back to the splash screen
		if (startGame == false)
		{
			OneShotSWTimer_t threeSecTimer;
			InitOneShotSWTimer(&threeSecTimer, &timer0, 3000000);
			StartOneShotSWTimer(&threeSecTimer);
			drawCurrentScore(g_sContext_p, joystick, screen);
			while (!OneShotSWTimerExpired(&threeSecTimer));
			updateScoreList(screen);
			screen->currentScore = 0;
			screen->state = splash;
			returnHome = true;
		}
		break;
	}

	// actions from the output boleans to take
	if (returnHome == true)
	{
		drawReturnHome(g_sContext_p, joystick);
		returnHome = false;
	}

	if (moveArrow == true && (screen->state == splash || screen->state == letsRock))
	{
		moveCursorUpDown(g_sContext_p, joystick, screen);
		moveArrow = false;
	}

	if (songMenu == true)
	{
		drawSongMenu(g_sContext_p, joystick);
		songMenu = false;
	}

	if (description == true && (prevPage != true && nextPage != true))
	{
		joystick->yPos = 0;
		drawDescription(g_sContext_p, joystick, screen);
		description = false;
	}

	if (description == true && (prevPage == true || nextPage == true))
	{
		moveCursorUpDown(g_sContext_p, joystick, screen);
		drawDescription(g_sContext_p, joystick, screen);
		description = false;
	}

	if (scoresList == true)
	{
		drawScoresList(g_sContext_p, screen);
		scoresList = false;
	}
}

int main(void)
{
	Graphics_Context g_sContext;
	InitGraphics(&g_sContext);
	initialize();
	startGraphics(&g_sContext);

	// yPosition is 0, joystick is at neutral state, the moving up and down start as false
	joystick_t joystick = { 0,Neutral, false, false, false, false };
	screen_t screen = { splash,0, 0 ,0 ,0 };
	note_t myNote = { Red };
	bool S1Pushed = false;
	unsigned vx, vy;
	InitSound();
	InitSongList();
	PlaySong(we_will_rock); // plays the intro we will rock song as a bonus feature
	while (1)
	{
		getSampleJoyStick(&vx, &vy);
		if (SwitchStatus_Boosterpack_Button1() == PRESSED)
			S1Pushed = true;
		else
			S1Pushed = false;
		debouncejoystickUpDown(vy, &joystick, &screen);
		topFSM(S1Pushed, &joystick, &screen, &g_sContext, &myNote);
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

	turnOn_BoosterpackLED_red();
	turnOff_BoosterpackLED_green();
	turnOff_BoosterpackLED_blue();
	turnOff_LaunchpadLED1();
	turnOff_LaunchpadLED2_red();
	turnOff_LaunchpadLED2_blue();
	turnOff_LaunchpadLED2_green();

	initHWTimer0();
	initHWTimer1();

	initADC();
	initJoyStick();
	startADC();
}

// this was used to check the button status
char SwitchStatus_Boosterpack_Button1()
{
	return (P5IN & BIT1);
}