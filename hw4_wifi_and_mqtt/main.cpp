#include "mbed.h"
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
Serial pc(USBTX, USBRX);
Thread accel_thread;

float sample_time[RECORD_TIMES];
float x_acc[RECORD_TIMES], y_acc[RECORD_TIMES], z_acc[RECORD_TIMES];
int total_sample = 0;
float current_time = 0;
bool is_tilting = false;
int m_addr = FXOS8700CQ_SLAVE_ADDR1;

void rpc_sample_num(Arguments *in, Reply *out);
RPCFunction sample_num(&rpc_sample_num, "sample_num");
void read_accelero();
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);

int main() {
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\r\n");
        return -1;
    }
    printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\r\n", ret);
        return -1;
    }
    NetworkInterface* net = wifi;
    MQTTNetwork mqttNetwork(net);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    //TODO: revise host to your ip
    const char* host = "192.168.43.209";
    printf("Connecting to TCP network...\r\n");
    int rc = mqttNetwork.connect(host, 1883);
    if (rc != 0) {
        printf("Connection error.");
        return -1;
    }
    printf("Successfully connected!\r\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";
    if ((rc = client.connect(data)) != 0){
        printf("Fail to connect MQTT\r\n");
    }
    if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
        printf("Fail to subscribe\r\n");
    }
    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    Ticker ticker;
    ticker.attach(mqtt_queue.event(&publish_message, &client), 0.5f);
    int num = 0;
    while (num != 5) {
        client.yield(100);
        ++num;
    }
    accel_thread.start(read_accelero);
    while (1) {
        pc.printf("hello\r\n");
        wait(5);
    }
}
void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char msg[300];
    sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf(msg);
    wait_ms(1000);
    char payload[300];
    sprintf(payload, "Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    printf(payload);
    ++arrivedcount;
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    while(1) {
        message_num++;
        MQTT::Message message;
        char buff[100];
        sprintf(buff, "FXOS8700Q ACC: X=%1.4f Y=%1.4f Z=%1.4f #%d\r\n",\
                  t[0], t[1], t[2], message_num);
        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*) buff;
        message.payloadlen = strlen(buff) + 1;
        int rc = client->publish(topic, message);
        printf("rc:  %d\r\n", rc);
        printf("Puslish message: %s\r\n", buff);
    }
}

void rpc_sample_num (Arguments *in, Reply *out)  {
    static prev_sec_sample = 0;
    int this_sec_sample = 0;
    this_sec_sample = total_sample - prev_sec_sample;
    prev_sec_sample = total_sample;
    pc.printf("%d/r/n", this_sec_sample);
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
        x_acc[total_sample] = ((float)acc16) / 4096.0f;
        acc16 = (res[2] << 6) | (res[3] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        y_acc[total_sample] = ((float)acc16) / 4096.0f;
        acc16 = (res[4] << 6) | (res[5] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        z_acc[total_sample] = ((float)acc16) / 4096.0f;
        if ((z_acc[total_sample] / sqrt(x_acc[total_sample] * x_acc[total_sample]
             + y_acc[total_sample] * y_acc[total_sample] + z_acc[total_sample] * z_acc[total_sample])) < 0.7071)
            is_tilting = true;
        else is_tilting = false;
        sample_time[total_sample] = current_time;
        printf("#%d %d %f ACC: X=%1.4f Y=%1.4f Z=%1.4f\r\n",\
            total_sample, is_tilting, sample_time[total_sample], x_acc[total_sample], y_acc[total_sample], z_acc[total_sample]);
        total_sample++;
        if (is_tilting) {
            accel_thread.wait(100);
            current_time = current_time + 0.1;
            
        }
        else {
            accel_thread.wait(500);
            current_time = current_time + 0.5;
        }
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