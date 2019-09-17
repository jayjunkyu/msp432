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


// This function initializes all the peripherals except graphics
void initialize();
void ModifyLEDColor(bool leftButtonWasPushed, bool rightButtonWasPushed);

#define EXPIRED 0
#define PRESSED 0
#define BOOSTER_BUTTON1 BIT1
#define BLOCKING 350000

#define DURATION 100

extern song_t enter_sandman;
extern song_t hokie_fight;

typedef enum {High, Neutral, Low} joystick_status_t;
typedef struct {unsigned int option; int yPos; joystick_status_t state; bool actionUp; bool actionDown; unsigned int buttonCount;} cursor_t;

unsigned colormix(unsigned r,unsigned g,unsigned b) {
    return (r & 0xff) | ((g & 0xff) << 8) | ((b & 0xff) << 16);
}

void drawBackground(Graphics_Context *g_sContext_p)
{
    unsigned i, r, g, b;

    for (i=0; i<128; i++) {
        r = 10;
        g = i*2;
        b = 256 - i*2;
        Graphics_setForegroundColor(g_sContext_p, colormix(r,g,b));
        Graphics_drawLineH(g_sContext_p, 0, 127, i);
    }

}

void drawMenu(Graphics_Context *g_sContext_p)
{
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
}

void eraseCursor(Graphics_Context *g_sContext_p)
{
    Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
    Graphics_Rectangle eraser;

    eraser.xMin = 7;
    eraser.xMax = 20;
    eraser.yMin = 70;
    eraser.yMax = 120;

    Graphics_fillRectangle(g_sContext_p, &eraser);
}

void drawCursor(Graphics_Context *g_sContext_p, cursor_t *cursor)
{
    char cursorStrg[2] = ">";
    int newPos = cursor->yPos;

    Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
    Graphics_drawString(g_sContext_p, cursorStrg, -1, 12, 68 + newPos, false);
}

void moveCursor(Graphics_Context *g_sContext_p, cursor_t *cursor)
{
    if(cursor->actionUp == true && cursor->option != 0 && cursor->yPos>0 && cursor->state == Neutral)
    {
        eraseCursor(g_sContext_p);
        cursor->option -= 1;
        cursor->yPos -= 17;
        drawCursor(g_sContext_p, cursor);
    }

    if(cursor->actionDown == true && cursor->option != 2 && cursor->yPos <= 17 && cursor->state == Neutral)
    {
        eraseCursor(g_sContext_p);
        cursor->option += 1;
        cursor->yPos += 17;
        drawCursor(g_sContext_p, cursor);
    }
}


bool joyStickPushedtoUp = false;
bool joyStickPushedtoDown = false;

void debounceAndMoveCursor(Graphics_Context *g_sContext_p, cursor_t *cursor, unsigned int vy)
{
    int highH = 16359;
    int highL = 16344;
    int lowH = 12;
    int lowL = 4;

    switch(cursor->state)
    {
    case Neutral:
        if(vy <= highH && vy >= highL)
        {
            cursor->actionUp = true;
            cursor->actionDown = false;

            moveCursor(g_sContext_p, cursor);
            cursor->state = High;
        }
        if(vy <= lowH && vy >= lowL)
        {
            cursor->actionUp = false;
            cursor->actionDown = true;
            moveCursor(g_sContext_p, cursor);
            cursor->state = Low;

        }
        break;

    case High:
        if (vy < highL && vy > lowH )
        {
            cursor->state = Neutral;

            cursor->actionUp = false;
            cursor->actionDown = false;
        }
        break;

    case Low:
        if (vy < highL && vy > lowH )
        {
            cursor->state = Neutral;

            cursor->actionUp = false;
            cursor->actionDown = false;
        }
        break;
    }
}

