#include "mbed.h"
// pinout model
LED1 == red led  // 1 is off, 0 is on
LED2 == green led
SW2 // 1 is unpressed, 0 is pressed
SW3
// 5 interrupt
// interrupt
// using ISR, only have limited ability, if heavy work is needed
// use eventqueue
InterruptIn button(SW2);
button.rise(&your_function_pointer);
button.fall(&your_function_pointer);
// timer
// simply start record time
Timer t;
t.start();
t.stop();
t.read();   // in float
t.read_ms();    // in int
t.read_us();    // in int
t.reset();  // get timer to 0, will start running

// timeout
// 
Timeout tout;
// to call function after seconds
tout.attach(&trigger_function_pointer, seconds); // seconds can be float
tout.attach(callback(&your_class_object, &your_class::your_function_in_class), seconds); 

// ticker
// repeatedly call a function
Ticker time_up;
time_up.attach(&trigger_function_pointer, seconds); // seconds can be float
time_up.attach(callback(&your_class_object, &your_class::your_function_in_class), seconds); 
time_up.detach();

// thread
Thread thread1(osPriority);
thread1.start(&your_function_pointer);
thread1.start((callback(your_function_pointer, argument_to your_function));
thread1.join(); // wait for thread1 to 
thread1.terminate();    // terminate a thread now

// eventqueue
EventQueue queue;
queue.call(printf, "called immediately\r\n");
queue.call_in(2000, printf, "called in 2 seconds\r\n");
queue.call_every(1000, printf, "called every 1 seconds\r\n");
// continue dispatch eventqueue by thread
thread.start(callback(&queue, &EventQueue::dispatch_forever));
queue.event(function_pointer)
