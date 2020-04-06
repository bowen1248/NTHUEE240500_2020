#include "mbed.h"

DigitalOut led3 (LED3);
Ticker ticker1;

void flip_led3() {
    led3 = !led3;
}
int main () {
    ticker1.attach(&flip_led3, 0.5);
}