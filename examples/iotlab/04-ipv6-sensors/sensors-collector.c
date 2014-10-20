#include "contiki.h"
#include <stdio.h>
#include <math.h>
#include "uip_util.h"

#include "sensors-poller.h"
#include "http-client.h"

/*---------------------------------------------------------------------------*/
PROCESS(sensors_collector, "Sensors collector");
AUTOSTART_PROCESSES(&sensors_collector);
/*---------------------------------------------------------------------------*/
extern void http_server_init();
/*---------------------------------------------------------------------------*/
PROCESS_NAME(border_router_process);
#define PROCESS_RUN(proc, arg) \
  process_start(&proc, (const char*)arg); \
  PROCESS_WAIT_EVENT_UNTIL(!process_is_running(&proc))
/*---------------------------------------------------------------------------*/
#include "state.h"
struct state state;
/*---------------------------------------------------------------------------*/
char*
get_sensors_json()
{
  static char buf[512];
  sprintf(buf, "{\n\
    \"light\": %0.2f,\n\
    \"pressure\": %0.2f,\n\
    \"accelero\": { \"x\": %d, \"y\": %d, \"z\": %d },\n\
    \"magneto\": { \"x\": %d, \"y\": %d, \"z\": %d },\n\
    \"gyro\": { \"x\": %d, \"y\": %d, \"z\": %d }\n}",
    sensor.light,
    sensor.pressure,
    sensor.acc.x, sensor.acc.y, sensor.acc.z,
    sensor.mag.x, sensor.mag.y, sensor.mag.z,
    sensor.gyr.x, sensor.gyr.y, sensor.gyr.z
  );
  return buf;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_collector, ev, data)
{
  static int is_border_router;
  static struct etimer timer;
  static struct http_request request;

  PROCESS_BEGIN();

  PROCESS_RUN(border_router_process, &is_border_router);
  printf("\nstarted as %s\n", is_border_router ? "border-router" : "node");
  uip_util_print_local_addresses();

  http_server_init();

  etimer_set(&timer, CLOCK_SECOND * 5);
  sensors_poller_init(&timer);

  while(1) {
    PROCESS_WAIT_EVENT();
    sensors_poller_process_events(ev, data);
    if(ev == PROCESS_EVENT_TIMER && state.dest_addr_set) {
      request.addr = state.dest_addr;
      request.port = state.dest_port;
      request.path = "/data-in";
      request.data = get_sensors_json();
      HTTP_CLIENT_POST(&request);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
