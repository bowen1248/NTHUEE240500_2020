#include "mbed.h"

#include "mbed_events.h"


DigitalOut led1(LED1);

DigitalOut led2(LED2);

InterruptIn btn(SW2);


EventQueue printfQueue;

EventQueue eventQueue;


void blink_led2() {
while(1) {
  // this runs in the normal priority thread

  led2 = !led2;
}
}


void print_toggle_led() {

  // this runs in the lower priority thread

  printf("Toggle LED!\r\n");

}


void btn_fall_irq() {

  led1 = !led1;


  // defer the printf call to the low priority thread

  printfQueue.call(&print_toggle_led);

}


int main() {

  // low priority thread for calling printf()

  Thread printfThread(osPriorityLow);

  printfThread.start(callback(&printfQueue, &EventQueue::dispatch_forever));


  // normal priority thread for other events

  Thread eventThread(osPriorityNormal);

  eventThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));


  // call blink_led2 every second, automatically defering to the eventThread
    while(1) {
  eventQueue.event(&blink_led2);
wait(1);
    }

  // button fall still runs in the ISR

  btn.fall(&btn_fall_irq);


  while (1) {wait(1);}

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