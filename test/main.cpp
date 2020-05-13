// uLCD-144-G2 basic text demo program for uLCD-4GL LCD driver library
//

#include "mbed.h"
#include "uLCD_4DGL.h"

Thread t2;
Ticker tick;
EventQueue queue(64 * EVENTS_EVENT_SIZE);
uLCD_4DGL uLCD(D1, D0, D2); // serial tx, serial rx, reset pin;

void taiko_games() {
    uLCD.circle(30, 110, 6, BLACK);
        for (int i = 0; i < 5; i++) {
          uLCD.circle(30, 70 + 10 * (i - 1) ,6, BLACK);
          uLCD.line(0, 50, 127, 50, GREEN);
          uLCD.circle(30, 70 + 10 * i , 6, WHITE);
          uLCD.line(0, 110, 127, 110, GREEN);
          wait(0.2);
        }
    }
int main() {
    t2.start(callback(&queue, &EventQueue::dispatch_forever));
    tick.attach(queue.event(taiko_games), 1);
    // basic printf demo = 16 by 18 characters on screen
}





















/*
void Taiko(void) {
    int x = 64;
    int y = 0;
    int score = 0;
    bool pointed = false;       // allow getting point only one time
    bool red = true;
    uLCD.background_color(BLACK);
    uLCD.cls();
    uLCD.line(0, 100, 127, 100, WHITE);
    uLCD.circle(64, 100, 10, WHITE);
    while(sw3) {
        if (red == true) {
            uLCD.filled_circle(x, y, 5, RED);
        }
        else {
            uLCD.filled_circle(x, y, 5, GREEN);
        }
        //
        if ((y > 85 && y < 115) && output == SLOPE && !pointed && red) {
            score += 2;
            pointed = true;
        }
        else if ((y > 70 && y < 115) && output == RING && !pointed && !red) {
            score += 2;
            pointed = true;
        }
        //
        if (y > 132) {
            uLCD.line(0, 100, 127, 100, WHITE);
            uLCD.circle(64, 100, 10, WHITE);
            pointed = false;
            red = !red;
            y = 0;
        }
        uLCD.filled_circle(x, y, 5, BLACK);
        uLCD.locate(1, 2);
        uLCD.printf("%d", score);
        y += 4;
    }
  */