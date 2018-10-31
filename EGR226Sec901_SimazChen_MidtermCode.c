
/**************************************************************
 * Name: Wayne Chen & Chris Simaz
 * Date: 10-29-18
 * Program: EGR226 Sec 901, Professor Neuson, Midterm Code
 * Description: This code runs the objectives stated in the
                document. These descriptions may be found in
                the lab report
 * Notes: LCD libraries were used and were from professor
          Kanadalaft.
 *************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "msp.h"
#include "Liquid_Crystal.h"
#include "inttypes.h"

void Systick_Delay(uint16_t);
void pinInital(void);
void LCDDisplay(int);
char readKeypad(void);
void pinRead(char string[], int);
void timerRGB(int, int);
void timerMotor(int);
void servoMotor(int);

// global variable used to in code
volatile int PWM_Red,PWM_Green,PWM_Blue; // current value of PWM of RGB LED
volatile int tempR = 0, tempG = 0, tempB = 0; // temp values to store PWM of RGB LED
volatile int i = 0; // variable used to determine state of RGB LED with the white pushbutton
volatile int screen = 0; // current screen displayed


void main(void)
{

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer


    int enterVal,color;
    static char entry[10] = {0};

    // Initializing pins and LCD
    pinInital();
    lcdInit();
    lcdWriteCmd(0x0C); // remove cursor on the bottom LCD

    // Initialize interrupt handlers
    NVIC->ISER[1] = 1 << ((PORT1_IRQn) & 31);
    __enable_interrupt();

    //starting settings
    P6->OUT |= BIT0; //RED LED on
    servoMotor(4500);
    servoMotor(0);
    timerMotor(0);


    while(1)
    {

        //main menu
        while(screen == 0)
        {
            LCDDisplay(0);

            // getting valid entries from pin
            do
            {

                pinRead(entry,1);
                enterVal = atoi(entry);
            }
            while (enterVal != 1 && enterVal != 2 && enterVal != 3);

            // if the second option was entered, jump to motor selection menu
            if (enterVal == 2)
                enterVal = 5;

            // clearing screen and setting new screen value based on value entered
            LCDDisplay(9);
            screen = enterVal;
        }

        // door menu
        while(screen == 1)
        {

            // displaying door menu options
            LCDDisplay(1);

            // Getting valued inputs from screen
            do
            {

                pinRead(entry,1);
                enterVal = atoi(entry);

            }
            while (enterVal != 1 && enterVal != 2);

            //condition for servos
            if (enterVal == 1)
            {

                //lowering drawbridge
                servoMotor(1800);
                servoMotor(0); // stop servo
                P6->OUT |= BIT1; //Green LED ON
                P6->OUT &=~BIT0; //Red LED OFF
            }

            else if (enterVal == 2)
            {

                //raising drawbridge
                servoMotor(4500);
                servoMotor(0); // stop servo
                P6->OUT &=~BIT1; // Green LED OFF
                P6->OUT |= BIT0; // Red LED ON
            }

            LCDDisplay(9); // clearing screen
            screen = 0; // set screen number back to the main menu
        }

        // Windmill/ DC motor screen
        while(screen == 2)
        {

            // setting screen number
            LCDDisplay(2);

            // getting valid PWM entry
            pinRead(entry,3);
            enterVal = atoi(entry);

            // if value is greater than 100, sets value back to 100
            if(enterVal > 100)
            {

                enterVal = 100;
            }

            // setting motor speed
            timerMotor(enterVal);

            LCDDisplay(9); // clearing screen
            screen = 0; // set screen back to main screen
        }

        // Light selection menu screen
        while(screen == 3)
        {

            LCDDisplay(3); // setting light selection menu screen

            // getting valid entry from keypad
            do
            {

                pinRead(entry,1);
                color = atoi(entry); // set color to the option selected

            }
            while (enterVal != 1 && enterVal != 2 && enterVal != 3);

            LCDDisplay(9); // clear screen
            screen = 4; // set screen to the light intensity menu
        }

        // RGB light intensity menu
        while(screen == 4)
        {

            // set LCD display
            LCDDisplay(4);

            // getting keypad input
            pinRead(entry,3);
            enterVal = atoi(entry);

            // set value to 100 if value was greater
            if(enterVal > 100)
            {

                enterVal = 100;
            }

            // PWM for each light
            timerRGB(color,enterVal); // set PWM for 1 LED or RGB LED
            screen = 0; // screen back to main menu
            LCDDisplay(9); // clear screen
        }

        // motor selection screen
        while(screen == 5)
        {

            // display lcd according to screen
            LCDDisplay(5);

            do
            {

                pinRead(entry,1);
                enterVal = atoi(entry);

            }
            while (enterVal != 1 && enterVal != 2);

            LCDDisplay(9); // clear screen

            // setting the correct screen based on option selected
            if(enterVal == 1)
                enterVal = 2;

            else if(enterVal == 2)
                enterVal = 6;

            screen = enterVal;
        }

        // Waterfall menu
        while(screen == 6)
        {

            // Display menu options
            LCDDisplay(6);

            // getting valid input
            do
            {

                pinRead(entry,1);
                enterVal = atoi(entry);

            }
            while (enterVal != 1 && enterVal != 2);

            // turn on Waterfall feature
            if (enterVal == 1)
            {

                P4->OUT |= BIT7;
            }

            // turn off Waterfall feature
            else if (enterVal == 2)
            {

                P4->OUT &=~ BIT7;
            }

            // clear screen and set back to main menu
            LCDDisplay(9);
            screen = 0;
        }
    }
}

// SysTick Delay in ms
void Systick_Delay(uint16_t delayMs)
{

    //initializing
    SysTick->CTRL = 0;
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL = 5;

    //setting timer value
    SysTick->LOAD = (delayMs * 3000);
    SysTick->VAL = 0;

    //delay
    while((SysTick->CTRL & 0x00010000) == 0);

}


/** Initializes pins */
void pinInital(void)
{

    /** LCD already initialized in library */

    // Initialized LED, G = P2.1, R = P2.0
    P6->SEL0 &=~ (BIT0|BIT1);
    P6->SEL1 &=~ (BIT0|BIT1);
    P6->DIR  |=  (BIT0|BIT1);
    P6->OUT  &=~ (BIT0|BIT1);

    // RGB LED , using timer A0, BIT7 is for the motor
    P2->SEL0 |=  (BIT4|BIT5|BIT6|BIT7);
    P2->SEL1 &=~ (BIT4|BIT5|BIT6|BIT7);
    P2->DIR  |=  (BIT4|BIT5|BIT6|BIT7);

    // LCD initial, and pump (BIT7 is pin for waterfall)
    P4->SEL0 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT7);
    P4->SEL1 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT7);
    P4->DIR  |=  (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT7);
    P4->OUT  &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT7);

    // keypad
    P5->SEL0 &= ~(BIT0|BIT1|BIT2|BIT7|BIT4|BIT5|BIT6);
    P5->SEL1 &= ~(BIT0|BIT1|BIT2|BIT7|BIT4|BIT5|BIT6);
    P5->DIR  &= ~(BIT0|BIT1|BIT2|BIT7|BIT4|BIT5|BIT6);
    P5->REN  |=  (BIT0|BIT1|BIT2|BIT7|BIT4|BIT5|BIT6); //enable resistor
    P5->OUT  |=  (BIT0|BIT1|BIT2|BIT7|BIT4|BIT5|BIT6); //set resistor to pull up

    // white and red push button as interrupts
    P1->SEL0 &=~ (BIT6|BIT7);
    P1->SEL1 &=~ (BIT6|BIT7);
    P1->DIR  &=~  (BIT6|BIT7);
    P1->REN  |= (BIT6|BIT7);
    P1->OUT  |= (BIT6|BIT7);
    P1->IE   |= (BIT6|BIT7);
    P1->IES |= (BIT6|BIT7);
    P1->IFG &=~ (BIT6|BIT7);

    // Initializing DC Motor
    P6->SEL0 |= BIT7;
    P6->SEL1 &=~ BIT7;
    P6->DIR |= BIT7;

}

