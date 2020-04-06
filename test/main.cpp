#include "mbed.h"

DigitalOut led1(LED1);
InterruptIn sw(SW2);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

void rise_handler(void) {
    printf("rise_handler in context %p\r\n", Thread::gettid());
    // Toggle LED
    led1 = !led1;
}

void fall_handler(void) {
    printf("fall_handler in context %p\r\n", Thread::gettid());
    // Toggle LED
    led1 = !led1;
}

int main() {
    // Start the event queue
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    printf("Starting in context %p\r\n", Thread::gettid());
    // The 'rise' handler will execute in IRQ context
    sw.rise(rise_handler);
    // The 'fall' handler will execute in the context of thread 't'
    sw.fall(queue.event(fall_handler));
}