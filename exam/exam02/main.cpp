#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

#define UINT14_MAX 16383
#define pi 3.14159265
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);
DigitalIn button(SW3);
DigitalOut led(LED1);
Thread t;
EventQueue queue;

// global variable
// counting which array place to log in
int counting = 0;
int i, j;
float xval[100], yval[100], zval[100];
int movement[100];
float displacement[100]; // current displacement
int m_addr = FXOS8700CQ_SLAVE_ADDR1;

// function
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void read_gravity();

int main() {
    // set thread and baud rate
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    pc.baud(9600);

    // big loop for repeat reading sequence
    while(1) {
        // set led to light
        led = 0;
        counting = 0;
        while (1) {
            if (button == 0)
                break;
        }
        // log in to arrays
        for (j = 0; j < 100; j++) {
            wait_ms(100);
            queue.call(&read_gravity);
        }
        for (i = 0; i < 100; i++) {
            pc.printf("%f\r\n", xval[i]);
            pc.printf("%f\r\n", yval[i]);
            pc.printf("%f\r\n", zval[i]);
            pc.printf("%d\r\n", movement[i]);
        }
    }
    return 0;
}

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}
void read_gravity () {
    uint8_t who_am_i, data[2], res[6];
    int16_t acc16;
    float t[3];
    // print out value every 1s
    // Enable the FXOS8700Q
    FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data, 2);
    // Get the slave address
    FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);
    FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
    // process data into float
    acc16 = (res[0] << 6) | (res[1] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[0] = ((float)acc16) / 4096.0f;
    acc16 = (res[2] << 6) | (res[3] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[1] = ((float)acc16) / 4096.0f;
    acc16 = (res[4] << 6) | (res[5] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[2] = ((float)acc16) / 4096.0f;
    // log into array
    if (counting < 100) {
        led = !led;
        xval[counting] = t[0];
        yval[counting] = t[1];
        zval[counting] = t[2];
    }
    if (counting == 0) // first number
        displacement[counting] = sqrt((t[0] * t[0] * 9.8 * 4.9) * (t[0] * t[0] * 9.8 * 4.9) + (t[1] * t[1] * 9.8 * 4.9) * (t[1] * t[1] * 9.8 * 4.9));
    else if (xval[counting] * xval[counting - 1] < 0 || yval[counting] * yval[counting - 1] < 0) // change direction
        displacement[counting] = sqrt((t[0] * t[0] * 9.8 * 4.9) * (t[0] * t[0] * 9.8 * 4.9) + (t[1] * t[1] * 9.8 * 4.9) * (t[1] * t[1] * 9.8 * 4.9));
    else 
        displacement[counting] = displacement[counting - 1] + sqrt((t[0] * t[0] * 9.8 * 4.9) * (t[0] * t[0] * 9.8 * 4.9) + (t[1] * t[1] * 9.8 * 4.9) * (t[1] * t[1] * 9.8 * 4.9));
    if (displacement[counting] >= 5)
        movement[counting] = 1;
    else movement[counting] = 0;
    counting++;
}