#include "contiki.h"
#include "net/routing/routing.h" // For setting border router to root
#include "net/netstack.h" // Start TSCH
#include "net/ipv6/simple-udp.h" // To send UDP packages to RPi

#include "dev/gpio-hal.h" // For pulling GPIO pins low
#include "arch/platform/simplelink/cc13xx-cc26xx/launchpad/cc2650/Board.h"

#include "sys/log.h" // For simple logging
#define LOG_MODULE "AoI Border Router"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

gpio_hal_pin_t out_pin1 = Board_PIN_RLED;
gpio_hal_pin_t out_pin2 = Board_PIN_GLED;

//extern gpio_hal_pin_t out_pin1, out_pin2, out_pin3;
//extern gpio_hal_pin_t btn_pin;

#if !GPIO_HAL_PORT_PIN_NUMBERING
extern gpio_hal_port_t out_port1, out_port2_3;
extern gpio_hal_port_t btn_port;
#else
#define out_port1   GPIO_HAL_NULL_PORT
#define out_port2 GPIO_HAL_NULL_PORT
#define btn_port    GPIO_HAL_NULL_PORT
#endif

static struct simple_udp_connection udp_conn;

PROCESS(aoi_border_router_process, "AoI Border Router");
AUTOSTART_PROCESSES(&aoi_border_router_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr, uint16_t sender_port, 
                const uip_ipaddr_t *receiver_addr, uint16_t receiver_port, const uint8_t *data, uint16_t datalen) {
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  #if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending response.\n");
  simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
  #endif /* WITH_SERVER_REPLY */
}
bool red = 1;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aoi_border_router_process, ev, data) {
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();
  NETSTACK_MAC.on();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  LOG_INFO("AoI Border Router started\n");

  static struct etimer et;
  while(1){
    etimer_set(&et, 5*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if(!red){
      gpio_hal_arch_write_pin(out_port1, out_pin1, 1);
      gpio_hal_arch_write_pin(out_port2, out_pin2, 0);
      red = 1;
      LOG_INFO("Red LED is on\n");
    } else {
      gpio_hal_arch_write_pin(out_port1, out_pin1, 0);
      gpio_hal_arch_write_pin(out_port2, out_pin2, 1);
      red = 0;
      LOG_INFO("Green LED is on\n");
    } 
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/