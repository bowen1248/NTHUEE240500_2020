#include "mbed.h"
#include "arm_math.h"
#include "FXOS8700CQ.h"
#include "bbcar.h"
#include <math.h>
#include <stdlib.h>
#define bound 0.9

DigitalOut redLED(LED1);
Serial uart(D1,D0); //tx,rx
Serial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
Ticker servo_ticker;
Ticker encoder_ticker;
Timer lock_turn_time;
Timer turn_time;
PwmOut pin9(D9), pin8(D8);
BBCar car(pin8, pin9, servo_ticker);
FXOS8700CQ acc(PTD9, PTD8, (0x1D<<1));
DigitalInOut pin10(D10);
DigitalIn pin3(D3);
DigitalIn pin4(D4);
parallax_ping  ping1(pin10);
parallax_encoder encoder0(pin3, encoder_ticker);
parallax_encoder encoder1(pin4, encoder_ticker);

float state[3] = {0};
float Kp = 0, Ki = 0, Kd = 0;
float a0 = 0, a1 = 0, a2 = 0;

//The formula is:
//y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]
//A0 = Kp + Ki + Kd
//A1 = (-Kp ) - (2 * Kd )
//A2 = Kd

void identify_obj() {
    redLED = 0;
    car.stop();
    wait(1);
    float data[5] = {5};
    car.turn(-26, 1);
    wait_ms(500);
    car.turn(25, 1);
    for(int i = 0; i < 5; i++) {
        data[i] = ping1;
        wait_ms(198);
    }
    /* for (int i = 0; i < 5; i++) {
        pc.printf("%f\r\n", data[i]);
    } */
    car.turn(-26,1);
    wait_ms(500);
    car.stop();
    if (data[0] > data[1] + 1 && data[2] + 1 < data[3])
        xbee.printf("This is twin triangle\r\n");
    else if (data[0] + 1 < data[1] && data[2] < data[3] + 1)
        xbee.printf("This is regular triangle\r\n");
    else if (data[0] > data[1] + 1 && data[2] > data[3] + 1)
        xbee.printf("This is right triangle\r\n");
    else xbee.printf("This is square\r\n");
    redLED = 1;
    return;
}
void advance() {
    car.goStraight(100);
    xbee.printf("Advancing\r\n");
}

void reverse() {
    car.goStraight(-100);
    xbee.printf("Reversing\r\n");
}
void turn_right() {
    car.stop();
    xbee.printf("Turn right\r\n");
    wait(0.5);
    encoder0.reset();
    encoder1.reset();
    turn_time.reset();
    turn_time.start();
    car.turn(-50, 1);
    while (encoder0.get_cm() + encoder1.get_cm() < 12 || turn_time.read() < 0.95) {
        if( turn_time.read() > 1)
            break;
    }
    car.stop();
    turn_time.stop();
    wait(0.5);
}

void turn_left() {
    car.stop();
    xbee.printf("Turn left\r\n");
    wait(0.5);
    encoder0.reset();
    encoder1.reset();
    turn_time.reset();
    turn_time.start();
    car.turn(50, 1);
    while (encoder0.get_cm() + encoder1.get_cm() < 12 || turn_time.read() < 0.95) {
        if(turn_time.read() > 1)
            break;
    }
    car.stop();
    turn_time.stop();
    wait(0.5);
}

int identify_num (void) {
    redLED = 0;
    wait(1);
    int result = 0;
    char buf[100] = {0};
    uart.putc('1');
    while (uart.readable()) {
        for (int i = 0; ; i++) {
            char recv = uart.getc();
            if (recv == '\r') {
                break;
            }
        buf[i] = recv;
        }
        result = atoi(buf);
    }
    xbee.printf("This is: %d\r\n", result);
    redLED = 1;
    wait(3);
    return result;
}
int main (void) {
    float dis;
    int result = 0;
    redLED = 1;
    encoder0.reset();
    uart.baud(9600);

    /*
    while(1) {
        turn_right();
        turn_left();
    }
    */
    // straight line
    advance();
    dis = ping1;
    lock_turn_time.reset();
    lock_turn_time.start();
    while ((dis < 5 || dis > 18) || lock_turn_time.read() < 8) {
        dis = ping1;
        pc.printf("%f\r\n", dis);
        wait_ms(1);
    }
    lock_turn_time.stop();
    turn_right();

    // into mission 1
    advance();
    wait(4.5);

    // reverse parking
    turn_left();
    reverse();
    wait(1.5);
    car.stop();
    wait(1);
    advance();
    wait(1);
    car.stop();

    // GO TO IDENTIFY PICTURE
    turn_left();
    reverse();
    wait(1.5);
    turn_right();
    result = identify_num();
    advance();
    wait(0.5);

    // leaving mission 1
    turn_left();
    advance();
    wait(4.2);
    turn_left();

    // long way to mission 2
    advance();
    dis = ping1;
    lock_turn_time.reset();
    lock_turn_time.start();
    while ((dis < 5 || dis > 15) || lock_turn_time.read() < 8) {
        dis = ping1;
        pc.printf("%f\r\n", dis);
        wait_ms(1);
    }
    lock_turn_time.stop();
    turn_left();

    // into mission 2
    advance();
    wait(3.6);
    turn_left();
    advance();
    wait(3);
    identify_obj();
    reverse();
    wait(0.8);
    turn_right();
    turn_right();
    advance();
    dis = ping1;
    while (dis < 5 || dis > 15) {
        dis = ping1;
        pc.printf("%f\r\n", dis);
        wait_ms(1);
    }
    turn_left();
    // leaving mission 2
    advance();
    dis = ping1;
    while (dis < 5 || dis > 15) {
        dis = ping1;
        pc.printf("%f\r\n", dis);
        wait_ms(1);
    }
    turn_left();
    advance();
    wait(12);
    car.stop();
    xbee.printf("END\r\n");
    return 0;
}