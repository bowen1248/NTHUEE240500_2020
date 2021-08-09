#include "mbed.h"
#include "bbcar.h"

Ticker servo_ticker;
Ticker encoder_ticker;

Serial pc(USBTX, USBRX);
PwmOut pin8(D8), pin9(D9);
DigitalIn pin3(D3);
BBCar car(pin8, pin9, servo_ticker);

int main() {
    parallax_encoder encoder0(pin3, encoder_ticker);
    encoder0.reset();
    car.goStraight(100);
    while(1) { 
        float cm = encoder0.get_cm();
        pc.printf("%f\r\n", cm);
        wait_ms(100);
    }
    car.stop();
}
