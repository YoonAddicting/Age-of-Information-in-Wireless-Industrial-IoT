#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "batmon-sensor.h" /* For temperature sensors */

#include "os/net/mac/tsch/tsch.h"

#include "os/net/ipv6/uiplib.h"
#include "project-conf.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

static void init_sensors(void){
  SENSORS_ACTIVATE(batmon_sensor);
}

/*static void get_sync_sensor_readings(void){
  int value;
  printf("-----------------------------------------\n");
  value = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
  printf("Temp=%d C\n", value);
  return;
}*/

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
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  uip_ipaddr_t python_monitor;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  init_sensors();
  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);


  NETSTACK_MAC.on();

  

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      snprintf(str, sizeof(str), "hello %d, temp %d", count, batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP));
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      uiplib_ipaddrconv("fd00:0:0:0:0:0:0:1", &python_monitor);
      LOG_INFO("And asn to ");
      LOG_INFO_6ADDR(&python_monitor);
      LOG_INFO("\n");
      snprintf(str, sizeof(str), "msb: %d, lsb: %ld", tsch_current_asn.ms1b, tsch_current_asn.ls4b);
      simple_udp_sendto(&udp_conn, str, strlen(str), &python_monitor);
      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/