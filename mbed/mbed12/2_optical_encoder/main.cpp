#include "mbed.h"

Serial pc(USBTX, USBRX);
DigitalIn encoder(D12);

Ticker encoder_ticker;
volatile int steps;
volatile int last;

void encoder_control(){
    int value = encoder;
    if(!last && value) steps++;
    last = value;
}

int main() {
    pc.baud(9600);
    pc.printf("Before start\r\n");
    encoder_ticker.attach(&encoder_control, 0.001);
    steps = 0;
    last = 0;
    while(1) {
        wait(2);
        pc.printf("encoder = %d\r\n", steps);
    }
}