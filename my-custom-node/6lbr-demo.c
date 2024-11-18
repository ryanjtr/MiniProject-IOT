

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"

#include <stdio.h>
#include <stdlib.h>

#define UDP_SERVER_LISTEN_PORT 3000

#define LED_OFF 0
#define LED_ON 1

#define CHANGE_LUX_GREEN_LED "c_lux"
#define SLS_GET_GW_STATUS "get_gw_status"

/*---------------------------------------------------------------------------*/
typedef enum
{
    LED_GREEN_ON = 1,
    LED_GREEN_OFF,
    LED_RED_ON,
    LED_RED_OFF,
    LED_BLUE_ON,
    LED_BLUE_OFF,
    ALL_LED_ON,
    ALL_LED_OFF,
    GET_NETWORK_STATUS,
    GET_GREEN_LED_STATUS,
    GET_RED_LED_STATUS,
    GET_BLUE_LED_STATUS
} command_t;

struct led_struct_t
{
    uint16_t id;
    uint16_t voltage;
    uint16_t power;
    uint16_t lux;
    uint8_t status;
};

struct net_struct_t
{
    radio_value_t radio;
    uint8_t channel;
    int8_t rssi;
    int8_t tx_power;
    int8_t lqi;
    uint16_t panid;
};

/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

static struct net_struct_t net_db;
static struct led_struct_t green_led;
static struct led_struct_t blue_led;
static struct led_struct_t red_led;

static char str_reply[100];
static char str_cmd[10];
static command_t cmd;
static char str_status_led[10];
static radio_value_t aux;

/* define prototype of fucntion call */
static void get_radio_parameter(void);
static void init_default_parameters(void);
static void reset_parameters(void);
static void handle_command(int command);
// static char *p;

/*---------------------------------------------------------------------------*/
PROCESS(udp_echo_server_process, "UDP echo server process");

/*---------------------------------------------------------------------------*/
static void tcpip_handler(void)
{

    memset(buf, 0, MAX_PAYLOAD_LEN);
    if (uip_newdata())
    {

        len = uip_datalen();
        memcpy(buf, uip_appdata, len); // copy payload of uip_appdata to buf
        PRINTF("Received from [");
        PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
        PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));

        uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr); // copy IP of source
        server_conn->rport = UIP_UDP_BUF->srcport;                      // setting port of client

        get_radio_parameter();
        reset_parameters();

        strcpy(str_cmd, buf);

        cmd = atoi(str_cmd);
        handle_command(cmd);

        /* echo back to sender */
        PRINTF("Echo back to [");
        PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
        PRINTF("]:%u %u bytes\n", UIP_HTONS(UIP_UDP_BUF->srcport), sizeof(str_reply));
        uip_udp_packet_send(server_conn, str_reply, sizeof(str_reply));
        uip_create_unspecified(&server_conn->ripaddr);
        server_conn->rport = 0;
    }
    return;
}

static void reset_parameters(void)
{
    memset(&str_cmd[0], 0, sizeof(str_cmd));
    memset(&str_reply[0], 0, sizeof(str_reply));
}

/*---------------------------------------------------------------------------*/
static void get_radio_parameter()
{
    NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
    net_db.channel = (unsigned int)aux;

    aux = packetbuf_attr(PACKETBUF_ATTR_RSSI);
    net_db.rssi = (int8_t)aux;

    aux = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    net_db.lqi = aux;

    NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &aux);
    net_db.tx_power = aux;
}