// Shows the current display based on chosen screen.
void LCDDisplay(int screen)
{

    // menu screen
    if (screen == 0)
    {

        lcdSetText("      Menu",     0,0);
        lcdSetText("1  Door",        0,1);
        lcdSetText("2  Motor",       0,2);
        lcdSetText("3  Lights",      0,3);
    }

    // door screen
    else if (screen == 1)
    {

        lcdSetText("Door",           6,0);
        lcdSetText("1  Open",        0,1);
        lcdSetText("2  Close",       0,2);
    }

    //motor speed setting
    else if (screen == 2)
    {

        lcdSetText("Enter Speed",     0,0);
        lcdSetText("Between 0 - 100", 1,1);
        lcdSetText("Value : ",        0,2);
    }

    // light menu
    else if (screen == 3)
    {


        lcdSetText("Lights",         5,0);
        lcdSetText("1  Red",         0,1);
        lcdSetText("2  Green",       0,2);
        lcdSetText("3  Blue",        0,3);
    }

    // brightness setting
    else if (screen == 4)
    {

        lcdSetText("Light Setting",     2,0);
        lcdSetText("Between 0 - 100",   1,1);
        lcdSetText("Value : ",          0,2);
    }

    // motor or waterfall
    else if (screen == 5)
    {

        lcdSetText("Select Option",     1,0);
        lcdSetText("1  Motor",     0,1);
        lcdSetText("2  Waterfall ",     0,2);
    }

    // turn waterfall feature on or off
    else if (screen == 6)
    {

        lcdSetText("Waterfall",        3,0);
        lcdSetText("1  ON",            0,1);
        lcdSetText("2  OFF",           0,2);
    }

    // manual clear screen, the clear function in library messed up
    // screen really badly
    else if (screen == 9)
    {

        lcdSetText("                ",0,0);
        lcdSetText("                ",0,1);
        lcdSetText("                ",0,2);
        lcdSetText("                ",0,3);
    }


}

