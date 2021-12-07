#include "contiki.h"
// For routing
#include "net/routing/routing.h" // For setting border router to root
// For MAC layer
#include "net/netstack.h" // Start TSCH
#include "os/net/mac/tsch/tsch.h" // Access ASN
// For sending UDP packets over IPv6
#include "net/ipv6/simple-udp.h"
#include "os/net/ipv6/uiplib.h"

// For GPIO
#include "dev/gpio-hal.h"
#include "arch/platform/simplelink/cc13xx-cc26xx/launchpad/cc2650/Board.h"

// Project general
#include "project-conf.h"

// Setup for logging to serial
#include "sys/log.h"
#define LOG_MODULE "AoI Border Router"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (20 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn; // UDP Connection

gpio_hal_pin_t out_pin1 = Board_PIN_RLED;
gpio_hal_pin_t out_pin2 = Board_PIN_GLED;

#if !GPIO_HAL_PORT_PIN_NUMBERING
extern gpio_hal_port_t out_port1, out_port2_3;
extern gpio_hal_port_t btn_port;
#else
#define out_port1   GPIO_HAL_NULL_PORT
#define out_port2 GPIO_HAL_NULL_PORT
#define btn_port    GPIO_HAL_NULL_PORT
#endif

/*---------------------------------------------------------------------------*/
PROCESS(aoi_border_router_process, "AoI Border Router");
AUTOSTART_PROCESSES(&aoi_border_router_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr, uint16_t sender_port, 
                const uip_ipaddr_t *receiver_addr, uint16_t receiver_port, const uint8_t *data, uint16_t datalen) {
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aoi_border_router_process, ev, data) {
  static struct etimer periodic_timer; // Timer for sending data periodically
  static char udp_buf[32]; // char[] buffer for sending data to RPi
  static uip_ipaddr_t rpi_ip; // IP for the RPi
  uiplib_ipaddrconv("fd00:0:0:0:0:0:0:1", &rpi_ip);
  // Int's for ASN
  static int ms1b;
  static long ls4b;

  PROCESS_BEGIN(); // Begin the process

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);
                    
  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();
  NETSTACK_MAC.on();

  LOG_INFO("AoI Border Router started\n");

  while(1) {
    /* Set a timer for SEND_INTERVAL seconds*/
    etimer_set(&periodic_timer, SEND_INTERVAL);
    
    /* Send ASN to RPi */
    // Set pin high, to signify that it has been measured
    gpio_hal_arch_write_pin(out_port1, out_pin1, 0);
    gpio_hal_arch_write_pin(out_port2, out_pin2, 1);
    ms1b = tsch_current_asn.ms1b;
    ls4b = tsch_current_asn.ls4b;
    // Build package to send
    snprintf(udp_buf, sizeof(udp_buf), "msb: %d, lsb: %ld", ms1b, ls4b);
    simple_udp_sendto(&udp_conn, udp_buf, strlen(udp_buf), &rpi_ip);
    // Print to serial
    LOG_INFO("Sent ASN (msb: %d, lsb: %ld) to ", ms1b, ls4b);
    LOG_INFO_6ADDR(&rpi_ip);
    LOG_INFO_("\n");
    // Set pin to low again
    gpio_hal_arch_write_pin(out_port1, out_pin1, 1);
    gpio_hal_arch_write_pin(out_port2, out_pin2, 0);
    
    /* Wait for timer to expire*/
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/