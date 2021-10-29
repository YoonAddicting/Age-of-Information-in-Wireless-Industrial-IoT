/**
 * \file
 *         Implementation of transmitting Temperature from a leaf node to a monitor node.
 * \author
 *         Jonas Bøttzauw Pedersen <jonbpe@dtu.dk>
 */

#include "contiki.h" /* For Contiki-NG */
#include "batmon-sensor.h" /* For temperature sensors */
#include "sys/etimer.h" /* For setting a timer to repeat events */
#include "os/net/linkaddr.h" /* For getting link addr */

#include "net/routing/routing.h"
#include "os/net/mac/tsch/tsch-asn.h"
#include "project-conf.h"

#include <stdio.h> /* For printf() */
#include <stdlib.h> /* for malloc */



/*---------------------------------------------------------------------------*/
PROCESS(temp_sensor_process, "Temperature Sensor Process");
AUTOSTART_PROCESSES(&temp_sensor_process);
/*---------------------------------------------------------------------------*/
static void init_sensors(void){
  SENSORS_ACTIVATE(batmon_sensor);
}

static void get_sync_sensor_readings(void){
  int value;
  printf("-----------------------------------------\n");
  value = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
  printf("Temp=%d C\n", value);
  return;
}

int is_monitor(const linkaddr_t *monitor_addr){
  return (linkaddr_cmp(&linkaddr_node_addr, monitor_addr));
}

PROCESS_THREAD(temp_sensor_process, ev, data)
{
  static struct etimer timer;
  int is_coord;

  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 10 seconds. */
  init_sensors();
  etimer_set(&timer, CLOCK_SECOND * 10);
  is_coord = 0;

  const linkaddr_t monitor_addr = {{0, 18, 75, 0, 15, 25, 176, 1}};

  if(is_monitor(&monitor_addr) != 0){
    is_coord = 1;
    NETSTACK_ROUTING.root_start();
  }

  NETSTACK_MAC.on();

  if (is_coord == 1){
      printf("This is the coordinator.!\n");
  }
  while(1) {
    get_sync_sensor_readings(); // Print sensor data.

    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