// read keys that are inputed.
char readKeypad(void)
{

    char keypad[4][3] = {{'1','2','3'},
                         {'4','5','6'},
                         {'7','8','9'},
                         {'*','0','#'}};

    char printed;
    uint8_t col, row;

    //checking each column
    for (col = 1; col < 4; col++)
    {

        //making sure all columns are set to inputs
        P5->DIR  &= ~(BIT4|BIT5|BIT6);
        P5->REN  |=  (BIT4|BIT5|BIT6); //enable resistor
        P5->OUT  |=  (BIT4|BIT5|BIT6); //set resistor to pull-up

        //set one col to output and low
        P5->DIR |= BIT(col + 3);
        P5->OUT &=~ BIT(col + 3);

        Systick_Delay(5);//delay for 5ms
        row = P5->IN & 0x87;

        //makes sure all buttons are release
        while(!(P5->IN & BIT0) | !(P5->IN & BIT1) |
                !(P5->IN & BIT2) | !(P5->IN & BIT7));

        if (row != 0x87)
            break;
    }

    //setting col back to inputs
    P5->DIR  &= ~(BIT4|BIT5|BIT6);

    //setting value of row depending on value from for loop

    //prints CCS from printing commas all day
    if (row != 0x87)
    {

        if(row == 0x86)
            row = 1;

        else if (row == 0x85)
            row = 2;

        else if(row == 0x83)
            row = 3;

        else if(row == 0x07)
            row = 4;

        // keypad setup was an idea from online site

        // setting char to the correct number or symbol based
        // on row and col combination
        printed = keypad[row-1][col-1];

        return printed;
    }

    return 'x';
}

// This reads the numbers entered.
// entry is the number total number of numbers that can be entered
void pinRead(char string[],int entry)
{

    int Continue = 1;
    int entries=0;
    int i=0;
    char pin[4]= {0};
    char x;

    // holds the pins
    while(Continue)
    {
        x = readKeypad(); // getting value from keypad

        if (x == '*' || x == 'x'); // Invalid values

        // termination
        else if (x == '#')
        {
            if(entries == 0)
            {
                entries = 0;

            }

            else if(entries >= entry)
            {

                entries = 0;
                Continue = 0;
            }
        }

        else
        {
            entries++;

            //shifting values over
            if (entries > entry)
            {

                // preventing error if only 1 input is needed
                if (entry == 1)
                {

                    pin[0] = x;
                }

                // adding shifting values over if pass the limited inputs required
                else
                {

                    for(i = 0; i < entry - 1; i++)
                    {

                        pin[i] = pin[i+1];

                    }

                    pin[i] = x;
                }
            }

            else if(entries <= entry)
            {
                pin[entries-1] = x;
            }
        }

        // display text if on Motor speed or RGB brightness
        if (screen == 2 || screen == 4)
            lcdSetText(pin,7,2);
    }

    // copying the string entered
    strcpy(string,pin);
}

