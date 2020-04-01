// uLCD-144-G2 basic text demo program for uLCD-4GL LCD driver library
//
#include "mbed.h"
#include "uLCD_4DGL.h"

Serial pc( USBTX, USBRX );
PwmOut PWM1(D6);
AnalogIn Ain(A0);
uLCD_4DGL uLCD(D1, D0, D2); // serial tx, serial rx, reset pin;

float ADCdata[128];

int main()
{
    uLCD.background_color(0xFFFFFF); // white background
    uLCD.textbackground_color(0xFFFFFF);
    uLCD.cls();
    // basic printf demo = 16 by 18 characters on screen
    uLCD.color(BLUE);
    uLCD.printf("\n107061113\n"); //Default Green on black text
    uLCD.text_width(6); //4X size text
    uLCD.text_height(6);
    uLCD.color(GREEN);
    uLCD.locate(2,2);
    uLCD.filled_rectangle(32,32,96,96,GREEN);
    PWM1.period(0.001);
    while(1) {
        index = 0;
        for(float i = 0; i <= 1; i = i + 0.1) {
            PWM1 = i;
            wait(0.1);
            
                ADCdata[index] = Ain;
                pc.printf("%1.3f\r\n", ADCdata[index]);
                index++;
            
        }
        for(float i = 1; i >= 0; i = i - 0.1) {
            PWM1 = i;
            wait(0.1);
            
                ADCdata[index] = Ain;
                pc.printf("%1.3f\r\n", ADCdata[index]);
                index++;
            
        }
    }
}