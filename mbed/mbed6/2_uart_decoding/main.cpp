#include "mbed.h"

Serial device(D12, D11);  // tx, rx

int main() {
  char cmd[10] = "PicoScope";
  int i = 0;
  while(1) {
    device.baud(9600);
    device.putc(cmd[i]);
    i++;
    if (i == 10)
        i = 0;
  }
}