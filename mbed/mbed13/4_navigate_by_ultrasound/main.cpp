#include "mbed.h"
#include "bbcar.h"

DigitalOut led1(LED1);
PwmOut pin9(D9), pin8(D8);
DigitalInOut pin10(D10);
Ticker servo_ticker;
BBCar car(pin8, pin9, servo_ticker);
Serial pc(USBTX, USBRX);

int main() {
    parallax_ping  ping1(pin10);
    while(1) {
        float cm = ping1;
        pc.printf("%f\r\n", cm);
        wait_ms(100);
     }
 }