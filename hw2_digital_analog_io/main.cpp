#include "mbed.h"
#define max_voltage 3.3
#define pi 3.1415926535897
Serial pc( USBTX, USBRX );
AnalogOut Aout(DAC0_OUT);
AnalogIn Ain(A0);
DigitalIn  Switch(SW3);
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

float ADCdata[500];
float input_amplitude = 1.0;
float offset = 1.0;
int sampling_rate = 500;
int frequency = 0;
int i;

void DAC_output (int freq) {
  float t = 0;
  double sample = 0;
  while (1) {
    if (Switch == 0)
      break;
    sample = ((0.5 * (sin(2*pi*freq*t))) + 0.5);
    Aout.write(sample);
    t = t + 0.0002;
    wait(0.0002);
  }
  Aout.write(0);
  return;
}

void freq_display(int value) {
  int dis_val = 0;
  dis_val = value / 100;
  display = table[dis_val];
  wait(0.5);
  dis_val = (value % 100) / 10;
  display = table[dis_val];
  wait(0.5);
  dis_val = value % 10;
  display = table[dis_val] + 0x80;
  wait(0.5);
  return;
}

int main() {
  int count = 0;
  Aout.write(0);
  greenLED = 0;
  redLED = 1;
  while(1) {
    Aout.write(0);
    count = 0;
    for (i = 0; i < sampling_rate; i++) {
      Aout = Ain;
      ADCdata[i] = Ain;
      wait(0.967/sampling_rate);
    }
    for (i = 0; i < sampling_rate; i++) {
      pc.printf("%1.3f\r\n",ADCdata[i]);
      if (ADCdata[i] <= (input_amplitude / max_voltage) / 2 && ADCdata[i + 1] >= (input_amplitude / max_voltage) / 2) {
        count++;
      }
    }
    frequency = count;
    if( Switch == 0 ) {
      greenLED = 1;
      redLED = 0;
      freq_display(frequency);
    }
    else {
      greenLED = 0;
      redLED = 1;
      display = 0x00;
      DAC_output(frequency);
    }
  }
  return 0;
}