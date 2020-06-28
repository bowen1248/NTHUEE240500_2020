#include "mbed.h"
#define CENTER_BASE 1500

Serial pc(USBTX, USBRX);
DigitalIn encoder(D10);
PwmOut servo(D11);

Timer t;
Ticker encoder_ticker;

volatile int steps;
volatile int last;

void servo_control(int speed){
    if (speed > 200)       speed = 200;
    else if (speed < -200) speed = -200;
    servo = (CENTER_BASE + speed)/20000.0f;
}

void encoder_control(){
    int value = encoder;
    if(!last && value) steps++;
    last = value;
}

int main() {
    pc.baud(9600);
    encoder_ticker.attach(&encoder_control, 0.001);
    servo.period(.02);
    float time;
    float speed_cm;
    float clockwise = -28.7;
    float counterclockwise = 39;
    while(1) {
        servo_control(clockwise);
        steps = 0;
        t.reset();
        t.start();
        wait(5);
        time = t.read();
        speed_cm = steps * 6.5 * 3.14 / 32 / time;
        pc.printf("%1.3f\r\n", speed_cm);
        servo_control(counterclockwise);
        steps = 0;
        t.reset();
        t.start();
        wait(5);
        time = t.read();
        speed_cm = steps * 6.5 * 3.14 / 32 / time;
        pc.printf("%1.3f\r\n", speed_cm);
    }
    servo_control(0);
    while(1);
}