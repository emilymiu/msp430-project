#include <msp430.h> 
/**
 * main.c
 */

#define     LED1                  BIT0            //P1.0
#define     LED2                  BIT7            //P4.7
#define     LED3                  BIT6            //P6.6

#define     ECHO                   BIT3           //P2.3/TA2.0
#define     TRIG                   BIT2           //P8.2
#define     LHBRIDGEPWM            BIT4           //P1.4/TA0.3
#define     LFORWARD               BIT4           //P2.4  1A
#define     LBACKWARD              BIT5           //P2.5  2A

#define     RHBRIDGEPWM            BIT5           //P1.5/TA0.4
#define     RFORWARD               BIT7           //P2.7 3A
#define     RBACKWARD              BIT2           //P2.2 4A

#define     BEEPPWM                BIT0           //P2.0/TA1.1

void InitMotors()
{
    // Port Configuration
    P2SEL  &= ~BIT6;  // Clear P2.6 in P2SEL  (by default Xin)

    // Motor Control pins
    P2DIR |= LFORWARD | LBACKWARD;

    P2DIR |= RFORWARD | RBACKWARD;

}

void InitSensor()
{

    P1DIR |= 0b00010000; // set pin P1.3 as input and P1.4 as output
}

void InitMotorPWM()
{
    P1DIR |= BIT4; // Output on Pin 1.4
    P1DIR |= BIT5; // Output on Pin 1.5
    P1SEL |= BIT4; // Pin 1.4 selected as PWM
    P1SEL |= BIT5; // Pin 1.5 selected as PWM

    TA0CCR0 = 512; // PWM period (square)

    TA0CCR3 = 512; // PWM duty cycle (amount of time of PWM that is high voltage)
    TA0CCTL3 = OUTMOD_7; // TA0CCR3 reset/set-high voltage
    TA0CCR4 = 512; // PWM duty cycle (amount of time of PWM that is high voltage)
    TA0CCTL4 = OUTMOD_7; // TA0CCR4 reset/set-high voltage

    TA0CTL = TASSEL_2 + MC_1 + TAIE + ID_0; // start timing
}

void InitBeeperPWM()
{
    P2DIR |= BIT0; // Output on Pin 2.0
    P2SEL |= BIT0; // Pin 2.0 selected as PWM

    TA1CCR0 = 512; // PWM period (square)
    TA1CCR1 = 512; // PWM duty cycle (amount of time of PWM that is high voltage)
    TA1CCTL1 = OUTMOD_7; // TA0CCR1 reset/set-high voltage

    TA1CTL = TASSEL_2 + MC_1 + TAIE + ID_0; // start timing
}

void turnLeft()
{
    // Rotate the Right Motor clockwise
    P2OUT |=  RFORWARD;               // P2.0 = 1
    P2OUT &= ~RBACKWARD;             // P2.2 = 0

    // Rotate the Left Motor counterclockwise
    P2OUT &=  ~LFORWARD;            // P2.4 = 0
    P2OUT |= LBACKWARD;             // P2.5 = 1

}

void turnRight()
{
    // Rotate the Left Motor clockwise
    P2OUT |=  LFORWARD;               // P2.4 = 1
    P2OUT &= ~LBACKWARD;             // P2.5 = 0

    // Rotate the Right Motor counterclockwise
    P2OUT &=  ~RFORWARD;            // P2.4 = 0
    P2OUT |= RBACKWARD;             // P2.5 = 1

}

void letsGo()
{
    // Rotate the Left Motor clockwise
    P2OUT |=  LFORWARD;              // P2.4 = 1
    P2OUT &=  ~LBACKWARD;           //  P2.5 = 0

    // Rotate the Right Motor clockwise
    P2OUT |=  RFORWARD;               // P2.0 = 1
    P2OUT &= ~RBACKWARD;             // P2.2 = 0

}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	P1DIR |= LED1;  // set LED
	P4DIR |= LED2;
	P6DIR |= LED3;

	InitMotors();

	InitSensor();

	InitMotorPWM();

	InitBeeperPWM();

	letsGo();

	P8DIR |= TRIG; // set trig for sensor

    while(1)
    {
        float distance;

        P8OUT = TRIG;

        _delay_cycles(20);

        P8OUT &= ~TRIG;

        P1OUT |= LED1;  // toggle the light every time we make a measurement.
        P4OUT &= ~LED2; // make sure that green LED is off

        while(!(P2IN & ECHO)); // wait until echo pin is high

        TA2CTL = TASSEL_2 | MC_2 | TACLR | ID_0; // start timing when echo pin is high

        P1OUT &= ~LED1;  // toggle the light every time we make a measurement.
        P4OUT &= ~LED2; // make sure that green LED is off

        while((P2IN & ECHO)); // when echo pin is 0, get the distance

        distance = (float)TA2R*0.000343/2;   // get distance by updating TA0R

        P1OUT |= LED1;  // toggle the light every time we make a measurement.
        P4OUT |= LED2; // make sure that green LED is off

        letsGo();

        if (distance < 0.25) {
            TA1CCR0 = 1000; // PWM period (square)
            TA1CCR1 = 500; // PWM duty cycle (amount of time of PWM that is high voltage)
        } else {
            TA1CCR0 = 512; // PWM period (square)
            TA1CCR1 = 512; // PWM duty cycle (amount of time of PWM that is high voltage)
        }

        if (distance < 0.1) {
            TA1CCR0 = 512; // PWM period (square)
            TA1CCR1 = 256; // PWM duty cycle (amount of time of PWM that is high voltage)
            turnLeft();
            P6OUT |= LED3;  // toggle the light
            _delay_cycles(500000);
            P6OUT &= ~LED3; // toggle the light
            _delay_cycles(500000);
            P6OUT |= LED3;  // toggle the light
            _delay_cycles(500000);
            P6OUT &= ~LED3; // toggle the light
            _delay_cycles(500000);
        } else {
            letsGo();
        }

        letsGo();

        _delay_cycles(100000);
    }
}
