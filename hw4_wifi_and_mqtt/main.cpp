#include "mbed.h"
#include <stdlib.h>
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "mbed_rpc.h"

#define UINT14_MAX 16383
#define RECORD_TIMES 2000
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
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
Thread accel_thread;
Thread xbee_thread;
EventQueue xbee_queue;

float sample_time[RECORD_TIMES];
float x_acc[RECORD_TIMES], y_acc[RECORD_TIMES], z_acc[RECORD_TIMES];
int total_sample = 0;
float current_time = 0;
bool is_tilting = false;
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
int first_time_sample = 0;
int get_rpc_times = 0;

void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);
void rpc_sample_num(Arguments *in, Reply *out);
RPCFunction sample_num(&rpc_sample_num, "get_sample_num");
void read_accelero();
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);

int main() {
    pc.baud(9600);
    char xbee_reply[4];
    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 0x141\r\n");
    reply_messange(xbee_reply, "setting MY : 0x141");

    xbee.printf("ATDL 0x140\r\n");
    reply_messange(xbee_reply, "setting DL : 0x140");

    xbee.printf("ATID 0x3\r\n");
    reply_messange(xbee_reply, "setting PAN ID : 0x3");

    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");

    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");

    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");

    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");

    xbee.getc();
    // start
    pc.printf("start\r\n");
    xbee_thread.start(callback(&xbee_queue, &EventQueue::dispatch_forever));

    // Setup a serial interrupt function of receiving data from xbee
    xbee.attach(xbee_rx_interrupt, Serial::RxIrq);

    accel_thread.start(read_accelero);
    while (1) {wait(1);}
    return 0;
}

void xbee_rx_interrupt(void) {
    xbee.attach(NULL, Serial::RxIrq); // detach interrupt
    xbee_queue.call(&xbee_rx);
}

void xbee_rx(void) {
    char buf[100] = {0};
    char outbuf[100] = {0};
    while(xbee.readable()){
        for (int i=0; ; i++) {
            char recv = xbee.getc();
            if (recv == '\r' || recv == '\n') {
                break;
            }
            buf[i] = recv;
        }
        RPC::call(buf, outbuf);
    }
    xbee.attach(xbee_rx_interrupt, Serial::RxIrq); // reattach interrupt
}

void reply_messange(char *xbee_reply, char *messange){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
   pc.printf("%s\r\n", messange);
   xbee_reply[0] = '\0';
   xbee_reply[1] = '\0';
   xbee_reply[2] = '\0';
  }
}


void check_addr(char *xbee_reply, char *messenger){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  xbee_reply[3] = xbee.getc();
  pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
  xbee_reply[0] = '\0';
  xbee_reply[1] = '\0';
  xbee_reply[2] = '\0';
  xbee_reply[3] = '\0';
}

void rpc_sample_num (Arguments *in, Reply *out)  {
    static int prev_sec_sample = 0;
    int this_sec_sample = 0;
    if (get_rpc_times == 0) 
        first_time_sample = total_sample;
    this_sec_sample = total_sample - prev_sec_sample;
    prev_sec_sample = total_sample;
    xbee.printf("%d\r\n", this_sec_sample);
    get_rpc_times++;
    if (get_rpc_times >= 21) {
        accel_thread.terminate();
        for (int i = first_time_sample; i < first_time_sample + 40; i++) {
            xbee.printf("%f\n", x_acc[i]);
            //pc.printf("%f\r\n", x_acc[i]);
            wait(0.05);
            xbee.printf("%f\n", y_acc[i]);
            //pc.printf("%f\r\n", y_acc[i]);
            wait(0.05);
            xbee.printf("%f\n", z_acc[i]);
            //pc.printf("%f\r\n", z_acc[i]);
            wait(0.05);
        }
    }
}
void read_accelero() {
    uint8_t who_am_i, data[2], res[6];
    int16_t acc16;
    // Enable the FXOS8700Q
    FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data, 2);
    // Get the slave address
    FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);
    while (true) {
        FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
        acc16 = (res[0] << 6) | (res[1] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        x_acc[total_sample] = acc16 / 4096.0;
        acc16 = (res[2] << 6) | (res[3] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        y_acc[total_sample] = acc16 / 4096.0;
        acc16 = (res[4] << 6) | (res[5] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        z_acc[total_sample] = acc16 / 4096.0;
        /* if ((z_acc[total_sample] / sqrt(x_acc[total_sample] * x_acc[total_sample]
             + y_acc[total_sample] * y_acc[total_sample] + z_acc[total_sample] * z_acc[total_sample])) < 0.7071)
            is_tilting = true;
        else is_tilting = false; */
        // sample_time[total_sample] = current_time;
        //printf("#%d ACC: X=%d Y=%d Z=%d\r\n",\
          //  total_sample, x_acc[total_sample], y_acc[total_sample], z_acc[total_sample]);
        total_sample++;
        accel_thread.wait(500);
        current_time = current_time + 0.5;
    }
}
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}