// turns on the specified color.
// colorf is the color selected from screen 3
// cycle is the PWM out of 100
void timerRGB(int colorf, int cycle)
{
    // 20 ms delay to give timer A enough time to set PWM
    Systick_Delay(20);

    // setting PWM of color component of RGB LED
    switch(colorf)
    {

        // RED LED
        case 1:
            PWM_Red = cycle;
            TIMER_A0->CCR[0] = 100;
            TIMER_A0->CCTL[1] = 0x00E0;
            TIMER_A0->CCR[1] = PWM_Red;
            TIMER_A0->CTL = 0x0214;
            break;

        // GREEN LED
        case 2:
            PWM_Green = cycle;
            TIMER_A0->CCR[0] = 100;
            TIMER_A0->CCTL[2] = 0x00E0;
            TIMER_A0->CCR[2] = PWM_Green;
            TIMER_A0->CTL = 0x0214;
            break;

        // BLUE LED
        case 3:
            PWM_Blue = cycle;
            TIMER_A0->CCR[0] = 100;
            TIMER_A0->CCTL[3] = 0x00E0;
            TIMER_A0->CCR[3] = PWM_Blue;
            TIMER_A0->CTL = 0x0214;
            break;
    }
}

// This turns on the motor at a given speed.
void timerMotor(int cycle)
{

    int period = 100;
    int time;

    // calculation to get a working PWM
    if (cycle != 0)
    {

        time = (cycle / 5) + 80;

        if (time >= 90)
        {

            time = 90;
        }
    }

    else if (cycle == 0)
    {

        time = 0;
    }


    Systick_Delay(500); // giving the motor enough time to turn on

    TIMER_A2->CCR[0] = period;
    TIMER_A2->CCR[4] = time;
    TIMER_A2->CCTL[4] = 0x00E0;
    TIMER_A2->CTL = 0x0214;
}

// turns the servo by the specific degrees, tested duty cycle values
// were entred
void servoMotor(int degree)
{

    int input;
    P6->SEL0 |= BIT6;
    P6->SEL1 &=~ BIT6;
    P6->DIR |= BIT6;

    input = degree;
    TIMER_A2->CCR[0] = 60000 - 1;
    TIMER_A2->CCR[3] = input;
    TIMER_A2->CCTL[3] = 0x00E0;
    TIMER_A2->CTL = 0x0214;

    Systick_Delay(1000);

}

// Port 1 interrupts for the 2 pushbuttons
void PORT1_IRQHandler()
{

    if (P1->IFG & BIT7)
    {

        // quick debounce
        Systick_Delay(10);
        while(!(P1->IN & BIT7));
        Systick_Delay(5);
        while(!(P1->IN & BIT7));

        // storing PWM values and setting PWMs of LED to 0
        if (i == 0)
        {
            tempR = PWM_Red;
            tempB = PWM_Blue;
            tempG = PWM_Green;

            timerRGB(1,0);
            timerRGB(2,0);
            timerRGB(3,0);

            i=1;
        }

        // reinstating the stored conditions from RGB LED
        else if(i == 1)
        {

            PWM_Red = tempR;
            PWM_Blue = tempB;
            PWM_Green = tempG;

            timerRGB(1,PWM_Red);
            timerRGB(3, PWM_Blue);
            timerRGB(2, PWM_Green);

            i=0;
        }
    }

    // emergency stop push button for motor
    if (P1->IFG & BIT6)
    {

        timerMotor(0);
    }

    // clearing the flag bits, BIT7 cleared twice due to issues
    P1->IFG &=~ (BIT6|BIT7);
    P1->IFG &=~ BIT7;
}