void drawOptionPages(Graphics_Context *g_sContext_p, cursor_t *cursor)
{
    if(SwitchStatus_Boosterpack_Button1() == PRESSED)
        cursor->buttonCount += 1;

    switch(cursor->option)
    {
    case 0:
        break;
    case 1:
        if(cursor->buttonCount == 1)
        {
            Graphics_clearDisplay(g_sContext_p);
            Graphics_setFont(g_sContext_p, &g_sFontCmsc16);
            Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
            Graphics_drawString(g_sContext_p, "Guitar Hero is" , -1, 1, 1, false);
            while(1)
            {

            }

        }

        if(cursor->buttonCount == 2)
        {

        }
        if(cursor->buttonCount == 3)
        {

        }
        if(cursor->buttonCount == 4)
        {

        }
        if(cursor->buttonCount == 5)
        {

        }
        if(cursor->buttonCount == 6)
        {

        }
        if(cursor->buttonCount == 7)
        {

        }
        if(cursor->buttonCount == 8)
        {

        }
        if(cursor->buttonCount == 9)
        {

        }
        if(cursor->buttonCount == 10)
        {

        }
        if(cursor->buttonCount == 11)
        {

        }
        break;
    case 2:
        break;
    }
}

int main(void)
{
    Graphics_Context g_sContext;
    InitGraphics(&g_sContext);

    initialize();

    unsigned vx, vy;
    cursor_t startCursor = {0, 0, Neutral, false, false, 0};

    drawBackground(&g_sContext);

    Timer32_setCount(timer0.timerID, timer0.loadValue);
    Timer32_startTimer(timer0.timerID, true); //false indicates "This is not one-shot mode"
    bool ThreeSecCount = false;

    while(ThreeSecCount == false)
    {
        if(Timer32_getValue(timer0.timerID) == EXPIRED)
            ThreeSecCount = true;
    }

    drawMenu(&g_sContext);
    char cursorStrg[2] = ">";
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_drawString(&g_sContext, cursorStrg, -1, 12, 68, false);

    //TODO: comment out this part once you complete part 3
    /*
    InitSound();
    InitSongList();
    PlaySong(enter_sandman);
    PlaySong(hokie_fight);
*/
    int i = 0;
    while (1)
    {
            getSampleJoyStick(&vx, &vy);
            for (i=0; i<BLOCKING; i++);
            //drawXY(&g_sContext, vx, vy);
            debounceAndMoveCursor(&g_sContext, &startCursor, vy);
            drawOptionPages(&g_sContext, &startCursor);
            //if(SwitchStatus_Boosterpack_Button1() == PRESSED)
                //Graphics_drawString(&g_sContext, "WHAT!!!", -1, 12, 30, false);
    }
}

char SwitchStatus_Boosterpack_Button1()
{
    return (P5IN & BOOSTER_BUTTON1);
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

    P5DIR &= ~BOOSTER_BUTTON1;

    initHWTimer0();
    initHWTimer1();

    initADC();
    initJoyStick();
    startADC();
}

// This FSM has two inputs each of them the FSM if a button has been pushed or not
// The FSM has three states: Red, Green, Blue. The initial state is Red
// The FSM has three outputs, each output is a boolean that decides if an LED should be on or off
// When the left button is pressed, the FSM goes
void ModifyLEDColor(bool leftButtonWasPushed, bool rightButtonWasPushed)
{
    typedef enum {red, green, blue} LED_state_t;

    static LED_state_t BoosterLED = red;

    // outputs of the FSM and their default
    bool toggleGreen = false;
    bool toggleBlue = false;
    bool toggleRed = false;

    switch(BoosterLED)
    {
    case red:
        if (leftButtonWasPushed)
        {
            // next state
            BoosterLED = green;

            //outputs
            // This turns green on
            toggleGreen = true;

            // This turns red off
            toggleRed = true;
        }
        else if (rightButtonWasPushed)
        {
            BoosterLED = blue;

            //outputs
            toggleBlue = true;
            toggleRed = true;
        }
        break;
    case green:
        if (leftButtonWasPushed)
        {
            // next state
            BoosterLED = blue;

            //outputs
            toggleBlue = true;
            toggleGreen = true;
        }
        else if (rightButtonWasPushed)
        {
            BoosterLED = red;

            //outputs
            toggleRed = true;
            toggleGreen = true;
        }
        break;
    case blue:
        if (leftButtonWasPushed)
        {
            // next state
            BoosterLED = red;

            //outputs
            toggleRed = true;
            toggleBlue = true;
        }
        else if (rightButtonWasPushed)
        {
            BoosterLED = green;

            //outputs
            toggleGreen = true;
            toggleBlue = true;
        }
    }

    if (toggleRed)
        toggle_BoosterpackLED_red();

    if (toggleGreen)
        toggle_BoosterpackLED_green();

    if (toggleBlue)
        toggle_BoosterpackLED_blue();

}


