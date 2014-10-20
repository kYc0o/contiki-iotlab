#include "contiki.h"

struct sensor {
#ifdef IOTLAB_M3
  float light;    // lux
  float pressure; // mbar
#endif
  struct { int x, y, z; } acc; // mg
  struct { int x, y, z; } mag; // mgauss
  struct { int x, y, z; } gyr; // m°/s 
};

extern struct sensor sensor;

void sensors_poller_init(struct etimer *timer);
void sensors_poller_process_events(process_event_t ev, process_data_t data);
