#include "contiki.h"
// For routing
#include "net/routing/routing.h"
// For MAC layer
#include "net/netstack.h"
#include "os/net/mac/tsch/tsch.h"
// For sending UDP packets over IPv6
#include "net/ipv6/simple-udp.h"
#include "os/net/ipv6/uiplib.h"

// For GPIO
#include "dev/gpio-hal.h"
#include "arch/platform/simplelink/cc13xx-cc26xx/launchpad/cc2650/Board.h"

// Project General
#include "project-conf.h"

// Setup for logging to serial 
#include "sys/log.h"
#define LOG_MODULE "Sensor Node"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn; // UDP Connection

gpio_hal_pin_t out_pin = Board_PIN_GLED; // Pin for green LED and DIO7

#if GPIO_HAL_PORT_PIN_NUMBERING
#define out_port   GPIO_HAL_NULL_PORT
#endif

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
        {
          LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
          LOG_INFO_6ADDR(sender_addr);
          LOG_INFO_("\n");
        }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer; // Timer for sending data periodically
  static char udp_buf[32]; // char[] buffer for sending data to RPi
  static uip_ipaddr_t rpi_ip; // IP for the RPi
  uiplib_ipaddrconv("fd00:0:0:0:0:0:0:1", &rpi_ip);

  PROCESS_BEGIN(); // Begin the process

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  /* Set a timer for SEND_INTERVAL seconds*/
  etimer_set(&periodic_timer, SEND_INTERVAL);

  /* Enable the TSCH layer*/
  NETSTACK_MAC.on();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable()) {
      /* Send ASN to RPi */
      LOG_INFO("Send ASN to ");
      LOG_INFO_6ADDR(&rpi_ip);
      LOG_INFO_("\n");
      snprintf(udp_buf, sizeof(udp_buf), "msb: %d, lsb: %ld", tsch_current_asn.ms1b, tsch_current_asn.ls4b);
      gpio_hal_arch_write_pin(out_port, out_pin, 1);
      LOG_INFO("Pulled DIO7 high\n");
      simple_udp_sendto(&udp_conn, udp_buf, strlen(udp_buf), &rpi_ip);
      gpio_hal_arch_write_pin(out_port, out_pin, 0);
      LOG_INFO("Pulled DIO7 low\n");
    } else {
      LOG_INFO("RPi is not reachable yet\n");
    }

    /* Reset timer */
    etimer_set(&periodic_timer, SEND_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/