/*---------------------------------------------------------------------------*/
static void init_default_parameters(void)
{
    green_led.id = 1;
    green_led.voltage = 3;
    green_led.power = 66; // mW
    green_led.lux = 100;
    green_led.status = LED_OFF;

    red_led.id = 2;
    red_led.voltage = 3;
    red_led.power = 56; // mW
    red_led.lux = 120;
    red_led.status = LED_OFF;

    blue_led.id = 3;
    blue_led.voltage = 3;
    blue_led.power = 60; // mW
    blue_led.lux = 150;
    blue_led.status = LED_OFF;

    net_db.panid = 0xABCD;
}
/*---------------------------------------------------------------------------*/
void handle_command(int command)
{
    switch (cmd)
    {
    case LED_GREEN_ON:
        PRINTF("Execute CMD = %d\n", LED_GREEN_ON);
        leds_on(LEDS_GREEN);
        sprintf(str_reply, "Replied = LED GREEN ON\n");
        green_led.status = LED_ON;
        break;
    case LED_GREEN_OFF:
        PRINTF("Execute CMD = %d\n", LED_GREEN_OFF);
        leds_off(LEDS_GREEN);
        sprintf(str_reply, "Replied = LED GREEN OFF\n");
        green_led.status = LED_OFF;
        break;
    case LED_RED_ON:
        PRINTF("Execute CMD = %d\n", LED_RED_ON);
        leds_on(LEDS_RED);
        sprintf(str_reply, "Replied = LED RED ON\n");
        red_led.status = LED_ON;
        break;
    case LED_RED_OFF:
        PRINTF("Execute CMD = %d\n", LED_RED_OFF);
        leds_off(LEDS_RED);
        sprintf(str_reply, "Replied = LED RED OFF\n");
        red_led.status = LED_ON;
        break;
    case LED_BLUE_ON:
        PRINTF("Execute CMD = %d\n", LED_BLUE_ON);
        leds_on(LEDS_BLUE);
        sprintf(str_reply, "Replied = LED BLUE ON\n");
        blue_led.status = LED_ON;
        break;
    case LED_BLUE_OFF:
        PRINTF("Execute CMD = %d\n", LED_BLUE_OFF);
        leds_off(LEDS_BLUE);
        sprintf(str_reply, "Replied = LED BLUE OFF\n");
        blue_led.status = LED_OFF;
        break;
    case ALL_LED_ON:
        PRINTF("Execute CMD = %d\n", ALL_LED_ON);
        leds_on(LEDS_RED);
        leds_on(LEDS_BLUE);
        leds_on(LEDS_GREEN);
        sprintf(str_reply, "Replied = ALL LED ON\n");
        red_led.status = LED_ON;
        blue_led.status = LED_ON;
        green_led.status = LED_ON;
        break;
    case ALL_LED_OFF:
        PRINTF("Execute CMD = %d\n", ALL_LED_OFF);
        leds_off(LEDS_RED);
        leds_off(LEDS_BLUE);
        leds_off(LEDS_GREEN);
        sprintf(str_reply, "Replied = ALL LED OFF\n");
        red_led.status = LED_OFF;
        blue_led.status = LED_OFF;
        green_led.status = LED_OFF;
        break;
    case GET_NETWORK_STATUS:
        PRINTF("Execute CMD = %d\n", GET_NETWORK_STATUS);
        sprintf(str_reply, "Replied: channel=%u|rssi=%ddBm|lqi=%u|tx_power=%ddBm|panid=0x%02X|\n",
                net_db.channel, net_db.rssi, net_db.lqi, net_db.tx_power, net_db.panid);
        break;
    case GET_GREEN_LED_STATUS:
        PRINTF("Execute CMD = %d\n", GET_GREEN_LED_STATUS);
        if (green_led.status)
            sprintf(str_status_led, "ON");
        else
            sprintf(str_status_led, "OFF");
        sprintf(str_reply, "Replied: Id=%d|Vol=%dV|Pow=%dW|Lux=%d|Status=%s|\n",
                green_led.id, green_led.voltage, green_led.power, green_led.lux, str_status_led);
        break;
    case GET_RED_LED_STATUS:
        PRINTF("Execute CMD = %d\n", GET_RED_LED_STATUS);
        if (red_led.status)
            sprintf(str_status_led, "ON");
        else
            sprintf(str_status_led, "OFF");
        sprintf(str_reply, "Replied: Id=%d|Vol=%dV|Pow=%dW|Lux=%d|Status=%s|\n",
                red_led.id, red_led.voltage, red_led.power, red_led.lux, str_status_led);
        break;
    case GET_BLUE_LED_STATUS:
        PRINTF("Execute CMD = %d\n", GET_BLUE_LED_STATUS);
        if (blue_led.status)
            sprintf(str_status_led, "ON");
        else
            sprintf(str_status_led, "OFF");
        sprintf(str_reply, "Replied: Id=%d|Vol=%dV|Pow=%dW|Lux=%d|Status=%s|\n",
                blue_led.id, blue_led.voltage, blue_led.power, blue_led.lux, str_status_led);
        break;

    default:
        sprintf(str_reply, "Replied = BAD COMMAND\n");
    }

    PRINTF("%s\n", str_reply);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_echo_server_process, ev, data)
{

    PROCESS_BEGIN();

    PRINTF("Initialization....\n");
    init_default_parameters();

    PRINTF("Starting UDP echo server\n");

    server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));

    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

    while (1)
    {
        PROCESS_YIELD();
        if (ev == tcpip_event)
        {
            tcpip_handler();
        }
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_NAME(demo_6lbr_process);
AUTOSTART_PROCESSES(&demo_6lbr_process, &udp_echo_server_process);

/*---------------------------------------------------------------------------*/
